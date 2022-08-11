/*
 * Copyright (c)  Semidrive
 */
#include <string.h>
#include <stdlib.h>
#include <reg.h>
#include <assert.h>
#include <debug.h>
#include <lib/reg.h>
#include <sys/types.h>
#include <platform/interrupts.h>
#include <kernel/thread.h>
#include <kernel/spinlock.h>
#include <kernel/mutex.h>
#include <lib/cbuf.h>

#include "__regs_base.h"

#ifdef SDPE
#include <stdlib.h>
#include <bits.h>
#include <sdpe/pdu.h>
#endif

#include "include/Can.h"

#ifdef SDPE
#ifdef SUPPORT_SDPE_RPC
#include "vcan_service.h"
#else
#include "vcan_cb.h"
#endif
#endif

/***************Internal macros***************/

#define CAN_WAKEUP_MASK 1U
#define CAN_BUSOFF_MASK 4U

#define CTRL2_RFFN_MASK 0x0F000000U
#define CTRL2_RFFN_SHIFT    24U

#define ERR_CBUF_LEN        512U    /* CBuf length must be power of 2. */
#define ERR_CBK_STACK_SIZE  1024U

#define DEBUG_PR_RX_FRAME(rx)   do { \
                                    dprintf(DBGV, "Received CAN frame:\n\tid = 0x%x\n\tlen = %d\n", \
                                            (rx)->id, (rx)->length); \
                                    for (uint16 i = 0U; i < (rx)->length; i++) { \
                                        dprintf(DBGV, "\tdata[%d] = 0x%x\n", i, (rx)->dataBuffer[i]); \
                                    } \
                                } while (0)

/*
 * The lock is used to protect registers writing,
 * make sure each writing is atmoic, and also to
 * protect other global resouces in the driver.
 */
#define CAN_LOCK(ControllerId)      mutex_acquire(&gCANCtrllerStatus[(ControllerId)].mutex)
#define CAN_UNLOCK(ControllerId)    mutex_release(&gCANCtrllerStatus[(ControllerId)].mutex)

/***************Struct definition***************/

typedef struct CanIntStatus {
    uint32 errIntMask;
    uint32 mb_0_31_IntMask;
    uint32 mb_32_63_IntMask;
} CanIntStatus_t;

typedef enum CanTxMBState {
    IDLE = 0,
    BUSY
} CanTxMBState_t;

typedef union CanMBStatus {
    uint32 hrh;
    struct {
        spin_lock_t lock;
        CanTxMBState_t state;
        uint32 txPduId;
    };
} CanMBStatus_t;

struct CanControllerStatus {
    uint32 baseAddr;
    mutex_t mutex;
    uint8 intDisableCnt;
    Can_ControllerStateType state;
    CanIntStatus_t intMask;
    CanMBStatus_t mb[FLEXCAN_HAS_MESSAGE_BUFFER_MAX_NUMBER];
};

#ifdef SDPE
typedef enum err_event {
    BUSOFF,
    WAKEUP
} err_event_t;

struct can_err_msg {
    uint8 controller_id;
    err_event_t err;
};

struct CanErrCbk {
    thread_t *thread;
    cbuf_t cbuf;
};
#endif

struct can_sleep_scr {
    uint32 base;
    uint8 stop_pos;
    uint8 doze_pos;
};

/***************Internal function prototypes***************/

static void __critical_code__ Can_Notify(uint8 ControllerId, flexcan_status_t status,
                       uint32 result, void* userData);
static void Can_ActivateRxMB(struct Can_MBConfig* mbCfg);
static void Can_ActivateRxFIFO(struct Can_RxFIFOConfig* rxFifoCfg);
static void Can_ConfigTxMB(struct Can_MBConfig* mbCfg);
static inline void Can_EnableRxMB(uint8 Controller);
static inline void Can_EnableRxFIFO(uint8 Controller);
static inline void Can_EnableTxMB(uint8 Controller);
static uint8 Can_GetRxFifoOccupiedMBNum(uint8 ControllerId);
static inline uint8 Can_FindCfgIndex(uint8 ControllerId);
static uint8 __critical_code__ Can_GetMBDataSize(uint8 ControllerId, uint8 mbId);
static inline void Can_ControllerRecovery(uint8 ControllerId);
static inline bool Can_DozeModeEnabled(uint8 ControllerId);
static inline void Can_SetSleepRequest(uint8 ControllerId, bool Request);
#ifdef SDPE
static int __critical_code__ Can_ErrCbkThread(void *arg);
static void Can_ErrCbkHandlerInit(void);
static void __critical_code__ Can_SendErrMsg(uint8 ControllerId, err_event_t msg);
static void Can_ReleaseErrMsgBuffer(void);
#endif

/***************Internal variable definitions***************/

static Can_StatusType gCANDriverStatus = CAN_UNINIT;
static const Can_ConfigType* gCANConfig __critical_data__;
static struct CanControllerStatus gCANCtrllerStatus[MAX_FLEXCAN_CH] __critical_data__;

#ifdef SDPE
static struct CanErrCbk gCanErrCbk __critical_data__;
#endif

