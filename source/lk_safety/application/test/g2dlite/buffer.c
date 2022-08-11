#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <disp_data_type.h>
#include <disp_common.h>
#include <sdm_display.h>

#include "buffer.h"

#define MAKE_RGBA(rgb, r, g, b, a) \
    ((((r) >> (8 - (rgb)->red.length)) << (rgb)->red.offset) | \
     (((g) >> (8 - (rgb)->green.length)) << (rgb)->green.offset) | \
     (((b) >> (8 - (rgb)->blue.length)) << (rgb)->blue.offset) | \
     (((a) >> (8 - (rgb)->alpha.length)) << (rgb)->alpha.offset))

#define MAKE_RGB24(rgb, r, g, b) \
    { .value = MAKE_RGBA(rgb, r, g, b, 0) }

#define MAKE_RGB_INFO(rl, ro, gl, go, bl, bo, al, ao) \
    .rgb = { { (rl), (ro) }, { (gl), (go) }, { (bl), (bo) }, { (al), (ao) } }

static const struct util_format_info format_info[] = {
    /* RGB16 */
    { COLOR_ARGB4444, "AR12", MAKE_RGB_INFO(4, 8, 4, 4, 4, 0, 4, 12) },
    { COLOR_BGRA4444, "BA12", MAKE_RGB_INFO(4, 4, 4, 8, 4, 12, 4, 0) },
    { COLOR_ARGB1555, "AR15", MAKE_RGB_INFO(5, 10, 5, 5, 5, 0, 1, 15) },
    { COLOR_RGB565, "RG16", MAKE_RGB_INFO(5, 11, 6, 5, 5, 0, 0, 0) },
    { COLOR_BGR565, "BG16", MAKE_RGB_INFO(5, 0, 6, 5, 5, 11, 0, 0) },
    /* RGB24 */
    { COLOR_BGR888, "BG24", MAKE_RGB_INFO(8, 0, 8, 8, 8, 16, 0, 0) },
    { COLOR_RGB888, "RG24", MAKE_RGB_INFO(8, 16, 8, 8, 8, 0, 0, 0) },
    /* RGB32 */
    { COLOR_ARGB8888, "AR24", MAKE_RGB_INFO(8, 16, 8, 8, 8, 0, 8, 24) },
    { COLOR_BGRA8888, "BA24", MAKE_RGB_INFO(8, 8, 8, 16, 8, 24, 8, 0) },
    { COLOR_ARGB2101010, "AR30", MAKE_RGB_INFO(10, 20, 10, 10, 10, 0, 2, 30) },
    { COLOR_BGRA2101010, "BA30", MAKE_RGB_INFO(10, 2, 10, 12, 10, 22, 2, 0) },
};

const struct util_format_info *util_format_info_find(uint32_t format)
{
    unsigned int i;

    for (i = 0; i < ARRAY_SIZE(format_info); i++)
        if (format_info[i].format == format)
            return &format_info[i];

    return NULL;
}

static void fill_smpte_rgb16(const struct util_rgb_info *rgb, void *mem,
                 unsigned int width, unsigned int height,
                 unsigned int stride)
{
    const uint16_t colors_top[] = {
        MAKE_RGBA(rgb, 192, 192, 192, 255), /* grey */
        MAKE_RGBA(rgb, 192, 192, 0, 255),   /* yellow */
        MAKE_RGBA(rgb, 0, 192, 192, 255),   /* cyan */
        MAKE_RGBA(rgb, 0, 192, 0, 255),     /* green */
        MAKE_RGBA(rgb, 192, 0, 192, 255),   /* magenta */
        MAKE_RGBA(rgb, 192, 0, 0, 255),     /* red */
        MAKE_RGBA(rgb, 0, 0, 192, 255),     /* blue */
    };
    const uint16_t colors_middle[] = {
        MAKE_RGBA(rgb, 0, 0, 192, 127),     /* blue */
        MAKE_RGBA(rgb, 19, 19, 19, 127),    /* black */
        MAKE_RGBA(rgb, 192, 0, 192, 127),   /* magenta */
        MAKE_RGBA(rgb, 19, 19, 19, 127),    /* black */
        MAKE_RGBA(rgb, 0, 192, 192, 127),   /* cyan */
        MAKE_RGBA(rgb, 19, 19, 19, 127),    /* black */
        MAKE_RGBA(rgb, 192, 192, 192, 127), /* grey */
    };
    const uint16_t colors_bottom[] = {
        MAKE_RGBA(rgb, 0, 33, 76, 255),     /* in-phase */
        MAKE_RGBA(rgb, 255, 255, 255, 255), /* super white */
        MAKE_RGBA(rgb, 50, 0, 106, 255),    /* quadrature */
        MAKE_RGBA(rgb, 19, 19, 19, 255),    /* black */
        MAKE_RGBA(rgb, 9, 9, 9, 255),       /* 3.5% */
        MAKE_RGBA(rgb, 19, 19, 19, 255),    /* 7.5% */
        MAKE_RGBA(rgb, 29, 29, 29, 255),    /* 11.5% */
        MAKE_RGBA(rgb, 19, 19, 19, 255),    /* black */
    };
    unsigned int x;
    unsigned int y;
    uint16_t * dat = (uint16_t *)mem;

    for (y = 0; y < height * 6 / 9; ++y) {
        for (x = 0; x < width; ++x)
            dat[x] = colors_top[x * 7 / width];
        dat += width;
    }

    for (; y < height * 7 / 9; ++y) {
        for (x = 0; x < width; ++x)
            dat[x] = colors_middle[x * 7 / width];
        dat += width;
    }

    for (; y < height; ++y) {
        for (x = 0; x < width * 5 / 7; ++x)
            dat[x] = colors_bottom[x * 4 / (width * 5 / 7)];
        for (; x < width * 6 / 7; ++x)
            dat[x] = colors_bottom[(x - width * 5 / 7) * 3
                          / (width / 7) + 4];
        for (; x < width; ++x)
            dat[x] = colors_bottom[7];
        dat+= width;
    }
}

