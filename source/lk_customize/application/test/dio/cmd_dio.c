/*
 * Copyright (c) Semidrive
 */
#include <app.h>
#include <compiler.h>
#include <stdio.h>
#include <platform.h>

#include <err.h>
#include <kernel/semaphore.h>
#include <lib/console.h>
#include <platform/interrupts.h>
#include <bits.h>

#include "res.h"
#include "chip_res.h"
#include "hal_dio.h"
#include "sdunittest.h"

extern const domain_res_t g_gpio_res;

const Dio_ChannelType channel50 = DIO_CHANNEL_50;
const Dio_ChannelType channel63 = DIO_CHANNEL_63;

// GPIO_C0~C3
const Dio_ChannelGroupType channelGrp = {
    .mask = 0x000f0000,
    .offset = 16,
    .port = 1,
};

// GPIO_C8~C11
const Dio_ChannelGroupType readchannelGrp = {
    .mask = 0x0f000000,
    .offset = 24,
    .port = 1,
};

static void soft_delay(uint32_t counter1, uint32_t counter2)
{
    uint32_t i = 0;
    uint32_t j = 0;
    uint32_t k = 0;

    for (i = 0; i < counter1; i++) {
        for (j = 0; j < counter2; j++) {
            for (k = 0; k < 0x10000; k++) {
                ;
            }
        }

        printf("*");
    }

    printf("\n");
}

static int dio_write_channel_test(int argc, const cmd_args *argv)
{
    static void *g_handle;
    bool ret;
    Dio_LevelType channelLevel = 1;

    ret = hal_dio_creat_handle(&g_handle,
                               g_gpio_res.res_id[0]);

    if (ret == false) {
        return -1;
    }

    channelLevel = 1;
    printf("hal_dio_write_channel: %d\n", 1);
    hal_dio_write_channel(g_handle, 48, 1);  // GPIO_C0
    hal_dio_write_channel(g_handle, 64, 1);  // GPIO_D0

    soft_delay(1000, 1000000);

    channelLevel = 0;
    hal_dio_write_channel(g_handle, channel50, channelLevel);
    printf("hal_dio_write_channel: %d\n", channelLevel);
    hal_dio_write_channel(g_handle, 48, 0);
    hal_dio_write_channel(g_handle, 64, 0);

    soft_delay(1000, 1000000);

    channelLevel = 1;
    hal_dio_write_channel(g_handle, channel50, channelLevel);
    printf("hal_dio_write_channel: %d\n", channelLevel);
    hal_dio_write_channel(g_handle, 48, 1);
    hal_dio_write_channel(g_handle, 64, 1);

    /*
     * Write Channel Tests, GPIO_D1,D2
     */
    printf("hal_dio_write_channel: %d\n", 1);
    hal_dio_write_channel(g_handle, 65, 1);
    hal_dio_write_channel(g_handle, 66, 1);

    soft_delay(1000, 1000000);

    printf("hal_dio_write_channel: %d\n", 0);
    hal_dio_write_channel(g_handle, 65, 0);
    hal_dio_write_channel(g_handle, 66, 0);

    soft_delay(1000, 1000000);

    printf("hal_dio_write_channel: %d\n", 1);
    hal_dio_write_channel(g_handle, 65, 1);
    hal_dio_write_channel(g_handle, 66, 1);

    hal_dio_release_handle(&g_handle);

    return 0;
}


