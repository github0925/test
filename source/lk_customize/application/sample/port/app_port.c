/*
* app_port.c
*
* Copyright (c) 2019 Semidrive Semiconductor.
* All rights reserved.
*
* Description: sample code for iomux/port
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
#include "hal_port.h"
#include "hal_dio.h"
#include "res.h"
#include "chip_res.h"

extern const domain_res_t g_iomuxc_res;
extern const domain_res_t g_gpio_res;

/* 48 - GPIO_C0 */
const Port_PinType pin48 = PortConf_PIN_GPIO_C0;
const Port_PinDirectionType direction48 = PORT_PIN_IN;

/* 64 - GPIO_D0 */
const Port_PinType pin64 = PortConf_PIN_GPIO_D0;
const Port_PinModeType mode64 = {
    ((uint32_t)PORT_PAD_POE__DISABLE | PORT_PAD_IS__IN | PORT_PAD_SR__FAST | PORT_PAD_DS__MID1 | PORT_PIN_IN_PULL_DOWN ),
    ((uint32_t)PORT_PIN_MUX_FV__MIN | PORT_PIN_MUX_FIN__MIN | PORT_PIN_OUT_OPENDRAIN | PORT_PIN_MODE_GPIO),
};
// 72/D8,73/D9,74/D10,75/D11, config to input
/* 72 - GPIO_D8 */
const Port_PinType pin72 = PortConf_PIN_GPIO_D8;
const Port_PinModeType mode72 = {
    ((uint32_t)PORT_PAD_POE__DISABLE | PORT_PAD_IS__IN | PORT_PAD_SR__FAST | PORT_PAD_DS__MID1 | PORT_PIN_IN_PULL_DOWN ),
    ((uint32_t)PORT_PIN_MUX_FV__MIN | PORT_PIN_MUX_FIN__MIN | PORT_PIN_OUT_OPENDRAIN | PORT_PIN_MODE_GPIO),
};
/* 73 - GPIO_D9 */
const Port_PinType pin73 = PortConf_PIN_GPIO_D9;
const Port_PinModeType mode73 = {
    ((uint32_t)PORT_PAD_POE__DISABLE | PORT_PAD_IS__IN | PORT_PAD_SR__FAST | PORT_PAD_DS__MID1 | PORT_PIN_IN_PULL_DOWN ),
    ((uint32_t)PORT_PIN_MUX_FV__MIN | PORT_PIN_MUX_FIN__MIN | PORT_PIN_OUT_OPENDRAIN | PORT_PIN_MODE_GPIO),
};
/* 74 - GPIO_D10 */
const Port_PinType pin74 = PortConf_PIN_GPIO_D10;
const Port_PinModeType mode74 = {
    ((uint32_t)PORT_PAD_POE__DISABLE | PORT_PAD_IS__IN | PORT_PAD_SR__FAST | PORT_PAD_DS__MID1 | PORT_PIN_IN_PULL_DOWN ),
    ((uint32_t)PORT_PIN_MUX_FV__MIN | PORT_PIN_MUX_FIN__MIN | PORT_PIN_OUT_OPENDRAIN | PORT_PIN_MODE_GPIO),
};
/* 75 - GPIO_D11 */
const Port_PinType pin75 = PortConf_PIN_GPIO_D11;
const Port_PinModeType mode75 = {
    ((uint32_t)PORT_PAD_POE__DISABLE | PORT_PAD_IS__IN | PORT_PAD_SR__FAST | PORT_PAD_DS__MID1 | PORT_PIN_IN_PULL_DOWN ),
    ((uint32_t)PORT_PIN_MUX_FV__MIN | PORT_PIN_MUX_FIN__MIN | PORT_PIN_OUT_OPENDRAIN | PORT_PIN_MODE_GPIO),
};

const Dio_ChannelType channel50 = DIO_CHANNEL_50;  // output, GPIO_C2
const Dio_ChannelType channel63 = DIO_CHANNEL_63;  // input

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