static const struct can_sleep_scr gCanSleepSCR[MAX_FLEXCAN_CH] = {
    [CAN1] = {APB_SCR_SAF_BASE + (0x600U << 10), 4U, 5U},
    [CAN2] = {APB_SCR_SAF_BASE + (0x604U << 10), 4U, 5U},
    [CAN3] = {APB_SCR_SAF_BASE + (0x608U << 10), 4U, 5U},
    [CAN4] = {APB_SCR_SAF_BASE + (0x60CU << 10), 4U, 5U},

#if MAX_FLEXCAN_CH > 4
    [CAN5] = {APB_SCR_SEC_BASE + (0x600U << 10), 4U, 5U},
    [CAN6] = {APB_SCR_SEC_BASE + (0x604U << 10), 4U, 5U},
    [CAN7] = {APB_SCR_SEC_BASE + (0x608U << 10), 4U, 5U},
    [CAN8] = {APB_SCR_SEC_BASE + (0x60CU << 10), 4U, 5U},

#if MAX_FLEXCAN_CH > 8
    [CAN9] = {APB_SCR_SEC_BASE + (0x694U << 10), 4U, 5U},
    [CAN10] = {APB_SCR_SEC_BASE + (0x698U << 10), 4U, 5U},
    [CAN11] = {APB_SCR_SEC_BASE + (0x69CU << 10), 4U, 5U},
    [CAN12] = {APB_SCR_SEC_BASE + (0x6A0U << 10), 4U, 5U},
    [CAN13] = {APB_SCR_SEC_BASE + (0x6A4U << 10), 4U, 5U},
    [CAN14] = {APB_SCR_SEC_BASE + (0x6A8U << 10), 4U, 5U},
    [CAN15] = {APB_SCR_SEC_BASE + (0x6ACU << 10), 4U, 5U},
    [CAN16] = {APB_SCR_SEC_BASE + (0x6B0U << 10), 4U, 5U},
    [CAN17] = {APB_SCR_SEC_BASE + (0x6B4U << 10), 4U, 5U},
    [CAN18] = {APB_SCR_SEC_BASE + (0x6B8U << 10), 4U, 5U},
    [CAN19] = {APB_SCR_SEC_BASE + (0x6BCU << 10), 4U, 5U},
    [CAN20] = {APB_SCR_SEC_BASE + (0x6C0U << 10), 4U, 5U},
#endif
#endif
};

/***************Function implementation***************/

#ifndef SDPE
__WEAK void CanIf_RxIndication(uint16_t ControllerId, Can_IdType CanId,
                               uint8_t CanDlc, const uint8_t *CanSduPtr)
{
    return;
}

__WEAK void CanIf_TxComfirmation(PduIdType CanTxPduId)
{
    return;
}
#endif

static void Can_Notify(uint8 ControllerId, flexcan_status_t status,
                       uint32 result, void* userData)
{
    flexcan_frame_t* buf = (flexcan_frame_t*)userData;
    struct CanControllerStatus *ctrller = &gCANCtrllerStatus[ControllerId];
    CanMBStatus_t *mb;
#ifdef SDPE
    extern void canif_rx_cb(bus_id_t can_bus, uint32 hrh, prot_id_t prot_id,
                            uint16 len, uint8 *data);
    extern void canif_tx_cb(pdu_id_t pdu_id);
    extern void canif_err_cb(bus_id_t can_bus);
#else
    extern void CanIf_RxIndication(uint16 ControllerId, Can_IdType CanId,
                                   uint8 CanDlc, const uint8 *CanSduPtr);
    extern void CanIf_TxComfirmation(PduIdType CanTxPduId);
#endif

    dprintf(DBGV,
            "\nCalling Can_Notify, controllerId=%d, status=%d, result=%d, userData=%d\n",
            ControllerId, status, result, (uint32)userData);

    switch (status) {
        case FLEXCAN_RX_IDLE:
            mb = &ctrller->mb[result];
            DEBUG_PR_RX_FRAME(buf);
        #ifndef SDPE
            CanIf_RxIndication(ControllerId,
                               buf->id | (buf->format << 31) | (buf->isCANFDFrame << 30),
                               buf->length, buf->dataBuffer);
        #else
            canif_rx_cb(ControllerId, mb->hrh,
                        buf->id | (buf->format << 31) | (buf->isCANFDFrame << 30) |
                        (buf->type << 29),
                        buf->length, buf->dataBuffer);
        #endif

            break;

        case FLEXCAN_RX_FIFO_IDLE:
            mb = &ctrller->mb[0];
            DEBUG_PR_RX_FRAME(buf);
        #ifndef SDPE
            CanIf_RxIndication(ControllerId,
                               buf->id | (buf->format << 31),
                               buf->length, buf->dataBuffer);
        #else
            canif_rx_cb(ControllerId, mb->hrh,
                        buf->id | (buf->format << 31) | (buf->type << 29),
                        buf->length, buf->dataBuffer);
        #endif

            break;

        case FLEXCAN_TX_IDLE:
            smp_rmb();
            mb = &ctrller->mb[result];
            uint32 tx_pdu_id = mb->txPduId;
            mb->state = IDLE;
        #ifndef SDPE
            CanIf_TxComfirmation(tx_pdu_id);
        #else
            canif_tx_cb(tx_pdu_id);
        #endif

            break;

        case FLEXCAN_ERROR_STATUS:
            if ((result & CAN_WAKEUP_MASK) != 0U) {
                ctrller->state = CAN_CS_STOPPED;
                Can_SetSleepRequest(ControllerId, false);
                /* shall not further process L-PDU after a wake-up. */
                flexcan_freeze(ControllerId, true);
            #ifdef SDPE
                Can_SendErrMsg(ControllerId, WAKEUP);
            #endif
            }

            if ((result & CAN_BUSOFF_MASK) != 0U) {
                ctrller->state = CAN_CS_STOPPED;
            #ifdef SDPE
                Can_SendErrMsg(ControllerId, BUSOFF);
                canif_err_cb(ControllerId);
            #endif
            }

            break;

        default:
            break;
    }
}

