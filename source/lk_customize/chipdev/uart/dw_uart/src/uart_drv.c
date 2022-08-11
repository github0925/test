
/*****************************************************************************
 ** uart_drv.c
 **
 ** Implementation of uart driver
 **
 ** History:
 **     2019-01-29  0.01  Qing chen     Initial version.
 **     2019-10-25  0.01  Yongjun wang  Restructure.
*****************************************************************************/
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

#include <platform/interrupts.h>
#include <platform/debug.h>
#include <assert.h>
#if defined(__GNUC__)
    #include <malloc.h>
#elif defined(__ICCARM__)
    #include"heap.h"
#else
    #error Unknown Compiler!
#endif
#include "uart_drv.h"
#include "reg.h"

#define UART_DRV_DEBUG_LEVEL ALWAYS

/******************************************************************************
 ** \brief Uart device common Initialize
 **
 ** \param [out] dev         Pointer to device controller base address
 ** \param [out] context     Context of the uart
 ** \param [in]  cfg         Uart configuration
 *****************************************************************************/
static void uart_drv_port_init(DW_APB_UART_uart_TypeDef *dev,
                               uart_drv_context_t *context, uart_drv_cfg_t *cfg)
{
    uint16_t divisor;
    uint16_t frac;

    divisor = cfg->port_cfg.sclk / ( 16 * cfg->port_cfg.baud);
    frac = ((cfg->port_cfg.sclk / ((cfg->port_cfg.baud * 16) / 1000)) - (divisor * 1000));
    frac = frac *16/1000;
    //baudrate set
    dev->LCR.DLAB = 1;  //divisor latch access bit
    dev->DLL.DLL = divisor & 0xff;
    dev->DLH.DLH = (divisor >> 8) & 0xff;
    dev->DLF.DLF = frac;
    dev->LCR.DLAB = 0;

    //data bits length
    switch (cfg->port_cfg.data_bits) {
        case UART_DRV_CHAR_5BITS:
        case UART_DRV_CHAR_6BITS:
        case UART_DRV_CHAR_7BITS:
        case UART_DRV_CHAR_8BITS:
            dev->LCR.DLS = cfg->port_cfg.data_bits;
            break;

        default:
            ASSERT(0); /* no other bits*/
            break;
    }

    //stop bits length
    switch (cfg->port_cfg.stop_bits) {
        case UART_DRV_STOP_1BIT:
            dev->LCR.STOP = 0;
            break;

        case UART_DRV_STOP_1_5BIT:
            if (dev->LCR.DLS != 0)
                ASSERT(0);

            dev->LCR.STOP = 1;
            break;

        case UART_DRV_STOP_2BIT:
            if (dev->LCR.DLS == 0)
                ASSERT(0);

            dev->LCR.STOP = 1;
            break;

        default:
            ASSERT(0);
            break;
    }

    //parity
    switch (cfg->port_cfg.parity) {
        case UART_DRV_NO_PARITY:
            dev->LCR.PEN = 0;
            break;

        case UART_DRV_ODD_PARITY:
            dev->LCR.EPS = 0;
            dev->LCR.PEN = 1;
            break;

        case UART_DRV_EVEN_PARITY:
            dev->LCR.EPS = 1;
            dev->LCR.PEN = 1;
            break;

        default:
            ASSERT(0);
            break;
    }

    if (cfg->port_cfg.loopback_enable) {
        uart_drv_loopback(dev, true);
    }
}

#ifdef UART_DRV_SUPPORT_RS485
/******************************************************************************
 ** \brief RS485 init
 **
 ** \param [in]  dev    Pointer to device information descriptor
 *****************************************************************************/
static void uart_drv_rs485_init(DW_APB_UART_uart_TypeDef *dev,
                                uart_drv_context_t *context, uart_drv_cfg_t *cfg)
{
    if (cfg->rs485_cfg.rs485_enable) {
        dev->TCR.RS485_EN = 1;
        dev->TCR.RE_POL = cfg->rs485_cfg.re_polarity;
        dev->TCR.DE_POL = cfg->rs485_cfg.de_polarity;
        dev->TCR.XFER_MODE = cfg->rs485_cfg.transfer_mode;
        dev->DET.DE_ASSERTION_TIME = cfg->rs485_cfg.de_assert_timer;
        dev->DET.DE_DEASSERTION_TIME = cfg->rs485_cfg.de_deassert_time;
        dev->TAT.DE_TO_RE = cfg->rs485_cfg.de2re_turn_around_time;
        dev->TAT.RE_TO_DE = cfg->rs485_cfg.re2de_turn_around_time;
        dev->RE_EN.RE_ENABLE = 1;
        //update the context
        context->rs485_enable = true;
        context->rs485_transfer_mode = cfg->rs485_cfg.transfer_mode;
    }
    else {
        dev->TCR.RS485_EN = 0;
        //update the context
        context->rs485_enable = false;
        context->rs485_transfer_mode = cfg->rs485_cfg.transfer_mode;
    }
}
#endif

