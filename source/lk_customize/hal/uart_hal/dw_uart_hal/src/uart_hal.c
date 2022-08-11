/*
* uart_hal.c
*
* Copyright (c) 2018 Semidrive Semiconductor.
* All rights reserved.
*
* Description: implement semidrive timer HAL
*
* Revision History:
* -----------------
* 011, 11/23/2019 wang yongjun implement this
*/

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
#include <sys/types.h>
#include <platform/debug.h>
#include <trace.h>
#include <string.h>
#include <assert.h>
#include <platform.h>
#include <kernel/thread.h>
#include <kernel/vm.h>
#include <kernel/spinlock.h>
#include <irq.h>
#include "uart_drv.h"
#include "uart_hal.h"

/*uart global instance*/
uart_instance_t g_uart_instance[DEFAULT_UART_MAX_NUM] = {0};
spin_lock_t uart_spin_lock = SPIN_LOCK_INITIAL_VALUE;

static const hal_uart_addr_to_irq_t
uart_addr2irq_table[DEFAULT_UART_MAX_NUM] = {
    { APB_UART1_BASE, UART1_INTR_NUM },
#ifndef RES_EPU
    { APB_UART2_BASE, UART2_INTR_NUM },
    { APB_UART3_BASE, UART3_INTR_NUM },
    { APB_UART4_BASE, UART4_INTR_NUM },
    { APB_UART5_BASE, UART5_INTR_NUM },
    { APB_UART6_BASE, UART6_INTR_NUM },
    { APB_UART7_BASE, UART7_INTR_NUM },
    { APB_UART8_BASE, UART8_INTR_NUM },
    { APB_UART9_BASE, UART9_INTR_NUM },
    { APB_UART10_BASE, UART10_INTR_NUM },
    { APB_UART11_BASE, UART11_INTR_NUM },
    { APB_UART12_BASE, UART12_INTR_NUM },
    { APB_UART13_BASE, UART13_INTR_NUM },
    { APB_UART14_BASE, UART14_INTR_NUM },
    { APB_UART15_BASE, UART15_INTR_NUM },
    { APB_UART16_BASE, UART16_INTR_NUM },
#endif
};

/******************************************************************************
 ** \brief Translate the uart controller base address to irq number
 **
 ** \param [in]
 *****************************************************************************/
static bool hal_uart_addr_to_phy(uint32_t addr, int32_t *phy_num,
                                 uint32_t *irq_num)
{
    uint32_t i;

    for (i = 0; i < DEFAULT_UART_MAX_NUM; i++) {
        if (uart_addr2irq_table[i].addr == addr) {
            *irq_num = uart_addr2irq_table[i].irq_num;
            *phy_num = i + 1;
            return true;
        }
    }

    return false;
}

/******************************************************************************
 ** \brief Get the instance of uart
 **
 ** \param [in]   res_glb_idx      global resource index
 ** \param [out]  instance         pointer of the instance get
 ** \return       uart_hal_err_t   result of get instance
 *****************************************************************************/
static hal_uart_err_t hal_uart_get_instance(uint32_t res_glb_idx,
        uart_instance_t **instance)
{
    uint32_t index = 0;
    addr_t phy_addr;
    int32_t phy_num;
    uint32_t irq_num = UART_HAL_INVALID_IRQ_NUM;
    hal_uart_err_t ret_value = UART_HAL_RES_ERR_NOT_FIND;
    spin_lock_saved_state_t states;
    spin_lock_irqsave(&uart_spin_lock, states);
    //find the resource info index according of resource global index
    *instance = NULL;

    if (!(res_get_info_by_id(res_glb_idx, &phy_addr, &phy_num) < 0)) {
        for (index = 0; index < DEFAULT_UART_MAX_NUM; index++) {
            if (!g_uart_instance[index].occupied
                    && (hal_uart_addr_to_phy(phy_addr, &phy_num, &irq_num))) {
                g_uart_instance[index].occupied = true;
#if WITH_KERNEL_VM
                g_uart_instance[index].uartc = (DW_APB_UART_uart_TypeDef *)((
                                                   uint64_t)paddr_to_kvaddr((addr_t)phy_addr));
#else
                g_uart_instance[index].uartc = (DW_APB_UART_uart_TypeDef *)(phy_addr);
#endif
                g_uart_instance[index].irq_num = irq_num;
                g_uart_instance[index].phy_num = phy_num;
                ret_value = UART_HAL_RES_OK;
                *instance = &g_uart_instance[index];
                spin_unlock_irqrestore(&uart_spin_lock, states);
                return ret_value;
            }
            else {
                continue;
            }
        }

        ret_value = UART_HAL_RES_ERR_OCCUPIED;
    }
    else {
        ret_value = UART_HAL_RES_ERR_NOT_FIND;
    }

    spin_unlock_irqrestore(&uart_spin_lock, states);
    return ret_value;
}

