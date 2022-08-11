/*
 * vcan_if.c
 *
 * Copyright (c) 2019 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: Virtual CAN interface.
 *
 * Revision History:
 * -----------------
 */

#include <stdlib.h>
#include <debug.h>
#include <string.h>
#include <assert.h>
#include <kernel/thread.h>
#include <kernel/event.h>
#include <kernel/mutex.h>
#include "Can.h"
#ifdef SUPPORT_SDPE_RPC
#include "vcan_service.h"
#else
#include "vcan_cb.h"
#endif
#include "sdpe/common.h"

#define VIRCANIF_DEBUG              INFO
#define VIRCANIF_PDUID_BUF_DEPTH    64
#define VIRCANIF_PDUID_INVALID      0xFFFFFFFF

#define VIRCAN_NUM_OF_CTRL_MAX      (20)
#define VIRCAN_NUM_OF_HRHS_MAX      (20 * 64)
#define VIRCAN_NUM_OF_HTHS_MAX      (20 * 64)
#define VIRCAN_NUM_OF_RXFIFOS_MAX   (20)

#define CONTROLLER_COUNT 20

#define INIT_POLLING_INFO(dir_type, cnt, init, update, notify, t_name, t_func)  \
                                                {.dir = dir_type, \
                                                 .get_polling_cnt = cnt, \
                                                 .init_polling = init, \
                                                 .ctx_update = update, \
                                                 .ctx_notify = notify, \
                                                 .thread.name = #t_name, \
                                                 .thread.func = t_func}
#define VCAN_POLLING_THREAD_STACK_SIZE  1024

typedef struct vcan_polling_ctrl {
    mutex_t lock;
    Can_HwHandleType hoh;
    bool update_flag;
} vcan_polling_ctrl_t;

typedef enum vcan_dir {
    RX,
    TX
} vcan_dir_t;

typedef struct vcan_polling_thread {
    char name[16];
    event_t event;
    int (*func)(void *arg);
} vcan_polling_thread_t;

typedef struct vcan_polling_info vcan_polling_info_t;

struct vcan_polling_info {
    bool init;
    uint16_t cnt;
    vcan_dir_t dir;
    vcan_polling_ctrl_t *ctrl;
    void (*get_polling_cnt)(vcan_polling_info_t *info,
                            void *cfg, uint16_t size);
    void (*init_polling)(vcan_polling_info_t *info,
                         void *cfg, uint16_t size);
    void *ctx;
    void (*ctx_update)(int index, void *value);
    void (*ctx_notify)(int index);
    vcan_polling_thread_t thread;
};

typedef struct vcan_rx_ctx {
    uint8_t len;
    Can_IdType id;
    uint8_t data[64];
} vcan_rx_ctx_t;

typedef struct vcan_tx_ctx {
    uint16_t sw_pdu_handle;
} vcan_tx_ctx_t;

extern void canif_init_config(const Can_ConfigType *Config);
extern uint8_t virCanIf_Write(uint8_t ControllerId, Can_HwHandleType Hth,
                              const Can_PduType *virPduInfo);

extern int Can_BinarySearchHth(Can_HwHandleType Hth);

extern uint32_t canif_to_physical_bus_id(uint32_t logical_bus_id);
extern uint32_t canif_to_logical_bus_id(uint32_t physical_bus_id);

static Can_ConfigType g_can_config;

static void virCan_GetMBPollingCnt(vcan_polling_info_t *info,
                                   void *cfg, uint16_t cfg_size);
static void virCan_InitMBPolling(vcan_polling_info_t *info,
                                 void *cfg, uint16_t cfg_size);
static void virCan_GetRxFIFOPollingCnt(vcan_polling_info_t *info,
                                       void *cfg, uint16_t cfg_size);
static void virCan_InitRxFIFOPolling(vcan_polling_info_t *info,
                                     void *cfg, uint16_t cfg_size);
static void virCan_RxCtxUpdate(int index, void *value);
static void virCan_RxNotify(int index);
static void virCan_RxFifoCtxUpdate(int index, void *value);
static void virCan_RxFifoNotify(int index);
static void virCan_TxCtxUpdate(int index, void *value);
static void virCan_TxNotify(int index);
static int virCan_Polling(void *arg);

