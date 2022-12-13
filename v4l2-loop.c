/* SPDX-License-Identifier: GPL-2.0 */
/*
 * v4l2-loop.c
 *
 * Copyright (C) 2022 Lukasz Wiecaszek <lukasz.wiecaszek(at)gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License (in file COPYING) for more details.
 */

#define pr_fmt(fmt) "v4l2-loop: " fmt

#include <linux/types.h>
#include <linux/version.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/printk.h>
#include <linux/atomic.h>
#include <linux/wait.h>
#include <linux/videodev2.h>
#include <linux/dma-buf.h>

#include <asm/rwonce.h>

#include <media/v4l2-device.h>
#include <media/v4l2-ioctl.h>
#include <media/videobuf2-core.h>
#include <media/videobuf2-v4l2.h>
#include <media/videobuf2-vmalloc.h>

#define v4l2_loop_dbg_at1(args...) \
	do { if (v4l2_loop_debug_level >= 1) pr_info(args); } while (0)

#define v4l2_loop_dbg_at2(args...) \
	do { if (v4l2_loop_debug_level >= 2) pr_info(args); } while (0)

#define v4l2_loop_dbg_at3(args...) \
	do { if (v4l2_loop_debug_level >= 3) pr_info(args); } while (0)

#define v4l2_loop_dbg_at4(args...) \
	do { if (v4l2_loop_debug_level >= 4) pr_info(args); } while (0)

#define V4L2_LOOP_VERSION_MAJOR 0
#define V4L2_LOOP_VERSION_MINOR 1
#define V4L2_LOOP_VERSION_MICRO 0

#define V4L2_LOOP_VERSION_STR \
	__stringify(V4L2_LOOP_VERSION_MAJOR) "." \
	__stringify(V4L2_LOOP_VERSION_MINOR) "." \
	__stringify(V4L2_LOOP_VERSION_MICRO)

#define V4L2_LOOP_ACTIVE_OUTPUT 0U
#define V4L2_LOOP_ACTIVE_INPUT  0U

#define V4L2_LOOP_DEFAULT_FRMSIZE_MIN_WIDTH     48U
#define V4L2_LOOP_DEFAULT_FRMSIZE_MIN_HEIGHT    32U
#define V4L2_LOOP_DEFAULT_FRMSIZE_MAX_WIDTH   8192U
#define V4L2_LOOP_DEFAULT_FRMSIZE_MAX_HEIGHT  8192U
#define V4L2_LOOP_DEFAULT_FPS_MIN                1U
#define V4L2_LOOP_DEFAULT_FPS_MAX             1000U

#define V4L2_LOOP_IS_PRODUCER(type)			\
	((type) == V4L2_BUF_TYPE_VIDEO_OUTPUT		\
	|| (type) == V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE)

#define V4L2_LOOP_IS_CONSUMER(type)			\
	((type) == V4L2_BUF_TYPE_VIDEO_CAPTURE		\
	|| (type) == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE)

/* module's params */
static int v4l2_loop_debug_level = 0; /* do not emmit any traces by default */
module_param_named(debug, v4l2_loop_debug_level, int, 0660);
MODULE_PARM_DESC(debug,
	"Verbosity of debug messages (range: [0(none)-4(max)], default: 0)");

static int v4l2_loop_devices = 1; /* number of virtual v4l2 devices created by this module */
module_param_named(devices, v4l2_loop_devices, int, 0660);
MODULE_PARM_DESC(devices,
	"Number of virtual v4l2 devices created by this module (default: 1)");

static int v4l2_loop_buffers = 2; /* minimum number of buffers needed before start streaming can be called */
module_param_named(buffers, v4l2_loop_buffers, int, 0660);
MODULE_PARM_DESC(buffers,
	"Minimum number of buffers needed before start streaming can be called (default: 2)");

static bool v4l2_loop_mplane = 0;
module_param_named(mplane, v4l2_loop_mplane, bool, 0660);
MODULE_PARM_DESC(mplane,
	"Whether v4l2-loop shall support multiple frame formats (default: false)");

static LIST_HEAD(v4l2_loop_devices_list);

#define V4L2_LOOP_MAX_PLANES 4
struct v4l2_loop_fmtdesc
{
	__u32	pixelformat;	/* format fourcc */
	const char *name;	/* format description */
	bool is_compressed;	/* is it compressed format */
	int planes;		/* number of planes */
	struct v4l2_fract depth[V4L2_LOOP_MAX_PLANES];	/* plane's depth */
};

/* A producer/output buffer */
struct v4l2_loop_pbuf
{
	struct vb2_v4l2_buffer vbuf;
	struct list_head pnode;		/* a node on producer 'queued_bufs' list */
};

/* A consumer/capture buffer */
struct v4l2_loop_cbuf
{
	struct vb2_v4l2_buffer vbuf;
	struct list_head cnode;		/* a node on consumer 'queued_bufs' list */
	struct v4l2_loop_pbuf *pbuf;	/* associated producer buffer */

};

enum v4l2_loop_handle_type {
	V4L2_LOOP_HANDLE_UNDEFINED,
	V4L2_LOOP_HANDLE_PRODUCER,	/* the one who requests output buffers */
	V4L2_LOOP_HANDLE_CONSUMER	/* the one who requests capture buffers */
};

struct v4l2_loop_producer_handle {
};

struct v4l2_loop_consumer_handle {
	__u32 buffers;
	struct v4l2_loop_cbuf *bufs;
	struct list_head queued_bufs;	/* consumer buffers will be queued here */
};

struct v4l2_loop_handle {
	struct v4l2_fh fh;
	enum v4l2_loop_handle_type htype;
	union {
		struct v4l2_loop_producer_handle p;
		struct v4l2_loop_consumer_handle c;
	};
};

struct v4l2_loop_device {
	struct v4l2_device v4l2_dev;
	struct video_device vdev;
	struct list_head node; 		/* a node on the 'v4l2_loop_devices_list' */
	struct v4l2_format format;	/* format as set by the producer */
	struct v4l2_captureparm captureparm;
	struct v4l2_outputparm outputparm;

	struct mutex vb_queue_lock;	/* protects vb_queue */
	struct vb2_queue vb_queue;

	spinlock_t queued_bufs_lock;	/* protects 'queued_bufs' list */
	struct list_head queued_bufs;	/* producer buffers will be queued here */

	wait_queue_head_t waiting_consumers;
	unsigned sequence;		/* buffer sequence counter */
};

static const struct v4l2_loop_fmtdesc v4l2_loop_fmtdescs_splanes[] = {
#include "v4l2-loop-fmtdesc-splanes.h"
};

static const struct v4l2_loop_fmtdesc v4l2_loop_fmtdescs_mplanes[] = {
#include "v4l2-loop-fmtdesc-splanes.h"
#include "v4l2-loop-fmtdesc-mplanes.h"
};

#include "v4l2-loop-print-functions.h"

//TODO: Shall I move following function to something like v4l2-loop-helpers.h
static int v4l2_loop_validate_planes(struct vb2_buffer *vb, struct v4l2_buffer *buffer)
{
	if (!V4L2_TYPE_IS_MULTIPLANAR(vb->type))
		return 0;

	/* Is memory for copying plane information present? */
	if (buffer->m.planes == NULL) {
		v4l2_loop_dbg_at1(
			"%s() multi-planar scheme used but planes array not provided\n",
			__func__);
		return -EINVAL;
	}

	if (buffer->length < vb->num_planes || buffer->length > VB2_MAX_PLANES) {
		v4l2_loop_dbg_at1("%s() incorrect planes array length, expected %d, got %d\n",
			__func__, vb->num_planes, buffer->length);
		return -EINVAL;
	}

	return 0;
}

static void v4l2_loop_release_cplanes(struct v4l2_loop_cbuf *cbuf)
{
	__u32 plane;
	struct vb2_buffer *vb = &cbuf->vbuf.vb2_buf;

	for (plane = 0; plane < vb->num_planes; plane++) {
		struct vb2_plane *p = &vb->planes[plane];
		if (p->dbuf) { /* release previous mapping (if any) */
			if (p->mem_priv) {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 18, 0)
				struct iosys_map map =
					IOSYS_MAP_INIT_VADDR(p->mem_priv);
#else
				struct dma_buf_map map =
					DMA_BUF_MAP_INIT_VADDR(p->mem_priv);
#endif
				dma_buf_vunmap(p->dbuf, &map);
			}
			dma_buf_put(p->dbuf);
		}
	}

}

static void v4l2_loop_release_cbufs(struct v4l2_loop_consumer_handle *c)
{
	if (c->bufs) {
		__u32 i;

		v4l2_loop_dbg_at2("releasing %u consumer buffers\n", c->buffers);

		for (i = 0; i < c->buffers; i++) {
			struct v4l2_loop_cbuf *cbuf = &c->bufs[i];
			v4l2_loop_release_cplanes(cbuf);
		}

		kfree(c->bufs);
		c->bufs = NULL;
	}
}

static int v4l2_loop_fill_user_buffer_mplane_mmap(
	struct v4l2_loop_pbuf *pbuf, struct v4l2_loop_cbuf *cbuf, struct v4l2_buffer *buffer)
{
	__u32 plane;
	__u32 num_planes = cbuf->vbuf.vb2_buf.num_planes;

