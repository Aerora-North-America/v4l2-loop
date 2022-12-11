/* SPDX-License-Identifier: GPL-2.0 */
/*
 * v4l2-loop-fmtdesc-mplane.h
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

/* two non contiguous planes - one Y, one Cr + Cb interleaved  */
#ifdef V4L2_PIX_FMT_NV12M
{
	.pixelformat = V4L2_PIX_FMT_NV12M,
	.name = "NV12M",
	.is_compressed = false,
	.planes = 2,
	.depth = {{8, 8}, {4, 8}}
},
#endif
#ifdef V4L2_PIX_FMT_NV21M
{
	.pixelformat = V4L2_PIX_FMT_NV21M,
	.name = "NV21M",
	.is_compressed = false,
	.planes = 2,
	.depth = {{8, 8}, {4, 8}}
},
#endif
#ifdef V4L2_PIX_FMT_NV16M
{
	.pixelformat = V4L2_PIX_FMT_NV16M,
	.name = "NV16M",
	.is_compressed = false,
	.planes = 2,
	.depth = {{8, 8}, {8, 8}}
},
#endif
#ifdef V4L2_PIX_FMT_NV61M
{
	.pixelformat = V4L2_PIX_FMT_NV61M,
	.name = "NV61M",
	.is_compressed = false,
	.planes = 2,
	.depth = {{8, 8}, {8, 8}}
},
#endif
/* three non contiguous planes - Y Cb, Cr */
#ifdef V4L2_PIX_FMT_YUV420M
{
	.pixelformat = V4L2_PIX_FMT_YUV420M,
	.name = "YUV420M",
	.is_compressed = false,
	.planes = 3,
	.depth = {{8, 8}, {2, 8}, {2, 8}}
},
#endif
#ifdef V4L2_PIX_FMT_YVU420M
{
	.pixelformat = V4L2_PIX_FMT_YVU420M,
	.name = "YVU420M",
	.is_compressed = false,
	.planes = 3,
	.depth = {{8, 8}, {2, 8}, {2, 8}}
},
#endif
#ifdef V4L2_PIX_FMT_YUV422M
{
	.pixelformat = V4L2_PIX_FMT_YUV422M,
	.name = "YUV422M",
	.is_compressed = false,
	.planes = 3,
	.depth = {{8, 8}, {4, 8}, {4, 8}}
},
#endif
#ifdef V4L2_PIX_FMT_YVU422M
{
	.pixelformat = V4L2_PIX_FMT_YVU422M,
	.name = "YVU422M",
	.is_compressed = false,
	.planes = 3,
	.depth = {{8, 8}, {4, 8}, {4, 8}}
},
#endif
#ifdef V4L2_PIX_FMT_YUV444M
{
	.pixelformat = V4L2_PIX_FMT_YUV444M,
	.name = "YUV444M",
	.is_compressed = false,
	.planes = 3,
	.depth = {{8, 8}, {8, 8}, {8, 8}}
},
#endif
#ifdef V4L2_PIX_FMT_YVU444M
{
	.pixelformat = V4L2_PIX_FMT_YVU444M,
	.name = "YVU444M",
	.is_compressed = false,
	.planes = 3,
	.depth = {{8, 8}, {8, 8}, {8, 8}}
},
#endif
#ifdef V4L2_PIX_FMT_NV12MT
{
	.pixelformat = V4L2_PIX_FMT_NV12MT,
	.name = "NV12MT",
	.is_compressed = false,
	.planes = 2,
	.depth = {{8, 8}, {8, 8}}
},
#endif
#ifdef V4L2_PIX_FMT_NV12MT_16X16
{
	.pixelformat = V4L2_PIX_FMT_NV12MT_16X16,
	.name = "NV12MT_16X16",
	.is_compressed = false,
	.planes = 2,
	.depth = {{8, 8}, {8, 8}}
},
#endif