static vcan_polling_info_t g_rx_polling_info =
    INIT_POLLING_INFO(RX, virCan_GetMBPollingCnt, virCan_InitMBPolling,
                      virCan_RxCtxUpdate, virCan_RxNotify, rx_polling,
                      virCan_Polling);
static vcan_polling_info_t g_rx_fifo_polling_info =
    INIT_POLLING_INFO(RX, virCan_GetRxFIFOPollingCnt, virCan_InitRxFIFOPolling,
                      virCan_RxFifoCtxUpdate, virCan_RxFifoNotify, rx_fifo_polling,
                      virCan_Polling);
static vcan_polling_info_t g_tx_polling_info =
    INIT_POLLING_INFO(TX, virCan_GetMBPollingCnt, virCan_InitMBPolling,
                      virCan_TxCtxUpdate, virCan_TxNotify, tx_polling,
                      virCan_Polling);

static void virCan_GetMBPollingCnt(vcan_polling_info_t *info,
                                   void *cfg, uint16_t cfg_size)
{
    Can_MBConfig *mb_cfg = cfg;

    for (size_t i = 0; i < cfg_size; i++) {
        if (mb_cfg[i].polling)
            info->cnt++;
    }
}

static void virCan_InitMBPolling(vcan_polling_info_t *info,
                                 void *cfg, uint16_t cfg_size)
{
    Can_MBConfig *mb_cfg = cfg;

    for (size_t i = 0, j = 0; i < cfg_size; i++) {
        Can_MBConfig *mb = &mb_cfg[i];

        if (mb->polling) {
            vcan_polling_ctrl_t *p = &info->ctrl[j++];
            mutex_init(&p->lock);
            p->hoh = mb->hwObjId;
            p->update_flag = false;
        }
    }
}

static void virCan_GetRxFIFOPollingCnt(vcan_polling_info_t *info,
                                       void *cfg, uint16_t cfg_size)
{
    Can_RxFIFOConfig *fifo_cfg = cfg;

    for (size_t i = 0; i < cfg_size; i++) {
        if (fifo_cfg[i].polling)
            info->cnt++;
    }
}

static void virCan_InitRxFIFOPolling(vcan_polling_info_t *info,
                                     void *cfg, uint16_t cfg_size)
{
    Can_RxFIFOConfig *fifo_cfg = cfg;

    for (size_t i = 0, j = 0; i < cfg_size; i++) {
        Can_RxFIFOConfig *fifo = &fifo_cfg[i];

        if (fifo->polling) {
            vcan_polling_ctrl_t *p = &info->ctrl[j++];
            mutex_init(&p->lock);
            p->hoh = fifo->hwObjId;
            p->update_flag = false;
        }
    }
}

static void virCan_RxCtxUpdate(int index, void *value)
{
    vcan_rx_ctx_t *ctx = (vcan_rx_ctx_t *)g_rx_polling_info.ctx;
    vcan_rx_ctx_t *v = value;

    ctx[index] = *v;
}

static void virCan_RxNotify(int index)
{
    vcan_rx_ctx_t *ctx = (vcan_rx_ctx_t *)g_rx_polling_info.ctx;

    ctx = &ctx[index];
    virCan_RxIndication(g_rx_polling_info.ctrl[index].hoh, ctx->id, ctx->len, ctx->data);
}

static void virCan_RxFifoCtxUpdate(int index, void *value)
{
    vcan_rx_ctx_t *ctx = (vcan_rx_ctx_t *)g_rx_fifo_polling_info.ctx;
    vcan_rx_ctx_t *v = value;

    ctx[index] = *v;
}

static void virCan_RxFifoNotify(int index)
{
    vcan_rx_ctx_t *ctx = (vcan_rx_ctx_t *)g_rx_fifo_polling_info.ctx;

    ctx = &ctx[index];
    virCan_RxIndication(g_rx_fifo_polling_info.ctrl[index].hoh, ctx->id, ctx->len, ctx->data);
}

