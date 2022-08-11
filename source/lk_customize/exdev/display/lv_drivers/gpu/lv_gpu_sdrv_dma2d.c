/**
 * @file lv_gpu_sdrv_dma2d.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "lv_gpu_sdrv_dma2d.h"
#include <stdio.h>
#include <stdlib.h>
#include "lvgl.h"
#include "disp_data_type.h"
#include <sdm_display.h>
#include <g2dlite_api.h>

#if LV_USE_GPU_SEMIDRIVE_G2D

#include "dma_hal.h"

/*********************
 *      DEFINES
 *********************/

#if LV_COLOR_16_SWAP
    // TODO: F7 has red blue swap bit in control register for all layers and output
    #error "Can't use DMA2D with LV_COLOR_16_SWAP 1"
#endif

#if LV_COLOR_DEPTH == 8
    #error "Can't use DMA2D with LV_COLOR_DEPTH == 8"
#endif

#if LV_COLOR_DEPTH == 16
    #define LV_DMA2D_COLOR_FORMAT LV_DMA2D_RGB565
#elif LV_COLOR_DEPTH == 32
    #define LV_DMA2D_COLOR_FORMAT LV_DMA2D_ARGB8888
#else
    /*Can't use GPU with other formats*/
#endif

/**********************
 *      TYPEDEFS
 **********************/
#define DMA_SYNC_TIMEOUT (1000)
/**********************
 *  STATIC PROTOTYPES
 **********************/
static void invalidate_cache(void* color_p, size_t size);
static void dma2d_wait(struct dma_desc *desc);

/**********************
 *  STATIC VARIABLES
 **********************/
struct dma_chan *hal_dma_chan_req(enum dma_chan_tr_type ch_type);
static struct dma_chan *DMA2D = NULL;
/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
struct g2dlite {
    int index;
    addr_t reg_addr;
    uint32_t irq_num;
};
static void *G2D = NULL;

/**
 * Turn on the peripheral and set output color mode, this only needs to be done once
 */
void lv_gpu_sdrv_dma2d_init(void)
{
    int ret;
    DMA2D = hal_dma_chan_req(DMA_MEM);

    ret = hal_g2dlite_creat_handle(&G2D, RES_G2D_G2D2);
    if (!ret) {
        LOGD("g2dlite creat handle failed\n");
    }

    hal_g2dlite_init(G2D);
    LOGD("[%s] g2d->index 0x%x\n", __func__, ((struct g2dlite *)G2D)->index);
}

typedef union {
    struct
    {
        uint32_t blue :10;
        uint32_t green :10;
        uint32_t red :10;
    } ch;
    uint32_t full;
} color_10bit_t;

static void gpu_fill_cb(lv_color_t * dest_buf, lv_coord_t dest_width,
                        const lv_area_t * fill_area, lv_color_t color, lv_opa_t opa)
{

    lv_coord_t x, y;
    lv_color_t *p = dest_buf;
    dest_buf += dest_width * fill_area->y1; /*Go to the first line*/

    for(y = fill_area->y1; y < fill_area->y2; y++) {
        for(x = fill_area->x1; x < fill_area->x2; x++) {
            // lv_color_premult(color, opa, (uint16_t *)&dest_buf[x]);
            dest_buf[x] = color;
            LV_COLOR_SET_A(dest_buf[x], opa);
            // dest_buf[x] = lv_color_mix(LV_COLOR_ORANGE, color , LV_OPA_100);
        }
        dest_buf+=dest_width;    /*Go to the next line*/
    }
    //arch_clean_cache_range((addr_t)dest_buf, dest_width * (fill_area->y2 - fill_area->y1) * 4);
}


/**
 * Fill an area in the buffer with a color
 * @param buf a buffer which should be filled
 * @param buf_w width of the buffer in pixels
 * @param color fill color
 * @param fill_w width to fill in pixels (<= buf_w)
 * @param fill_h height to fill in pixels
 * @note `buf_w - fill_w` is offset to the next line after fill
 */

