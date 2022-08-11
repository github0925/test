#include <app.h>
#include <lib/console.h>
#include <kernel/event.h>
#include <thread.h>
#include <err.h>
#include <mbox_hal.h>
#include <v4l2.h>
#include <csi_hal.h>
#include <avm_app_csi.h>
#include <ext_data.h>
#include <heap.h>
#include <sdrpc.h>
#include <macros.h>

#include "vstreamer.h"
#include "avm_data_structure_def.h"
#include "avm_fastavm_config.h"
#include "res_loader.h"
#include <ext_data.h>
#include <early_app_cfg.h>
#include <image_cfg.h>

#include "sample_avm.h"

#include "avm_lvgl.h"
#include "sms_storage.h"

#define VDSP_ELF_PATH    "early_app/fastavm/vdsp.elf"
#define MAP_TLB_PATH     "early_app/fastavm/MappingTable.bin"

#if defined(STANDALONE_DISPLAY_THREAD)
void player_task(vstreamer_t *strm);
#endif
void fastavm_task(vstreamer_t *strm);
void vdsp2disp_proc(void *prod_rsp, void *cons_req);

static vstreamer_t *vdsp_strm, *disp_strm;
event_t evt;

void vdsp_rst_stall(uint32_t vector_base);
void vdsp_go(void);
int vdsp_load_firmware(uint32_t elf_base, uint32_t elf_len,
                       void *dummy_binary_base);

// uint8_t* VDSP_FW_RUNTIME_MEM = VDSP_MEMBASE;
uint8_t *map_tlb = NULL;
#define VDSP_FW_RUNTIME_MEM (ap2p(VDSP_MEMBASE))

int avm_csi_entry(uint8_t *avm_input);

bool avm_csi_preinit(void *token)
{
    uint8_t *avm_input = memalign(0x1000,
                                  IMG_COUNT * CAMERA_MAX_BUF * IMG_WIDTH * IMG_HEIGHT * 2);

    if (avm_input == NULL) {
        printf("malloc input buffer failed\n");
        return -1;
    }

    /* init csi cameras */

    int ret = avm_csi_entry(avm_input);

    if (ret < 0) {
        printf("int csi failed.\n");
        return -1;
    }

    void **container = NULL;

    container = unified_service_get_container_pointer(token);
    *container = (void *)avm_input;

    return 0;
}

void avm_init(void *token)
{
    unified_service_publish(token, ussInit);
    int elf_size = ROUNDUP(res_size(VDSP_ELF_PATH), 32);
    void *elf = memalign(32, elf_size);
    ASSERT(elf);
    vdsp_rst_stall(p2ap((paddr_t)VDSP_FW_RUNTIME_MEM));
    res_load(VDSP_ELF_PATH, elf, elf_size, 0);
    init_avm_storage();

    if (get_avm_img_state()) {
        int map_tlb_size = ROUNDUP(res_size(MAP_TLB_PATH), 32);
        uint32_t mtlb = (uint32_t)(VDSP_FW_RUNTIME_MEM + VDSP_MEMSIZE - map_tlb_size);
        printf("elf file @ %p size:0x%x map_tlb_size 0x%x\n", elf, elf_size,
               map_tlb_size);
        ASSERT(!(mtlb % 32));
        map_tlb = (uint8_t *)mtlb;
        res_load(MAP_TLB_PATH, (void *)map_tlb, map_tlb_size, 0);
    }
    else {
        int map_tlb_size = ROUNDUP(get_avm_img_size(), 32);
        uint32_t mtlb = (uint32_t)(VDSP_FW_RUNTIME_MEM + VDSP_MEMSIZE - map_tlb_size);
        printf("elf file @ %p size:0x%x map_tlb_size 0x%x\n", elf, elf_size,
               map_tlb_size);
        ASSERT(!(mtlb % 32));
        map_tlb = (uint8_t *)mtlb;
        read_avm_partition(map_tlb);
    }

    deinit_avm_storage();
    vdsp_load_firmware((uint32_t)elf, elf_size, NULL);
    printf("vdsp load done\n");
    vdsp_go();
    free(elf);

    lvgl_init();

    if (avm_csi_preinit(token)) {
        //lv_obj_t * scr = lv_disp_get_scr_act(get_display(INFOTAINMENT));     /*Get the current screen*/
        //lv_obj_set_style_local_bg_opa(scr, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
        printf("avm csi preinit fail.\n");
        return;
    }


    setCarImg();

    unified_service_publish(token, ussReady);

}