static void Can_ActivateRxMB(struct Can_MBConfig* mbCfg)
{
    flexcan_rx_mb_config_t rxMBCfg;
    uint8 can_ch = mbCfg->controllerId;
    uint8 messageBufferId = mbCfg->mbId;
    flexcan_mb_transfer_t transfer = {NULL,
                                      messageBufferId
                                     };

    rxMBCfg.id = mbCfg->rx.frameId;
    rxMBCfg.format = mbCfg->rx.frameFormat;
    rxMBCfg.type = mbCfg->rx.frameType;

    if (gCANConfig->ctrllerCfg[can_ch].flexcanCfg.enableIndividMask == true) {
        dprintf(INFO, "\nSet individual MB filter mask for CAN %d MB %d\n",
                can_ch, messageBufferId);
        flexcan_set_rx_individual_mask(can_ch, messageBufferId,
                                       mbCfg->rx.rxIDFilterMask);
    }

    dprintf(INFO,
            "\nSet Rx MB, ch=%d, messageBufferId=%d, messageID=%d, message format=%d\n",
            can_ch, messageBufferId, rxMBCfg.id, rxMBCfg.format);
    flexcan_set_rx_mb_config(can_ch, messageBufferId,
                             &rxMBCfg);

#ifndef SDPE
    if (!mbCfg->polling)
#endif
        flexcan_receive_nonblocking(can_ch, &transfer);
}

static void Can_ActivateRxFIFO(struct Can_RxFIFOConfig* rxFifoCfg)
{
    uint8 can_ch = rxFifoCfg->controllerId;
    uint8 idFilterIdx;
    flexcan_fifo_transfer_t transfer = {NULL};

    dprintf(INFO, "\nSet Rx FIFO, ch=%d\n", can_ch);
    flexcan_set_rx_fifo_config(can_ch,
                               &rxFifoCfg->flexcanRxFIFOCfg);

    if (gCANConfig->ctrllerCfg[can_ch].flexcanCfg.enableIndividMask == true) {
        for (idFilterIdx = 0U;
             (idFilterIdx < rxFifoCfg->flexcanRxFIFOCfg.idFilterNum) &&
             (idFilterIdx < Can_GetRxFifoOccupiedMBNum(can_ch));
             idFilterIdx++) {
            flexcan_rx_fifo_filter_table_t *filter_tab =
                            &rxFifoCfg->flexcanRxFIFOCfg.filter_tab[idFilterIdx];
            dprintf(INFO,
                    "\nSet individual FIFO filter mask for CAN %d Filter Table %d\n",
                    can_ch, idFilterIdx);
            flexcan_set_rx_individual_mask(can_ch, idFilterIdx, filter_tab->filter_mask);
        }
    }

#ifndef SDPE
    if (!rxFifoCfg->polling)
#endif
        flexcan_receive_fifo_nonblocking(can_ch, &transfer);
}

static void Can_ConfigTxMB(struct Can_MBConfig* mbCfg)
{
#ifdef SDPE
    bool tx_by_interrupt = true;
#else
    bool tx_by_interrupt = !mbCfg->polling;
#endif
    uint8 mb_id = mbCfg->mbId;

#ifndef SDPE
    for (size_t i = 0; i < mbCfg->CanHwObjectCount; i++) {
#endif
        flexcan_set_tx_mb_config(mbCfg->controllerId, mb_id,
                                 tx_by_interrupt);
#ifndef SDPE
        mb_id++;
    }
#endif
}

static inline void Can_EnableRxMB(uint8 Controller)
{
    for (uint16 idx = 0U; idx < gCANConfig->rxCount; idx++) {
        if (gCANConfig->rxMBCfg[idx].controllerId != Controller)
            continue;

        uint8 mbIdx = gCANConfig->rxMBCfg[idx].mbId;
        gCANCtrllerStatus[Controller].mb[mbIdx].hrh =
                                            gCANConfig->rxMBCfg[idx].hwObjId;

        Can_ActivateRxMB(&gCANConfig->rxMBCfg[idx]);

    #ifndef SDPE
        if (!gCANConfig->rxMBCfg[idx].polling) {
    #endif
            if (mbIdx <= 31U) {
                gCANCtrllerStatus[Controller].intMask.mb_0_31_IntMask |=
                                                                1U << mbIdx;
            }
            else {
                gCANCtrllerStatus[Controller].intMask.mb_32_63_IntMask |=
                                                        1U << (mbIdx - 32U);
            }
    #ifndef SDPE
        }
    #endif
    }
}

