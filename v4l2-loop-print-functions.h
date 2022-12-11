/* SPDX-License-Identifier: GPL-2.0 */
/*
 * v4l2-loop-print-functions.h
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
#ifndef V4L2_LOOP_PRINT_FUNCTIONS
#define V4L2_LOOP_PRINT_FUNCTIONS

#include <linux/types.h>
#include <linux/videodev2.h>

static inline const char* v4l2_loop_buf_type_to_string(enum v4l2_buf_type buf_type)
{
	static const char* buf_types[] = {
		[0]                                  = "0",
		[V4L2_BUF_TYPE_VIDEO_CAPTURE]        = "V4L2_BUF_TYPE_VIDEO_CAPTURE",
		[V4L2_BUF_TYPE_VIDEO_OVERLAY]        = "V4L2_BUF_TYPE_VIDEO_OVERLAY",
		[V4L2_BUF_TYPE_VIDEO_OUTPUT]         = "V4L2_BUF_TYPE_VIDEO_OUTPUT",
		[V4L2_BUF_TYPE_VBI_CAPTURE]          = "V4L2_BUF_TYPE_VBI_CAPTURE",
		[V4L2_BUF_TYPE_VBI_OUTPUT]           = "V4L2_BUF_TYPE_VBI_OUTPUT",
		[V4L2_BUF_TYPE_SLICED_VBI_CAPTURE]   = "V4L2_BUF_TYPE_SLICED_VBI_CAPTURE",
		[V4L2_BUF_TYPE_SLICED_VBI_OUTPUT]    = "V4L2_BUF_TYPE_SLICED_VBI_OUTPUT",
		[V4L2_BUF_TYPE_VIDEO_OUTPUT_OVERLAY] = "V4L2_BUF_TYPE_VIDEO_OUTPUT_OVERLAY",
		[V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE] = "V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE",
		[V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE]  = "V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE",
		[V4L2_BUF_TYPE_SDR_CAPTURE]          = "V4L2_BUF_TYPE_SDR_CAPTURE",
		[V4L2_BUF_TYPE_SDR_OUTPUT]           = "V4L2_BUF_TYPE_SDR_OUTPUT",
	};

	if (buf_type >= ARRAY_SIZE(buf_types))
		buf_type = 0;

	return buf_types[buf_type];
}

static inline const char* v4l2_loop_memory_to_string(enum v4l2_memory memory)
{
	static const char* memories[] = {
		[0]                    = "0",
		[V4L2_MEMORY_MMAP]     = "V4L2_MEMORY_MMAP",
		[V4L2_MEMORY_USERPTR]  = "V4L2_MEMORY_USERPTR",
		[V4L2_MEMORY_OVERLAY]  = "V4L2_MEMORY_OVERLAY",
		[V4L2_MEMORY_DMABUF]   = "V4L2_MEMORY_DMABUF",
	};

	if (memory >= ARRAY_SIZE(memories))
		memory = 0;

	return memories[memory];
}

static inline const char* v4l2_loop_frmsizetype_to_string(enum v4l2_frmsizetypes type)
{
	static const char* types[] = {
		[0]                              = "0",
		[V4L2_FRMSIZE_TYPE_DISCRETE]     = "V4L2_FRMSIZE_TYPE_DISCRETE",
		[V4L2_FRMSIZE_TYPE_CONTINUOUS]   = "V4L2_FRMSIZE_TYPE_CONTINUOUS",
		[V4L2_FRMSIZE_TYPE_STEPWISE]     = "V4L2_FRMSIZE_TYPE_STEPWISE",
	};

	if (type >= ARRAY_SIZE(types))
		type = 0;

	return types[type];
}

static inline const char* v4l2_loop_frmivaltype_to_string(enum v4l2_frmivaltypes type)
{
	static const char* types[] = {
		[0]                              = "0",
		[V4L2_FRMIVAL_TYPE_DISCRETE]     = "V4L2_FRMIVAL_TYPE_DISCRETE",
		[V4L2_FRMIVAL_TYPE_CONTINUOUS]   = "V4L2_FRMIVAL_TYPE_CONTINUOUS",
		[V4L2_FRMIVAL_TYPE_STEPWISE]     = "V4L2_FRMIVAL_TYPE_STEPWISE",
	};

	if (type >= ARRAY_SIZE(types))
		type = 0;

	return types[type];
}

static inline void v4l2_loop_print_fmtdesc(const struct v4l2_fmtdesc* fmtdesc)
{
	v4l2_loop_dbg_at4(
		"v4l2_fmtdesc:\n"
		"\tindex       : %u\n"
		"\ttype        : %s\n"
		"\tflags       : 0x%08x\n"
		"\tdescription : %s\n"
		"\tpixelformat : '%c%c%c%c'\n",
		fmtdesc->index,
		v4l2_loop_buf_type_to_string(fmtdesc->type),
		fmtdesc->flags,
		fmtdesc->description,
		(fmtdesc->pixelformat >>  0) & 0xff,
		(fmtdesc->pixelformat >>  8) & 0xff,
		(fmtdesc->pixelformat >> 16) & 0xff,
		(fmtdesc->pixelformat >> 24) & 0xff
	);
}

static inline void v4l2_loop_print_frmsizeenum(const struct v4l2_frmsizeenum* frmsizeenum)
{
	char buf[512];
	size_t n = 0;
	size_t limit = sizeof(buf);
	int status;

	memset(buf, 0, sizeof(buf));

	do {
		status = snprintf(&buf[n], limit,
			"v4l2_frmsizeenum:\n"
			"\tindex       : %u\n"
			"\tpixelformat : '%c%c%c%c'\n"
			"\ttype        : %s\n",
			frmsizeenum->index,
			(frmsizeenum->pixel_format >>  0) & 0xff,
			(frmsizeenum->pixel_format >>  8) & 0xff,
			(frmsizeenum->pixel_format >> 16) & 0xff,
			(frmsizeenum->pixel_format >> 24) & 0xff,
			v4l2_loop_frmsizetype_to_string(frmsizeenum->type)
		);
		if ((status < 0) || (status >= limit))
			break;
		else
			n += status, limit -= status;

		if (frmsizeenum->type == V4L2_FRMSIZE_TYPE_DISCRETE) {
			status = snprintf(&buf[n], limit,
				"\tdiscrete    : width: %u, height: %u\n",
				frmsizeenum->discrete.width,
				frmsizeenum->discrete.height
			);
			if ((status < 0) || (status >= limit))
				break;
			else
				n += status, limit -= status;
		} else
		if (frmsizeenum->type == V4L2_FRMSIZE_TYPE_STEPWISE) {
			status = snprintf(&buf[n], limit,
				"\tstepwise    : width: [%u:%u:%u], height: [%u:%u:%u]\n",
				frmsizeenum->stepwise.min_width,
				frmsizeenum->stepwise.max_width,
				frmsizeenum->stepwise.step_width,
				frmsizeenum->stepwise.min_height,
				frmsizeenum->stepwise.max_height,
				frmsizeenum->stepwise.step_height
			);
			if ((status < 0) || (status >= limit))
				break;
			else
				n += status, limit -= status;
		} else {
			/* do nothing */
		}
	} while (0);

	v4l2_loop_dbg_at4("%s", buf);
}