void lv_gpu_sdrv_dma2d_fill(lv_color_t * buf, lv_coord_t buf_w, lv_color_t color, lv_coord_t fill_w, lv_coord_t fill_h, lv_opa_t opa)
{
    struct g2dlite_output_cfg output;
    color_10bit_t bg_color;

    bg_color.ch.blue = (color.ch.blue * 0x3FF)/ 255;
    bg_color.ch.green = (color.ch.green * 0x3FF) / 255;
    bg_color.ch.red = (color.ch.red * 0x3FF) / 255;

    output.width = fill_w;
    output.height = fill_h;
    output.fmt = COLOR_ARGB8888;
    output.addr[0] = (unsigned long)buf;
    output.stride[0] =  buf_w * 4;
    output.rotation = 0;

    hal_g2dlite_fill_rect(G2D, bg_color.full, opa, 0, 0, 0, &output);

    // LV_LOG_WARN("[%s] color 0x%x: (bg_color) 0x%x, opa %d, (%d, %d)", __func__, color.full, bg_color.full, opa, fill_w, fill_h);
}

/**
 * Fill an area in the buffer with a color but take into account a mask which describes the opacity of each pixel
 * @param buf a buffer which should be filled using a mask
 * @param buf_w width of the buffer in pixels
 * @param color fill color
 * @param mask 0..255 values describing the opacity of the corresponding pixel. It's width is `fill_w`
 * @param opa overall opacity. 255 in `mask` should mean this opacity.
 * @param fill_w width to fill in pixels (<= buf_w)
 * @param fill_h height to fill in pixels
 * @note `buf_w - fill_w` is offset to the next line after fill
 */
void lv_gpu_sdrv_dma2d_fill_mask(lv_color_t * buf, lv_coord_t buf_w, lv_color_t color, const lv_opa_t * mask,
                                  lv_opa_t opa, lv_coord_t fill_w, lv_coord_t fill_h)
{
    struct g2dlite_output_cfg output;
    color_10bit_t bg_color;

    bg_color.ch.blue = (color.ch.blue * 0x3FF)/ 255;
    bg_color.ch.green = (color.ch.green * 0x3FF) / 255;
    bg_color.ch.red = (color.ch.red * 0x3FF) / 255;

    output.width = fill_w;
    output.height = fill_h;
    output.fmt = COLOR_ARGB8888;
    output.addr[0] = (unsigned long)buf;
    output.stride[0] =  buf_w * 4;
    output.rotation = 0;
    LV_LOG_ERROR("[lv_gpu_sdrv_dma2d_fill_mask] mask = %p, opa = %d\n", mask, opa);
    hal_g2dlite_fill_rect(G2D, bg_color.full, opa, (addr_t )mask, 8, fill_w, &output);

}

/**
 * Copy a map (typically RGB image) to a buffer
 * @param buf a buffer where map should be copied
 * @param buf_w width of the buffer in pixels
 * @param map an "image" to copy
 * @param map_w width of the map in pixels
 * @param copy_w width of the area to copy in pixels (<= buf_w)
 * @param copy_h height of the area to copy in pixels
 * @note `map_w - fill_w` is offset to the next line after copy
 */
int lv_gpu_sdrv_g2d_copy(lv_color_t * buf, lv_coord_t buf_w, const lv_color_t * map, lv_coord_t map_w,
                             lv_coord_t copy_w, lv_coord_t copy_h)
{
    struct fcopy_t in = {
        .addr = (addr_t)map,
        .width = copy_w,
        .height = copy_h,
        .stride = map_w * 4,
    };

    struct fcopy_t out = {
        .addr = (addr_t)buf,
        .width = copy_w,
        .height = copy_h,
        .stride = buf_w * 4,
    };

    hal_g2dlite_fastcopy(G2D, &in, &out);
    //arch_invalidate_cache_range((addr_t)buf, buf_w * 4 * copy_h);
    return 0;
}


