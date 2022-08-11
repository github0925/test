/*
* Copyright (c) 2020 Semidrive Semiconductor.
* All rights reserved.
*
*/
#include <stdio.h>
#include <console.h>
#include <sys/types.h>
#include <kernel/mutex.h>
#include "tmp411.h"

int tmp_sample_once(int argc, const cmd_args *argv)
{
    u8 value;
    float result;

    struct tmp411_device *tmp411 = tmp411_init();

    if (!tmp411) {
        printf("tmp411 device allocation failed");
        return 0;
    }

    value = tmp411_get_manufacture_id(tmp411);
    printf("%s() manufacture_id = 0x%x\n", __func__, value);

    result = tmp411_get_result(tmp411);
    printf("%s() local result = %f\n", __func__, result);

    extended_range_config(tmp411);
    /*wait time*/
    thread_sleep(1000);
    result = tmp411_get_result(tmp411);
    printf("%s() local result = %f\n", __func__, result);

    tmp411_deinit(tmp411);

    return 0;
}

#if defined(WITH_LIB_CONSOLE)
STATIC_COMMAND_START
STATIC_COMMAND("tmp411_sample", "tmp411 sample once",
               (console_cmd)&tmp_sample_once)
STATIC_COMMAND_END(tmp411_sample);

#endif