static int cmd_dio(int argc, const cmd_args *argv)
{
    bool ret = true;
    static void *g_handle;
    Dio_LevelType channelLevel;
    Dio_PortLevelType tmpPortLevel;
    Dio_PortLevelType mask = 0x0f;
    Dio_LevelType channel50Level = 1;
    Dio_PortLevelType portLevel = 0x00000000;
    Dio_PortType portId = 1;  // port2/NG, port3/NG

    printf("+++++dio sample cmd_dio 15\n");
    ret = hal_dio_creat_handle(&g_handle,
                               g_gpio_res.res_id[0]);  // RES_GPIO_GPIO2 for secure

#if 1
    soft_delay(1000, 1000000);

    /*
     * Write Channel Tests, Tested OK
     */
    channel50Level = 1; // 0->1->0
    printf("hal_dio_write_channel: %d\n", 1);
    hal_dio_write_channel(g_handle, 48, 1);  // GPIO_C0
    //hal_dio_write_channel(g_handle, 56, 1);  // GPIO_C8
    hal_dio_write_channel(g_handle, 64, 1);  // GPIO_D0
    //hal_dio_write_channel(g_handle, 72, 1);  // GPIO_D8


    //thread_sleep(1); // 1ms
    soft_delay(1000, 1000000);

    channel50Level = 0; // 0->1->0
    hal_dio_write_channel(g_handle, channel50, channel50Level);
    printf("hal_dio_write_channel: %d\n", channel50Level);
    hal_dio_write_channel(g_handle, 48, 0);
    //hal_dio_write_channel(g_handle, 56, 0);
    hal_dio_write_channel(g_handle, 64, 0);
    //hal_dio_write_channel(g_handle, 72, 0);  // GPIO_D8

    soft_delay(1000, 1000000);

    channel50Level = 1; // 0->1->0
    hal_dio_write_channel(g_handle, channel50, channel50Level);
    printf("hal_dio_write_channel: %d\n", channel50Level);
    hal_dio_write_channel(g_handle, 48, 1);
    //hal_dio_write_channel(g_handle, 56, 1);
    hal_dio_write_channel(g_handle, 64, 1);
    //hal_dio_write_channel(g_handle, 72, 1);  // GPIO_D8
#endif

#if 1
    soft_delay(1000, 1000000);

    /*
     * Write Channel Tests, GPIO_D1,D2
     */
    printf("hal_dio_write_channel: %d\n", 1);
    hal_dio_write_channel(g_handle, 65, 1);  // GPIO_D1, OK
    hal_dio_write_channel(g_handle, 66, 1);  // GPIO_D2

    soft_delay(1000, 1000000);

    printf("hal_dio_write_channel: %d\n", 0);
    hal_dio_write_channel(g_handle, 65, 0);  // OK
    hal_dio_write_channel(g_handle, 66, 0);

    soft_delay(1000, 1000000);

    printf("hal_dio_write_channel: %d\n", 1);
    hal_dio_write_channel(g_handle, 65, 1);  // OK
    hal_dio_write_channel(g_handle, 66, 1);
#endif

#if 1
    /*
     * Write Port Tests
     */
    soft_delay(1000, 1000000);  // 0

    portLevel = 0x55555555;
    printf("hal_dio_write_port: 0x%lx\n", portLevel);
    hal_dio_write_port(g_handle, portId, portLevel);  // OK

    soft_delay(1000, 1000000);  // 0xaaaa

    portLevel = 0xaaaaaaaa;
    printf("hal_dio_write_port: 0x%lx\n", portLevel);
    hal_dio_write_port(g_handle, portId, portLevel);  // OK
#endif
#if 1
    // FIXME: Write Port/P2/P3 NG, because bulid SPI looback. OK
    soft_delay(1000, 1000000);
    // 0000 0000 0000 0000 0000 0000 0000 0000

    portId = 2; // port2 or port3

    portLevel = 0x12345670;
    printf("hal_dio_write_port: 0x%lx\n", portLevel);
    hal_dio_write_port(g_handle, portId, portLevel);

    soft_delay(1000, 1000000);
    // 0000 1110 0000 1010 0010 1100 0100 1000  // P2, 0x07054321, OK

    portLevel = 0x00000000;
    printf("hal_dio_write_port: 0x%lx\n", portLevel);
    hal_dio_write_port(g_handle, portId, portLevel);

    soft_delay(1000, 1000000);
    // 0000 0000 0000 0000 0000 0000 0000 0000 // OK

    portLevel = 0xcba98765;
    printf("hal_dio_write_port: 0x%lx\n", portLevel);
    hal_dio_write_port(g_handle, portId, portLevel);

    soft_delay(1000, 1000000);
    // 1010 0110 1010 0001 1001 0101 1101 0011  // P2, 56589abc, OK

    portLevel = 0x00000000; // OK
    printf("hal_dio_write_port: 0x%lx\n", portLevel);
    hal_dio_write_port(g_handle, portId, portLevel);

    soft_delay(1000, 1000000);

    portLevel = 0xfedcba98;
    printf("hal_dio_write_port: 0x%lx\n", portLevel);
    hal_dio_write_port(g_handle, portId, portLevel);

    soft_delay(1000, 1000000);
    // 0001 1001 0001 1101 0011 1011 0111 1111 // P2, 898bcded, OK

    portLevel = 0xFFFFFFFF; // OK
    printf("hal_dio_write_port: 0x%lx\n", portLevel);
    hal_dio_write_port(g_handle, portId, portLevel);

    soft_delay(1000, 1000000);
    // 1111 1111 1111 1111 1111 1111 1111 1111

    hal_dio_write_channel(g_handle, 65, 0);  // GPIO_D1
    hal_dio_write_channel(g_handle, 66, 0);  // GPIO_D2

    soft_delay(1000, 1000000);
    // 1001 1111 1001 1111 1111 1111 1111 1111

    hal_dio_write_channel(g_handle, 65, 1);  // GPIO_D1
    hal_dio_write_channel(g_handle, 66, 1);  // GPIO_D2
    // 1111 1111 1111 1111 1111 1111 1111 1111

#endif

#if 1
    /*
     * Dio Read functions test.
     */
    printf("----------------Port1 Read test--------------\n"); // OK
    soft_delay(1000, 1000000);
    //0000010000000000

    // Port1: 48, 49, 50, 51  -> 56, 57, 58, 59
    portId = 1;

    hal_dio_write_channel(g_handle, 48, 1);  // GPIO_C0
    hal_dio_write_channel(g_handle, 49, 1);  // GPIO_C1
    hal_dio_write_channel(g_handle, 50, 1);  // GPIO_C2
    hal_dio_write_channel(g_handle, 51, 1);  // GPIO_C3

    tmpPortLevel = hal_dio_read_port(g_handle, 1);
    printf("hal_dio_read_port[%d]: 0x%lx\n", 1,
           tmpPortLevel); // 0x0f000000, C8~C11

    channelLevel = hal_dio_read_channel(g_handle, 56);
    printf("hal_dio_read_channel:[56] %d\n", channelLevel); // 1
    channelLevel = hal_dio_read_channel(g_handle, 57);
    printf("hal_dio_read_channel:[57] %d\n", channelLevel); // 1
    channelLevel = hal_dio_read_channel(g_handle, 58);
    printf("hal_dio_read_channel:[58] %d\n", channelLevel); // 1
    channelLevel = hal_dio_read_channel(g_handle, 59);
    printf("hal_dio_read_channel:[59] %d\n", channelLevel); // 1

    soft_delay(1000, 1000000);
    // 1111 0100 1111 0000

    hal_dio_write_channel(g_handle, 48, 1);  // GPIO_C0
    hal_dio_write_channel(g_handle, 49, 1);  // GPIO_C1
    hal_dio_write_channel(g_handle, 50, 0);  // GPIO_C2
    hal_dio_write_channel(g_handle, 51, 0);  // GPIO_C3

    tmpPortLevel = hal_dio_read_port(g_handle, 1);
    printf("hal_dio_read_port[%d]: 0x%lx\n", 1, tmpPortLevel); // 0x03000000

    channelLevel = hal_dio_read_channel(g_handle, 56);
    printf("hal_dio_read_channel:[56] %d\n", channelLevel); // 1
    channelLevel = hal_dio_read_channel(g_handle, 57);
    printf("hal_dio_read_channel:[57] %d\n", channelLevel); // 1
    channelLevel = hal_dio_read_channel(g_handle, 58);
    printf("hal_dio_read_channel:[58] %d\n", channelLevel); // 0
    channelLevel = hal_dio_read_channel(g_handle, 59);
    printf("hal_dio_read_channel:[59] %d\n", channelLevel); // 0

    hal_dio_write_channel(g_handle, 48, 1);  // GPIO_C0
    hal_dio_write_channel(g_handle, 49, 0);  // GPIO_C1
    hal_dio_write_channel(g_handle, 50, 0);  // GPIO_C2
    hal_dio_write_channel(g_handle, 51, 0);  // GPIO_C3

    tmpPortLevel = hal_dio_read_port(g_handle, 1);
    printf("hal_dio_read_port[%d]: 0x%lx\n", 1,
           tmpPortLevel); // 0x01000000,  C11/C10/C9/C8=0001

    tmpPortLevel = hal_dio_read_channel_group(g_handle, &readchannelGrp);
    printf("hal_dio_read_channel_group: 0x%lx\n", tmpPortLevel); // 0x1

    soft_delay(1000, 1000000);
    // 1000 0100 1000 0000, C0/C1/C2/C3, C8/C9/C10/C11

    hal_dio_write_channel(g_handle, 48, 0);  // GPIO_C0
    hal_dio_write_channel(g_handle, 49, 1);  // GPIO_C1
    hal_dio_write_channel(g_handle, 50, 0);  // GPIO_C2
    hal_dio_write_channel(g_handle, 51, 0);  // GPIO_C3

    tmpPortLevel = hal_dio_read_port(g_handle, 1);
    printf("hal_dio_read_port[%d]: 0x%lx\n", 1,
           tmpPortLevel); // 0x02000000, C11/C10/C9/C8=0010

    tmpPortLevel = hal_dio_read_channel_group(g_handle, &readchannelGrp);
    printf("hal_dio_read_channel_group: 0x%lx\n", tmpPortLevel); // 0x2

    soft_delay(1000, 1000000);
    // 0100 0100 0100 0000

    hal_dio_write_channel(g_handle, 48, 0);  // GPIO_C0
    hal_dio_write_channel(g_handle, 49, 0);  // GPIO_C1
    hal_dio_write_channel(g_handle, 50, 1);  // GPIO_C2
    hal_dio_write_channel(g_handle, 51, 0);  // GPIO_C3

    tmpPortLevel = hal_dio_read_port(g_handle, 1);
    printf("hal_dio_read_port[%d]: 0x%lx\n", 1,
           tmpPortLevel); // 0x04000000, C11/C10/C9/C8=0100

    tmpPortLevel = hal_dio_read_channel_group(g_handle, &readchannelGrp);
    printf("hal_dio_read_channel_group: 0x%lx\n", tmpPortLevel); // 0x4

    soft_delay(1000, 1000000);
    // 0010 0100 0010 0000

    hal_dio_write_channel(g_handle, 48, 0);  // GPIO_C0
    hal_dio_write_channel(g_handle, 49, 0);  // GPIO_C1
    hal_dio_write_channel(g_handle, 50, 0);  // GPIO_C2
    hal_dio_write_channel(g_handle, 51, 1);  // GPIO_C3

    tmpPortLevel = hal_dio_read_port(g_handle, 1);
    printf("hal_dio_read_port[%d]: 0x%lx\n", 1,
           tmpPortLevel); // 0x08000000, C11/C10/C9/C8=1000

    tmpPortLevel = hal_dio_read_channel_group(g_handle, &readchannelGrp);
    printf("hal_dio_read_channel_group: 0x%lx\n", tmpPortLevel); // 0x8

    soft_delay(1000, 1000000);
    // 0001 0100 0001 0000

    portLevel = 0x00;
    printf("hal_dio_write_channel_group: 0x%lx\n", portLevel);
    hal_dio_write_channel_group(g_handle, &channelGrp, portLevel); // 0x0

    soft_delay(1000, 1000000);
    // 0000 0100 0000 0000

    portLevel = 0x0f; //0x0f0000; // GPIO_C0~C3
    printf("hal_dio_write_channel_group: 0x%lx\n", portLevel);
    hal_dio_write_channel_group(g_handle, &channelGrp, portLevel); // 0xf0000

    soft_delay(1000, 1000000);
    // 1111 0100 1111 0000

    portLevel = 0x01; // GPIO_C0~C3
    printf("hal_dio_write_channel_group: 0x%lx\n", portLevel);
    hal_dio_write_channel_group(g_handle, &channelGrp,
                                portLevel); // 0x00010000

    tmpPortLevel = hal_dio_read_channel_group(g_handle, &readchannelGrp);
    printf("hal_dio_read_channel_group: 0x%lx\n", tmpPortLevel); // 0x1

    soft_delay(1000, 1000000);
    // 1000 0100 1000 0000

    portLevel = 0x02; // GPIO_C0~C3
    printf("hal_dio_write_channel_group: 0x%lx\n", portLevel);
    hal_dio_write_channel_group(g_handle, &channelGrp, portLevel); // 0x20000

    tmpPortLevel = hal_dio_read_channel_group(g_handle, &readchannelGrp);
    printf("hal_dio_read_channel_group: 0x%lx\n", tmpPortLevel); // 0x2

    soft_delay(1000, 1000000);
    // 0100 0100 0100 0000

    portLevel = 0x04; // GPIO_C0~C3
    printf("hal_dio_write_channel_group: 0x%lx\n", portLevel);
    hal_dio_write_channel_group(g_handle, &channelGrp, portLevel); // 0x40000

    tmpPortLevel = hal_dio_read_channel_group(g_handle, &readchannelGrp);
    printf("hal_dio_read_channel_group: 0x%lx\n", tmpPortLevel); // 0x4

    soft_delay(1000, 1000000);
    // 0010 0100 0010 0000

    portLevel = 0x08; // GPIO_C0~C3
    printf("hal_dio_write_channel_group: 0x%lx\n", portLevel);
    hal_dio_write_channel_group(g_handle, &channelGrp,
                                portLevel); // 0x00080000

    tmpPortLevel = hal_dio_read_channel_group(g_handle, &readchannelGrp);
    printf("hal_dio_read_channel_group: 0x%lx\n", tmpPortLevel); // 0x8

    soft_delay(1000, 1000000);
    // 0001 0100 0001 0000

    channelLevel = hal_dio_read_channel(g_handle, 56);
    printf("hal_dio_read_channel:[56] %d\n", channelLevel); // 0
    channelLevel = hal_dio_read_channel(g_handle, 57);
    printf("hal_dio_read_channel:[57] %d\n", channelLevel); // 0
    channelLevel = hal_dio_read_channel(g_handle, 58);
    printf("hal_dio_read_channel:[58] %d\n", channelLevel); // 0
    channelLevel = hal_dio_read_channel(g_handle, 59);
    printf("hal_dio_read_channel:[59] %d\n", channelLevel); // 1

    tmpPortLevel = hal_dio_read_port(g_handle, portId);
    printf("hal_dio_read_port: 0x%lx\n", tmpPortLevel); // 0x8000000

    tmpPortLevel = hal_dio_read_channel_group(g_handle, &readchannelGrp);
    printf("hal_dio_read_channel_group: 0x%lx\n", tmpPortLevel);  // 0x8

    printf("----------------Port2 Read test--------------\n");
    soft_delay(1000, 1000000);
    //10000000100000000000000000000000
    // 0001 0100 0001 0000

    //Port2: 64, 65, 66, 67  -> 72, 73, 74, 75, OK
    portId = 2;

    hal_dio_write_channel(g_handle, 64, 1);  // GPIO_D0
    hal_dio_write_channel(g_handle, 65, 1);  // GPIO_D1
    hal_dio_write_channel(g_handle, 66, 1);  // GPIO_D2
    hal_dio_write_channel(g_handle, 67, 1);  // GPIO_D3

    tmpPortLevel = hal_dio_read_port(g_handle, 2);
    printf("hal_dio_read_port[%d]: 0x%lx\n", 2, tmpPortLevel); // 0x00000f00

    channelLevel = hal_dio_read_channel(g_handle, 72);
    printf("hal_dio_read_channel:[72] %d\n", channelLevel); // 1
    channelLevel = hal_dio_read_channel(g_handle, 73);
    printf("hal_dio_read_channel:[73] %d\n", channelLevel); // 1
    channelLevel = hal_dio_read_channel(g_handle, 74);
    printf("hal_dio_read_channel:[74] %d\n", channelLevel); // 1
    channelLevel = hal_dio_read_channel(g_handle, 75);
    printf("hal_dio_read_channel:[75] %d\n", channelLevel); // 1

    soft_delay(1000, 1000000);
    //1111 0000 1111 0000 0000 0000 0000 0000

    hal_dio_write_channel(g_handle, 64, 0);  // GPIO_D0
    hal_dio_write_channel(g_handle, 65, 1);  // GPIO_D1
    hal_dio_write_channel(g_handle, 66, 0);  // GPIO_D2
    hal_dio_write_channel(g_handle, 67, 1);  // GPIO_D3

    tmpPortLevel = hal_dio_read_port(g_handle, 2);
    printf("hal_dio_read_port[%d]: 0x%lx\n", 2,
           tmpPortLevel); // 0x00000a00, D11/D10/D9/D8=1010

    channelLevel = hal_dio_read_channel(g_handle, 72);
    printf("hal_dio_read_channel:[72] %d\n", channelLevel); // 0
    channelLevel = hal_dio_read_channel(g_handle, 73);
    printf("hal_dio_read_channel:[73] %d\n", channelLevel); // 1
    channelLevel = hal_dio_read_channel(g_handle, 74);
    printf("hal_dio_read_channel:[74] %d\n", channelLevel); // 0
    channelLevel = hal_dio_read_channel(g_handle, 75);
    printf("hal_dio_read_channel:[75] %d\n", channelLevel); // 1

    soft_delay(1000, 1000000);
    //0101 0000 0101 0000 0000 0000 0000 0000, D8/D9/D10/D11

    portLevel = 0x00000000;
    mask = 0x0000000f;
    printf("hal_dio_masked_write_port: 0x%lx\n", portLevel);
    hal_dio_masked_write_port(g_handle, portId, portLevel, mask); // 0x0

    tmpPortLevel = hal_dio_read_port(g_handle, portId);
    printf("hal_dio_read_port: 0x%lx\n", tmpPortLevel);

    soft_delay(1000, 1000000);
    //00000000000000000000000000000000

    portLevel = 0x00000007; // D11/D10/D9/D8=0111
    mask = 0x0000000f;
    printf("hal_dio_masked_write_port: 0x%lx\n", portLevel);
    hal_dio_masked_write_port(g_handle, portId, portLevel, mask); // 0x7

    tmpPortLevel = hal_dio_read_port(g_handle, portId);
    printf("hal_dio_read_port: 0x%lx\n",
           tmpPortLevel); // 0x700, D11/D10/D9/D8=0111

    soft_delay(1000, 1000000);
    //1110 0000 1110 0000 0000 00000 0000 0000

    portLevel = 0x0000000e; // D11/D10/D9/D8=1110
    mask = 0x0000000f;
    printf("hal_dio_masked_write_port: 0x%lx\n", portLevel);
    hal_dio_masked_write_port(g_handle, portId, portLevel, mask); // 0xe

    tmpPortLevel = hal_dio_read_port(g_handle, portId);
    printf("hal_dio_read_port: 0x%lx\n", tmpPortLevel); // 0xe00

    soft_delay(1000, 1000000);
    //01110000011100000000000000000000

    channelLevel = hal_dio_read_channel(g_handle, 72);
    printf("hal_dio_read_channel:[72] %d\n", channelLevel); // 0
    channelLevel = hal_dio_read_channel(g_handle, 73);
    printf("hal_dio_read_channel:[73] %d\n", channelLevel); // 1
    channelLevel = hal_dio_read_channel(g_handle, 74);
    printf("hal_dio_read_channel:[74] %d\n", channelLevel); // 1
    channelLevel = hal_dio_read_channel(g_handle, 75);
    printf("hal_dio_read_channel:[75] %d\n", channelLevel); // 1

    //soft_delay(1000, 1000000);

    tmpPortLevel = hal_dio_read_port(g_handle, portId);
    printf("hal_dio_read_port: 0x%lx\n", tmpPortLevel); // 0xe00

    soft_delay(1000, 1000000);
    //01110000011100000000000000000000
#endif

#if 1
    /*
     * Dio Write channel group test
     */
    // 0110 1110 0110 1010 0010 1100 0100 1000  // 0x12345676
    // {0x0000ffff, 0x0f, 2};  // port2
    portLevel = 0xfffffff;
    printf("hal_dio_write_channel_group: 0x%lx\n", portLevel);
    hal_dio_write_channel_group(g_handle, &channelGrp, portLevel);  // OK

    soft_delay(1000, 1000000);  // 0x1234d676
    // 0110 1110 0110 1011 0010 1100 0100 1000

    portLevel = 0x00000000;
    printf("hal_dio_write_port: 0x%lx\n", portLevel);
    hal_dio_write_port(g_handle, portId, portLevel);

    soft_delay(1000, 1000000);

    portLevel = 0x00ffff00;
    printf("hal_dio_write_channel_group: 0x%lx\n", portLevel);
    hal_dio_write_channel_group(g_handle, &channelGrp, portLevel);  // OK

    soft_delay(1000, 1000000);
    // 0110 1110 0110 1011 0010 1100 0100 1000
#endif

#if 1
    /*
     * Dio Flip/ masked write port test
     */
    channelLevel = hal_dio_flip_channel(g_handle, channel63);  // GPIO_C15, OK
    printf("hal_dio_flip_channel: %d\n", channelLevel);

    soft_delay(1000, 1000000);  // 0->1

    channelLevel = hal_dio_flip_channel(g_handle, channel63);  // OK
    printf("hal_dio_flip_channel: %d\n", channelLevel);

    soft_delay(1000, 1000000);  // 1->0
    // 0101 0101 0101 0101 0101 0101 0101 0101

    portLevel = 0xf0f0f0f0;
    mask = 0x0000ffff;
    printf("hal_dio_masked_write_port: 0x%lx\n", portLevel);
    hal_dio_masked_write_port(g_handle, portId, portLevel, mask);  // OK

    // 0000 1111 0000 1111 0101 0101 0101 0101
#endif

    //thread_sleep(1); // 1ms
    //soft_delay(1000000, 1000000);

    hal_dio_release_handle(&g_handle);
    return ret;
}