int lv_gpu_sdrv_g2d_blend(const lv_area_t * disp_area, lv_color_t * disp_buf,
                            const lv_area_t * draw_area,
                            const lv_area_t * map_area, const lv_color_t * map_buf, lv_opa_t opa,
                            const lv_opa_t * mask, lv_draw_mask_res_t mask_res) {
    struct g2dlite_input input;
    memset(&input, 0, sizeof(struct g2dlite_input));
    int blend_mode_ = 0;

    input.layer_num = 2;
    #if LV_COLOR_DEPTH == 32

    switch (mask_res) {
        case LV_DRAW_MASK_RES_TRANSP:
        return 0;
        case LV_DRAW_MASK_RES_FULL_COVER:
        blend_mode_ = BLEND_PIXEL_NONE;
        break;
        case LV_DRAW_MASK_RES_CHANGED:
        blend_mode_ = BLEND_PIXEL_COVERAGE;
        break;
    }
    //arch_clean_cache_range((addr_t)map_buf, lv_area_get_size(map_area) * 4);
    //arch_clean_cache_range((addr_t)disp_buf, lv_area_get_size(disp_area) * 4);
    int map_width = lv_area_get_width(map_area);
    int map_height = lv_area_get_height(map_area);
    int disp_width = lv_area_get_width(disp_area);
    int disp_height = lv_area_get_height(disp_area);
    int draw_width = lv_area_get_width(draw_area);
    int draw_height = lv_area_get_height(draw_area);
    uint32_t start  = lv_tick_get();

    // for (int i= 0; i < draw_height; i++)
    // {
    //     arch_clean_cache_range((addr_t)(map_buf+(draw_area->y1-map_area->y1+i)*map_width+(draw_area->x1-map_area->x1)), draw_width*4);
    //     arch_clean_cache_range((addr_t)(disp_buf+(draw_area->y1-disp_area->y1+i)*disp_width+(draw_area->x1-disp_area->x1)), draw_width*4);
    // }
    //LV_LOG_WARN("flush cache  %d\n", lv_tick_get() - start);
    //arch_clean_invalidate_cache_range((addr_t)map_buf, lv_area_get_size(map_area) * 4);
    //arch_clean_invalidate_cache_range((addr_t)disp_buf, lv_area_get_size(disp_area) * 4);
    //thread_sleep(50);

#if 0
    LV_LOG_WARN("[%s] disp_area (%d, %d, %d, %d), map_area (%d, %d, %d, %d), draw_area(%d, %d, %d, %d)\n",
                __func__,
                DUMP_AREA(disp_area), DUMP_AREA(map_area), DUMP_AREA(draw_area));
    LV_LOG_WARN("mask_res = %d, opa = 0x%x\n", mask_res, opa);
#endif
#if 1
    for (int i = 0; i < input.layer_num; i++) {
        struct g2dlite_input_cfg  *l = &input.layer[i];
        l->layer_en = 1;
        l->layer = i;
        l->fmt = COLOR_ARGB8888;
        l->zorder = i;

        l->ckey.en = 0;
        l->blend = blend_mode_;
        l->alpha = opa;

        if (i == 0) {

            l->addr[0] = (unsigned long) disp_buf;
            l->src.x = draw_area->x1;
            l->src.y = draw_area->y1;
            l->src.w = lv_area_get_width(draw_area);
            l->src.h = lv_area_get_height(draw_area);
            l->src_stride[0] = disp_width * 4;

            l->dst.x = 0;// canvas :output x y
            l->dst.y = 0;
            l->dst.w = l->src.w;
            l->dst.h = l->src.h;

        } else {
            l->addr[0] = (unsigned long) map_buf;
            l->src.x = draw_area->x1 - map_area->x1;
            l->src.y = draw_area->y1 - map_area->y1;
            l->src.w = lv_area_get_width(draw_area);
            l->src.h = lv_area_get_height(draw_area);

            l->dst.x = 0;
            l->dst.y = 0;
            l->dst.w = l->src.w;
            l->dst.h = l->src.h;
            l->src_stride[0] = map_width * 4;
        }
    }

    input.output.width = draw_width;
    input.output.height = draw_height;
    input.output.fmt = COLOR_ARGB8888;
    input.output.addr[0] = (unsigned long)(disp_buf+(draw_area->y1)*disp_width+(draw_area->x1));
    input.output.stride[0] = disp_width * 4;
    input.output.rotation = 0;
    hal_g2dlite_blend(G2D, &input);
    //LV_LOG_WARN("blend  %d\n", lv_tick_get() - start);
#else
    lv_color_t* disp_buf_first = lv_area_get_width(disp_area) * 4 * draw_area->y1 + 4 * draw_area->x1 + disp_buf;
    lv_color_t* map_buf_first = lv_area_get_width(map_area) * 4 * (draw_area->y1 - map_area->y1) + 4 * (draw_area->x1 - map_area->y1) * 4 + map_buf;
    for (int i = 0; i < input.layer_num; i++) {
        struct g2dlite_input_cfg  *l = &input.layer[i];
        l->layer_en = 1;
        l->layer = i;
        l->fmt = COLOR_ARGB8888;
        l->zorder = i;

        l->ckey_en = 0;
        l->blend = blend_mode_;
        l->alpha = opa;

        if (i == 0) {

            l->addr[0] = (unsigned long) disp_buf_first;
            l->src_x = 0;
            l->src_y = 0;
            l->src_w = lv_area_get_width(draw_area);
            l->src_h = lv_area_get_height(draw_area);

            l->dst_x = l->src_x;
            l->dst_y = l->src_y;
            l->dst_w = l->src_w;
            l->dst_h = l->src_h;
            l->src_stride[0] = lv_area_get_width(disp_area) * 4;
        } else {
            l->addr[0] = (unsigned long) map_buf_first;
            l->src_x = 0;
            l->src_y = 0;
            l->src_w = lv_area_get_width(draw_area);
            l->src_h = lv_area_get_height(draw_area);
            if ((lv_coord_t)l->src_w != lv_area_get_width(draw_area))
                LOGD("l->src_w --- map_w %d : %d\n", l->src_w, lv_area_get_width(draw_area));
            l->dst_x = l->src_x;
            l->dst_y = l->src_y;
            l->dst_w = l->src_w;
            l->dst_h = l->src_h;
            l->src_stride[0] = lv_area_get_width(map_area) * 4;
        }
    }

    input.output.width = lv_area_get_width(draw_area);
    input.output.height = lv_area_get_height(draw_area);
    input.output.fmt = COLOR_ARGB8888;
    input.output.addr[0] = (unsigned long)disp_buf_first;
    input.output.stride[0] = lv_area_get_width(disp_area)  * 4;
    input.output.rotation = 0;
    hal_g2dlite_blend(G2D, &input);
#endif
    #endif
    return 0;
}