/******************************************************************************
 ** \brief Release the instance of uart
 **
 ** \param [out]  instance         pointer of the instance get
 *****************************************************************************/
static void hal_uart_release_instance(uart_instance_t *instance)
{
    instance->occupied = false;
}

/******************************************************************************
 ** \brief Create the handle of uart
 **
 ** \param [in]   res_glb_idx      global resource index
 ** \param [out]  handle           pointer of the handle create
 ** \return       bool             result of get instance
 *****************************************************************************/
bool hal_uart_creat_handle(void **handle, uint32_t res_glb_idx)
{
    uart_instance_t *instance = NULL;

    if (hal_uart_get_instance(res_glb_idx, &instance) != UART_HAL_RES_OK) {
        *handle = NULL;
        return false;
    }

    *handle = instance;
    return true;
}

/******************************************************************************
 ** \brief Release the handle of uart
 **
 ** \param [out]  handle           pointer of the handle create
 ** \return       bool             result of get instance
 *****************************************************************************/
bool hal_uart_release_handle(void *handle)
{
    uart_instance_t *instance = (uart_instance_t *)handle;
    spin_lock_saved_state_t states;
    spin_lock_irqsave(&uart_spin_lock, states);
    instance->occupied = false;
    spin_unlock_irqrestore(&uart_spin_lock, states);
    return true;
}

/******************************************************************************
 ** \brief Uart IRQ handle
 **
 ** \param [out]  arg               pointer of the handle create
 ** \return       handler_return
 *****************************************************************************/
static enum handler_return hal_uart_irq_handle(void *arg)
{
    uart_instance_t *instance = (uart_instance_t *)arg;

    if (instance == NULL)
        return INT_NO_RESCHEDULE;

    return uart_drv_irq_handle(instance->uartc, &(instance->drv_context));
}

/******************************************************************************
 ** \brief HAL confuration copy to driver confuration
 **
 ** \param [in]   hal_cfg               pointer of the hal confuration
 ** \param [out]  drv_cfg               pointer of the driver confuration
 *****************************************************************************/