static int cmd_port(int argc, const cmd_args *argv)
{
    bool ret = true;
    static void *g_handle;
    // for test
    //int32_t i = 0;

    printf("+++++port sample cmd_port 15\n");
    // RES_PAD_CONTROL_SEC_GPIO_MUX2_IO107, RES_IOMUXC_RTC_IOMUXC_RTC
    ret = hal_port_creat_handle(&g_handle,
                                g_iomuxc_res.res_id[0]);  // FIXME: only IOMUXC_RTC availbale for dummy, zhuming, 191118

    if (ret == false) {
        return ret;
    }

#if 0
    // FIXME: How to used the domain_res.h to check valid or not ? zhuming, 191118
    /* get all IOMUXC res phy_addr & real_idx */
    printf("-----------port sample: get all IOMUX RES--------------\n");

    for (i = 0; i < g_iomuxc_res.res_num; i++) {
        printf("i[%d], res_id[0x%lx]\n", i, g_iomuxc_res.res_id[i]);
        hal_port_check_res(g_handle, g_iomuxc_res.res_id[i]);
    }

    printf("-----------port sample: get all IOMUX RES END-----------\n");
#endif

    /*get handle ok and enable port is true*/
    ret = hal_port_init(g_handle);

    if (ret) {
        ret = hal_port_set_pin_mode(g_handle, pin72, mode72);
    }
    else {
        hal_port_release_handle(&g_handle);
        return false;
    }

    if (ret) {
        ret = hal_port_set_pin_mode(g_handle, pin73, mode73);
    }
    else {
        hal_port_release_handle(&g_handle);
        return false;
    }

    if (ret) {
        ret = hal_port_set_pin_mode(g_handle, pin74, mode74);
    }
    else {
        hal_port_release_handle(&g_handle);
        return false;
    }

    if (ret) {
        ret = hal_port_set_pin_mode(g_handle, pin75, mode75);
    }
    else {
        hal_port_release_handle(&g_handle);
        return false;
    }

    if (ret) {
        ret = hal_port_set_pin_direction(g_handle, pin48, direction48);
    }
    else {
        hal_port_release_handle(&g_handle);
        return false;
    }

    if (ret) {
        ret = hal_port_refresh_port_direction(g_handle);
    }
    else {
        hal_port_release_handle(&g_handle);
        return false;
    }

    hal_port_release_handle(&g_handle);
    return 0;
}

static void port_entry(const struct app_descriptor *app, void *args)
{
    printf("++++ port sample port_entry\n");
    cmd_port(0, 0);

    printf("++++ dio sample port_entry\n");
    cmd_dio(0, 0);
}

APP_START(port_example)
.flags = 0,
.entry = port_entry,
APP_END