int lv_gpu_sdrv_g2d_blend_nocache(lv_color_t * dst_buf, uint16_t dst_stride,
                            const lv_color_t * fg_buf, uint16_t fg_stride,
                            const lv_color_t * bg_buf, uint16_t bg_stride,
                            const lv_area_t * dst_area,
                            const lv_area_t * fg_src_area, const lv_area_t * fg_dst_area,
                            const lv_area_t * bg_src_area, const lv_area_t * bg_dst_area) {
    struct g2dlite_input input;
    memset(&input, 0, sizeof(struct g2dlite_input));
    input.layer_num = 2;
    uint32_t start  = lv_tick_get();

    for (int i = 0; i < input.layer_num; i++) {
        struct g2dlite_input_cfg  *l = &input.layer[i];
        l->layer_en = 1;
        l->layer = i;
        l->fmt = COLOR_ARGB8888;
        l->zorder = i;

        l->ckey.en = 0;
        l->blend = BLEND_PIXEL_COVERAGE;
        l->alpha = LV_OPA_COVER;

        if (i == 0) {
            l->addr[0] = (unsigned long) bg_buf;
            l->src.x = bg_src_area->x1;
            l->src.y = bg_src_area->y1;
            l->src.w = lv_area_get_width(bg_src_area);
            l->src.h = lv_area_get_height(bg_src_area);
            l->src_stride[0] = bg_stride;

            l->dst.x = bg_dst_area->x1;// canvas :output x y
            l->dst.y = bg_dst_area->y1;
            l->dst.w = lv_area_get_width(bg_dst_area);
            l->dst.h = lv_area_get_height(bg_dst_area);
        } else {
            l->addr[0] = (unsigned long) fg_buf;
            l->src.x = fg_src_area->x1;
            l->src.y = fg_src_area->y1;
            l->src.w = lv_area_get_width(fg_src_area);
            l->src.h = lv_area_get_height(fg_src_area);
            l->src_stride[0] = fg_stride;

            l->dst.x = fg_dst_area->x1;// canvas :output x y
            l->dst.y = fg_dst_area->y1;
            l->dst.w = lv_area_get_width(fg_dst_area);
            l->dst.h = lv_area_get_height(fg_dst_area);
        }
    }

    input.output.width = lv_area_get_width(dst_area);
    input.output.height = lv_area_get_height(dst_area);
    input.output.fmt = COLOR_ARGB8888;
    input.output.addr[0] = (unsigned long)(dst_buf);
    input.output.stride[0] = dst_stride;
    input.output.rotation = 0;
    hal_g2dlite_blend(G2D, &input);
    //LV_LOG_WARN("blend  %d\n", lv_tick_get() - start);
    return 0;
}