static void virCan_TxCtxUpdate(int index, void *value)
{
    vcan_tx_ctx_t *ctx = (vcan_tx_ctx_t *)g_tx_polling_info.ctx;
    vcan_tx_ctx_t *v = value;

    ctx[index] = *v;
}

static void virCan_TxNotify(int index)
{
    vcan_tx_ctx_t *ctx = (vcan_tx_ctx_t *)g_tx_polling_info.ctx;

    virCan_TxConfirmation(ctx[index].sw_pdu_handle);
}

static int virCan_Polling(void *arg)
{
    vcan_polling_info_t *info = arg;

    while (true) {
        event_wait(&info->thread.event);

        for (size_t i = 0; i < info->cnt; i++) {
            vcan_polling_ctrl_t *p = &info->ctrl[i];

            mutex_acquire(&p->lock);
            if (p->update_flag) {
                p->update_flag = false;
                info->ctx_notify(i);
            }
            mutex_release(&p->lock);
        }
    }

    return 0;
}

static void virCan_ReorderPollingCtrInfo(vcan_polling_info_t *info)
{
    for (uint16 i = 1; i < info->cnt; i++) {
        for (uint16 j = 0; j < info->cnt - i; j++) {
            if (info->ctrl[j].hoh > info->ctrl[j + 1].hoh) {
                vcan_polling_ctrl_t temp;
                temp = info->ctrl[j];
                info->ctrl[j] =info->ctrl[j + 1];
                info->ctrl[j + 1] = temp;
            }
        }
    }
}

static inline void virCan_CreatePollingThread(vcan_polling_info_t *info)
{
    event_init(&info->thread.event, false, EVENT_FLAG_AUTOUNSIGNAL);
    thread_t *t = thread_create(info->thread.name, info->thread.func, info,
                            DEFAULT_PRIORITY, VCAN_POLLING_THREAD_STACK_SIZE);
    SDPE_ASSERT(t);
    thread_detach_and_resume(t);
}

static void virCan_InitPollingCtrlInfo(vcan_polling_info_t *info,
                                       void *cfg, uint16_t cfg_size)
{
    if (info->init)
        return;

    info->get_polling_cnt(info, cfg, cfg_size);

    if (!info->cnt)
        goto out;

    if (!info->ctrl) {
        SDPE_ASSERT(info->ctrl = malloc(sizeof(vcan_polling_ctrl_t) * info->cnt));
    }

    if (info->dir == RX) {
        if (!info->ctx) {
            SDPE_ASSERT(info->ctx = malloc(sizeof(vcan_rx_ctx_t) * info->cnt));
        }
    }
    else {
        if (!info->ctx) {
            SDPE_ASSERT(info->ctx = malloc(sizeof(vcan_tx_ctx_t) * info->cnt));
        }
    }

    info->init_polling(info, cfg, cfg_size);

    virCan_ReorderPollingCtrInfo(info);

    virCan_CreatePollingThread(info);

out:
    info->init = true;
}

static void virCan_DeInitPollingCtrlInfo(vcan_polling_info_t *info)
{
    for (size_t i = 0; i < info->cnt; i++) {
        info->ctrl[i].update_flag = false;
    }
}

static int virCan_BinarySearchHoh(vcan_polling_info_t *info,
                                  Can_HwHandleType Hoh)
{
    if (!info->cnt)
        return -1;

    int left = 0, right = info->cnt - 1;
    int mid;

    while (left <= right) {
        mid = (left + right) / 2;

        if (Hoh < info->ctrl[mid].hoh)
            right = mid - 1;
        else if (Hoh > info->ctrl[mid].hoh)
            left = mid + 1;
        else
            return mid;
    }

    return -1;
}

static void virCan_CtxUpdate(vcan_polling_info_t *info, int index, void *ctx)
{
    vcan_polling_ctrl_t *p = &info->ctrl[index];

    mutex_acquire(&p->lock);
    /* Only the oldest frame is buffered before polling by safety,
     * this is consistent with AutoSAR CAN MCAL.
     */
    if (!p->update_flag) {
        p->update_flag = true;
        info->ctx_update(index, ctx);
    }
    mutex_release(&p->lock);
}