	buffer->length = num_planes;
	for (plane = 0; plane < num_planes; ++plane) {
		struct v4l2_plane *dst = &buffer->m.planes[plane];
		struct vb2_plane *psrc = &pbuf->vbuf.vb2_buf.planes[plane];
		struct vb2_plane *csrc = &cbuf->vbuf.vb2_buf.planes[plane];

		memset(dst->reserved, 0, sizeof(dst->reserved));

		if (cbuf->vbuf.vb2_buf.memory == VB2_MEMORY_MMAP) {
			dst->m.mem_offset = psrc->m.offset;
			dst->length = psrc->length;
			dst->bytesused = psrc->bytesused;
			dst->data_offset = psrc->data_offset;
		}
		else
		if (cbuf->vbuf.vb2_buf.memory == VB2_MEMORY_USERPTR) {
			void *vaddr;

			vaddr = vb2_plane_vaddr(&pbuf->vbuf.vb2_buf, plane);
			if (!vaddr) {
				v4l2_loop_dbg_at1("cannot obtain vaddr of producer plane %d\n", plane);
				return -EFAULT;
			}

			if (!csrc->m.userptr)
				return -EFAULT;

			dst->m.userptr = csrc->m.userptr;
			dst->length = csrc->length;
			dst->bytesused = min(psrc->bytesused, dst->length);
			WARN_ON(copy_to_user((void*)dst->m.userptr, vaddr, dst->bytesused));
		}
		else
		if (cbuf->vbuf.vb2_buf.memory == VB2_MEMORY_DMABUF) {
			void *vaddr;
			struct dma_buf *dbuf;

			vaddr = vb2_plane_vaddr(&pbuf->vbuf.vb2_buf, plane);
			if (!vaddr) {
				v4l2_loop_dbg_at1("cannot obtain vaddr of producer plane %d\n", plane);
				return -EFAULT;
			}

			dbuf = dma_buf_get(csrc->m.fd);
			if (IS_ERR_OR_NULL(dbuf)) {
				v4l2_loop_dbg_at1("invalid dmabuf fd for plane %d\n", plane);
				return -EFAULT;
			}

			dst->m.fd = csrc->m.fd;
			dst->length = csrc->length ?: dbuf->size; /* use DMABUF size if length is not provided */
			dst->bytesused = min(psrc->bytesused, dst->length);

			if (csrc->dbuf != dbuf) {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 18, 0)
				struct iosys_map map = IOSYS_MAP_INIT_VADDR(csrc->mem_priv);
#else
				struct dma_buf_map map = DMA_BUF_MAP_INIT_VADDR(csrc->mem_priv);
#endif
				if (csrc->dbuf && csrc->mem_priv) /* release previous mapping (if any) */
					dma_buf_vunmap(dbuf, &map);
				if (!dma_buf_vmap(dbuf, &map)) { /* create new mapping */
					csrc->dbuf = dbuf;
					csrc->mem_priv = map.vaddr;
				} else {
					csrc->dbuf = NULL;
					csrc->mem_priv = NULL;
					dma_buf_put(dbuf);
				}
			}
			else
				dma_buf_put(dbuf);

			if (!csrc->mem_priv)
				return -EFAULT;

			memcpy(csrc->mem_priv, vaddr, dst->bytesused);
		}
		else
			return -EFAULT;
	}

	return 0;
}

static int v4l2_loop_fill_user_buffer_mplane_userptr(
	struct v4l2_loop_pbuf *pbuf, struct v4l2_loop_cbuf *cbuf, struct v4l2_buffer *buffer)
{
	return -ENOSYS; /* not implemented */
}

static int v4l2_loop_fill_user_buffer_mplane_dmabuf(
	struct v4l2_loop_pbuf *pbuf, struct v4l2_loop_cbuf *cbuf, struct v4l2_buffer *buffer)
{
	return -ENOSYS; /* not implemented */
}

static int v4l2_loop_fill_user_buffer_mplane(
	struct v4l2_loop_pbuf *pbuf, struct v4l2_loop_cbuf *cbuf, struct v4l2_buffer *buffer)
{
	switch (pbuf->vbuf.vb2_buf.memory) {
	case VB2_MEMORY_MMAP:
		return v4l2_loop_fill_user_buffer_mplane_mmap(pbuf, cbuf, buffer);

	case VB2_MEMORY_USERPTR:
		return v4l2_loop_fill_user_buffer_mplane_userptr(pbuf, cbuf, buffer);

	case VB2_MEMORY_DMABUF:
		return v4l2_loop_fill_user_buffer_mplane_dmabuf(pbuf, cbuf, buffer);

	default:
		return -EFAULT;
	}
}

static int v4l2_loop_fill_user_buffer_splane_mmap(
	struct v4l2_loop_pbuf *pbuf, struct v4l2_loop_cbuf *cbuf, struct v4l2_buffer *buffer)
{
	struct vb2_plane *psrc = &pbuf->vbuf.vb2_buf.planes[0];
	struct vb2_plane *csrc = &cbuf->vbuf.vb2_buf.planes[0];

	if (cbuf->vbuf.vb2_buf.memory == VB2_MEMORY_MMAP) {
		buffer->m.offset = psrc->m.offset;
		buffer->length = psrc->length;
		buffer->bytesused = psrc->bytesused;
	}
	else
	if (cbuf->vbuf.vb2_buf.memory == VB2_MEMORY_USERPTR) {
		void *vaddr;

		vaddr = vb2_plane_vaddr(&pbuf->vbuf.vb2_buf, 0);
		if (!vaddr) {
			v4l2_loop_dbg_at1("cannot obtain vaddr of producer plane %d\n", 0);
			return -EFAULT;
		}

		if (!csrc->m.userptr)
			return -EFAULT;

		buffer->m.userptr = csrc->m.userptr;
		buffer->length = csrc->length;
		buffer->bytesused = min(psrc->bytesused, buffer->length);
		WARN_ON(copy_to_user((void*)buffer->m.userptr, vaddr, buffer->bytesused));
	}
	else
	if (cbuf->vbuf.vb2_buf.memory == VB2_MEMORY_DMABUF) {
		void *vaddr;
		struct dma_buf *dbuf;

		vaddr = vb2_plane_vaddr(&pbuf->vbuf.vb2_buf, 0);
		if (!vaddr) {
			v4l2_loop_dbg_at1("cannot obtain vaddr of producer plane %d\n", 0);
			return -EFAULT;
		}

		dbuf = dma_buf_get(csrc->m.fd);
		if (IS_ERR_OR_NULL(dbuf)) {
			v4l2_loop_dbg_at1("invalid dmabuf fd for plane %d\n", 0);
			return -EFAULT;
		}

		buffer->m.fd = csrc->m.fd;
		buffer->length = csrc->length ?: dbuf->size; /* use DMABUF size if length is not provided */
		buffer->bytesused = min(psrc->bytesused, buffer->length);

		if (csrc->dbuf != dbuf) {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 18, 0)
			struct iosys_map map = IOSYS_MAP_INIT_VADDR(csrc->mem_priv);
#else
			struct dma_buf_map map = DMA_BUF_MAP_INIT_VADDR(csrc->mem_priv);
#endif
			if (csrc->dbuf && csrc->mem_priv) /* release previous mapping (if any) */
				dma_buf_vunmap(dbuf, &map);
			if (!dma_buf_vmap(dbuf, &map)) { /* create new mapping */
				csrc->dbuf = dbuf;
				csrc->mem_priv = map.vaddr;
			} else {
				csrc->dbuf = NULL;
				csrc->mem_priv = NULL;
				dma_buf_put(dbuf);
			}
		}
		else
			dma_buf_put(dbuf);

		if (!csrc->mem_priv)
			return -EFAULT;

		memcpy(csrc->mem_priv, vaddr, buffer->bytesused);
	}
	else
		return -EFAULT;

	return 0;
}

static int v4l2_loop_fill_user_buffer_splane_userptr(
	struct v4l2_loop_pbuf *pbuf, struct v4l2_loop_cbuf *cbuf, struct v4l2_buffer *buffer)
{
	return -ENOSYS; /* not implemented */
}

static int v4l2_loop_fill_user_buffer_splane_dmabuf(
	struct v4l2_loop_pbuf *pbuf, struct v4l2_loop_cbuf *cbuf, struct v4l2_buffer *buffer)
{
	return -ENOSYS; /* not implemented */
}

static int v4l2_loop_fill_user_buffer_splane(
	struct v4l2_loop_pbuf *pbuf, struct v4l2_loop_cbuf *cbuf, struct v4l2_buffer *buffer)
{
	switch (pbuf->vbuf.vb2_buf.memory) {
	case VB2_MEMORY_MMAP:
		return v4l2_loop_fill_user_buffer_splane_mmap(pbuf, cbuf, buffer);

	case VB2_MEMORY_USERPTR:
		return v4l2_loop_fill_user_buffer_splane_userptr(pbuf, cbuf, buffer);

	case VB2_MEMORY_DMABUF:
		return v4l2_loop_fill_user_buffer_splane_dmabuf(pbuf, cbuf, buffer);

	default:
		return -EFAULT;
	}
}

static int v4l2_loop_fill_user_buffer(
	struct v4l2_loop_pbuf *pbuf, struct v4l2_loop_cbuf *cbuf, struct v4l2_buffer *buffer)
{
	buffer->index = cbuf->vbuf.vb2_buf.index;
	buffer->type = cbuf->vbuf.vb2_buf.type;
	buffer->bytesused = 0;
	buffer->flags = V4L2_BUF_FLAG_TIMESTAMP_COPY;
	if (vb2_buffer_in_use(pbuf->vbuf.vb2_buf.vb2_queue, &pbuf->vbuf.vb2_buf))
		buffer->flags |= V4L2_BUF_FLAG_MAPPED;
	if (pbuf->vbuf.request_fd >= 0) {
		buffer->flags |= V4L2_BUF_FLAG_REQUEST_FD;
		buffer->request_fd = pbuf->vbuf.request_fd;
	}
	buffer->field = pbuf->vbuf.field;
	v4l2_buffer_set_timestamp(buffer, pbuf->vbuf.vb2_buf.timestamp);
	buffer->timecode = pbuf->vbuf.timecode;
	buffer->sequence = pbuf->vbuf.sequence;
	buffer->reserved2 = 0;
	buffer->request_fd = -1;

	return V4L2_TYPE_IS_MULTIPLANAR(buffer->type) ?
		v4l2_loop_fill_user_buffer_mplane(pbuf, cbuf, buffer) :
		v4l2_loop_fill_user_buffer_splane(pbuf, cbuf, buffer);
}

static void v4l2_loop_fill_vb2_buffer_mplane(struct v4l2_buffer *buffer, struct vb2_buffer *vb)
{
	__u32 plane;
	__u32 num_planes = buffer->length;

	for (plane = 0; plane < num_planes; ++plane) {
		if (vb->memory == VB2_MEMORY_USERPTR) {
			vb->planes[plane].m.userptr = buffer->m.planes[plane].m.userptr;
			vb->planes[plane].length = buffer->m.planes[plane].length;
		}
		else
		if (vb->memory == VB2_MEMORY_DMABUF) {
			vb->planes[plane].m.fd = buffer->m.planes[plane].m.fd;
			vb->planes[plane].length = buffer->m.planes[plane].length;
		}
		else
			; /* do nothing */
	}
}

