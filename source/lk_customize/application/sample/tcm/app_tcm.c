/*
* app_port.c
*
* Copyright (c) 2019 Semidrive Semiconductor.
* All rights reserved.
*
* Description: sample code for TCM code/data.
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
#include <arch.h>
#include <trace.h>
#include <reg.h>

#define LOCAL_TRACE         1

#define TCM_CODE_SECTION    __attribute__((section(".tcmcode")))
#define TCM_DATA_SECTION    __attribute__((section(".tcmdata")))

#define TCM_TEST_DATA_SIZE  (1024)

TCM_DATA_SECTION
static volatile int g_tcm_data[TCM_TEST_DATA_SIZE] = {
    [0] = 1,
    [1] = 3,
    [2] = 5,
    [3] = 7,
    [4] = 9,
    [TCM_TEST_DATA_SIZE - 5] = 2,
    [TCM_TEST_DATA_SIZE - 4] = 4,
    [TCM_TEST_DATA_SIZE - 3] = 6,
    [TCM_TEST_DATA_SIZE - 2] = 8,
    [TCM_TEST_DATA_SIZE - 1] = 10,
};

TCM_CODE_SECTION __attribute__ ((__noinline__))
static void tcm_code_func(void)
{
    for (int i = 0; i < TCM_TEST_DATA_SIZE; i++) {
        g_tcm_data[i] = i;
    }
}

TCM_CODE_SECTION __attribute__ ((__noinline__))
static void tcm_data_show(void)
{
    int i;

    for (i = 0; i < 5; i++) {
        LTRACEF("[%d] %d\n", i, g_tcm_data[i]);
    }

    for (i = TCM_TEST_DATA_SIZE - 5; i < TCM_TEST_DATA_SIZE - 1; i++) {
        LTRACEF("[%d] %d\n", i, g_tcm_data[i]);
    }
}

int tcm_test(int argc, const cmd_args *argv)
{
    tcm_data_show();
    tcm_code_func();
    tcm_data_show();

    return 0;
}

// LK console cmd
#if defined(WITH_LIB_CONSOLE)
STATIC_COMMAND_START
STATIC_COMMAND("tcm_test", "tcm_test", (console_cmd)&tcm_test)
STATIC_COMMAND_END(tcm_test);
#endif