static void fill_smpte_rgb24(const struct util_rgb_info *rgb, void *mem,
                 unsigned int width, unsigned int height,
                 unsigned int stride)
{
    LOGE("%s %p entry\n", __func__, mem);
    const struct color_rgb24 colors_top[] = {
        MAKE_RGB24(rgb, 192, 192, 192), /* grey */
        MAKE_RGB24(rgb, 192, 192, 0),   /* yellow */
        MAKE_RGB24(rgb, 0, 192, 192),   /* cyan */
        MAKE_RGB24(rgb, 0, 192, 0), /* green */
        MAKE_RGB24(rgb, 192, 0, 192),   /* magenta */
        MAKE_RGB24(rgb, 192, 0, 0), /* red */
        MAKE_RGB24(rgb, 0, 0, 192), /* blue */
    };
    const struct color_rgb24 colors_middle[] = {
        MAKE_RGB24(rgb, 0, 0, 192), /* blue */
        MAKE_RGB24(rgb, 19, 19, 19),    /* black */
        MAKE_RGB24(rgb, 192, 0, 192),   /* magenta */
        MAKE_RGB24(rgb, 19, 19, 19),    /* black */
        MAKE_RGB24(rgb, 0, 192, 192),   /* cyan */
        MAKE_RGB24(rgb, 19, 19, 19),    /* black */
        MAKE_RGB24(rgb, 192, 192, 192), /* grey */
    };
    const struct color_rgb24 colors_bottom[] = {
        MAKE_RGB24(rgb, 0, 33, 76), /* in-phase */
        MAKE_RGB24(rgb, 255, 255, 255), /* super white */
        MAKE_RGB24(rgb, 50, 0, 106),    /* quadrature */
        MAKE_RGB24(rgb, 19, 19, 19),    /* black */
        MAKE_RGB24(rgb, 9, 9, 9),   /* 3.5% */
        MAKE_RGB24(rgb, 19, 19, 19),    /* 7.5% */
        MAKE_RGB24(rgb, 29, 29, 29),    /* 11.5% */
        MAKE_RGB24(rgb, 19, 19, 19),    /* black */
    };
    unsigned int x;
    unsigned int y;
    struct color_rgb24 *dat = (struct color_rgb24 *)mem;

    for (y = 0; y < height * 6 / 9; ++y) {
        for (x = 0; x < width; ++x)
            dat[x] = colors_top[x * 7 / width];
        dat += width;
    }

    for (; y < height * 7 / 9; ++y) {
        for (x = 0; x < width; ++x)
            dat[x] = colors_middle[x * 7 / width];
        dat += width;
    }

    for (; y < height; ++y) {
        for (x = 0; x < width * 5 / 7; ++x)
            dat[x] =
                colors_bottom[x * 4 / (width * 5 / 7)];
        for (; x < width * 6 / 7; ++x)
            dat[x] = colors_bottom[(x - width * 5 / 7) * 3
                     / (width / 7) + 4];
        for (; x < width; ++x)
            dat[x] = colors_bottom[7];
        dat += width;
    }
}