static void v4l2_loop_fill_vb2_buffer_splane(struct v4l2_buffer *buffer, struct vb2_buffer *vb)
{
	if (vb->memory == VB2_MEMORY_USERPTR) {
		vb->planes[0].m.userptr = buffer->m.userptr;
		vb->planes[0].length = buffer->length;
	}
	else
	if (vb->memory == VB2_MEMORY_DMABUF) {
		vb->planes[0].m.fd = buffer->m.fd;
		vb->planes[0].length = buffer->length;
	}
	else
		; /* do nothing */
}

static void v4l2_loop_fill_vb2_buffer(struct v4l2_buffer *buffer, struct vb2_buffer *vb)
{
	if (V4L2_TYPE_IS_MULTIPLANAR(buffer->type))
		v4l2_loop_fill_vb2_buffer_mplane(buffer, vb);
	else
		v4l2_loop_fill_vb2_buffer_splane(buffer, vb);
}

static const char* v4l2_loop_handle_name(struct v4l2_loop_handle *h)
{
	if (h->htype == V4L2_LOOP_HANDLE_PRODUCER)
		return "producer";
	else
	if (h->htype == V4L2_LOOP_HANDLE_CONSUMER)
		return "consumer";
	else
		return "undefined";
}

static struct v4l2_loop_pbuf *v4l2_loop_pbuf(struct vb2_buffer *vb)
{
	struct vb2_v4l2_buffer *vbuf = to_vb2_v4l2_buffer(vb);
	return container_of(vbuf, struct v4l2_loop_pbuf, vbuf);
}

static int v4l2_loop_buffer_is_available(struct v4l2_loop_device *dev)
{
	unsigned long flags;
	int available;

	spin_lock_irqsave(&dev->queued_bufs_lock, flags);
	available = !list_empty(&dev->queued_bufs);
	spin_unlock_irqrestore(&dev->queued_bufs_lock, flags);

	return available;
}

static int v4l2_loop_validate_buffer_types(struct v4l2_loop_device *dev, enum v4l2_buf_type type)
{
	if (type != V4L2_BUF_TYPE_VIDEO_CAPTURE &&
		type != V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE) {
		v4l2_loop_dbg_at1("unsuported buffer type\n");
		return -EINVAL;
	}

	if (!dev->format.type) { /* format is not yet set by the producer */
		v4l2_loop_dbg_at2("format is not yet set by the producer\n");
		return -EINVAL;
	}

	if (dev->format.type == V4L2_BUF_TYPE_VIDEO_OUTPUT) {
		/* producer uses single planar format */
		if (type != V4L2_BUF_TYPE_VIDEO_CAPTURE) {
			v4l2_loop_dbg_at1("incompatible buffer types "
				"(producer: single planar, consumer: multi planar)\n");
			return -EINVAL;
		}
	} else
	if (dev->format.type == V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE) {
		/* producer uses multi planar format */
		if (type != V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE) {
			v4l2_loop_dbg_at1("incompatible buffer types "
				"(producer: multi planar, consumer: single planar)\n");
			return -EINVAL;
		}
	} else
		return -EINVAL;

	return 0;
}

static int v4l2_loop_queue_setup(struct vb2_queue *vq, unsigned int *nbuffers,
	unsigned int *nplanes, unsigned int sizes[], struct device *alloc_devs[])
{
	struct v4l2_loop_device *dev = vb2_get_drv_priv(vq);

	if (!dev->format.type) { /* format is not yet set by the producer */
		v4l2_loop_dbg_at2("format is not yet set by the producer\n");
		*nbuffers = 0;
		*nplanes = 0;
		return -EINVAL;
	}

	v4l2_loop_print_format(&dev->format);

	if (vq->num_buffers + *nbuffers < v4l2_loop_buffers)
		*nbuffers = v4l2_loop_buffers - vq->num_buffers;
	if (V4L2_TYPE_IS_MULTIPLANAR(dev->format.type)) {
		int i;
		*nplanes = dev->format.fmt.pix_mp.num_planes;
		for (i = 0; i < *nplanes; ++i)
			sizes[i] = PAGE_ALIGN(dev->format.fmt.pix_mp.plane_fmt[i].sizeimage);
	} else {
		*nplanes = 1;
		sizes[0] = PAGE_ALIGN(dev->format.fmt.pix.sizeimage);
	}

	v4l2_loop_dbg_at2("%s(%s) nbuffers: %u, nplanes: %u\n",
		__func__, video_device_node_name(&dev->vdev), *nbuffers, *nplanes);

	return 0;
}

static int v4l2_loop_queue_buf_init(struct vb2_buffer *vb)
{
	return 0;
}

static void v4l2_loop_queue_buf_queue(struct vb2_buffer *vb)
{
	struct v4l2_loop_device *dev = vb2_get_drv_priv(vb->vb2_queue);
	struct v4l2_loop_pbuf *pbuf = v4l2_loop_pbuf(vb);
	unsigned long flags;

	spin_lock_irqsave(&dev->queued_bufs_lock, flags);
	list_add_tail(&pbuf->pnode, &dev->queued_bufs);
	spin_unlock_irqrestore(&dev->queued_bufs_lock, flags);

	wake_up_all(&dev->waiting_consumers);
}

static int v4l2_loop_queue_start_streaming(struct vb2_queue *vq, unsigned int i)
{
	struct v4l2_loop_device *dev = vb2_get_drv_priv(vq);

	dev->sequence = 0;

	return 0;
}

static void v4l2_loop_queue_stop_streaming(struct vb2_queue *vq)
{
	struct v4l2_loop_device *dev = vb2_get_drv_priv(vq);
	unsigned long flags;

	spin_lock_irqsave(&dev->queued_bufs_lock, flags); {
		struct v4l2_loop_pbuf *pbuf;
		list_for_each_entry(pbuf, &dev->queued_bufs, pnode)
			vb2_buffer_done(&pbuf->vbuf.vb2_buf, VB2_BUF_STATE_ERROR);
		INIT_LIST_HEAD(&dev->queued_bufs);
	} spin_unlock_irqrestore(&dev->queued_bufs_lock, flags);

	wake_up_all(&dev->waiting_consumers);
}

static void v4l2_loop_queue_wait_prepare(struct vb2_queue *vq)
{
	mutex_unlock(vq->lock);
}

static void v4l2_loop_queue_wait_finish(struct vb2_queue *vq)
{
	mutex_lock(vq->lock);
}

static const struct vb2_ops v4l2_loop_vb2_ops = {
	.queue_setup       = v4l2_loop_queue_setup,
	.buf_init          = v4l2_loop_queue_buf_init,
	.buf_queue         = v4l2_loop_queue_buf_queue,
	.start_streaming   = v4l2_loop_queue_start_streaming,
	.stop_streaming    = v4l2_loop_queue_stop_streaming,
	.wait_prepare      = v4l2_loop_queue_wait_prepare,
	.wait_finish       = v4l2_loop_queue_wait_finish
};

static int v4l2_loop_open(struct file *file)
{
	struct video_device *vdev = video_devdata(file);
	struct v4l2_loop_handle *h;

	v4l2_loop_dbg_at3("%s(%s)\n", __func__, video_device_node_name(vdev));

	h = kzalloc(sizeof(*h), GFP_KERNEL);
	if (h == NULL) {
		file->private_data = NULL;
		return -ENOMEM;
	}

	h->htype = V4L2_LOOP_HANDLE_UNDEFINED;
	INIT_LIST_HEAD(&h->c.queued_bufs);

	file->private_data = &h->fh;
	v4l2_fh_init(&h->fh, vdev);
	v4l2_fh_add(&h->fh);

	return 0;
}

static int v4l2_loop_close(struct file *file)
{
	struct video_device *vdev = video_devdata(file);
	struct v4l2_loop_device *dev =
		container_of(vdev, struct v4l2_loop_device, vdev);
	struct v4l2_loop_handle *h;

	v4l2_loop_dbg_at3("%s(%s)\n", __func__, video_device_node_name(vdev));

	h = container_of(file->private_data, struct v4l2_loop_handle, fh);

	if (h->htype == V4L2_LOOP_HANDLE_PRODUCER) {
		memset(&dev->format, 0, sizeof(dev->format));
		memset(&dev->captureparm, 0, sizeof(dev->captureparm));
		memset(&dev->outputparm, 0, sizeof(dev->outputparm));
	}
	else
	if (h->htype == V4L2_LOOP_HANDLE_CONSUMER) {
		v4l2_loop_release_cbufs(&h->c);
	}
	else {
		/* do nothing */
	}

	v4l2_fh_del(&h->fh);
	v4l2_fh_exit(&h->fh);

	file->private_data = NULL;
	kfree(h);

	return 0;
}

static ssize_t v4l2_loop_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
	struct video_device *vdev = video_devdata(file);

	v4l2_loop_dbg_at3("%s(%s)\n", __func__, video_device_node_name(vdev));

	return -ENOSYS; /* not implemented yet */;
}

static ssize_t v4l2_loop_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
	struct video_device *vdev = video_devdata(file);

	v4l2_loop_dbg_at3("%s(%s)\n", __func__, video_device_node_name(vdev));

	return -ENOSYS; /* not implemented yet */;
}

static __poll_t v4l2_loop_poll(struct file *file, struct poll_table_struct *poll)
{
	struct video_device *vdev = video_devdata(file);
	struct v4l2_loop_device *dev =
		container_of(vdev, struct v4l2_loop_device, vdev);
	struct v4l2_loop_handle *h =
		container_of(file->private_data, struct v4l2_loop_handle, fh);
	__poll_t events = poll_requested_events(poll);
	__poll_t revents = EPOLLERR; /* returned events */

	v4l2_loop_dbg_at3("%s(%s, %s) events: 0x%08x\n",
		__func__, video_device_node_name(vdev), v4l2_loop_handle_name(h), events);

	if (h->htype == V4L2_LOOP_HANDLE_PRODUCER) {
		int status;
		void *fileio;

		status = mutex_lock_interruptible(&dev->vb_queue_lock);
		if (status)
			return EPOLLERR;

		fileio = vdev->queue->fileio;

		revents = vb2_poll(vdev->queue, file, poll);

		/* If fileio was started, then we have a new queue owner. */
		if (!fileio && vdev->queue->fileio)
			vdev->queue->owner = file->private_data;

		mutex_unlock(&dev->vb_queue_lock);
	} else
	if (h->htype == V4L2_LOOP_HANDLE_CONSUMER) {
		poll_wait(file, &dev->waiting_consumers, poll);

		if (!(events & (EPOLLIN | EPOLLRDNORM)))
			return 0;

		if (!vdev->queue->streaming || vdev->queue->error)
			return EPOLLERR;

		if (v4l2_loop_buffer_is_available(dev))
			return (EPOLLIN | EPOLLRDNORM);

		revents = 0;
	}
	else
		; /* do nothing */

	return revents;
}

