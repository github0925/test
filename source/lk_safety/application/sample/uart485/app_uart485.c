/*
* app_uart485.c
*
* Copyright (c) 2018 Semidrive Semiconductor.
* All rights reserved.
*
* Description: implement the app to auto jump to kernel.
*
* Revision History:
* -----------------
* 001, 10/20/2019 henglei create this file
*/
#include <app.h>
#include <stdlib.h>
#include <string.h>
#include <debug.h>
#include <platform.h>
#include <lib/console.h>
#include <arch.h>
#include <reg.h>
#include <stdio.h>
#include <trace.h>
#include <lib/cbuf.h>
#include <kernel/thread.h>
#include <platform/interrupts.h>
#if WITH_KERNEL_VM
#include <kernel/vm.h>
#endif
#include <uart_hal.h>
#include <irq_v.h>
#include <__regs_base.h>
#include <target_res.h>
#include <ckgen_init.h>

#include "hal_port.h"
#include "hal_dio.h"
#include "res.h"
#include "chip_res.h"

//#include "iomux_ctrl.h"

#define PLATFORM_UART_DEBUG_LEVEL CRITICAL

//#define RXBUF_SIZE  16
#define UART_SCLK   CKGEN_UART_SAF  /* 84M */
#define UART_BAUD   115200

#define udelay(x) spin(x)
#define mdelay(x) spin(x * 1000)
extern const domain_res_t g_gpio_res;
void *uart485_port_handle;
void *uart485_dio_handle;
char buf = 0x00;

const Port_PinType UART485_TX_PIN = PortConf_PIN_GPIO_A6;
const Port_PinModeType UART485_TX_PIN_MODE = {
    ((uint32_t)PORT_PAD_POE__DISABLE | PORT_PAD_IS__OUT | PORT_PAD_SR__FAST | PORT_PAD_DS__MID1 | PORT_PIN_IN_NO_PULL ),
    ((uint32_t)PORT_PIN_MUX_FV__MIN | PORT_PIN_MUX_FIN__MIN  | PORT_PIN_OUT_PUSHPULL | PORT_PIN_MODE_ALT1),
};

const Port_PinType UART485_RX_PIN = PortConf_PIN_GPIO_A7;
const Port_PinModeType UART485_RX_PIN_MODE = {
    ((uint32_t)PORT_PAD_POE__DISABLE | PORT_PAD_IS__OUT | PORT_PAD_SR__FAST | PORT_PAD_DS__MID1 | PORT_PIN_IN_NO_PULL ),
    ((uint32_t)PORT_PIN_MUX_FV__MIN | PORT_PIN_MUX_FIN__MIN  | PORT_PIN_OUT_PUSHPULL | PORT_PIN_MODE_ALT1),
};

const Port_PinType UART485_DE_PIN = PortConf_PIN_GPIO_A2;
const Port_PinModeType UART485_DE_PIN_MODE = {
    ((uint32_t)PORT_PAD_POE__DISABLE | PORT_PAD_IS__OUT | PORT_PAD_SR__FAST | PORT_PAD_DS__MID1 | PORT_PIN_IN_NO_PULL ),
    ((uint32_t)PORT_PIN_MUX_FV__MIN | PORT_PIN_MUX_FIN__MIN  | PORT_PIN_OUT_PUSHPULL | PORT_PIN_MODE_ALT5),
};

const Port_PinType UART485_RE_PIN = PortConf_PIN_GPIO_A3;
const Port_PinModeType UART485_RE_PIN_MODE = {
    ((uint32_t)PORT_PAD_POE__DISABLE | PORT_PAD_IS__OUT | PORT_PAD_SR__FAST | PORT_PAD_DS__MID1 | PORT_PIN_IN_NO_PULL ),
    ((uint32_t)PORT_PIN_MUX_FV__MIN | PORT_PIN_MUX_FIN__MIN  | PORT_PIN_OUT_PUSHPULL | PORT_PIN_MODE_ALT5),
};