#ifdef UART_DRV_SUPPORT_9BITS
/******************************************************************************
 ** \brief Uart device line extended control register Initialize
 **
 ** \param [in] dev         Pointer to device information descriptor
 *****************************************************************************/
static void uart_drv_9bits_init(DW_APB_UART_uart_TypeDef *dev,
                                uart_drv_context_t *context, uart_drv_cfg_t *cfg)
{
    if (cfg->nine_bits_cfg.nine_bits_enable) {
        //enable 9-bit data for transmit and receive transfers
        dev->LCR_EXT.DLS_E = 1;
        /*
        * ADDR_MATCH = 1 : Address match mode. uart will wait until the incoming character with 9-th
        *                  bit is set to 1. And further to check to see if the address matches with
        *                  what is programmed in 'Receive Address Match  Register'. If match, then
        *                  sub-sequent characters will be treated as valid data.
        * ADDR_MATCH = 0 : Normal mode. uart will start receive the data and 9-bit character will be
        *                  formed and written into the receive RXFIFO. User is responsible to read the
        *                  data and differentiable address and data.
        */
        dev->LCR_EXT.ADDR_MATCH = cfg->nine_bits_cfg.rx_addr_match;

        if (cfg->nine_bits_cfg.rx_addr_match) {
            dev->RAR.RAR = cfg->nine_bits_cfg.rx_addr;
        }

        /*
        * TRANSMIT_MODE = 1 : THR(Transmit holding register) and STHR(Shadow Transmit holding register)
        *                     are 9-bit wide. Address:9th bit is 1. Data:9th bit is 0.
        * TRANSMIT_MODE = 0 : THR(Transmit holding register) and STHR(Shadow Transmit holding register)
        *                     are 9-bit wide. User need to program the address into TAR(Transmit Address
        *                     Register). SEND_ADDR bit is used as a control knob to indicate the uart
        *                     when to send the address.
        */
        dev->LCR_EXT.TRANSMIT_MODE = cfg->nine_bits_cfg.tx_mode_sw;
    }
    else {
        dev->LCR_EXT.DLS_E = 0;
    }

    //update the context
    context->nine_bits_cfg = cfg->nine_bits_cfg;
}
#endif

/******************************************************************************
 ** \brief Uart device fifo control register Initialize
 **
 ** \param [in] dev         Pointer to device information descriptor
 *****************************************************************************/
static void uart_drv_fifo_init(DW_APB_UART_uart_TypeDef *dev,
                               uart_drv_context_t *context, uart_drv_cfg_t *cfg)
{
    if (cfg->fifo_cfg.fifo_enable) {
        context->fcr_shadow.RFIFOR = 1; //reset fifo
        context->fcr_shadow.XFIFOR = 1;
        dev->FCR.v = context->fcr_shadow.v;
        context->fcr_shadow.RFIFOR = 0;
        context->fcr_shadow.XFIFOR = 0;
        context->fcr_shadow.RT = cfg->fifo_cfg.rx_trigger;  //receive trigger
        context->fcr_shadow.TET =
            cfg->fifo_cfg.tx_trigger; //transmit empty trigger
#ifdef UART_DRV_SUPPORT_DMA

        if (cfg->dma_cfg.dma_enable) {
            context->fcr_shadow.DMAM = 1;
        }
        else {
            context->fcr_shadow.DMAM = 0;
        }

#else
        context->fcr_shadow.DMAM = 0;
#endif
        context->fcr_shadow.FIFOE = 1;
        dev->FCR.v = context->fcr_shadow.v;
        context->fifo_enable = true;
    }
    else {
#ifdef UART_DRV_SUPPORT_DMA

        if (cfg->dma_cfg.dma_enable) {
            context->fcr_shadow.DMAM = 0;
        }

#endif
        context->fcr_shadow.FIFOE = 0;
        dev->FCR.v = context->fcr_shadow.v;
        context->fifo_enable = false;
    }
}

/******************************************************************************
 ** \brief Uart device interrupt enable register Initialize
 **
 ** \param [in] dev         Pointer to device information descriptor
 *****************************************************************************/
static void uart_drv_interrupt_init(DW_APB_UART_uart_TypeDef *dev,
                                    uart_drv_cfg_t *cfg)
{
    dev->IER.ERBFI = 1;   //received data available interrupt
#ifdef UART_DRV_SUPPORT_DMA

    if (cfg->dma_cfg.dma_enable) {  //transmit holding register empty interrupt
        dev->IER.ETBEI = 1;
    }
    else
#endif
    {
        dev->IER.ETBEI = 0;
    }

    dev->IER.ELSI = 1;      //receiver line status interrupt
    dev->IER.EDSSI = 0;     //modem status interrupt
    dev->IER.PTIME = 0;     //programmable THRE interrupt mode
}