static const struct v4l2_file_operations v4l2_loop_fops = {
	.owner		= THIS_MODULE,
	.open		= v4l2_loop_open,
	.release	= v4l2_loop_close,
	.read		= v4l2_loop_read,
	.write		= v4l2_loop_write,
	.poll		= v4l2_loop_poll,
	.mmap		= vb2_fop_mmap,
	.unlocked_ioctl	= video_ioctl2
};

static int v4l2_loop_querycap(struct file *file, void *priv, struct v4l2_capability *capability)
{
	struct video_device *vdev = video_devdata(file);

	v4l2_loop_dbg_at3("%s(%s)\n", __func__, video_device_node_name(vdev));

	strscpy(capability->driver, KBUILD_MODNAME, sizeof(capability->driver));
	strscpy(capability->card, vdev->name, sizeof(capability->card));
	snprintf(capability->bus_info, sizeof(capability->bus_info), "virtual:%s", vdev->name);

	return 0;
}

static int v4l2_loop_enum_framesizes(struct file *file, void *fh, struct v4l2_frmsizeenum *frmsizeenum)
{
	struct video_device *vdev = video_devdata(file);
	struct v4l2_loop_device *dev =
		container_of(vdev, struct v4l2_loop_device, vdev);

	v4l2_loop_dbg_at3("%s(%s)\n", __func__, video_device_node_name(vdev));

	if (frmsizeenum->index > 0)
		return -EINVAL;

	if (dev->format.type) { /* format is already set by the producer */
		if (frmsizeenum->pixel_format != dev->format.fmt.pix.pixelformat)
			return -EINVAL;

		frmsizeenum->type = V4L2_FRMSIZE_TYPE_DISCRETE;
		frmsizeenum->discrete.width = dev->format.fmt.pix.width;
		frmsizeenum->discrete.height = dev->format.fmt.pix.height;
	} else { /* format is not negotiated yet, allow for wide range of resolutions */
		frmsizeenum->type = V4L2_FRMSIZE_TYPE_CONTINUOUS;
		frmsizeenum->stepwise.min_width = V4L2_LOOP_DEFAULT_FRMSIZE_MIN_WIDTH;
		frmsizeenum->stepwise.max_width = V4L2_LOOP_DEFAULT_FRMSIZE_MAX_WIDTH;
		frmsizeenum->stepwise.step_width = 1;
		frmsizeenum->stepwise.min_height = V4L2_LOOP_DEFAULT_FRMSIZE_MIN_HEIGHT;
		frmsizeenum->stepwise.max_height = V4L2_LOOP_DEFAULT_FRMSIZE_MAX_HEIGHT;
		frmsizeenum->stepwise.step_height = 1;
	}

	v4l2_loop_print_frmsizeenum(frmsizeenum);

	return 0;
}

static int v4l2_loop_enum_frameintervals(struct file *file, void *fh, struct v4l2_frmivalenum *frmivalenum)
{
	struct video_device *vdev = video_devdata(file);
	struct v4l2_loop_device *dev =
		container_of(vdev, struct v4l2_loop_device, vdev);

	v4l2_loop_dbg_at3("%s(%s)\n", __func__, video_device_node_name(vdev));

	if (frmivalenum->index > 0)
		return -EINVAL;

	if (dev->format.type) { /* format is already set by the producer */
		if (frmivalenum->pixel_format != dev->format.fmt.pix.pixelformat)
			return -EINVAL;

		if (frmivalenum->width != dev->format.fmt.pix.width)
			return -EINVAL;

		if (frmivalenum->height != dev->format.fmt.pix.height)
			return -EINVAL;

		frmivalenum->type = V4L2_FRMIVAL_TYPE_DISCRETE;
		frmivalenum->discrete = dev->captureparm.timeperframe;
	} else { /* format is not negotiated yet, allow for wide range of frame intervals */
		frmivalenum->type = V4L2_FRMIVAL_TYPE_CONTINUOUS;
		frmivalenum->stepwise.min.numerator = 1;
		frmivalenum->stepwise.min.denominator = V4L2_LOOP_DEFAULT_FPS_MAX;
		frmivalenum->stepwise.max.numerator = 1;
		frmivalenum->stepwise.max.denominator = V4L2_LOOP_DEFAULT_FPS_MIN;
		frmivalenum->stepwise.step.numerator = 1;
		frmivalenum->stepwise.step.denominator = V4L2_LOOP_DEFAULT_FPS_MAX;
	}

	v4l2_loop_print_frmivalenum(frmivalenum);

	return 0;
}

static int v4l2_loop_enum_output(struct file *file, void *fh, struct v4l2_output *output)
{
	struct video_device *vdev = video_devdata(file);
	__u32 index = output->index;

	v4l2_loop_dbg_at3("%s(%s)\n", __func__, video_device_node_name(vdev));


	if (V4L2_LOOP_ACTIVE_OUTPUT != index)
		return -EINVAL;

	memset(output, 0, sizeof(*output));

	output->index = index;
	snprintf(output->name, sizeof(output->name), "out%u", index);

	return 0;
}

static int v4l2_loop_g_output(struct file *file, void *fh, unsigned int *i)
{
	struct video_device *vdev = video_devdata(file);

	v4l2_loop_dbg_at3("%s(%s)\n", __func__, video_device_node_name(vdev));

	if (i)
		*i = V4L2_LOOP_ACTIVE_OUTPUT;

	return 0;
}

static int v4l2_loop_s_output(struct file *file, void *fh, unsigned int i)
{
	struct video_device *vdev = video_devdata(file);

	v4l2_loop_dbg_at3("%s(%s)\n", __func__, video_device_node_name(vdev));

	if (i != V4L2_LOOP_ACTIVE_OUTPUT)
		return -EINVAL;

	return 0;
}

static int v4l2_loop_enum_input(struct file *file, void *fh, struct v4l2_input *input)
{
	struct video_device *vdev = video_devdata(file);
	__u32 index = input->index;

	v4l2_loop_dbg_at3("%s(%s)\n", __func__, video_device_node_name(vdev));

	if (V4L2_LOOP_ACTIVE_INPUT != index)
		return -EINVAL;

	memset(input, 0, sizeof(*input));

	input->index = index;
	snprintf(input->name, sizeof(input->name), "in%u", index);

	return 0;
}

static int v4l2_loop_g_input(struct file *file, void *fh, unsigned int *i)
{
	struct video_device *vdev = video_devdata(file);

	v4l2_loop_dbg_at3("%s(%s)\n", __func__, video_device_node_name(vdev));

	if (i)
		*i = V4L2_LOOP_ACTIVE_INPUT;

	return 0;
}

static int v4l2_loop_s_input(struct file *file, void *fh, unsigned int i)
{
	struct video_device *vdev = video_devdata(file);

	v4l2_loop_dbg_at3("%s(%s)\n", __func__, video_device_node_name(vdev));

	if (i != V4L2_LOOP_ACTIVE_INPUT)
		return -EINVAL;

	return 0;
}

static int v4l2_loop_enum_fmt_cap(struct file *file, void *fh, struct v4l2_fmtdesc *fmtdesc)
{
	struct video_device *vdev = video_devdata(file);
	struct v4l2_loop_device *dev =
		container_of(vdev, struct v4l2_loop_device, vdev);
	const struct v4l2_loop_fmtdesc *f = NULL;
	size_t i;

	v4l2_loop_dbg_at3("%s(%s)\n", __func__, video_device_node_name(vdev));

	if (fmtdesc->index > 0)
		return -EINVAL;

	if (!dev->format.type) {/* format is not yet set by the producer */
		v4l2_loop_dbg_at2("%s(%s) format is not yet set by the producer\n",
			__func__, video_device_node_name(vdev));
		return -EINVAL;
	}

	if (dev->format.type == V4L2_BUF_TYPE_VIDEO_OUTPUT) {
		/* producer uses single planar format */
		for (i = 0; i < ARRAY_SIZE(v4l2_loop_fmtdescs_splanes); ++i) {
			f = &v4l2_loop_fmtdescs_splanes[i];
			if (f->pixelformat == dev->format.fmt.pix.pixelformat)
				break;
		}

		if (i == ARRAY_SIZE(v4l2_loop_fmtdescs_splanes))
			return -EINVAL;
	} else
	if (dev->format.type == V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE) {
		/* producer uses multi planar format */
		for (i = 0; i < ARRAY_SIZE(v4l2_loop_fmtdescs_mplanes); ++i) {
			f = &v4l2_loop_fmtdescs_mplanes[i];
			if (f->pixelformat == dev->format.fmt.pix_mp.pixelformat)
				break;
		}

		if (i == ARRAY_SIZE(v4l2_loop_fmtdescs_mplanes))
			return -EINVAL;
	} else {
		/* do nothing */
	}

	if (f == NULL)
		return -EINVAL;

	fmtdesc->flags = f->is_compressed ? V4L2_FMT_FLAG_COMPRESSED : 0;
	strscpy(fmtdesc->description, f->name, sizeof(fmtdesc->description));
	fmtdesc->pixelformat = f->pixelformat;

	return 0;
}

static int v4l2_loop_s_fmt_cap(struct file *file, void *priv, struct v4l2_format *format)
{
	struct video_device *vdev = video_devdata(file);
	struct v4l2_loop_device *dev =
		container_of(vdev, struct v4l2_loop_device, vdev);

	v4l2_loop_dbg_at3("%s(%s)\n", __func__, video_device_node_name(vdev));

	if (!dev->format.type) { /* format is not yet set by the producer */
		v4l2_loop_dbg_at2("%s(%s) format is not yet set by the producer\n",
			__func__, video_device_node_name(vdev));
		return -EINVAL;
	}

	if (dev->format.type != V4L2_BUF_TYPE_VIDEO_OUTPUT)
		return -EINVAL;

	if (dev->format.fmt.pix.pixelformat != format->fmt.pix.pixelformat)
		return -EINVAL;

	if (dev->format.fmt.pix.width != format->fmt.pix.width)
		return -EINVAL;

	if (dev->format.fmt.pix.height != format->fmt.pix.height)
		return -EINVAL;

	*format = dev->format;
	format->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	v4l2_loop_print_format(format);

	return 0;
}