//layer 1 do not support scale !!!!!!!!!!!!!!!!!!!!!!!!!!!!
int lv_gpu_sdrv_g2d_blend_scale(lv_color_t * dst_buf, uint16_t dst_stride,
                            const lv_color_t * fg_buf, uint16_t fg_stride,
                            const lv_color_t * bg_buf, uint16_t bg_stride,
                            const lv_area_t * dst_area,
                            const lv_area_t * fg_src_area, const lv_area_t * fg_dst_area,
                            const lv_area_t * bg_src_area, const lv_area_t * bg_dst_area) {
    struct g2dlite_input input;
    memset(&input, 0, sizeof(struct g2dlite_input));
    input.layer_num = 2;
    uint32_t start  = lv_tick_get();

    for (int i = 0; i < input.layer_num; i++) {
        struct g2dlite_input_cfg  *l = &input.layer[i];
        l->layer_en = 1;
        l->layer = i;
        l->fmt = COLOR_ARGB8888;

        l->ckey.en = 0;
        l->blend = BLEND_PIXEL_COVERAGE;
        l->alpha = LV_OPA_COVER;

        //layer 1 do not support scale !!!!!!!!!!!!!!!!!!!!!!!!!!!!
        if (i == 1) {
            l->zorder = 0;
            l->addr[0] = (unsigned long) bg_buf;
            l->src.x = bg_src_area->x1;
            l->src.y = bg_src_area->y1;
            l->src.w = lv_area_get_width(bg_src_area);
            l->src.h = lv_area_get_height(bg_src_area);
            l->src_stride[0] = bg_stride;

            l->dst.x = bg_dst_area->x1;// canvas :output x y
            l->dst.y = bg_dst_area->y1;
            l->dst.w = lv_area_get_width(bg_dst_area);
            l->dst.h = lv_area_get_height(bg_dst_area);
        } else {
            l->zorder = 1;
            l->addr[0] = (unsigned long) fg_buf;
            l->src.x = fg_src_area->x1;
            l->src.y = fg_src_area->y1;
            l->src.w = lv_area_get_width(fg_src_area);
            l->src.h = lv_area_get_height(fg_src_area);
            l->src_stride[0] = fg_stride;

            l->dst.x = fg_dst_area->x1;// canvas :output x y
            l->dst.y = fg_dst_area->y1;
            l->dst.w = lv_area_get_width(fg_dst_area);
            l->dst.h = lv_area_get_height(fg_dst_area);
        }
    }

    input.output.width = lv_area_get_width(dst_area);
    input.output.height = lv_area_get_height(dst_area);
    input.output.fmt = COLOR_ARGB8888;
    input.output.addr[0] = (unsigned long)(dst_buf);
    input.output.stride[0] = dst_stride;
    input.output.rotation = 0;
    hal_g2dlite_blend(G2D, &input);
    //LV_LOG_WARN("blend  %d\n", lv_tick_get() - start);
    return 0;
}
// sharing fmt is COLOR_ABGR8888
int lv_gpu_sdrv_g2d_blend_sharing(lv_color_t * dst_buf, uint16_t dst_stride,
                            const lv_color_t * fg_buf, uint16_t fg_stride,
                            const lv_color_t * bg_buf, uint16_t bg_stride,
                            const lv_area_t * dst_area,
                            const lv_area_t * fg_src_area, const lv_area_t * fg_dst_area,
                            const lv_area_t * bg_src_area, const lv_area_t * bg_dst_area) {
    struct g2dlite_input input;
    memset(&input, 0, sizeof(struct g2dlite_input));
    input.layer_num = 2;
    uint32_t start  = lv_tick_get();

    for (int i = 0; i < input.layer_num; i++) {
        struct g2dlite_input_cfg  *l = &input.layer[i];
        l->layer_en = 1;
        l->layer = i;
        l->fmt = COLOR_ARGB8888;
        l->zorder = i;

        l->ckey.en = 0;
        l->blend = BLEND_PIXEL_COVERAGE;
        l->alpha = LV_OPA_COVER;

        if (i == 0) {
            l->addr[0] = (unsigned long) bg_buf;
            l->fmt = COLOR_ABGR8888;
            l->src.x = bg_src_area->x1;
            l->src.y = bg_src_area->y1;
            l->src.w = lv_area_get_width(bg_src_area);
            l->src.h = lv_area_get_height(bg_src_area);
            l->src_stride[0] = bg_stride;

            l->dst.x = bg_dst_area->x1;// canvas :output x y
            l->dst.y = bg_dst_area->y1;
            l->dst.w = lv_area_get_width(bg_dst_area);
            l->dst.h = lv_area_get_height(bg_dst_area);
        } else {
            l->addr[0] = (unsigned long) fg_buf;
            l->src.x = fg_src_area->x1;
            l->src.y = fg_src_area->y1;
            l->src.w = lv_area_get_width(fg_src_area);
            l->src.h = lv_area_get_height(fg_src_area);
            l->src_stride[0] = fg_stride;

            l->dst.x = fg_dst_area->x1;// canvas :output x y
            l->dst.y = fg_dst_area->y1;
            l->dst.w = lv_area_get_width(fg_dst_area);
            l->dst.h = lv_area_get_height(fg_dst_area);
        }
    }

    input.output.width = lv_area_get_width(dst_area);
    input.output.height = lv_area_get_height(dst_area);
    input.output.fmt = COLOR_ARGB8888;
    input.output.addr[0] = (unsigned long)(dst_buf);
    input.output.stride[0] = dst_stride;
    input.output.rotation = 0;
    hal_g2dlite_blend(G2D, &input);
    //LV_LOG_WARN("blend  %d\n", lv_tick_get() - start);
    return 0;
}

