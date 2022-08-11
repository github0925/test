#include <lk_wrapper.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <heap.h>

#if WITH_KERNEL_VM
#include <kernel/vm.h>
#endif

#include <app.h>
#include <err.h>
#include <lib/console.h>
#include <ext_data.h>
#include <chip_res.h>
#include <task.h>
#include <queue.h>
#include <sdm_display.h>
#include <disp_data_type.h>
#include <mbox_hal.h>
#include <v4l2.h>
#include <csi_hal.h>
#include <dcf.h>
#include <dc_status.h>
#include <avm_app_csi.h>

#include "avm_data_structure_def.h"
#include "avm_fastavm_config.h"
#include "vstreamer.h"
#include "avm_player_utility.h"
#include "early_app_cfg.h"
#include "early_app_common.h"
#include <g2dlite_api.h>

#include <image_cfg.h>

#include "avm_lvgl.h"
#define SWITCH_NUM 5
#define SWITCH_HEIGHT 80
#define AVM_OFFSET_X(x) ((x-AVM_WIDTH-CAMERA_WIDTH)>>1)

typedef enum
{
    MAIN_DISP   = 0,
    SIDE_DISP   = 1,
    MAX_DISP    = 4,
}disp_idx_t;

#pragma pack(push)
#pragma pack(4)
typedef union disp_sync_t
{
    struct dargs
    {
        unsigned char state : 3;
        unsigned char       : 5;
    }args[MAX_DISP];
    uint32_t val;

}disp_sync_args_t;
#pragma pack(pop)

typedef struct disp_kick_t
{
    int state;
    void* dummy;
}disp_kick_t;


typedef struct disp_buffer_t
{
    struct sdm_buffer sdm_bufs[2];
    struct sdm_post_config post_data;

}disp_buffer_t;

static void disp_buffer_init(disp_buffer_t* db, const struct sdm_buffer* template1, const struct sdm_buffer* template2)
{
    db->post_data.bufs = &db->sdm_bufs[0];
    db->post_data.n_bufs           = 0;
    db->post_data.custom_data      = NULL;
    db->post_data.custom_data_size = 0;

    if(template1)
    {
        memcpy(&db->post_data.bufs[0],template1,sizeof(struct sdm_buffer));
        // db->post_data.bufs[0].layer = 0;
        // db->post_data.bufs[0].z_order = 1;
        db->post_data.n_bufs++;
    }
    if(template2)
    {
        memcpy(&db->post_data.bufs[1],template2,sizeof(struct sdm_buffer));
        // db->post_data.bufs[0].layer = 1;
        // db->post_data.bufs[0].z_order = 0;
        db->post_data.n_bufs++;
    }
}

static void disp_sync_post(sdm_display_t* disp,disp_buffer_t* db, disp_sync_args_t* disp_args, int layers, uint32_t idx)
{
    int layer_en = 0;
    if(disp_args->args[idx].state == DC_STAT_CLOSING || disp_args->args[idx].state == DC_STAT_CLOSED)
    {
        layer_en = 0;
    }
    else if (disp_args->args[idx].state == DC_STAT_BOOTING) {
        layer_en = 1;
    }
    else
    {
        printf("unkonwn disp state:0x%x\n",disp_args->val);
    }

    if(!layer_en)
    {
        sdm_clear_display(disp->handle);
    }
    else
    {
        sdm_post(disp->handle, &db->post_data);
    }


    // printf("post data to display %d, bufs:%d, do %d to %d layers\n",idx,db->post_data.n_bufs,layer_en, layers);

}
int avm_csi_entry(uint8_t *avm_input)
{
    struct v4l2_device *vdev;
    struct v4l2_mbus_framefmt fmt;
    struct v4l2_fract frame_interval;
    struct v4l2_fwnode_endpoint endpoint;
    int ret = 0;
    struct v4l2_ctrl ctrl = {
        .val = 1,
        .id  = V4L2_CID_VFLIP,
    };
    void *csi_handle;

    ret = avm_csi_init(0);

    if (ret < 0) {
        printf("%s(): init error\n", __func__);
        return ret;
    }

    ret = avm_csi_config(0, (uint8_t (*)[IMG_COUNT][IMG_WIDTH * IMG_HEIGHT * 2])avm_input);

    if (ret < 0) {
        printf("%s(): config error\n", __func__);
        return ret;
    }

    ret = avm_csi_start(0);

    if (ret < 0) {
        printf("%s(): start error\n", __func__);
    }
        return ret;
}