const Port_PinType UART485_RTS_PIN = PortConf_PIN_GPIO_A4;
const Port_PinModeType UART485_RTS_PIN_MODE = {
    ((uint32_t)PORT_PAD_POE__DISABLE | PORT_PAD_IS__OUT | PORT_PAD_SR__FAST | PORT_PAD_DS__MID1 | PORT_PIN_IN_NO_PULL ),
    ((uint32_t)PORT_PIN_MUX_FV__MIN | PORT_PIN_MUX_FIN__MIN  | PORT_PIN_OUT_PUSHPULL | PORT_PIN_MODE_GPIO),
};

const Port_PinType UART485_CTS_PIN = PortConf_PIN_GPIO_A5;
const Port_PinModeType UART485_CTS_PIN_MODE = {
    ((uint32_t)PORT_PAD_POE__DISABLE | PORT_PAD_IS__OUT | PORT_PAD_SR__FAST | PORT_PAD_DS__MID1 | PORT_PIN_IN_NO_PULL ),
    ((uint32_t)PORT_PIN_MUX_FV__MIN | PORT_PIN_MUX_FIN__MIN  | PORT_PIN_OUT_PUSHPULL | PORT_PIN_MODE_GPIO),
};

static void *uart_handle;

static void uart_platform_cfg_printf(uart_instance_t *instance, hal_uart_cfg_t *hal_cfg)
{
    printf("Platform UART%d port: sclk=%d, baud=%d, ", instance->phy_num, hal_cfg->port_cfg.sclk, hal_cfg->port_cfg.baud );

    if (hal_cfg->port_cfg.data_bits == UART_HAL_CHAR_5BITS) {
        printf("data bits=5, ");
    }
    else if (hal_cfg->port_cfg.data_bits == UART_HAL_CHAR_6BITS) {
        printf("data bits=6, ");
    }
    else if (hal_cfg->port_cfg.data_bits == UART_HAL_CHAR_7BITS) {
        printf("data bits=7, ");
    }
    else if (hal_cfg->port_cfg.data_bits == UART_HAL_CHAR_8BITS) {
        printf("data bits=8, ");
    }

    if (hal_cfg->port_cfg.stop_bits == UART_HAL_STOP_1BIT) {
        printf("stop bits=1, ");
    }
    else if (hal_cfg->port_cfg.stop_bits == UART_HAL_STOP_1_5BIT) {
        printf("stop bits=1.5, ");
    }
    else if (hal_cfg->port_cfg.stop_bits == UART_HAL_STOP_2BIT) {
        printf("stop bits=2, ");
    }

    if (hal_cfg->port_cfg.parity == UART_HAL_ODD_PARITY) {
        printf("parity=odd!\r\n");
    }
    else if (hal_cfg->port_cfg.parity == UART_HAL_EVEN_PARITY) {
        printf("parity=even!\r\n");
    }
    else if (hal_cfg->port_cfg.parity == UART_HAL_NO_PARITY) {
        printf("parity=no!\r\n");
    }
}

int recive_callback(uint8_t data) {
    buf = data;
    printf("@@@@@buf = %c\n", buf);
    return 0;
};

