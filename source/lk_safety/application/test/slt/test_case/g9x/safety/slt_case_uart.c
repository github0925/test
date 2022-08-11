/*
 * Copyright (c) 2019, SemiDrive, Inc. All rights reserved
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
//uart1  <-> mp
//uart2  <-> safety
//uart3  <-> mp        <-> lin2
//uart4  <-> safety    <-> tty
//uart5  <-> mp
//uart6  <-> mp
//uart7  <-> mp        <-> lin0
//uart8  <-> mp        <-> lin1
//uart9  <-> mp        <-> tty
//uart10 <-> mp
//uart13 <-> mp
//uart15 <-> mp

#include <lib/console.h>
#include <assert.h>
#include <bits.h>
#include <kernel/event.h>
#include <kernel/mutex.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include "uart_hal.h"
#include <lib/slt_module_test.h>

#define TIMEOUT    5000
typedef int(*slt_uart_callback)(uint8_t data);
static int slt_uart2_interrupt(uint8_t data);

typedef enum {
    UART_CHN2 = 0,
    UART_CHN_MAX
} usart_chn_e;

typedef struct {
    uint32_t hrdChannel;
    uint32_t sclk;
    uint32_t baud;
    uint8_t data_bits;
    uint8_t stop_bits;
    uint8_t parity;
    uint8_t loopback_enable;
    uint8_t fifo_enable;
    uint8_t rx_trigger;
    uint8_t tx_trigger;
} uart_contrl_cfg_t;

typedef struct {
    usart_chn_e count;
    const uart_contrl_cfg_t *config;
} uart_config_t;

typedef struct {
    bool    s_flag;
    void    *handle;
    mutex_t mutex;
    event_t event;
    uint8_t txdata[1];
    bool result;
} uart_ChannelType_t;

static uart_ChannelType_t urat_channel[UART_CHN_MAX];
static slt_uart_callback uart_callback[UART_CHN_MAX] = {
    slt_uart2_interrupt,
};

const static uart_contrl_cfg_t uart_contrl_cfg[UART_CHN_MAX] = {
    [UART_CHN2] = {
        .hrdChannel = RES_UART_UART2,
        .sclk = 80000000,
        .baud = 19200,
        .data_bits = UART_HAL_CHAR_8BITS,
        .stop_bits = UART_HAL_STOP_1BIT,
        .parity = UART_HAL_NO_PARITY,
        .loopback_enable = true,
        .fifo_enable = true,
        .rx_trigger = UART_HAL_RX_FIFO_CHAR_1,
        .tx_trigger = UART_HAL_TX_FIFO_EMPTY
    }
};

const static uart_config_t uart_config = {
    .count = UART_CHN_MAX,
    .config = uart_contrl_cfg
};

static void slt_uart_interrupt(usart_chn_e chn, uint8_t data)
{
    if (urat_channel[chn].txdata[0] == data)
        urat_channel[chn].result = true;

    printf("slt_uart_interrupt chn=%d, data=%d\n", chn, data);

    event_signal(&urat_channel[chn].event, true);
}

static int slt_uart2_interrupt(uint8_t data)
{
    printf("slt_uart2_interrupt data = %d\n", data);
    slt_uart_interrupt(UART_CHN2, data);
    return 0;
}

static void slt_uart_SendOneByte(uint8_t Channel, uint8_t Data)
{
    hal_uart_putc(urat_channel[Channel].handle, Data);
}

static void inline slt_uart_config(uint8_t Channel,
                                   hal_uart_cfg_t *hal_uart_cfg,
                                   const uart_config_t *uart_config)
{
    if ((uart_config == NULL) || (Channel >= uart_config->count)) {
        return;
    }

    hal_uart_cfg->port_cfg.sclk = uart_config->config[Channel].sclk;
    hal_uart_cfg->port_cfg.baud = uart_config->config[Channel].baud;
    hal_uart_cfg->port_cfg.data_bits = uart_config->config[Channel].data_bits;
    hal_uart_cfg->port_cfg.stop_bits = uart_config->config[Channel].stop_bits;
    hal_uart_cfg->port_cfg.parity = uart_config->config[Channel].parity;
    hal_uart_cfg->port_cfg.loopback_enable =
        uart_config->config[Channel].loopback_enable;
    hal_uart_cfg->fifo_cfg.fifo_enable = uart_config->config[Channel].fifo_enable;
    hal_uart_cfg->fifo_cfg.rx_trigger = uart_config->config[Channel].rx_trigger;
    hal_uart_cfg->fifo_cfg.tx_trigger = uart_config->config[Channel].tx_trigger;
}

void slt_UartInit(const uart_config_t *config)
{
    hal_uart_cfg_t uartCfg;
    uart_ChannelType_t *chn;
    const uart_config_t *pConfigPtr = config;

    if (NULL == pConfigPtr) {
        return;
    }

    for (usart_chn_e Index = UART_CHN2; Index < config->count; Index++) {
        chn = &urat_channel[Index];

        if (pConfigPtr->config[Index].hrdChannel == 0) {
            printf("uart channel %d not configured. Ignore\n", Index);
            continue;
        }

        if (chn->s_flag == true) {
            continue;
        }

        chn->s_flag = true;

        /* Configure UART channel. */
        hal_uart_creat_handle((void **)&chn->handle,
                              pConfigPtr->config[Index].hrdChannel);
        printf("Index = %d, chn->handle =%p\n", Index, chn->handle);
        ASSERT(NULL != chn->handle);

        slt_uart_config(Index, &uartCfg, pConfigPtr);
        hal_uart_init(chn->handle, &uartCfg);
        hal_uart_int_cbk_register(chn->handle, UART_DRV_RX_CHAR_INT_SRC,
                                  uart_callback[Index]);
        chn->txdata[0] = Index;
        mutex_init(&chn->mutex);
        event_init(&chn->event, false, EVENT_FLAG_AUTOUNSIGNAL);
    }
}

