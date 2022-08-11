/*
 * sw_timer.h
 *
 * Copyright (c) 2020 SemiDrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */
#ifndef __SW_TIMER_H_
#define __SW_TIMER_H_
#include "board_diag.h"
#include "cfg.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

/**
 * @brief check CON1_19 GPIO value
 */
typedef enum {
    OFF_LINE = 0,
    ON_LINE  = 1
} GPIO_VALUE;

/**
 * @brief master or slaver mode
 */
typedef enum {
    XMASTER_MODE = 0X0,
    XSLAVE_MODE  = 0X1
} MASTER_SLAVER_MODE;

typedef struct {
#define DECT_ONLINE_MAX_TIMERS 1

    /*master_slave_mode*/
    MASTER_SLAVER_MODE master_slave_state;
    /*check online times*/
    uint8_t dect_online_timers;
    /*check online times*/
    uint8_t dect_offline_timers;
    /*eth ip previous state*/
    uint8_t pre_state;
    /*eth ip current state*/
    uint8_t cur_state;
    /*is eth set successfully*/
    int status;
} dev_master_slave_t;

extern void timer_init(void);
extern void usb_intr(void);
extern MASTER_SLAVER_MODE get_dev_master_slave_mode(void);
#endif