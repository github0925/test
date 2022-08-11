/*
* uart.c
*
* Copyright (c) 2018 Semidrive Semiconductor.
* All rights reserved.
*
* Description: platform uart high level api
*
* Revision History:
* -----------------
* 011, 11/24/2018 chenqing create this file
* 012, 10/28/2019 wang yongjun update as call uart hal
*/
#include <reg.h>
#include <stdio.h>
#include <trace.h>
#include <lib/cbuf.h>
#include <kernel/thread.h>
#include <platform/interrupts.h>
#include <platform/debug.h>
#if WITH_KERNEL_VM
#include <kernel/vm.h>
#endif
#include <uart_hal.h>
#include <irq_v.h>
#include <__regs_base.h>
#include <target_res.h>
#include <ckgen_init.h>
//#include "iomux_ctrl.h"

#define PLATFORM_UART_DEBUG_LEVEL INFO

#define RXBUF_SIZE  16
#define UART_SCLK   CKGEN_UART_SEC1  /* 60M */
#define UART_BAUD   115200

static cbuf_t uart_rx_buf;
static void *uart_handle = NULL;


/******************************************************************************
 ** \brief save received character to uart_rx_buf.
 **
 ** \param [in]
 ** \param [out]
 *****************************************************************************/
static int save_char(uint8_t data)
{
#if CONSOLE_HAS_INPUT_BUFFER
    cbuf_write_char(&console_input_cbuf, c, false);
#else

    if (cbuf_space_avail(&uart_rx_buf) == 0) {  //uart_rx_buf no space
        hal_uart_int_src_disable(uart_handle,
                                 UART_HAL_RX_CHAR_INT_SRC); //disable the receive data interrupt
        return 0;
    }

    cbuf_write_char(&uart_rx_buf, data, false);
    //dprintf(PLATFORM_UART_DEBUG_LEVEL, "%c", data);
#endif
    return 1;
}

/******************************************************************************
 ** \brief Init the receive cbuf, register the interrupt.
 **
 ** \param [in]
 ** \param [out]
 *****************************************************************************/
void uart_init(void)
{
#if 0
    /************ below will be change with the iomux hal ************/
    iomux_UART9_RX_1_ENABLE(0);
    iomux_UART9_TX_1_ENABLE(0);
    uint32_t addr, val;
    /*
     * Z1 only - disable pull of uart9 tx/rx.
     */
    addr = APB_IOMUXC_SEC_BASE + (0x10 << 10);
    val = readl(addr);
    val &= ~1ul;
    writel(val, addr);
    addr = APB_IOMUXC_SEC_BASE + (0x14 << 10);
    val = readl(addr);
    val &= ~1ul;
    writel(val, addr);
    /************ above will be change with the iomux hal ************/
#endif

    if (uart_handle != NULL) {
        // create circular buffer to hold received data
        cbuf_initialize(&uart_rx_buf, RXBUF_SIZE);
        hal_uart_int_cbk_register(uart_handle, UART_HAL_RX_CHAR_INT_SRC,
                                  save_char);
        dprintf(PLATFORM_UART_DEBUG_LEVEL, "Platform UART inited!\r\n");
    }
}

static void uart_platform_cfg_printf(uart_instance_t *instance,
                                     hal_uart_cfg_t *hal_cfg)
{
    dprintf(PLATFORM_UART_DEBUG_LEVEL,
            "Platform UART%d port: sclk=%d, baud=%d, ",
            instance->phy_num,
            hal_cfg->port_cfg.sclk,
            hal_cfg->port_cfg.baud);

    if (hal_cfg->port_cfg.data_bits == UART_HAL_CHAR_5BITS) {
        dprintf(PLATFORM_UART_DEBUG_LEVEL, "data bits=5, ");
    }
    else if (hal_cfg->port_cfg.data_bits == UART_HAL_CHAR_6BITS) {
        dprintf(PLATFORM_UART_DEBUG_LEVEL, "data bits=6, ");
    }
    else if (hal_cfg->port_cfg.data_bits == UART_HAL_CHAR_7BITS) {
        dprintf(PLATFORM_UART_DEBUG_LEVEL, "data bits=7, ");
    }
    else if (hal_cfg->port_cfg.data_bits == UART_HAL_CHAR_8BITS) {
        dprintf(PLATFORM_UART_DEBUG_LEVEL, "data bits=8, ");
    }

    if (hal_cfg->port_cfg.stop_bits == UART_HAL_STOP_1BIT) {
        dprintf(PLATFORM_UART_DEBUG_LEVEL, "stop bits=1, ");
    }
    else if (hal_cfg->port_cfg.stop_bits == UART_HAL_STOP_1_5BIT) {
        dprintf(PLATFORM_UART_DEBUG_LEVEL, "stop bits=1.5, ");
    }
    else if (hal_cfg->port_cfg.stop_bits == UART_HAL_STOP_2BIT) {
        dprintf(PLATFORM_UART_DEBUG_LEVEL, "stop bits=2, ");
    }

    if (hal_cfg->port_cfg.parity == UART_HAL_ODD_PARITY) {
        dprintf(PLATFORM_UART_DEBUG_LEVEL, "parity=odd!\r\n");
    }
    else if (hal_cfg->port_cfg.parity == UART_HAL_EVEN_PARITY) {
        dprintf(PLATFORM_UART_DEBUG_LEVEL, "parity=even!\r\n");
    }
    else if (hal_cfg->port_cfg.parity == UART_HAL_NO_PARITY) {
        dprintf(PLATFORM_UART_DEBUG_LEVEL, "parity=no!\r\n");
    }
}