static int slt_uart_SendFrame(uint8_t channel, uart_ChannelType_t *channel_type)
{
    int ret = -1;
    uart_ChannelType_t *chn = channel_type;

    if (NULL == chn)
        return ret;

    mutex_acquire(&chn->mutex);
    slt_uart_SendOneByte(channel, chn->txdata[0]);
    ret = event_wait_timeout(&chn->event, TIMEOUT);
    mutex_release(&chn->mutex);

    return ret;
}

static int slt_uart_internal_ip_diagnose(uart_ChannelType_t *channel_type)
{
    int ret = -1;

    for (usart_chn_e nr = UART_CHN2; nr < UART_CHN_MAX; nr++) {
        if (channel_type[nr].handle == NULL)
            continue;

        ret = slt_uart_SendFrame(nr, &channel_type[nr]);
    }

    return ret;
}

static int slt_uart_checkout_result(uart_ChannelType_t *channel_type)
{
    int ret = -1;

    for (usart_chn_e nr = UART_CHN2; nr < UART_CHN_MAX; nr++) {
        if (channel_type[nr].result == true)
            printf("uart%d pass\n", nr);
        else {
            printf("uart%d fail\n", nr);
            goto out;
        }
    }

    ret = 0;

out:
    return ret;
}

int TEST_SAFE_SS_07(uint times, uint timeout, char *result_string)
{
    int ret = -1;
    slt_UartInit(&uart_config);
    slt_uart_internal_ip_diagnose(urat_channel);
    ret = slt_uart_checkout_result(urat_channel);
    return ret;
}

// test case name: module_test_sample1
// test case entry: slt_module_test_sample_hook_1
// test case level: SLT_MODULE_TEST_LEVEL_SAMPLE_1(must define in enum slt_module_test_level)
// test case parallel: test parallel with other test case
// test case time: run test times in test case, if not support set default 1
// test case timeout: run test timeout default value, must bigger than case us time, or case will be kill
// test case flag: usd user define stack size
// test case stack size: user define stack size
SLT_MODULE_TEST_HOOK_DETAIL(safe_ss_07, TEST_SAFE_SS_07,
                            SLT_MODULE_TEST_LEVEL_SAMPLE_1, 0, 1, 500, 0, 0);


