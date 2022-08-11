#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <math.h>
#include <stdint.h>
#include <string>
#include <sys/time.h>
#include <time.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <getopt.h>

#include <drm.h>
#include <drm_fourcc.h>
#include <xf86drm.h>
#include <xf86drmMode.h>
#include <sdrv_g2d_cfg.h>
#include "g2dapi.h"
#include "g2d_test_utils.h"
#include "drm_handle.h"

#include <pthread.h>
#ifdef __ANDROID__
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#endif

g2d_dev_ops* g2dDevOps;
//G2DInput *pInput;

struct img_format_file input_img[IN_FORMAT_MAX] = {
	{ DRM_FORMAT_XRGB8888,    "xrgb8888.yuv" },
	{ DRM_FORMAT_XBGR8888,    "xbgr8888.yuv" },
	{ DRM_FORMAT_BGRX8888,    "bgrx8888.yuv" },
	{ DRM_FORMAT_ARGB8888,    "argb8888.yuv" },
	{ DRM_FORMAT_ABGR8888,    "abgr8888.yuv" },
	{ DRM_FORMAT_BGRA8888,    "bgra8888.yuv" },
	{ DRM_FORMAT_RGB888,      "rgb888.yuv" },
	{ DRM_FORMAT_BGR888,      "bgr888.yuv" },
	{ DRM_FORMAT_RGB565,      "rgb565.yuv" },
	{ DRM_FORMAT_BGR565,      "bgr565.yuv" },
	{ DRM_FORMAT_XRGB4444,    "xrgb4444.yuv" },
	{ DRM_FORMAT_XBGR4444,    "xbgr4444.yuv" },
	{ DRM_FORMAT_BGRX4444,    "bgrx4444.yuv" },
	{ DRM_FORMAT_ARGB4444,    "argb4444.yuv" },
	{ DRM_FORMAT_ABGR4444,    "abgr4444.yuv" },
	{ DRM_FORMAT_BGRA4444,    "bgra4444.yuv" },
	{ DRM_FORMAT_XRGB1555,    "xrgb1555.yuv" },
	{ DRM_FORMAT_XBGR1555,    "xbgr1555.yuv" },
	{ DRM_FORMAT_BGRX5551,    "bgrx5551.yuv" },
	{ DRM_FORMAT_ARGB1555,    "argb1555.yuv" },
	{ DRM_FORMAT_ABGR1555,    "abgr1555.yuv" },
	{ DRM_FORMAT_BGRA5551,    "bgra5551.yuv" },
	{ DRM_FORMAT_XRGB2101010, "xrgb2101010.yuv" },
	{ DRM_FORMAT_XBGR2101010, "xbgr2101010.yuv" },
	{ DRM_FORMAT_BGRX1010102, "bgrx1010102.yuv" },
	{ DRM_FORMAT_ARGB2101010, "argb2101010.yuv" },
	{ DRM_FORMAT_ABGR2101010, "abgr2101010.yuv" },
	{ DRM_FORMAT_BGRA1010102, "bgra1010102.yuv" },
	{ DRM_FORMAT_R8,          "r8.yuv" },
	{ DRM_FORMAT_YUYV,        "yuyv.yuv" },
	{ DRM_FORMAT_UYVY,        "uyvy.yuv" },
	{ DRM_FORMAT_VYUY,        "vyuy.yuv" },
	{ DRM_FORMAT_AYUV,        "ayuv.yuv" },
	{ DRM_FORMAT_NV12,        "nv12.yuv" },
	{ DRM_FORMAT_NV21,        "nv21.yuv" },
	{ DRM_FORMAT_YUV420,      "yuv420.yuv" },
	{ DRM_FORMAT_YVU420,      "yvu420.yuv" },
	{ DRM_FORMAT_YUV422,      "yuv422.yuv" },
	{ DRM_FORMAT_YUV444,      "yuv444.yuv" },
};

struct img_format_file output_img[OUT_FORMAT_MAX] = {
    { DRM_FORMAT_R8, "r8.yuv" },
    { DRM_FORMAT_RGB565, "rgb565.yuv" },
    { DRM_FORMAT_RGB888,"rgb888.yuv" },
    { DRM_FORMAT_XRGB4444, "xrgb4444.yuv" },
    { DRM_FORMAT_ARGB4444, "argb4444.yuv" },
    { DRM_FORMAT_XRGB1555, "xrgb1555.yuv" },
    { DRM_FORMAT_ARGB1555, "argb1555.yuv" },
    { DRM_FORMAT_XRGB8888, "xrgb8888.yuv" },
    { DRM_FORMAT_ARGB8888, "argb8888.yuv" },
    { DRM_FORMAT_XRGB2101010, "xrgb2101010.yuv" },
    { DRM_FORMAT_ARGB2101010, "argb2101010.yuv" },
    { DRM_FORMAT_YUYV, "yuyv.yuv" },
    { DRM_FORMAT_UYVY, "uyvy.yuv" },
    { DRM_FORMAT_AYUV, "ayuv.yuv" },
    { DRM_FORMAT_NV21, "nv21.yuv" },
    { DRM_FORMAT_YUV420, "yuv420.yuv" },
    { DRM_FORMAT_YUV422, "yuv422.yuv" },
    { DRM_FORMAT_YUV444, "yuv444.yuv" },
};

#define LOG_TAG "DeviceMemory"

#define DMA_NON_CONSISTENT (0x1 << 3)
#define DMA_WRITE_COMBINE (0x1 << 2)


void dump_convert_inform(struct err_index *index)
{
	int cont = 0;

	slog_info("**********dump convert fail inform**********\n");

	while (index[cont].in_index > 0) {
		slog_info("convert fail %s to %s\n",
				input_img[index[cont].in_index - 1].file_patch, output_img[index[cont].out_index].file_patch);
		cont ++;
	}

	slog_info("********************************************\n");
}

#define DMA_NON_CONSISTENT (0x1 << 3)
#define DMA_WRITE_COMBINE (0x1 << 2)

static void init_yuv_buf(int type, unsigned char* buf, int width, int height)
{
    unsigned char *src = buf;
    int i, j;

    /*
    unsigned int rainbow_rgb[] = {
        0xFF0000, 0xFF6100, 0xFFFF00, 0x00FF00, 0x00FFFF,
        0x0000FF, 0xA020F0, 0x000000, 0xFFFFFF, 0xF4A460};
    */
    // 由上数组转换而成
/*
    unsigned int rainbow_yuv[] = {
        0x4c54ff, 0x8534d6, 0xe10094, 0x952b15, 0xb2ab00,
        0x1dff6b, 0x5dd2af, 0xbb1654, 0x9c4bc5, 0xb450ad};
*/
    unsigned int rainbow_yuv[] = {
        0x4c54ff, 0x952b15, 0x1dff6b};

    unsigned char *p_y = src;
    unsigned char *p_u = src+width*height;
    unsigned char *p_v = src+2*width*height;

    unsigned char *sp_y = src;
    unsigned char *sp_v = src+width*height;
    unsigned char *sp_u = src+width*height+width*height/4;

    int slice = height / 3;
    for (i = 0; i < height; i++) // h
    {
        int index = i / slice;
        unsigned char y = (rainbow_yuv[index] & 0xff0000 ) >> 16;
        unsigned char u = (rainbow_yuv[index] & 0x00ff00) >> 8;
        unsigned char v = (rainbow_yuv[index] & 0x0000ff);
        if (type == DRM_FORMAT_YUV444)
        {
            for (j=0;j<width;j++) // w
            {
                *p_y++ = y;
                *p_u++ = u;
                *p_v++ = v;
            }
        } else if (type == DRM_FORMAT_YVU420) {
            for (j=0;j<width;j++) // w
            {
                *sp_y++ = y;
            }
            for (j=0;j<width/4;j++) {
                *sp_v++ = v;
                *sp_u++ = u;
            }
        } else {
            for (j=0; j<width*2; j+=4) // w
            {
                if (type == DRM_FORMAT_YUYV)
                {
                    src[i*width*2+j+0] = y; // Y0
                    src[i*width*2+j+1] = u; // U
                    src[i*width*2+j+2] = y; // Y1
                    src[i*width*2+j+3] = v; // V
                }
                if (type == DRM_FORMAT_YVYU)
                {
                    src[i*width*2+j+0] = y; // Y0
                    src[i*width*2+j+1] = v; // V
                    src[i*width*2+j+2] = y; // Y1
                    src[i*width*2+j+3] = u; // U
                }
                else if (type == DRM_FORMAT_UYVY)
                {
                    src[i*width*2+j+0] = u; // U
                    src[i*width*2+j+1] = y; // Y0
                    src[i*width*2+j+2] = v; // V
                    src[i*width*2+j+3] = y; // Y1
                }
                else if (type == DRM_FORMAT_VYUY)
                {
                    src[i*width*2+j+0] = v; // V
                    src[i*width*2+j+1] = y; // Y0
                    src[i*width*2+j+2] = u; // U
                    src[i*width*2+j+3] = y; // Y1
                }
            }
        }
    }
}

