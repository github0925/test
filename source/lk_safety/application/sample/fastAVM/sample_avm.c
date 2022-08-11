#include "sample_avm.h"
#include <heap.h>
#include <early_app_cfg.h>
#include <macros.h>
#include "res_loader.h"
#include <dcf.h>
#include <dc_status.h>
#include <sdm_display.h>
#include "early_app_common.h"
#include "fastavm_api.h"
#include <sdm_display.h>
#include <disp_data_type.h>
#include "avm_player_utility.h"
#include <string.h>
#include <image_cfg.h>
#include "timer.h"
#include <g2dlite_api.h>
#include "lvgl_gui.h"
/*
void fastAVM_loadMappingTable(void * pMappingTable);
void fastAVM_loadFastAVM(void * pelfbuffer);
void fastAVM_setCameraResolution(uint16_t width, uint16_t height);
void fastAVM_getCameraResolution(uint16_t * width, uint16_t * height);
void fastAVM_setFPS( uint32_t numerator, uint32_t denominator );
void fastAVM_getFPS( uint32_t * numerator, uint32_t * denominator );
void fastAVM_init(void);
void fastAVM_start(void);
void fastAVM_stop(void);
void fastAVM_update_one_frame(uint8_t * pframebuffer, uint8_t * * pcamerabuffer);
fastAVM_context_t fastAVM_getstatus(void);
fastAVM_car_t fastAVM_getCarPosition(void);
*/

//extern uint8_t VDSP_FW_RUNTIME_MEM[];
#define VDSP_FW_RUNTIME_MEM (ap2p(VDSP_MEMBASE))


#define CARIMG_PATH    "early_app/fastavm/car.bin"
#define VDSP_ELF_PATH    "early_app/fastavm/vdsp.elf"
#define MAP_TLB_PATH     "early_app/fastavm/MappingTable.bin"

typedef struct packedframe_t
{
    unsigned long bufPackedData;
    unsigned long stride;
    unsigned int  width;
    unsigned int  height;
}packedframe_t;

typedef struct disp_buffer_t
{
    struct sdm_buffer sdm_bufs[2];
    struct sdm_post_config post_data;

}disp_buffer_t;

static const struct sdm_buffer avm_template = DISPLAY_AVM_TEMPLATE;
static const struct sdm_buffer single_template = DISPLAY_SINGLE_TEMPLATE;

static void disp_buffer_init(disp_buffer_t* db, const struct sdm_buffer* template1, const struct sdm_buffer* template2)
{
    db->post_data.bufs = &db->sdm_bufs[0];
    db->post_data.n_bufs           = 0;
    db->post_data.custom_data      = NULL;
    db->post_data.custom_data_size = 0;

    if(template1)
    {
        memcpy(&db->post_data.bufs[0],template1,sizeof(struct sdm_buffer));
        db->post_data.n_bufs++;
    }
    if(template2)
    {
        memcpy(&db->post_data.bufs[1],template2,sizeof(struct sdm_buffer));
        db->post_data.n_bufs++;
    }
}

#define max(a, b)   ((a) > (b) ? (a) : (b))

#define min(a, b)   ((a) < (b) ? (a) : (b))

void lvglkickmainloop(void)
{
    lvgl_mainloop();
}