static void hal_uart_cfg_copy_to_drv(hal_uart_cfg_t *hal_cfg,
                                     uart_drv_cfg_t *drv_cfg)
{
    if ((hal_cfg == NULL) || (drv_cfg == NULL))
        return;

    drv_cfg->port_cfg.sclk = hal_cfg->port_cfg.sclk;
    drv_cfg->port_cfg.baud = hal_cfg->port_cfg.baud;
    drv_cfg->port_cfg.data_bits = hal_cfg->port_cfg.data_bits;
    drv_cfg->port_cfg.stop_bits = hal_cfg->port_cfg.stop_bits;
    drv_cfg->port_cfg.parity = hal_cfg->port_cfg.parity;
    drv_cfg->port_cfg.loopback_enable = hal_cfg->port_cfg.loopback_enable;
    drv_cfg->fifo_cfg.fifo_enable = hal_cfg->fifo_cfg.fifo_enable;
    drv_cfg->fifo_cfg.rx_trigger = hal_cfg->fifo_cfg.rx_trigger;
    drv_cfg->fifo_cfg.tx_trigger = hal_cfg->fifo_cfg.tx_trigger;
#ifdef UART_DRV_SUPPORT_9BITS
    drv_cfg->nine_bits_cfg.nine_bits_enable =
        hal_cfg->nine_bits_cfg.nine_bits_enable;
    drv_cfg->nine_bits_cfg.rx_addr = hal_cfg->nine_bits_cfg.rx_addr;
    drv_cfg->nine_bits_cfg.tx_addr = hal_cfg->nine_bits_cfg.tx_addr;
    drv_cfg->nine_bits_cfg.rx_addr_match =
        hal_cfg->nine_bits_cfg.rx_addr_match;
    drv_cfg->nine_bits_cfg.tx_mode_sw = hal_cfg->nine_bits_cfg.tx_mode_sw;
#endif
#ifdef UART_DRV_SUPPORT_RS485
    drv_cfg->rs485_cfg.rs485_enable = hal_cfg->rs485_cfg.rs485_enable;
    drv_cfg->rs485_cfg.re_polarity = hal_cfg->rs485_cfg.re_polarity;
    drv_cfg->rs485_cfg.de_polarity = hal_cfg->rs485_cfg.de_polarity;
    drv_cfg->rs485_cfg.de_assert_timer = hal_cfg->rs485_cfg.de_assert_timer;
    drv_cfg->rs485_cfg.de_deassert_time = hal_cfg->rs485_cfg.de_deassert_time;
    drv_cfg->rs485_cfg.de2re_turn_around_time =
        hal_cfg->rs485_cfg.de2re_turn_around_time;
    drv_cfg->rs485_cfg.re2de_turn_around_time =
        hal_cfg->rs485_cfg.re2de_turn_around_time;
    drv_cfg->rs485_cfg.transfer_mode = hal_cfg->rs485_cfg.transfer_mode;
#endif
#ifdef UART_DRV_SUPPORT_DMA
    drv_cfg->dma_cfg.dma_enable = hal_cfg->dma_cfg.dma_enable;
    drv_cfg->dma_cfg.rx_burst = hal_cfg->dma_cfg.rx_burst;
    drv_cfg->dma_cfg.tx_burst = hal_cfg->dma_cfg.tx_burst;
#endif
}

/******************************************************************************
 ** \brief HAL confuration copy to driver confuration
 **
 ** \param [in]   handle      pointer of the handle create
 ** \param [in]   cfg         pointer of the hal confuration
 *****************************************************************************/
void hal_uart_init(void *handle, hal_uart_cfg_t *cfg)
{
    if (handle == NULL)
        return;

    uart_instance_t *instance = (uart_instance_t *)handle;
    uart_drv_cfg_t drv_cfg;
    hal_uart_cfg_copy_to_drv(cfg, &drv_cfg);
    uart_drv_init(instance->uartc, &(instance->drv_context), &drv_cfg);
}

/******************************************************************************
 ** \brief Send a character
 **
 ** \param [in]   handle      pointer of the handle create
 ** \param [in]   data        Character to put
 *****************************************************************************/
void hal_uart_putc(void *handle, char data)
{
    if (handle == NULL)
        return;

    uart_instance_t *instance = (uart_instance_t *)handle;
#ifdef UART_DRV_SUPPORT_RS485

    if (instance->drv_context.rs485_enable) {
        uart_drv_rs485_driver_set(instance->uartc, true);
        uart_drv_rs485_receiver_set(instance->uartc, false);
    }

#endif
    uart_drv_putc(instance->uartc, &(instance->drv_context), data);
#ifdef UART_DRV_SUPPORT_RS485

    if (instance->drv_context.rs485_enable) {
        uart_drv_rs485_driver_set(instance->uartc, false);
        uart_drv_rs485_receiver_set(instance->uartc, true);
    }

#endif
}

/******************************************************************************
 ** \brief Send some character
 **
 ** \param [in]   handle      pointer of the handle create
 ** \param [in]   buf         pointer of characters to put
 ** \param [in]   size        Size of characters
 ** \param [in]   async       if true, uart asynchronous transmit, after transmit
 **                           success, call the transmit callback function registered.
 **                           if false, uart synchronous transmit.
 *****************************************************************************/
void hal_uart_transmit(void *handle, char *buf, size_t size, bool async)
{
    if (handle == NULL)
        return;

    uart_instance_t *instance = (uart_instance_t *)handle;
#ifdef UART_DRV_SUPPORT_RS485

    if (instance->drv_context.rs485_enable) {
        uart_drv_rs485_driver_set(instance->uartc, true);
        uart_drv_rs485_receiver_set(instance->uartc, false);
    }

#endif
    uart_drv_transmit(instance->uartc, &(instance->drv_context), buf, size,
                      async);
#ifdef UART_DRV_SUPPORT_RS485

    if (instance->drv_context.rs485_enable) {
        uart_drv_rs485_driver_set(instance->uartc, false);
        uart_drv_rs485_receiver_set(instance->uartc, true);
    }

#endif
}