#ifdef UART_DRV_SUPPORT_DMA
/******************************************************************************
 ** \brief Uart device DMA configure
 **
 ** \param [in] dev         Pointer to device information descriptor
 *****************************************************************************/
static void uart_drv_dma_config(DW_APB_UART_uart_TypeDef *dev,
                                uart_drv_context_t *context, uart_drv_cfg_t *cfg)
{
    if (!cfg->dma_cfg.dma_enable) {
        context->dma_enable = false;
        return;
    }

    struct dma_dev_cfg dma_cfg;

    dma_cfg.direction = DMA_MEM2DEV;

    dma_cfg.src_addr  = (addr_t)(&(dev->RBR));

    dma_cfg.dst_addr  = (addr_t)(&(dev->THR));

    // This part should be bet by peri width, such as for I2S 16bits should set to 2 bytes.
    dma_cfg.src_addr_width = DMA_DEV_BUSWIDTH_1_BYTE;

    dma_cfg.dst_addr_width = DMA_DEV_BUSWIDTH_1_BYTE;

    dma_cfg.src_maxburst = cfg->dma_cfg.tx_burst;

    dma_cfg.dst_maxburst = cfg->dma_cfg.rx_burst;

    context->dma_context.dma_rx_burst = 1 * cfg->dma_cfg.rx_burst;

    context->dma_context.dma_tx_burst = 1 * cfg->dma_cfg.rx_burst;

    context->dma_context.dma_rx_len = DMA_DEV_BUSWIDTH_1_BYTE;

    //config tx channel for dev
    struct dma_chan *tx_chan = dma_chan_req();

    if (tx_chan) {
        dma_dev_config(tx_chan, &dma_cfg);
        context->dma_context.tx_chan = tx_chan;
    }
    else {
        context->dma_context.tx_chan = NULL;
        context->dma_context.dma_tx_inited = false;
    }

    //config rx channel
    struct dma_chan *rx_chan = dma_chan_req();
    dma_cfg.direction = DMA_DEV2MEM;

    if (rx_chan) {
        dma_dev_config(rx_chan, &dma_cfg);
        context->dma_context.rx_chan = rx_chan;
        context->dma_context.rx_index = 0;
    }
    else {
        context->dma_context.rx_chan = NULL;
        context->dma_context.dma_rx_inited = false;
    }

    context->dma_enable = true;
}

/******************************************************************************
 ** \brief DMA transmit timer one shot overflow callback.
 **
 ** \param [in] dev         Pointer to device information descriptor
 *****************************************************************************/
static enum handler_return uart_drv_dma_tx_timer_callback(timer_t *t,
        lk_time_t now, void *arg)
{
    uart_drv_context_t *context = (uart_drv_context_t *)arg;
    event_signal(&context->dma_context.tx_event, true);
    return INT_NO_RESCHEDULE;
}

/******************************************************************************
 ** \brief Start the DMA transmit timer for timeout.
 **
 ** \param [in] dev         Pointer to device information descriptor
 *****************************************************************************/
static void uart_drv_dma_start_tx_timer(uart_drv_context_t *context)
{
    timer_cancel(&context->dma_context.txtimer);
    timer_set_oneshot(&context->dma_context.txtimer, 1,
                      uart_drv_dma_tx_timer_callback, (void *)context);
}

/******************************************************************************
 ** \brief Add a char to DMA transmit cbuf_t tx_buff.
 **
 ** \param [in] dev         Pointer to device information descriptor
 *****************************************************************************/
static void uart_drv_dma_add_tx_char(uart_drv_context_t *context, char  c)
{
    do {
        if (cbuf_space_avail(&context->dma_context.tx_buff) > 0) {
            cbuf_write_char_nosignal(&context->dma_context.tx_buff, c);

            if (cbuf_space_used(&context->dma_context.tx_buff) > 0) {
                if (!thread_lock_held()) //
                    event_signal(&context->dma_context.tx_event, false);
                else
                    uart_drv_dma_start_tx_timer(context);
            }

            break;
        }
    } while (1);
}
#endif

static void uart_drv_context_init(uart_drv_context_t *context)
{
    context->tx_str_cbk = NULL;
    context->rx_char_cbk = NULL;
    context->rx_str_cbk = NULL;
    context->tx_shadow.tx_ptr = NULL;
    context->tx_shadow.len_rest = 0;
    context->rx_shadow.rx_ptr = NULL;
    context->rx_shadow.len_rest = 0;
}

/******************************************************************************
 ** \brief Uart overall init.
 **
 ** \param [in] dev         Pointer to device information descriptor
 *****************************************************************************/