int vdsp_start_one_frame(hal_mb_chan_t *mchan, paddr_t pout, paddr_t pin) {
    /* send output addr using mailbox */
    int ret = -1;
    struct vdsp_message msg;
    msg.output_addr = pout;
    msg.input_addr  = pin;
    ret = hal_mb_send_data_dsp(mchan, (u8 *)&msg, 8);
    if (ret != NO_ERROR) {
        USDBG("mb send_data failed %d\n", ret);
    }
    return ret;
}


int vdsp_wait_interrupt(event_t *signal, int timeout) {
    if (event_wait_timeout(signal, timeout) == ERR_TIMED_OUT) {
        printf("vdsp waiting interrupt timeout!\n");
        return -1;
    }
    event_unsignal(signal);
    return 0;
}

static uint32_t ret_addr;
void vdsp_callback(hal_mb_client_t cl, void *mssg, u16 len)
{
    event_t* signal = (event_t *)hal_mb_get_user(cl);
    ret_addr        = ((uint32_t*)mssg)[0];
    uint32_t addr2  = ((uint32_t*)mssg)[0];
    USDBG("get vdsp response 0x%lu 0x%lu, len: %d\n", ret_addr, addr2, len);
    event_signal(signal, false);
}


// static dc_state_t disp_mp_get(disp_sync_args_t* disp_args, disp_idx_t idx)
// {
//     system_property_get(DMP_ID_DC_STATUS, (int*)&disp_args->val);
//     if(idx >= MAX_DISP)
//     {
//         return 0;
//     }
//     printf("get mp disp args value @ 0x%x\n",disp_args->val);

//     return disp_args->args[idx].state;
// }

// static void disp_mp_set(disp_sync_args_t* disp_args, disp_idx_t idx)
// {
//     system_property_get(DMP_ID_DC_STATUS, (int*)&disp_args->val);
//     if(idx >= MAX_DISP)
//     {
//         return 0;
//     }
//     printf("get mp disp args value @ 0x%x\n",disp_args->val);

//     return disp_args->args[idx].state;
// }


static void disp_dummy_mp_response(disp_sync_args_t* disp_args,bool force)
{
    uint32_t i = 0;
    if(force)
    {
        printf("dummy:avm closed layers\n");
        for(i = 0; i<MAX_DISPLAY_NUM; i++)
            disp_args->args[i].state = DC_STAT_CLOSED;
        system_property_set(DMP_ID_DC_STATUS,disp_args->val);
        return;
    }
    system_property_get(DMP_ID_DC_STATUS, (int*)&disp_args->val);
    printf("get dummy disp args value @ 0x%x\n",disp_args->val);

    for(i=0;i<MAX_DISP;i++)
    {
        if (disp_args->args[i].state == DC_STAT_CLOSING) {
            printf("dummy:avm closed layers\n");
            disp_args->args[i].state = DC_STAT_CLOSED;
            system_property_set(DMP_ID_DC_STATUS,disp_args->val);
        }
    }
    printf("set dummy disp args value @ 0x%x\n",disp_args->val);

}

void kick_side_run(sdm_display_t* display)
{

}

int kick_display_run(sdm_display_t* display, int* state, void** dummy)
{
    struct sdm_buffer sdm_buf = DISPLAY_AVM_TEMPLATE;
    struct sdm_post_config post_data;
    if(!*state)
    {
        printf("1st kick\n");
        *dummy = malloc(1280*720*3);
        if(!(*dummy))
        {
            printf("malloc dummy fail\n");
            return -1;
        }
        *state = 1;
        avm_fill_sdm_buf(&sdm_buf,(unsigned long)(*dummy), 1280);
        avm_crop_sdm_buf(&sdm_buf, 0, 0, 1280, 720);
        avm_map_sdm_buf(&sdm_buf, 0, 0, 1280, 720);
        post_data.bufs             = &sdm_buf;
        post_data.n_bufs           = 1;
        post_data.custom_data      = NULL;
        post_data.custom_data_size = 0;

        post_data.bufs[0].layer_en = 1;
        post_data.bufs[0].alpha = 0;
        sdm_post(display->handle, &post_data);

    }

    else if(*state == 1)
    {
        printf("2nd kick\n");
        *state = 2;
        avm_fill_sdm_buf(&sdm_buf, (unsigned long)(*dummy), 1280);
        avm_crop_sdm_buf(&sdm_buf, 0, 0, 1280, 720);
        avm_map_sdm_buf(&sdm_buf, 0, 0, 1280, 720);
        post_data.bufs             = &sdm_buf;
        post_data.n_bufs           = 1;
        post_data.custom_data      = NULL;
        post_data.custom_data_size = 0;
        post_data.bufs[0].layer_en = 0;
        post_data.bufs[0].alpha = 0xff;
        sdm_clear_display(display->handle);
    }
    else if(*state == 2)
    {
        printf("kick done\n");
        *state = 3;
        free(*dummy);
    }

    return *state;

}