static inline void Can_EnableRxFIFO(uint8 Controller)
{
    for (uint16 idx = 0U; idx < gCANConfig->rxFifoCount; idx++) {
        if (gCANConfig->rxFIFOCfg[idx].controllerId == Controller) {
            gCANCtrllerStatus[Controller].mb[0].hrh =
                                gCANConfig->rxFIFOCfg[idx].hwObjId;

            Can_ActivateRxFIFO(&gCANConfig->rxFIFOCfg[idx]);

        #ifndef SDPE
            if (!gCANConfig->rxFIFOCfg[idx].polling)
        #endif
                gCANCtrllerStatus[Controller].intMask.mb_0_31_IntMask |=
                                            (1U << 5)|(1U << 6)|(1U << 7);
        }
    }
}

static inline void Can_EnableTxMB(uint8 Controller)
{
    for (uint16 idx = 0U; idx < gCANConfig->txCount; idx++) {
        if (gCANConfig->txMBCfg[idx].controllerId != Controller)
            continue;

        uint8 mbIdx = gCANConfig->txMBCfg[idx].mbId;

        CanMBStatus_t *mb_sts = &gCANCtrllerStatus[Controller].mb[mbIdx];
        mb_sts->state = IDLE;
        spin_lock_init(&mb_sts->lock);

        Can_ConfigTxMB(&gCANConfig->txMBCfg[idx]);

    #ifndef SDPE
        if (!gCANConfig->txMBCfg[idx].polling) {
            for (size_t i = 0; i < gCANConfig->txMBCfg[idx].CanHwObjectCount; i++) {
    #endif
                if (mbIdx <= 31U) {
                    gCANCtrllerStatus[Controller].intMask.mb_0_31_IntMask |=
                                                                    1U << mbIdx;
                }
                else {
                    gCANCtrllerStatus[Controller].intMask.mb_32_63_IntMask |=
                                                            1U << (mbIdx - 32U);
                }
    #ifndef SDPE
                mbIdx++;
            }
        }
    #endif
    }
}

static uint8 Can_GetRxFifoOccupiedMBNum(uint8 ControllerId)
{
    uint8 ret_val = readl(gCANCtrllerStatus[ControllerId].baseAddr + 0x34U);
    ret_val = (ret_val & CTRL2_RFFN_MASK) >> CTRL2_RFFN_SHIFT;
    ret_val = (ret_val + 1U) * 8U;

    return ret_val;
}

static inline uint8 Can_FindCfgIndex(uint8 ControllerId)
{
    uint8 idx;

    for (idx = 0U; idx < gCANConfig->controllerCount; idx++) {
        if (gCANConfig->ctrllerCfg[idx].controllerId ==
                ControllerId) {
            break;
        }
    }

    return idx;
}

static uint8 Can_GetMBDataSize(uint8 ControllerId, uint8 mbId)
{
    uint8 ret_data_size;
    flexcan_config_t* flexcan_cfg =
                    &gCANConfig->ctrllerCfg[ControllerId].flexcanCfg;
    if (flexcan_cfg->enableCANFD == false) {
        ret_data_size = 8U;
    }
    else {
        flexcan_fd_data_size_t r0_data_size = flexcan_cfg->can_fd_cfg.r0_mb_data_size;
        flexcan_fd_data_size_t r1_data_size = flexcan_cfg->can_fd_cfg.r1_mb_data_size;
        uint8 r0_max_mb_id;
        uint8 canfd_mb_data_size[4] = {
            [CAN_FD_8BYTES_PER_MB] = 8U,
            [CAN_FD_16BYTES_PER_MB] = 16U,
            [CAN_FD_32BYTES_PER_MB] = 32U,
            [CAN_FD_64BYTES_PER_MB] = 64U
        };

        if (r0_data_size == CAN_FD_8BYTES_PER_MB) {
            r0_max_mb_id = 31U;
        }
        else if (r0_data_size == CAN_FD_16BYTES_PER_MB) {
            r0_max_mb_id = 20U;
        }
        else if (r0_data_size == CAN_FD_32BYTES_PER_MB) {
            r0_max_mb_id = 11U;
        }
        else {
            r0_max_mb_id = 6U;
        }

        if(mbId <= r0_max_mb_id) {
            ret_data_size = canfd_mb_data_size[r0_data_size];
        }
        else {
            ret_data_size = canfd_mb_data_size[r1_data_size];
        }
    }

    return ret_data_size;
}

static inline void Can_ControllerRecovery(uint8 ControllerId)
{
    if (gCANCtrllerStatus[ControllerId].state == CAN_CS_SLEEP) {
        Can_SetSleepRequest(ControllerId, false);
    }

    flexcan_deinit(ControllerId);
    flexcan_init(ControllerId,
                    &gCANConfig->ctrllerCfg[ControllerId].flexcanCfg);

    /* Interrupts are enabled automatically when recovered. */
    gCANCtrllerStatus[ControllerId].intDisableCnt = 0U;
}

