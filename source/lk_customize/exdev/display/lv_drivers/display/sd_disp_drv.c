/*
* lvgl_disp_drv.c
*
* Copyright (c) 2019 Semidrive Semiconductor.
* All rights reserved.
*
* Description:
*
* Revision History:
* -----------------
* 011, 07/13/2019 BI create this file
*/
#include <debug.h>
#if defined (__GNUC__)
    #include <malloc.h>
#elif defined (__ICCARM__)
    #include "heap.h"
#else
    #error Unknown Compiler!
#endif

#include "lvgl_disp_config.h"
#include "disp_data_type.h"
#include <sdm_display.h>
#if WITH_KERNEL_VM
#include <kernel/vm.h>
#endif
#include <kernel/spinlock.h>
#include <ext_data.h>
#include <g2dlite_api.h>
#include <heap.h>
#include <kernel/thread.h>
#include <platform.h>
#define SCREEN_MAX 3

#define LOGO_LAYER         0
#define BOOT_MENU_LAYER    1

#if WITH_KERNEL_VM
#define v2p(va)    (paddr_t)(vaddr_to_paddr(va))
#define p2v(pa)    (vaddr_t)(paddr_to_kvaddr(pa))
#else
#define v2p(va)    (paddr_t)(va)
#define p2v(pa)    (vaddr_t)(pa)
#endif
/*struct disp_xxx_data {
    unsigned char panel_index;
};*/

unsigned int sd_get_color_format(void)
{
    unsigned int format;
#if LV_COLOR_DEPTH == 16
    format = COLOR_RGB565;
#elif LV_COLOR_DEPTH == 32
    format = COLOR_ARGB8888;
#else
#error "Invalid LV_COLOR_DEPTH in lv_conf.h! Set it to 16 or 32!"
#endif

    return format;
}

unsigned char sd_get_bpp_by_format(unsigned int format)
{
    unsigned char bpp = 0;

    switch (format) {
        case COLOR_RGB565:
            bpp = 2;
            break;
        case COLOR_ARGB8888:
            bpp = 4;
            break;
        default:
            LOGD("Not support this format\n");
            break;
    }
    return bpp;
}

int area_valid_check(const lv_area_t * area)
{
    int lcd_w, lcd_h;

    lcd_w = lv_area_get_width(area);
    lcd_h = lv_area_get_height(area);

    LOGD("lcd_w = %d, lcd_h = %d\n", lcd_w, lcd_h);
    LOGD("area x1 = %d, area y1 = %d\n", area->x1, area->y1);
    LOGD("area x2 = %d, area y2 = %d\n", area->x2, area->y2);
    if ((area->x2 > lcd_w) || (area->y2 > lcd_h)) {
        LOGD("area size is over lcd size\n");
        return -1;
    }

    return 0;
}

int sd_disp_post(struct _disp_drv_t * disp_drv, const lv_area_t * area,
                lv_color_t *buf_bottom, lv_color_t *buf_top,
                uint32_t length, lv_opa_t opa) {
    struct sdm_post_config post;
    int i;
    int ret;
    int n_bufs;
    lv_color_t *bufs[] = {buf_bottom, buf_top};
    struct disp_data *data = (struct disp_data*)disp_drv->user_data;
    sdm_display_t *sdm = data->sdm;

	if (!sdm) {
		LOGE("display handle is invalid\n");
		return -1;
	}

    // LOGD("disp flush... sdm: %d, %d -- %p , reserved bufs: %p, %p\n",
    //     sdm->id, sdm->handle->display_id, buf_bottom, disp_drv->buffer->buf1, disp_drv->buffer->buf2);
    memset(&post, 0, sizeof(struct sdm_post_config));
    if (!buf_bottom) {
        LOGE("Error: buf1 is null\n");
        return -3;
    }
    n_bufs = 1;
    if (buf_top) {
        n_bufs ++;
    }
    post.bufs = (struct sdm_buffer*) malloc(sizeof(struct sdm_buffer) * n_bufs);
    if (!post.bufs) {
        LOGE("Error: malloc sdm_buffer failed\n");
        return -2;
    }

    post.n_bufs = n_bufs;
    for (i = 0; i < n_bufs; i++) {
        struct sdm_buffer *buf = &post.bufs[i];
        buf->addr[0] = (unsigned long)v2p((void *)bufs[i]);
        if (i == 1)
            buf->alpha = opa;
        else
            buf->alpha = 0xff;
        buf->layer = 1;
        buf->alpha_en = 0;// for bg transp 1->0
        buf->ckey = 0;
        buf->ckey_en = 0;
        buf->fmt = sd_get_color_format();
        buf->layer_en = 1;
        buf->src.x = area->x1;
        buf->src.y = area->y1;
        buf->src.w = lv_area_get_width(area);
        buf->src.h = lv_area_get_height(area);

        buf->start.x = area->x1;
        buf->start.y = area->y1;
        buf->start.w = lv_area_get_width(area);
        buf->start.h = lv_area_get_height(area);
        buf->src_stride[0] = lv_area_get_width(area) * sd_get_bpp_by_format(buf->fmt);

        // dc do not support scaling.
        buf->dst.x = area->x1;
        buf->dst.y = area->y1;
        buf->dst.w = lv_area_get_width(area);
        buf->dst.h = lv_area_get_height(area);
        buf->z_order = 1;
    }

    ret = sdm_post(sdm->handle, &post);
    if (ret) {
        LOGD("post failed: %d\n", ret);
    }
    free(post.bufs);
    return 0;
}

