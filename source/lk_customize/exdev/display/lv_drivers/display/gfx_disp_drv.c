
#include "lvgl_disp_config.h"

#include <malloc.h>
#include <lib/gfx.h>
#include <debug.h>
#include <dev/display.h>

#define SCREEN_MAX 3
struct display_framebuffer fbs[SCREEN_MAX];

gfx_surface *surfs[SCREEN_MAX] = {NULL, NULL};


static void disp_flush(lv_disp_drv_t * disp_drv, const lv_area_t * area,
                       lv_color_t * color_p);
static void gpu_fill_cb(lv_disp_drv_t * disp_drv, lv_color_t * dest_buf,
                        lv_coord_t dest_width, const lv_area_t * fill_area,
                        lv_color_t color);
static void gpu_blend(lv_disp_drv_t * disp_drv, lv_color_t * dest,
                      const lv_color_t * src, uint32_t length, lv_opa_t opa);

static gfx_surface *ui_gfx_init(u32 id, struct display_framebuffer *fb)
{
    status_t rc = 0;
    gfx_surface *surf = NULL;
#if WITH_MULTI_DISPLAYS
    rc = display_get_framebuffer_by_id(id,fb);
    if ( rc < 0) {
        goto EXIT;
    }
#else
    rc = display_get_framebuffer(fb);
    if (rc < 0) {
        goto EXIT;
    }
#endif
    surf = gfx_create_surface_from_display(fb);

EXIT:
    return surf;
}

/* Initialize your display and the required peripherals. */
static void disp_init(void)
{

    int i;
    for (i=0;i < 2; i++) {
        LOGD("ui_gfx_init init\n");
        surfs[i] = ui_gfx_init(i, &fbs[i]);
    }
}


lv_disp_t *lvgl_lcd_display_init(sdm_display_t *sdm)
{
    disp_init();
    /* Example for 3) */
    static lv_disp_buf_t disp_buf_3;
    static lv_color_t buf3_1[LV_HOR_RES_MAX * LV_VER_RES_MAX];            /*A screen sized buffer*/
    static lv_color_t buf3_2[LV_HOR_RES_MAX * LV_VER_RES_MAX];            /*An other screen sized buffer*/
    //static lv_color_t *buf3_2 = NULL;
    LOGD("buf1: 0x%lx, buf2: 0x%lx\n", (long)buf3_1, (long)buf3_2);
    lv_disp_buf_init(&disp_buf_3, buf3_1, buf3_2, LV_HOR_RES_MAX * LV_VER_RES_MAX);   /*Initialize the display buffer*/


    lv_disp_drv_t disp_drv;                         /*Descriptor of a display driver*/
    lv_disp_drv_init(&disp_drv);                    /*Basic initialization*/

    /*Set up the functions to access to your display*/

    /*Set the resolution of the display*/
    disp_drv.hor_res = sdm->handle->info.width;
    disp_drv.ver_res = sdm->handle->info.height;
    disp_drv.user_data = sdm;
    /*Used to copy the buffer's content to the display*/
    disp_drv.flush_cb = disp_flush;

    /*Set a display buffer*/
    disp_drv.buffer = &disp_buf_3;

#if LV_USE_GPU
    /*Optionally add functions to access the GPU. (Only in buffered mode, LV_VDB_SIZE != 0)*/

    /*Blend two color array using opacity*/
    disp_drv.gpu_blend_cb = gpu_blend;

    /*Fill a memory array with a color*/
    disp_drv.gpu_fill_cb = gpu_fill_cb;
#endif
    disp_drv.user_data = surfs[0];
    /*Finally register the driver*/
    return lv_disp_drv_register(&disp_drv);

}

/* Flush the content of the internal buffer the specific area on the display
 *  * You can use DMA or any hardware acceleration to do this operation in the background but
 *   * 'lv_disp_flush_ready()' has to be called when finished. */
static void disp_flush(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p)
{
    /*The most simple case (but also the slowest) to put all pixels to the screen one-by-one*/
    //lv_disp_t * disp = lv_disp_get_default();
    gfx_surface *surface = surfs[0];
    #if 0
    int32_t x;
    int32_t y;
    lv_color_t *p = color_p;

    // disp->driver->user_data


    for(y = area->y1; y <= area->y2; y++) {
        for(x = area->x1; x <= area->x2; x++) {
            /* Put a pixel to the display. For example: */
            /* put_px(x, y, *color_p)*/
            gfx_putpixel(surface, x, y, p->full);
            p++;
        }
    }
    #endif
    memcpy(surface->ptr, color_p, (area->x2 - area->x1+1) * (area->y2 - area->y1 +1) * 4);

    gfx_flush(surface);
    /* IMPORTANT!!!
     *      * Inform the graphics library that you are ready with the flushing*/
    lv_disp_flush_ready(disp_drv);
}

/*OPTIONAL: GPU INTERFACE*/
#if LV_USE_GPU

/* If your MCU has hardware accelerator (GPU) then you can use it to blend to memories using opacity
 *  * It can be used only in buffered mode (LV_VDB_SIZE != 0 in lv_conf.h)*/
static void gpu_blend(lv_disp_drv_t * disp_drv, lv_color_t * dest, const lv_color_t * src, uint32_t length, lv_opa_t opa)
{
    /*It's an example code which should be done by your GPU*/

    uint32_t i;
    for(i = 0; i < length; i++) {
        dest[i] = lv_color_mix(dest[i], src[i], opa);
    }
}

/* If your MCU has hardware accelerator (GPU) then you can use it to fill a memory with a color
 *  * It can be used only in buffered mode (LV_VDB_SIZE != 0 in lv_conf.h)*/
static void gpu_fill_cb(lv_disp_drv_t * disp_drv, lv_color_t * dest_buf, lv_coord_t dest_width,
                        const lv_area_t * fill_area, lv_color_t color)
{
    /*It's an example code which should be done by your GPU*/
    lv_coord_t x, y;
    dest_buf += dest_width * fill_area->y1; /*Go to the first line*/

    for(y = fill_area->y1; y < fill_area->y2; y++) {
        for(x = fill_area->x1; x < fill_area->x2; x++) {
            dest_buf[x] = color;
        }
        dest_buf+=dest_width;    /*Go to the next line*/
    }
}

#endif