static int v4l2_loop_g_fmt_cap(struct file *file, void *priv, struct v4l2_format *format)
{
	struct video_device *vdev = video_devdata(file);
	struct v4l2_loop_device *dev =
		container_of(vdev, struct v4l2_loop_device, vdev);

	v4l2_loop_dbg_at3("%s(%s)\n", __func__, video_device_node_name(vdev));

	if (!dev->format.type) { /* format is not yet set by the producer */
		v4l2_loop_dbg_at2("%s(%s) format is not yet set by the producer\n",
			__func__, video_device_node_name(vdev));
		return -EINVAL;
	}

	if (dev->format.type != V4L2_BUF_TYPE_VIDEO_OUTPUT)
		return -EINVAL;

	*format = dev->format;
	format->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	v4l2_loop_print_format(format);

	return 0;
}

static int v4l2_loop_try_fmt_cap(struct file *file, void *priv, struct v4l2_format *format)
{
	struct video_device *vdev = video_devdata(file);
	struct v4l2_loop_device *dev =
		container_of(vdev, struct v4l2_loop_device, vdev);

	v4l2_loop_dbg_at3("%s(%s)\n", __func__, video_device_node_name(vdev));

	if (!dev->format.type) { /* format is not yet set by the producer */
		v4l2_loop_dbg_at2("%s(%s) format is not yet set by the producer\n",
			__func__, video_device_node_name(vdev));
		return -EINVAL;
	}

	if (dev->format.type != V4L2_BUF_TYPE_VIDEO_OUTPUT)
		return -EINVAL;

	if (format->fmt.pix.pixelformat &&
		(dev->format.fmt.pix.pixelformat != format->fmt.pix.pixelformat))
		return -EINVAL;

	if (format->fmt.pix.width &&
		(dev->format.fmt.pix.width != format->fmt.pix.width))
		return -EINVAL;

	if (format->fmt.pix.height &&
		(dev->format.fmt.pix.height != format->fmt.pix.height))
		return -EINVAL;

	*format = dev->format;
	format->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	v4l2_loop_print_format(format);

	return 0;
}

static int v4l2_loop_s_fmt_cap_mplane(struct file *file, void *priv, struct v4l2_format *format)
{
	struct video_device *vdev = video_devdata(file);
	struct v4l2_loop_device *dev =
		container_of(vdev, struct v4l2_loop_device, vdev);

	v4l2_loop_dbg_at3("%s(%s)\n", __func__, video_device_node_name(vdev));

	if (!dev->format.type) { /* format is not yet set by the producer */
		v4l2_loop_dbg_at2("%s(%s) format is not yet set by the producer\n",
			__func__, video_device_node_name(vdev));
		return -EINVAL;
	}

	if (dev->format.type != V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE)
		return -EINVAL;

	if (dev->format.fmt.pix_mp.pixelformat != format->fmt.pix_mp.pixelformat)
		return -EINVAL;

	if (dev->format.fmt.pix_mp.width != format->fmt.pix_mp.width)
		return -EINVAL;

	if (dev->format.fmt.pix_mp.height != format->fmt.pix_mp.height)
		return -EINVAL;

	*format = dev->format;
	format->type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;

	v4l2_loop_print_format(format);

	return 0;
}

static int v4l2_loop_g_fmt_cap_mplane(struct file *file, void *priv, struct v4l2_format *format)
{
	struct video_device *vdev = video_devdata(file);
	struct v4l2_loop_device *dev =
		container_of(vdev, struct v4l2_loop_device, vdev);

	v4l2_loop_dbg_at3("%s(%s)\n", __func__, video_device_node_name(vdev));

	if (!dev->format.type) { /* format is not yet set by the producer */
		v4l2_loop_dbg_at2("%s(%s) format is not yet set by the producer\n",
			__func__, video_device_node_name(vdev));
		return -EINVAL;
	}

	if (dev->format.type != V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE)
		return -EINVAL;

	*format = dev->format;
	format->type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;

	v4l2_loop_print_format(format);

	return 0;
}

static int v4l2_loop_try_fmt_cap_mplane(struct file *file, void *priv, struct v4l2_format *format)
{
	struct video_device *vdev = video_devdata(file);
	struct v4l2_loop_device *dev =
		container_of(vdev, struct v4l2_loop_device, vdev);

	v4l2_loop_dbg_at3("%s(%s)\n", __func__, video_device_node_name(vdev));

	if (!dev->format.type) { /* format is not yet set by the producer */
		v4l2_loop_dbg_at2("%s(%s) format is not yet set by the producer\n",
			__func__, video_device_node_name(vdev));
		return -EINVAL;
	}

	if (dev->format.type != V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE)
		return -EINVAL;

	if (format->fmt.pix.pixelformat &&
		(dev->format.fmt.pix.pixelformat != format->fmt.pix.pixelformat))
		return -EINVAL;

	if (format->fmt.pix.width &&
		(dev->format.fmt.pix.width != format->fmt.pix.width))
		return -EINVAL;

	if (format->fmt.pix.height &&
		(dev->format.fmt.pix.height != format->fmt.pix.height))
		return -EINVAL;

	*format = dev->format;
	format->type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;

	v4l2_loop_print_format(format);

	return 0;
}

static int v4l2_loop_enum_fmt_out(struct file *file, void *fh, struct v4l2_fmtdesc *fmtdesc)
{
	struct video_device *vdev = video_devdata(file);
	const struct v4l2_loop_fmtdesc *f = NULL;

	v4l2_loop_dbg_at3("%s(%s)\n", __func__, video_device_node_name(vdev));

	if (fmtdesc->type == V4L2_BUF_TYPE_VIDEO_OUTPUT) {
		v4l2_loop_dbg_at3("%s(%s) enumerating single planar formats\n",
			__func__, video_device_node_name(vdev));
		if (fmtdesc->index < ARRAY_SIZE(v4l2_loop_fmtdescs_splanes))
			f = &v4l2_loop_fmtdescs_splanes[fmtdesc->index];
	} else
	if (fmtdesc->type == V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE) {
		v4l2_loop_dbg_at3("%s(%s) enumerating multi planar formats\n",
			__func__, video_device_node_name(vdev));
		if (fmtdesc->index < ARRAY_SIZE(v4l2_loop_fmtdescs_mplanes))
			f = &v4l2_loop_fmtdescs_mplanes[fmtdesc->index];
	} else {
		/* do nothing */
	}

	if (f == NULL)
		return -EINVAL;

	fmtdesc->flags = f->is_compressed ? V4L2_FMT_FLAG_COMPRESSED : 0;
	strscpy(fmtdesc->description, f->name, sizeof(fmtdesc->description));
	fmtdesc->pixelformat = f->pixelformat;

	return 0;
}

static int v4l2_loop_s_fmt_out(struct file *file, void *priv, struct v4l2_format *format)
{
	struct video_device *vdev = video_devdata(file);
	struct v4l2_loop_device *dev =
		container_of(vdev, struct v4l2_loop_device, vdev);
	const struct v4l2_loop_fmtdesc *f = NULL;
	size_t i;

	v4l2_loop_dbg_at3("%s(%s)\n", __func__, video_device_node_name(vdev));

	for (i = 0; i < ARRAY_SIZE(v4l2_loop_fmtdescs_splanes); ++i) {
		f = &v4l2_loop_fmtdescs_splanes[i];
		if (f->pixelformat == format->fmt.pix.pixelformat)
			break;
	}

	if (i == ARRAY_SIZE(v4l2_loop_fmtdescs_splanes))
		return -EINVAL;

	if (f == NULL)
		return -EINVAL;

	if (f->planes != 1)
		return -EINVAL;

	if (format->fmt.pix.field == V4L2_FIELD_ANY)
		format->fmt.pix.field = V4L2_FIELD_NONE;

	if (format->fmt.pix.colorspace == V4L2_COLORSPACE_DEFAULT)
		format->fmt.pix.colorspace = V4L2_COLORSPACE_SRGB;

	if (format->fmt.pix.bytesperline != 0)
		format->fmt.pix.sizeimage =
			format->fmt.pix.bytesperline * format->fmt.pix.height;
	else
		format->fmt.pix.sizeimage =
			(format->fmt.pix.width * format->fmt.pix.height *
			f->depth[0].numerator) / f->depth[0].denominator;

	dev->format = *format;
	v4l2_loop_print_format(format);

	return 0;
}

static int v4l2_loop_g_fmt_out(struct file *file, void *priv, struct v4l2_format *format)
{
	struct video_device *vdev = video_devdata(file);
	struct v4l2_loop_device *dev =
		container_of(vdev, struct v4l2_loop_device, vdev);

	v4l2_loop_dbg_at3("%s(%s)\n", __func__, video_device_node_name(vdev));

	if (!dev->format.type) {/* format is not yet set by the producer */
		v4l2_loop_dbg_at2("%s(%s) format is not yet set by the producer\n",
			__func__, video_device_node_name(vdev));
		return -EINVAL;
	}

	*format = dev->format;
	v4l2_loop_print_format(format);

	return 0;
}

static int v4l2_loop_try_fmt_out(struct file *file, void *priv, struct v4l2_format *format)
{
	struct video_device *vdev = video_devdata(file);
	const struct v4l2_loop_fmtdesc *f = NULL;
	size_t i;

	v4l2_loop_dbg_at3("%s(%s)\n", __func__, video_device_node_name(vdev));


	for (i = 0; i < ARRAY_SIZE(v4l2_loop_fmtdescs_splanes); ++i) {
		f = &v4l2_loop_fmtdescs_splanes[i];
		if (f->pixelformat == format->fmt.pix.pixelformat)
			break;
	}

	if (i == ARRAY_SIZE(v4l2_loop_fmtdescs_splanes))
		return -EINVAL;

	if (f == NULL)
		return -EINVAL;

	if (f->planes != 1)
		return -EINVAL;

	if (format->fmt.pix.field == V4L2_FIELD_ANY)
		format->fmt.pix.field = V4L2_FIELD_NONE;

	if (format->fmt.pix.colorspace == V4L2_COLORSPACE_DEFAULT)
		format->fmt.pix.colorspace = V4L2_COLORSPACE_SRGB;

	if (format->fmt.pix.bytesperline != 0)
		format->fmt.pix.sizeimage =
			format->fmt.pix.bytesperline * format->fmt.pix.height;
	else
		format->fmt.pix.sizeimage =
			(format->fmt.pix.width * format->fmt.pix.height *
			f->depth[0].numerator) / f->depth[0].denominator;

	v4l2_loop_print_format(format);

	return 0;
}