void virCan_ReceiveFrame(Can_HwHandleType Hrh, Can_IdType Id,
                         uint8_t len, uint8_t *data)
{
    dprintf(DBGV, "%s: hrh %d, can id 0x%x\n", __func__, Hrh, Id);

    int mb_polling_hrh_idx = virCan_BinarySearchHoh(&g_rx_polling_info, Hrh);
    int fifo_polling_hrh_idx = virCan_BinarySearchHoh(&g_rx_fifo_polling_info, Hrh);

    if ((mb_polling_hrh_idx == -1) && (fifo_polling_hrh_idx == -1)) {
        /* Interrupt */
        virCan_RxIndication(Hrh, Id, len, data);
    }
    else {
        /* Polling */
        vcan_rx_ctx_t ctx = {.len = len, .id = Id};
        memcpy(ctx.data, data, len);
        if (mb_polling_hrh_idx != -1)
            virCan_CtxUpdate(&g_rx_polling_info, mb_polling_hrh_idx, &ctx);
        else if (fifo_polling_hrh_idx != -1)
            virCan_CtxUpdate(&g_rx_fifo_polling_info, fifo_polling_hrh_idx, &ctx);
    }
}

void virCan_TxCompleted(uint16_t SwPduHandle, uint32_t Hth)
{
    dprintf(DBGV, "%s: hth %d, SwPduHandle %d\n", __func__, Hth, SwPduHandle);

    int polling_hth_idx = virCan_BinarySearchHoh(&g_tx_polling_info, Hth);

    if (polling_hth_idx == -1) {
        /* Interrupt */
        virCan_TxConfirmation(SwPduHandle);
    }
    else {
        /* Polling */
        vcan_tx_ctx_t ctx = {SwPduHandle};
        virCan_CtxUpdate(&g_tx_polling_info, polling_hth_idx, &ctx);
    }
}

static void virCan_MainFunction(vcan_polling_info_t *info)
{
    if (info->cnt)
        event_signal(&info->thread.event, false);
}

extern int Can_BinarySearchHth(Can_HwHandleType Hth);

extern uint32_t canif_to_physical_bus_id(uint32_t logical_bus_id);
extern uint32_t canif_to_logical_bus_id(uint32_t physical_bus_id);

static Can_ConfigType g_can_config;