void avm_start(void *token)
{
#if 1
    // avm_init();

    unified_service_publish(token, ussRun);
    SetAvmFlag(1);
    vdsp_strm = vstream_create(0, 0);
    disp_strm = vstream_create(sizeof(disp_req_t), 2); //a/b buffer
    event_init(&evt, false, 0);
    vstream_link(vdsp_strm, disp_strm, vdsp2disp_proc, &evt);
    thread_t *fastavm = thread_create("avm", (thread_start_routine)fastavm_task,
                                      token, DEFAULT_PRIORITY, DEFAULT_STACK_SIZE);
#if defined(STANDALONE_DISPLAY_THREAD)
    /* standalone thread to handle display, maybe used when G2D is needed to blend */
    thread_t *player  = thread_create("player", (thread_start_routine)player_task,
                                      disp_strm, DEFAULT_PRIORITY, DEFAULT_STACK_SIZE);
    thread_detach_and_resume(player);
#endif
    thread_detach_and_resume(fastavm);
#else
    unified_service_publish(token, ussRun);
    thread_t *fastavmthread = thread_create("avm",
                                            (thread_start_routine)fastavm_test,
                                            NULL, DEFAULT_PRIORITY, DEFAULT_STACK_SIZE);
    thread_detach_and_resume(fastavmthread);
#endif
}

void avm_stop(void)
{
    avm_csi_stop(0);
}


int vdsp_start_one_frame(hal_mb_chan_t *mchan, paddr_t pout, paddr_t pin);

static void vdsp_receive_msg(hal_mb_client_t cl, void *mssg, u16 len)
{
    event_t *signal = (event_t *)hal_mb_get_user(cl);
    uint32_t addr1 = ((uint32_t *)mssg)[0];
    uint32_t addr2 = ((uint32_t *)mssg)[1];
    //printf("get vdsp response 0x%lx 0x%lx, len: %d\n", addr1, addr2, len);
    event_signal(signal, false);
}

static void *mbox_handle;
static int vdsp_ping_test(int argc, const cmd_args *argv)
{
    hal_mb_client_t cl;
    hal_mb_chan_t *mchan;
    event_t vdsp_msg_signal;
    status_t ret;
    hal_mb_cfg_t hal_cfg;

    hal_mb_create_handle(&mbox_handle, RES_MB_MB_MEM);

    if (mbox_handle != NULL) {
        hal_mb_init(mbox_handle, &hal_cfg);
    }

    cl = hal_mb_get_client();

    if (!cl) {
        printf("get cl failed\n");
        return -1;
    }

    mchan = hal_mb_request_channel(cl, true, vdsp_receive_msg, IPCC_RRPOC_VDSP);

    if (!mchan) {
        printf("request channel failed\n");
        goto fail1;
    }

    event_init(&vdsp_msg_signal, false, 0);
    hal_mb_set_user(cl, &vdsp_msg_signal);

    lk_bigtime_t start_time = current_time_hires();

    ret = vdsp_start_one_frame(mchan, 0x40000055, 0x50005000);

    if (ret != NO_ERROR) {
        printf("send_data failed %d\n", ret);
        goto fail2;
    }

    if (ERR_TIMED_OUT == event_wait_timeout(&vdsp_msg_signal, 300000)) {
        printf("vdsp waiting interrupt timeout!\n");
        goto fail2;
    }

    lk_bigtime_t end_time = current_time_hires();

    printf("ping complete, takes %d us\n", (long)(end_time - start_time));

fail2:
    hal_mb_free_channel(mchan);
fail1:
    hal_mb_put_client(cl);

    return 0;
}

#if defined(WITH_LIB_CONSOLE)
#include <lib/console.h>
STATIC_COMMAND_START
STATIC_COMMAND("ping_vdsp", "test for short message sending",
               (console_cmd)&vdsp_ping_test)
#if 0
STATIC_COMMAND("avm_start", "fastavm start", (console_cmd)&avm_start)
STATIC_COMMAND("avm_stop",  "fastavm stop",  (console_cmd)&avm_stop)
#endif
STATIC_COMMAND_END(fastavm);
#endif

#if 0
APP_START(fastavm)
.entry = (app_entry)avm_start,
.stack_size = 8192,
APP_END
#endif