static int v4l2_loop_s_fmt_out_mplane(struct file *file, void *priv, struct v4l2_format *format)
{
	struct video_device *vdev = video_devdata(file);
	struct v4l2_loop_device *dev =
		container_of(vdev, struct v4l2_loop_device, vdev);
	const struct v4l2_loop_fmtdesc *f = NULL;
	size_t i;
	int plane;

	v4l2_loop_dbg_at3("%s(%s)\n", __func__, video_device_node_name(vdev));

	for (i = 0; i < ARRAY_SIZE(v4l2_loop_fmtdescs_mplanes); ++i) {
		f = &v4l2_loop_fmtdescs_mplanes[i];
		if (f->pixelformat == format->fmt.pix.pixelformat)
			break;
	}

	if (i == ARRAY_SIZE(v4l2_loop_fmtdescs_mplanes))
		return -EINVAL;

	if (f == NULL)
		return -EINVAL;

	if ((f->planes >= V4L2_LOOP_MAX_PLANES) || (f->planes >= VIDEO_MAX_PLANES))
		return -EINVAL;

	if (format->fmt.pix_mp.field == V4L2_FIELD_ANY)
		format->fmt.pix_mp.field = V4L2_FIELD_NONE;

	if (format->fmt.pix_mp.colorspace == V4L2_COLORSPACE_DEFAULT)
		format->fmt.pix_mp.colorspace = V4L2_COLORSPACE_SRGB;

	format->fmt.pix_mp.num_planes = f->planes;
	for (plane = 0; plane < f->planes; ++plane) {
		if (format->fmt.pix_mp.plane_fmt[plane].bytesperline != 0) {
			format->fmt.pix_mp.plane_fmt[plane].sizeimage =
				format->fmt.pix_mp.plane_fmt[plane].bytesperline * format->fmt.pix_mp.height;
		} else {
			format->fmt.pix_mp.plane_fmt[plane].sizeimage =
				(format->fmt.pix_mp.width * format->fmt.pix_mp.height *
				f->depth[plane].numerator) / f->depth[plane].denominator;
		}
	}

	dev->format = *format;
	v4l2_loop_print_format(format);

	return 0;
}

static int v4l2_loop_g_fmt_out_mplane(struct file *file, void *priv, struct v4l2_format *format)
{
	struct video_device *vdev = video_devdata(file);
	struct v4l2_loop_device *dev =
		container_of(vdev, struct v4l2_loop_device, vdev);

	v4l2_loop_dbg_at3("%s(%s)\n", __func__, video_device_node_name(vdev));

	if (!dev->format.type) {/* format is not yet set by the producer */
		v4l2_loop_dbg_at2("%s(%s) format is not yet set by the producer\n",
			__func__, video_device_node_name(vdev));
		return -EINVAL;
	}

	*format = dev->format;
	v4l2_loop_print_format(format);

	return 0;
}

static int v4l2_loop_try_fmt_out_mplane(struct file *file, void *priv, struct v4l2_format *format)
{
	struct video_device *vdev = video_devdata(file);
	const struct v4l2_loop_fmtdesc *f = NULL;
	size_t i;
	int plane;

	v4l2_loop_dbg_at3("%s(%s)\n", __func__, video_device_node_name(vdev));

	for (i = 0; i < ARRAY_SIZE(v4l2_loop_fmtdescs_mplanes); ++i) {
		f = &v4l2_loop_fmtdescs_mplanes[i];
		if (f->pixelformat == format->fmt.pix_mp.pixelformat)
			break;
	}

	if (i == ARRAY_SIZE(v4l2_loop_fmtdescs_mplanes))
		return -EINVAL;

	if (f == NULL)
		return -EINVAL;

	if ((f->planes >= V4L2_LOOP_MAX_PLANES) || (f->planes >= VIDEO_MAX_PLANES))
		return -EINVAL;

	if (format->fmt.pix_mp.field == V4L2_FIELD_ANY)
		format->fmt.pix_mp.field = V4L2_FIELD_NONE;

	if (format->fmt.pix_mp.colorspace == V4L2_COLORSPACE_DEFAULT)
		format->fmt.pix_mp.colorspace = V4L2_COLORSPACE_SRGB;

	format->fmt.pix_mp.num_planes = f->planes;
	for (plane = 0; plane < f->planes; ++plane) {
		if (format->fmt.pix_mp.plane_fmt[plane].bytesperline != 0) {
			format->fmt.pix_mp.plane_fmt[plane].sizeimage =
				format->fmt.pix_mp.plane_fmt[plane].bytesperline * format->fmt.pix_mp.height;
		} else {
			format->fmt.pix_mp.plane_fmt[plane].sizeimage =
				(format->fmt.pix_mp.width * format->fmt.pix_mp.height *
				f->depth[plane].numerator) / f->depth[plane].denominator;
		}
	}

	v4l2_loop_print_format(format);

	return 0;
}

static int v4l2_loop_g_parm(struct file *file, void *priv, struct v4l2_streamparm *parm)
{
	struct video_device *vdev = video_devdata(file);
	struct v4l2_loop_device *dev =
		container_of(vdev, struct v4l2_loop_device, vdev);

	v4l2_loop_dbg_at3("%s(%s)\n", __func__, video_device_node_name(vdev));

	if (V4L2_LOOP_IS_PRODUCER(parm->type))
		parm->parm.output = dev->outputparm;
	else
	if (V4L2_LOOP_IS_CONSUMER(parm->type))
		parm->parm.capture = dev->captureparm;
	else
		return -EINVAL;

	return 0;
}

static int v4l2_loop_s_parm(struct file *file, void *priv, struct v4l2_streamparm *parm)
{
	struct video_device *vdev = video_devdata(file);
	struct v4l2_loop_device *dev =
		container_of(vdev, struct v4l2_loop_device, vdev);

	v4l2_loop_dbg_at3("%s(%s)\n", __func__, video_device_node_name(vdev));

	if (V4L2_LOOP_IS_PRODUCER(parm->type))
		dev->outputparm = parm->parm.output;
	else
	if (V4L2_LOOP_IS_CONSUMER(parm->type))
		dev->captureparm = parm->parm.capture;
	else
		return -EINVAL;

	return 0;
}

static int v4l2_loop_reqbufs_producer(
	struct file *file,
	struct v4l2_fh *fh,
	struct v4l2_requestbuffers *requestbuffers)
{
	struct video_device *vdev = video_devdata(file);
	struct v4l2_loop_handle *h =
		container_of(fh, struct v4l2_loop_handle, fh);
	struct vb2_queue *vq = vdev->queue;
	int status;

	h->htype = V4L2_LOOP_HANDLE_PRODUCER;

	if (vq->owner && vq->owner != file->private_data) {
		v4l2_loop_dbg_at1("%s(%s) queue is busy\n",
			__func__, video_device_node_name(vdev));
		return -EBUSY;
	}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 16, 0)
	status = vb2_core_reqbufs(vdev->queue,
		requestbuffers->memory, requestbuffers->flags, &requestbuffers->count);
#else
	status = vb2_core_reqbufs(vdev->queue,
		requestbuffers->memory, &requestbuffers->count);
#endif
	if (status) {
		v4l2_loop_dbg_at1("%s(%s) vb2_core_reqbufs() failed\n",
			__func__, video_device_node_name(vdev));
		return status;
	}

	if (requestbuffers->count)
		vq->owner = file->private_data;
	else
		vq->owner = NULL;

	return 0;
}

static int v4l2_loop_reqbufs_consumer(
	struct file *file,
	struct v4l2_fh *fh,
	struct v4l2_requestbuffers *requestbuffers)
{
	struct video_device *vdev = video_devdata(file);
	struct v4l2_loop_device *dev =
		container_of(vdev, struct v4l2_loop_device, vdev);
	struct v4l2_loop_handle *h =
		container_of(fh, struct v4l2_loop_handle, fh);
	struct vb2_queue *vq = vdev->queue;
	int status;

	h->htype = V4L2_LOOP_HANDLE_CONSUMER;

	if (requestbuffers->memory != VB2_MEMORY_MMAP &&
		requestbuffers->memory != VB2_MEMORY_USERPTR &&
		requestbuffers->memory != VB2_MEMORY_DMABUF) {
		v4l2_loop_dbg_at1("%s(%s) unsupported memory type\n",
			__func__, video_device_node_name(vdev));
		return -EINVAL;
	}

	status = v4l2_loop_validate_buffer_types(dev, requestbuffers->type);
	if (status)
		return status;

	if (requestbuffers->count > 0)
		requestbuffers->count = vq->num_buffers;

	if (requestbuffers->count > 0) {
		__u32 i;
		__u32 plane;

		if (h->c.bufs)
			return 0;

		h->c.bufs = kzalloc(sizeof(*h->c.bufs) * requestbuffers->count, GFP_KERNEL);
		if (!h->c.bufs)
			return -ENOMEM;

		h->c.buffers = requestbuffers->count;

		for (i = 0; i < h->c.buffers; i++) {
			struct v4l2_loop_cbuf *cbuf = &h->c.bufs[i];
			struct vb2_buffer *vb = &cbuf->vbuf.vb2_buf;

			vb->vb2_queue = NULL;
			vb->index = i;
			vb->type = requestbuffers->type;
			vb->memory = requestbuffers->memory;
			vb->num_planes = vq->bufs[i]->num_planes;
			for (plane = 0; plane < vb->num_planes; plane++) {
				vb->planes[plane].length = vq->bufs[i]->planes[plane].length;
				vb->planes[plane].min_length = vq->bufs[i]->planes[plane].min_length;
			}
			vb->state = VB2_BUF_STATE_DEQUEUED;
		}
	} else
		v4l2_loop_release_cbufs(&h->c);

	requestbuffers->capabilities =
		V4L2_BUF_CAP_SUPPORTS_ORPHANED_BUFS |
		V4L2_BUF_CAP_SUPPORTS_MMAP |
		V4L2_BUF_CAP_SUPPORTS_USERPTR |
		V4L2_BUF_CAP_SUPPORTS_DMABUF;

	return 0;
}