static inline bool Can_DozeModeEnabled(uint8 ControllerId)
{
    #define DOZE_EN_MASK    0x00040000U

    if (readl(gCANCtrllerStatus[ControllerId].baseAddr) & DOZE_EN_MASK) {
        return true;
    }
    else {
        return false;
    }
}

static inline void Can_SetSleepRequest(uint8 ControllerId, bool Request)
{
    /* Safety domain SCR not accessible now, CAN1-4 sleep request
     * is sent by safety.
     */
    if (ControllerId < 4)
        return;

    const struct can_sleep_scr *s = &gCanSleepSCR[ControllerId];

    if (Can_DozeModeEnabled(ControllerId)) {
        RMWREG32(s->base, s->doze_pos, 1, !!Request);
    }
    else {
        RMWREG32(s->base, s->stop_pos, 1, !!Request);
    }
}

#ifdef SDPE
static int Can_ErrCbkThread(void *arg)
{
    struct can_err_msg buf;

    while (true) {
        uint8_t *p = (uint8_t *)&buf;
        size_t len = 0;

        /* Read error message. Might block. */
        while (len < sizeof(buf)) {
            len += cbuf_read(&gCanErrCbk.cbuf, p + len, sizeof(buf) - len, true);
        }

        /* Process the error. */
        switch (buf.err) {
        case BUSOFF:
            virCan_ControllerBusOff(buf.controller_id);
            break;

        case WAKEUP:
            virCan_ControllerWakeUp(buf.controller_id);
            break;

        default:
            break;
        }
    }

    return 0;
}

static void Can_ErrCbkHandlerInit(void)
{
    if (!gCanErrCbk.thread) {
        void *buf = malloc(ERR_CBUF_LEN);
        ASSERT(buf);
        cbuf_initialize_etc(&gCanErrCbk.cbuf, ERR_CBUF_LEN, buf);

        /* Reporting busoff/wakeup by RPC may block, this must not be done
         * in ISR context. So here create error callback thread to report
         * busoff, wakeup, etc. to vcan.
         */
        gCanErrCbk.thread = thread_create("can error cbk", Can_ErrCbkThread,
                            NULL, DEFAULT_PRIORITY - 1U, ERR_CBK_STACK_SIZE);
        ASSERT(gCanErrCbk.thread);
        thread_detach_and_resume(gCanErrCbk.thread);
    }
}

static void Can_SendErrMsg(uint8 ControllerId, err_event_t msg)
{
    struct can_err_msg err_msg = {ControllerId, msg};
    cbuf_write(&gCanErrCbk.cbuf, &err_msg, sizeof(struct can_err_msg), false);
}

static void Can_ReleaseErrMsgBuffer(void)
{
    cbuf_deinitialize(&gCanErrCbk.cbuf);
}
#endif

static void Can_ReorderHth(const Can_ConfigType* Config)
{
    uint16 hth_cnt = Config->txCount;
    Can_MBConfig *hth_cfg = Config->txMBCfg;

    for (uint16 i = 1U; i < hth_cnt; i++) {
        Can_MBConfig temp;
        for (uint16 j = 0U; j < hth_cnt - i; j++) {
            if (hth_cfg[j].hwObjId > hth_cfg[j + 1U].hwObjId) {
                temp = hth_cfg[j];
                hth_cfg[j] = hth_cfg[j + 1U];
                hth_cfg[j + 1U] = temp;
            }
        }
    }

    dprintf(DBGV, "%s: after reorder\n", __func__);
    for (uint16 i = 0U; i < hth_cnt; i++) {
        dprintf(DBGV, "Hth %d: %d\n", i, hth_cfg[i].hwObjId);
    }
}

void Can_Init(const Can_ConfigType* Config)
{
    uint8 idx;
    uint8 can_ch;
    struct CanControllerStatus *ctrller;

    static flexcan_handle_t handle[MAX_FLEXCAN_CH] __critical_data__;

    if ((gCANDriverStatus == CAN_UNINIT) && (Config != NULL)) {
        /* Initialize CAN controllers. */
        for (idx = 0U; idx < Config->controllerCount; idx++) {
            can_ch = Config->ctrllerCfg[idx].controllerId;
            ctrller = &gCANCtrllerStatus[can_ch];

            ctrller->baseAddr = p2v(Config->ctrllerCfg[idx].baseAddr);

            flexcan_create_handle(can_ch,
                                  (void*)ctrller->baseAddr,
                                  &handle[idx],
                                  (flexcan_transfer_callback_t)Can_Notify,
                                  (void*)(uint32)idx);
            flexcan_init(can_ch, &Config->ctrllerCfg[idx].flexcanCfg);

            /* Register interrupt. */
            register_int_handler(Config->ctrllerCfg[idx].irq_num, flexcan_irq_handler, (void*)(addr_t)can_ch);
            unmask_interrupt(Config->ctrllerCfg[idx].irq_num);

            mutex_init(&ctrller->mutex);

            ctrller->intMask.errIntMask = FLEXCAN_BusOffInterruptEnable
                                        #if EN_ERR_INT
                                            | FLEXCAN_ErrorInterruptEnable
                                        #endif
                                        #if EN_WARNING_INT
                                            | FLEXCAN_RxWarningInterruptEnable | FLEXCAN_TxWarningInterruptEnable
                                        #endif
                                        #if EN_WAKE_UP_INT
                                            | FLEXCAN_WakeUpInterruptEnable
                                        #endif
                                            ;
            ctrller->state = CAN_CS_INIT;
        }

    #ifdef SDPE
        Can_ErrCbkHandlerInit();
    #endif

        gCANConfig = Config;
        gCANDriverStatus = CAN_READY;
    }

    /* Reorder Hth configuration in ascending order
     * so that binary search can be used in Can_Write
     * when searching Hth.
     */
    Can_ReorderHth(Config);
}

