/*
 * func_can.h
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: ii4 Test App.
 *
 * Revision History:
 * -----------------
 */
#ifndef __FUNC_CFG_H_
#define __FUNC_CFG_H_

#include "board_start.h"

#define CAN_SER_OFFSET 1
#define CAN_SEND_VAL   0x22

typedef enum {
    _IDLE,
    _SEND,
    _RECV,
    _ERR,
} CAN_CHN_STATE;

typedef enum {
    _INVALID,
    _VALID,
} CAN_CHN_RESULT;

typedef enum {
    MONITOR_DUT_MODE        = 0x1,
    CAN_MASTER_RESP_MODE    = 0x2,
    CAN_SLAVE_RESP_MODE     = 0x4,
} DTU_CALL_MODE;

#define CAN_EVENT   (MONITOR_DUT_MODE | CAN_MASTER_RESP_MODE | CAN_SLAVE_RESP_MODE)

typedef struct {
    uint8_t channel;
    uint8_t send_data;
    uint8_t recv_data;
    CAN_CHN_STATE can_chn_sta;
    CAN_CHN_RESULT result;
} can_chn_condition_t;

typedef struct {
    uint8_t dev_id;
    uint8_t route_channel_id;
    uint8_t recv_data;
    uint8_t standby_data1;
    uint8_t standby_data2;
    uint8_t standby_data3;
    uint8_t standby_data4;
    uint8_t standby_data5;
} can_cmd_t;

typedef struct {
    uint8_t cmd_status;
    uint8_t route_channel_id;
    uint8_t send_data;
    uint8_t standby_data1;
    uint8_t standby_data2;
    uint8_t standby_data3;
    uint8_t standby_data4;
    uint8_t standby_data5;
} can_resp_t;

typedef struct {
    uint8_t chn_num;
    uint8_t send_data;
    uint8_t standby_data1;
    uint8_t standby_data2;
    uint8_t standby_data3;
    uint8_t standby_data4;
    uint8_t standby_data5;
    uint8_t standby_data6;
} can_send_t;

typedef struct {
    uint8_t chn_num;
    uint8_t recv_data;
    uint8_t standby_data1;
    uint8_t standby_data2;
    uint8_t standby_data3;
    uint8_t standby_data4;
    uint8_t standby_data5;
    uint8_t standby_data6;
} can_recv_t;

typedef struct {
    spin_lock_t lock;
    DTU_CALL_MODE remote_call_mode;
    event_t  monitor_master_slave_event;
    EventGroupHandle_t xEventGroupHandle;
} can_dtu_t;

typedef struct {
    can_dtu_t dtu;
    uint8_t resp_chn_id;
    uint32_t canId;
    event_t  resp_event;
    spin_lock_t lock;
    uint8_t pay_load[8];
    can_resp_t *can_resp;
    can_send_t *can_send;
} canx_opt_t;

extern canx_opt_t canx_app;

extern can_chn_condition_t can_chn_condition[];

extern void can_check_back_package(board_test_exec_t *exec);

extern void clean_can_chn_queue_info(void);

extern void push_can_info_into_table(can_cmd_t *can_cmd);

extern bool get_can_info_from_table(board_test_exec_t *exec);

extern void push_can_info_into_queue(uint16_t Hrh, uint8_t CanSduPtr);

extern uint8_t get_can_info_chn_from_queue(uint8_t *info);

extern void CanIf_RxIndication( uint16_t Hrh, Can_IdType CanId,
                                uint8_t CanDlc, const uint8_t *CanSduPtr );

extern bool can_channel_to_write(canx_opt_t *canx_opt, uint32_t canId);

extern bool slave_dev_resp_can_chn_info(board_test_exec_t *exec);

#endif