static void paint2DrmBufferWithCpuNV21(unsigned char *plane0, unsigned char *plane1, int width, int height, int stride)
{
    int width_served = 8;
    int height_served = 8;

    int r_color = 255;
    int g_color = 0;
    int b_color = 0;

    unsigned char y = 0.257f * r_color + 0.504f * g_color + 0.098f * b_color + 16.5f;
    unsigned char u = -0.148f * r_color - 0.291f * g_color + 0.439f * b_color + 128.5f;
    unsigned char v = 0.439f * r_color - 0.368f * g_color - 0.071f * b_color + 128.5f;

    // set y value
    unsigned char *ypointer = plane0;
    int i = 0;
    for (; i < height / 4; i++)
    {
        int j = 0;
        for (; j < stride - width_served; j++)
            *ypointer++ = y;

        for (; j < stride; j++)
            *ypointer++ = 0;
    }

    r_color = 0;
    g_color = 255;
    b_color = 0;

    y = 0.257f * r_color + 0.504f * g_color + 0.098f * b_color + 16.5f;
    u = -0.148f * r_color - 0.291f * g_color + 0.439f * b_color + 128.5f;
    v = 0.439f * r_color - 0.368f * g_color - 0.071f * b_color + 128.5f;
    for (; i < height / 4 * 2; i++)
    {
        int j = 0;
        for (; j < stride - width_served; j++)
            *ypointer++ = y;

        for (; j < stride; j++)
            *ypointer++ = 0;
    }

    r_color = 0;
    g_color = 0;
    b_color = 255;

    y = 0.257f * r_color + 0.504f * g_color + 0.098f * b_color + 16.5f;
    u = -0.148f * r_color - 0.291f * g_color + 0.439f * b_color + 128.5f;
    v = 0.439f * r_color - 0.368f * g_color - 0.071f * b_color + 128.5f;
    for (; i < height / 4 * 3; i++)
    {
        int j = 0;
        for (; j < stride - width_served; j++)
            *ypointer++ = y;

        for (; j < stride; j++)
            *ypointer++ = 0;
    }

    r_color = 255;
    g_color = 0;
    b_color = 0;

    y = 0.257f * r_color + 0.504f * g_color + 0.098f * b_color + 16.5f;
    u = -0.148f * r_color - 0.291f * g_color + 0.439f * b_color + 128.5f;
    v = 0.439f * r_color - 0.368f * g_color - 0.071f * b_color + 128.5f;
    for (; i < height - height_served; i++)
    {
        int j = 0;
        for (; j < stride - width_served; j++)
            *ypointer++ = y;

        for (; j < stride; j++)
            *ypointer++ = 0;
    }

    for (; i < height; i++)
    {
        for (int j = 0; j < stride; j++)
            *ypointer++ = 0;
    }

    // set uv value . for nv12 , u ->v->u->v
    unsigned char *uvpointer = plane1;
    i = 0;
    for (; i <  height / 2 / 4 * 1; i++)
    {
        int j = 0;
        for (; j < (stride - width_served) / 2; j++)
        {
            *uvpointer++ = v;
            *uvpointer++ = u;
        }
        for (; j < stride / 2; j++)
        {
            *uvpointer++ = 0;
            *uvpointer++ = 0;
        }
    }

    r_color = 0;
    g_color = 255;
    b_color = 0;

    y = 0.257f * r_color + 0.504f * g_color + 0.098f * b_color + 16.5f;
    u = -0.148f * r_color - 0.291f * g_color + 0.439f * b_color + 128.5f;
    v = 0.439f * r_color - 0.368f * g_color - 0.071f * b_color + 128.5f;
    for (; i <  height / 2 / 4 * 2; i++)
    {
        int j = 0;
        for (; j < (stride - width_served) / 2; j++)
        {
            *uvpointer++ = v;
            *uvpointer++ = u;
        }
        for (; j < stride / 2; j++)
        {
            *uvpointer++ = 0;
            *uvpointer++ = 0;
        }
    }

    r_color = 0;
    g_color = 0;
    b_color = 255;

    y = 0.257f * r_color + 0.504f * g_color + 0.098f * b_color + 16.5f;
    u = -0.148f * r_color - 0.291f * g_color + 0.439f * b_color + 128.5f;
    v = 0.439f * r_color - 0.368f * g_color - 0.071f * b_color + 128.5f;
    for (; i < height / 2 / 4 * 3; i++)
    {
        int j = 0;
        for (; j < (stride - width_served) / 2; j++)
        {
            *uvpointer++ = v;
            *uvpointer++ = u;
        }
        for (; j < stride / 2; j++)
        {
            *uvpointer++ = 0;
            *uvpointer++ = 0;
        }
    }

    r_color = 255;
    g_color = 0;
    b_color = 0;

    y = 0.257f * r_color + 0.504f * g_color + 0.098f * b_color + 16.5f;
    u = -0.148f * r_color - 0.291f * g_color + 0.439f * b_color + 128.5f;
    v = 0.439f * r_color - 0.368f * g_color - 0.071f * b_color + 128.5f;
    for (; i <  (height - height_served) / 2; i++)
    {
        int j = 0;
        for (; j < (stride - width_served) / 2; j++)
        {
            *uvpointer++ = v;
            *uvpointer++ = u;
        }
        for (; j < stride / 2; j++)
        {
            *uvpointer++ = 0;
            *uvpointer++ = 0;
        }
    }

    for (; i < height / 2; i++)
    {
        int j = 0;
        for (; j < stride / 2; j++)
        {
            *uvpointer++ = 0;
            *uvpointer++ = 0;
        }
    }
}

static void paint2DrmBufferWithCpuBGRA1010102(unsigned int *plane0, int width, int height, int stride)
{
    int a_color = 0x3;
    int r_color = 0x3ff;
    int g_color = 0;
    int b_color = 0;

    // set y value
    unsigned int *ypointer = plane0;
    int i = 0;
    for (; i < height / 3; i++)
    {
        int j = 0;

        for (; j < width; j++){
			*ypointer++ = (b_color << 22) | (g_color << 12) | (r_color << 2) | a_color;
		}

    }

    r_color = 0;
    g_color = 0x3ff;
    b_color = 0;

    for (i = 0; i < height / 3; i++)
    {
        int j = 0;

        for (; j < width; j++){
			*ypointer++ = (b_color << 22) | (g_color << 12) | (r_color << 2) | a_color;
		}
    }

    r_color = 0;
    g_color = 0;
    b_color = 0x3ff;

    for (i = 0; i < height / 3; i++)
    {
        int j = 0;

		for (; j < width; j++){
			*ypointer++ = (b_color << 22) | (g_color << 12) | (r_color << 2) | a_color;
		}
    }

}