static void fill_smpte_rgb32(const struct util_rgb_info *rgb, void *mem,
                 unsigned int width, unsigned int height,
                 unsigned int stride)
{
    const uint32_t colors_top[] = {
        MAKE_RGBA(rgb, 192, 192, 192, 255), /* grey */
        MAKE_RGBA(rgb, 192, 192, 0, 255),   /* yellow */
        MAKE_RGBA(rgb, 0, 192, 192, 255),   /* cyan */
        MAKE_RGBA(rgb, 0, 192, 0, 255),     /* green */
        MAKE_RGBA(rgb, 192, 0, 192, 255),   /* magenta */
        MAKE_RGBA(rgb, 192, 0, 0, 255),     /* red */
        MAKE_RGBA(rgb, 0, 0, 192, 255),     /* blue */
    };
    const uint32_t colors_middle[] = {
        MAKE_RGBA(rgb, 0, 0, 192, 127),     /* blue */
        MAKE_RGBA(rgb, 19, 19, 19, 127),    /* black */
        MAKE_RGBA(rgb, 192, 0, 192, 127),   /* magenta */
        MAKE_RGBA(rgb, 19, 19, 19, 127),    /* black */
        MAKE_RGBA(rgb, 0, 192, 192, 127),   /* cyan */
        MAKE_RGBA(rgb, 19, 19, 19, 127),    /* black */
        MAKE_RGBA(rgb, 192, 192, 192, 127), /* grey */
    };
    const uint32_t colors_bottom[] = {
        MAKE_RGBA(rgb, 0, 33, 76, 255),     /* in-phase */
        MAKE_RGBA(rgb, 255, 255, 255, 255), /* super white */
        MAKE_RGBA(rgb, 50, 0, 106, 255),    /* quadrature */
        MAKE_RGBA(rgb, 19, 19, 19, 255),    /* black */
        MAKE_RGBA(rgb, 9, 9, 9, 255),       /* 3.5% */
        MAKE_RGBA(rgb, 19, 19, 19, 255),    /* 7.5% */
        MAKE_RGBA(rgb, 29, 29, 29, 255),    /* 11.5% */
        MAKE_RGBA(rgb, 19, 19, 19, 255),    /* black */
    };
    unsigned int x;
    unsigned int y;
    uint32_t *dat = (uint32_t *)mem;

    for (y = 0; y < height * 6 / 9; ++y) {
        for (x = 0; x < width; ++x)
            dat[x] = colors_top[x * 7 / width];
        dat += width;
    }

    for (; y < height * 7 / 9; ++y) {
        for (x = 0; x < width; ++x)
            dat[x] = colors_middle[x * 7 / width];
        dat += width;
    }

    for (; y < height; ++y) {
        for (x = 0; x < width * 5 / 7; ++x)
            dat[x] = colors_bottom[x * 4 / (width * 5 / 7)];
        for (; x < width * 6 / 7; ++x)
            dat[x] = colors_bottom[(x - width * 5 / 7) * 3
                          / (width / 7) + 4];
        for (; x < width; ++x)
            dat[x] = colors_bottom[7];
        dat += width;
    }
}


void fill_smpte(const struct util_format_info *info, void *planes[3],
               unsigned int width, unsigned int height,
               unsigned int stride)
{
    switch (info->format) {
        case COLOR_ARGB4444:
        case COLOR_BGRA4444:
        case COLOR_ARGB1555:
        case COLOR_RGB565:
        case COLOR_BGR565:
            fill_smpte_rgb16(&info->rgb, planes[0], width, height, stride);
            break;
        case COLOR_BGR888:
        case COLOR_RGB888:
            fill_smpte_rgb24(&info->rgb, planes[0], width, height, stride);
            break;
        case COLOR_ARGB8888:
        case COLOR_BGRA8888:
        case COLOR_ARGB2101010:
        case COLOR_BGRA2101010:
            fill_smpte_rgb32(&info->rgb, planes[0], width, height, stride);
            break;
        default:
            break;
    }
}


static uint32_t create_arg_init(int format)
{
    uint32_t bpp = 0;

    switch (format)
    {
    case COLOR_ARGB8888:
    case COLOR_BGRA8888:
    case COLOR_ABGR8888:
    case COLOR_ARGB2101010:
    case COLOR_BGRA2101010:
        bpp = 32;
        break;
    case COLOR_RGB888:
    case COLOR_BGR888:
        bpp = 24;
        break;
    case COLOR_RGB565:
    case COLOR_BGR565:
    case COLOR_ARGB4444:
    case COLOR_BGRA4444:
    case COLOR_ARGB1555:
    case COLOR_BGRA1555:
        bpp = 16;
        break;
    case COLOR_RGB666:
    case COLOR_BGR666:
        bpp = 18;
        break;
    case COLOR_NV21:
    case COLOR_NV12:
    case COLOR_YUV420P:
    case COLOR_YUYV:
    case COLOR_NV61:
    case COLOR_NV16:
        bpp = 8;
        break;
    default:
        LOGE("ERROE !!! don't support this format!\n");
        break;
    }

    dprintf(0, "fmt bpp is %d!\n", bpp);
    return bpp;
}

