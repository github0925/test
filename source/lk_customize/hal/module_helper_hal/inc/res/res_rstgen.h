/*
 * res_rstgen.h
 *
 * Copyright (c) 2019 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: MODULE HELPER rstgen.
 *
 * Revision History:
 * -----------------
 */
#ifndef _RES_RSTGEN_H
#define _RES_RSTGEN_H

#include <lk/list.h>
#include <assert.h>
//#include <module_helper_hal.h>
#include <module_helper_hal_internal.h>

#define REFCNT_NUM ((MAX_PER_ID-1)/32 +1)
enum rstgen_type {
    RST_TYPE_ISO,
    RST_TYPE_MODULE,
    RST_TYPE_CORE,
};

enum rstgen_param {
    RST_INVALID = 0xffffffff,
    RST_HOLD = 0,
    RST_RELEASE = 1,
};
enum rstgen_id {
    RES_RSTGEN_START = RES_CLK_END + 1,
    RSTGEN_ID_FIRST = RES_RSTGEN_START,
    /*ISO*/
    RSTGEN_TYPE_ISO_START = RSTGEN_ID_FIRST,
    RSTGEN_ID_ISO_CPU1 = RSTGEN_TYPE_ISO_START,
    RSTGEN_ID_ISO_DDR,
    RSTGEN_ID_ISO_GPU1,
    RSTGEN_ID_ISO_USB,
    RSTGEN_ID_ISO_PCIE,
    RSTGEN_TYPE_ISO_END = RSTGEN_ID_ISO_PCIE,
    /*CORE*/
    RSTGEN_TYPE_CORE_START,
    RSTGEN_ID_CORE_SAF_CR5_SW = RSTGEN_TYPE_CORE_START,
    RSTGEN_ID_CORE_SAF_CR5_EN,
    RSTGEN_ID_CORE_ADSP_SW,
    RSTGEN_ID_CORE_ADSP_EN,
    RSTGEN_ID_CORE_CPU2_CORE_SW,
    RSTGEN_ID_CORE_CPU2_CORE_EN,
    RSTGEN_ID_CORE_CPU1_CORE_ALL_SW,
    RSTGEN_ID_CORE_CPU1_CORE_ALL_EN,
    RSTGEN_ID_CORE_CR5_MP_SW,
    RSTGEN_ID_CORE_CR5_MP_EN,
    RSTGEN_ID_CORE_CR5_SEC_SW,
    RSTGEN_ID_CORE_CR5_SEC_EN,
    RSTGEN_ID_CORE_VDSP_SW,
    RSTGEN_ID_CORE_VDSP_EN,
    RSTGEN_TYPE_CORE_END = RSTGEN_ID_CORE_VDSP_EN,
    /*MODULE*/
    RSTGEN_TYPE_MODULE_START,
    RSTGEN_ID_MODULE_SEM2 = RSTGEN_TYPE_MODULE_START,
    RSTGEN_ID_MODULE_SEM1,
    RSTGEN_ID_MODULE_CANFD4,
    RSTGEN_ID_MODULE_CANFD3,
    RSTGEN_ID_MODULE_CANFD2,
    RSTGEN_ID_MODULE_CANFD1,
    RSTGEN_ID_MODULE_I2S_SC2,
    RSTGEN_ID_MODULE_I2S_SC1,
    RSTGEN_ID_MODULE_ENET1,
    RSTGEN_ID_MODULE_OSPI1,
    RSTGEN_ID_MODULE_GIC1,
    RSTGEN_ID_MODULE_CANFD20,
    RSTGEN_ID_MODULE_CANFD19,
    RSTGEN_ID_MODULE_CANFD18,
    RSTGEN_ID_MODULE_CANFD17,
    RSTGEN_ID_MODULE_CANFD16,
    RSTGEN_ID_MODULE_CANFD15,
    RSTGEN_ID_MODULE_CANFD14,
    RSTGEN_ID_MODULE_CANFD13,
    RSTGEN_ID_MODULE_CANFD12,
    RSTGEN_ID_MODULE_CANFD11,
    RSTGEN_ID_MODULE_CANFD10,
    RSTGEN_ID_MODULE_CANFD9,
    RSTGEN_ID_MODULE_DBG_REQ,
    RSTGEN_ID_MODULE_GPU2_SS,
    RSTGEN_ID_MODULE_GPU2_CORE,
    RSTGEN_ID_MODULE_GPU1_SS,
    RSTGEN_ID_MODULE_GPU1_CORE,
    RSTGEN_ID_MODULE_G2D2,
    RSTGEN_ID_MODULE_G2D1,
    RSTGEN_ID_MODULE_DISP_MUX,
    RSTGEN_ID_MODULE_CSI3,
    RSTGEN_ID_MODULE_CSI2,
    RSTGEN_ID_MODULE_CSI1,
    RSTGEN_ID_MODULE_LVDS_SS,
    RSTGEN_ID_MODULE_DP3,
    RSTGEN_ID_MODULE_DP2,
    RSTGEN_ID_MODULE_DP1,
    RSTGEN_ID_MODULE_DC5,
    RSTGEN_ID_MODULE_DC4,
    RSTGEN_ID_MODULE_DC3,
    RSTGEN_ID_MODULE_DC2,
    RSTGEN_ID_MODULE_DC1,
    RSTGEN_ID_MODULE_MIPI_DSI2,
    RSTGEN_ID_MODULE_MIPI_DSI1,
    RSTGEN_ID_MODULE_MIPI_CSI3,
    RSTGEN_ID_MODULE_MIPI_CSI2,
    RSTGEN_ID_MODULE_MIPI_CSI1,
    RSTGEN_ID_MODULE_USB2,
    RSTGEN_ID_MODULE_USB1,
    RSTGEN_ID_MODULE_PCIEPHY,
    RSTGEN_ID_MODULE_PCIE1,
    RSTGEN_ID_MODULE_PCIE2,
    RSTGEN_ID_MODULE_CPU2_SS,
    RSTGEN_ID_MODULE_CPU1_SS,
    RSTGEN_ID_MODULE_MJPEG,
    RSTGEN_ID_MODULE_VPU2,
    RSTGEN_ID_MODULE_VPU1,
    RSTGEN_ID_MODULE_VDSP_DRESET,
    RSTGEN_ID_MODULE_NNA,
    RSTGEN_ID_MODULE_CSSYS_TRESET_N,
    RSTGEN_ID_MODULE_GIC5,
    RSTGEN_ID_MODULE_GIC4,
    RSTGEN_ID_MODULE_DDR_SW_3,
    RSTGEN_ID_MODULE_DDR_SW_2,
    RSTGEN_ID_MODULE_DDR_SW_1,
    RSTGEN_ID_MODULE_DDR_SS,
    RSTGEN_ID_MODULE_CPU1_SCU_WARM,
    RSTGEN_ID_MODULE_CPU1_CORE5_WARM,
    RSTGEN_ID_MODULE_CPU1_CORE4_WARM,
    RSTGEN_ID_MODULE_CPU1_CORE3_WARM,
    RSTGEN_ID_MODULE_CPU1_CORE2_WARM,
    RSTGEN_ID_MODULE_CPU1_CORE1_WARM,
    RSTGEN_ID_MODULE_CPU1_CORE0_WARM,
    RSTGEN_ID_MODULE_GIC3,
    RSTGEN_ID_MODULE_GIC2,
    RSTGEN_ID_MODULE_ADSP,
    RSTGEN_ID_MODULE_MSHC4,
    RSTGEN_ID_MODULE_MSHC3,
    RSTGEN_ID_MODULE_MSHC2,
    RSTGEN_ID_MODULE_MSHC1,
    RSTGEN_ID_MODULE_ENET2,
    RSTGEN_ID_MODULE_CANFD8,
    RSTGEN_ID_MODULE_CANFD7,
    RSTGEN_ID_MODULE_CANFD6,
    RSTGEN_ID_MODULE_CANFD5,
    RSTGEN_ID_MODULE_I2S_MC2,
    RSTGEN_ID_MODULE_I2S_MC1,
    RSTGEN_ID_MODULE_I2S_SC8,
    RSTGEN_ID_MODULE_I2S_SC7,
    RSTGEN_ID_MODULE_I2S_SC6,
    RSTGEN_ID_MODULE_I2S_SC5,
    RSTGEN_ID_MODULE_I2S_SC4,
    RSTGEN_ID_MODULE_I2S_SC3,
    RSTGEN_ID_MODULE_OSPI2,
    RSTGEN_TYPE_MODULE_END = RSTGEN_ID_MODULE_OSPI2,
    RSTGEN_ID_END = RSTGEN_TYPE_MODULE_END,
    RES_RSTGEN_END = RSTGEN_ID_END,
};
struct rstgen {
    enum rstgen_id rstid;
    uint32_t resid;//global res id
    char name[32];

    uint32_t refcount[REFCNT_NUM];
    struct list_node node;  //dynmic, current node
    //struct list_node child;   //dynmic, head of child list

    int polar;//0: (0, hold, 1 release), 1: (1, hold, 0 release)
    enum rstgen_type type;
    //debug
    int release_cnt;
    int hold_cnt;

    int (*is_released)(struct rstgen *rst);
    int (*release)(struct rstgen *rst);
    int (*hold)(struct rstgen *rst);
};
void register_res_rstgens(void);

int res_rstgen_request(unsigned long per_id, unsigned long rstid,
                       unsigned long param);
//for debug
void dump_rstgen(void);

#endif /* _RES_RSTGEN_H */