void uart_drv_init(DW_APB_UART_uart_TypeDef *dev,
                   uart_drv_context_t *context, uart_drv_cfg_t *cfg)
{
    uart_drv_port_init(dev, context, cfg);
    uart_drv_fifo_init(dev, context, cfg);
#ifdef UART_DRV_SUPPORT_RS485
    uart_drv_rs485_init(dev, context, cfg);
#endif
#ifdef UART_DRV_SUPPORT_9BITS
    uart_drv_9bits_init(dev, context, cfg);
#endif
#ifdef UART_DRV_SUPPORT_DMA
    uart_drv_dma_config(dev, context, cfg);

    if (context->dma_enable) {
        uart_drv_dma_start_tx(context);
        uart_drv_dma_start_rx(context);
    }

#endif
    uart_drv_context_init(context);
    uart_drv_interrupt_init(dev, cfg);
}

#ifdef UART_DRV_SUPPORT_9BITS
/******************************************************************************
 ** \brief 9-bits character receive handle.
 **
 ** \param [in]     dev         Pointer to device information descriptor
 ** \param [out]
 *****************************************************************************/
static bool uart_drv_9bit_rx_handle(DW_APB_UART_uart_TypeDef *dev,
                                    uart_drv_context_t *context, char *data)
{
    uint32_t rbr = dev->RBR.RBR9;
    uint32_t isaddr = (rbr & (1 << 8)) >> 8;

    if (context->nine_bits_cfg.rx_addr_match) {
        //hw mode
        if (isaddr == 1) {
            //addr
            if ((rbr & 0xff) != context->nine_bits_cfg.rx_addr)
                ASSERT(0);

            return false;
        }
        else {
            *data = rbr & 0xff;
        }
    }
    else {
        //sw mode
        static char last_addr = 0;

        if (isaddr == 1) {
            //addr
            last_addr = rbr & 0xff;
            return false;
        }
        else if (last_addr == context->nine_bits_cfg.rx_addr) {
            //our data
            *data = rbr & 0xff;
        }
        else {
            //not our data
            return false;
        }
    }

    return true;
}
#endif

/******************************************************************************
 ** \brief uart transmit a byte with none 9bits mode.
 **
 ** \param [in]
 ** \param [out]
 *****************************************************************************/
void uart_drv_putc(DW_APB_UART_uart_TypeDef *dev,
                   uart_drv_context_t *context, char data)
{
#ifdef UART_DRV_SUPPORT_DMA

    if (context->dma_enable && context->dma_context.dma_tx_inited) {
        //mode1
        if (context->fcr_shadow.DMAM == 1) {
            uart_drv_dma_add_tx_char(context, data);
        }
        else {
            //if mode0
            //no implement
            ASSERT(0);
        }
    }
    else
#endif
    {
        if (context->fifo_enable) {
            while (dev->USR.TFNF != 1); //wait transmit fifo is not full
        }
        else {
            while (dev->LSR.THRE != 1); //wait transmit holding register empty
        }

        writeb(data, &dev->THR);
        //dev->THR.THR8 = data;
    }
}

#ifdef UART_DRV_SUPPORT_9BITS
/******************************************************************************
 ** \brief Uart transmit a byte with 9bits mode.
 **        Only for software transmit mode.
 **
 ** \param [in]
 ** \param [out]
 *****************************************************************************/
void uart_drv_9bits_putc(DW_APB_UART_uart_TypeDef *dev,
                         uart_drv_context_t *context, char data, bool addr)
{
    if (context->fifo_enable) {
        while (dev->USR.TFNF != 1); //wait transmit fifo is not full
    }
    else {
        while (dev->LSR.THRE != 1); //wait transmit holding register empty
    }

    if (addr) {
        dev->THR.THR9 = (data | addr << 8);
    }
    else {
        dev->THR.THR9 = data;
    }
}

/******************************************************************************
 ** \brief Uart transmit with 9-bits mode.
 **
 ** \param [in]
 ** \param [out]
 *****************************************************************************/
void uart_drv_9bits_transmit(DW_APB_UART_uart_TypeDef *dev,
                             uart_drv_context_t *context, char addr, char *buf, size_t size)
{
    size_t i;

    if (!context->nine_bits_cfg.nine_bits_enable) {
        ASSERT(0);
    }

    if (context->nine_bits_cfg.tx_mode_sw) {
        uart_drv_9bits_putc(dev, context, addr, true); //addr

        for (i = 0; i < size; i++) { //data
            uart_drv_9bits_putc(dev, context, buf[i], false);
        }
    }
    else {
        dev->TAR.TAR = addr;    //addr
        dev->LCR_EXT.SEND_ADDR = 1;

        for (i = 0; i < size; i++) { //data
            uart_drv_putc(dev, context, buf[i]);
        }
    }
}
#endif

