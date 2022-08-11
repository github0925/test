/********************************************************
 *            Copyright(c) 2018 Semidrive               *
 *******************************************************/

/**
 * @file    srv_pin.c
 * @brief   pin service. Usually ATB call it to
 *          configure pad on the soc.
 */

#include "srv_pin.h"
#include <soc.h>
#include <helper.h>

//#define PIN_DEBUG
#ifdef PIN_DEBUG
#define PIN_DBG(fmt, args...)   DBG(fmt, ##args)
#else
#define PIN_DBG(fmt, args...)
#endif

void srv_pin_cfg(const pad_ctrl_t *pad_list)
{
    if (NULL != pad_list) {
        const pad_ctrl_t *pad = pad_list;

        for (; pad->reg_sel != 0; pad++) {
            U32 ctrl_id = GBF_PAD_REG_SEL_ID(pad->reg_sel);
            U32 mux_reg_id = GBF_PAD_REG_SEL_MUX(pad->reg_sel);
            U32 pad_reg_id = GBF_PAD_REG_SEL_PAD(pad->reg_sel);
            U32 in_sel_reg_id = GBF_PAD_REG_SEL_IN_SEL(pad->reg_sel);

            U32 addr;

            if (MUX_NONE != mux_reg_id) {
                addr = soc_get_pin_mux_reg_addr(ctrl_id, mux_reg_id);
                writel(pad->mux, addr);
            }

            if (PAD_NONE != pad_reg_id) {
                addr = soc_get_pin_pad_reg_addr(ctrl_id, pad_reg_id);
                writel(pad->pad, addr);
            }

            if (SEL_NONE != in_sel_reg_id) {
                addr = soc_get_pin_in_sel_reg_addr(ctrl_id, in_sel_reg_id);
                writel(pad->in_sel, addr);
            }
        }
    }
}

#if defined(DEBUG_ENABLE) && defined(PIN_DEBUG)
void srv_pin_dump(const pad_ctrl_t *pad_list)
{
    if (NULL != pad_list) {
        const pad_ctrl_t *pad = &pad_list[0];

        for (; (pad->reg_sel != 0) && (pad->pad != 0); pad++) {
            U32 ctrl_id = GBF_PAD_REG_SEL_ID(pad->reg_sel);
            U32 mux_reg_id = GBF_PAD_REG_SEL_MUX(pad->reg_sel);
            U32 pad_reg_id = GBF_PAD_REG_SEL_PAD(pad->reg_sel);
            U32 in_sel_reg_id = GBF_PAD_REG_SEL_IN_SEL(pad->reg_sel);

            PIN_DBG("iomux: ctrl_id:%d, mux_reg_id:%d, pad_reg_id:%d, in_sel_reg_id:%d\n",
                    ctrl_id, mux_reg_id, pad_reg_id, in_sel_reg_id);
            U32 addr = soc_get_pin_mux_reg_addr(ctrl_id, mux_reg_id);
            PIN_DBG("iomux%d_mux_0x%x: 0x%x\n", ctrl_id,
                    addr - soc_get_pin_mux_reg_addr(ctrl_id, 0), readl(addr));
            addr = soc_get_pin_pad_reg_addr(ctrl_id, pad_reg_id);
            PIN_DBG("iomux%d_pad_0x%x: 0x%x\n", ctrl_id,
                    addr - soc_get_pin_pad_reg_addr(ctrl_id, 0), readl(addr));
            addr = soc_get_pin_in_sel_reg_addr(ctrl_id, in_sel_reg_id);
            PIN_DBG("iomux%d_in_sel_0x%x: 0x%x\n", ctrl_id,
                    addr - soc_get_pin_pad_reg_addr(ctrl_id, 0), readl(addr));
        }
    }
}
#endif