int uart485_test(void)
{
    hal_uart_cfg_t  hal_cfg;
    uart_instance_t *instance;
    char buf_tr[10] = {'F','B','C','D','1','2','3','x','y','z'};
    char tr_buf = 'A';
    int i;
    printf("app:[uart_thread_rev] %s():.\n", __func__);


    hal_port_creat_handle(&uart485_port_handle, RES_PAD_CONTROL_SAF_JTAG_TMS);//safety iomuxc res

    if (uart485_port_handle) {
        hal_port_set_pin_mode(uart485_port_handle, UART485_RTS_PIN, UART485_RTS_PIN_MODE);
        hal_port_set_pin_mode(uart485_port_handle, UART485_CTS_PIN, UART485_CTS_PIN_MODE);

        hal_port_set_pin_mode(uart485_port_handle, UART485_TX_PIN, UART485_TX_PIN_MODE);
        hal_port_set_pin_mode(uart485_port_handle, UART485_RX_PIN, UART485_RX_PIN_MODE);
        hal_port_set_pin_mode(uart485_port_handle, UART485_DE_PIN, UART485_DE_PIN_MODE);
        hal_port_set_pin_mode(uart485_port_handle, UART485_RE_PIN, UART485_RE_PIN_MODE);

        hal_port_set_pin_direction(uart485_port_handle, UART485_DE_PIN, PORT_PIN_OUT);
        hal_port_set_pin_direction(uart485_port_handle, UART485_RE_PIN, PORT_PIN_OUT);

        hal_port_set_pin_direction(uart485_port_handle, UART485_RTS_PIN, PORT_PIN_IN);
        hal_port_set_pin_direction(uart485_port_handle, UART485_CTS_PIN, PORT_PIN_IN);
    }
    else {
        printf("port get handle failed!\n");
    }

    hal_port_release_handle(uart485_port_handle);

/*	hal_dio_creat_handle(&uart485_dio_handle, g_gpio_res.res_id[0]);

	hal_dio_write_channel(uart485_dio_handle, UART485_DE_PIN, 0);
	hal_dio_write_channel(uart485_dio_handle, UART485_RE_PIN, 0);*/

    hal_uart_creat_handle(&uart_handle, RES_UART_UART2);

    if (uart_handle != NULL) {
       instance = (uart_instance_t *)uart_handle;

       hal_cfg.port_cfg.sclk = UART_SCLK;
       hal_cfg.port_cfg.baud = UART_BAUD;
       hal_cfg.port_cfg.data_bits = UART_HAL_CHAR_8BITS;
       hal_cfg.port_cfg.stop_bits = UART_HAL_STOP_1BIT;
       hal_cfg.port_cfg.parity = UART_HAL_NO_PARITY;
       hal_cfg.fifo_cfg.fifo_enable = true;
       hal_cfg.fifo_cfg.rx_trigger = UART_HAL_RX_FIFO_CHAR_1;
       hal_cfg.fifo_cfg.tx_trigger = UART_HAL_TX_FIFO_EMPTY;

       hal_cfg.rs485_cfg.rs485_enable = true;
       hal_cfg.rs485_cfg.re_polarity = 0;
       hal_cfg.rs485_cfg.de_polarity = 1;
       hal_cfg.rs485_cfg.transfer_mode = 2;
       hal_cfg.rs485_cfg.de_assert_timer = 0x32;
       hal_cfg.rs485_cfg.de_deassert_time = 0xf;
       hal_cfg.rs485_cfg.de2re_turn_around_time = 0x51;
       hal_cfg.rs485_cfg.re2de_turn_around_time = 0x38;

#ifdef UART_DRV_SUPPORT_9BITS
       hal_cfg.nine_bits_cfg.nine_bits_enable = false;
#endif
#ifdef UART_DRV_SUPPORT_DMA
       hal_cfg.dma_cfg.dma_enable = false;
#endif
       hal_uart_init(uart_handle, &hal_cfg);
       hal_uart_int_cbk_register(uart_handle, UART_HAL_RX_CHAR_INT_SRC, recive_callback);
       hal_uart_int_src_disable(uart_handle, UART_HAL_RX_STR_INT_SRC);
       hal_uart_int_src_disable(uart_handle, UART_HAL_TX_EMPTY_INT_SRC);
       hal_uart_int_src_enable(uart_handle, UART_HAL_RX_CHAR_INT_SRC);

       uart_platform_cfg_printf(instance, &hal_cfg);
    }

    hal_uart_transmit(uart_handle, &buf_tr[2], 1, true);
    while(1){
       while(1){
          if(buf != 0x00){
              break ;
          }
       }

       hal_uart_transmit(uart_handle, &buf, 1, true);
       buf = 0x00;
    }

    return 0;

}

#if defined(WITH_LIB_CONSOLE)
STATIC_COMMAND_START
STATIC_COMMAND("uart485_test", "test uart485", (console_cmd)&uart485_test)
STATIC_COMMAND_END(uart485_test);
#endif


APP_START(uart485_test)
.flags = 0
APP_END