void sd_gpu_blend(lv_disp_drv_t * disp_drv, lv_color_t * dest, const lv_color_t * src, uint32_t length, lv_opa_t opa) {
    struct disp_data *data = (struct disp_data*)disp_drv->user_data;
    struct g2dlite_input input;
    LOGD("gpu blend......: length %d\n", length);

    memset(&input, 0, sizeof(struct g2dlite_input));
    input.layer_num = 2;
    #if LV_COLOR_DEPTH == 32
    for (int i = 0; i < input.layer_num; i++) {
        input.layer[i].layer = i;
        input.layer[i].layer_en = 1;

        input.layer[i].fmt = COLOR_ARGB8888;
        input.layer[i].zorder = i;
        input.layer[i].src.x = 0;
        input.layer[i].src.y = 0;
        input.layer[i].src.w = length;
        input.layer[i].src.h = 1;

        input.layer[i].addr[0] = i == 0? (unsigned long)dest: (unsigned long)src;

        input.layer[i].src_stride[0] = length * 4;
        input.layer[i].dst.x = 0;
        input.layer[i].dst.y = 0;
        input.layer[i].dst.w = length;
        input.layer[i].dst.h = 1;
        input.layer[i].ckey.en = 0;
        input.layer[i].blend = BLEND_PIXEL_COVERAGE;
        input.layer[i].alpha = opa;
    }

    input.output.width = length;
    input.output.height = 1;
    input.output.fmt = COLOR_ARGB8888;
    input.output.addr[0] = (unsigned long)dest;
    input.output.stride[0] = length * 4;
    input.output.rotation = 0;
    hal_g2dlite_blend(data->g2d, &input);
    #endif

}

void disp_flush(struct _disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p)
{
#if LV_USE_USER_DATA
//    struct disp_data *data = (struct disp_data*)disp_drv->user_data;
#endif

    // arch_clean_cache_range((addr_t)color_p, disp_drv->hor_res * 4);
    arch_clean_invalidate_cache_range((addr_t)color_p, disp_drv->hor_res * disp_drv->ver_res * 4);

    sd_disp_post(disp_drv, area, color_p, NULL, 0, 0xff);

    lv_disp_flush_ready(disp_drv);
}

/** OPTIONAL: Called after every refresh cycle to tell the rendering and flushing time + the
* number of flushed pixels */
static void sd_monitor_cb(struct _disp_drv_t * disp_drv, uint32_t time, uint32_t px)
 {
    if (time % 1000 == 5)
        LOGD("time: %d, time %d\n", lv_tick_get(), time);
 }

#if LV_USE_GPU
/* If your MCU has hardware accelerator (GPU) then you can use it to blend to memories using opacity
 *  * It can be used only in buffered mode (LV_VDB_SIZE != 0 in lv_conf.h)*/