//layer 1 do not support scale !!!!!!!!!!!!!!!!!!!!!!!!!!!!
int lv_gpu_sdrv_g2d_blend_sharing_scale(lv_color_t * dst_buf, uint16_t dst_stride,
                            const lv_color_t * fg_buf, uint16_t fg_stride,
                            const lv_color_t * bg_buf, uint16_t bg_stride,
                            const lv_area_t * dst_area,
                            const lv_area_t * fg_src_area, const lv_area_t * fg_dst_area,
                            const lv_area_t * bg_src_area, const lv_area_t * bg_dst_area) {
    struct g2dlite_input input;
    memset(&input, 0, sizeof(struct g2dlite_input));
    input.layer_num = 2;
    uint32_t start  = lv_tick_get();

    for (int i = 0; i < input.layer_num; i++) {
        struct g2dlite_input_cfg  *l = &input.layer[i];
        l->layer_en = 1;
        l->layer = i;
        l->fmt = COLOR_ARGB8888;

        l->ckey.en = 0;
        l->blend = BLEND_PIXEL_COVERAGE;
        l->alpha = LV_OPA_COVER;

        //layer 1 do not support scale !!!!!!!!!!!!!!!!!!!!!!!!!!!!
        if (i == 1) {
            l->zorder = 0;
            l->addr[0] = (unsigned long) bg_buf;
            l->fmt = COLOR_ABGR8888;
            l->src.x = bg_src_area->x1;
            l->src.y = bg_src_area->y1;
            l->src.w = lv_area_get_width(bg_src_area);
            l->src.h = lv_area_get_height(bg_src_area);
            l->src_stride[0] = bg_stride;

            l->dst.x = bg_dst_area->x1;// canvas :output x y
            l->dst.y = bg_dst_area->y1;
            l->dst.w = lv_area_get_width(bg_dst_area);
            l->dst.h = lv_area_get_height(bg_dst_area);
        } else {
            l->zorder = 1;
            l->addr[0] = (unsigned long) fg_buf;
            l->src.x = fg_src_area->x1;
            l->src.y = fg_src_area->y1;
            l->src.w = lv_area_get_width(fg_src_area);
            l->src.h = lv_area_get_height(fg_src_area);
            l->src_stride[0] = fg_stride;

            l->dst.x = fg_dst_area->x1;// canvas :output x y
            l->dst.y = fg_dst_area->y1;
            l->dst.w = lv_area_get_width(fg_dst_area);
            l->dst.h = lv_area_get_height(fg_dst_area);
        }
    }

    input.output.width = lv_area_get_width(dst_area);
    input.output.height = lv_area_get_height(dst_area);
    input.output.fmt = COLOR_ARGB8888;
    input.output.addr[0] = (unsigned long)(dst_buf);
    input.output.stride[0] = dst_stride;
    input.output.rotation = 0;
    hal_g2dlite_blend(G2D, &input);
    //LV_LOG_WARN("blend  %d\n", lv_tick_get() - start);
    return 0;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

static void invalidate_cache(void* color_p, size_t size)
{
    arch_clean_invalidate_cache_range((addr_t)color_p, size);
}

static void dma2d_wait(struct dma_desc *desc)
{
    // lv_disp_t * disp = _lv_refr_get_disp_refreshing();
    int ret = hal_dma_sync_wait(desc, DMA_SYNC_TIMEOUT);
    if (ret)
    LV_LOG_ERROR("dma2d_wait timeout: %d\n", ret);
}

#endif