static int dio_write_port_test(int argc, const cmd_args *argv)
{
    static void *g_handle;
    bool ret;
    Dio_PortLevelType portLevel = 0x00000000;
    Dio_PortType portId = 1;

    ret = hal_dio_creat_handle(&g_handle,
                               g_gpio_res.res_id[0]);

    if (ret == false) {
        return -1;
    }

    /*
     * Write Port Tests
     */

    portLevel = 0x55555555;
    printf("hal_dio_write_port: 0x%lx\n", portLevel);
    hal_dio_write_port(g_handle, portId, portLevel);

    soft_delay(1000, 1000000);

    portLevel = 0xaaaaaaaa;
    printf("hal_dio_write_port: 0x%lx\n", portLevel);
    hal_dio_write_port(g_handle, portId, portLevel);


    soft_delay(1000, 1000000);

    portId = 2; // port2

    portLevel = 0x12345670;
    printf("hal_dio_write_port: 0x%lx\n", portLevel);
    hal_dio_write_port(g_handle, portId, portLevel);

    soft_delay(1000, 1000000);

    portLevel = 0x00000000;
    printf("hal_dio_write_port: 0x%lx\n", portLevel);
    hal_dio_write_port(g_handle, portId, portLevel);

    soft_delay(1000, 1000000);

    portLevel = 0xcba98765;
    printf("hal_dio_write_port: 0x%lx\n", portLevel);
    hal_dio_write_port(g_handle, portId, portLevel);

    soft_delay(1000, 1000000);

    portLevel = 0x00000000;
    printf("hal_dio_write_port: 0x%lx\n", portLevel);
    hal_dio_write_port(g_handle, portId, portLevel);

    soft_delay(1000, 1000000);

    portLevel = 0xfedcba98;
    printf("hal_dio_write_port: 0x%lx\n", portLevel);
    hal_dio_write_port(g_handle, portId, portLevel);

    soft_delay(1000, 1000000);

    portLevel = 0xFFFFFFFF;
    printf("hal_dio_write_port: 0x%lx\n", portLevel);
    hal_dio_write_port(g_handle, portId, portLevel);

    hal_dio_release_handle(&g_handle);

    return 0;
}

static int dio_write_channel_group_test(int argc, const cmd_args *argv)
{
    static void *g_handle;
    bool ret;
    Dio_PortLevelType portLevel = 0x00000000;

    ret = hal_dio_creat_handle(&g_handle,
                               g_gpio_res.res_id[0]);

    if (ret == false) {
        return -1;
    }

    /*
     * Dio Write channel group test
     */
    portLevel = 0xfffffff;
    printf("hal_dio_write_channel_group: 0x%lx\n", portLevel);
    hal_dio_write_channel_group(g_handle, &channelGrp, portLevel);

    soft_delay(1000, 1000000);

    portLevel = 0x00ffff00;
    printf("hal_dio_write_channel_group: 0x%lx\n", portLevel);
    hal_dio_write_channel_group(g_handle, &channelGrp, portLevel);

    soft_delay(1000, 1000000);

    hal_dio_release_handle(&g_handle);

    return 0;
}


static int dio_flip_test(int argc, const cmd_args *argv)
{
    static void *g_handle;
    bool ret;
    Dio_LevelType channelLevel;

    ret = hal_dio_creat_handle(&g_handle,
                               g_gpio_res.res_id[0]);

    if (ret == false) {
        return -1;
    }

    /*
     * Dio Flip/ masked write port test
     */
    channelLevel = hal_dio_flip_channel(g_handle, channel63);
    printf("hal_dio_flip_channel: %d\n", channelLevel);

    soft_delay(1000, 1000000);

    channelLevel = hal_dio_flip_channel(g_handle, channel63);
    printf("hal_dio_flip_channel: %d\n", channelLevel);

    soft_delay(1000, 1000000);

    hal_dio_release_handle(&g_handle);

    return 0;
}

static int dio_masked_write_test(int argc, const cmd_args *argv)
{
    static void *g_handle;
    bool ret;
    Dio_PortLevelType mask = 0x0f;
    Dio_PortLevelType portLevel = 0x00000000;
    Dio_PortType portId = 1;

    ret = hal_dio_creat_handle(&g_handle,
                               g_gpio_res.res_id[0]);

    if (ret == false) {
        return -1;
    }

    portLevel = 0xf0f0f0f0;
    mask = 0x0000ffff;
    printf("hal_dio_masked_write_port: 0x%lx\n", portLevel);
    hal_dio_masked_write_port(g_handle, portId, portLevel, mask);

    hal_dio_release_handle(&g_handle);

    return 0;
}