static void _csi_timedout_daemon(void)
{
    struct sdm_buffer sdm_buf = DISPLAY_AVM_TEMPLATE;
    struct sdm_post_config post_data;

    disp_sync_args_t dargs;
    struct list_node* disp_node = sdm_get_display_list();
    uint32_t disp_num = list_length(disp_node);
    uint32_t i = 0;
    uint32_t disp_id = 0;
    sdm_display_t* disp[MAX_DISPLAY_NUM+1];
    for(i = 0; i<disp_num;i++){
        disp[disp_num]= containerof(disp_node->next,sdm_display_t,node);
        disp_id = disp[disp_num]->handle->display_id;
        USDBG("display:%p,%s\n",disp[i],disp[i]->handle->name);
        disp[disp_id]=disp[disp_num];
        disp_node = disp_node->next;
    }

    post_data.bufs             = &sdm_buf;
    post_data.n_bufs           = 1;
    post_data.custom_data      = NULL;
    post_data.custom_data_size = 0;
    post_data.bufs[0].layer_en = 0;
    post_data.bufs[0].alpha = 0;

    printf("go into csi timeout daemon task\n");


    for(;;)
    {
        system_property_get(DMP_ID_DC_STATUS, (int*)&dargs.val);

        //margin case: close side display cause no avm need while andriod launcher start.
        if(dargs.args[CLUSTER].state == DC_STAT_CLOSING)
        {
            sdm_clear_display(disp[CLUSTER]->handle);
            dargs.args[CLUSTER].state = DC_STAT_CLOSED;
            system_property_set(DMP_ID_DC_STATUS,dargs.val);
        }

        if(dargs.args[INFOTAINMENT].state == DC_STAT_CLOSING)
        {
            sdm_clear_display(disp[INFOTAINMENT]->handle);
            dargs.args[INFOTAINMENT].state = DC_STAT_CLOSED;

            sdm_clear_display(disp[INFOTAINMENT]->handle);
            dargs.args[INFOTAINMENT].state = DC_STAT_CLOSED;

            system_property_set(DMP_ID_DC_STATUS,dargs.val);
        }

        if((dargs.args[CLUSTER].state == DC_STAT_CLOSED) && (dargs.args[INFOTAINMENT].state == DC_STAT_CLOSED) && (dargs.args[INFOTAINMENT].state == DC_STAT_CLOSED))
        {
            printf("main,side and cluster all closed\n");
            while(1)
            {
                thread_sleep(1000);
            }
        }

        thread_sleep(1);
    }

}



static const struct sdm_buffer avm_template = DISPLAY_AVM_TEMPLATE;
static const struct sdm_buffer single_template = DISPLAY_SINGLE_TEMPLATE;

void sdm_display_init(void);
static void *mbox_handle;
static event_t avm_fin;
void avm_csi_tslide(sdm_display_t* disp, disp_buffer_t* db);

void avm_state_change_cb(void* pargs, int uargs, int signal);
static void _img_event_cb(lv_obj_t * obj, lv_event_t event)
{
    event_signal( &avm_fin,false);
    //lv_obj_set_hidden(obj,true);
}

event_t avm_get_finevent(void)
{
    return avm_fin;
}

static void _avm_fininish(disp_buffer_t db,sdm_display_t* displ)
{
    SetAvmFlag(0); //Notify Tp

    disp_buffer_init(&db,&avm_template,&single_template); //Init layer1 & clear
    sdm_clear_display(displ->handle);

    stopLvgl();
    if(lv_disp_get_scr_act(get_display(INFOTAINMENT)))
        lv_obj_del(lv_disp_get_scr_act(get_display(INFOTAINMENT)));
    // lv_disp_remove(get_display(INFOTAINMENT));
    //avm_csi_tslide(displ,&db); //close dev
    avm_csi_stop(0);
    avm_csi_close(0);
}