/******************************************************************************
 ** \brief uart transmit with none 9bits mode.
 **
 ** \param [in]  dev        uart controller base address
 ** \param [in]  context    uart context
 ** \param [in]  buf        uart send buffer start address
 ** \param [in]  size       uart send buffer length
 ** \param [in]  async      if true, uart asynchronous transmit, after transmit
 **                         success, call the transmit callback function registered.
 **                         if false, uart synchronous transmit.
 *****************************************************************************/
void uart_drv_transmit(DW_APB_UART_uart_TypeDef *dev,
                       uart_drv_context_t *context, char *buf, size_t size,
                       bool async)
{
    size_t i;

    if (size == 0) {
        return;
    }

    if (async) {
        context->tx_shadow.tx_ptr = buf;    //save the context to shadow
        context->tx_shadow.len_rest = size;

        if (context->fifo_enable) {
            //until tx fifo is full or len_rest is zero
            while (context->tx_shadow.len_rest && dev->USR.TFNF) {
                //dev->THR.THR8 = *context->tx_shadow.tx_ptr++;
                writeb(*context->tx_shadow.tx_ptr++, &dev->THR);
                context->tx_shadow.len_rest -= 1;
            }
        }
        else {
            if (dev->LSR.THRE) {  //Transmit Holding Register Empty
                //dev->THR.THR8 = *context->tx_shadow.tx_ptr++;
                writeb(*context->tx_shadow.tx_ptr++, &dev->THR);
                context->tx_shadow.len_rest -= 1;
            }
        }

        uart_drv_int_src_enable(dev, UART_DRV_TX_EMPTY_INT_SRC);
    }
    else {
        for (i = 0; i < size; i++) { //data
            uart_drv_putc(dev, context, buf[i]);
        }
    }
}

static void uart_drv_rx_fifo_trig_update(DW_APB_UART_uart_TypeDef *dev,
        uart_drv_context_t *context)
{
    if (context->rx_shadow.len_rest >= UART_DRV_RX_FIFO_HALF_DEPTH) {
        context->fcr_shadow.RT = UART_DRV_RX_FIFO_HALF_FULL;
        dev->FCR.v = context->fcr_shadow.v;
    }
    else if (context->rx_shadow.len_rest >= UART_DRV_RX_FIFO_QUARTER_DEPTH) {
        context->fcr_shadow.RT = UART_DRV_RX_FIFO_QUARTER_FULL;
        dev->FCR.v = context->fcr_shadow.v;
    }
    else {
        context->fcr_shadow.RT = UART_DRV_RX_FIFO_CHAR_1;
        dev->FCR.v = context->fcr_shadow.v;
    }
}

/******************************************************************************
 ** \brief uart asynchronous receive with none 9bits mode.
 **
 ** \param [in]  dev        uart controller base address
 ** \param [in]  context    uart context
 ** \param [in]  buf        uart asynchronous receive buffer start address
 ** \param [in]  size       uart asynchronous receive length
 **                         after received size of bytes,
 **                         call the receive callback function registered.
 *****************************************************************************/
void uart_drv_receive(DW_APB_UART_uart_TypeDef *dev,
                      uart_drv_context_t *context, char *buf, size_t size)
{
    if (size == 0) {
        return;
    }

    context->rx_shadow.rx_ptr = buf;    //save the context to shadow
    context->rx_shadow.len_rest = size;
    //uart_drv_rx_fifo_trig_update(dev, context);
    uart_drv_int_src_enable(dev, UART_DRV_RX_CHAR_INT_SRC);
    //printf("%s size: %d, fcr_shadow.RT: %d\n", __func__, size, context->fcr_shadow.RT);
}

/******************************************************************************
 ** \brief Uart get a character.
 **
 ** \param [in]
 ** \param [out]
 *****************************************************************************/
bool uart_drv_getc(DW_APB_UART_uart_TypeDef *dev,
                   uart_drv_context_t *context, char *data)
{
    bool ret = false;

    if (dev->LSR.DR == 1) { //receive data ready
        if (context->fifo_enable) {
            /* while fifo is not empty, read chars out of it */
            if (dev->USR.RFNE) {
                *data = dev->RBR.RBR8;
                ret = true;
            }
        }
        else {
            *data = dev->RBR.RBR8;
            ret = true;
        }
    }
    else {
        //not ready?
        ret = false;
    }

    return ret;
}

/******************************************************************************
 ** \brief Enable or disable loopback mode.
 **
 ** \param [in]
 ** \param [out]
 *****************************************************************************/
void uart_drv_loopback(DW_APB_UART_uart_TypeDef *dev, bool enable)
{
    if (enable) {
        dev->MCR.LOOPBACK = 1;
    }
    else {
        dev->MCR.LOOPBACK = 0;
    }
}

/******************************************************************************
 ** \brief Register the receive callback function.
 **
 ** \param [in]
 ** \param [out]
 *****************************************************************************/
void uart_drv_int_cbk_register(uart_drv_context_t *context,
                               uart_drv_int_src_t int_src,
                               uart_drv_int_callback cbk)