static int dio_read_channel_test(int argc, const cmd_args *argv)
{
    static void *g_handle;
    bool ret = true;
    Dio_LevelType channelLevel;
    Dio_PortLevelType mask = 0x0f;

    ret = hal_dio_creat_handle(&g_handle,
                               g_gpio_res.res_id[0]);

    if (ret == false) {
        return -1;
    }

    /*
     * Dio Read functions test.
     */
    printf("----------------Port1 Read test--------------\n");
    soft_delay(1000, 1000000);

    hal_dio_write_channel(g_handle, 48, 1);  // GPIO_C0
    hal_dio_write_channel(g_handle, 49, 1);  // GPIO_C1
    hal_dio_write_channel(g_handle, 50, 1);  // GPIO_C2
    hal_dio_write_channel(g_handle, 51, 1);  // GPIO_C3

    channelLevel = hal_dio_read_channel(g_handle, 56);
    printf("hal_dio_read_channel:[56] %d\n", channelLevel); // 1
    channelLevel = hal_dio_read_channel(g_handle, 57);
    printf("hal_dio_read_channel:[57] %d\n", channelLevel); // 1
    channelLevel = hal_dio_read_channel(g_handle, 58);
    printf("hal_dio_read_channel:[58] %d\n", channelLevel); // 1
    channelLevel = hal_dio_read_channel(g_handle, 59);
    printf("hal_dio_read_channel:[59] %d\n", channelLevel); // 1

    soft_delay(1000, 1000000);

    hal_dio_write_channel(g_handle, 48, 1);  // GPIO_C0
    hal_dio_write_channel(g_handle, 49, 1);  // GPIO_C1
    hal_dio_write_channel(g_handle, 50, 0);  // GPIO_C2
    hal_dio_write_channel(g_handle, 51, 0);  // GPIO_C3


    channelLevel = hal_dio_read_channel(g_handle, 56);
    printf("hal_dio_read_channel:[56] %d\n", channelLevel); // 1
    channelLevel = hal_dio_read_channel(g_handle, 57);
    printf("hal_dio_read_channel:[57] %d\n", channelLevel); // 1
    channelLevel = hal_dio_read_channel(g_handle, 58);
    printf("hal_dio_read_channel:[58] %d\n", channelLevel); // 0
    channelLevel = hal_dio_read_channel(g_handle, 59);
    printf("hal_dio_read_channel:[59] %d\n", channelLevel); // 0


    printf("----------------Port2 Read test--------------\n");
    soft_delay(1000, 1000000);

    hal_dio_write_channel(g_handle, 64, 1);  // GPIO_D0
    hal_dio_write_channel(g_handle, 65, 1);  // GPIO_D1
    hal_dio_write_channel(g_handle, 66, 1);  // GPIO_D2
    hal_dio_write_channel(g_handle, 67, 1);  // GPIO_D3


    channelLevel = hal_dio_read_channel(g_handle, 72);
    printf("hal_dio_read_channel:[72] %d\n", channelLevel); // 1
    channelLevel = hal_dio_read_channel(g_handle, 73);
    printf("hal_dio_read_channel:[73] %d\n", channelLevel); // 1
    channelLevel = hal_dio_read_channel(g_handle, 74);
    printf("hal_dio_read_channel:[74] %d\n", channelLevel); // 1
    channelLevel = hal_dio_read_channel(g_handle, 75);
    printf("hal_dio_read_channel:[75] %d\n", channelLevel); // 1

    soft_delay(1000, 1000000);

    hal_dio_write_channel(g_handle, 64, 0);  // GPIO_D0
    hal_dio_write_channel(g_handle, 65, 1);  // GPIO_D1
    hal_dio_write_channel(g_handle, 66, 0);  // GPIO_D2
    hal_dio_write_channel(g_handle, 67, 1);  // GPIO_D3


    channelLevel = hal_dio_read_channel(g_handle, 72);
    printf("hal_dio_read_channel:[72] %d\n", channelLevel); // 0
    channelLevel = hal_dio_read_channel(g_handle, 73);
    printf("hal_dio_read_channel:[73] %d\n", channelLevel); // 1
    channelLevel = hal_dio_read_channel(g_handle, 74);
    printf("hal_dio_read_channel:[74] %d\n", channelLevel); // 0
    channelLevel = hal_dio_read_channel(g_handle, 75);
    printf("hal_dio_read_channel:[75] %d\n", channelLevel); // 1

    soft_delay(1000, 1000000);

    hal_dio_release_handle(&g_handle);

    return 0;
}