void fastavm_task(void* token)
{
    int ret = 0;
    uint32_t frame_width, frame_height;
    vdsp_rsp_t vdsp_rsp;
    uint8_t (*pout)[AVM_WIDTH * AVM_HEIGHT * 3];
    uint32_t pin;
    int index = 0;
    event_t vdsp_signal;

    hal_mb_client_t cl;
    hal_mb_chan_t *mchan;
    u8 msg_buf[64];
    int16_t screenwidth,screenheight;
    bool lvglkicked = false;
    lv_obj_avm avmobj[SWITCH_NUM];

    lv_obj_t * scr = lv_disp_get_scr_act(get_display(INFOTAINMENT));     /*Get the current screen*/
    lv_obj_set_style_local_bg_opa(scr, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
    getScrSize(scr,&screenwidth,&screenheight);

    sdm_display_t* disp;
    event_init(&avm_fin,0,false);

    unified_service_subscribe(unified_service_get_property_id(token),
                                unified_service_get_current_state(token),
                                avm_state_change_cb,token,0);


#if !defined(STANDALONE_DISPLAY_THREAD)
    disp_buffer_t db;
    disp_buffer_init(&db,&avm_template,NULL);
    disp = get_disp_handle(INFOTAINMENT);
    ASSERT(disp);
#endif

    void *handle = NULL;
    bool retg2d;
    retg2d = hal_g2dlite_creat_handle(&handle, RES_G2D_G2D2);
    if(!retg2d)
    {
        printf("Failed to create g2dlite handle\r\n");
        return;
    }
    hal_g2dlite_init(handle);

    struct g2dlite_input input;
    memset(&input, 0, sizeof(struct g2dlite_input));

    initG2dInput(&input,screenwidth, screenheight);

    uint8_t* g2d_output = memalign(0x1000, 3 * screenwidth * screenheight * 3);
    if (g2d_output == NULL) {
        printf("malloc output buffer failed\r\n");
        return;
    }
    uint8_t (*pout_g2d)[ screenwidth * screenheight * 3];
    pout_g2d = (uint8_t (*)[screenwidth * screenheight * 3])g2d_output;

    lv_obj_t * carimg = lv_img_create(scr, NULL);
    setImgPosition(carimg,AVM_WIDTH,screenheight,AVM_OFFSET_X(screenwidth));

#ifndef DISABLE_FASTAVM_SWITCH
    createSwitch(avmobj,SWITCH_NUM,SWITCH_HEIGHT,AVM_WIDTH+AVM_OFFSET_X(screenwidth));
#endif

    hal_mb_cfg_t hal_cfg;
    hal_mb_create_handle(&mbox_handle, RES_MB_MB_MEM);

    if (mbox_handle != NULL) {
        hal_mb_init(mbox_handle, &hal_cfg);
    }

    cl = hal_mb_get_client();
    if (!cl) {
        printf("get mb cl failed %d failed\n");
        return;
    }

    mchan = hal_mb_request_channel(cl, true, vdsp_callback, IPCC_RRPOC_VDSP);
    if (!mchan) {
        printf("request mb channel failed\n");
        goto FAIL1;
    }

    hal_mb_set_user(cl, &vdsp_signal);

    /* alloc input output buffer */

    void** container = NULL;

    container = unified_service_get_container_pointer(token);

    uint8_t* avm_input = *container;

    uint8_t *avm_output = memalign(0x1000, 3 * AVM_WIDTH * AVM_HEIGHT);
    if (avm_output == NULL) {
        printf("malloc avm_output buffer failed\r\n");
        goto FAIL3;
    }
    pout = (uint8_t (*)[AVM_WIDTH * AVM_HEIGHT * 3])avm_output;

    void *csi_handle = avm_csi_get_handle(0);
    if (!csi_handle) {
        printf(" get csi handle failed.\n");
        goto FAIL4;
    }

    csi_instance_t *instance = NULL;
    struct csi_image *img;
    instance = (csi_instance_t *)csi_handle;

    if (!instance->dev) {
        printf("get csi dev failed.\n");
        goto FAIL4;
    }

    USDBG("start vdsp avm task.\n");
    event_init(&vdsp_signal, false, 0);

    vdsp_rsp.bufCb = 0;
    vdsp_rsp.bufCb = 0;
    vdsp_rsp.bufCr = 0;

extern uint8_t* map_tlb;

    ret = vdsp_start_one_frame(mchan, p2ap((paddr_t)map_tlb), p2ap(0x123FF));
    if (ret) {
        printf("vdsp send mappingtable failed!\n");
        goto FAIL4;
    }
    vdsp_wait_interrupt(&vdsp_signal, 3000);

    APP_PRF("------->mappingtable loaded\n");

    int timeout_count = 0;

    do {
RETRY:
        pin = (uintptr_t)avm_input;
        for (int i = 0; i < 1; i++) { // 4 ints is at same time for 4 cameras
            img = instance->dev->ops.get_image(instance->dev, i);

            if (!img || !img->enable) {
                printf("csi img[%d] is not available.\n", img);
                continue;
            }
            struct csi_device *dev = img->csi;
            int pos, n=0;

            pos=1;
            (void)pos;

            if (ERR_TIMED_OUT == event_wait_timeout(&img->completion, 50)) {
                {
                    timeout_count++;
                    if(timeout_count == 25)
                    {
                        printf("CSI wait timeout, do nothing.\n");
                        _avm_fininish(db,disp);
                        goto FAIL4;
                    }
                }

                goto RETRY;
            }
            else
            {
            #if CONFIG_USE_SYS_PROPERTY
                static bool set = 0;
                if(!set)
                    system_property_set(DMP_ID_DC_STATUS, 1);
            #endif
            }

            USDBG("pos=%d, img->buf_pos=%d\n", pos, img->buf_pos);
            // if (pos != img->buf_pos)
            {
                n = CAMERA_BUF_POS(img->buf_pos);
                //n = ((n - 1) >= 0) ? (n - 1) : 2;
                USDBG("csi vid:%d, pos:%d, baddr:0x%x\n", img->id, n, img->rgby_baddr[n]);
                pos = img->buf_pos;
                //pin |= (n << 0)|(n << 2)|(n << 4)|(n << 6); // 4 ints is synced at same pos same time
                pin += n * IMG_COUNT * IMG_WIDTH * IMG_HEIGHT * 2;
                if (i == 0) {
                    vdsp_rsp.bufCb = (img->rgby_baddr[CAMERA_BUF_POS(img->buf_pos)]); // borrow to send csi
                    vdsp_rsp.bufCr = IMG_WIDTH * 2;
                }
            }
        }

        APP_PRF("++csi  end@%lu: pin = 0x%x\n", current_time(), pin);
        ret = vdsp_start_one_frame(mchan, p2ap((uintptr_t)(pout)), p2ap((uintptr_t)(pin)));
        if (ret) {
            printf("vdsp start one frame failed!\n");
            continue;
        }
        APP_PRF("++vdsp start@%lu\n", current_time());
        int timeout = vdsp_wait_interrupt(&vdsp_signal, 100);
        APP_PRF("++vdsp end@%lu\n", current_time());
        USDBG("frm idx:%d\n", index);
        USDBG("ret addr = 0x%x\n", ret_addr);
        if (timeout == -1) {
            unified_service_publish(token,ussTerminated);
            _avm_fininish(db,disp);
            goto FAIL4;
        }
        input.layer[1].addr[0] = (unsigned long)(avm_output+AVM_WIDTH*3*(MAX((AVM_HEIGHT-screenheight)/2,0)));
        input.layer[0].addr[0] = (unsigned long)(pin + CAMERA_WIDTH * CAMERA_HEIGHT * 2 * getCameraId() + CAMERA_WIDTH*2*(MAX(0,(CAMERA_HEIGHT-screenheight)/2)) + ( MAX(0,((CAMERA_WIDTH-(screenwidth-AVM_WIDTH))/2))*2));
        input.output.addr[0] = (unsigned long)(pout_g2d+index);

        hal_g2dlite_blend(handle, &input);

        vdsp_rsp.bufY = (paddr_t)(pout_g2d + index);
        vdsp_rsp.strideY = screenwidth * 3;//frame_buffer[proc_result.indexFrameDisplay].stride;
        vdsp_rsp.end = 0;
#if defined(STANDALONE_DISPLAY_THREAD)
        APP_PRF("++disp wait@%lu\n", current_time());
        strm->push(strm, &vdsp_rsp);
        event_signal(evt_done, false);
#else
        avm_fill_sdm_buf(&db.sdm_bufs[0], vdsp_rsp.bufY, vdsp_rsp.strideY);
#if ENABLE_SERDES
        avm_crop_sdm_buf(&db.sdm_bufs[0], 0, 0, screenwidth, screenheight);
        avm_map_sdm_buf(&db.sdm_bufs[0], 0, 0, screenwidth, screenheight);
#else
        /*TBD*/
        avm_crop_sdm_buf(&db.sdm_bufs[0], 0, 0, AVM_WIDTH, AVM_HEIGHT);
        avm_map_sdm_buf(&db.sdm_bufs[0], AVM_X, AVM_Y, AVM_WIDTH, AVM_HEIGHT);
#endif

#endif

        if(lvglkicked==false)
        {
            lvglkicked = true;
            thread_t* lvglkickmainloop_thread = thread_create("lvglkickmainloop", (thread_start_routine)lvglkickmainloop,
                                                  0, DEFAULT_PRIORITY, DEFAULT_STACK_SIZE);
            thread_detach_and_resume(lvglkickmainloop_thread);
        }

        sdm_post(disp->handle,&db.post_data);

        if(!event_wait_timeout(&avm_fin,0))
        {
            USDBG("trigger avm slide.\n");
            unified_service_publish(token,ussTerminated);
            _avm_fininish(db,disp);
            goto FAIL4;
        }

        APP_PRF("++disp start@%lu\n", current_time());
        index++;
        if(index > 2) index = 0;
    } while (true);

    // vstream_destroy(strm);

FAIL4:
    free(avm_output);
    free(g2d_output);
FAIL3:
    free(avm_input);
FAIL2:
    hal_mb_free_channel(mchan);
FAIL1:
    hal_mb_put_client(cl);

    printf("fastavm_task exit\n");
}


void avm_state_change_cb(void* pargs, int uargs,int singal)
{
    void* token = (void*)pargs;
    // printf("go into avm state change cb.\n");

    if(singal) //trigger by remote call
    {
        // printf("avm fin trigger!\n");
        event_signal(&avm_fin,false);
    }
}

#define OFFSET_X 40

void avm_csi_tslide(sdm_display_t* disp, disp_buffer_t* db){
    uint32_t i = 0;


    uint32_t posx_left = 0;
    uint32_t accumpos = 0;
    for(i = 1; i<6;i++){
        accumpos += i*OFFSET_X;

#if ENABLE_SERDES
        avm_crop_sdm_buf(&db->sdm_bufs[0], posx_left+accumpos, (AVM_HEIGHT - disp->handle->info.height)/2, AVM_WIDTH-accumpos, disp->handle->info.height);
        avm_map_sdm_buf(&db->sdm_bufs[0], AVM_X, AVM_Y, AVM_WIDTH , disp->handle->info.height);
#else
        avm_crop_sdm_buf(&db->sdm_bufs[0], posx_left, 0, AVM_WIDTH, AVM_HEIGHT);
        avm_map_sdm_buf(&db->sdm_bufs[0], AVM_X, AVM_Y, AVM_WIDTH, AVM_HEIGHT);
#endif

#if ENABLE_SERDES
        avm_crop_sdm_buf(&db->sdm_bufs[1], posx_left, 0, IMG_WIDTH, IMG_HEIGHT);
        avm_map_sdm_buf(&db->sdm_bufs[1], SINGLE_IMG_X-accumpos, SINGLE_IMG_Y, disp->handle->info.width, disp->handle->info.height);
#else
        avm_crop_sdm_buf(&db->sdm_bufs[1], posx_left, 0, IMG_WIDTH/2, IMG_HEIGHT);
        avm_map_sdm_buf(&db->sdm_bufs[1], SINGLE_IMG_X, SINGLE_IMG_Y, SINGLE_IMG_WIDTH, SINGLE_IMG_HEIGHT);
#endif
        sdm_post(disp->handle,&db->post_data);
    }

    db->post_data.bufs = &db->sdm_bufs[1];
    db->post_data.n_bufs = 1;
    accumpos = 0;

    for(i = 0;i<5;i++){
        accumpos += i*OFFSET_X*2;

#if ENABLE_SERDES
        avm_crop_sdm_buf(&db->sdm_bufs[1], posx_left+accumpos, 0, IMG_WIDTH-accumpos, IMG_HEIGHT);
        avm_map_sdm_buf(&db->sdm_bufs[1], 0, SINGLE_IMG_Y, disp->handle->info.width-accumpos, disp->handle->info.height);
#else
        avm_crop_sdm_buf(&db->sdm_bufs[1], posx_left, 0, IMG_WIDTH/2, IMG_HEIGHT);
        avm_map_sdm_buf(&db->sdm_bufs[1], SINGLE_IMG_X, SINGLE_IMG_Y, SINGLE_IMG_WIDTH, SINGLE_IMG_HEIGHT);
#endif
        sdm_post(disp->handle,&db->post_data);
    }

    sdm_clear_display(disp->handle);
}
