#include <stdint.h>
#include <string.h>
#include <Can.h>
/* #include <spi.h> */
#include "can_proxy.h"

/*
 * Special canid for time test, store the sys count (uint32)from/to can driver
 * into payload of the canframe at offset 4~7 and store the timestamp of
 * linux vircan driver in offset 0~3
 * sys counter is a global clock counter with 3MHz frequency.
 * set CONFIG_VIRCAN_LATENCY_TEST = 1 to enable
 */
#define CONFIG_VIRCAN_LATENCY_TEST	(0)

#if CONFIG_VIRCAN_LATENCY_TEST

#define CAN_ID_TIMESTAMP_TEST	(1U)
#define CAN_ID_MASK 			(0x000007FFU) /* standard frame format */

#include "__regs_base.h"
#include "reg.h"
static inline uint32_t get_sys_cnt(void)
{
    return readl(APB_SYS_CNT_RO_BASE);
}
#endif

void CanIf_RxIndication( uint16_t ControllerId, Can_IdType CanId,uint8_t CanDlc, const uint8_t *CanSduPtr)
{
    struct cp_can_frame *cpframe;

    __CP_CREATE_PFRAME(cp_if_frame_t,64,frame); //64B for CANFD frame
    frame->bus = cpifBusCan;
    frame->if_id = ControllerId;
    frame->len = CanDlc+sizeof(Can_IdType)+1/*CanDlc*/;

    /* pack into cp can frame */
    cpframe = (struct cp_can_frame *) &frame->data[0];
    cpframe->can_id = CanId;
    cpframe->len = CanDlc;
    memcpy(&cpframe->data[0],CanSduPtr,CanDlc);

#if CONFIG_VIRCAN_LATENCY_TEST
    if ((CAN_ID_MASK & CanId) == CAN_ID_TIMESTAMP_TEST) {
        u32 cnt = get_sys_cnt();
        memcpy(&frame->data[sizeof(Can_IdType)+5],&cnt,sizeof(cnt));
    }
#endif

    CPDBG("Can%d rx <0x%x> [%d] ", ControllerId, cpframe->can_id, cpframe->len);

    for(int i=0;i<cpframe->len;i++)
    {
        CPDBG("%x ",cpframe->data[i]);
    }
    CPDBG("\n");

    cp_if_frame_post(frame);
}

/* Std_ReturnType Can_Write(Can_HwHandleType Hth, const Can_PduType* PduInfo) */
int can_if_tx(struct cp_if_t* cpif,void* from, int len)
{
    Can_HwHandleType Hth = cpif->if_id * 7;

    /* data 0 ~ 3: id
     * data 4: length
     * data 5 ~ 13: payload
     */
    Can_PduType PduInfo = {0,};
    struct cp_can_frame *frame;

    frame = from;
    PduInfo.length = frame->len;
    PduInfo.id = frame->can_id;
    PduInfo.sdu = &frame->data[0];

#if CONFIG_VIRCAN_LATENCY_TEST
    if ((CAN_ID_MASK & frame->can_id) == CAN_ID_TIMESTAMP_TEST) {
        u32 cnt = get_sys_cnt();
        memcpy(&frame->data[4],&cnt,sizeof(cnt));
    }
#endif

    CPDBG("Can%d tx <0x%x> [%d] ", cpif->if_id, frame->can_id, frame->len);

    for(int i=0;i<frame->len;i++)
    {
        CPDBG("%x ",frame->data[i]);
    }

    if (Can_Write(Hth,&PduInfo) == E_OK)
        CPDBG("\n");
    else {
        CPDBG("  FAILED!\n");
    }

    return 0;
}

int can_if_init(struct cp_if_t* cpif)
{
extern const Can_ConfigType gCan_Config;

    Can_Init(&gCan_Config);
    Can_SetControllerMode(cpif->if_id, CAN_CS_STARTED);
    cpif->if_state = cpifStateRunning;

    return 0;
}

int can_if_rx_cb(struct cp_if_t* cpif, void* data, int len)
{
    // switch(cpif->if_id):
    // case CAN1:
    // cp_bundle_attach(TARGET_IVI_BUNDLE,data,len);
    //     ...
    // break;
    // ...
    cp_bundle_attach(TARGET_AP_BUNDLE,data,len);

    return len;
}