/**********************************************************************************
** Used for Autosar write to SDPE, SDPE is server
**********************************************************************************/
void virCan_Init(const Can_ConfigType *Config)
{
#if (VIRCANIF_DEBUG <= LK_DEBUGLEVEL)
    dprintf(VIRCANIF_DEBUG, "\n%s()\n", __func__);

    dprintf(VIRCANIF_DEBUG, "\n");
    dprintf(VIRCANIF_DEBUG, "Can_Config:\n");
    for (size_t i = 0; i < Config->controllerCount; i++) {
        dprintf(VIRCANIF_DEBUG,
                "ControllerId: %d  baseAddr: 0x%X  clkSrc: %d  maxMbNum: %d  enableLoopBack: %d\n",
                Config->ctrllerCfg[i].controllerId,
                (uint32_t)Config->ctrllerCfg[i].baseAddr,
                (uint8_t)Config->ctrllerCfg[i].flexcanCfg.clkSrc,
                Config->ctrllerCfg[i].flexcanCfg.maxMbNum,
                Config->ctrllerCfg[i].flexcanCfg.enableLoopBack);
        dprintf(VIRCANIF_DEBUG,
                "enableListenOnly: %d  enableSelfWakeup: %d  enableIndividMask: %d  enableCANFD: %d\n",
                Config->ctrllerCfg[i].flexcanCfg.enableListenOnly,
                Config->ctrllerCfg[i].flexcanCfg.enableSelfWakeup,
                Config->ctrllerCfg[i].flexcanCfg.enableIndividMask,
                Config->ctrllerCfg[i].flexcanCfg.enableCANFD);
        dprintf(VIRCANIF_DEBUG,
                "NomiBitTiming -- preDivider: %d rJumpwidth: %d, propSeg: %d, phaseSeg1: %d phaseSeg2: %d\n",
                Config->ctrllerCfg[i].flexcanCfg.nominalBitTiming.preDivider,
                Config->ctrllerCfg[i].flexcanCfg.nominalBitTiming.rJumpwidth,
                Config->ctrllerCfg[i].flexcanCfg.nominalBitTiming.propSeg,
                Config->ctrllerCfg[i].flexcanCfg.nominalBitTiming.phaseSeg1,
                Config->ctrllerCfg[i].flexcanCfg.nominalBitTiming.phaseSeg2);
        dprintf(VIRCANIF_DEBUG,
                "DataBitTiming -- preDivider: %d rJumpwidth: %d, propSeg: %d, phaseSeg1: %d phaseSeg2: %d\n",
                Config->ctrllerCfg[i].flexcanCfg.dataBitTiming.preDivider,
                Config->ctrllerCfg[i].flexcanCfg.dataBitTiming.rJumpwidth,
                Config->ctrllerCfg[i].flexcanCfg.dataBitTiming.propSeg,
                Config->ctrllerCfg[i].flexcanCfg.dataBitTiming.phaseSeg1,
                Config->ctrllerCfg[i].flexcanCfg.dataBitTiming.phaseSeg2);
        dprintf(VIRCANIF_DEBUG,
                "enableISOCANFD: %d enableBRS: %d, enableTDC: %d, TDCOffset: %d r0_mb_data_size: %d r1_mb_data_size: %d\n",
                Config->ctrllerCfg[i].flexcanCfg.can_fd_cfg.enableISOCANFD,
                Config->ctrllerCfg[i].flexcanCfg.can_fd_cfg.enableBRS,
                Config->ctrllerCfg[i].flexcanCfg.can_fd_cfg.enableTDC,
                Config->ctrllerCfg[i].flexcanCfg.can_fd_cfg.TDCOffset,
                (uint8_t)Config->ctrllerCfg[i].flexcanCfg.can_fd_cfg.r0_mb_data_size,
                (uint8_t)Config->ctrllerCfg[i].flexcanCfg.can_fd_cfg.r1_mb_data_size);
        dprintf(VIRCANIF_DEBUG, "\n");
    }

    dprintf(VIRCANIF_DEBUG, "\n");
    dprintf(VIRCANIF_DEBUG, "rxMBCfg:\n");
    for (size_t i = 0; i < Config->rxCount; i++) {
        dprintf(VIRCANIF_DEBUG,
                "hwObjId: %d  controllerId: %d  mbId: %d  frameId: 0x%X\n",
                Config->rxMBCfg[i].hwObjId,
                Config->rxMBCfg[i].controllerId,
                Config->rxMBCfg[i].mbId,
                Config->rxMBCfg[i].rx.frameId);
        dprintf(VIRCANIF_DEBUG,
                "rxIDFilterMask: 0x%X  frameType: %d  frameFormat: %d\n",
                Config->rxMBCfg[i].rx.rxIDFilterMask,
                Config->rxMBCfg[i].rx.frameType,
                Config->rxMBCfg[i].rx.frameFormat);
        dprintf(VIRCANIF_DEBUG, "\n");
    }

    dprintf(VIRCANIF_DEBUG, "\n");
    dprintf(VIRCANIF_DEBUG, "txMBCfg:\n");
    for (size_t i = 0; i < Config->txCount; i++) {
        dprintf(VIRCANIF_DEBUG,
                "hwObjId: %d  controllerId: %d  mbId: %d  paddingVal: 0x%X\n",
                Config->txMBCfg[i].hwObjId,
                Config->txMBCfg[i].controllerId,
                Config->txMBCfg[i].mbId,
                Config->txMBCfg[i].tx.paddingVal);
        dprintf(VIRCANIF_DEBUG, "\n");
    }
#endif

    static uint8_t ctrller_cnt = 0U;
    static uint16_t rx_fifo_cnt, rx_mb_cnt, tx_mb_cnt;
    static Can_ControllerConfig controller_config[VIRCAN_NUM_OF_CTRL_MAX];
    static Can_MBConfig rx_mb_config[VIRCAN_NUM_OF_HRHS_MAX];
    static Can_MBConfig tx_mb_config[VIRCAN_NUM_OF_HTHS_MAX];
    static Can_RxFIFOConfig rx_fifo_config[VIRCAN_NUM_OF_RXFIFOS_MAX];

    if (Config->controllerCount) {
        if (Config->ctrllerCfg) {
            memcpy(&controller_config[ctrller_cnt], Config->ctrllerCfg, sizeof(Can_ControllerConfig));
            ctrller_cnt++;
        }
        if (Config->rxMBCfg) {
            memcpy(&rx_mb_config[rx_mb_cnt], Config->rxMBCfg, sizeof(Can_MBConfig) * Config->rxCount);
            rx_mb_cnt += Config->rxCount;
        }
        if (Config->txMBCfg) {
            memcpy(&tx_mb_config[tx_mb_cnt], Config->txMBCfg, sizeof(Can_MBConfig) * Config->txCount);
            tx_mb_cnt += Config->txCount;
        }
        if (Config->rxFIFOCfg) {
            memcpy(&rx_fifo_config[rx_fifo_cnt], Config->rxFIFOCfg, sizeof(Can_RxFIFOConfig));
            uint32_t filter_tab_size =
                Config->rxFIFOCfg->flexcanRxFIFOCfg.idFilterNum * sizeof(flexcan_rx_fifo_filter_table_t);
            flexcan_rx_fifo_filter_table_t *filter_tab = malloc(filter_tab_size);
            SDPE_ASSERT(filter_tab);
            memcpy(filter_tab, Config->rxFIFOCfg->flexcanRxFIFOCfg.filter_tab, filter_tab_size);
            rx_fifo_config[rx_fifo_cnt].flexcanRxFIFOCfg.filter_tab = filter_tab;
            rx_fifo_cnt++;
        }
    }
    else {
        g_can_config.ctrllerCfg = controller_config;
        g_can_config.rxMBCfg = rx_mb_config;
        g_can_config.txMBCfg = tx_mb_config;
        g_can_config.rxFIFOCfg = rx_fifo_config;
        g_can_config.controllerCount = ctrller_cnt;
        g_can_config.rxFifoCount = rx_fifo_cnt;
        g_can_config.rxCount = rx_mb_cnt;
        g_can_config.txCount = tx_mb_cnt;

        ctrller_cnt = 0U;
        rx_fifo_cnt = 0U;
        rx_mb_cnt = 0U;
        tx_mb_cnt = 0U;

        dprintf(VIRCANIF_DEBUG, "Can_Init()....\n");
        canif_init_config(&g_can_config);
        Can_Init(&g_can_config);
        virCan_InitPollingCtrlInfo(&g_rx_polling_info,
                                   g_can_config.rxMBCfg,
                                   g_can_config.rxCount);
        virCan_InitPollingCtrlInfo(&g_rx_fifo_polling_info,
                                   g_can_config.rxFIFOCfg,
                                   g_can_config.rxFifoCount);
        virCan_InitPollingCtrlInfo(&g_tx_polling_info,
                                   g_can_config.txMBCfg,
                                   g_can_config.txCount);
    }
}

