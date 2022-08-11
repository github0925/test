/*****************************************************************************
 ** uart_drv.c
 **
 ** Implementation of uart driver
 **
 ** History:
 **     2019-01-29  0.01  Qing chen     Initial version.
 **     2019-10-25  0.01  Yongjun wang  Restructure.
*****************************************************************************/
#ifndef __UART_DRV_H__
#define __UART_DRV_H__

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

#include <stdbool.h>
#include <sys/types.h>
#include <__regs_DW_apb_uart.h>
#include <lib/cbuf.h>
#include <kernel/timer.h>

/**
 *****************************************************************************
 ** \brief macro define
 *****************************************************************************/
//#define UART_DRV_SUPPORT_DMA
//#define UART_DRV_SUPPORT_RS485
//#define UART_DRV_SUPPORT_9BITS
//#define UART_DRV_SUPPORT_MODEM  //Not implement
//#define UART_DRV_SUPPORT_IrDA   //Not implement
//#define UART_DRV_SUPPORT_LIN    //Not implement

#ifdef UART_DRV_SUPPORT_DMA
#include <dma.h>
#endif

#define UART_DRV_IID_THR_EMPTY           0x02
#define UART_DRV_IID_RX_DATA_AVAILABLE   0x04
#define UART_DRV_IID_RX_LINE_STATUS      0x06
#define UART_DRV_IID_RX_CHAR_TIMEOUT     0x0C

#define UART_DRV_LSR_RX_DATA_NOT_READY   0
#define UART_DRV_LSR_RX_DATA_READY       1

#define UART_DRV_LSR_RX_ADDR_IND         1
#define UART_DRV_LSR_RX_DATA_IND         0

#define UART_DRV_DMA_TX_BUFF_LEN     (3*CACHE_LINE)  //DMA transmit source buffer
#define UART_DRV_DMA_RX_BUFF_LEN     (1*CACHE_LINE)  //DMA receive dest buffer

#define UART_DRV_TX_BUFF_LEN         1024   //UART tramsmit buffer length

#define UART_DRV_RX_FIFO_FULL_DEPTH       (16)
#define UART_DRV_RX_FIFO_QUARTER_DEPTH    (UART_DRV_RX_FIFO_FULL_DEPTH >> 2) //4 bytes
#define UART_DRV_RX_FIFO_HALF_DEPTH       (UART_DRV_RX_FIFO_FULL_DEPTH >> 1) //8 bytes
#define UART_DRV_RX_FIFO_CHAR_DEPTH       (1) //1 byte

/**
 *****************************************************************************
 ** \brief enum define
 *****************************************************************************/
typedef enum {
    UART_DRV_CHAR_5BITS = 0,
    UART_DRV_CHAR_6BITS,
    UART_DRV_CHAR_7BITS,
    UART_DRV_CHAR_8BITS,
} uart_drv_char_bits_t;

typedef enum {
    UART_DRV_STOP_1BIT = 0,
    UART_DRV_STOP_1_5BIT,
    UART_DRV_STOP_2BIT,
} uart_drv_stop_bits_t;

typedef enum {
    UART_DRV_ODD_PARITY = 0,
    UART_DRV_EVEN_PARITY,
    UART_DRV_NO_PARITY,
} uart_drv_parity_t;

typedef enum {
    //Transmit and receive can happen simultaneously.
    //User can enable DE_EN and RE_EN at any point of time.
    UART_DRV_RS485_FULL_DUPLEX_MODE = 0,
    //User software control the driver enable signal and receiver enable signal.
    //Program the DE_EN to 0, before program the RE_EN to 1.
    UART_DRV_RS485_SW_HALF_DUPLEX_MODE,
    //Hardware control the driver enable signal and receiver enable signal.
    //when pushed the data into the TX FIFO and if there is  no ongoing receive.
    //transfer, RE signal will de-assert, and DE signal will assert.
    UART_DRV_RS485_HW_HALF_DUPLEX_MODE,
} uart_drv_rs485_transfer_mode_t;

typedef enum {
    UART_DRV_RX_FIFO_CHAR_1 = 0,    //0: 1 character in FIFO
    UART_DRV_RX_FIFO_QUARTER_FULL,  //1: FIFO 1/4 full
    UART_DRV_RX_FIFO_HALF_FULL,     //2: FIFO 1/2 full
    UART_DRV_RX_FIFO_FULL_2,        //3: FIFO 2 less than full
} uart_drv_fifo_rx_trigger_t;

typedef enum {
    UART_DRV_TX_FIFO_EMPTY = 0,     //0: FIFO Empty
    UART_DRV_TX_FIFO_CHA_2,         //1: 2 characters in FIFO
    UART_DRV_TX_FIFO_QUARTER_FULL,  //2: FIFO 1/4 full
    UART_DRV_TX_FIFO_HALF_FULL,     //3: FIFO 1/2 full
} uart_drv_fifo_tx_trigger_t;

typedef enum {
    UART_DRV_RX_CHAR_INT_SRC = 0,
    UART_DRV_RX_STRING_INT_SRC,
    UART_DRV_TX_EMPTY_INT_SRC,

    UART_DRV_INT_SRC_TOTAL
} uart_drv_int_src_t;

/**
 *****************************************************************************
 ** \brief device initialize configuration structure define.
 *****************************************************************************/
typedef struct {
    uint32_t sclk;
    uint32_t baud;
    uart_drv_char_bits_t data_bits;
    uart_drv_stop_bits_t stop_bits;
    uart_drv_parity_t parity;
    bool loopback_enable;
} uart_drv_port_cfg_t;

typedef struct {
    bool nine_bits_enable;
    uint8_t rx_addr;
    uint8_t tx_addr;
    uint8_t rx_addr_match;
    uint8_t tx_mode_sw;
} uart_drv_9bits_cfg_t;