{
    if (int_src == UART_DRV_RX_CHAR_INT_SRC) {
        context->rx_char_cbk = cbk;
    }
    else if (int_src == UART_DRV_RX_STRING_INT_SRC) {
        context->rx_str_cbk = cbk;
    }
    else if (int_src == UART_DRV_TX_EMPTY_INT_SRC) {
        context->tx_str_cbk = cbk;
    }
}

void uart_drv_int_src_enable(DW_APB_UART_uart_TypeDef *dev,
                             uart_drv_int_src_t int_src)
{
    if (int_src == UART_DRV_RX_CHAR_INT_SRC) {
        dev->IER.ERBFI = 1;
    }
    else if (int_src == UART_DRV_RX_STRING_INT_SRC) {
        dev->IER.ERBFI = 1;
    }
    else if (int_src == UART_DRV_TX_EMPTY_INT_SRC) {
        dev->IER.ETBEI = 1;
    }
}

void uart_drv_int_src_disable(DW_APB_UART_uart_TypeDef *dev,
                              uart_drv_int_src_t int_src)
{
    if (int_src == UART_DRV_RX_CHAR_INT_SRC) {
        dev->IER.ERBFI = 0;
    }
    else if (int_src == UART_DRV_RX_STRING_INT_SRC) {
        dev->IER.ERBFI = 0;
    }
    else if (int_src == UART_DRV_TX_EMPTY_INT_SRC) {
        dev->IER.ETBEI = 0;
    }
}

static void uart_drv_async_rx_char(DW_APB_UART_uart_TypeDef *dev,
                                   uart_drv_context_t *context, char data)
{
    *context->rx_shadow.rx_ptr = data;
    context->rx_shadow.rx_ptr++;
    context->rx_shadow.len_rest -= 1;

    if (context->rx_shadow.len_rest == 0) {
        uart_drv_int_src_disable(dev, UART_DRV_RX_CHAR_INT_SRC);
        context->fcr_shadow.TET = UART_DRV_RX_FIFO_CHAR_1;
        dev->FCR.v = context->fcr_shadow.v;
        context->rx_shadow.rx_ptr = NULL;
        context->rx_str_cbk(data);
    }
}

/******************************************************************************
 ** \brief Uart interrupt service function.
 **
 ** \param [in]
 ** \param [out]
 *****************************************************************************/
enum handler_return uart_drv_irq_handle(DW_APB_UART_uart_TypeDef *dev,
                                        uart_drv_context_t *context)
{
    bool resched = true;
    char data = 0;
    IIR_Type IIR = dev->IIR;        //interrupt identification register
    LSR_Type LSR = dev->LSR;        //line status register

    if (IIR.IID == UART_DRV_IID_RX_DATA_AVAILABLE
            || IIR.IID == UART_DRV_IID_RX_LINE_STATUS) {
        //rx irq
        if (LSR.DR == UART_DRV_LSR_RX_DATA_READY
                || LSR.ADDR_RCVD == UART_DRV_LSR_RX_ADDR_IND) {
            if (context->fifo_enable) {
                /* while fifo is not empty, read chars out of it */
                while (dev->USR.RFNE) {
#ifdef UART_DRV_SUPPORT_9BITS

                    if (context->nine_bits_cfg.nine_bits_enable) {
                        //ninebits
                        if (!uart_drv_9bit_rx_handle(dev, context, &data)) {
                            continue;
                        }
                    }
                    else
#endif
                    {
                        data = dev->RBR.RBR8;
                    }

                    //for asynchronous receive
                    if ((context->rx_str_cbk) && (context->rx_shadow.len_rest)) {
                        uart_drv_async_rx_char(dev, context, data);
                    }
                    else if ((context->rx_char_cbk == NULL)
                             || (context->rx_char_cbk(data) != 1)) {
                        //save not success
                        resched = false;
                        break;
                    }
                }

                if ((context->rx_str_cbk) && (context->rx_shadow.len_rest)) {
                    //uart_drv_rx_fifo_trig_update(dev, context);
                    resched = false;
                }
            }
            else {
#ifdef UART_DRV_SUPPORT_9BITS

                if (context->nine_bits_cfg.nine_bits_enable) {
                    //ninebits
                    if (!uart_drv_9bit_rx_handle(dev, context, &data)) {
                        return INT_RESCHEDULE;
                    }
                }
                else
#endif
                {
                    data = dev->RBR.RBR8;
                }

                //asynchronous receive
                if ((context->rx_str_cbk) && (context->rx_shadow.len_rest)) {
                    uart_drv_async_rx_char(dev, context, data);

                    if (context->rx_shadow.len_rest) {
                        resched = false;
                    }
                }
                else if ((context->rx_char_cbk == NULL)
                         || (context->rx_char_cbk(data) != 1)) {
                    resched = false;
                }
            }
        }
        else {
            //not ready?
        }
    }
    else if (IIR.IID == UART_DRV_IID_RX_CHAR_TIMEOUT) {
        //read receive buffer will clear the irq(character timeout)
        data = dev->RBR.RBR8;
    }
    else if (IIR.IID == UART_DRV_IID_THR_EMPTY) {
        //for asynchronous transmit
        if (context->fifo_enable) {
            //until tx fifo is full or len_rest is zero
            while (context->tx_shadow.len_rest && dev->USR.TFNF) {
                //dev->THR.THR8 = *context->tx_shadow.tx_ptr++;
                writeb(*context->tx_shadow.tx_ptr++, &dev->THR);
                context->tx_shadow.len_rest -= 1;
            }
        }
        else {
            //Transmit Holding Register Empty
            if (context->tx_shadow.len_rest && dev->LSR.THRE) {
                //dev->THR.THR8 = *context->tx_shadow.tx_ptr++;
                writeb(*context->tx_shadow.tx_ptr++, &dev->THR);
                context->tx_shadow.len_rest -= 1;
            }
        }

        if (context->tx_shadow.len_rest == 0) {
            uart_drv_int_src_disable(dev, UART_DRV_TX_EMPTY_INT_SRC);

            if (context->tx_str_cbk) {
                context->tx_str_cbk(true);
            }
        }
    }

    return resched ? INT_RESCHEDULE : INT_NO_RESCHEDULE;
}