void virCan_DeInit(void)
{
    Can_DeInit();
    for (size_t i = 0; i < g_can_config.rxFifoCount; i++) {
        free(g_can_config.rxFIFOCfg[i].flexcanRxFIFOCfg.filter_tab);
        g_can_config.rxFIFOCfg[i].flexcanRxFIFOCfg.filter_tab = NULL;
    }

    virCan_DeInitPollingCtrlInfo(&g_rx_polling_info);
    virCan_DeInitPollingCtrlInfo(&g_rx_fifo_polling_info);
    virCan_DeInitPollingCtrlInfo(&g_tx_polling_info);
    dprintf(VIRCANIF_DEBUG, "\n%s()\n", __func__);
}

int virCan_SetBaudrate(uint8_t Controller, uint16_t BaudRateConfigID)
{
    int ret = 0;
    dprintf(VIRCANIF_DEBUG, "\n%s() controller: %d, BaudRateConfigID: %d\n",
            __func__, Controller, BaudRateConfigID);
    ret = Can_SetBaudrate(Controller, BaudRateConfigID);
    dprintf(VIRCANIF_DEBUG, "%s() ret: %d\n", __func__, ret);
    return ret;
}

int virCan_SetControllerMode(uint8_t Controller, uint8_t Transition)
{
    int ret = 0;
    dprintf(VIRCANIF_DEBUG, "\n%s() controller: %d, Transition: %d\n",
            __func__, Controller, Transition);
    ret = Can_SetControllerMode(Controller,
                                (Can_ControllerStateType)Transition);
    dprintf(VIRCANIF_DEBUG, "%s() ret: %d\n", __func__, ret);
    return ret;
}

