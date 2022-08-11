#include "stdlib.h"
#include "stdio.h"
#include <debug.h>
#include "g2dlite_api.h"
#include "heap.h"
#if defined (__GNUC__)
    #include "malloc.h"
#elif defined (__ICCARM__)
    #include "heap.h"
#else
    #error Unknown Compiler!
#endif
#include "disp_data_type.h"
#include "sdm_display.h"
#include "string.h"

#include "ba_config.h"
#include "player_g2d.h"

#include "lv_gpu_sdrv_dma2d.h"


fmt_yuv420p_t* dstout;
fmt_yuv420p_t srcin;


void* g_handle = NULL;
struct g2dlite_input input;

bool setG2dHandle(void)
{
    bool ret = false;

    ret = hal_g2dlite_creat_handle(&g_handle,RES_G2D_G2D2);
    if(!ret){
        LOGD("failed to create g2dlite\r\n");
        return ret;
    }
    hal_g2dlite_init(g_handle);

    return ret;
}

void _alloc_g2dm(fmt_yuv420p_t* dst)
{
    uint32_t dstw = dst->rect.w;
    uint32_t dsth = dst->rect.h;

    uint32_t ysize = dstw * dsth;
    uint32_t csize = (dstw*dsth / 4);
    dst->bufY = memalign(1024,ROUNDUP(ysize,1024));
    dst->bufCb = memalign(256,ROUNDUP(csize,256) );
    dst->bufCr =memalign(256,ROUNDUP(csize,256));

    memset(dst->bufY, 0, ROUNDUP(ysize,1024));
    arch_clean_cache_range((addr_t)dst->bufY, ROUNDUP(ysize,1024));
    memset(dst->bufCb, 0, ROUNDUP(csize,256));
    arch_clean_cache_range((addr_t)dst->bufCb, ROUNDUP(csize,256));
    memset(dst->bufCr, 0, ROUNDUP(csize,256));
    arch_clean_cache_range((addr_t)dst->bufCr, ROUNDUP(csize,256));

}
void free_g2dm(fmt_yuv420p_t* dst)
{
    free(dst->bufY);
    free(dst->bufCb);
    free(dst->bufCr);
    dst->bufY = NULL;
    dst->bufCb = NULL;
    dst->bufCr = NULL;

}


void scale_g2d(fmt_yuv420p_t src,fmt_yuv420p_t* dst,uint32_t stride)
{
#ifdef USE_ARGB888
    // USE ARGB8888
    uint32_t ysize = dst->rect.w * dst->rect.h;
    dst->bufY = memalign(1024,ROUNDUP(ysize * 4,1024));
    dst->bufCb = NULL;
    dst->bufCr = NULL;
    (void)_alloc_g2dm;
#else
    _alloc_g2dm(dst);
#endif

    void* bufY = dst->bufY;
    void* bufCb = dst->bufCb;
    void* bufCr = dst->bufCr;

    uint32_t srcw = src.rect.w;
    uint32_t srch = src.rect.h;
    dst->rect.x = 0;
    dst->rect.y = 0;

    memset(&input,0,sizeof(struct g2dlite_input));

    input.layer_num = 1;
    for (int i = 0; i < input.layer_num; i++) {
        input.layer[i].layer = i;
        input.layer[i].layer_en = 1;

        input.layer[i].fmt = COLOR_YUV420P;
        input.layer[i].zorder = i;
        memcpy(&(input.layer[i].src),&src.rect,sizeof(prect_t));
        input.layer[i].addr[0] = (unsigned long)src.bufY;
        input.layer[i].src_stride[0] = stride;
        input.layer[i].addr[1] = (unsigned long)src.bufCb;
        input.layer[i].src_stride[1] = stride/2;
        input.layer[i].addr[2] = (unsigned long)src.bufCr;
        input.layer[i].src_stride[2] = stride /2;

        memcpy(&(input.layer[i].dst),&(dst->rect),sizeof(prect_t));

        input.layer[i].ckey.en = 0;
        input.layer[i].blend = BLEND_PIXEL_NONE;
        input.layer[i].alpha = 0xFF;
    }

    input.output.width = dst->rect.w;
    input.output.height = dst->rect.h;
#ifdef USE_ARGB888
    input.output.fmt = COLOR_ARGB8888;
    input.output.stride[0] = dst->rect.w * 4;
#else
    input.output.fmt = COLOR_YUV420P;
    input.output.stride[0] = dst->rect.w;
#endif
    input.output.addr[0] = (unsigned long)bufY;

    input.output.addr[1] = (unsigned long)bufCb;
    input.output.stride[1] = dst->rect.w/2;
    input.output.addr[2] = (unsigned long)bufCr;
    input.output.stride[2] = dst->rect.w/2;
    input.output.rotation = 0;

    hal_g2dlite_blend(&g_handle, &input);
}
