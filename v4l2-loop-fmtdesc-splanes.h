/* SPDX-License-Identifier: GPL-2.0 */
/*
 * v4l2-loop-fmtdesc-splane.h
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

/* RGB formats */
#ifdef V4L2_PIX_FMT_RGB332
{
	.pixelformat = V4L2_PIX_FMT_RGB332,
	.name = "RGB332",
	.is_compressed = false,
	.planes = 1,
	.depth = {{8, 8}}
},
#endif
#ifdef V4L2_PIX_FMT_RGB444
{
	.pixelformat = V4L2_PIX_FMT_RGB444,
	.name = "RGB444",
	.is_compressed = false,
	.planes = 1,
	.depth = {{16, 8}}
},
#endif
#ifdef V4L2_PIX_FMT_ARGB444
{
	.pixelformat = V4L2_PIX_FMT_ARGB444,
	.name = "ARGB444",
	.is_compressed = false,
	.planes = 1,
	.depth = {{16, 8}}
},
#endif
#ifdef V4L2_PIX_FMT_XRGB444
{
	.pixelformat = V4L2_PIX_FMT_XRGB444,
	.name = "XRGB444",
	.is_compressed = false,
	.planes = 1,
	.depth = {{16, 8}}
},
#endif
#ifdef V4L2_PIX_FMT_RGBA444
{
	.pixelformat = V4L2_PIX_FMT_RGBA444,
	.name = "RGBA444",
	.is_compressed = false,
	.planes = 1,
	.depth = {{16, 8}}
},
#endif
#ifdef V4L2_PIX_FMT_RGBX444
{
	.pixelformat = V4L2_PIX_FMT_RGBX444,
	.name = "RGBX444",
	.is_compressed = false,
	.planes = 1,
	.depth = {{16, 8}}
},
#endif
#ifdef V4L2_PIX_FMT_ABGR444
{
	.pixelformat = V4L2_PIX_FMT_ABGR444,
	.name = "ABGR444",
	.is_compressed = false,
	.planes = 1,
	.depth = {{16, 8}}
},
#endif
#ifdef V4L2_PIX_FMT_XBGR444
{
	.pixelformat = V4L2_PIX_FMT_XBGR444,
	.name = "XBGR444",
	.is_compressed = false,
	.planes = 1,
	.depth = {{16, 8}}
},
#endif
#ifdef V4L2_PIX_FMT_BGRA444
{
	.pixelformat = V4L2_PIX_FMT_BGRA444,
	.name = "BGRA444",
	.is_compressed = false,
	.planes = 1,
	.depth = {{16, 8}}
},
#endif
#ifdef V4L2_PIX_FMT_BGRX444
{
	.pixelformat = V4L2_PIX_FMT_BGRX444,
	.name = "BGRX444",
	.is_compressed = false,
	.planes = 1,
	.depth = {{16, 8}}
},
#endif
#ifdef V4L2_PIX_FMT_RGB555
{
	.pixelformat = V4L2_PIX_FMT_RGB555,
	.name = "RGB555",
	.is_compressed = false,
	.planes = 1,
	.depth = {{16, 8}}
},
#endif
#ifdef V4L2_PIX_FMT_ARGB555
{
	.pixelformat = V4L2_PIX_FMT_ARGB555,
	.name = "ARGB555",
	.is_compressed = false,
	.planes = 1,
	.depth = {{16, 8}}
},
#endif
#ifdef V4L2_PIX_FMT_XRGB555
{
	.pixelformat = V4L2_PIX_FMT_XRGB555,
	.name = "XRGB555",
	.is_compressed = false,
	.planes = 1,
	.depth = {{16, 8}}
},
#endif
#ifdef V4L2_PIX_FMT_RGBA555
{
	.pixelformat = V4L2_PIX_FMT_RGBA555,
	.name = "RGBA555",
	.is_compressed = false,
	.planes = 1,
	.depth = {{16, 8}}
},
#endif
#ifdef V4L2_PIX_FMT_RGBX555
{
	.pixelformat = V4L2_PIX_FMT_RGBX555,
	.name = "RGBX555",
	.is_compressed = false,
	.planes = 1,
	.depth = {{16, 8}}
},
#endif
#ifdef V4L2_PIX_FMT_ABGR555
{
	.pixelformat = V4L2_PIX_FMT_ABGR555,
	.name = "ABGR555",
	.is_compressed = false,
	.planes = 1,
	.depth = {{16, 8}}
},
#endif
#ifdef V4L2_PIX_FMT_XBGR555
{
	.pixelformat = V4L2_PIX_FMT_XBGR555,
	.name = "XBGR555",
	.is_compressed = false,
	.planes = 1,
	.depth = {{16, 8}}
},
#endif
#ifdef V4L2_PIX_FMT_BGRA555
{
	.pixelformat = V4L2_PIX_FMT_BGRA555,
	.name = "BGRA555",
	.is_compressed = false,
	.planes = 1,
	.depth = {{16, 8}}
},
#endif
#ifdef V4L2_PIX_FMT_BGRX555
{
	.pixelformat = V4L2_PIX_FMT_BGRX555,
	.name = "BGRX555",
	.is_compressed = false,
	.planes = 1,
	.depth = {{16, 8}}
},
#endif
#ifdef V4L2_PIX_FMT_RGB565
{
	.pixelformat = V4L2_PIX_FMT_RGB565,
	.name = "RGB565",
	.is_compressed = false,
	.planes = 1,
	.depth = {{16, 8}}
},
#endif
#ifdef V4L2_PIX_FMT_RGB555X
{
	.pixelformat = V4L2_PIX_FMT_RGB555X,
	.name = "RGB555X",
	.is_compressed = false,
	.planes = 1,
	.depth = {{16, 8}}
},
#endif
#ifdef V4L2_PIX_FMT_ARGB555X
{
	.pixelformat = V4L2_PIX_FMT_ARGB555X,
	.name = "ARGB555X",
	.is_compressed = false,
	.planes = 1,
	.depth = {{16, 8}}
},
#endif
#ifdef V4L2_PIX_FMT_XRGB555X
{
	.pixelformat = V4L2_PIX_FMT_XRGB555X,
	.name = "XRGB555X",
	.is_compressed = false,
	.planes = 1,
	.depth = {{16, 8}}
},
#endif
#ifdef V4L2_PIX_FMT_RGB565X
{
	.pixelformat = V4L2_PIX_FMT_RGB565X,
	.name = "RGB565X",
	.is_compressed = false,
	.planes = 1,
	.depth = {{16, 8}}
},
#endif
#ifdef V4L2_PIX_FMT_BGR666
{
	.pixelformat = V4L2_PIX_FMT_BGR666,
	.name = "BGR666",
	.is_compressed = false,
	.planes = 1,
	.depth = {{18, 8}}
},
#endif
#ifdef V4L2_PIX_FMT_BGR24
{
	.pixelformat = V4L2_PIX_FMT_BGR24,
	.name = "BGR24",
	.is_compressed = false,
	.planes = 1,
	.depth = {{24, 8}}
},
#endif
#ifdef V4L2_PIX_FMT_RGB24
{
	.pixelformat = V4L2_PIX_FMT_RGB24,
	.name = "RGB24",
	.is_compressed = false,
	.planes = 1,
	.depth = {{24, 8}}
},
#endif
#ifdef V4L2_PIX_FMT_BGR32
{
	.pixelformat = V4L2_PIX_FMT_BGR32,
	.name = "BGR32",
	.is_compressed = false,
	.planes = 1,
	.depth = {{32, 8}}
},
#endif
#ifdef V4L2_PIX_FMT_ABGR32
{
	.pixelformat = V4L2_PIX_FMT_ABGR32,
	.name = "ABGR32",
	.is_compressed = false,
	.planes = 1,
	.depth = {{32, 8}}
},
#endif
#ifdef V4L2_PIX_FMT_XBGR32
{
	.pixelformat = V4L2_PIX_FMT_XBGR32,
	.name = "XBGR32",
	.is_compressed = false,
	.planes = 1,
	.depth = {{32, 8}}
},
#endif
#ifdef V4L2_PIX_FMT_BGRA32
{
	.pixelformat = V4L2_PIX_FMT_BGRA32,
	.name = "BGRA32",
	.is_compressed = false,
	.planes = 1,
	.depth = {{32, 8}}
},
#endif
#ifdef V4L2_PIX_FMT_BGRX32
{
	.pixelformat = V4L2_PIX_FMT_BGRX32,
	.name = "BGRX32",
	.is_compressed = false,
	.planes = 1,
	.depth = {{32, 8}}
},
#endif
#ifdef V4L2_PIX_FMT_RGB32
{
	.pixelformat = V4L2_PIX_FMT_RGB32,
	.name = "RGB32",
	.is_compressed = false,
	.planes = 1,
	.depth = {{32, 8}}
},
#endif
#ifdef V4L2_PIX_FMT_RGBA32
{
	.pixelformat = V4L2_PIX_FMT_RGBA32,
	.name = "RGBA32",
	.is_compressed = false,
	.planes = 1,
	.depth = {{32, 8}}
},
#endif
#ifdef V4L2_PIX_FMT_RGBX32
{
	.pixelformat = V4L2_PIX_FMT_RGBX32,
	.name = "RGBX32",
	.is_compressed = false,
	.planes = 1,
	.depth = {{32, 8}}
},
#endif
#ifdef V4L2_PIX_FMT_ARGB32
{
	.pixelformat = V4L2_PIX_FMT_ARGB32,
	.name = "ARGB32",
	.is_compressed = false,
	.planes = 1,
	.depth = {{32, 8}}
},
#endif
#ifdef V4L2_PIX_FMT_XRGB32
{
	.pixelformat = V4L2_PIX_FMT_XRGB32,
	.name = "XRGB32",
	.is_compressed = false,
	.planes = 1,
	.depth = {{32, 8}}
},
#endif
/* Grey formats */
#ifdef V4L2_PIX_FMT_GREY
{
	.pixelformat = V4L2_PIX_FMT_GREY,
	.name = "GREY",
	.is_compressed = false,
	.planes = 1,
	.depth = {{8, 8}}
},
#endif
#ifdef V4L2_PIX_FMT_Y4
{
	.pixelformat = V4L2_PIX_FMT_Y4,
	.name = "Y4",
	.is_compressed = false,
	.planes = 1,
	.depth = {{4, 8}}
},
#endif
#ifdef V4L2_PIX_FMT_Y6
{
	.pixelformat = V4L2_PIX_FMT_Y6,
	.name = "Y6",
	.is_compressed = false,
	.planes = 1,
	.depth = {{6, 8}}
},
#endif
#ifdef V4L2_PIX_FMT_Y10
{
	.pixelformat = V4L2_PIX_FMT_Y10,
	.name = "Y10",
	.is_compressed = false,
	.planes = 1,
	.depth = {{10, 8}}
},
#endif
#ifdef V4L2_PIX_FMT_Y12
{
	.pixelformat = V4L2_PIX_FMT_Y12,
	.name = "Y12",
	.is_compressed = false,
	.planes = 1,
	.depth = {{12, 8}}
},
#endif
#ifdef V4L2_PIX_FMT_Y14
{
	.pixelformat = V4L2_PIX_FMT_Y14,
	.name = "Y14",
	.is_compressed = false,
	.planes = 1,
	.depth = {{14, 8}}
},
#endif
#ifdef V4L2_PIX_FMT_Y16
{
	.pixelformat = V4L2_PIX_FMT_Y16,
	.name = "Y16",
	.is_compressed = false,
	.planes = 1,
	.depth = {{16, 8}}
},
#endif
#ifdef V4L2_PIX_FMT_Y16_BE
{
	.pixelformat = V4L2_PIX_FMT_Y16_BE,
	.name = "Y16_BE",
	.is_compressed = false,
	.planes = 1,
	.depth = {{16, 8}}
},
#endif
/* Grey bit-packed formats */
#ifdef V4L2_PIX_FMT_Y10BPACK
{
	.pixelformat = V4L2_PIX_FMT_Y10BPACK,
	.name = "Y10BPACK",
	.is_compressed = false,
	.planes = 1,
	.depth = {{10, 8}}
},
#endif
#ifdef V4L2_PIX_FMT_Y10P
{
	.pixelformat = V4L2_PIX_FMT_Y10P,
	.name = "Y10P",
	.is_compressed = false,
	.planes = 1,
	.depth = {{10, 8}}
},
#endif
/* Palette formats */
#ifdef V4L2_PIX_FMT_PAL8
{
	.pixelformat = V4L2_PIX_FMT_PAL8,
	.name = "PAL8",
	.is_compressed = false,
	.planes = 1,
	.depth = {{8, 8}}
},
#endif
/* Chrominance formats */
#ifdef V4L2_PIX_FMT_UV8
{
	.pixelformat = V4L2_PIX_FMT_UV8,
	.name = "V4L2_PIX_FMT_UV8",
	.is_compressed = false,
	.planes = 1,
	.depth = {{8, 8}}
},
#endif
/* Luminance+Chrominance formats */
#ifdef V4L2_PIX_FMT_YUYV
{
	.pixelformat = V4L2_PIX_FMT_YUYV,
	.name = "YUYV",
	.is_compressed = false,
	.planes = 1,
	.depth = {{16, 8}}
},
#endif
#ifdef V4L2_PIX_FMT_YYUV
{
	.pixelformat = V4L2_PIX_FMT_YYUV,
	.name = "YYUV",
	.is_compressed = false,
	.planes = 1,
	.depth = {{16, 8}}
},
#endif
#ifdef V4L2_PIX_FMT_YVYU
{
	.pixelformat = V4L2_PIX_FMT_YVYU,
	.name = "YVYU",
	.is_compressed = false,
	.planes = 1,
	.depth = {{16, 8}}
},
#endif
#ifdef V4L2_PIX_FMT_UYVY
{
	.pixelformat = V4L2_PIX_FMT_UYVY,
	.name = "UYVY",
	.is_compressed = false,
	.planes = 1,
	.depth = {{16, 8}}
},
#endif
#ifdef V4L2_PIX_FMT_VYUY
{
	.pixelformat = V4L2_PIX_FMT_VYUY,
	.name = "VYUY",
	.is_compressed = false,
	.planes = 1,
	.depth = {{16, 8}}
},
#endif
#ifdef V4L2_PIX_FMT_Y41P
{
	.pixelformat = V4L2_PIX_FMT_Y41P,
	.name = "V4L2_PIX_FMT_Y41P",
	.is_compressed = false,
	.planes = 1,
	.depth = {{12, 8}}
},
#endif
#ifdef V4L2_PIX_FMT_YUV444
{
	.pixelformat = V4L2_PIX_FMT_YUV444,
	.name = "YUV444",
	.is_compressed = false,
	.planes = 1,
	.depth = {{16, 8}}
},
#endif
#ifdef V4L2_PIX_FMT_YUV555
{
	.pixelformat = V4L2_PIX_FMT_YUV555,
	.name = "YUV555",
	.is_compressed = false,
	.planes = 1,
	.depth = {{16, 8}}
},
#endif
#ifdef V4L2_PIX_FMT_YUV565
{
	.pixelformat = V4L2_PIX_FMT_YUV565,
	.name = "YUV565",
	.is_compressed = false,
	.planes = 1,
	.depth = {{16, 8}}
},
#endif
#ifdef V4L2_PIX_FMT_YUV24
{
	.pixelformat = V4L2_PIX_FMT_YUV24,
	.name = "YUV24",
	.is_compressed = false,
	.planes = 1,
	.depth = {{24, 8}}
},
#endif
#ifdef V4L2_PIX_FMT_YUV32
{
	.pixelformat = V4L2_PIX_FMT_YUV32,
	.name = "YUV32",
	.is_compressed = false,
	.planes = 1,
	.depth = {{32, 8}}
},
#endif
#ifdef V4L2_PIX_FMT_AYUV32
{
	.pixelformat = V4L2_PIX_FMT_AYUV32,
	.name = "AYUV32",
	.is_compressed = false,
	.planes = 1,
	.depth = {{32, 8}}
},
#endif
#ifdef V4L2_PIX_FMT_XYUV32
{
	.pixelformat = V4L2_PIX_FMT_XYUV32,
	.name = "XYUV32",
	.is_compressed = false,
	.planes = 1,
	.depth = {{32, 8}}
},
#endif
#ifdef V4L2_PIX_FMT_VUYA32
{
	.pixelformat = V4L2_PIX_FMT_VUYA32,
	.name = "VUYA32",
	.is_compressed = false,
	.planes = 1,
	.depth = {{32, 8}}
},
#endif
#ifdef V4L2_PIX_FMT_VUYX32
{
	.pixelformat = V4L2_PIX_FMT_VUYX32,
	.name = "VUYX32",
	.is_compressed = false,
	.planes = 1,
	.depth = {{32, 8}}
},
#endif
#ifdef V4L2_PIX_FMT_M420
{
	.pixelformat = V4L2_PIX_FMT_M420,
	.name = "M420",
	.is_compressed = false,
	.planes = 1,
	.depth = {{12, 8}}
},
#endif
/* two contiguous planes (represented as one plane) -- one Y, one Cr + Cb interleaved  */
#ifdef V4L2_PIX_FMT_NV12
{
	.pixelformat = V4L2_PIX_FMT_NV12,
	.name = "NV12",
	.is_compressed = false,
	.planes = 1,
	.depth = {{12, 8}}
},
#endif
#ifdef V4L2_PIX_FMT_NV21
{
	.pixelformat = V4L2_PIX_FMT_NV21,
	.name = "NV21",
	.is_compressed = false,
	.planes = 1,
	.depth = {{12, 8}}
},
#endif
#ifdef V4L2_PIX_FMT_NV16
{
	.pixelformat = V4L2_PIX_FMT_NV16,
	.name = "NV16",
	.is_compressed = false,
	.planes = 1,
	.depth = {{16, 8}}
},
#endif
#ifdef V4L2_PIX_FMT_NV61
{
	.pixelformat = V4L2_PIX_FMT_NV61,
	.name = "NV61",
	.is_compressed = false,
	.planes = 1,
	.depth = {{16, 8}}
},
#endif
#ifdef V4L2_PIX_FMT_NV24
{
	.pixelformat = V4L2_PIX_FMT_NV24,
	.name = "NV24",
	.is_compressed = false,
	.planes = 1,
	.depth = {{24, 8}}
},
#endif
#ifdef V4L2_PIX_FMT_NV42
{
	.pixelformat = V4L2_PIX_FMT_NV42,
	.name = "NV42",
	.is_compressed = false,
	.planes = 1,
	.depth = {{24, 8}}
},
#endif
/* three contiguous planes - Y Cb, Cr (represented as one plane) */
#ifdef V4L2_PIX_FMT_YUV410
{
	.pixelformat = V4L2_PIX_FMT_YUV410,
	.name = "YUV410",
	.is_compressed = false,
	.planes = 1,
	.depth = {{9, 8}}
},
#endif
#ifdef V4L2_PIX_FMT_YVU410
{
	.pixelformat = V4L2_PIX_FMT_YVU410,
	.name = "YVU410",
	.is_compressed = false,
	.planes = 1,
	.depth = {{9, 8}}
},
#endif
#ifdef V4L2_PIX_FMT_YUV411P
{
	.pixelformat = V4L2_PIX_FMT_YUV411P,
	.name = "YUV411P",
	.is_compressed = false,
	.planes = 1,
	.depth = {{12, 8}}
},
#endif
#ifdef V4L2_PIX_FMT_YUV420
{
	.pixelformat = V4L2_PIX_FMT_YUV420,
	.name = "YUV420",
	.is_compressed = false,
	.planes = 1,
	.depth = {{12, 8}}
},
#endif
#ifdef V4L2_PIX_FMT_YVU420
{
	.pixelformat = V4L2_PIX_FMT_YVU420,
	.name = "YVU420",
	.is_compressed = false,
	.planes = 1,
	.depth = {{12, 8}}
},
#endif
#ifdef V4L2_PIX_FMT_YUV422P
{
	.pixelformat = V4L2_PIX_FMT_YUV422P,
	.name = "YUV422P",
	.is_compressed = false,
	.planes = 1,
	.depth = {{16, 8}}
},
#endif
/* Tiled NV12 formats */
#ifdef V4L2_PIX_FMT_NV12_4L4
{
	.pixelformat = V4L2_PIX_FMT_NV12_4L4,
	.name = "NV12_4L4",
	.is_compressed = false,
	.planes = 1,
	.depth = {{12, 8}}
},
#endif
#ifdef V4L2_PIX_FMT_NV12_16L16
{
	.pixelformat = V4L2_PIX_FMT_NV12_16L16,
	.name = "NV12_16L16",
	.is_compressed = false,
	.planes = 1,
	.depth = {{12, 8}}
},
#endif
#ifdef V4L2_PIX_FMT_NV12_32L32
{
	.pixelformat = V4L2_PIX_FMT_NV12_32L32,
	.name = "NV12_32L32",
	.is_compressed = false,
	.planes = 1,
	.depth = {{12, 8}}
},
#endif
/* Bayer formats */
#ifdef V4L2_PIX_FMT_SBGGR8
{
	.pixelformat = V4L2_PIX_FMT_SBGGR8,
	.name = "SBGGR8",
	.is_compressed = false,
	.planes = 1,
	.depth = {{8, 8}}
},
#endif
#ifdef V4L2_PIX_FMT_SGBRG8
{
	.pixelformat = V4L2_PIX_FMT_SGBRG8,
	.name = "SGBRG8",
	.is_compressed = false,
	.planes = 1,
	.depth = {{8, 8}}
},
#endif
#ifdef V4L2_PIX_FMT_SGRBG8
{
	.pixelformat = V4L2_PIX_FMT_SGRBG8,
	.name = "SGRBG8",
	.is_compressed = false,
	.planes = 1,
	.depth = {{8, 8}}
},
#endif
#ifdef V4L2_PIX_FMT_SRGGB8
{
	.pixelformat = V4L2_PIX_FMT_SRGGB8,
	.name = "SRGGB8",
	.is_compressed = false,
	.planes = 1,
	.depth = {{8, 8}}
},
#endif
#ifdef V4L2_PIX_FMT_SBGGR10
{
	.pixelformat = V4L2_PIX_FMT_SBGGR10,
	.name = "SBGGR10",
	.is_compressed = false,
	.planes = 1,
	.depth = {{10, 8}}
},
#endif
#ifdef V4L2_PIX_FMT_SGBRG10
{
	.pixelformat = V4L2_PIX_FMT_SGBRG10,
	.name = "SGBRG10",
	.is_compressed = false,
	.planes = 1,
	.depth = {{10, 8}}
},
#endif
#ifdef V4L2_PIX_FMT_SGRBG10
{
	.pixelformat = V4L2_PIX_FMT_SGRBG10,
	.name = "SGRBG10",
	.is_compressed = false,
	.planes = 1,
	.depth = {{10, 8}}
},
#endif
#ifdef V4L2_PIX_FMT_SRGGB10
{
	.pixelformat = V4L2_PIX_FMT_SRGGB10,
	.name = "SRGGB10",
	.is_compressed = false,
	.planes = 1,
	.depth = {{10, 8}}
},
#endif
#ifdef V4L2_PIX_FMT_SBGGR10P
{
	.pixelformat = V4L2_PIX_FMT_SBGGR10P,
	.name = "SBGGR10P",
	.is_compressed = false,
	.planes = 1,
	.depth = {{10, 8}}
},
#endif
#ifdef V4L2_PIX_FMT_SGBRG10P
{
	.pixelformat = V4L2_PIX_FMT_SGBRG10P,
	.name = "SGBRG10P",
	.is_compressed = false,
	.planes = 1,
	.depth = {{10, 8}}
},
#endif
#ifdef V4L2_PIX_FMT_SGRBG10P
{
	.pixelformat = V4L2_PIX_FMT_SGRBG10P,
	.name = "SGRBG10P",
	.is_compressed = false,
	.planes = 1,
	.depth = {{10, 8}}
},
#endif
#ifdef V4L2_PIX_FMT_SRGGB10P
{
	.pixelformat = V4L2_PIX_FMT_SRGGB10P,
	.name = "SRGGB10P",
	.is_compressed = false,
	.planes = 1,
	.depth = {{10, 8}}
},
#endif
#ifdef V4L2_PIX_FMT_SBGGR10ALAW8
{
	.pixelformat = V4L2_PIX_FMT_SBGGR10ALAW8,
	.name = "SBGGR10ALAW8",
	.is_compressed = false,
	.planes = 1,
	.depth = {{10, 8}}
},
#endif
#ifdef V4L2_PIX_FMT_SGBRG10ALAW8
{
	.pixelformat = V4L2_PIX_FMT_SGBRG10ALAW8,
	.name = "SGBRG10ALAW8",
	.is_compressed = false,
	.planes = 1,
	.depth = {{10, 8}}
},
#endif
#ifdef V4L2_PIX_FMT_SGRBG10ALAW8
{
	.pixelformat = V4L2_PIX_FMT_SGRBG10ALAW8,
	.name = "SGRBG10ALAW8",
	.is_compressed = false,
	.planes = 1,
	.depth = {{10, 8}}
},
#endif
#ifdef V4L2_PIX_FMT_SRGGB10ALAW8
{
	.pixelformat = V4L2_PIX_FMT_SRGGB10ALAW8,
	.name = "V4L2_PIX_FMT_SRGGB10ALAW8",
	.is_compressed = false,
	.planes = 1,
	.depth = {{10, 8}}
},
#endif
#ifdef V4L2_PIX_FMT_SBGGR10DPCM8
{
	.pixelformat = V4L2_PIX_FMT_SBGGR10DPCM8,
	.name = "SBGGR10DPCM8",
	.is_compressed = false,
	.planes = 1,
	.depth = {{10, 8}}
},
#endif
#ifdef V4L2_PIX_FMT_SGBRG10DPCM8
{
	.pixelformat = V4L2_PIX_FMT_SGBRG10DPCM8,
	.name = "SGBRG10DPCM8",
	.is_compressed = false,
	.planes = 1,
	.depth = {{10, 8}}
},
#endif
#ifdef V4L2_PIX_FMT_SGRBG10DPCM8
{
	.pixelformat = V4L2_PIX_FMT_SGRBG10DPCM8,
	.name = "SGRBG10DPCM8",
	.is_compressed = false,
	.planes = 1,
	.depth = {{10, 8}}
},
#endif
#ifdef V4L2_PIX_FMT_SRGGB10DPCM8
{
	.pixelformat = V4L2_PIX_FMT_SRGGB10DPCM8,
	.name = "SRGGB10DPCM8",
	.is_compressed = false,
	.planes = 1,
	.depth = {{10, 8}}
},
#endif
#ifdef V4L2_PIX_FMT_SBGGR12
{
	.pixelformat = V4L2_PIX_FMT_SBGGR12,
	.name = "SBGGR12",
	.is_compressed = false,
	.planes = 1,
	.depth = {{12, 8}}
},
#endif
#ifdef V4L2_PIX_FMT_SGBRG12
{
	.pixelformat = V4L2_PIX_FMT_SGBRG12,
	.name = "SGBRG12",
	.is_compressed = false,
	.planes = 1,
	.depth = {{12, 8}}
},
#endif
#ifdef V4L2_PIX_FMT_SGRBG12
{
	.pixelformat = V4L2_PIX_FMT_SGRBG12,
	.name = "SGRBG12",
	.is_compressed = false,
	.planes = 1,
	.depth = {{12, 8}}
},
#endif
#ifdef V4L2_PIX_FMT_SRGGB12
{
	.pixelformat = V4L2_PIX_FMT_SRGGB12,
	.name = "SRGGB12",
	.is_compressed = false,
	.planes = 1,
	.depth = {{12, 8}}
},
#endif
#ifdef V4L2_PIX_FMT_SBGGR12P
{
	.pixelformat = V4L2_PIX_FMT_SBGGR12P,
	.name = "SBGGR12P",
	.is_compressed = false,
	.planes = 1,
	.depth = {{12, 8}}
},
#endif
#ifdef V4L2_PIX_FMT_SGBRG12P
{
	.pixelformat = V4L2_PIX_FMT_SGBRG12P,
	.name = "SGBRG12P",
	.is_compressed = false,
	.planes = 1,
	.depth = {{12, 8}}
},
#endif
#ifdef V4L2_PIX_FMT_SGRBG12P
{
	.pixelformat = V4L2_PIX_FMT_SGRBG12P,
	.name = "SGRBG12P",
	.is_compressed = false,
	.planes = 1,
	.depth = {{12, 8}}
},
#endif
#ifdef V4L2_PIX_FMT_SRGGB12P
{
	.pixelformat = V4L2_PIX_FMT_SRGGB12P,
	.name = "SRGGB12P",
	.is_compressed = false,
	.planes = 1,
	.depth = {{12, 8}}
},
#endif
#ifdef V4L2_PIX_FMT_SBGGR14
{
	.pixelformat = V4L2_PIX_FMT_SBGGR14,
	.name = "SBGGR14",
	.is_compressed = false,
	.planes = 1,
	.depth = {{14, 8}}
},
#endif
#ifdef V4L2_PIX_FMT_SGBRG14
{
	.pixelformat = V4L2_PIX_FMT_SGBRG14,
	.name = "SGBRG14",
	.is_compressed = false,
	.planes = 1,
	.depth = {{14, 8}}
},
#endif
#ifdef V4L2_PIX_FMT_SGRBG14
{
	.pixelformat = V4L2_PIX_FMT_SGRBG14,
	.name = "SGRBG14",
	.is_compressed = false,
	.planes = 1,
	.depth = {{14, 8}}
},
#endif
#ifdef V4L2_PIX_FMT_SRGGB14
{
	.pixelformat = V4L2_PIX_FMT_SRGGB14,
	.name = "SRGGB14",
	.is_compressed = false,
	.planes = 1,
	.depth = {{14, 8}}
},
#endif
#ifdef V4L2_PIX_FMT_SBGGR14P
{
	.pixelformat = V4L2_PIX_FMT_SBGGR14P,
	.name = "SBGGR14P",
	.is_compressed = false,
	.planes = 1,
	.depth = {{14, 8}}
},
#endif
#ifdef V4L2_PIX_FMT_SGBRG14P
{
	.pixelformat = V4L2_PIX_FMT_SGBRG14P,
	.name = "SGBRG14P",
	.is_compressed = false,
	.planes = 1,
	.depth = {{14, 8}}
},
#endif
#ifdef V4L2_PIX_FMT_SGRBG14P
{
	.pixelformat = V4L2_PIX_FMT_SGRBG14P,
	.name = "SGRBG14P",
	.is_compressed = false,
	.planes = 1,
	.depth = {{14, 8}}
},
#endif
#ifdef V4L2_PIX_FMT_SRGGB14P
{
	.pixelformat = V4L2_PIX_FMT_SRGGB14P,
	.name = "SRGGB14P",
	.is_compressed = false,
	.planes = 1,
	.depth = {{14, 8}}
},
#endif
#ifdef V4L2_PIX_FMT_SBGGR16
{
	.pixelformat = V4L2_PIX_FMT_SBGGR16,
	.name = "SBGGR16",
	.is_compressed = false,
	.planes = 1,
	.depth = {{16, 8}}
},
#endif
#ifdef V4L2_PIX_FMT_SGBRG16
{
	.pixelformat = V4L2_PIX_FMT_SGBRG16,
	.name = "SGBRG16",
	.is_compressed = false,
	.planes = 1,
	.depth = {{16, 8}}
},
#endif
#ifdef V4L2_PIX_FMT_SGRBG16
{
	.pixelformat = V4L2_PIX_FMT_SGRBG16,
	.name = "SGRBG16",
	.is_compressed = false,
	.planes = 1,
	.depth = {{16, 8}}
},
#endif
#ifdef V4L2_PIX_FMT_SRGGB16
{
	.pixelformat = V4L2_PIX_FMT_SRGGB16,
	.name = "SRGGB16",
	.is_compressed = false,
	.planes = 1,
	.depth = {{16, 8}}
},
#endif
/* HSV formats */
#ifdef V4L2_PIX_FMT_HSV24
{
	.pixelformat = V4L2_PIX_FMT_HSV24,
	.name = "HSV24",
	.is_compressed = false,
	.planes = 1,
	.depth = {{24, 8}}
},
#endif
#ifdef V4L2_PIX_FMT_HSV32
{
	.pixelformat = V4L2_PIX_FMT_HSV32,
	.name = "HSV32",
	.is_compressed = false,
	.planes = 1,
	.depth = {{32, 8}}
},
#endif
/* compressed formats */
#ifdef V4L2_PIX_FMT_MJPEG
{
	.pixelformat = V4L2_PIX_FMT_MJPEG,
	.name = "MJPEG",
	.is_compressed = true,
	.planes = 1,
	.depth = {{8, 8}}
},
#endif
#ifdef V4L2_PIX_FMT_JPEG
{
	.pixelformat = V4L2_PIX_FMT_JPEG,
	.name = "JPEG",
	.is_compressed = true,
	.planes = 1,
	.depth = {{8, 8}}
},
#endif
#ifdef V4L2_PIX_FMT_DV
{
	.pixelformat = V4L2_PIX_FMT_DV,
	.name = "DV",
	.is_compressed = true,
	.planes = 1,
	.depth = {{8, 8}}
},
#endif
#ifdef V4L2_PIX_FMT_MPEG
{
	.pixelformat = V4L2_PIX_FMT_MPEG,
	.name = "MPEG",
	.is_compressed = true,
	.planes = 1,
	.depth = {{8, 8}}
},
#endif
#ifdef V4L2_PIX_FMT_H264
{
	.pixelformat = V4L2_PIX_FMT_H264,
	.name = "H264",
	.is_compressed = true,
	.planes = 1,
	.depth = {{8, 8}}
},
#endif
#ifdef V4L2_PIX_FMT_H264_NO_SC
{
	.pixelformat = V4L2_PIX_FMT_H264_NO_SC,
	.name = "H264_NO_SC",
	.is_compressed = true,
	.planes = 1,
	.depth = {{8, 8}}
},
#endif
#ifdef V4L2_PIX_FMT_H264_MVC
{
	.pixelformat = V4L2_PIX_FMT_H264_MVC,
	.name = "H264_MVC",
	.is_compressed = true,
	.planes = 1,
	.depth = {{8, 8}}
},
#endif
#ifdef V4L2_PIX_FMT_H263
{
	.pixelformat = V4L2_PIX_FMT_H263,
	.name = "H263",
	.is_compressed = true,
	.planes = 1,
	.depth = {{8, 8}}
},
#endif
#ifdef V4L2_PIX_FMT_MPEG1
{
	.pixelformat = V4L2_PIX_FMT_MPEG1,
	.name = "MPEG1",
	.is_compressed = true,
	.planes = 1,
	.depth = {{8, 8}}
},
#endif
#ifdef V4L2_PIX_FMT_MPEG2
{
	.pixelformat = V4L2_PIX_FMT_MPEG2,
	.name = "MPEG2",
	.is_compressed = true,
	.planes = 1,
	.depth = {{8, 8}}
},
#endif
#ifdef V4L2_PIX_FMT_MPEG2_SLICE
{
	.pixelformat = V4L2_PIX_FMT_MPEG2_SLICE,
	.name = "MPEG2_SLICE",
	.is_compressed = true,
	.planes = 1,
	.depth = {{8, 8}}
},
#endif
#ifdef V4L2_PIX_FMT_MPEG4
{
	.pixelformat = V4L2_PIX_FMT_MPEG4,
	.name = "MPEG4",
	.is_compressed = true,
	.planes = 1,
	.depth = {{8, 8}}
},
#endif
#ifdef V4L2_PIX_FMT_XVID
{
	.pixelformat = V4L2_PIX_FMT_XVID,
	.name = "XVID",
	.is_compressed = true,
	.planes = 1,
	.depth = {{8, 8}}
},
#endif
#ifdef V4L2_PIX_FMT_VC1_ANNEX_G
{
	.pixelformat = V4L2_PIX_FMT_VC1_ANNEX_G,
	.name = "VC1_ANNEX_G",
	.is_compressed = true,
	.planes = 1,
	.depth = {{8, 8}}
},
#endif
#ifdef V4L2_PIX_FMT_VC1_ANNEX_L
{
	.pixelformat = V4L2_PIX_FMT_VC1_ANNEX_L,
	.name = "VC1_ANNEX_L",
	.is_compressed = true,
	.planes = 1,
	.depth = {{8, 8}}
},
#endif
#ifdef V4L2_PIX_FMT_VP8
{
	.pixelformat = V4L2_PIX_FMT_VP8,
	.name = "VP8",
	.is_compressed = true,
	.planes = 1,
	.depth = {{8, 8}}
},
#endif
#ifdef V4L2_PIX_FMT_VP8_FRAME
{
	.pixelformat = V4L2_PIX_FMT_VP8_FRAME,
	.name = "VP8_FRAME",
	.is_compressed = true,
	.planes = 1,
	.depth = {{8, 8}}
},
#endif
#ifdef V4L2_PIX_FMT_VP9
{
	.pixelformat = V4L2_PIX_FMT_VP9,
	.name = "VP9",
	.is_compressed = true,
	.planes = 1,
	.depth = {{8, 8}}
},
#endif
#ifdef V4L2_PIX_FMT_HEVC
{
	.pixelformat = V4L2_PIX_FMT_HEVC,
	.name = "HEVC",
	.is_compressed = true,
	.planes = 1,
	.depth = {{8, 8}}
},
#endif
#ifdef V4L2_PIX_FMT_FWHT
{
	.pixelformat = V4L2_PIX_FMT_FWHT,
	.name = "FWHT",
	.is_compressed = true,
	.planes = 1,
	.depth = {{8, 8}}
},
#endif
#ifdef V4L2_PIX_FMT_FWHT_STATELESS
{
	.pixelformat = V4L2_PIX_FMT_FWHT_STATELESS,
	.name = "FWHT_STATELESS",
	.is_compressed = true,
	.planes = 1,
	.depth = {{8, 8}}
},
#endif
#ifdef V4L2_PIX_FMT_H264_SLICE
{
	.pixelformat = V4L2_PIX_FMT_H264_SLICE,
	.name = "H264_SLICE",
	.is_compressed = true,
	.planes = 1,
	.depth = {{8, 8}}
},
#endif
