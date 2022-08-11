#include <app.h>
#include <lk_wrapper.h>
#include "sdm_display.h"
#include <disp_data_type.h>
#include "data_structure_def.h"
#include "container.h"
#include "animation_config.h"
#include "dcf.h"
#include "early_app_common.h"
#include "early_app_cfg.h"
#include "ba_config.h"
#include "player_g2d.h"
#include <g2dlite_api.h>
#include "ba_uart.h"
#include <uart_hal.h>


struct list_node *sdm_get_display_list(void);

void *memset(void *s, int c, size_t count);

#ifdef ENABLE_AVM_FIRST
volatile uint32_t iAvmState = 0;
#endif

#if defined (ENABLE_BA_UART_A) || defined (ENABLE_BA_UART_B)
void* ba_uart_handle = NULL;
static char ba_txdata = 'A';
#endif

uint32_t getReplaySts(void);

#define MAX_DISPLAY_NUMBER 3
#define DISPLAY_BUF_TEMPLATE { \
    1,/*layer*/\
    0,/*layer_dirty*/\
    1,/*layer_en*/\
    COLOR_YUV420P,/*fmt*/\
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

static uint32_t skipSdm(uint32_t id_screen)
{
    uint32_t status = 0;

    if(id_screen == CONTROLPANEL){
        if(getReplaySts())
            status = 1;
        else{
#ifdef ENABLE_AVM_FIRST
        if(iAvmState == ussReady)
            status = 1;
#endif
        }
    }

    if(id_screen == INFOTAINMENT){
        if(getReplaySts())
            status = 0;
        else{
#ifdef ENABLE_AVM_FIRST
        if(iAvmState == ussReady)
            status = 1;
#endif
        }
    }

    return status;
}

static enum handler_return fps_tmr_cb(struct timer *t, lk_time_t now, void *arg)
{
    event_t* evt = arg;
    event_signal(evt,true);

    return 0;
}
#ifdef ENABLE_AVM_FIRST
void avm_state_cb(void* pargs, int uargs,int singal)
{
    iAvmState = ussReady;
}
#endif

void fill_sdm_buf(struct sdm_buffer* sdm_buffer,unsigned long bufY, unsigned long bufCB, unsigned long bufCR, int stride)
{
    sdm_buffer->addr[0] = bufY;
    sdm_buffer->addr[1] = bufCB;
    sdm_buffer->addr[2] = bufCR;
    sdm_buffer->src_stride[0] = stride;
    sdm_buffer->src_stride[1] = (stride / 2);
    sdm_buffer->src_stride[2] = (stride / 2);
}

void crop_sdm_buf(struct sdm_buffer* sdm_buffer,int px,int py, int width, int height)
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

void map_sdm_buf(struct sdm_buffer* sdm_buffer,int px,int py, int width, int height)
{
    sdm_buffer->dst.x = px;
    sdm_buffer->dst.y = py;
    sdm_buffer->dst.w = width;
    sdm_buffer->dst.h = height;
}

extern fmt_yuv420p_t* dstout;
extern fmt_yuv420p_t srcin;


static bool _chkClusterstatus(void)
{
    int val = 0;
    system_property_get(DMP_ID_CLUSTER_STATUS,&val);
    if((val >> 16) == 0x01)
        return true;
    return false;
}

static void _set_post(struct sdm_post_config* dst,struct sdm_buffer* srcbuf)
{
    dst->bufs = srcbuf;
    dst->n_bufs = 1;
    dst->custom_data = NULL;
    dst->custom_data_size = 0;
}
static void _clear_display(void)
{
    for(uint32_t i = 0; i<SCREEN_MAX; i++)
    {
        sdm_display_t* d = get_disp_handle(i);
        if(d)
        {
            if(skipSdm(i) == 0)
                sdm_clear_display(d->handle);
        }
    }
}