typedef struct {
    bool fifo_enable;
    uart_drv_fifo_rx_trigger_t rx_trigger;
    uart_drv_fifo_tx_trigger_t tx_trigger;
} uart_drv_fifo_cfg_t;

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
    uart_drv_rs485_transfer_mode_t transfer_mode;
} uart_drv_rs485_cfg_t;

typedef struct {
    bool dma_enable;
    uint32_t rx_burst;
    uint32_t tx_burst;
} uart_drv_dma_cfg_t;

typedef struct {
    uart_drv_port_cfg_t port_cfg;
    uart_drv_fifo_cfg_t fifo_cfg;
#ifdef UART_DRV_SUPPORT_9BITS
    uart_drv_9bits_cfg_t nine_bits_cfg;
#endif
#ifdef UART_DRV_SUPPORT_RS485
    uart_drv_rs485_cfg_t rs485_cfg;
#endif
#ifdef UART_DRV_SUPPORT_DMA
    uart_drv_dma_cfg_t dma_cfg;
#endif
} uart_drv_cfg_t;

#ifdef UART_DRV_SUPPORT_DMA
/**
 *****************************************************************************
 ** \brief device dma context structure define.
 *****************************************************************************/
typedef struct {
    bool dma_tx_inited;
    bool dma_rx_inited;

    struct dma_chan *tx_chan;
    struct dma_chan *rx_chan;

    uint8_t *rx_buff[2];
    uint8_t rx_index;
    uint32_t dma_rx_len;
    uint32_t dma_rx_burst;
    event_t rx_event;
    thread_t *rx_thread;

    cbuf_t tx_buff;
    uint32_t dma_tx_burst;
    event_t tx_event;
    timer_t txtimer;
    thread_t *tx_thread;
} uart_drv_dma_context;
#endif

typedef int (*uart_drv_int_callback)(uint8_t data);

typedef struct {
    char *tx_ptr;
    size_t len_rest;
} uart_drv_async_tx_shadow;

typedef struct {
    char *rx_ptr;
    size_t len_rest;
} uart_drv_async_rx_shadow;

/**
 *****************************************************************************
 ** \brief device context descriptor.
 *****************************************************************************/
typedef struct {
    bool fifo_enable;
    FCR_Type fcr_shadow;
#ifdef UART_DRV_SUPPORT_9BITS
    uart_drv_9bits_cfg_t nine_bits_cfg;
#endif
#ifdef UART_DRV_SUPPORT_RS485
    bool rs485_enable;
    uart_drv_rs485_transfer_mode_t rs485_transfer_mode;
#endif
#ifdef UART_DRV_SUPPORT_DMA
    bool dma_enable;
    uart_drv_dma_context dma_context;
#endif
    uart_drv_int_callback rx_char_cbk;
    uart_drv_int_callback rx_str_cbk;
    uart_drv_int_callback tx_str_cbk;
    uart_drv_async_tx_shadow tx_shadow;
    uart_drv_async_rx_shadow rx_shadow;
} uart_drv_context_t;


/**
 *****************************************************************************
 ** \brief device driver function interface.
 *****************************************************************************/
void uart_drv_init(DW_APB_UART_uart_TypeDef *dev,
                   uart_drv_context_t *context, uart_drv_cfg_t *cfg);
void uart_drv_putc(DW_APB_UART_uart_TypeDef *dev,
                   uart_drv_context_t *context, char data);
void uart_drv_transmit(DW_APB_UART_uart_TypeDef *dev,
                       uart_drv_context_t *context, char *buf, size_t size,
                       bool async);
void uart_drv_receive(DW_APB_UART_uart_TypeDef *dev,
                       uart_drv_context_t *context, char *buf, size_t size);
#ifdef UART_DRV_SUPPORT_9BITS
void uart_drv_9bits_putc(DW_APB_UART_uart_TypeDef *dev,
                         uart_drv_context_t *context, char data, bool addr);
void uart_drv_9bits_transmit(DW_APB_UART_uart_TypeDef *dev,
                             uart_drv_context_t *context, char addr, char *buf, size_t size);
#endif
bool uart_drv_getc(DW_APB_UART_uart_TypeDef *dev,
                   uart_drv_context_t *context, char *data);
void uart_drv_loopback(DW_APB_UART_uart_TypeDef *dev, bool enable);
enum handler_return uart_drv_irq_handle(DW_APB_UART_uart_TypeDef *dev,
                                        uart_drv_context_t *context);
void uart_drv_int_cbk_register(uart_drv_context_t *context,
                               uart_drv_int_src_t int_src,
                               uart_drv_int_callback cbk);
void uart_drv_int_src_enable(DW_APB_UART_uart_TypeDef *dev,
                             uart_drv_int_src_t int_src);
void uart_drv_int_src_disable(DW_APB_UART_uart_TypeDef *dev,
                              uart_drv_int_src_t int_src);
#ifdef UART_DRV_SUPPORT_DMA
bool uart_drv_dma_start_tx(uart_drv_context_t *context);
bool uart_drv_dma_start_rx(uart_drv_context_t *context);
#endif
#ifdef UART_DRV_SUPPORT_RS485
void uart_drv_rs485_driver_set(DW_APB_UART_uart_TypeDef *dev, bool enable);
void uart_drv_rs485_receiver_set(DW_APB_UART_uart_TypeDef *dev,
                                 bool enable);
#endif
void uart_drv_baudrate_set(DW_APB_UART_uart_TypeDef *dev, uint32_t sclk, uint32_t baud);


//*****************************************************************************
//
// Mark the end of the C bindings section for C++ compilers.
//
//*****************************************************************************
#ifdef __cplusplus
}
#endif
#endif