void Can_DeInit(void)
{
    if (gCANDriverStatus != CAN_READY) {
        return;
    }

    for (uint8 insIdx = CAN1; insIdx < MAX_FLEXCAN_CH; insIdx++) {
        if (gCANCtrllerStatus[insIdx].state == CAN_CS_UNINIT) {
            continue;
        }

        if (gCANCtrllerStatus[insIdx].state != CAN_CS_STOPPED) {
            Can_SetControllerMode(insIdx, CAN_CS_STOPPED);
        }

        /* Wait for possible ongoing MB reading finished. */
        spin(100U);

        flexcan_deinit(insIdx);

        if (gCANCtrllerStatus[insIdx].state == CAN_CS_SLEEP) {
            Can_SetSleepRequest(insIdx, false);
        }

        mutex_destroy(&gCANCtrllerStatus[insIdx].mutex);

        gCANCtrllerStatus[insIdx].state = CAN_CS_UNINIT;
    }

    memset(&gCANCtrllerStatus, 0, sizeof(gCANCtrllerStatus));
    gCANConfig = NULL;
    gCANDriverStatus = CAN_UNINIT;
}

Std_ReturnType Can_SetBaudrate(uint8 Controller, uint16 BaudRateConfigID)
{
    Std_ReturnType retVal = E_OK;
    uint8 cfgIdx;

    if ((gCANDriverStatus == CAN_UNINIT) ||
        (BaudRateConfigID >= gCANConfig->baudRateCfgCount) ||
        (gCANConfig->baudRateCfg[BaudRateConfigID].controllerId != Controller)) {
        retVal = E_NOT_OK;
    }
    else {
        cfgIdx = Can_FindCfgIndex(Controller);

        if (cfgIdx >= gCANConfig->controllerCount) {
            retVal = E_NOT_OK;
        }
        else {
            CAN_LOCK(Controller);

            if (gCANConfig->ctrllerCfg[cfgIdx].flexcanCfg.enableCANFD == false) {
                flexcan_classic_set_timing_config(Controller,
                                                  &gCANConfig->baudRateCfg[BaudRateConfigID].nomianlBitTimingCfg);
            }
            else {
                flexcan_fd_set_timing_config(Controller,
                                             &gCANConfig->baudRateCfg[BaudRateConfigID].nomianlBitTimingCfg,
                                             &gCANConfig->baudRateCfg[BaudRateConfigID].dataBitTimingCfg);
            }

            CAN_UNLOCK(Controller);
        }
    }

    return retVal;
}

Std_ReturnType Can_SetControllerMode(uint8 Controller,
                                     Can_ControllerStateType Transition)
{
    Std_ReturnType retVal =  E_OK;
    struct CanControllerStatus *ctrller = &gCANCtrllerStatus[Controller];

    if (gCANDriverStatus != CAN_READY) {
        return E_NOT_OK;
    }

    if (ctrller->state == Transition) {
        dprintf(DBGV, "CAN %d MCR 0x%x\n", Controller,
                readl(ctrller->baseAddr));
        dprintf(DBGV, "CAN %d ESR1 0x%x\n", Controller,
                readl(ctrller->baseAddr + 0x20));
        return E_OK;
    }

    CAN_LOCK(Controller);

    switch (Transition) {
        case CAN_CS_STARTED:
            if (ctrller->state != CAN_CS_INIT) {
                Can_ControllerRecovery(Controller);
            }

            if (FLEXCAN_SUCCESS == flexcan_freeze(Controller, false)) {
                Can_EnableRxFIFO(Controller);
                Can_EnableRxMB(Controller);
                Can_EnableTxMB(Controller);

                ctrller->state = CAN_CS_STARTED;
            }
            else {
                retVal = E_NOT_OK;
            }
            break;

        case CAN_CS_STOPPED:
            if (ctrller->state == CAN_CS_SLEEP) {
                /* Make freeze possible. */
                Can_SetSleepRequest(Controller, false);
            }

            if (FLEXCAN_SUCCESS == flexcan_freeze(Controller, true)) {
                ctrller->state = CAN_CS_STOPPED;
            }
            else {
                if (ctrller->state == CAN_CS_SLEEP) {
                    /* Restore sleep request. */
                    Can_SetSleepRequest(Controller, true);
                }
                retVal = E_NOT_OK;
            }
            break;

        case CAN_CS_SLEEP:
            Can_SetSleepRequest(Controller, true);
            ctrller->state = CAN_CS_SLEEP;
            break;

        case CAN_CS_WAKEUP:
            if (ctrller->state == CAN_CS_SLEEP) {
                Can_SetSleepRequest(Controller, false);
                ctrller->state = CAN_CS_STOPPED;
            }
            else if (ctrller->state != CAN_CS_STOPPED) {
                retVal = E_NOT_OK;
            }
            break;

        default:
            retVal = E_NOT_OK;
            break;
    }

    CAN_UNLOCK(Controller);

    return retVal;
}

