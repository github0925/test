/*
 * reboot_service.h
 *
 * Copyright (c) 2020 SemiDrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */

#ifndef _REBOOT_SERVICE_H_
#define _REBOOT_SERVICE_H_

#include <sys/types.h>

#define RB_ENTRY_AP_F (1U << 0)
#define RB_COLD       (1U << 1)

typedef enum rb_module_e {
    RB_SAF_M,
    RB_SEC_M,
    RB_MP_M,
    RB_AP1_M,
    RB_AP2_M,
    RB_MAX_M
} rb_module_e;


typedef enum rb_opc_e {
    RB_RB_OPC,
    RB_STDN_OPC
} rb_opc_e;

typedef struct rb_arg {
    uint64_t entry;
    uint64_t sz;
    uint32_t flags;
} rb_arg;

int reboot_module(rb_module_e m, rb_opc_e opc, rb_arg *);
#endif