static int v4l2_loop_reqbufs(struct file *file, void *fh, struct v4l2_requestbuffers *requestbuffers)
{
	struct video_device *vdev = video_devdata(file);
	int status;

	v4l2_loop_dbg_at3("%s(%s, %s, %s, %d)\n", __func__,
		video_device_node_name(vdev),
		v4l2_loop_buf_type_to_string(requestbuffers->type),
		v4l2_loop_memory_to_string(requestbuffers->memory),
		requestbuffers->count);

	v4l2_loop_print_requestbuffers(requestbuffers);

	if (V4L2_LOOP_IS_PRODUCER(requestbuffers->type))
		status = v4l2_loop_reqbufs_producer(file, fh, requestbuffers);
	else
	if (V4L2_LOOP_IS_CONSUMER(requestbuffers->type))
		status = v4l2_loop_reqbufs_consumer(file, fh, requestbuffers);
	else
		return -EINVAL;

	if (status)
		return status;

	return 0;
}

static int v4l2_loop_querybuf_producer(struct file *file, void *fh, struct v4l2_buffer *buffer)
{
	struct video_device *vdev = video_devdata(file);
	struct vb2_queue *vq = vdev->queue;
	int status;

	status = vb2_querybuf(vq, buffer);
	if (status) {
		v4l2_loop_dbg_at1("%s(%s) vb2_querybuf() failed\n",
			__func__, video_device_node_name(vdev));
		return status;
	}

	return 0;
}

static int v4l2_loop_querybuf_consumer(struct file *file, void *fh, struct v4l2_buffer *buffer)
{
	struct video_device *vdev = video_devdata(file);
	struct v4l2_loop_device *dev =
		container_of(vdev, struct v4l2_loop_device, vdev);
	struct v4l2_loop_handle *h =
		container_of(fh, struct v4l2_loop_handle, fh);
	struct v4l2_loop_pbuf *pbuf;
	struct v4l2_loop_cbuf *cbuf;
	struct vb2_queue *vq = vdev->queue;
	int status;

	status = v4l2_loop_validate_buffer_types(dev, buffer->type);
	if (status)
		return status;

	if (!h->c.bufs)
		return -EINVAL;

	if (buffer->index >= vq->num_buffers) {
		v4l2_loop_dbg_at1("%s(%s) buffer index #%u is bigger than number of allocated buffers (%u)\n",
			__func__, video_device_node_name(vdev), buffer->index, vq->num_buffers);
		return -EINVAL;
	}

	pbuf = v4l2_loop_pbuf(vdev->queue->bufs[buffer->index]);
	cbuf = &h->c.bufs[buffer->index];

	status = v4l2_loop_validate_planes(&cbuf->vbuf.vb2_buf, buffer);
	if (status)
		return -EINVAL;

	status = v4l2_loop_fill_user_buffer(pbuf, cbuf, buffer);
	if (status)
		return status;

	buffer->bytesused = 0; /* reset bytesused to 0 when querying */

	return 0;
}

static int v4l2_loop_querybuf(struct file *file, void *fh, struct v4l2_buffer *buffer)
{
	struct video_device *vdev = video_devdata(file);
	int status;

	v4l2_loop_dbg_at3("%s(%s, %s, %d)\n", __func__,
		video_device_node_name(vdev),
		v4l2_loop_buf_type_to_string(buffer->type),
		buffer->index);

	if (V4L2_LOOP_IS_PRODUCER(buffer->type))
		status = v4l2_loop_querybuf_producer(file, fh, buffer);
	else
	if (V4L2_LOOP_IS_CONSUMER(buffer->type))
		status = v4l2_loop_querybuf_consumer(file, fh, buffer);
	else
		return -EINVAL;

	if (status)
		return status;

	v4l2_loop_print_buffer(buffer);

	return 0;
}

static int v4l2_loop_qbuf_producer(struct file *file, void *fh, struct v4l2_buffer *buffer)
{
	struct video_device *vdev = video_devdata(file);
	struct vb2_queue *vq = vdev->queue;
	int status;

	/* The queue is busy if there is an owner and you are not that owner */
	if (vq->owner && vq->owner != file->private_data)
		return EBUSY;

	status = vb2_qbuf(vq, vdev->v4l2_dev->mdev, buffer);
	if (status) {
		v4l2_loop_dbg_at1("%s(%s) vb2_qbuf() failed\n",
			__func__, video_device_node_name(vdev));
		return status;
	}

	return 0;
}

static int v4l2_loop_qbuf_consumer(struct file *file, void *fh, struct v4l2_buffer *buffer)
{
	struct video_device *vdev = video_devdata(file);
	struct v4l2_loop_device *dev =
		container_of(vdev, struct v4l2_loop_device, vdev);
	struct v4l2_loop_handle *h =
		container_of(fh, struct v4l2_loop_handle, fh);
	struct v4l2_loop_pbuf *pbuf;
	struct v4l2_loop_cbuf *cbuf;
	struct vb2_queue *vq = vdev->queue;
	int status;

	status = v4l2_loop_validate_buffer_types(dev, buffer->type);
	if (status)
		return status;

	if (!h->c.bufs)
		return -EINVAL;

	if (buffer->index >= vq->num_buffers) {
		v4l2_loop_dbg_at1("%s(%s) buffer index #%u is bigger than number of allocated buffers (%u)\n",
			__func__, video_device_node_name(vdev), buffer->index, vq->num_buffers);
		return -EINVAL;
	}

	cbuf = &h->c.bufs[buffer->index];
	pbuf = cbuf->pbuf;

	if (pbuf) {
		vb2_buffer_done(&pbuf->vbuf.vb2_buf, VB2_BUF_STATE_DONE);
		cbuf->pbuf = NULL;
	}

	if (cbuf->vbuf.vb2_buf.state == VB2_BUF_STATE_QUEUED)
		return -EINVAL;

	v4l2_loop_fill_vb2_buffer(buffer, &cbuf->vbuf.vb2_buf);

	cbuf->vbuf.vb2_buf.state = VB2_BUF_STATE_QUEUED;
	list_add_tail(&cbuf->cnode, &h->c.queued_bufs);

	return 0;
}

static int v4l2_loop_qbuf(struct file *file, void *fh, struct v4l2_buffer *buffer)
{
	struct video_device *vdev = video_devdata(file);
	int status;

	v4l2_loop_dbg_at3("%s(%s, %s, %u)\n", __func__,
		video_device_node_name(vdev),
		v4l2_loop_buf_type_to_string(buffer->type),
		buffer->index);

	v4l2_loop_print_buffer(buffer);

	if (V4L2_LOOP_IS_PRODUCER(buffer->type))
		status = v4l2_loop_qbuf_producer(file, fh, buffer);
	else
	if (V4L2_LOOP_IS_CONSUMER(buffer->type))
		status = v4l2_loop_qbuf_consumer(file, fh, buffer);
	else
		return -EINVAL;

	if (status)
		return status;

	return 0;
}

static int v4l2_loop_dqbuf_producer(struct file *file, void *fh, struct v4l2_buffer *buffer)
{
	struct video_device *vdev = video_devdata(file);
	struct vb2_queue *vq = vdev->queue;
	int status;

	/* The queue is busy if there is an owner and you are not that owner */
	if (vq->owner && vq->owner != file->private_data)
		return EBUSY;

	status = vb2_dqbuf(vq, buffer, file->f_flags & O_NONBLOCK);
	if (status) {
		v4l2_loop_dbg_at1("%s(%s) vb2_dqbuf() failed\n",
			__func__, video_device_node_name(vdev));
		return status;
	}

	return 0;
}

static int v4l2_loop_dqbuf_consumer(struct file *file, void *fh, struct v4l2_buffer *buffer)
{
	struct video_device *vdev = video_devdata(file);
	struct v4l2_loop_device *dev =
		container_of(vdev, struct v4l2_loop_device, vdev);
	struct v4l2_loop_handle *h =
		container_of(fh, struct v4l2_loop_handle, fh);
	struct v4l2_loop_pbuf *pbuf;
	struct v4l2_loop_cbuf *cbuf;
	struct vb2_queue *vq = vdev->queue;
	int status;
	unsigned long flags;

	if (!h->c.bufs)
		return -EINVAL;

	cbuf = NULL;
	if (!list_empty(&h->c.queued_bufs))
		cbuf = list_first_entry(&h->c.queued_bufs, struct v4l2_loop_cbuf, cnode);

	if (!cbuf)
		return -EINVAL;

	// TODO: Add support for file->f_flags & O_NONBLOCK
	status = wait_event_interruptible(dev->waiting_consumers,
		v4l2_loop_buffer_is_available(dev) ||
			!vq->streaming || vq->error);
	if (status)
		return status;

	if (!vq->streaming) {
		v4l2_loop_dbg_at1("%s(%s) streaming off\n",
			__func__, video_device_node_name(vdev));
		return -EINVAL;
	}

	if (vq->error) {
		v4l2_loop_dbg_at1("%s(%s) queue in error state\n",
			__func__, video_device_node_name(vdev));
		return -EIO;
	}

	spin_lock_irqsave(&dev->queued_bufs_lock, flags);
	pbuf = list_first_entry(&dev->queued_bufs, struct v4l2_loop_pbuf, pnode);
	status = v4l2_loop_validate_planes(&pbuf->vbuf.vb2_buf, buffer);
	if (!status)
		list_del(&pbuf->pnode);
	spin_unlock_irqrestore(&dev->queued_bufs_lock, flags);

	if (status)
		return status;

	status = v4l2_loop_fill_user_buffer(pbuf, cbuf, buffer);
	if (status) {
		vb2_buffer_done(&pbuf->vbuf.vb2_buf, VB2_BUF_STATE_ERROR);
		return status;
	}

	list_del(&cbuf->cnode);
	cbuf->pbuf = pbuf;
	cbuf->vbuf.vb2_buf.state = VB2_BUF_STATE_DEQUEUED;

	return 0;
}

static int v4l2_loop_dqbuf(struct file *file, void *fh, struct v4l2_buffer *buffer)
{
	struct video_device *vdev = video_devdata(file);
	int status;

	v4l2_loop_dbg_at3("%s(%s, %s)\n", __func__,
		video_device_node_name(vdev),
		v4l2_loop_buf_type_to_string(buffer->type));

	if (V4L2_LOOP_IS_PRODUCER(buffer->type))
		status = v4l2_loop_dqbuf_producer(file, fh, buffer);
	else
	if (V4L2_LOOP_IS_CONSUMER(buffer->type))
		status = v4l2_loop_dqbuf_consumer(file, fh, buffer);
	else
		return -EINVAL;

	if (status)
		return status;

	v4l2_loop_print_buffer(buffer);

	return 0;
}