#ifdef UART_DRV_SUPPORT_DMA
/******************************************************************************
 ** \brief DMA transmit complete interrupt callback.
 **
 ** \param [in]
 ** \param [out]
 *****************************************************************************/
static void uart_drv_dma_tx_irq_evt_handle(enum dma_status status,
        uint32_t err, void *ctx)
{
    dprintf(UART_DRV_DEBUG_LEVEL, "uart dma tx status %d (0x%x)\n", status,
            err);
    uart_drv_context_t *context = (uart_drv_context_t *)ctx;
    event_signal(&context->dma_context.tx_event, false);
}

/******************************************************************************
 ** \brief DMA receive complete interrupt callback.
 **
 ** \param [in]
 ** \param [out]
 *****************************************************************************/
static void uart_drv_dma_rx_irq_evt_handle(enum dma_status status,
        uint32_t err, void *ctx)
{
    uart_drv_context_t *context = (uart_drv_context_t *)ctx;
    int r_index = context->dma_context.rx_index;
    context->dma_context.rx_index = (r_index + 1) % 2;
    dprintf(UART_DRV_DEBUG_LEVEL, "uart dma rx status %d (0x%x)\n", status,
            err);
    arch_invalidate_cache_range((addr_t)context->dma_context.rx_buff[r_index],
                                context->dma_context.dma_rx_len / context->dma_context.dma_rx_burst);

    for (uint32_t i = 0;
            i < (context->dma_context.dma_rx_len / context->dma_context.dma_rx_burst);
            i++) {
        context->recv_cbk(context->dma_context.rx_buff[r_index][i]);
    }

    event_signal(&context->dma_context.rx_event, false);
}

/******************************************************************************
 ** \brief DMA receive thread entry.
 **
 ** \param [in]
 ** \param [out]
 *****************************************************************************/
static int uart_drv_rx_thread(void *arg)
{
    uart_drv_context_t *context = (uart_drv_context_t *)arg;

    for (;;) {
        static struct dma_desc *desc_rx = NULL;
        event_wait(&context->dma_context.rx_event);

        if (desc_rx) {
            dma_free_desc(desc_rx);
        }

        desc_rx = dw_prep_dma_dev(context->dma_context.rx_chan,
                                  (addr_t)context->dma_context.rx_buff[context->dma_context.rx_index],
                                  context->dma_context.dma_rx_len,
                                  DMA_INTERRUPT);
        // setup call back function.
        desc_rx->dmac_irq_evt_handle = uart_drv_dma_rx_irq_evt_handle;
        desc_rx->context = (void *)context;
        dma_submit(desc_rx);
    }

    return 0;
}

/******************************************************************************
 ** \brief DMA transmit thread entry.
 **
 ** \param [in]
 ** \param [out]
 *****************************************************************************/
