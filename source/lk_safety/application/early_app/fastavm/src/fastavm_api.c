#include <v4l2.h>
#include "res.h"
#include <csi_hal.h>
#include <avm_app_csi.h>
#include <stdint.h>
#include <err.h>
#include <lk_wrapper.h>
#include <mbox_hal.h>
#include <image_cfg.h>
#include <heap.h>
#include "early_app_cfg.h"


#include "fastavm_api.h"
#include "avm_data_structure_def.h"

#include <disp_data_type.h>
#include <chip_res.h>
//#include <g2dlite_api.h>


int vdsp_load_firmware(uint32_t elf_base, uint32_t elf_len,
                       void *dummy_binary_base);
void vdsp_rst_stall(uint32_t vector_base);
void vdsp_go(void);

//extern uint8_t VDSP_FW_RUNTIME_MEM[];
#define VDSP_FW_RUNTIME_MEM (ap2p(VDSP_MEMBASE))

static fastAVM_t fastavm = {1280, 720, 25, 1, 0, NULL, NULL};

static uint8_t *avm_input = NULL;
static csi_instance_t *instance = NULL;

static void *mbox_handle;
static hal_mb_client_t cl;
static hal_mb_chan_t *mchan;
static event_t vdsp_signal;

enum avm_statemachine {
    avmstatus_idle = 0,
    avmstatus_inited,
    avmstatus_mappingtableready,
    avmstatus_running,
    avmstatus_paused,
};
static enum avm_statemachine avmstatus = avmstatus_idle;

extern int initstatus;

static int avm_csi_entry_api(void)
{
    struct v4l2_fract frame_interval;
    int ret = 0;

    while (initstatus != 2) {
        thread_sleep(50);
    }


    if (ret < 0) {
        USDBG("%s(): init error\n", __func__);
        return ret;
    }

    frame_interval.numerator = fastavm.numerator;
    frame_interval.denominator = fastavm.denominator;

    ret = avm_csi_config_anyRes(0, avm_input, fastavm.inputwidth,
                                fastavm.inputheight, frame_interval);

    if (ret < 0) {
        USDBG("%s(): config error\n", __func__);
        return ret;
    }

    ret = avm_csi_start(0);

    if (ret < 0) {
        USDBG("%s(): start error\n", __func__);
    }

    return ret;
}


void fastAVM_loadMappingTable(void *pMappingTable)
{
    fastavm.pMappingTable = pMappingTable;
}

void fastAVM_loadFastAVM(void *pelfbuffer, int elf_size)
{
    fastavm.pelfbuffer = pelfbuffer;
    fastavm.elf_size = elf_size;
}

void fastAVM_setCameraResolution(uint16_t width, uint16_t height)
{
    fastavm.inputwidth = width;
    fastavm.inputheight = height;
}

void fastAVM_getCameraResolution(uint16_t *width, uint16_t *height)
{
    *width = fastavm.inputwidth;
    *height = fastavm.inputheight;
}

void fastAVM_setFPS( uint32_t numerator, uint32_t denominator )
{
    fastavm.denominator = denominator;
    fastavm.numerator = numerator;
}

void fastAVM_getFPS( uint32_t *numerator, uint32_t *denominator )
{
    *denominator = fastavm.denominator;
    *numerator = fastavm.numerator;
}

static uint32_t timeforwaitmappingtable;
static void vdsp_callback(hal_mb_client_t cl, void *mssg, u16 len)
{
    event_t *signal = (event_t *)hal_mb_get_user(cl);
    fastavm.status.cycles  = ((uint32_t *)mssg)[0];
    timeforwaitmappingtable = ((uint32_t *)mssg)[1];
    //USDBG("vdsp response fps %d", fps);
    event_signal(signal, false);
}