int virCan_DisableControllerInterrupts(uint8_t Controller)
{
    dprintf(VIRCANIF_DEBUG, "\n%s() controller: %d\n", __func__, Controller);
    Can_DisableControllerInterrupts(Controller);

    return 0;
}

int virCan_EnableControllerInterrupts(uint8_t Controller)
{
    dprintf(VIRCANIF_DEBUG, "\n%s() controller: %d\n", __func__, Controller);
    Can_EnableControllerInterrupts(Controller);

    return 0;
}

int virCan_CheckWakeup(uint8_t Controller)
{
    dprintf(VIRCANIF_DEBUG, "\n%s() controller: %d\n", __func__, Controller);
    Can_CheckWakeup(Controller);

    return 0;
}

int virCan_GetControllerErrorState(uint8_t Controller,
                                       uint8_t *ErrorStatePtr)
{
    int ret = 0;
    dprintf(VIRCANIF_DEBUG, "\n%s() controller: %d\n", __func__, Controller);
    ret = Can_GetControllerErrorState(Controller,
                                      (Can_ErrorStateType *)ErrorStatePtr);
    dprintf(VIRCANIF_DEBUG, "%s() ret: %d, error_state: %d\n", __func__, ret,
            *ErrorStatePtr);
    return ret;
}

int virCan_GetControllerMode(uint8_t Controller,
                                 uint8_t *ControllerModePtr)
{
    int ret = 0;
    dprintf(VIRCANIF_DEBUG, "\n%s() controller: %d\n", __func__, Controller);
    ret = Can_GetControllerMode(Controller,
                                (Can_ControllerStateType *)ControllerModePtr);
    dprintf(VIRCANIF_DEBUG, "%s() ret: %d, mode: %d\n", __func__, ret,
            *ControllerModePtr);
    return ret;
}

int virCan_Write(Can_HwHandleType Hth,
                 const Can_PduType *virPduInfo)
{
    int ret;

#if VCAN_TEST
    Can_PduType PduInfo;

    PduInfo.swPduHandle = virPduInfo->swPduHandle;
    PduInfo.length = virPduInfo->length;
    PduInfo.id = virPduInfo->id;
    PduInfo.sdu = virPduInfo->sdu;

    dprintf(VIRCANIF_DEBUG,
            "%s() Hth: %d, swPduHandle: %d, length: %d, id: 0x%X\n",
            __func__, Hth, virPduInfo->swPduHandle, virPduInfo->length,
            virPduInfo->id);

    ret = Can_Write(Hth, &PduInfo);
#else
    int hth_idx = Can_BinarySearchHth(Hth);
    if (hth_idx == -1) {
        ret = E_NOT_OK;
    }
    else {
        uint32_t controller_id = canif_to_physical_bus_id(
                                g_can_config.txMBCfg[hth_idx].controllerId);
        ret = virCanIf_Write(controller_id, Hth, virPduInfo);
    }
#endif

    dprintf(VIRCANIF_DEBUG, "%s() ret: %d\n", __func__, ret);
    return ret;
}

void vircan_MainFunction_Read(void)
{
    virCan_MainFunction(&g_rx_polling_info);
    virCan_MainFunction(&g_rx_fifo_polling_info);
}

void vircan_MainFunction_Write(void)
{
    virCan_MainFunction(&g_tx_polling_info);
}