static int dio_read_port_test(int argc, const cmd_args *argv)
{
    static void *g_handle;
    bool ret;
    Dio_LevelType channelLevel;
    Dio_PortLevelType tmpPortLevel;
    Dio_PortLevelType mask = 0x0f;
    Dio_PortLevelType portLevel = 0x00000000;
    Dio_PortType portId;

    ret = hal_dio_creat_handle(&g_handle,
                               g_gpio_res.res_id[0]);

    if (ret == false) {
        return -1;
    }

    /*
     * Dio Read functions test.
     */
    printf("----------------Port1 Read test--------------\n");
    soft_delay(1000, 1000000);

    // Port1: 48, 49, 50, 51  -> 56, 57, 58, 59
    portId = 1;

    hal_dio_write_channel(g_handle, 48, 1);  // GPIO_C0
    hal_dio_write_channel(g_handle, 49, 1);  // GPIO_C1
    hal_dio_write_channel(g_handle, 50, 1);  // GPIO_C2
    hal_dio_write_channel(g_handle, 51, 1);  // GPIO_C3

    tmpPortLevel = hal_dio_read_port(g_handle, 1);
    printf("hal_dio_read_port[%d]: 0x%lx\n", 1,
           tmpPortLevel); // 0x0f000000, C8~C11

    soft_delay(1000, 1000000);

    hal_dio_write_channel(g_handle, 48, 1);  // GPIO_C0
    hal_dio_write_channel(g_handle, 49, 1);  // GPIO_C1
    hal_dio_write_channel(g_handle, 50, 0);  // GPIO_C2
    hal_dio_write_channel(g_handle, 51, 0);  // GPIO_C3

    tmpPortLevel = hal_dio_read_port(g_handle, 1);
    printf("hal_dio_read_port[%d]: 0x%lx\n", 1, tmpPortLevel);

    hal_dio_write_channel(g_handle, 48, 1);  // GPIO_C0
    hal_dio_write_channel(g_handle, 49, 0);  // GPIO_C1
    hal_dio_write_channel(g_handle, 50, 0);  // GPIO_C2
    hal_dio_write_channel(g_handle, 51, 0);  // GPIO_C3

    tmpPortLevel = hal_dio_read_port(g_handle, 1);
    printf("hal_dio_read_port[%d]: 0x%lx\n", 1,
           tmpPortLevel); // 0x01000000,  C11/C10/C9/C8=0001

    soft_delay(1000, 1000000);

    hal_dio_write_channel(g_handle, 48, 0);  // GPIO_C0
    hal_dio_write_channel(g_handle, 49, 1);  // GPIO_C1
    hal_dio_write_channel(g_handle, 50, 0);  // GPIO_C2
    hal_dio_write_channel(g_handle, 51, 0);  // GPIO_C3

    tmpPortLevel = hal_dio_read_port(g_handle, 1);
    printf("hal_dio_read_port[%d]: 0x%lx\n", 1,
           tmpPortLevel); // 0x02000000, C11/C10/C9/C8=0010

    soft_delay(1000, 1000000);

    hal_dio_write_channel(g_handle, 48, 0);  // GPIO_C0
    hal_dio_write_channel(g_handle, 49, 0);  // GPIO_C1
    hal_dio_write_channel(g_handle, 50, 1);  // GPIO_C2
    hal_dio_write_channel(g_handle, 51, 0);  // GPIO_C3

    tmpPortLevel = hal_dio_read_port(g_handle, 1);
    printf("hal_dio_read_port[%d]: 0x%lx\n", 1,
           tmpPortLevel); // 0x04000000, C11/C10/C9/C8=0100

    soft_delay(1000, 1000000);

    hal_dio_write_channel(g_handle, 48, 0);  // GPIO_C0
    hal_dio_write_channel(g_handle, 49, 0);  // GPIO_C1
    hal_dio_write_channel(g_handle, 50, 0);  // GPIO_C2
    hal_dio_write_channel(g_handle, 51, 1);  // GPIO_C3

    tmpPortLevel = hal_dio_read_port(g_handle, 1);
    printf("hal_dio_read_port[%d]: 0x%lx\n", 1,
           tmpPortLevel); // 0x08000000, C11/C10/C9/C8=1000

    soft_delay(1000, 1000000);

    printf("----------------Port2 Read test--------------\n");
    soft_delay(1000, 1000000);

    //Port2: 64, 65, 66, 67  -> 72, 73, 74, 75, OK
    portId = 2;

    hal_dio_write_channel(g_handle, 64, 1);  // GPIO_D0
    hal_dio_write_channel(g_handle, 65, 1);  // GPIO_D1
    hal_dio_write_channel(g_handle, 66, 1);  // GPIO_D2
    hal_dio_write_channel(g_handle, 67, 1);  // GPIO_D3

    tmpPortLevel = hal_dio_read_port(g_handle, 2);
    printf("hal_dio_read_port[%d]: 0x%lx\n", 2, tmpPortLevel); // 0x00000f00

    soft_delay(1000, 1000000);

    hal_dio_write_channel(g_handle, 64, 0);  // GPIO_D0
    hal_dio_write_channel(g_handle, 65, 1);  // GPIO_D1
    hal_dio_write_channel(g_handle, 66, 0);  // GPIO_D2
    hal_dio_write_channel(g_handle, 67, 1);  // GPIO_D3

    tmpPortLevel = hal_dio_read_port(g_handle, 2);
    printf("hal_dio_read_port[%d]: 0x%lx\n", 2,
           tmpPortLevel); // 0x00000a00, D11/D10/D9/D8=1010

    soft_delay(1000, 1000000);

    portLevel = 0x00000000;
    mask = 0x0000000f;
    printf("hal_dio_masked_write_port: 0x%lx\n", portLevel);
    hal_dio_masked_write_port(g_handle, portId, portLevel, mask); // 0x0

    tmpPortLevel = hal_dio_read_port(g_handle, portId);
    printf("hal_dio_read_port: 0x%lx\n", tmpPortLevel);

    soft_delay(1000, 1000000);

    portLevel = 0x00000007; // D11/D10/D9/D8=0111
    mask = 0x0000000f;
    printf("hal_dio_masked_write_port: 0x%lx\n", portLevel);
    hal_dio_masked_write_port(g_handle, portId, portLevel, mask); // 0x7

    tmpPortLevel = hal_dio_read_port(g_handle, portId);
    printf("hal_dio_read_port: 0x%lx\n",
           tmpPortLevel); // 0x700, D11/D10/D9/D8=0111

    soft_delay(1000, 1000000);

    portLevel = 0x0000000e; // D11/D10/D9/D8=1110
    mask = 0x0000000f;
    printf("hal_dio_masked_write_port: 0x%lx\n", portLevel);
    hal_dio_masked_write_port(g_handle, portId, portLevel, mask);

    tmpPortLevel = hal_dio_read_port(g_handle, portId);
    printf("hal_dio_read_port: 0x%lx\n", tmpPortLevel);

    soft_delay(1000, 1000000);

    hal_dio_release_handle(&g_handle);

    return 0;
}