void bo_update_plane(struct bo *bo, unsigned long planes[4], int stride[], void *addr[4])
{
    int pitches[3];
    unsigned long offsets[3];

    pitches[0] = bo->src_width * bo->bpp / 8;

    switch (bo->fmt) {
        case COLOR_NV21:
        case COLOR_NV12:
            offsets[0] = 0;
            pitches[1] = pitches[0];
            offsets[1] = pitches[0] * bo->src_height;

            planes[0] = (unsigned long)addr[0];
            planes[1] = (unsigned long)addr[1];

            stride[0] = pitches[0];
            break;

        case COLOR_YUV420P:
            pitches[1] = pitches[0] / 2;
            offsets[1] = pitches[0] * bo->src_height;
            offsets[2] = offsets[1] + pitches[1] * bo->src_height / 2;

            planes[0] = (unsigned long)addr[0];
            planes[1] = (unsigned long)addr[1];
            planes[2] = (unsigned long)addr[2];

            stride[0] = pitches[0];
            stride[1] = stride[0] / 2;
            stride[2] = stride[0] / 2;
            break;

        case COLOR_YUYV:
        case COLOR_RGB565:
        case COLOR_BGR565:
        case COLOR_ARGB4444:
        case COLOR_BGRA4444:
        case COLOR_ARGB1555:
        case COLOR_RGB888:
        case COLOR_BGR888:
        case COLOR_ARGB8888:
        case COLOR_BGRA8888:
        case COLOR_ARGB2101010:
        case COLOR_BGRA2101010:
            planes[0] = (unsigned long)addr[0];
            stride[0] = pitches[0];
            break;
    }

    LOGE("pitches[0]:%d stride:%d src_w:%d\n", pitches[0], *stride, bo->src_width);
}

int bo_init(struct bo *bo)
{
    const struct util_format_info *info;
    void *planes[3] = {0};

    bo->bpp = create_arg_init(bo->fmt);
    bo->stride = bo->src_width * bo->bpp / 8;

    bo->addr[0] = memalign(256, bo->stride * bo->src_height);
    if (!bo->addr[0]) return -1;

    memset(bo->addr[0], 0 , bo->stride * bo->src_height);

    info = util_format_info_find(bo->fmt);
    if (!info) {
        LOGE("util not find fmt 0x%x\n", bo->fmt);
        goto find_err;
    }

    planes[0] = bo->addr[0];

    fill_smpte(info, planes, bo->src_width, bo->src_height, bo->stride);

    return 0;

find_err:
    free(bo->addr[0]);
    return -1;
}

void bo_free(struct bo *bo)
{
    free(bo->addr[0]);
}

void dshow_post(int display_id, struct bo *bo)
{
    struct sdm_post_config post;
    struct sdm_buffer bufs;
    display_handle *handle;

    memset(&bufs, 0, sizeof(struct sdm_buffer));
    memset(&post, 0, sizeof(struct sdm_post_config));

    post.n_bufs = 1;
    post.bufs = &bufs;

    bo->bpp = create_arg_init(bo->fmt);

    bo_update_plane(bo, bufs.addr, bufs.src_stride, bo->addr);

    bufs.alpha = 0xff;
    bufs.alpha_en = 1;
    bufs.ckey = 0;
    bufs.ckey_en = 0;
    bufs.fmt = bo->fmt;
    bufs.layer_en = 1;
    bufs.layer = 1;

    bufs.z_order = 0;

    bufs.src.x = 0;
    bufs.src.y = 0;
    bufs.src.w = bo->src_width;
    bufs.src.h = bo->src_height;

    bufs.start.x = 0;
    bufs.start.y = 0;
    bufs.start.w = bo->dst_width;
    bufs.start.h = bo->dst_height;

    bufs.dst.x = 0;
    bufs.dst.y = 0;
    bufs.dst.w = bo->dst_width;
    bufs.dst.h = bo->dst_height;

    handle = hal_get_display_handle(display_id);
    if (!handle) {
        LOGE("display:%d is invalid\n", display_id);
        return;
    }

    sdm_post(handle, &post);
}