void fastavm_test(void)
{
    //1. load fastavm & mappingtable resource
    fastAVM_init();

    lvgl_init();

    //1. load fastavm firmware & mappingtable resource
    int elf_size = ROUNDUP(res_size(VDSP_ELF_PATH),32);
    void* pelf = memalign(32,elf_size);
    if(NULL==pelf)
    {
        USDBG("ERROR: pelf is NULL\n");
        return;
    }

    res_load(VDSP_ELF_PATH,pelf,elf_size,0);
    int map_tlb_size = ROUNDUP(res_size(MAP_TLB_PATH),32);
    void* pmappingtable = (void*)(VDSP_FW_RUNTIME_MEM + VDSP_MEMSIZE - map_tlb_size);
    if(NULL==pmappingtable)
    {
        USDBG("ERROR: pmappingtable is NULL\n");
        return;
    }
    res_load(MAP_TLB_PATH,pmappingtable,map_tlb_size,0);

    int carimg_size = ROUNDUP(res_size(CARIMG_PATH),32);
    void* pcarimgdata = memalign(32,carimg_size);
    if(NULL==pcarimgdata)
    {
        USDBG("ERROR: pcarimgdata is NULL\n");
        return;
    }
    res_load(CARIMG_PATH,pcarimgdata,carimg_size,0);

    lv_img_dsc_t carimg_dsc;
    carimg_dsc.header = *((lv_img_header_t *)pcarimgdata);
    carimg_dsc.data = (uint8_t*)pcarimgdata+4;
    carimg_dsc.data_size = carimg_dsc.header.w*carimg_dsc.header.h*LV_IMG_PX_SIZE_ALPHA_BYTE;

    fastAVM_loadMappingTable(pmappingtable);
    fastAVM_loadFastAVM(pelf,elf_size);

    //2. set camera width\height\fps
    fastAVM_setCameraResolution(CAMERA_WIDTH,CAMERA_HEIGHT);
    fastAVM_setFPS(1, 25);

    //3. prepare output framebuffer, trible buffer to avoid frame tear
    uint8_t *avm_output = memalign(0x1000, 3 * AVM_WIDTH * AVM_HEIGHT);

    lv_obj_t * scr = lv_disp_get_scr_act(get_display(INFOTAINMENT));     /*Get the current screen*/
    lv_obj_set_style_local_bg_opa(scr, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
    int16_t screenwidth,screenheight;
    screenwidth = scr->coords.x2 - scr->coords.x1 + 1;
    screenheight = scr->coords.y2 - scr->coords.y1 + 1;

    uint8_t *g2d_output = memalign(0x1000, 3 * screenwidth * screenheight * 3);

    uint8_t (*pout)[screenwidth * screenheight * 3];
    if (avm_output == NULL) {
        USDBG("malloc output buffer failed\n");
        return;
    }
    if (g2d_output == NULL) {
        USDBG("malloc output buffer failed\n");
        return;
    }
    pout = (uint8_t (*)[screenwidth * screenheight * 3])g2d_output;

    //4. setting display
    sdm_display_t* disp;
#if !defined(STANDALONE_DISPLAY_THREAD)
    disp_buffer_t db;
    disp_buffer_init(&db,&avm_template,NULL);
    disp = get_disp_handle(INFOTAINMENT);
    ASSERT(disp);
#endif

    //5. start
    fastAVM_start();
    free(pelf);
    pelf = NULL;

    //6. do-while update_one_frame

    void *handle = NULL;
    struct g2dlite_input input;

    bool ret;
    ret = hal_g2dlite_creat_handle(&handle, RES_G2D_G2D2);
    if (!ret) {
        LOGD("g2dlite creat handle failed\n");
    }

    hal_g2dlite_init(handle);

    memset(&input, 0, sizeof(struct g2dlite_input));

    input.layer_num = 2;

    input.layer[0].layer = 0;
    input.layer[0].layer_en = 1;
    input.layer[0].fmt = COLOR_YUYV;
    input.layer[0].zorder = 0;
    input.layer[0].src.x = max(0,(screenwidth-AVM_WIDTH-CAMERA_WIDTH)/2);
    input.layer[0].src.y = max(0,(screenheight-CAMERA_HEIGHT)/2);
    input.layer[0].src.w = min(CAMERA_WIDTH,(screenwidth-AVM_WIDTH));
    input.layer[0].src.h = min(screenheight,CAMERA_HEIGHT);
    input.layer[0].src_stride[0] = CAMERA_WIDTH * 2;
    input.layer[0].dst.x = AVM_WIDTH+max(0,(screenwidth-AVM_WIDTH-CAMERA_WIDTH)/2);
    input.layer[0].dst.y = max(0,(screenheight-CAMERA_HEIGHT)/2);
    input.layer[0].dst.w = min(CAMERA_WIDTH,screenwidth-AVM_WIDTH);
    input.layer[0].dst.h = min(screenheight,CAMERA_HEIGHT);
    input.layer[0].ckey.en = 0;
    input.layer[0].blend = BLEND_PIXEL_NONE;
    input.layer[0].alpha = 0xFF;

    input.layer[1].layer = 1;
    input.layer[1].layer_en = 1;
    input.layer[1].fmt = COLOR_BGR888;
    input.layer[1].zorder = 1;
    input.layer[1].src.x = 0;
    input.layer[1].src.y = max(0,(screenheight-AVM_HEIGHT)/2);
    input.layer[1].src.w = AVM_WIDTH;
    input.layer[1].src.h = min(screenheight,AVM_HEIGHT);
    input.layer[1].src_stride[0] = AVM_WIDTH * 3;
    input.layer[1].dst.x = 0;
    input.layer[1].dst.y = max(0,(screenheight-AVM_HEIGHT)/2);
    input.layer[1].dst.w = AVM_WIDTH;
    input.layer[1].dst.h = min(screenheight,AVM_HEIGHT);
    input.layer[1].ckey.en = 0;
    input.layer[1].blend = BLEND_PIXEL_NONE;
    input.layer[1].alpha = 0xFF;

    input.output.width = screenwidth;
    input.output.height = screenheight;
    input.output.fmt = COLOR_RGB888;
    input.output.stride[0] = screenwidth* 3;
    input.output.rotation = 0;

    lv_obj_t * carimg = lv_img_create(scr, NULL);
    lv_img_set_src(carimg, &carimg_dsc);
    lv_obj_set_pos(carimg, (AVM_WIDTH-carimg_dsc.header.w)/2, (screenheight-carimg_dsc.header.h)/2);
    lv_obj_set_size(carimg, carimg_dsc.header.w, carimg_dsc.header.h);

    bool lvglkicked = false;

    int index = 0;
    do
    {
        uint8_t * pcamerabuffer;

        int ret;
        ret = fastAVM_update_one_frame((uint8_t * )avm_output,(uint8_t * *)&pcamerabuffer);

        if(ret!=0)
        {
            USDBG("fastAVM_update_one_frame failed!\n");
            continue;
        }

        input.layer[1].addr[0] = (unsigned long)(avm_output+AVM_WIDTH*3*(max((AVM_HEIGHT-screenheight)/2,0)));
        input.layer[0].addr[0] = (unsigned long)(pcamerabuffer + CAMERA_WIDTH * CAMERA_HEIGHT * 2 * singlecameraid + CAMERA_WIDTH*2*(max(0,(CAMERA_HEIGHT-screenheight)/2)) + ( max(0,((CAMERA_WIDTH-(screenwidth-AVM_WIDTH))/2))*2));
        input.output.addr[0] = (unsigned long)(pout+index);

        hal_g2dlite_blend(handle, &input);

        packedframe_t avmdata;

        /*
            Diaplay Panel 1280x800
             _________________
            |  640   |  640   |
            |        |        |
         800|fastAVM |original|
            |        |camera  |
            |________|________|

        */
        avmdata.bufPackedData = (paddr_t)(pout + index);
        avmdata.stride = screenwidth * 3;
        avmdata.width = screenwidth;
        avmdata.height = screenheight;

        avm_fill_sdm_buf(&db.sdm_bufs[0], avmdata.bufPackedData, avmdata.stride);
        avm_crop_sdm_buf(&db.sdm_bufs[0], 0, 0, screenwidth, screenheight);
        avm_map_sdm_buf(&db.sdm_bufs[0], 0, 0, screenwidth, screenheight);

        if(lvglkicked==false)
        {
            lvglkicked = true;
            thread_t* lvglkickmainloop_thread = thread_create("lvglkickmainloop", (thread_start_routine)lvglkickmainloop,
                                                  0, DEFAULT_PRIORITY, DEFAULT_STACK_SIZE);
            thread_detach_and_resume(lvglkickmainloop_thread);
        }

        sdm_post(disp->handle,&db.post_data);

        index++;
        if(index > 2) index = 0;
    }while(1);

    //7. stop
    fastAVM_stop();

    free(avm_output);
    free(g2d_output);
}