static int dio_read_channel_group_test(int argc, const cmd_args *argv)
{
    static void *g_handle;
    bool ret;
    Dio_PortLevelType tmpPortLevel;
    Dio_PortLevelType portLevel;

    ret = hal_dio_creat_handle(&g_handle,
                               g_gpio_res.res_id[0]);

    if (ret == false) {
        return -1;
    }

    /*
     * Dio Read channel group test.
     */
    printf("----------------Port1 Read test--------------\n"); // OK

    hal_dio_write_channel(g_handle, 48, 1);  // GPIO_C0
    hal_dio_write_channel(g_handle, 49, 0);  // GPIO_C1
    hal_dio_write_channel(g_handle, 50, 0);  // GPIO_C2
    hal_dio_write_channel(g_handle, 51, 0);  // GPIO_C3


    tmpPortLevel = hal_dio_read_channel_group(g_handle, &readchannelGrp);
    printf("hal_dio_read_channel_group: 0x%lx\n", tmpPortLevel); // 0x1

    soft_delay(1000, 1000000);

    hal_dio_write_channel(g_handle, 48, 0);  // GPIO_C0
    hal_dio_write_channel(g_handle, 49, 1);  // GPIO_C1
    hal_dio_write_channel(g_handle, 50, 0);  // GPIO_C2
    hal_dio_write_channel(g_handle, 51, 0);  // GPIO_C3


    tmpPortLevel = hal_dio_read_channel_group(g_handle, &readchannelGrp);
    printf("hal_dio_read_channel_group: 0x%lx\n", tmpPortLevel); // 0x2

    soft_delay(1000, 1000000);

    hal_dio_write_channel(g_handle, 48, 0);  // GPIO_C0
    hal_dio_write_channel(g_handle, 49, 0);  // GPIO_C1
    hal_dio_write_channel(g_handle, 50, 1);  // GPIO_C2
    hal_dio_write_channel(g_handle, 51, 0);  // GPIO_C3


    tmpPortLevel = hal_dio_read_channel_group(g_handle, &readchannelGrp);
    printf("hal_dio_read_channel_group: 0x%lx\n", tmpPortLevel); // 0x4

    soft_delay(1000, 1000000);

    hal_dio_write_channel(g_handle, 48, 0);  // GPIO_C0
    hal_dio_write_channel(g_handle, 49, 0);  // GPIO_C1
    hal_dio_write_channel(g_handle, 50, 0);  // GPIO_C2
    hal_dio_write_channel(g_handle, 51, 1);  // GPIO_C3


    tmpPortLevel = hal_dio_read_channel_group(g_handle, &readchannelGrp);
    printf("hal_dio_read_channel_group: 0x%lx\n", tmpPortLevel); // 0x8

    soft_delay(1000, 1000000);

    portLevel = 0x01; // GPIO_C0~C3
    printf("hal_dio_write_channel_group: 0x%lx\n", portLevel);
    hal_dio_write_channel_group(g_handle, &channelGrp,
                                portLevel); // 0x00010000

    tmpPortLevel = hal_dio_read_channel_group(g_handle, &readchannelGrp);
    printf("hal_dio_read_channel_group: 0x%lx\n", tmpPortLevel); // 0x1

    soft_delay(1000, 1000000);

    portLevel = 0x02; // GPIO_C0~C3
    printf("hal_dio_write_channel_group: 0x%lx\n", portLevel);
    hal_dio_write_channel_group(g_handle, &channelGrp, portLevel); // 0x20000

    tmpPortLevel = hal_dio_read_channel_group(g_handle, &readchannelGrp);
    printf("hal_dio_read_channel_group: 0x%lx\n", tmpPortLevel); // 0x2

    soft_delay(1000, 1000000);

    portLevel = 0x04; // GPIO_C0~C3
    printf("hal_dio_write_channel_group: 0x%lx\n", portLevel);
    hal_dio_write_channel_group(g_handle, &channelGrp, portLevel); // 0x40000

    tmpPortLevel = hal_dio_read_channel_group(g_handle, &readchannelGrp);
    printf("hal_dio_read_channel_group: 0x%lx\n", tmpPortLevel); // 0x4

    soft_delay(1000, 1000000);

    portLevel = 0x08; // GPIO_C0~C3
    printf("hal_dio_write_channel_group: 0x%lx\n", portLevel);
    hal_dio_write_channel_group(g_handle, &channelGrp,
                                portLevel); // 0x00080000

    tmpPortLevel = hal_dio_read_channel_group(g_handle, &readchannelGrp);
    printf("hal_dio_read_channel_group: 0x%lx\n", tmpPortLevel); // 0x8

    soft_delay(1000, 1000000);

    hal_dio_release_handle(&g_handle);

    return 0;
}