static void paint2DrmBufferWithCpuABGR2101010(unsigned int *plane0, int width, int height, int stride)
{
    int a_color = 0x3;
    int r_color = 0x3ff;
    int g_color = 0;
    int b_color = 0;

    // set y value
    unsigned int *ypointer = plane0;
    int i = 0;
    for (; i < height / 3; i++)
    {
        int j = 0;

        for (; j < width; j++){
			*ypointer++ = (a_color << 30) | (b_color << 20) | (g_color << 10) | r_color;
		}

    }

    r_color = 0;
    g_color = 0x3ff;
    b_color = 0;

    for (i = 0; i < height / 3; i++)
    {
        int j = 0;

        for (; j < width; j++){
			*ypointer++ = (a_color << 30) | (b_color << 20) | (g_color << 10) | r_color;
		}
    }

    r_color = 0;
    g_color = 0;
    b_color = 0x3ff;

    for (i = 0; i < height / 3; i++)
    {
        int j = 0;

		for (; j < width; j++){
			*ypointer++ = (a_color << 30) | (b_color << 20) | (g_color << 10) | r_color;
		}
    }

}


static void paint2DrmBufferWithCpuBGRA5551(unsigned short *plane0, int width, int height, int stride)
{
    int a_color = 0x1;
    int r_color = 0x1f;
    int g_color = 0;
    int b_color = 0;

    // set y value
    unsigned short *ypointer = plane0;
    int i = 0;
    for (; i < height / 3; i++)
    {
        int j = 0;

        for (; j < width; j++){
			*ypointer++ = (b_color << 11) | (g_color << 6) | (r_color << 1) | a_color;
		}

    }

    r_color = 0;
    g_color = 0x1f;
    b_color = 0;

    for (i = 0; i < height / 3; i++)
    {
        int j = 0;

        for (; j < width; j++){
			*ypointer++ = (b_color << 11) | (g_color << 6) | (r_color << 1) | a_color;
		}
    }

    r_color = 0;
    g_color = 0;
    b_color = 0x1f;

    for (i = 0; i < height / 3; i++)
    {
        int j = 0;

		for (; j < width; j++){
			*ypointer++ = (b_color << 11) | (g_color << 6) | (r_color << 1) | a_color;
		}
    }

}


static void paint2DrmBufferWithCpuABGR1555(unsigned short *plane0, int width, int height, int stride)
{
    int a_color = 0x1;
    int r_color = 0x1f;
    int g_color = 0;
    int b_color = 0;

    // set y value
    unsigned short *ypointer = plane0;
    int i = 0;
    for (; i < height / 3; i++)
    {
        int j = 0;

        for (; j < width; j++){
			*ypointer++ = (a_color << 15) | (b_color << 10) | (g_color << 5) | r_color;
		}

    }

    r_color = 0;
    g_color = 0x1f;
    b_color = 0;

    for (i = 0; i < height / 3; i++)
    {
        int j = 0;

        for (; j < width; j++){
			*ypointer++ = (a_color << 15) | (b_color << 10) | (g_color << 5) | r_color;
		}
    }

    r_color = 0;
    g_color = 0;
    b_color = 0x1f;

    for (i = 0; i < height / 3; i++)
    {
        int j = 0;

		for (; j < width; j++){
			*ypointer++ = (a_color << 15) | (b_color << 10) | (g_color << 5) | r_color;
		}
    }

}

static void paint2DrmBufferWithCpuBGRA4444(unsigned char *plane0, int width, int height, int stride)
{
    int a_color = 0xf;
    int r_color = 0xf;
    int g_color = 0;
    int b_color = 0;

    // set y value
    unsigned char *ypointer = plane0;
    int i = 0;
    for (; i < height / 3; i++)
    {
        int j = 0;

        for (; j < width; j++){
			*ypointer++ = (r_color << 4) | a_color;
	        *ypointer++ = (b_color << 4) | g_color;
		}

    }

    r_color = 0;
    g_color = 0xf;
    b_color = 0;

    for (i = 0; i < height / 3; i++)
    {
        int j = 0;

        for (; j < width; j++){
			*ypointer++ = (r_color << 4) | a_color;
	        *ypointer++ = (b_color << 4) | g_color;
		}
    }

    r_color = 0;
    g_color = 0;
    b_color = 0xf;

    for (i = 0; i < height / 3; i++)
    {
        int j = 0;

		for (; j < width; j++){
			*ypointer++ = (r_color << 4) | a_color;
	        *ypointer++ = (b_color << 4) | g_color;
		}
    }

}


static void paint2DrmBufferWithCpuABGR4444(unsigned char *plane0, int width, int height, int stride)
{
    int a_color = 0xf;
    int r_color = 0xf;
    int g_color = 0;
    int b_color = 0;

    // set y value
    unsigned char *ypointer = plane0;
    int i = 0;
    for (; i < height / 3; i++)
    {
        int j = 0;

        for (; j < width; j++){
			*ypointer++ = (g_color << 4) | r_color;
	        *ypointer++ = (a_color << 4) | b_color;
		}

    }

    r_color = 0;
    g_color = 0xf;
    b_color = 0;

    for (i = 0; i < height / 3; i++)
    {
        int j = 0;

        for (; j < width; j++){
			*ypointer++ = (g_color << 4) | r_color;
	        *ypointer++ = (a_color << 4) | b_color;
		}
    }

    r_color = 0;
    g_color = 0;
    b_color = 0xf;

    for (i = 0; i < height / 3; i++)
    {
        int j = 0;

		for (; j < width; j++){
			*ypointer++ = (g_color << 4) | r_color;
	        *ypointer++ = (a_color << 4) | b_color;
		}
    }

}

static void paint2DrmBufferWithCpuBGR(unsigned char *plane0, int width, int height, int stride)
{
    int r_color = 255;
    int g_color = 0;
    int b_color = 0;

    // set y value
    unsigned char *ypointer = plane0;
    int i = 0;
    for (; i < height / 3; i++)
    {
        int j = 0;

        for (; j < width; j++){
	        *ypointer++ = r_color;
			*ypointer++ = g_color;
			*ypointer++ = b_color;
		}

    }

    r_color = 0;
    g_color = 255;
    b_color = 0;

    for (i = 0; i < height / 3; i++)
    {
        int j = 0;

        for (; j < width; j++){
	        *ypointer++ = r_color;
			*ypointer++ = g_color;
			*ypointer++ = b_color;
		}
    }

    r_color = 0;
    g_color = 0;
    b_color = 255;

    for (i = 0; i < height / 3; i++)
    {
        int j = 0;

		for (; j < width; j++){
	        *ypointer++ = r_color;
			*ypointer++ = g_color;
			*ypointer++ = b_color;
		}
    }

}


static int load_raw_data(const char *pic_name, int w, int h, int stride, void *vaddr, int format)
{
    struct drm_mode_create_dumb arg;

	if (!vaddr) {
		slog_err("mapped vaddr is null pointer\n");
		return -1;
	}

    memset(&arg, 0, sizeof(arg));
    create_arg_init(format, w, h, &arg);
    uint32_t data_length = arg.width * arg.height * arg.bpp / 8;
    slog_verbose("input file data length = %d", data_length);

	if (pic_name) {
#ifdef __ANDROID__
		if (strstr(pic_name, ".png")) {
			int w, h, c;
			char *data = (char *)stbi_load(pic_name, &w, &h, &c, 0);
			slog_info("load png %s: %d x %d x %d\n", pic_name, w, h, c);
			char *p, *q;
			for(int i = 0; i < h; i++) {
				p = (char *)((char *)data + i * w *c);
				q = (char *)((char *)vaddr + stride * i);
				memcpy(q, p, w * c);
			}
			stbi_image_free(data);
		} else
#endif
		{
			slog_info("read raw buffer %s\n", pic_name);
			FILE *fp = fopen(pic_name, "rb");
			if (fp) {
				size_t sz = fread(vaddr, 1, data_length, fp);
				slog_info("sz len = %ld\n", sz);
			} else {
				slog_info("ERROR open %s is fail!!\n", pic_name);
				return -1;
			}
			fclose(fp);
		}
#if DEBUG_INPUT
        //write input buffer to file
        {
            slog_info("............. start write input buffer to file : vaddr = %p\n", vaddr);
            FILE *fp = fopen("/mnt/media_rw/input_c.yuv", "w+b");
            fseek(fp, 0, SEEK_SET);
            fwrite(vaddr, data_length, 1, fp);
            fclose(fp);
        }
#endif
	}
	return 0;
}

