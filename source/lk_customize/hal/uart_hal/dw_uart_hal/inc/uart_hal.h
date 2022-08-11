//*****************************************************************************
//
// uart_hal.h - Prototypes for the Watchdog hal
//
// Copyright (c) 2019-2029 Semidrive Incorporated.  All rights reserved.
// Software License Agreement
//
//
//*****************************************************************************

#ifndef __UART_HAL_H__
#define __UART_HAL_H__
//*****************************************************************************
//
// If building with a C++ compiler, make all of the definitions in this header
// have a C binding.
//
//*****************************************************************************
#ifdef __cplusplus
extern "C"
{
#endif

#include <__regs_base.h>
#include <kernel/mutex.h>
#include <platform/interrupts.h>
#include <res.h>
#include <chip_res.h>
#if ENABLE_DW_UART
#include "uart_drv.h"
#endif

#define SDV_UART_DRIVER_VERSION (MAKE_VERSION(1, 0, 0)) /*!< Version 1.0.0 */
#define MAKE_VERSION(major, minor, bugfix) (((major) << 16) | ((minor) << 8) | (bugfix))
#define DEFAULT_UART_MAX_NUM  16

// Check the arguments.
#define HAL_ASSERT_PARAMETER(handle)  \
if(handle == NULL){ \
    LTRACEF("paramenter error handle:%p\n", handle);    \
    return false;   \
}   \

/**
 *****************************************************************************
 ** \brief macro define
 *****************************************************************************/
#define UART_HAL_INVALID_IRQ_NUM    0xFFFFFFFF

/**
 *****************************************************************************
 ** \brief enum define
 *****************************************************************************/
typedef enum {
    UART_HAL_RES_ERR_OCCUPIED = 0,
    UART_HAL_RES_ERR_NOT_FIND,
    UART_HAL_RES_OK,
} hal_uart_err_t;

//data length select
typedef enum {
    UART_HAL_CHAR_5BITS = 0,
    UART_HAL_CHAR_6BITS,
    UART_HAL_CHAR_7BITS,
    UART_HAL_CHAR_8BITS,
} hal_uart_char_bits_t;

//stop bits
typedef enum {
    UART_HAL_STOP_1BIT = 0,
    UART_HAL_STOP_1_5BIT,
    UART_HAL_STOP_2BIT,
} hal_uart_stop_bits_t;

//parity bits
typedef enum {
    UART_HAL_ODD_PARITY = 0,
    UART_HAL_EVEN_PARITY,
    UART_HAL_NO_PARITY,
} hal_uart_parity_mode_t;

typedef enum {
    UART_HAL_FULL_DUPLEX_MODE = 0,
    UART_HAL_SW_HALF_DUPLEX_MODE,
    UART_HAL_HW_HALF_DUPLEX_MODE,
} hal_uart_rs485_transfer_mode_t;

typedef enum {
    UART_HAL_RX_FIFO_CHAR_1 = 0,    //0: 1 character in FIFO
    UART_HAL_RX_FIFO_QUARTER_FULL,  //1: FIFO 1/4 full
    UART_HAL_RX_FIFO_HALF_FULL,     //2: FIFO 1/2 full
    UART_HAL_RX_FIFO_FULL_2,        //3: FIFO 2 less than full
} hal_uart_fifo_rx_trigger_t;

typedef enum {
    UART_HAL_TX_FIFO_EMPTY = 0,     //0: FIFO Empty
    UART_HAL_TX_FIFO_CHA_2,         //1: 2 characters in FIFO
    UART_HAL_TX_FIFO_QUARTER_FULL,  //2: FIFO 1/4 full
    UART_HAL_TX_FIFO_HALF_FULL,     //3: FIFO 1/2 full
} hal_uart_fifo_tx_trigger_t;

typedef enum {
    UART_HAL_RX_CHAR_INT_SRC = 0,
    UART_HAL_RX_STR_INT_SRC,
    UART_HAL_TX_EMPTY_INT_SRC,

    UART_HAL_INT_SRC_TOTAL
} hal_uart_int_src_t;

/**
 *****************************************************************************
 ** \brief data type define
 *****************************************************************************/
typedef struct {
    uint32_t addr;
    uint32_t irq_num;
} hal_uart_addr_to_irq_t;

typedef struct {
    uint32_t sclk;
    uint32_t baud;
    hal_uart_char_bits_t data_bits;
    hal_uart_stop_bits_t stop_bits;
    hal_uart_parity_mode_t parity;
    bool loopback_enable;
} hal_uart_port_cfg_t;

typedef struct {
    bool nine_bits_enable;
    uint8_t rx_addr;
    uint8_t tx_addr;
    uint8_t rx_addr_match;
    uint8_t tx_mode_sw;
} hal_uart_9bits_cfg_t;

typedef struct {
    bool fifo_enable;
    hal_uart_fifo_rx_trigger_t rx_trigger;
    hal_uart_fifo_tx_trigger_t tx_trigger;
} hal_uart_fifo_cfg_t;

typedef struct {
    bool rs485_enable;
    //receiver enable polarity
    uint8_t re_polarity;
    //driver enable polarity
    uint8_t de_polarity;
    //driver aseert time: the time between the assertion of rising edge
    //of driver output enable signal to serial transmit enable
    uint8_t de_assert_timer;
    //driver de-assert time:the time between end of stop time
    //to the falling edge of driver output enable signal.
    uint8_t de_deassert_time;
    //driver enable to receive enable TurnAround time
    uint16_t de2re_turn_around_time;
    //receiver enable to driver enable TurnAround time
    uint16_t re2de_turn_around_time;
    //transfer mode
    hal_uart_rs485_transfer_mode_t transfer_mode;
} hal_uart_rs485_cfg_t;

typedef struct {
    bool dma_enable;
    uint32_t rx_burst;
    uint32_t tx_burst;
} hal_uart_dma_cfg_t;

/**
 *****************************************************************************
 ** \brief uart hal overall configuration.
 *****************************************************************************/
typedef struct {
    hal_uart_port_cfg_t port_cfg;
    hal_uart_fifo_cfg_t fifo_cfg;
#ifdef UART_DRV_SUPPORT_9BITS
    hal_uart_9bits_cfg_t nine_bits_cfg;
#endif
#ifdef UART_DRV_SUPPORT_RS485
    hal_uart_rs485_cfg_t rs485_cfg;
#endif
#ifdef UART_DRV_SUPPORT_DMA
    hal_uart_dma_cfg_t dma_cfg;
#endif
} hal_uart_cfg_t;

/**
 *****************************************************************************
 ** \brief uart instance information descriptor.
 *****************************************************************************/
typedef struct {
    bool occupied;
    uint32_t phy_num;
    uint32_t irq_num;   //uart irq number
#if ENABLE_DW_UART
    DW_APB_UART_uart_TypeDef *uartc;    //uart peripheral base address
    uart_drv_context_t drv_context;     //driver context
#endif
} uart_instance_t;

typedef int (*hal_uart_int_callback)(uint8_t data);

/**
 *****************************************************************************
 ** \brief uart function interface descriptor.
 *****************************************************************************/
bool hal_uart_creat_handle(void **handle, uint32_t res_glb_idx);
bool hal_uart_release_handle(void *handle);
void hal_uart_init(void *handle, hal_uart_cfg_t *cfg);
void hal_uart_putc(void *handle, char data);
void hal_uart_transmit(void *handle, char *buf, size_t size, bool async);
void hal_uart_receive(void *handle, char *buf, size_t size);
#ifdef UART_DRV_SUPPORT_9BITS
void hal_uart_9bits_putc(void *handle, char data, bool addr);
void hal_uart_9bits_transmit(void *handle, char addr, char *buf,
                             size_t size);
#endif
bool hal_uart_getc(void *handle, char *data);
void hal_uart_int_src_enable(void *handle, hal_uart_int_src_t int_src);
void hal_uart_int_src_disable(void *handle, hal_uart_int_src_t int_src);
void hal_uart_int_cbk_register(void *handle, hal_uart_int_src_t int_src,
                               hal_uart_int_callback cbk);
void hal_uart_baudrate_set(void *handle, uint32_t sclk, uint32_t baud);
void hal_uart_irq_mask(void *handle);
void hal_uart_irq_unmask(void *handle);

//*****************************************************************************
//
// Mark the end of the C bindings section for C++ compilers.
//
//*****************************************************************************
#ifdef __cplusplus
}
#endif
#endif // __WDG_IP_TEST_H__