static int v4l2_loop_streamon(struct file *file, void *fh, enum v4l2_buf_type type)
{
	struct video_device *vdev = video_devdata(file);
	struct vb2_queue *vq = vdev->queue;
	int status;

	v4l2_loop_dbg_at3("%s(%s, %s)\n", __func__,
		video_device_node_name(vdev), v4l2_loop_handle_name(fh));

	if (V4L2_LOOP_IS_PRODUCER(type)) {
		/* The queue is busy if there is a owner and you are not that owner */
		if (vq->owner && vq->owner != file->private_data)
			return -EBUSY;

		status = vb2_streamon(vq, type);
		if (status) {
			v4l2_loop_dbg_at1("%s(%s) vb2_streamon() failed\n",
				__func__, video_device_node_name(vdev));
			return status;
		}
	} else
	if (V4L2_LOOP_IS_CONSUMER(type))
		; /* do nothing */
	else
		return -EINVAL;

	return 0;
}

static int v4l2_loop_streamoff(struct file *file, void *fh, enum v4l2_buf_type type)
{
	struct video_device *vdev = video_devdata(file);
	struct vb2_queue *vq = vdev->queue;
	int status;

	v4l2_loop_dbg_at3("%s(%s, %s)\n", __func__,
		video_device_node_name(vdev), v4l2_loop_handle_name(fh));

	if (V4L2_LOOP_IS_PRODUCER(type)) {
		/* The queue is busy if there is a owner and you are not that owner */
		if (vq->owner && vq->owner != file->private_data)
			return -EBUSY;

		status = vb2_streamoff(vq, type);
		if (status) {
			v4l2_loop_dbg_at1("%s(%s) vb2_streamoff() failed\n",
				__func__, video_device_node_name(vdev));
			return status;
		}
	} else
	if (V4L2_LOOP_IS_CONSUMER(type))
		; /* do nothing */
	else
		return -EINVAL;

	return 0;
}

static int v4l2_loop_subscribe_event(struct v4l2_fh *fh, const struct v4l2_event_subscription *sub)
{
	struct video_device *vdev = fh->vdev;

	v4l2_loop_dbg_at3("%s(%s)\n", __func__, video_device_node_name(vdev));

	return 0;
}

static int v4l2_loop_unsubscribe_event(struct v4l2_fh *fh, const struct v4l2_event_subscription *sub)
{
	struct video_device *vdev = fh->vdev;

	v4l2_loop_dbg_at3("%s(%s)\n", __func__, video_device_node_name(vdev));

	return 0;
}

static const struct v4l2_ioctl_ops v4l2_loop_ioctl_ops = {
	.vidioc_querycap		= v4l2_loop_querycap,
	.vidioc_enum_framesizes		= v4l2_loop_enum_framesizes,
	.vidioc_enum_frameintervals	= v4l2_loop_enum_frameintervals,

	.vidioc_enum_output		= v4l2_loop_enum_output,
	.vidioc_g_output		= v4l2_loop_g_output,
	.vidioc_s_output		= v4l2_loop_s_output,

	.vidioc_enum_input		= v4l2_loop_enum_input,
	.vidioc_g_input			= v4l2_loop_g_input,
	.vidioc_s_input			= v4l2_loop_s_input,

	.vidioc_enum_fmt_vid_cap	= v4l2_loop_enum_fmt_cap,
	.vidioc_s_fmt_vid_cap		= v4l2_loop_s_fmt_cap,
	.vidioc_g_fmt_vid_cap		= v4l2_loop_g_fmt_cap,
	.vidioc_try_fmt_vid_cap		= v4l2_loop_try_fmt_cap,
	.vidioc_s_fmt_vid_cap_mplane	= v4l2_loop_s_fmt_cap_mplane,
	.vidioc_g_fmt_vid_cap_mplane	= v4l2_loop_g_fmt_cap_mplane,
	.vidioc_try_fmt_vid_cap_mplane	= v4l2_loop_try_fmt_cap_mplane,

	.vidioc_enum_fmt_vid_out	= v4l2_loop_enum_fmt_out,
	.vidioc_s_fmt_vid_out		= v4l2_loop_s_fmt_out,
	.vidioc_g_fmt_vid_out		= v4l2_loop_g_fmt_out,
	.vidioc_try_fmt_vid_out		= v4l2_loop_try_fmt_out,
	.vidioc_s_fmt_vid_out_mplane	= v4l2_loop_s_fmt_out_mplane,
	.vidioc_g_fmt_vid_out_mplane	= v4l2_loop_g_fmt_out_mplane,
	.vidioc_try_fmt_vid_out_mplane	= v4l2_loop_try_fmt_out_mplane,

	.vidioc_g_parm			= v4l2_loop_g_parm,
	.vidioc_s_parm			= v4l2_loop_s_parm,

	.vidioc_reqbufs			= v4l2_loop_reqbufs,
	.vidioc_querybuf		= v4l2_loop_querybuf,
	.vidioc_qbuf			= v4l2_loop_qbuf,
	.vidioc_dqbuf			= v4l2_loop_dqbuf,

	.vidioc_streamon		= v4l2_loop_streamon,
	.vidioc_streamoff		= v4l2_loop_streamoff,

	.vidioc_subscribe_event		= v4l2_loop_subscribe_event,
	.vidioc_unsubscribe_event	= v4l2_loop_unsubscribe_event
};

static struct v4l2_loop_device* v4l2_loop_alloc_device(int i)
{
	int status;
	struct v4l2_loop_device *dev;

	dev = kzalloc(sizeof(*dev), GFP_KERNEL);
	if (!dev)
		return ERR_PTR(-ENOMEM);

	snprintf(dev->v4l2_dev.name, sizeof(dev->v4l2_dev.name),
		"v4l2-loop-device-%d", i);

	status = v4l2_device_register(NULL, &dev->v4l2_dev);
	if (status) {
		pr_err("v4l2_device_register() failed\n");
		goto out_free_dev;
	}

	/* Init videobuf2 queue structure */
	mutex_init(&dev->vb_queue_lock);
	dev->vb_queue.lock = &dev->vb_queue_lock;
	dev->vb_queue.type = v4l2_loop_mplane ?
		V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE : V4L2_BUF_TYPE_VIDEO_OUTPUT;
	dev->vb_queue.io_modes = VB2_MMAP | VB2_USERPTR | VB2_DMABUF | VB2_WRITE;
	dev->vb_queue.drv_priv = dev;
	dev->vb_queue.buf_struct_size = sizeof(struct v4l2_loop_pbuf);
	dev->vb_queue.ops = &v4l2_loop_vb2_ops;
	dev->vb_queue.mem_ops = &vb2_vmalloc_memops;
	dev->vb_queue.timestamp_flags = V4L2_BUF_FLAG_TIMESTAMP_MONOTONIC;
	dev->vb_queue.min_buffers_needed = v4l2_loop_buffers;
	status = vb2_queue_init(&dev->vb_queue);
	if (status) {
		pr_err("vb2_queue_init() failed\n");
		goto out_free_dev;
	}

	spin_lock_init(&dev->queued_bufs_lock);
	INIT_LIST_HEAD(&dev->queued_bufs);

	init_waitqueue_head(&dev->waiting_consumers);
	dev->sequence = 0;

	dev->vdev.queue = &dev->vb_queue;
	dev->vdev.fops = &v4l2_loop_fops;
	dev->vdev.ioctl_ops = &v4l2_loop_ioctl_ops;
	dev->vdev.vfl_dir = VFL_DIR_M2M;
	dev->vdev.device_caps =
		V4L2_CAP_DEVICE_CAPS |
		V4L2_CAP_STREAMING |
		V4L2_CAP_READWRITE |
		V4L2_CAP_VIDEO_CAPTURE |
		V4L2_CAP_VIDEO_OUTPUT |
		V4L2_CAP_VIDEO_M2M;
	if (v4l2_loop_mplane) {
		dev->vdev.device_caps |=
			V4L2_CAP_VIDEO_CAPTURE_MPLANE |
			V4L2_CAP_VIDEO_OUTPUT_MPLANE |
			V4L2_CAP_VIDEO_M2M_MPLANE;
	}
	dev->vdev.v4l2_dev = &dev->v4l2_dev;
	dev->vdev.release = video_device_release_empty;
	strscpy(dev->vdev.name, dev->v4l2_dev.name, sizeof(dev->vdev.name));

	status = video_register_device(&dev->vdev, VFL_TYPE_VIDEO, -1);
	if (status) {
		pr_err("video_register_device() failed\n");
		goto out_free_dev;
	}

	pr_info("registered new video device '%s'\n",
		video_device_node_name(&dev->vdev));

	return dev;

out_free_dev:
	kfree(dev);
	return ERR_PTR(status);
}

static void v4l2_loop_free_device(struct v4l2_loop_device *dev)
{
	video_unregister_device(&dev->vdev);
	v4l2_device_unregister(&dev->v4l2_dev);
	kfree(dev);
}

static int __init v4l2_loop_init(void)
{
	int i;

	for (i = 0; i < v4l2_loop_devices; ++i) {
		struct v4l2_loop_device *dev = v4l2_loop_alloc_device(i);
		if (IS_ERR(dev))
			break;
		list_add_tail(&dev->node, &v4l2_loop_devices_list);
	}

	if (i < v4l2_loop_devices) {
		struct list_head *p, *n;

		list_for_each_safe(p, n, &v4l2_loop_devices_list) {
			struct v4l2_loop_device *dev;
			dev = list_entry(p, struct v4l2_loop_device, node);
			v4l2_loop_free_device(dev);
		}
		return -EFAULT;
	}

	pr_info("module loaded (version: %s)\n", V4L2_LOOP_VERSION_STR);

	return 0;
}
module_init(v4l2_loop_init);

#ifdef MODULE
static void __exit v4l2_loop_exit(void)
{
	struct list_head *p, *n;

	list_for_each_safe(p, n, &v4l2_loop_devices_list) {
		struct v4l2_loop_device *dev;
		dev = list_entry(p, struct v4l2_loop_device, node);
		v4l2_loop_free_device(dev);
	}

	pr_info("module removed\n");
}
module_exit(v4l2_loop_exit);
#endif

MODULE_DESCRIPTION("v4l2 loop device with dma-buf and mplane support");
MODULE_AUTHOR("Lukasz Wiecaszek <lukasz.wiecaszek(at)gmail.com>");
MODULE_LICENSE("GPL v2");
MODULE_VERSION(V4L2_LOOP_VERSION_STR);
MODULE_IMPORT_NS(DMA_BUF);