static int paint_raw_data(dm_drm_handle_t *data, void *vaddr, uint32_t width, uint32_t hight)
{
    struct drm_mode_create_dumb arg;
    create_arg_init(data->drm_format, width, hight, &arg);
    uint32_t data_length = arg.width * arg.height * arg.bpp / 8;
    // paint raw data
    if (data->drm_format == DRM_FORMAT_NV21)
    {
        unsigned char *plane0 = (unsigned char *)vaddr;
        plane0 = plane0 + data->offset[0];
        unsigned char *plane1 = (unsigned char *)vaddr;
        plane1 = plane1 + data->offset[1];
        paint2DrmBufferWithCpuNV21(plane0, plane1, width, hight, data->stride[0]);
    } else if((data->drm_format == DRM_FORMAT_BGR888) || (data->drm_format == DRM_FORMAT_RGB888))
	{
		unsigned char *plane0 = (unsigned char *)vaddr;
		paint2DrmBufferWithCpuBGR(plane0, width, hight, data->stride[0]);
	} else if ((data->format == DRM_FORMAT_XBGR4444) || (data->format == DRM_FORMAT_ABGR4444)) {
        unsigned char *plane0 = (unsigned char *)vaddr;
        paint2DrmBufferWithCpuABGR4444(plane0, width, hight, data->stride[0]);
    } else if ((data->format == DRM_FORMAT_BGRX4444) || (data->format == DRM_FORMAT_BGRA4444)) {
        unsigned char *plane0 = (unsigned char *)vaddr;
        paint2DrmBufferWithCpuBGRA4444(plane0, width, hight, data->stride[0]);
    } else if ((data->format == DRM_FORMAT_XBGR1555) || (data->format == DRM_FORMAT_ABGR1555)) {
        unsigned short *plane0 = (unsigned short *)vaddr;
        paint2DrmBufferWithCpuABGR1555(plane0, width, hight, data->stride[0]);
    } else if ((data->format == DRM_FORMAT_BGRX5551) || (data->format == DRM_FORMAT_BGRA5551)) {
        unsigned short *plane0 = (unsigned short *)vaddr;
        paint2DrmBufferWithCpuBGRA5551(plane0, width, hight, data->stride[0]);
    } else if ((data->format == DRM_FORMAT_XBGR2101010) || (data->format == DRM_FORMAT_ABGR2101010)) {
        unsigned int *plane0 = (unsigned int *)vaddr;
        paint2DrmBufferWithCpuABGR2101010(plane0, width, hight, data->stride[0]);
    } else if ((data->format == DRM_FORMAT_BGRX1010102) || (data->format == DRM_FORMAT_BGRA1010102)) {
        unsigned int *plane0 = (unsigned int *)vaddr;
        paint2DrmBufferWithCpuBGRA1010102(plane0, width, hight, data->stride[0]);
    } else if ((data->format == DRM_FORMAT_YUV444) || (data->format == DRM_FORMAT_YUYV) ||
        (data->format == DRM_FORMAT_YVYU) || (data->format == DRM_FORMAT_UYVY) ||
        (data->format == DRM_FORMAT_VYUY) || (data->format == DRM_FORMAT_YVU420)) {
        unsigned char *plane0 = (unsigned char *)vaddr;
        init_yuv_buf(data->format, plane0, width, hight);
    } else {
        slog_info("............. data->drm_format dosent match ! no input raw data \n");
        return -1;
    }

#if 1
    //write input buffer to file
    {
        slog_info("............. start write input buffer to file : vaddr = %p\n", vaddr);
        FILE *fp = fopen("/mnt/media_rw/input_c.yuv", "w+b");
        if (fp == NULL) {
            slog_err("error : can not open input file ");
            return -1;
        }
        fseek(fp, 0, SEEK_SET);
        fwrite(vaddr, data_length, 1, fp);
        fclose(fp);
    }
#endif

    return 0;
}

struct dm_drm_t *create_dm_drm(void)
{
    //open device
    return dm_drm_create();
}

struct dm_drm_bo_t *creat_drm_bo(dm_drm_t *dm_drm, int width, int heigh, int *prime_fd, int format, void **vaddr)
{
	dm_handle_t *target = NULL;
	struct dm_drm_bo_t *bo = NULL;
	int stride;
	int err = 0;

    //alloc bo
    bo = dm_drm_bo_create(dm_drm, width, heigh, format, 0);
    if (!bo)
    {
        slog_info("ERROR: Failed to create object dm.\n");
        return NULL;
    }
    stride = bo->handle->stride[0];
    target = (dm_handle_t *)bo->handle;
    slog_info("................dm alloc: target=%p, stride=%d, format: %d stride: %d err:%d\n",
              target, stride, format, stride, err);

    //map

    err = dm_drm_bo_lock(bo, target->flags, 0, 0, width, heigh, vaddr);
    if (err)
    {
        slog_info("ERROR: Failed to dm_drm_bo_lock.\n");
        return NULL;
    }
    slog_info("............. before render to frame buffer dm map: vaddr = %p, err:%d\n", *vaddr, err);

	struct dm_drm_handle_t *data = (struct dm_drm_handle_t *)target;

	*prime_fd = data->prime_fd;

	return bo;
}

void creat_outfile_path(char *out, char *in_path, const char *out_path)
{
	char *tmp_head = NULL;
	char *tmp_tail = NULL;

	tmp_head = strrchr(in_path,'/');

	tmp_tail = strrchr(in_path,'.');

    strcpy(out, "/mnt/media_rw/");

	if((!tmp_head) && tmp_tail) {
        strncat(out, in_path, tmp_tail - in_path);
    } else if(tmp_tail && tmp_head)
		strncat(out, tmp_head + 1, tmp_tail - 1 - tmp_head);
	else
		strncat(out, in_path, strlen(in_path));

	strcat(out, "_to_");
	strcat(out, out_path);
}

static int g2d_input_fill(dm_drm_t *dm_drm, G2DInput *input, char raw_path[][128])
{
    struct dm_drm_bo_t *input_bo[G2D_LAYER_MAX_NUM];
	struct dm_drm_handle_t *data;
    int err = 0;

	for (int i = 0; i < input->layer_num; i++) {
		//input layer
		slog_info("fill layer[%d] \n", i);
		input_bo[i] = creat_drm_bo(dm_drm, input->layer[i].src_w, input->layer[i].src_h, &input->layer[i].src_fd,
			input->layer[i].src_format, &input->layer[i].input_vaddr);
		if(!input_bo[i]) {
			slog_info("ERROR: Failed to create object input_bo.\n");
			goto input_err;
		}

		//paint raw data
		data = (struct dm_drm_handle_t *)input_bo[i]->handle;
		if(strlen(raw_path[i]) == 0) {
			slog_info("start paint raw date !");
			strcpy(raw_path[i], "in");
			err = paint_raw_data(data, input->layer[i].input_vaddr, input->layer[i].src_w, input->layer[i].src_h);
		} else {
			slog_info("load %s raw data !", raw_path[i]);
			err = load_raw_data(raw_path[i], input->layer[i].src_w, input->layer[i].src_h, input_bo[i]->handle->stride[0],
				input->layer[i].input_vaddr, input->layer[i].src_format);
		}
		if (err) {
			slog_info("paint layer[%d] failed \n", i);
			goto input_err;
		}
	}

    return 0;

input_err:
	for (int i = 0; i < input->layer_num; i++) {
		dm_drm_bo_unlock(input_bo[i]);
		input_bo[i]->drm->drv->free(input_bo[i]->drm, input_bo[i]);
	}
	return -1;
}

