/*
 * func_usb.h
 *
 * Copyright (c) 2020 SemiDrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */
#ifndef FUNC_USB_H_
#define FUNC_USB_H_
#include "board_diag.h"

typedef enum{
    USB_BREAK = 0,
    USB_LINK  = 1,
}USB_MENU;

extern bool usb1_reply_deal(test_exec_t *exec, test_state_e state);

#endif