DEFINE_REGISTER_TEST_COMMAND(dio_test_1, dio, dio_write_channel_test)
DEFINE_REGISTER_TEST_COMMAND(dio_test_2, dio, dio_write_port_test)
DEFINE_REGISTER_TEST_COMMAND(dio_test_3, dio, dio_write_channel_group_test)
DEFINE_REGISTER_TEST_COMMAND(dio_test_4, dio, dio_flip_test)
DEFINE_REGISTER_TEST_COMMAND(dio_test_5, dio, dio_masked_write_test)
DEFINE_REGISTER_TEST_COMMAND(dio_test_6, dio, dio_read_channel_test)
DEFINE_REGISTER_TEST_COMMAND(dio_test_7, dio, dio_read_port_test)
DEFINE_REGISTER_TEST_COMMAND(dio_test_8, dio, dio_read_channel_group_test)

#if defined(WITH_LIB_CONSOLE)
#include <lib/console.h>

STATIC_COMMAND_START
    STATIC_COMMAND("dio_write_channel_test", "dio_write_channel_test", (console_cmd)&dio_write_channel_test)
    STATIC_COMMAND("dio_write_port_test", "dio_write_port_test", (console_cmd)&dio_write_port_test)
    STATIC_COMMAND("dio_write_channel_group_test", "dio_write_channel_group_test", (console_cmd)&dio_write_channel_group_test)
    STATIC_COMMAND("dio_flip_test", "dio_flip_test", (console_cmd)&dio_flip_test)
    STATIC_COMMAND("dio_masked_write_test", "dio_masked_write_test", (console_cmd)&dio_masked_write_test)
    STATIC_COMMAND("dio_read_channel_test", "dio_read_channel_test", (console_cmd)&dio_read_channel_test)
    STATIC_COMMAND("dio_read_port_test", "dio_read_port_test", (console_cmd)&dio_read_port_test)
    STATIC_COMMAND("dio_read_channel_group_test", "dio_read_channel_group_test", (console_cmd)&dio_read_channel_group_test)
STATIC_COMMAND_END(dio_test);

#endif