void hal_uart_receive(void *handle, char *buf, size_t size)
{
    if (handle == NULL)
        return;

    uart_instance_t *instance = (uart_instance_t *)handle;
#ifdef UART_DRV_SUPPORT_RS485

    if (instance->drv_context.rs485_enable) {
        uart_drv_rs485_driver_set(instance->uartc, false);
        uart_drv_rs485_receiver_set(instance->uartc, true);
    }

#endif
    uart_drv_receive(instance->uartc, &(instance->drv_context), buf, size);
#ifdef UART_DRV_SUPPORT_RS485

    if (instance->drv_context.rs485_enable) {
        uart_drv_rs485_driver_set(instance->uartc, true);
        uart_drv_rs485_receiver_set(instance->uartc, false);
    }

#endif
}

#ifdef UART_DRV_SUPPORT_9BITS
/******************************************************************************
 ** \brief Send a character
 **
 ** \param [in]   handle      pointer of the handle create
 ** \param [in]   data        Character to put
 *****************************************************************************/
void hal_uart_9bits_putc(void *handle, char data, bool addr)
{
    if (handle == NULL)
        return;

    uart_instance_t *instance = (uart_instance_t *)handle;
    uart_drv_9bits_putc(instance->uartc, &(instance->drv_context), data, addr);
}

/******************************************************************************
 ** \brief Send some character
 **
 ** \param [in]   handle      pointer of the handle create
 ** \param [in]   buf         pointer of characters to put
 ** \param [in]   size        Size of characters
 *****************************************************************************/
void hal_uart_9bits_transmit(void *handle, char addr, char *buf,
                             size_t size)
{
    if (handle == NULL)
        return;

    uart_instance_t *instance = (uart_instance_t *)handle;
    uart_drv_9bits_transmit(instance->uartc, &(instance->drv_context), addr,
                            buf, size);
}
#endif

/******************************************************************************
 ** \brief Get a received character
 **
 ** \param [in]   handle      pointer of the handle create
 ** \param [out]  data        pointer of characters to get
 *****************************************************************************/
bool hal_uart_getc(void *handle, char *data)
{
    if (handle == NULL)
        return false;

    uart_instance_t *instance = (uart_instance_t *)handle;
    return uart_drv_getc(instance->uartc, &(instance->drv_context), data);
}

/******************************************************************************
 ** \brief Set receive character callback function
 **
 ** \param [in]   handle      pointer of the handle create
 ** \param [in]   cbk
 *****************************************************************************/
void hal_uart_int_cbk_register(void *handle, hal_uart_int_src_t int_src,
                               hal_uart_int_callback cbk)
{
    if (handle == NULL)
        return;

    uart_instance_t *instance = (uart_instance_t *)handle;
    uart_drv_int_cbk_register(&(instance->drv_context), int_src, cbk);

    if (instance->irq_num != UART_HAL_INVALID_IRQ_NUM) {
        register_int_handler(instance->irq_num, hal_uart_irq_handle, handle);
        unmask_interrupt(instance->irq_num);
    }
}

void hal_uart_irq_mask(void *handle)
{
    if (handle == NULL)
        return;

    uart_instance_t *instance = (uart_instance_t *)handle;
    mask_interrupt(instance->irq_num);
}

void hal_uart_irq_unmask(void *handle)
{
    if (handle == NULL)
        return;

    uart_instance_t *instance = (uart_instance_t *)handle;
    unmask_interrupt(instance->irq_num);
}

void hal_uart_int_src_enable(void *handle, hal_uart_int_src_t int_src)
{
    if (handle == NULL)
        return;

    uart_drv_int_src_enable(handle, int_src);
}

void hal_uart_int_src_disable(void *handle, hal_uart_int_src_t int_src)
{
    if (handle == NULL)
        return;

    uart_drv_int_src_disable(handle, int_src);
}

void hal_uart_baudrate_set(void *handle, uint32_t sclk, uint32_t baud)
{
    uart_instance_t *instance = (uart_instance_t *)handle;
    uart_drv_baudrate_set(instance->uartc, sclk, baud);
}

#ifdef __cplusplus
}
#endif