static int uart_drv_tx_thread(void *arg)
{
    static char buff[UART_DRV_DMA_TX_BUFF_LEN] __ALIGNED(CACHE_LINE);
    uart_drv_context_t *context = (uart_drv_context_t *)arg;

    for (;;) {
        int used = 0;
        event_wait(&context->dma_context.tx_event);
        //check if cbuf has useful character
        used = cbuf_space_used(&context->dma_context.tx_buff);

        if ((used > 0)
                && (dma_get_chan_status(context->dma_context.tx_chan) == DMA_COMP)) {
            static struct dma_desc *desc_tx = NULL;
            int len = 0;    //(used > DMA_BUFF_LEN ? DMA_BUFF_LEN : used);
            //read dev->dmac_context.tx_buff to buff
            len = cbuf_read(&context->dma_context.tx_buff, buff,
                            UART_DRV_DMA_TX_BUFF_LEN, false);

            if (len == 0)
                continue;

            if (desc_tx)
                dma_free_desc(desc_tx);

            desc_tx = dw_prep_dma_dev(context->dma_context.tx_chan,
                                      (addr_t)buff,
                                      len * context->dma_context.dma_tx_burst,
                                      DMA_INTERRUPT);
            // setup call back function. and need set DMA_INTERRUPT flag.
            desc_tx->dmac_irq_evt_handle = uart_drv_dma_tx_irq_evt_handle;
            desc_tx->context = (void *)context;
            arch_clean_cache_range((addr_t)buff,
                                   len * context->dma_context.dma_tx_burst);
            dma_submit(desc_tx);
            //dma_sync_wait(desc_tx, 10);
        }

        //else
        //  spin(1);
    }

    return 0;
}

/******************************************************************************
 ** \brief Create the DMA receive thread, init the cbuf, timer, event used.
 **
 ** \param [in]
 ** \param [out]
 *****************************************************************************/
bool uart_drv_dma_start_rx(uart_drv_context_t *context)
{
    if (context->dma_context.rx_chan == NULL) {
        return false;
    }

    event_init(&context->dma_context.rx_event, false, EVENT_FLAG_AUTOUNSIGNAL);

    for (int i = 0; i < 2; i++)
        context->dma_context.rx_buff[i] = memalign(CACHE_LINE,
                                          UART_DRV_DMA_RX_BUFF_LEN);

    context->dma_context.rx_thread = thread_create("uart dma rx thread",
                                     uart_drv_rx_thread,
                                     (void *)context,
                                     HIGH_PRIORITY,
                                     DEFAULT_STACK_SIZE);
    thread_resume(context->dma_context.rx_thread);
    event_signal(&context->dma_context.rx_event, false);
    context->dma_context.dma_rx_inited = true;
    return true;
}

/******************************************************************************
 ** \brief Create the DMA transmit thread, init the cbuf, timer, event used.
 **
 ** \param [in]
 ** \param [out]
 *****************************************************************************/
bool uart_drv_dma_start_tx(uart_drv_context_t *context)
{
    if (context->dma_context.tx_chan == NULL) {
        printf("dma tx chan request failed\n");
        return false;
    }

    cbuf_initialize(&context->dma_context.tx_buff, UART_DRV_TX_BUFF_LEN);
    event_init(&context->dma_context.tx_event, false, EVENT_FLAG_AUTOUNSIGNAL);
    timer_initialize(&context->dma_context.txtimer);
    context->dma_context.tx_thread = thread_create("uart dma tx thread",
                                     uart_drv_tx_thread,
                                     (void *)context,
                                     HIGH_PRIORITY,
                                     DEFAULT_STACK_SIZE);
    thread_resume(context->dma_context.tx_thread);
    context->dma_context.dma_tx_inited = true;
    return true;
}
#endif

#ifdef UART_DRV_SUPPORT_RS485
/******************************************************************************
 ** \brief RS485 driver output enable signal enable/disable.
 **
 ** \param [in]
 ** \param [out]
 *****************************************************************************/
void uart_drv_rs485_driver_set(DW_APB_UART_uart_TypeDef *dev, bool enable)
{
    if (enable) {
        dev->DE_EN.DE_ENABLE = 1;
    }
    else {
        dev->DE_EN.DE_ENABLE = 0;
    }
}

/******************************************************************************
 ** \brief RS485 receiver input enable signal enable/disable.
 **
 ** \param [in]
 ** \param [out]
 *****************************************************************************/
void uart_drv_rs485_receiver_set(DW_APB_UART_uart_TypeDef *dev,
                                 bool enable)
{
    if (enable) {
        dev->RE_EN.RE_ENABLE = 1;
    }
    else {
        dev->RE_EN.RE_ENABLE = 0;
    }
}
#endif

void uart_drv_baudrate_set(DW_APB_UART_uart_TypeDef *dev, uint32_t sclk,
                           uint32_t baud)
{
    uint16_t divisor;
    uint16_t frac;

    divisor = sclk / (16 * baud);
    frac = ((sclk / ((baud * 16) / 1000)) - (divisor * 1000));
    frac = frac *16/1000;

    //baudrate set
    dev->LCR.DLAB = 1;  //divisor latch access bit
    dev->DLL.DLL = divisor & 0xff;
    dev->DLH.DLH = (divisor >> 8) & 0xff;
    dev->DLF.DLF = frac;
    dev->LCR.DLAB = 0;
}

//*****************************************************************************
//
// Mark the end of the C bindings section for C++ compilers.
//
//*****************************************************************************
#ifdef __cplusplus
}
#endif