static void gpu_blend(lv_disp_drv_t * disp_drv, lv_color_t * dest, const lv_color_t * src, uint32_t length, lv_opa_t opa)
{
    /*It's an example code which should be done by your GPU*/
#if 0
    uint32_t i;
    // LOGD("gpu blend......: length %d\n", length);
    for(i = 0; i < length; i++) {
        dest[i] = lv_color_mix(dest[i], src[i], opa);
    }
    arch_clean_cache_range((addr_t)dest, disp_drv->hor_res * 4);
#else
    sd_gpu_blend(disp_drv, dest, src, length, opa);
#endif
}

/* If your MCU has hardware accelerator (GPU) then you can use it to fill a memory with a color
 *  * It can be used only in buffered mode (LV_VDB_SIZE != 0 in lv_conf.h)*/
static void gpu_fill_cb(lv_disp_drv_t * disp_drv, lv_color_t * dest_buf, lv_coord_t dest_width,
                        const lv_area_t * fill_area, lv_color_t color)
{
    /*It's an example code which should be done by your GPU*/
    lv_coord_t x, y;
    // LOGD("gpu fill.....(%d, %d, %d, %d).\n", fill_area->x1, fill_area->y1, fill_area->x2, fill_area->y2);
    lv_color_t *p = dest_buf;
    dest_buf += dest_width * fill_area->y1; /*Go to the first line*/
    // arch_clean_cache_range((addr_t)dest_buf, dest_width * (fill_area->y2 - fill_area->y1) * 4);
    for(y = fill_area->y1; y < fill_area->y2; y++) {
        for(x = fill_area->x1; x < fill_area->x2; x++) {
            dest_buf[x] = color;
            // dest_buf[x] = lv_color_mix(LV_COLOR_ORANGE, color , LV_OPA_100);
        }
        dest_buf+=dest_width;    /*Go to the next line*/
    }

}

#endif

struct g2dlite {
    int index;
    addr_t reg_addr;
    uint32_t irq_num;
};

lv_disp_t *lvgl_lcd_display_init(sdm_display_t *sdm)
{
    int ret;
    struct disp_data *data;
    lv_color_t *buf3_1 = memalign(0x1000, sdm->handle->info.width * sdm->handle->info.height * LV_COLOR_DEPTH / 8);
    memset(buf3_1, 0, sdm->handle->info.width * sdm->handle->info.height * LV_COLOR_DEPTH / 8);
    lv_color_t *buf3_2 = memalign(0x1000, sdm->handle->info.width * sdm->handle->info.height * LV_COLOR_DEPTH / 8);
    memset(buf3_2, 0, sdm->handle->info.width * sdm->handle->info.height * LV_COLOR_DEPTH / 8);

    lv_disp_buf_t *disp_buf_3 = malloc(sizeof(lv_disp_buf_t));

    lv_disp_buf_init(disp_buf_3, buf3_1, buf3_2, sdm->handle->info.width * sdm->handle->info.height);   /*Initialize the display buffer*/
    LOGD("register buffers: %p ,%p for disp %d\n", buf3_1, buf3_2, sdm->id);
    lv_disp_drv_t disp_drv;                         /*Descriptor of a display driver*/
    lv_disp_drv_init(&disp_drv);                    /*Basic initialization*/

    data = (struct disp_data *) malloc(sizeof(struct disp_data));
    if (!data) {
        LOGD("create disp_data failed\n");
    }
    /*Set up the functions to access to your display*/

    /*Set the resolution of the display*/
    disp_drv.hor_res = sdm->handle->info.width;
    disp_drv.ver_res = sdm->handle->info.height;

    /*Used to copy the buffer's content to the display*/
    disp_drv.flush_cb = disp_flush;
    disp_drv.monitor_cb = sd_monitor_cb;

    /*Set a display buffer*/
    disp_drv.buffer = disp_buf_3;
    data->sdm = sdm;
    ret = hal_g2dlite_creat_handle(&data->g2d, RES_G2D_G2D2);
    if (!ret) {
        LOGD("g2dlite creat handle failed\n");
    }
    hal_g2dlite_init(data->g2d);
    disp_drv.user_data = data;

#if LV_USE_GPU
    /*Optionally add functions to access the GPU. (Only in buffered mode, LV_VDB_SIZE != 0)*/

    /*Blend two color array using opacity*/
    disp_drv.gpu_blend_cb = gpu_blend;

    /*Fill a memory array with a color*/
    disp_drv.gpu_fill_cb = gpu_fill_cb;
#endif
    /*Finally register the driver*/
    return lv_disp_drv_register(&disp_drv);
}