static void* g2d_alloc_layer_buffer(dm_drm_t *dm_drm, int w, int h, int fmt, int *fd, struct dm_drm_bo_t *bo)
{
    void *vaddr;

    bo = creat_drm_bo(dm_drm, w, h, fd, fmt, &vaddr);
    if(!bo) {
        slog_info("ERROR: Failed to create object bo.\n");
        goto input_err;
    }

    return vaddr;

input_err:
    dm_drm_bo_unlock(bo);
	bo->drm->drv->free(bo->drm, bo);
    return NULL;
}

static int g2d_fill_layer_img(dm_drm_t *dm_drm, struct dpc_layer *layer, char *raw_path)
{
    struct dm_drm_handle_t *data;
    struct dm_drm_bo_t *bo = nullptr;
    void *vaddr;
    int err = 0;
    int fd;

    vaddr = g2d_alloc_layer_buffer(dm_drm, layer->src_w, layer->src_h, layer->format, &fd, bo);
    //paint raw data
    data = (struct dm_drm_handle_t *)bo->handle;
    if(strlen(raw_path) == 0) {
        slog_info("start paint raw date !");
        strcpy(raw_path, "in");
        err = paint_raw_data(data, vaddr, layer->src_w, layer->src_h);
    } else {
        slog_info("load %s raw data !", raw_path);
        err = load_raw_data(raw_path, layer->src_w, layer->src_h, bo->handle->stride[0],
            vaddr, layer->format);
    }
    if (err) {
        slog_info("paint layer failed \n");
        goto input_err;
    }

    return fd;
input_err:
    dm_drm_bo_unlock(bo);
	bo->drm->drv->free(bo->drm, bo);
    return -1;

}

static int g2d_layer_blend_splice(G2DInput *input)
{
    dm_drm_t *dm_drm = NULL;
	struct dm_drm_bo_t *output_bo = NULL;
	char print_path[50] = {0};
	int ret = 0;
	struct drm_mode_create_dumb arg;
	char path[G2D_LAYER_MAX_NUM][128] = {{0}};
	char out_path[128] = "out1.yuv";

	int width = 1280;
	int high = 720;

	input->layer_num = 3;
	for (int i = 0; i < input->layer_num; i++) {
		input->layer[i].src_w = input->layer[i].src_crop_w = input->layer[i].dst_crop_w = width;
		input->layer[i].src_h = input->layer[i].src_crop_h = input->layer[i].dst_crop_h = high;
	}
	input->layer[0].src_format = DRM_FORMAT_YUYV;
	input->layer[0].alpha = 0xff;
	input->layer[0].blend_mode = BLEND_PIXEL_COVERAGE;
	input->layer[0].zpos = 0;
	input->layer[0].layer_index = 0;
    input->layer[0].src_crop_x = 0;
    input->layer[0].src_crop_y = 0;
    input->layer[0].dst_crop_x = 0;
    input->layer[0].dst_crop_y = 0;

	input->layer[1].src_format = DRM_FORMAT_YUYV;
	input->layer[1].alpha = 0xff;
	input->layer[1].blend_mode = BLEND_PIXEL_COVERAGE;
	input->layer[1].zpos = 1;
	input->layer[1].layer_index = 1;
    input->layer[1].src_crop_x = 0;
    input->layer[1].src_crop_y = 0;
    input->layer[1].dst_crop_x = 1280;
    input->layer[1].dst_crop_y = 0;

	input->layer[2].src_format = DRM_FORMAT_YUYV;
	input->layer[2].alpha = 0xff;
	input->layer[2].blend_mode = BLEND_PIXEL_COVERAGE;
	input->layer[2].zpos = 2;
	input->layer[2].layer_index = 2;
    input->layer[2].src_crop_x = 0;
    input->layer[2].src_crop_y = 0;
    input->layer[2].dst_crop_x = 0;
    input->layer[2].dst_crop_y = 720;

	// strcpy(path[0], "/mnt/media_rw/1280x720.rgba");
	// strcpy(path[1], "/mnt/media_rw/1280x720.rgba");
    // strcpy(path[2], "/mnt/media_rw/1280x720.rgba");

	strcpy(path[0], "/mnt/media_rw/1280x720_yuyv.yuv");
	strcpy(path[1], "/mnt/media_rw/1280x720_yuyv.yuv");
    strcpy(path[2], "/mnt/media_rw/1280x720_yuyv.yuv");
    input->dst_w = 2560;
    input->dst_h = 1440;
    input->dst_format = DRM_FORMAT_R8;
    //g2dinput->rotation = ROTATION_TYPE_ROT_90;

	dm_drm = create_dm_drm();
	if (!dm_drm)
    {
        slog_info("ERROR: Failed to create drm device.\n");
        return -1;
    }

	ret = g2d_input_fill(dm_drm, input, path);
	if (ret == -1)
		goto load_raw_err;

	//output layer
	slog_info("fill output layer\n");
    output_bo = creat_drm_bo(dm_drm, input->dst_w, input->dst_h, &input->dst_fd, input->dst_format, &input->dst_vaddr);
    if (!output_bo)
    {
        slog_info("ERROR: Failed to create object output_bo.\n");
		ret = -1;
        goto output_bo_lock_err;
    }

	//g2d convert
    g2dDevOps->g2dOpen();
    slog_info(".............g2d convert start!");
    ret = g2dDevOps->g2dConvert(input);
    slog_info(".............g2d convert end !");
    if (ret)
    {
        slog_info("g2d_convert error err:%d\n", ret);
		goto g2d_convert_err;
    }

    ///write output buffer to file
    {
        slog_info("dst_w : %d, dst_h : %d ", input->dst_w, input->dst_h);
		create_arg_init(input->dst_format, input->dst_w, input->dst_h, &arg);
        slog_info("............. start write output buffer to file dm map: vaddr = %p, err:%d\n", input->dst_vaddr, ret);
		creat_outfile_path(print_path, path[0], out_path);
		slog_info("output file input_path=%s out_path=%s print_path:%s\n", path[0], out_path, print_path);

        FILE *fp = fopen(print_path, "w+b");
        if(fp == NULL) {
            slog_err(" fopen is error, fp = NULL");
        }
        fseek(fp, 0, SEEK_SET);
        fwrite(input->dst_vaddr, arg.width * arg.height * arg.bpp / 8, 1, fp);
        fclose(fp);
    }

g2d_convert_err:
	g2dDevOps->g2dClose();
output_bo_lock_err:
	dm_drm_bo_unlock(output_bo);
    output_bo->drm->drv->free(output_bo->drm, output_bo);
load_raw_err:
//drm_bo_err:
    if (dm_drm->drv)
        dm_drm->drv->destroy(dm_drm->drv);
    close(dm_drm->fd);
    free(dm_drm);
    return ret;
}