void Can_DisableControllerInterrupts(uint8 Controller)
{
    CAN_LOCK(Controller);

    struct CanControllerStatus *ctrller = &gCANCtrllerStatus[Controller];

    /* Disable error and wake up interrupts. */
    flexcan_disable_interrupts(Controller,
                               FLEXCAN_BusOffInterruptEnable | FLEXCAN_ErrorInterruptEnable |
                               FLEXCAN_RxWarningInterruptEnable | FLEXCAN_TxWarningInterruptEnable |
                               FLEXCAN_WakeUpInterruptEnable);
    /* Disable MB0-31 interrupts. */
    writel(0U, ctrller->baseAddr + 0x28U);
    /* Disable MB31-63 interrupts. */
    writel(0U, ctrller->baseAddr + 0x24U);
    /* Increment interrupt mask counter. */
    ctrller->intDisableCnt++;

    CAN_UNLOCK(Controller);
}

void Can_EnableControllerInterrupts(uint8 Controller)
{
    CAN_LOCK(Controller);

    struct CanControllerStatus *ctrller = &gCANCtrllerStatus[Controller];

    /* The function Can_EnableControllerInterrupts shall perform no action
       when Can_DisableControllerInterrupts has not been called before. */
    if (ctrller->intDisableCnt > 0U) {
        ctrller->intDisableCnt--;

        if (ctrller->intDisableCnt == 0U) {
            /* Enable error and wake up interrupts if allowed. */
            flexcan_enable_interrupts(Controller, ctrller->intMask.errIntMask);
            /* Enable MB0-31 interrupt if allowed. */
            writel(ctrller->intMask.mb_0_31_IntMask, ctrller->baseAddr + 0x28U);
            /* Enable MB31-63 interrupt if allowed. */
            writel(ctrller->intMask.mb_32_63_IntMask, ctrller->baseAddr + 0x24U);
        }
    }

    CAN_UNLOCK(Controller);
}

void Can_CheckWakeup(uint8 Controller)
{
    //TODO: Implemented in the future.
}

Std_ReturnType Can_GetControllerErrorState(uint8 ControllerId,
        Can_ErrorStateType* ErrorStatePtr)
{
    Std_ReturnType retVal = E_NOT_OK;
    uint8 errorState;

    if ((ControllerId < MAX_FLEXCAN_CH) && (ErrorStatePtr != NULL)) {
        /* Get ESR1 value. */
        errorState = readl(gCANCtrllerStatus[ControllerId].baseAddr + 0x20U);
        /* Get the value of FLTOCNF field. */
        errorState = (errorState >> 4) & 3U;

        switch (errorState) {
            case CAN_ERROR_ACTIVE:
                *ErrorStatePtr = CAN_ERRORSTATE_ACTIVE;
                break;

            case CAN_ERROR_PASSIVE:
                *ErrorStatePtr = CAN_ERRORSTATE_PASSIVE;
                break;

            case CAN_BUS_OFF:
                *ErrorStatePtr = CAN_ERRORSTATE_BUSOFF;
                break;
        }

        retVal = E_OK;
    }

    return retVal;
}

Std_ReturnType Can_GetControllerMode(uint8 Controller,
                                     Can_ControllerStateType* ControllerModePtr)
{
    Std_ReturnType retVal = E_NOT_OK;

    if ((Controller < MAX_FLEXCAN_CH) && (ControllerModePtr != NULL)) {
        *ControllerModePtr = gCANCtrllerStatus[Controller].state;
        retVal = E_OK;
    }

    return retVal;
}

int Can_BinarySearchHth(Can_HwHandleType Hth)
{
    if (!gCANConfig) {
        /* CAN not initialized or had been de-initialized,
         * return invalid value directly.
         */
        return -1;
    }

    int left = 0, right = gCANConfig->txCount - 1;
    int mid;

    while (left <= right) {
        mid = (left + right) / 2;

        if (Hth < gCANConfig->txMBCfg[mid].hwObjId)
            right = mid - 1;
        else if (Hth > gCANConfig->txMBCfg[mid].hwObjId)
            left = mid + 1;
        else
            return mid;
    }

    return -1;
}

