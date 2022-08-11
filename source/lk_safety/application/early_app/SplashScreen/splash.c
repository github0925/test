#include <sdm_display.h>
#include <heap.h>
#include <early_app_cfg.h>
#include <res_loader.h>
#include <macros.h>
#include <disp_data_type.h>
#include <assert.h>
#include "early_app_common.h"


void *memset(void *s, int c, size_t count);

#define SPLASH_PIC_PATH     "early_app/SplashScreen/splash.rgb"

#define DISPLAY_BUF_TEMPLATE { \
    1,/*layer*/\
    0,/*layer_dirty*/\
    1,/*layer_en*/\
    COLOR_RGB565,/*fmt*/\
    {0,0,0,0},/*x,y,w,h src */ \
    {0x0,0x0,0x0,0x0},/*y,u,v,a*/ \
    {0,0,0,0},/*stride*/ \
    {0,0,0,0},/*start*/ \
    {0,0,0,0},/*dest*/ \
    0,/*ckey_en*/\
    0,/*ckey*/\
    1,/*alpha_en*/\
    0xff,/*alpha*/\
    1,/*z-order*/ \
    0/*security*/\
}

static void splash_fill_rgb_sdm_buf(struct sdm_buffer* sdm_buffer,unsigned long buf_start, unsigned long stride)
{
    sdm_buffer->addr[0] = buf_start;
    sdm_buffer->src_stride[0] = stride;

}

static void splash_crop_sdm_buf(struct sdm_buffer* sdm_buffer,int px,int py, int width, int height)
{
    sdm_buffer->start.x = px;
    sdm_buffer->start.y = py;
    sdm_buffer->start.w = width;
    sdm_buffer->start.h = height;
    sdm_buffer->src.x = px;
    sdm_buffer->src.y = py;
    sdm_buffer->src.w = width;
    sdm_buffer->src.h = height;
}

void splash_map_sdm_buf(struct sdm_buffer* sdm_buffer,int px,int py, int width, int height)
{
    sdm_buffer->dst.x = px;
    sdm_buffer->dst.y = py;
    sdm_buffer->dst.w = width;
    sdm_buffer->dst.h = height;
}

void* splash_load_pic(void)
{
    void* pic = memalign(32,ROUNDUP(res_size(SPLASH_PIC_PATH),32));
    ASSERT(pic);
    res_load(SPLASH_PIC_PATH,pic,ROUNDUP(res_size(SPLASH_PIC_PATH),32),0);
    return pic;
}

void splash_eliminate_pic(void* pic)
{
    free(pic);
}


void splash_screen(void* pic, bool on)
{
    ASSERT(pic);

    uint32_t i = 0;
    sdm_display_t* disp;


    uint32_t pw = 0;
    uint32_t ph = 0;

    struct sdm_post_config post_data;
    memset(&post_data,0,sizeof(struct sdm_post_config));

    struct sdm_buffer sdm_buf = DISPLAY_BUF_TEMPLATE;

    sdm_buf.layer_en = 1;
    post_data.bufs = &sdm_buf;
    post_data.n_bufs = 1;
    post_data.custom_data = NULL;
    post_data.custom_data_size = 0;
    splash_fill_rgb_sdm_buf(&sdm_buf,(unsigned long)pic,SC_PIC_WIDTH*2);//rgb565 - bpp = 2


    for(i = 0; i<SCREEN_MAX; i++){
        disp = get_disp_handle(i);
        if(disp)
        {
            get_screen_attr(disp,&pw, &ph);
            splash_map_sdm_buf(&sdm_buf,0,0,pw,ph);
            splash_crop_sdm_buf(&sdm_buf,(SC_PIC_WIDTH - pw)/2,(SC_PIC_HEIGHT - ph)/2,pw,ph);
            if(on)
            {
                sdm_post(disp->handle,&post_data);
            }
            else
            {
                sdm_clear_display(disp->handle);
            }
        }


    }

}