static int g2d_layer_blend(G2DInput *input)
{
    dm_drm_t *dm_drm = NULL;
	struct dm_drm_bo_t *output_bo = NULL;
	char print_path[50] = {0};
	int ret = 0;
	struct drm_mode_create_dumb arg;
	char path[G2D_LAYER_MAX_NUM][128] = {{0}};
	char out_path[128] = "out1.yuv";

	int width = 800;
	int high = 600;

	input->layer_num = 2;
	for (int i = 0; i < input->layer_num; i++) {
		input->layer[i].src_w = input->layer[i].src_crop_w = width;
		input->layer[i].src_h = input->layer[i].src_crop_h = high;
	}
	input->layer[0].src_format = DRM_FORMAT_ARGB8888;
	input->layer[0].alpha = 0x55;
	input->layer[0].blend_mode = BLEND_PIXEL_COVERAGE;
	input->layer[0].zpos = 1;
	input->layer[0].layer_index = 0;

	input->layer[1].src_format = DRM_FORMAT_ARGB8888;
	input->layer[1].alpha = 0x55;
	input->layer[1].blend_mode = BLEND_PIXEL_COVERAGE;
	input->layer[1].zpos = 0;
	input->layer[1].layer_index = 1;

	strcpy(path[0], "/mnt/media_rw/input.yuv");
	strcpy(path[1], "/mnt/media_rw/b_argb.yuv");

    input->dst_w = input->layer[0].dst_crop_w = input->layer[1].dst_crop_w = width;
    input->dst_h = input->layer[0].dst_crop_h = input->layer[1].dst_crop_h = high;
    input->dst_format = DRM_FORMAT_ARGB8888;
    //g2dinput->rotation = ROTATION_TYPE_ROT_90;


	dm_drm = create_dm_drm();
	if (!dm_drm)
    {
        slog_info("ERROR: Failed to create drm device.\n");
        return -1;
    }

	ret = g2d_input_fill(dm_drm, input, path);
	if (ret == -1)
		goto load_raw_err;

	//output layer
	slog_info("fill output layer\n");
    output_bo = creat_drm_bo(dm_drm, input->dst_w, input->dst_h, &input->dst_fd, input->dst_format, &input->dst_vaddr);
    if (!output_bo)
    {
        slog_info("ERROR: Failed to create object output_bo.\n");
		ret = -1;
        goto output_bo_lock_err;
    }

	//g2d convert
    g2dDevOps->g2dOpen();
    slog_info(".............g2d convert start!");
    ret = g2dDevOps->g2dConvert(input);
    slog_info(".............g2d convert end !");
    if (ret)
    {
        slog_info("g2d_convert error err:%d\n", ret);
		goto g2d_convert_err;
    }

    ///write output buffer to file
    {
        slog_info("dst_w : %d, dst_h : %d ", input->dst_w, input->dst_h);
		create_arg_init(input->dst_format, input->dst_w, input->dst_h, &arg);
        slog_info("............. start write output buffer to file dm map: vaddr = %p, err:%d\n", input->dst_vaddr, ret);
		creat_outfile_path(print_path, path[0], out_path);
		slog_info("output file input_path=%s out_path=%s print_path:%s\n", path[0], out_path, print_path);

        FILE *fp = fopen(print_path, "w+b");
        if(fp == NULL) {
            slog_err(" fopen is error, fp = NULL");
        }
        fseek(fp, 0, SEEK_SET);
        fwrite(input->dst_vaddr, arg.width * arg.height * arg.bpp / 8, 1, fp);
        fclose(fp);
    }

g2d_convert_err:
	g2dDevOps->g2dClose();
output_bo_lock_err:
	dm_drm_bo_unlock(output_bo);
    output_bo->drm->drv->free(output_bo->drm, output_bo);
load_raw_err:
//drm_bo_err:
    if (dm_drm->drv)
        dm_drm->drv->destroy(dm_drm->drv);
    close(dm_drm->fd);
    free(dm_drm);
    return ret;
}


static int g2d_convert_set(G2DInput *input, char *raw_path, const char *out_path)
{
    dm_drm_t *dm_drm = NULL;
    struct dm_drm_bo_t *input_bo = NULL;
	struct dm_drm_bo_t *output_bo = NULL;
	struct dm_drm_handle_t *data;
	char print_path[128] = {0};
    int err = 0;
	struct drm_mode_create_dumb arg;
	dm_drm = create_dm_drm();
	if (!dm_drm)
    {
        slog_info("ERROR: Failed to create drm device.\n");
        return -1;
    }
	int out_width = 1280;
	int out_high = 720;
/*
	static int do_count = 0;
	if (do_count % 2)
		strcpy(raw_path, "/mnt/media_rw/1920x1080_bgr24.yuv");
	else
		strcpy(raw_path, "/mnt/media_rw/in_to_out.yuv");
	do_count++;
*/
	strcpy(raw_path, "/mnt/media_rw/1280x720_nv21.yuv");

	input->layer_num = 1;
	input->layer[0].layer_index = 0;
	input->layer[0].src_w = out_width;
	input->layer[0].src_h = out_high;
	input->layer[0].src_crop_w = out_width;
	input->layer[0].src_crop_h = out_high;
	input->layer[0].src_format = DRM_FORMAT_NV21;
	input->layer[0].alpha = 0xff;
	input->layer[0].blend_mode = BLEND_PIXEL_NONE;
	input->layer[0].zpos = 0;

    input->dst_w = input->layer[0].dst_crop_w = out_width;
    input->dst_h = input->layer[0].dst_crop_h = out_high;
    input->dst_format = DRM_FORMAT_ARGB8888;
	//input->rotation = ROTATION_TYPE_ROT_270;

	input_bo = creat_drm_bo(dm_drm, input->layer[0].src_w, input->layer[0].src_h, &input->layer[0].src_fd,
		input->layer[0].src_format, &input->layer[0].input_vaddr);
	if(!input_bo) {
		slog_info("ERROR: Failed to create object input_bo.\n");
		err = -1;
		goto load_raw_err;
	}

    //paint raw data
    data = (struct dm_drm_handle_t *)input_bo->handle;
    if(strlen(raw_path) == 0) {
        slog_info("start paint raw date !");
		strcpy(raw_path, "in");
        err = paint_raw_data(data, input->layer[0].input_vaddr, input->layer[0].src_w, input->layer[0].src_h);
    } else {
        slog_info("load %s raw data !", raw_path);
        err = load_raw_data(raw_path, input->layer[0].src_w, input->layer[0].src_h, input_bo->handle->stride[0],
            input->layer[0].input_vaddr, input->layer[0].src_format);
    }
	if (err)
		goto load_raw_err;

    output_bo = creat_drm_bo(dm_drm, input->dst_w, input->dst_h, &input->dst_fd, input->dst_format, &input->dst_vaddr);
    if (!output_bo)
    {
        slog_info("ERROR: Failed to create object output_bo.\n");
		err = -1;
        goto output_bo_lock_err;
    }


    g2dDevOps->g2dOpen();
    slog_info(".............g2d open end ! data->prime_fd[%d]", data->prime_fd);

    slog_info(".............g2d convert start!");
    err = g2dDevOps->g2dConvert(input);
    slog_info(".............g2d convert end !");
    if (err)
    {
        slog_info("g2d_convert error err:%d\n", err);
		slog_info("cuishang >>>> raw_path = %s", raw_path);
		goto g2d_convert_err;
    }

    ///write output buffer to file
    {
        slog_info("dst_w : %d, dst_h : %d ", input->dst_w, input->dst_h);
		create_arg_init(input->dst_format, input->dst_w, input->dst_h, &arg);
        slog_info("............. start write output buffer to file dm map: vaddr = %p, err:%d\n",
			input->dst_vaddr, err);
		creat_outfile_path(print_path, raw_path, out_path);
		slog_info("output file input_path=%s out_path=%s print_path:%s\n", raw_path, out_path, print_path);

        FILE *fp = fopen(print_path, "w+b");
        if(fp == NULL) {
            slog_err(" fopen is error, fp = NULL");
        }
        fseek(fp, 0, SEEK_SET);
        fwrite(input->dst_vaddr, arg.width * arg.height * arg.bpp / 8, 1, fp);
        fclose(fp);
    }

g2d_convert_err:
	g2dDevOps->g2dClose();
output_bo_lock_err:
	dm_drm_bo_unlock(output_bo);
    output_bo->drm->drv->free(output_bo->drm, output_bo);
load_raw_err:
	dm_drm_bo_unlock(input_bo);
    input_bo->drm->drv->free(input_bo->drm, input_bo);
//drm_bo_err:
    if (dm_drm->drv)
        dm_drm->drv->destroy(dm_drm->drv);
    close(dm_drm->fd);
    free(dm_drm);
    return err;
}

