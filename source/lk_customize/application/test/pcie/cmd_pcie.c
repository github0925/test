//*****************************************************************************
//
//
//
// Copyright (c) 2019-2029 SemiDrive Incorporated.  All rights reserved.
// Software License Agreement
//
//*****************************************************************************

//*****************************************************************************
//
//! \addtogroup pcie test
//! @{
//
//*****************************************************************************
#include <app.h>
#include <lib/console.h>
#include <sys/types.h>
#include <platform/debug.h>
#include <trace.h>
#include <reg.h>
#include <assert.h>
#include <errno.h>
#include <platform.h>
#include <kernel/thread.h>
#include "__regs_base.h"
#include "dw_pcie.h"

struct kunlun_pcie kunlun_pcie1_ep;
struct kunlun_pcie kunlun_pcie2_ep;
struct kunlun_pcie kunlun_pcie1_rc;
struct kunlun_pcie kunlun_pcie2_rc;

#define PCIE_TEST_TIME_HOURS_NUM 240
#define PCIE_TEST_TIME_MINUTES_NUM 60
#define PCIE_TEST_TIME_SECONDS_NUM 60

static void pcie_test_entry(const struct app_descriptor *app, void *args)
{
    struct kunlun_pcie kunlun_pcie_s;

    kunlun_pcie2_phy_loopback_test(&kunlun_pcie_s);
}

uint32_t htol_pcie_test(void)
{
    uint32_t ret = 0;
    struct kunlun_pcie kunlun_pcie_s;
    int hour = 0;
    int min = 0;
    int secend = 0;
    int result = 0;

    //wait 1 min
    min = 1;

    while (min) {
        secend = PCIE_TEST_TIME_SECONDS_NUM;

        while (secend) {
            spin(1000 * 1000); //sleep 1 second
            secend--;
        }

        min--;
    }

    kunlun_pcie2_phy_loopback_test(&kunlun_pcie_s);

    spin(10 * 15);

    while (hour < PCIE_TEST_TIME_HOURS_NUM) {
        min = PCIE_TEST_TIME_MINUTES_NUM;

        while (min) {
            secend = PCIE_TEST_TIME_SECONDS_NUM;

            while (secend) {
                spin(1000 * 1000); //sleep 1 second
                secend--;
            }

            min--;
        }

        result = kunlun_pcie_phy_internal_loopback_check(&kunlun_pcie_s, 0x1, 0x0);

        if (result) {
            ret = 1;
            printf("check pcie phy pass on hour=%d\n", hour);
        }
        else {
            ret = 0;
            printf("check pcie phy fail on hour=%d\n", hour);
            break;
        }

        hour++;
    }

    return ret;
}

static void htol_pcie_test_entry(const struct app_descriptor *app, void *args)
{
    htol_pcie_test();
}

static void pcie1_ep_entry(const struct app_descriptor *app, void *args)
{
    kunlun_pcie1_ep_mode_init(&kunlun_pcie1_ep);
}

#if defined(WITH_LIB_CONSOLE)
STATIC_COMMAND_START
STATIC_COMMAND("pcie_test", "pcie test entry", (console_cmd)&pcie_test_entry)
STATIC_COMMAND("htol_pcie_test", "htol pcie test entry",
               (console_cmd)&htol_pcie_test)
STATIC_COMMAND("pcie1_ep", "pcie1 ep mode app", (console_cmd)&pcie1_ep_entry)
STATIC_COMMAND_END(pcie_test);
#endif

// DEFINE_REGISTER_TEST_COMMAND(htol_pcie_test, x9_htol, htol_pcie_test)

APP_START(pcie_test_entry)
.flags = 1,
.entry = htol_pcie_test_entry,
APP_END
