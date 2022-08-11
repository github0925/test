/********************************************************
 *      Copyright(c) 2019   Semidrive  Semiconductor    *
 *      All rights reserved.                            *
 ********************************************************/

#include <common_hdr.h>
#include <uart/uart.h>
#include <soc.h>
#include "DW_apb_uart_reg.h"
#include <_assert.h>

static int32_t dwc_uart_init(uintptr_t b, uart_cfg_t *cfg)
{
    if (NULL == cfg) {
        return -1;
    }

    writel(0, b + MCR_OFF);
    uint32_t v = FV_FCR_RT(1) | FV_FCR_TET(3) | BM_FCR_DMAM | BM_FCR_FIFOE;
    writel(v, b + FCR_OFF);
    v |= BM_FCR_XFIFOR | BM_FCR_RFIFOR;
    writel(v, b + FCR_OFF);

    U32 lcr = readl(b + LCR_OFF);

    if (UART_PARITY_NONE == cfg->parity) {
        lcr &= ~BM_LCR_PEN;
    } else {
        lcr |= BM_LCR_PEN;

        if (UART_PARITY_EVEN == cfg->parity) {
            lcr |= BM_LCR_EPS;
        } else {
            lcr &= ~BM_LCR_EPS;
        }
    }

    if (cfg->stop == STOP_1BIT) {
        lcr &= ~BM_LCR_STOP;
    } else {
        lcr |= BM_LCR_STOP;
    }

    lcr |= FV_LCR_DLS(3);   /* 8bits per character */
    writel(lcr, b + LCR_OFF);

    lcr |= BM_LCR_DLAB;
    writel(lcr, b + LCR_OFF);

    if (0u != cfg->baud_rate) {
        uint32_t rate = cfg->baud_rate;
        uint32_t div_i = UART_ROOT_CLK_FREQ / 16 / rate;
        uint32_t div_f = UART_ROOT_CLK_FREQ - (div_i * rate * 16);
        div_f = div_f / rate;    /* DLF_SIZE = 4 */

        assert(0 != div_i);

        writel((div_i >> 8) & 0xffu, b + DLH_OFF);
        writel(div_i & 0xffu, b + DLL_OFF);

        writel(div_f, b + DLF_OFF);
    }

    lcr &= ~BM_LCR_DLAB;
    writel(lcr, b + LCR_OFF);

    return 0;
}

static int32_t dwc_uart_tx(uintptr_t b, uint8_t *data, size_t sz)
{
    while (sz--) {
        while (!(readl(b + USR_OFF) & (BM_USR_TFNF | BM_USR_TFE)));

        writel(*data, b + THR_OFF);
        data++;
    }

    return 0;
}

static int32_t dwc_uart_rx(uintptr_t b, uint8_t *data, size_t sz)
{
    while (sz--) {
        while (!(readl(b + USR_OFF) & (BM_USR_RFNE | BM_USR_RFF)));

        *data++ = (uint8_t)readl(b + RBR_OFF);
    }

    return 0;
}

const uart_ops_t uart_ctl = {
    dwc_uart_init,
    dwc_uart_tx,
    dwc_uart_rx,
};