/******************************************************************************
 ** \brief Early init the uart hardware device.
 **
 ** \param [in]
 ** \param [out]
 *****************************************************************************/
void uart_init_early(void)
{
    hal_uart_cfg_t hal_cfg;
    uart_instance_t *instance;
#if 0
    /************ below will be change with the iomux hal ************/
    iomux_UART9_RX_1_ENABLE(0);
    iomux_UART9_TX_1_ENABLE(0);
    uint32_t addr, val;
    /*
     * Z1 only - disable pull of uart9 tx/rx.
     */
    addr = APB_IOMUXC_SEC_BASE + (0x10 << 10);
    val = readl(addr);
    val &= ~1ul;
    writel(val, addr);
    addr = APB_IOMUXC_SEC_BASE + (0x14 << 10);
    val = readl(addr);
    val &= ~1ul;
    writel(val, addr);
    /************ above will be change with the iomux hal ************/
#endif
    hal_uart_creat_handle(&uart_handle, DEBUG_COM);

    if (uart_handle != NULL) {
        instance = (uart_instance_t *)uart_handle;
        hal_cfg.port_cfg.sclk = UART_SCLK;
        hal_cfg.port_cfg.baud = UART_BAUD;
        hal_cfg.port_cfg.data_bits = UART_HAL_CHAR_8BITS;
        hal_cfg.port_cfg.stop_bits = UART_HAL_STOP_1BIT;
        hal_cfg.port_cfg.parity = UART_HAL_NO_PARITY;
        hal_cfg.port_cfg.loopback_enable = false;
        hal_cfg.fifo_cfg.fifo_enable = true;
        hal_cfg.fifo_cfg.rx_trigger = UART_HAL_RX_FIFO_CHAR_1;
        hal_cfg.fifo_cfg.tx_trigger = UART_HAL_TX_FIFO_EMPTY;
#ifdef UART_DRV_SUPPORT_RS485
        hal_cfg.rs485_cfg.rs485_enable = false;
#endif
#ifdef UART_DRV_SUPPORT_9BITS
        hal_cfg.nine_bits_cfg.nine_bits_enable = false;
#endif
#ifdef UART_DRV_SUPPORT_DMA
        hal_cfg.dma_cfg.dma_enable = false;
#endif
        hal_uart_init(uart_handle, &hal_cfg);
        uart_platform_cfg_printf(instance, &hal_cfg);
    }
}

/******************************************************************************
 ** \brief Put a character.
 **
 ** \param [in]
 ** \param [out]
 *****************************************************************************/
int uart_putc(int port, char data)
{
    hal_uart_putc(uart_handle, data);
    return 1;
}

/******************************************************************************
 ** \brief Early init the uart hardware device.
 **
 ** \param [in]
 ** \param [out]
 *****************************************************************************/
int uart_getc(int port, bool wait)
{
    cbuf_t *rxbuf = &uart_rx_buf;
    char c;

    if (!uart_handle)
        return -1;

    if (cbuf_read_char(rxbuf, &c, wait) == 1) {
        hal_uart_int_src_enable(uart_handle, UART_HAL_RX_CHAR_INT_SRC);
        return c;
    }

    return -1;
}

/* panic-time getc/putc */
int uart_pputc(int port, char c)
{
    return uart_putc(port, c);
}

int uart_pgetc(int port, bool wait)
{
    return uart_getc(port, wait);
}

void uart_flush_tx(int port)
{
}

void uart_flush_rx(int port)
{
}

void uart_init_port(int port, uint baud)
{
}

int uart_get_current_port(void)
{
    return 0;
}

void uart_exit(void)
{
    if (uart_handle) {
        hal_uart_irq_mask(uart_handle);
        hal_uart_release_handle(uart_handle);
        uart_handle = NULL;
    }
}