static inline void v4l2_loop_print_frmivalenum(const struct v4l2_frmivalenum* frmivalenum)
{
	char buf[512];
	size_t n = 0;
	size_t limit = sizeof(buf);
	int status;

	memset(buf, 0, sizeof(buf));

	do {
		status = snprintf(&buf[n], limit,
			"v4l2_frmivalenum:\n"
			"\tindex       : %u\n"
			"\tpixelformat : '%c%c%c%c'\n"
			"\twidth       : %u\n"
			"\theight      : %u\n"
			"\ttype        : %s\n",
			frmivalenum->index,
			(frmivalenum->pixel_format >>  0) & 0xff,
			(frmivalenum->pixel_format >>  8) & 0xff,
			(frmivalenum->pixel_format >> 16) & 0xff,
			(frmivalenum->pixel_format >> 24) & 0xff,
			frmivalenum->width,
			frmivalenum->height,
			v4l2_loop_frmivaltype_to_string(frmivalenum->type)
		);
		if ((status < 0) || (status >= limit))
			break;
		else
			n += status, limit -= status;

		if (frmivalenum->type == V4L2_FRMIVAL_TYPE_DISCRETE) {
			status = snprintf(&buf[n], limit,
				"\tdiscrete    : %u/%u\n",
				frmivalenum->discrete.numerator,
				frmivalenum->discrete.denominator
			);
			if ((status < 0) || (status >= limit))
				break;
			else
				n += status, limit -= status;
		} else
		if (frmivalenum->type == V4L2_FRMSIZE_TYPE_STEPWISE) {
			status = snprintf(&buf[n], limit,
				"\tstepwise    : min: %u/%u, max: %u/%u, step: %u/%u\n",
				frmivalenum->stepwise.min.numerator,
				frmivalenum->stepwise.min.denominator,
				frmivalenum->stepwise.max.numerator,
				frmivalenum->stepwise.max.denominator,
				frmivalenum->stepwise.step.numerator,
				frmivalenum->stepwise.step.denominator
			);
			if ((status < 0) || (status >= limit))
				break;
			else
				n += status, limit -= status;
		} else {
			/* do nothing */
		}
	} while (0);

	v4l2_loop_dbg_at4("%s", buf);
}

