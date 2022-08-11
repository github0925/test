/********************************************************
 *            Copyright(c) 2018 Semidrive               *
 *******************************************************/

/**
 * @file    srv_pin.h
 * @brief   overall header file for pin service. Usually ATB call it to
 *          configure pad on the soc.
 */

#ifndef __SRV_PIN_H__
#define __SRV_PIN_H__

#include <stdint.h>

#define FS_PAD_REG_SEL_ID       30
#define FM_PAD_REG_SEL_ID       (0x3UL << FS_PAD_REG_SEL_ID)
#define FS_PAD_REG_SEL_MUX      20
#define FM_PAD_REG_SEL_MUX      (0x3FFUL << FS_PAD_REG_SEL_MUX)
#define FS_PAD_REG_SEL_PAD      10
#define FM_PAD_REG_SEL_PAD      (0x3FFUL << FS_PAD_REG_SEL_PAD)
#define FS_PAD_REG_SEL_IN_SEL   0
#define FM_PAD_REG_SEL_IN_SEL   (0x3FFUL << FS_PAD_REG_SEL_IN_SEL)
#define PAD_REG_SEL(id, mux, pad, in_sel)    \
    ((((id) << FS_PAD_REG_SEL_ID) & FM_PAD_REG_SEL_ID) | \
    (((mux) << FS_PAD_REG_SEL_MUX) & FM_PAD_REG_SEL_MUX) | \
    (((pad) << FS_PAD_REG_SEL_PAD) & FM_PAD_REG_SEL_PAD) | \
    (((in_sel) << FS_PAD_REG_SEL_IN_SEL) & FM_PAD_REG_SEL_IN_SEL))

#define GBF_PAD_REG_SEL_ID(x)   (((x) & FM_PAD_REG_SEL_ID) >> FS_PAD_REG_SEL_ID)
#define GBF_PAD_REG_SEL_MUX(x)   \
    (((x) & FM_PAD_REG_SEL_MUX) >> FS_PAD_REG_SEL_MUX)
#define GBF_PAD_REG_SEL_PAD(x)   \
    (((x) & FM_PAD_REG_SEL_PAD) >> FS_PAD_REG_SEL_PAD)
#define GBF_PAD_REG_SEL_IN_SEL(x)   \
    (((x) & FM_PAD_REG_SEL_IN_SEL) >> FS_PAD_REG_SEL_IN_SEL)

typedef struct {
    uint32_t reg_sel;    /**<  */
    uint32_t mux;        /**< value of mux register */
    uint32_t pad;        /**< value of pad register */
    uint32_t in_sel;     /**< value of input select register */
} pad_ctrl_t;

#define SEL_NONE    0x3FF
#define PAD_NONE    0x3FF
#define MUX_NONE    0x3FF

#define PIN_DCLR_NONE   {0, 0, 0, 0}

void srv_pin_cfg(const pad_ctrl_t *);
void srv_pin_dump(const pad_ctrl_t *);

#endif  /* __SRV_PIN_H__ */