void player_task(token_handle_t token)
{

    disp_req_t disp_req;
    timer_t fps_timer;
    event_t fps_tmr_cb_signal;
    container_handle_t  container;
    uint32_t ts = 0;
    uint32_t frames = 0;

    sdm_display_t* disp = NULL;
    uint32_t i = 0;
    uint32_t disp_num =  list_length(sdm_get_display_list());

    uint32_t rescount = getsrcCnt();
    uint32_t dstcount = MIN(disp_num,getdstCnt());
    bool animal_flag = false;
    uint32_t id_bufab = 0;
    uint32_t id_post = 0;
    uint32_t id_disp = 0;
    uint32_t id_src = 0;
    fmt_yuv420p_t dstoutAB[2*disp_num];
    uint32_t id_mismatch =0;
    bool bgetmis = false;
#if defined (ENABLE_BA_UART_A) || defined (ENABLE_BA_UART_B)
    hal_uart_cfg_t uartcfg;
    bool btx = false;
#endif

    dstout = malloc(sizeof(fmt_yuv420p_t));
    memset(dstout,0, sizeof(fmt_yuv420p_t));
    memset(dstoutAB,0, sizeof(fmt_yuv420p_t)*2*disp_num);
    setG2dHandle();

    event_init(&fps_tmr_cb_signal,false,EVENT_FLAG_AUTOUNSIGNAL);
    timer_initialize(&fps_timer);
    timer_set_periodic(&fps_timer,1000/ANIMATION_FPS,fps_tmr_cb,&fps_tmr_cb_signal);

    struct sdm_post_config post_data;
    memset(&post_data,0,sizeof(struct sdm_post_config));

    struct sdm_buffer sdm_buf = DISPLAY_BUF_TEMPLATE;

    USDBG("go into player task\n");
    _set_post(&post_data,&sdm_buf);

#if defined (ENABLE_BA_UART_A) || defined (ENABLE_BA_UART_B)
    uart_config(&ba_uart_handle,uartcfg);
#endif
    ts = current_time();

#ifdef ENABLE_BA_UART_B
    sideb_rx_char(token,ba_uart_handle,ba_txdata);
#endif

#ifdef ENABLE_AVM_FIRST
    if(getReplaySts() == 0)
        unified_service_subscribe(DMP_ID_AVM_STATUS,ussReady,
                            avm_state_cb,NULL,0);
#endif
    while(1)
    {
        if(token_getstatus(token) == TOKEN_ABNORMAL)
        {
            post_data.bufs->layer_en = 0;
            _clear_display();
            printf("player_token is destroyed unexpectedly!\r\n");
            break;
        }

        container_take(token,&container,true);

        container_carrydown(container,&disp_req,sizeof(disp_req));

        if(disp_req.end)
        {
            USDBG("No frame to display, finish.\n");
            post_data.bufs->layer_en = 0;
            _clear_display();

            break;
        }
        id_src = id_disp = 0;

        for(i = 0; i<SCREEN_MAX; i++){
            disp = get_disp_handle(i);
            if(disp)
            {
                if(i == CLUSTER && _chkClusterstatus() && !getReplaySts())
                {
//linux cluster clear
#ifndef ENABLE_CLUSTER
                if(!animal_flag) {
                    sdm_clear_display(disp->handle);
                    animal_flag = true;
                }
#endif
                    // tmp set layer_en 0
                    post_data.bufs->layer_en = 0;
                    // should reset to 1
                    post_data.bufs->layer_en = 1;
                }
                else if(skipSdm(i) == 1)
                        ;
                else
                {
                    if(g_dstdisp[id_disp].w == g_srcres[id_src].w &&
                    g_dstdisp[id_disp].h == g_srcres[id_src].h){
                        /* received vpu data */
#ifdef USE_ARGB888
                        sdm_buf.fmt = COLOR_YUV420P;
#endif
                        fill_sdm_buf(&sdm_buf,disp_req.bufY,disp_req.bufCb,disp_req.bufCr,disp_req.strideY);
                        crop_sdm_buf(&sdm_buf,g_srcres[id_src].x,g_srcres[id_src].y, g_srcres[id_src].w,g_srcres[id_src].h);

                    }else{
                        if(!bgetmis)
                            id_mismatch++;
                        /* received vpu data */
                        srcin.bufY = (void*)disp_req.bufY;
                        srcin.bufCb = (void*)disp_req.bufCb;
                        srcin.bufCr = (void*)disp_req.bufCr;

                        memcpy(&srcin.rect,&g_srcres[id_src],sizeof(prect_t));
                        memcpy((void*)&dstout->rect,&g_dstdisp[id_disp],sizeof(prect_t));

                        scale_g2d(srcin,dstout,RES_WIDTH);
                        memcpy(&dstoutAB[id_bufab++],dstout,sizeof(fmt_yuv420p_t));
#ifdef USE_ARGB888
                        sdm_buf.fmt = COLOR_ARGB8888;
                        fill_sdm_buf(&sdm_buf,(unsigned long)dstout->bufY, 0, 0,dstout->rect.w * 4);
#else
                        fill_sdm_buf(&sdm_buf,(unsigned long)dstout->bufY,(unsigned long)dstout->bufCb,(unsigned long)dstout->bufCr,dstout->rect.w);
#endif
                        crop_sdm_buf(&sdm_buf,0,0,g_dstdisp[id_disp].w, g_dstdisp[id_disp].h);
                        id_post++;
                    }
                    _set_post(&post_data,&sdm_buf);
                    map_sdm_buf(&sdm_buf,g_dstdisp[id_disp].x,g_dstdisp[id_disp].y,g_dstdisp[id_disp].w, g_dstdisp[id_disp].h);
                    sdm_post(disp->handle, &post_data);
                    if(id_post >= (2*id_mismatch) && ((id_post % id_mismatch) == 0) ){
                        for(int j = 0; j<id_mismatch; j++){
                            free_g2dm(&dstoutAB[id_post % (2*id_mismatch)+j]);
                        }
                    }
                    if(id_bufab >= 2* id_mismatch)
                        id_bufab = 0;
                }
                id_src++;
                id_disp++;
                if(id_src >= rescount)
                    id_src = 0;
                if(id_disp >=dstcount){
                    id_disp = 0;
                    bgetmis = true;
                }
#ifdef ENABLE_BA_UART_A
        if((btx == false) && (i >=(SCREEN_MAX-2)))
        {
            sidea_tx_char(token,ba_uart_handle,ba_txdata);
            btx = true;
        }
#endif
            }

            }

        frames++;

        dprintf( INFO, "Current FPS:%d fps\n",1000/((current_time()-ts)/frames));

        event_wait(&fps_tmr_cb_signal);

        if(is_container_referenced(container))
        {
            container_dereferenced(container);
        }

    }

    timer_cancel(&fps_timer);
    event_destroy(&fps_tmr_cb_signal);
    token_setstatus(token,TOKEN_FIN);


}