static inline void v4l2_loop_print_format(const struct v4l2_format* format)
{
	char buf[512];
	size_t n = 0;
	size_t limit = sizeof(buf);
	int status;

	memset(buf, 0, sizeof(buf));

	do {
		status = snprintf(&buf[n], limit,
			"v4l2_format:\n"
			"\ttype        : %s\n"
			"\twidth       : %u\n"
			"\theight      : %u\n"
			"\tpixelformat : '%c%c%c%c'\n",
			v4l2_loop_buf_type_to_string(format->type),
			format->fmt.pix.width,
			format->fmt.pix.height,
			(format->fmt.pix.pixelformat >>  0) & 0xff,
			(format->fmt.pix.pixelformat >>  8) & 0xff,
			(format->fmt.pix.pixelformat >> 16) & 0xff,
			(format->fmt.pix.pixelformat >> 24) & 0xff
		);
		if ((status < 0) || (status >= limit))
			break;
		else
			n += status, limit -= status;

		if (V4L2_TYPE_IS_MULTIPLANAR(format->type)) {
			status = snprintf(&buf[n], limit,
				"\tfield       : %u\n"
				"\tcolorspace  : %u\n"
				"\tnum_planes  : %u\n",
				format->fmt.pix_mp.field,
				format->fmt.pix_mp.colorspace,
				format->fmt.pix_mp.num_planes
			);
			if ((status < 0) || (status >= limit))
				break;
			else
				n += status, limit -= status;
		} else {
			status = snprintf(&buf[n], limit,
				"\tfield       : %u\n"
				"\tbytesperline: %u\n"
				"\tsizeimage   : %u\n"
				"\tcolorspace  : %u\n",
				format->fmt.pix.field,
				format->fmt.pix.bytesperline,
				format->fmt.pix.sizeimage,
				format->fmt.pix.colorspace
			);
			if ((status < 0) || (status >= limit))
				break;
			else
				n += status, limit -= status;
		}
	} while (0);

	v4l2_loop_dbg_at4("%s", buf);
}

static inline void v4l2_loop_print_requestbuffers(const struct v4l2_requestbuffers* requestbuffers)
{
	v4l2_loop_dbg_at4(
		"v4l2_requestbuffers:\n"
		"\tcount       : %u\n"
		"\ttype        : %s\n"
		"\tmemory      : %s\n"
		"\tcapabilities: 0x%08x\n",
		requestbuffers->count,
		v4l2_loop_buf_type_to_string(requestbuffers->type),
		v4l2_loop_memory_to_string(requestbuffers->memory),
		requestbuffers->capabilities
	);
}

static inline void v4l2_loop_print_buffer(const struct v4l2_buffer* buffer)
{
	char buf[512];
	size_t n = 0;
	size_t limit = sizeof(buf);
	int status;

	memset(buf, 0, sizeof(buf));

	do {
		status = snprintf(&buf[n], limit,
			"v4l2_buffer:\n"
			"\tindex       : %u\n"
			"\ttype        : %s\n"
			"\tbyteused    : %u\n"
			"\tflags       : 0x%08x\n"
			"\tfield       : %u\n"
			"\tsequence    : %u\n"
			"\tmemory      : %s\n"
			"\tlength      : %u\n",
			buffer->index,
			v4l2_loop_buf_type_to_string(buffer->type),
			buffer->bytesused,
			buffer->flags,
			buffer->field,
			buffer->sequence,
			v4l2_loop_memory_to_string(buffer->memory),
			buffer->length
		);
		if ((status < 0) || (status >= limit))
			break;
		else
			n += status, limit -= status;

		if (V4L2_TYPE_IS_MULTIPLANAR(buffer->type)) {
			unsigned i;
			for (i = 0; i < buffer->length; ++i) {
				status = snprintf(&buf[n], limit,
					"\t\tbytesused   : %u\n"
					"\t\tlength      : %u\n",
					buffer->m.planes[i].bytesused,
					buffer->m.planes[i].length
				);
				if ((status < 0) || (status >= limit))
					break;
				else
					n += status, limit -= status;

				if (buffer->memory == V4L2_MEMORY_MMAP) {
					status = snprintf(&buf[n], limit,
						"\t\tmem_offset  : %u\n",
						buffer->m.planes[i].m.mem_offset
					);
				} else
				if (buffer->memory == V4L2_MEMORY_USERPTR) {
					status = snprintf(&buf[n], limit,
						"\t\tuserptr     : 0x%lx\n",
						buffer->m.planes[i].m.userptr
					);
				} else
				if (buffer->memory == V4L2_MEMORY_DMABUF) {
					status = snprintf(&buf[n], limit,
						"\t\tfd          : %d\n",
						buffer->m.planes[i].m.fd
					);
				} else {
					status = 0;
				}
				if ((status < 0) || (status >= limit))
					break;
				else
					n += status, limit -= status;
			}
		} else {
			if (buffer->memory == V4L2_MEMORY_MMAP) {
				status = snprintf(&buf[n], limit,
					"\toffset      : %u\n",
					buffer->m.offset
				);
			} else
			if (buffer->memory == V4L2_MEMORY_USERPTR) {
				status = snprintf(&buf[n], limit,
					"\tuserptr     : 0x%lx\n",
					buffer->m.userptr
				);
			} else
			if (buffer->memory == V4L2_MEMORY_DMABUF) {
				status = snprintf(&buf[n], limit,
					"\tfd          : %d\n",
					buffer->m.fd
				);
			} else {
				status = 0;
			}
			if ((status < 0) || (status >= limit))
				break;
			else
				n += status, limit -= status;
		}
	} while (0);

	v4l2_loop_dbg_at4("%s", buf);
}

#endif /* V4L2_LOOP_PRINT_FUNCTIONS */