static int g2d_test_fillrect(char *raw_path, const char *out_path)
{
    dm_drm_t *dm_drm = NULL;
	struct dm_drm_bo_t *output_bo = NULL;
	char print_path[128] = {0};
    int err = 0;
	struct drm_mode_create_dumb arg;
	void *dst_vaddr;

	struct g2d_bg_cfg bgcfg;
	memset(&bgcfg,0x0,sizeof(struct g2d_bg_cfg));
	struct g2d_output_cfg outcfg;
	memset(&outcfg, 0x0, sizeof(struct g2d_output_cfg));

	outcfg.width = 1920;
	outcfg.height = 1080;
	outcfg.fmt = DRM_FORMAT_YUYV;
	//outcfg.stride[0] =

	bgcfg.en = 1;
    bgcfg.color = 0xff3ff;
    bgcfg.g_alpha = 0xff;

	strcpy(raw_path, "fillrect");
	dm_drm = create_dm_drm();
	if (!dm_drm)
    {
        slog_info("ERROR: Failed to create drm device.\n");
        return -1;
    }

	/* output */
    output_bo = creat_drm_bo(dm_drm, outcfg.width, outcfg.height, &outcfg.bufs[0].fd, outcfg.fmt, &dst_vaddr);
    if (!output_bo)
    {
        slog_info("ERROR: Failed to create object output_bo.\n");
		err = -1;
        goto output_bo_lock_err;
    }

	/* g2d deal with*/
    g2dDevOps->g2dOpen();
    slog_info(".............g2d open end ! ");
    err = g2dDevOps->g2dFillrect(&bgcfg, &outcfg);
    if (err)
    {
        slog_info("g2dFastcopy error err:%d\n", err);
		goto g2d_convert_err;
    }

	slog_info("******************g2d deal with succsed \n");
    /**write output buffer to file**/
    {
        slog_info("dst_w : %d, dst_h : %d ", outcfg.width, outcfg.height);
		create_arg_init(outcfg.fmt, outcfg.width, outcfg.height, &arg);
        slog_info("............. start write output buffer to file dm map: vaddr = %p, err:%d\n", dst_vaddr, err);
		creat_outfile_path(print_path, raw_path, out_path);
		slog_info("output file input_path=%s out_path=%s print_path:%s\n", raw_path, out_path, print_path);

        FILE *fp = fopen(print_path, "w+b");
        if(fp == NULL) {
            slog_err(" fopen is error, fp = NULL");
        }
        fseek(fp, 0, SEEK_SET);
        fwrite(dst_vaddr, arg.width * arg.height * arg.bpp / 8, 1, fp);
        fclose(fp);
    }

g2d_convert_err:
	g2dDevOps->g2dClose();
output_bo_lock_err:
	dm_drm_bo_unlock(output_bo);
    output_bo->drm->drv->free(output_bo->drm, output_bo);
//drm_bo_err:
    if (dm_drm->drv)
        dm_drm->drv->destroy(dm_drm->drv);
    close(dm_drm->fd);
    free(dm_drm);
    return err;
}

static int g2d_test_fastcopy(char *raw_path, const char *out_path)
{
    dm_drm_t *dm_drm = NULL;
    struct dm_drm_bo_t *input_bo = NULL;
	struct dm_drm_bo_t *output_bo = NULL;
	struct dm_drm_handle_t *data;
	char print_path[128] = {0};
    int err = 0;
	struct drm_mode_create_dumb arg;
	int src_fd;
	int src_format;
	void *input_vaddr;
	int dst_fd;
	int dst_format;
	void *dst_vaddr;


	dm_drm = create_dm_drm();
	if (!dm_drm)
    {
        slog_info("ERROR: Failed to create drm device.\n");
        return -1;
    }
	int out_width = 1920;
	int out_high = 1080;
	src_format = DRM_FORMAT_NV21;
	dst_format = DRM_FORMAT_NV21;
	int in_stride, out_stride;

	create_arg_init(dst_format, out_width, out_high, &arg);

	in_stride = out_stride = arg.width * arg.height * arg.bpp / 8 / out_high;

	strcpy(raw_path, "/mnt/media_rw/1920x1080_nv21.yuv");


	/* input */
	input_bo = creat_drm_bo(dm_drm, out_width, out_high, &src_fd, src_format, &input_vaddr);
	if(!input_bo) {
		slog_info("ERROR: Failed to create object input_bo.\n");
		err = -1;
		goto load_raw_err;
	}
    //paint raw data
    data = (struct dm_drm_handle_t *)input_bo->handle;
    if(strlen(raw_path) == 0) {
        slog_info("start paint raw date !");
		strcpy(raw_path, "in");
        err = paint_raw_data(data, input_vaddr, out_width, out_high);
    } else {
        slog_info("load %s raw data !", raw_path);
        err = load_raw_data(raw_path, out_width, out_high, input_bo->handle->stride[0],
            input_vaddr, src_format);
    }
	if (err)
		goto load_raw_err;

	/* output */
    output_bo = creat_drm_bo(dm_drm, out_width, out_high, &dst_fd, dst_format, &dst_vaddr);
    if (!output_bo)
    {
        slog_info("ERROR: Failed to create object output_bo.\n");
		err = -1;
        goto output_bo_lock_err;
    }

	/* g2d deal with*/
    g2dDevOps->g2dOpen();
    slog_info(".............g2d open end ! data->prime_fd[%d]", data->prime_fd);

	slog_info("src_fd %d dst_fd %d", src_fd, dst_fd);
    err = g2dDevOps->g2dFastcopy(src_fd, dst_fd, out_width, out_high,
		in_stride, out_stride, dst_format);
    if (err)
    {
        slog_info("g2dFastcopy error err:%d\n", err);
		goto g2d_convert_err;
    }

	slog_info("******************g2d deal with succsed \n");
    /**write output buffer to file**/
    {
        slog_info("dst_w : %d, dst_h : %d ", out_width, out_high);
		create_arg_init(dst_format, out_width, out_high, &arg);
        slog_info("............. start write output buffer to file dm map: vaddr = %p, err:%d\n", dst_vaddr, err);
		creat_outfile_path(print_path, raw_path, out_path);
		slog_info("output file input_path=%s out_path=%s print_path:%s\n", raw_path, out_path, print_path);

        FILE *fp = fopen(print_path, "w+b");
        if(fp == NULL) {
            slog_err(" fopen is error, fp = NULL");
        }
        fseek(fp, 0, SEEK_SET);
        fwrite(dst_vaddr, arg.width * arg.height * arg.bpp / 8, 1, fp);
        fclose(fp);
    }

g2d_convert_err:
	g2dDevOps->g2dClose();
output_bo_lock_err:
	dm_drm_bo_unlock(output_bo);
    output_bo->drm->drv->free(output_bo->drm, output_bo);
load_raw_err:
	dm_drm_bo_unlock(input_bo);
    input_bo->drm->drv->free(input_bo->drm, input_bo);
//drm_bo_err:
    if (dm_drm->drv)
        dm_drm->drv->destroy(dm_drm->drv);
    close(dm_drm->fd);
    free(dm_drm);
    return err;
}


static void usage()
{
    static const char *msg =
"g2d_test [ -h ] [ options ]\n"
"General options:\n"
"  --help           Commandline help (you're reading it)\n"
"  -p, --path           input file path \n"
"  --src_format          input format (DRM_FORMAT_xxx)\n"
"  --dst_format          output format (DRM_FORMAT_xxx)\n"
"  --src_w          input src_w \n"
"  --dst_w          input dst_w \n"
"  --src_crop_w          input src_crop_w \n"
"  --dst_crop_w          output dst_crop_w \n"
"  --src_h          input src_h \n"
"  --dst_h          output dst_h \n"
"  --src_crop_h          input src_crop_h \n"
"  --dst_crop_h          output dst_crop_h \n"
"  -r, --rotation          output rotation (support input):\n"
"       90,180,270,hflip,vflip,hv90,vf90\n"
"  --auto				auto test\n"
"";
    printf("usage: %s", msg);
    exit(0);
}

