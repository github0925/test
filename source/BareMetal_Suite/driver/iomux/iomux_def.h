/********************************************************
 *          Copyright(c) 2018   Semidrive               *
 *******************************************************/

/**
 * @file    iomux_def.h
 * @brief   register definiations of iomux controller.
 */

#ifndef __IOMUX_DEF_H__
#define __IOMUX_DEF_H__


#define BM_IO_PAD_CONFIG_POE    (0x01U << 16U)
/* low for cmos and high for schmitt input */
#define BM_IO_PAD_CONFIG_IS (0x01U << 12U)
/* high for slow slew rate and low for fast */
#define BM_IO_PAD_CONFIG_SR (0x01U << 8U)

/* 00 - 2mA, 10 - 4mA, 01 - 8mA, 11 - 12mA*/
#define FM_IO_PAD_CONFIG_DS (0x3U << 4U)
#define FV_IO_PAD_CONFIG_DS(v) \
    (((v) << 4U) & FM_IO_PAD_CONFIG_DS)
#define GFV_IO_PAD_CONFIG_DS(v) \
    (((v) & FM_IO_PAD_CONFIG_DS) >> 4U)
/* 1 - Pull up; 0 - Pull down*/
#define BM_IO_PAD_CONFIG_PS (0x01U << 1U)
/* Pull enable*/
#define BM_IO_PAD_CONFIG_PE (0x01U << 0U)
/* Force Input Value when fin=0x02*/
#define BM_PIN_MUX_CONFIG_FV    (0x01U << 12U)
/* Force Input On/Off mode.
 * 00 - IE=~func_dir; 01: IE=1; 10: IE=1'b0; 11: reserved */
#define FM_PIN_MUX_CONFIG_FIN   (0x3U << 8U)
#define FV_PIN_MUX_CONFIG_FIN(v) \
    (((v) << 8U) & FM_PIN_MUX_CONFIG_FIN)
#define GFV_PIN_MUX_CONFIG_FIN(v) \
    (((v) & FM_PIN_MUX_CONFIG_FIN) >> 8U)
/* Open-Drain Mode enable*/
#define BM_PIN_MUX_CONFIG_ODE   (0x01U << 4U)

#define FM_PIN_MUX_CONFIG_MUX   (0x7U << 0U)
#define FV_PIN_MUX_CONFIG_MUX(v) \
    (((v) << 0U) & FM_PIN_MUX_CONFIG_MUX)
#define GFV_PIN_MUX_CONFIG_MUX(v) \
    (((v) & FM_PIN_MUX_CONFIG_MUX) >> 0U)


#define FM_INPUT_SOURCE_SELECT_SRC_SEL  (0x3U << 0U)
#define FV_INPUT_SOURCE_SELECT_SRC_SEL(v) \
    (((v) << 0U) & FM_INPUT_SOURCE_SELECT_SRC_SEL)
#define GFV_INPUT_SOURCE_SELECT_SRC_SEL(v) \
    (((v) & FM_INPUT_SOURCE_SELECT_SRC_SEL) >> 0U)


#endif  /* __IOMUX_DEF_H__ */