static int vdsp_start_one_frame(hal_mb_chan_t *mchan, paddr_t pout, paddr_t pin)
{
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

static int vdsp_wait_interrupt(event_t *signal, int timeout)
{
    static int ret = -1;

    if (event_wait_timeout(signal, timeout) == ERR_TIMED_OUT) {
        USDBG("%s vdsp waiting interrupt timeout!\n", __func__);
        return ret;
    }

    event_unsignal(signal);
    return 0;
}

static void fastAVM_updatemappingtable(void)
{
    if (avmstatus_inited == avmstatus) {
        int ret = -1;
        ret = vdsp_start_one_frame(mchan, p2ap((uintptr_t)(fastavm.pMappingTable)),
                                   p2ap(0xFF));

        if (ret) {
            USDBG("vdsp send mappingtable failed!\n");
            return;
        }

        vdsp_wait_interrupt(&vdsp_signal, 100);
        avmstatus = avmstatus_mappingtableready;
        float preproctiming = (float)timeforwaitmappingtable / 748000000.0f;
        //USDBG("--->time for waitting mappingtable: %f\n",preproctiming);
    }
}

void fastAVM_init(void)
{
    thread_t *avm_csi_init_thread = thread_create("avm_csi_init",
                                    (thread_start_routine)avm_csi_init,
                                    0, DEFAULT_PRIORITY, DEFAULT_STACK_SIZE);
    thread_detach_and_resume(avm_csi_init_thread);
}

void fastAVM_start(void)
{
    int ret = 0;

    if (avmstatus_idle == avmstatus) {

        if (NULL == fastavm.pMappingTable) {
            USDBG("ERROR: MappingTable is NULL\n");
            return;
        }

        if (NULL == fastavm.pelfbuffer) {
            USDBG("ERROR: pelfbuffer is NULL\n");
            return;
        }

        hal_mb_create_handle(&mbox_handle, RES_MB_MB_MEM);

        hal_mb_cfg_t hal_cfg;

        if (mbox_handle != NULL) {
            hal_mb_init(mbox_handle, &hal_cfg);
        }

        cl = hal_mb_get_client();

        if (!cl) {
            USDBG("get mb cl failed %d failed\n");
            return;
        }

        mchan = hal_mb_request_channel(cl, true, vdsp_callback, IPCC_RRPOC_VDSP);

        if (!mchan) {
            hal_mb_put_client(cl);
            USDBG("request mb channel failed\n");
            goto FAIL1;
        }

        hal_mb_set_user(cl, &vdsp_signal);

        //prepare camera buffer
        if (NULL != avm_input) {
            free(avm_input);
            avm_input = NULL;
        }

        avm_input = memalign(0x1000,
                             CAMERA_MAX_BUF * IMG_COUNT * fastavm.inputwidth * fastavm.inputheight * 2);

        if (NULL == avm_input) {
            USDBG("ERROR: malloc input buffer failed\n");
            goto FAIL2;
        }

        /* init csi cameras */

        ret = avm_csi_entry_api();

        if (ret < 0) {
            USDBG("int csi failed.\n");
            goto FAIL3;
        }

        void *csi_handle = avm_csi_get_handle(0);

        if (!csi_handle) {
            USDBG(" get csi handle failed.\n");
            goto FAIL3;
        }

        instance = (csi_instance_t *)csi_handle;

        if (!instance->dev) {
            USDBG("get csi dev failed.\n");
            goto FAIL3;
        }

        vdsp_rst_stall(p2ap((paddr_t)VDSP_FW_RUNTIME_MEM));
        vdsp_load_firmware((uint32_t)fastavm.pelfbuffer, fastavm.elf_size, NULL);
        vdsp_go();
        //USDBG("---->vdsp_go\n");

        event_init(&vdsp_signal, false, 0);

        avmstatus = avmstatus_inited;
        return;

FAIL3:
        free(avm_input);
        avm_input = NULL;
FAIL2:
        hal_mb_free_channel(mchan);
FAIL1:
        hal_mb_put_client(cl);

    }

    return;
}

void fastAVM_pause(void)
{
    if (avmstatus_running != avmstatus) {
        avmstatus = avmstatus_paused;
    }
}

void fastAVM_stop(void)
{
    fastAVM_pause();
#if 0

    if (avmstatus_idle != avmstatus) {
        avm_csi_stop(0);

        if (NULL != avm_input) {
            free(avm_input);
            avm_input = NULL;
        }

        hal_mb_free_channel(mchan);
        hal_mb_put_client(cl);
        avmstatus = avmstatus_idle;
    }

#endif
}

int fastAVM_update_one_frame(uint8_t *pframebuffer, uint8_t * * pcamerabuffer)
{
    int ret = 0;

    if (NULL == pframebuffer) return -1;

    if (avmstatus_idle == avmstatus) {
        fastAVM_start();

        if (avmstatus_idle == avmstatus) {
            return -1;
        }
    }

    if (avmstatus_inited == avmstatus) {
        //USDBG("%s: line %d @%lu\n", __func__, __LINE__,current_time());
        fastAVM_updatemappingtable();

        if (avmstatus_inited == avmstatus) {
            return -1;
        }

        avmstatus = avmstatus_mappingtableready;
    }

    if (avmstatus_mappingtableready == avmstatus) {
        avmstatus = avmstatus_running;
    }

    if (avmstatus_running == avmstatus) {
        uint32_t pin;
        pin = (uintptr_t)avm_input;

        struct csi_image *img = instance->dev->ops.get_image(instance->dev, 0);


        if (!img || !img->enable) {
            USDBG("csi img is not available.\n");
            return -1;
        }

        struct csi_device *dev = img->csi;

        int n;


        int timeout_count = 0;

        while (ERR_TIMED_OUT == event_wait_timeout(&img->completion, 50)) {
            timeout_count++;

            if (timeout_count == 20) {
                USDBG("CSI wait timeout, do nothing.\n");
                return -1;
            }
        }

        n = CAMERA_BUF_POS(img->buf_pos);
        //n = ((n - 1) >= 0) ? (n - 1) : 2;
        // get img->id, img->addr
        //img->id, img->rgby_baddr[n]);
        USDBG("csi vid:%d, pos:%d, baddr:0x%x\n", img->id, n, img->rgby_baddr[n]);

        pin += n * IMG_COUNT * fastavm.inputwidth * fastavm.inputheight * 2;

        *pcamerabuffer = (uint8_t *)
                         pin;//avm_input + CAMERA_BUF_POS(img->buf_pos) * IMG_COUNT * fastavm.inputwidth * fastavm.inputheight * 2;

        uint8_t (*pout)[AVM_WIDTH * AVM_HEIGHT * 3];
        pout = (uint8_t (*)[AVM_WIDTH * AVM_HEIGHT * 3])pframebuffer;

        USDBG("++csi  end@%lu: pin = 0x%x\n", current_time(), pin);

        ///////////////////////////////G2DLite CSC YUYV2RGB

        ret = vdsp_start_one_frame(mchan, p2ap((uintptr_t)(pout)),
                                   p2ap((uintptr_t)(pin)));

        if (ret) {
            USDBG("vdsp start one frame failed!\n");
            return -1;
        }

        vdsp_wait_interrupt(&vdsp_signal, 100);
    }

    return 0;
}

fastAVM_context_t fastAVM_getstatus(void)
{
    return fastavm.status;
}

fastAVM_car_t fastAVM_getCarPosition(void)
{
    fastAVM_car_t carinfo;
    carinfo.width = 240;
    carinfo.length = 240;
    carinfo.wheeltrack = 240;
    carinfo.wheelbase = 200;
    carinfo.frontwheeloffset = 20;

    if (NULL != fastavm.pMappingTable) {
        uint16_t *pshortdata = (uint16_t *)(fastavm.pMappingTable);
        carinfo.width = *pshortdata++;
        carinfo.length = *pshortdata++;
        carinfo.wheeltrack = *pshortdata++;
        carinfo.wheelbase = *pshortdata++;
        carinfo.frontwheeloffset = *pshortdata++;
        return carinfo;
    }

    return carinfo;
}