static int parse_input_args(int argc, char *argv[], G2DInput *input, char *path, bool *auto_flag, int *test_cases)
{
    int opt;
    //char *string = "p:h:c:d";
    int option_index = 0;
    int format;
    static const struct option slong_options[] = {
        {"src_format", 1,NULL, 'f'},
        {"dst_format", 1,NULL, 'f'},
        {"src_w",      1,NULL, 'w'},
        {"dst_w",      1,NULL, 'w'},
        {"src_crop_w",      1,NULL, 'w'},
        {"dst_crop_w",      1,NULL, 'w'},
        {"src_h",      1,NULL, 'h'},
        {"dst_h",      1,NULL, 'h'},
        {"src_crop_h",      1,NULL, 'h'},
        {"dst_crop_h",      1,NULL, 'h'},
        {"rotation",      1,NULL, 'r'},
        {"path",       1,NULL, 'p'},
		{"auto",       0,NULL, 'a'},
        {"help",       0,NULL, 'x'},
        {"noarg",      0,NULL,'n'},
        { 0, 0, 0, 0 }
    };
    while((opt =getopt_long(argc,argv,"p:r:a:t:d",slong_options,&option_index))!= -1) {
        switch (opt) {
            default:
            case 'x': usage(); break;
            case 'p':
                strcpy(path, optarg);
                slog_verbose(" input file path : %s", path);
                break;
            case 'f':
                if (strlen(optarg) > 4)
                    break;
                format = fourcc_code(optarg[0], optarg[1], optarg[2], optarg[3]);
                if (option_index == 0) {
                    input->layer[0].src_format = format;
                    slog_verbose("input src fromat : %c%c%c%c", input->layer[0].src_format & 0xff, (input->layer[0].src_format >> 8) & 0xff,
                        (input->layer[0].src_format >> 16) & 0xff, (input->layer[0].src_format >> 24) & 0xff);
                } else if (option_index == 1) {
                    input->dst_format = format;
                    slog_verbose("input dst fromat : %c%c%c%c", input->dst_format & 0xff, (input->dst_format >> 8) & 0xff,
                        (input->dst_format >> 16) & 0xff, (input->dst_format >> 24) & 0xff);
                }
                break;
            case 'w':
                switch(option_index) {
                    case 2://src_w
                        input->layer[0].src_w = atoi(optarg);
                        slog_verbose("input src w : %d", input->layer[0].src_w);
                        break;
                    case 3://dst_w
                        input->dst_w = atoi(optarg);
                        slog_verbose("input dst w : %d", input->dst_w);
                        break;
                    case 4://src_crop_w
                        input->layer[0].src_crop_w = atoi(optarg);
                        slog_verbose("input src crop w : %d", input->layer[0].src_crop_w);
                        break;
                    case 5://dst_crop_w
                        input->layer[0].dst_crop_w = atoi(optarg);
                        slog_verbose("input dst crop w : %d", input->layer[0].dst_crop_w);
                        break;
                    default:
                        break;
                }
                break;
            case 'h':
                switch(option_index) {
                    case 6://src_h
                        input->layer[0].src_h = atoi(optarg);
                        slog_verbose("input src w : %d", input->layer[0].src_h);
                        break;
                    case 7://dst_h
                        input->dst_h = atoi(optarg);
                        slog_verbose("input dst w : %d", input->dst_h);
                        break;
                    case 8://src_crop_h
                        input->layer[0].src_crop_h = atoi(optarg);
                        slog_verbose("input src crop w : %d", input->layer[0].src_crop_h);
                        break;
                    case 9://dst_crop_h
                        input->layer[0].dst_crop_h= atoi(optarg);
                        slog_verbose("input dst crop w : %d", input->layer[0].dst_crop_h);
                        break;
                    default:
                        break;
                }
                break;
            case 'r':
                if(!strcmp(optarg, "90")) {
                    input->rotation = ROTATION_TYPE_ROT_90;
                } else if(!strcmp(optarg, "180")) {
                    input->rotation = ROTATION_TYPE_ROT_180;
                } else if(!strcmp(optarg, "270")) {
                    input->rotation = ROTATION_TYPE_ROT_270;
                } else if(!strcmp(optarg, "hflip")) {
                    input->rotation = ROTATION_TYPE_HFLIP;
                } else if(!strcmp(optarg, "vflip")) {
                    input->rotation = ROTATION_TYPE_VFLIP;
                } else if(!strcmp(optarg, "vf90")) {
                    input->rotation = ROTATION_TYPE_VF_90;
                } else if(!strcmp(optarg, "hf90")) {
                    input->rotation = ROTATION_TYPE_HF_90;
                } else {
                    slog_err("input rotation args: %s invail !\n", optarg);
                }
                slog_info("input rotation is %s \n", optarg);
                break;
			case 'a':
				*auto_flag = true;
				slog_info("******The test is auto test*********\n");
                break;
			case 't':
				slog_info("2 atoi(optarg) = %d",atoi(optarg));
				*test_cases = atoi(optarg);
				slog_info("3");
				break;
        }
    }

    return 0;
}
#define ALIGN32(_x)             (((_x)+0x1f)&~0x1f)


int main(int argc, char *argv[])
{
    char path[255] = {0};
    G2DInput *g2dinput = NULL;
	bool auto_flag = false;
	int in_index = 0, out_index = 0, err_index = 0;
	struct err_index store_err_index[IN_FORMAT_MAX * OUT_FORMAT_MAX] = { {-1, -1} };
	int ret = 0;
	int test_cases = -1;

    g2dinput = (G2DInput*)malloc(sizeof(G2DInput));
    if (g2dinput == NULL) {
        slog_info( "g2dinput malloc failed");
        goto ERROR_OUT;
    }
    memset((void *)g2dinput, 0, sizeof(G2DInput));

    g2dDevOps = GetG2dDevOps();
    parse_input_args(argc, argv, g2dinput, path, &auto_flag, &test_cases);

	switch (test_cases) {
	case 1 : //blend
		g2d_layer_blend(g2dinput);
		goto ERROR_OUT;
	break;
	case 2 : //fast copy
		g2d_test_fastcopy(path, "out.yuv");
		goto ERROR_OUT;
	break;
	case 3 : //fill rect
		g2d_test_fillrect(path, "out,yuv");
		goto ERROR_OUT;
	break;
    case 4 : //blend
	    g2d_layer_blend_splice(g2dinput);
	    goto ERROR_OUT;
	break;
	default :
	break;
	}

	if (!auto_flag) {
        {
#if 0 //random width , high
            width = ALIGN32(rand() % 1920);
            high = ALIGN32(rand() % 1080);
            if (width < 32)
                width = 32;
            if (high < 32)
                high = 32;
            g2dinput->src_w = g2dinput->src_crop_w = width;
            g2dinput->src_h = g2dinput->src_crop_h = high;
            g2dinput->dst_w = g2dinput->dst_crop_w = width;
            g2dinput->dst_h = g2dinput->dst_crop_h = high;
#endif
			int count = 0;
			int ret = 0;
			do {
	            ret = g2d_convert_set(g2dinput, path, "out.yuv");
				if (ret) {
					slog_err("cuishang >>>> do g2d_convert_set count = %d", count);
					goto ERROR_OUT;
				}
				count++;
			} while (false);
        }
	} else {
		for (in_index = 0; in_index < IN_FORMAT_MAX; in_index++)
		{
			if (strlen(input_img[in_index].file_patch) <=0 )	continue;

			for(out_index = 0; out_index < OUT_FORMAT_MAX; out_index++)
			{
				g2dinput->layer[0].src_format = input_img[in_index].format;
				g2dinput->dst_format = output_img[out_index].format;

				ret = g2d_convert_set(g2dinput, input_img[in_index].file_patch, output_img[out_index].file_patch);
				if (ret < 0) {
					store_err_index[err_index].in_index = in_index + 1;
					store_err_index[err_index++].out_index = out_index;
					slog_info(" in_index : %d, out_index :%d", in_index, out_index);
				}
			}
		}

		dump_convert_inform(store_err_index);
	}


ERROR_OUT:
    if (g2dinput != NULL)
    {
        free(g2dinput);
        g2dinput = NULL;
        slog_info("free g2dinput !");
    }
    return 0;
}