static Std_ReturnType Can_SendAsync(uint8 ControllerId, uint8 MBId,
                                    uint8 PaddingValue,
                                    const Can_PduType *PduInfo)
{
    flexcan_frame_t txFrame;
    flexcan_mb_transfer_t transfer;
    uint8 id_type = GET_ID_TYPE(PduInfo->id);

    txFrame.length = PduInfo->length;
    /* Set Tx frame type. AutoSAR CAN doesn't support remote frame. */
    txFrame.type = FLEXCAN_FrameTypeData;

    /* Set Tx frame ID type according the 2 most significant bits of PDU ID. */
    if ((id_type == STANDARD_CAN) || (id_type == STANDARD_CAN_FD)) {
        txFrame.format = FLEXCAN_STANDARD_FRAME;
    }
    else {
        txFrame.format = FLEXCAN_EXTEND_FRAME;
    }

    /* Set Tx frame mode according the 2 most significant bits of PDU ID. */
    if ((id_type == STANDARD_CAN) || (id_type == EXTENDED_CAN)) {
        txFrame.isCANFDFrame = false;
        txFrame.isCANFDBrsEn = false;

        if (txFrame.length > 8U) {
            txFrame.length = 8U;
        }
    }
    else {
        txFrame.isCANFDFrame = true;
        txFrame.isCANFDBrsEn = true;

        uint8 mbDataSize = Can_GetMBDataSize(ControllerId, MBId);
        if (txFrame.length > mbDataSize) {
            txFrame.length = mbDataSize;
        }
    }

    /* Set Tx frame ID. */
    txFrame.id = GET_CAN_ID(PduInfo->id);
    /* Set Tx frame buffer. */
    txFrame.dataBuffer = PduInfo->sdu;

    /* Construct Tx transfer. */
    transfer.pFrame = &txFrame;
    transfer.mbIdx = MBId;

    if (FLEXCAN_SUCCESS != flexcan_send_nonblocking(ControllerId, &transfer,
                                                    PaddingValue)) {
        dprintf(INFO, "Send non blocking failed\n");
        return E_NOT_OK;
    }
    else {
        return E_OK;
    }
}

Std_ReturnType Can_Write(Can_HwHandleType Hth, const Can_PduType *PduInfo)
{
    Std_ReturnType ret = E_NOT_OK;
    int hth_idx = Can_BinarySearchHth(Hth);

    if (hth_idx == -1)
        return ret;

    struct Can_MBConfig *hth_cfg = &gCANConfig->txMBCfg[hth_idx];
    struct CanControllerStatus *ctrller_sts =
                        &gCANCtrllerStatus[hth_cfg->controllerId];

    if (ctrller_sts->state != CAN_CS_STARTED)
        return ret;

    uint8 mb_id = hth_cfg->mbId;

#ifndef SDPE
    for (size_t i = 0; i < hth_cfg->CanHwObjectCount; i++)
#endif
    {
        CanMBStatus_t *mb_sts = &ctrller_sts->mb[mb_id];
        spin_lock_saved_state_t state;

        spin_lock_irqsave(&mb_sts->lock, state);
        if (mb_sts->state == IDLE) {
            mb_sts->state = BUSY;
            spin_unlock_irqrestore(&mb_sts->lock, state);

            mb_sts->txPduId = PduInfo->swPduHandle;
            smp_wmb();
            ret = Can_SendAsync(hth_cfg->controllerId, mb_id,
                                hth_cfg->tx.paddingVal, PduInfo);

            if (ret != E_OK)
                mb_sts->state = IDLE;
            else
                goto out;
        }
        else {
            spin_unlock_irqrestore(&mb_sts->lock, state);
            ret = CAN_BUSY;
        }

        mb_id++;
    }

out:
    return ret;
}

#ifndef SDPE
void Can_MainFunction_Read(void)
{
    for (size_t i = 0; i< gCANConfig->rxCount; i++) {
        Can_MBConfig *mb_cfg = &gCANConfig->rxMBCfg[i];

        if (mb_cfg->polling &&
            flexcan_read_mb_int_status(mb_cfg->controllerId,
                                       mb_cfg->mbId)) {
            uint8_t rxDataBuf[64];
            flexcan_frame_t frameBuf = {.dataBuffer = rxDataBuf};

            if (FLEXCAN_SUCCESS == flexcan_read_rx_mb(mb_cfg->controllerId,
                                                      mb_cfg->mbId, &frameBuf))
                CanIf_RxIndication(mb_cfg->controllerId,
                                   frameBuf.id | (frameBuf.format << 31),
                                   frameBuf.length, frameBuf.dataBuffer);
        }
    }
}

void Can_MainFunction_Write(void)
{
    for (size_t i = 0; i< gCANConfig->txCount; i++) {
        Can_MBConfig *mb_cfg = &gCANConfig->txMBCfg[i];

        if (mb_cfg->polling &&
            flexcan_read_mb_int_status(mb_cfg->controllerId,
                                       mb_cfg->mbId)) {
            smp_rmb();

            CanMBStatus_t *mb_sts =
                &gCANCtrllerStatus[mb_cfg->controllerId].mb[mb_cfg->mbId];
            uint32 tx_pdu_id = mb_sts->txPduId;
            mb_sts->state = IDLE;
            CanIf_TxComfirmation(tx_pdu_id);
        }
    }
}
#endif
