/*
* virt_com_test.c
*
* Copyright (c) 2018 Semidrive Semiconductor.
* All rights reserved.
*
* Description: virt_com_test samplecode.
*
* Revision History:
* -----------------
*/
#include <app.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <debug.h>
#include <platform.h>
#include <lib/console.h>
#include <platform/interrupts.h>
#include "Can.h"
#include "vcan_cb.h"

int vircan_busoff_test(int argc, const cmd_args *argv)
{
    if (argc != 2) {
        printf("argc must be 2\n");
        return -1;
    }

    virCan_ControllerBusOff(argv[1].u);
    return 0;
}

int vircan_set_wakeup_test(int argc, const cmd_args *argv)
{
    if (argc != 2) {
        printf("argc must be 2\n");
        return -1;
    }

    virCan_SetWakeupEvent(argv[1].u);
    return 0;
}

int vircan_rx_indication_test(int argc, const cmd_args *argv)
{
    uint16_t hrh = 1;
    uint32_t can_id = 18;
    uint8_t can_dlc = 8;
    uint8_t can_data[8] = {1, 3, 4, 6, 8, 9, 0, 2};
    virCan_RxIndication(hrh, can_id, can_dlc, can_data);
    return 0;
}

int vircan_tx_confirm_test(int argc, const cmd_args *argv)
{
    if (argc != 2) {
        printf("argc must be 2\n");
        return -1;
    }

    virCan_TxConfirmation(argv[1].u);
    return 0;
}

int spin_test(int argc, const cmd_args *argv)
{
    dprintf(ALWAYS, "%s() start\n", __func__);
    spin(3000000);
    dprintf(ALWAYS, "%s() end\n", __func__);
    return 0;
}

static int dump_regs(int argc, const cmd_args *argv)
{
    void* base = (void*)0xF0030000U;
    uint16_t len = 32U;

    if (argc > 1) {
        base = (void*)argv[1].u;
        if (argc > 2) {
            len = argv[2].u;
        }
        
    }

    hexdump(base, len);

    return 0;
}

#if defined(WITH_LIB_CONSOLE)
STATIC_COMMAND_START
STATIC_COMMAND("vircan_busoff_indication", "vircan busoff indication test",
               (console_cmd)&vircan_busoff_test)
STATIC_COMMAND("vircan_set_wakeup_indication",
               "vircan set wakeup indication test", (console_cmd)&vircan_set_wakeup_test)
STATIC_COMMAND("vircan_rx_indication", "vircan rx indication test",
               (console_cmd)&vircan_rx_indication_test)
STATIC_COMMAND("vircan_tx_confirm", "vircan tx confirm test",
               (console_cmd)&vircan_tx_confirm_test)
STATIC_COMMAND("spin_test", "spin test", (console_cmd)&spin_test)
STATIC_COMMAND("dump_regs", "Dump registers", dump_regs)
STATIC_COMMAND_END(virt_com_test);
#endif

APP_START(virt_com_test)
//.init = ,
//.entry = ,
.flags = 0,
APP_END
