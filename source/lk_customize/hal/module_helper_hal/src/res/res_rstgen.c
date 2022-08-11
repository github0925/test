/*
 * res_rstgen.c
 *
 * Copyright (c) 2019 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: MODULE HELPER RSTGEN.
 *
 * Revision History:
 * -----------------
 */
#include <res/res_rstgen.h>
#include <chip_res.h>
#include "res.h"
#include <module_helper_hal_internal.h>
#include <string.h>
#include <macros.h>
#include <rstgen_hal.h>

struct list_node g_rstgen_root = LIST_INITIAL_VALUE(g_rstgen_root);

//internal for pll & ckgen
static bool rstgen_is_iso(struct rstgen *rst)
{
    return (rst->rstid >= RSTGEN_TYPE_ISO_START
            && rst->rstid <= RSTGEN_TYPE_ISO_END);
}

static bool rstgen_is_core(struct rstgen *rst)
{
    return (rst->rstid >= RSTGEN_TYPE_CORE_START
            && rst->rstid <= RSTGEN_TYPE_CORE_END);
}

static bool rstgen_is_module(struct rstgen *rst)
{
    return (rst->rstid >= RSTGEN_TYPE_MODULE_START
            && rst->rstid <= RSTGEN_TYPE_MODULE_END);
}

static enum rstgen_param rstgen_get_stat_from_hw(struct rstgen *rst)
{
    int ret;
    void *g_handle;
    ret = hal_rstgen_creat_handle(&g_handle, RES_GLOBAL_RST_SEC_RST_EN);

    if (!ret) {
        return RST_INVALID;
    }

    /*get handle ok and enable rstgen is true*/
    ret = hal_rstgen_init(g_handle);

    if (!ret) {
        hal_rstgen_release_handle(g_handle);
        return RST_INVALID;
    }

    ret = RST_INVALID;

    if (rstgen_is_iso(rst)) {
        ret = hal_rstgen_iso_status(g_handle, rst->resid);
    }
    else if (rstgen_is_core(rst)) {
        ret = hal_rstgen_core_status(g_handle, rst->resid);
    }
    else if (rstgen_is_module(rst)) {
        ret = hal_rstgen_module_status(g_handle, rst->resid);
    }

    hal_rstgen_release_handle(g_handle);

    if (ret == 0) {
        return RST_HOLD;
    }
    else if (ret == 1) {
        return RST_RELEASE;
    }
    else {
        return RST_INVALID;
    }
}
static int rstgen_is_released(struct rstgen *rst)
{
    return rst->release_cnt > rst->hold_cnt;
}

static int rstgen_endis(struct rstgen *rst, bool isrelease)
{
    int ret = 0;
    void *g_handle;
    ret = hal_rstgen_creat_handle(&g_handle, RES_GLOBAL_RST_SEC_RST_EN);

    if (!ret) {
        return -1;
    }

    /*get handle ok and enable rstgen is true*/
    ret = hal_rstgen_init(g_handle);

    if (!ret) {
        hal_rstgen_release_handle(g_handle);
        return -1;
    }

    if (rstgen_is_iso(rst)) {
        if (isrelease) {
            ret = hal_rstgen_iso_disable(g_handle, rst->resid);
        }
        else {
            ret = hal_rstgen_iso_enable(g_handle, rst->resid);
        }
    }
    else if (rstgen_is_module(rst)) {
        ret = hal_rstgen_module_ctl(g_handle, rst->resid, isrelease);
    }
    else if (rstgen_is_core(rst)) {
        ret = hal_rstgen_core_ctl(g_handle, rst->resid, isrelease);
    }
    else {
        ASSERT(0);
    }

    hal_rstgen_release_handle(g_handle);

    if (ret) {
        ret = 0;
    }
    else {
        ret = -1;
    }

    if (ret == 0) {
        isrelease ? rst->release_cnt++ : rst->hold_cnt++;
    }

    return ret;
}

static int rstgen_release(struct rstgen *rst)
{
    return rstgen_endis(rst, true);
}
static int rstgen_hold(struct rstgen *rst)
{
    return rstgen_endis(rst, false);
}
//
static struct rstgen *get_rstgen_by_id_internal(struct list_node *list,
        unsigned long rstid)
{
    struct rstgen *rst;

    if (!list) {
        return get_rstgen_by_id_internal(&g_rstgen_root, rstid);
    }

    if (list_is_empty(list)) {
        return NULL;
    }

    list_for_every_entry(list, rst, struct rstgen, node) {
        //dprintf(CRITICAL,"clk %s id %d, p %ld\n", clk->name, clk->clkid, clkid);
        if (rst->rstid ==
                rstid) { //TODO:also can check if this clk allowed to be accessed by user
            return rst;
        }
    }
    return NULL;
}

static struct rstgen *get_rstgen_by_id(unsigned long rstid)
{
    return get_rstgen_by_id_internal(NULL, rstid);
}

static int rstgen_init_refcount_bitmap(struct rstgen *rst)
{
    int i;

    for (i = 0; i < REFCNT_NUM; i++) {
        rst->refcount[i] = 0;
    }

    return 0;
}
static int rstgen_get_refcount_bits(struct rstgen *rst)
{
    int i;
    uint32_t n;
    int count = 0;

    for (i = 0; i < REFCNT_NUM; i++) {
        int j;
        n = rst->refcount[i];

        for (j = 0; j < 32; j++) {
            if ((1 << j)&n) {
                count++;
            }
        }
    }

    return count;
}

static int rstgen_set_refcount_bit(struct rstgen *rst, int per_id)
{
    int i = per_id / 32;
    ASSERT(i < REFCNT_NUM);
    rst->refcount[i] |= 1 << (per_id % 32);
    return 0;
}

static int rstgen_clear_refcount_bit(struct rstgen *rst, int per_id)
{
    int i = per_id / 32;
    ASSERT(i < REFCNT_NUM);
    rst->refcount[i] &= ~(1 << (per_id % 32));
    return 0;
}

static int rstgen_test_refcount_bit(struct rstgen *rst, int per_id)
{
    int i = per_id / 32;
    ASSERT(i < REFCNT_NUM);
    return (rst->refcount[i] & (1 << (per_id % 32)));
}


static int rstgen_get_refcount(struct rstgen *rst)
{
    int count = 0;
    //count refcount bitmap
    count += rstgen_get_refcount_bits(rst);
    return count;
}



/* child must call this function after add itself refcount,
*  and must change from 0 ->1.
*/
static int res_rstgen_inc_refcount(struct rstgen *rst,
                                   unsigned long per_id)
{
    int ret = 0;
    bool is_released = false;

    if (per_id != INVALID_PER_ID) {
        rstgen_set_refcount_bit(rst, per_id);
    }

    //TODO, have not check the refcount
    int cnt = rstgen_get_refcount(rst);

    if (cnt == 1) {
        if (rst->polar) {
            if (rst->is_released) {
                is_released = rst->is_released(rst);
            }

            if (is_released && rst->hold) {
                ret = rst->hold(rst);
            }
        }
        else {
            if (rst->is_released) {
                is_released = rst->is_released(rst);
            }

            if (!is_released && rst->release) {
                ret |= rst->release(rst);
            }
        }
    }

    return ret;
}
static int res_rstgen_dec_refcount(struct rstgen *rst,
                                   unsigned long per_id)
{
    int ret = 0;
    bool is_released = true;

    if (per_id != INVALID_PER_ID) {
        rstgen_clear_refcount_bit(rst, per_id);
    }

    int cnt = rstgen_get_refcount(rst);

    if (cnt == 0) {
        if (rst->polar) {
            if (rst->is_released) {
                is_released = rst->is_released(rst);
            }

            if (!is_released && rst->release) {
                ret |= rst->release(rst);
            }
        }
        else {
            if (rst->is_released) {
                is_released = rst->is_released(rst);
            }

            if (is_released && rst->hold) {
                ret = rst->hold(rst);
            }
        }
    }

    return ret;
}

int res_rstgen_request(unsigned long per_id, unsigned long rstid,
                       unsigned long param)
{
    struct rstgen *rst = get_rstgen_by_id(rstid);
    dprintf(DBGV, "req from %ld : rst %p id %ld param %lu\n", per_id, rst,
            rstid, param);

    if (!rst) {
        dprintf(CRITICAL, "no such rst %ld\n", rstid);
        return -1;
    }

    enum rstgen_param p = param;

    if (rst->polar) {
        if (p == RST_HOLD) {
            return res_rstgen_inc_refcount(rst, per_id);
        }
        else if (p == RST_RELEASE) {
            return res_rstgen_dec_refcount(rst, per_id);
        }
    }
    else {
        if (p == RST_HOLD) {
            return res_rstgen_dec_refcount(rst, per_id);
        }
        else if (p == RST_RELEASE) {
            return res_rstgen_inc_refcount(rst, per_id);
        }
    }

    ASSERT(0);
    return 0;
}

#define RSTGEN_ITEM_ISO(_rstid, _resid, _name)  \
     {   \
     .rstid = _rstid,    \
     .resid= _resid,     \
      .name = _name, \
      .polar = 0,   \
     }

#define RSTGEN_ITEM(_rstid, _resid, _name)  \
      {   \
      .rstid = _rstid,    \
      .resid= _resid,     \
       .name = _name, \
       .polar = 0,   \
      }

static struct rstgen init_rstgens_table[] = {
#if MODULE_HELPER_RSTGEN_ISO
    /*ISO*/
    RSTGEN_ITEM_ISO(RSTGEN_ID_ISO_CPU1, RES_ISO_EN_SEC_CPU1, "ISO_CPU1"),
    RSTGEN_ITEM_ISO(RSTGEN_ID_ISO_DDR, RES_ISO_EN_SEC_DDR, "ISO_DDR"),
    RSTGEN_ITEM_ISO(RSTGEN_ID_ISO_GPU1, RES_ISO_EN_SEC_GPU1, "ISO_GPU1"),
    RSTGEN_ITEM_ISO(RSTGEN_ID_ISO_USB, RES_ISO_EN_SEC_USB, "ISO_USB"),
    RSTGEN_ITEM_ISO(RSTGEN_ID_ISO_PCIE, RES_ISO_EN_SEC_PCIE, "ISO_PCIE"),
#endif
#if MODULE_HELPER_RSTGEN_MODULE
    /*MODULE*/
    RSTGEN_ITEM(RSTGEN_ID_MODULE_SEM2, RES_MODULE_RST_SAF_SEM2, "SEM2"),
    RSTGEN_ITEM(RSTGEN_ID_MODULE_SEM1, RES_MODULE_RST_SAF_SEM1, "SEM1"),
    RSTGEN_ITEM(RSTGEN_ID_MODULE_CANFD4, RES_MODULE_RST_SAF_CANFD4, "CANFD4"),
    RSTGEN_ITEM(RSTGEN_ID_MODULE_CANFD3, RES_MODULE_RST_SAF_CANFD3, "CANFD3"),
    RSTGEN_ITEM(RSTGEN_ID_MODULE_CANFD2, RES_MODULE_RST_SAF_CANFD2, "CANFD2"),
    RSTGEN_ITEM(RSTGEN_ID_MODULE_CANFD1, RES_MODULE_RST_SAF_CANFD1, "CANFD1"),
    RSTGEN_ITEM(RSTGEN_ID_MODULE_I2S_SC2, RES_MODULE_RST_SAF_I2S_SC2, "I2S_SC2"),
    RSTGEN_ITEM(RSTGEN_ID_MODULE_I2S_SC1, RES_MODULE_RST_SAF_I2S_SC1, "I2S_SC1"),
    RSTGEN_ITEM(RSTGEN_ID_MODULE_ENET1, RES_MODULE_RST_SAF_ENET1, "ENET1"),
    RSTGEN_ITEM(RSTGEN_ID_MODULE_OSPI1, RES_MODULE_RST_SAF_OSPI1, "OSPI1"),
    RSTGEN_ITEM(RSTGEN_ID_MODULE_GIC1, RES_MODULE_RST_SAF_GIC1, "GIC1"),
    RSTGEN_ITEM(RSTGEN_ID_MODULE_CANFD20, RES_MODULE_RST_SEC_CANFD20, "CANFD20"),
    RSTGEN_ITEM(RSTGEN_ID_MODULE_CANFD19, RES_MODULE_RST_SEC_CANFD19, "CANFD19"),
    RSTGEN_ITEM(RSTGEN_ID_MODULE_CANFD18, RES_MODULE_RST_SEC_CANFD18, "CANFD18"),
    RSTGEN_ITEM(RSTGEN_ID_MODULE_CANFD17, RES_MODULE_RST_SEC_CANFD17, "CANFD17"),
    RSTGEN_ITEM(RSTGEN_ID_MODULE_CANFD16, RES_MODULE_RST_SEC_CANFD16, "CANFD16"),
    RSTGEN_ITEM(RSTGEN_ID_MODULE_CANFD15, RES_MODULE_RST_SEC_CANFD15, "CANFD15"),
    RSTGEN_ITEM(RSTGEN_ID_MODULE_CANFD14, RES_MODULE_RST_SEC_CANFD14, "CANFD14"),
    RSTGEN_ITEM(RSTGEN_ID_MODULE_CANFD13, RES_MODULE_RST_SEC_CANFD13, "CANFD13"),
    RSTGEN_ITEM(RSTGEN_ID_MODULE_CANFD12, RES_MODULE_RST_SEC_CANFD12, "CANFD12"),
    RSTGEN_ITEM(RSTGEN_ID_MODULE_CANFD11, RES_MODULE_RST_SEC_CANFD11, "CANFD11"),
    RSTGEN_ITEM(RSTGEN_ID_MODULE_CANFD10, RES_MODULE_RST_SEC_CANFD10, "CANFD10"),
    RSTGEN_ITEM(RSTGEN_ID_MODULE_CANFD9, RES_MODULE_RST_SEC_CANFD9, "CANFD9"),
    RSTGEN_ITEM(RSTGEN_ID_MODULE_DBG_REQ, RES_MODULE_RST_SEC_DBG_REQ, "DBG_REQ"),
    RSTGEN_ITEM(RSTGEN_ID_MODULE_GPU2_SS, RES_MODULE_RST_SEC_GPU2_SS, "GPU2_SS"),
    RSTGEN_ITEM(RSTGEN_ID_MODULE_GPU2_CORE, RES_MODULE_RST_SEC_GPU2_CORE, "GPU2_CORE"),
    RSTGEN_ITEM(RSTGEN_ID_MODULE_GPU1_SS, RES_MODULE_RST_SEC_GPU1_SS, "GPU1_SS"),
    RSTGEN_ITEM(RSTGEN_ID_MODULE_GPU1_CORE, RES_MODULE_RST_SEC_GPU1_CORE, "GPU1_CORE"),
    RSTGEN_ITEM(RSTGEN_ID_MODULE_G2D2, RES_MODULE_RST_SEC_G2D2, "G2D2"),
    RSTGEN_ITEM(RSTGEN_ID_MODULE_G2D1, RES_MODULE_RST_SEC_G2D1, "G2D1"),
    RSTGEN_ITEM(RSTGEN_ID_MODULE_DISP_MUX, RES_MODULE_RST_SEC_DISP_MUX, "DISP_MUX"),
    RSTGEN_ITEM(RSTGEN_ID_MODULE_CSI3, RES_MODULE_RST_SEC_CSI3, "CSI3"),
    RSTGEN_ITEM(RSTGEN_ID_MODULE_CSI2, RES_MODULE_RST_SEC_CSI2, "CSI2"),
    RSTGEN_ITEM(RSTGEN_ID_MODULE_CSI1, RES_MODULE_RST_SEC_CSI1, "CSI1"),
    RSTGEN_ITEM(RSTGEN_ID_MODULE_LVDS_SS, RES_MODULE_RST_SEC_LVDS_SS, "LVDS_SS"),
    RSTGEN_ITEM(RSTGEN_ID_MODULE_DP3, RES_MODULE_RST_SEC_DP3, "DP3"),
    RSTGEN_ITEM(RSTGEN_ID_MODULE_DP2, RES_MODULE_RST_SEC_DP2, "DP2"),
    RSTGEN_ITEM(RSTGEN_ID_MODULE_DP1, RES_MODULE_RST_SEC_DP1, "DP1"),
    RSTGEN_ITEM(RSTGEN_ID_MODULE_DC5, RES_MODULE_RST_SEC_DC5, "DC5"),
    RSTGEN_ITEM(RSTGEN_ID_MODULE_DC4, RES_MODULE_RST_SEC_DC4, "DC4"),
    RSTGEN_ITEM(RSTGEN_ID_MODULE_DC3, RES_MODULE_RST_SEC_DC3, "DC3"),
    RSTGEN_ITEM(RSTGEN_ID_MODULE_DC2, RES_MODULE_RST_SEC_DC2, "DC2"),
    RSTGEN_ITEM(RSTGEN_ID_MODULE_DC1, RES_MODULE_RST_SEC_DC1, "DC1"),
    RSTGEN_ITEM(RSTGEN_ID_MODULE_MIPI_DSI2, RES_MODULE_RST_SEC_MIPI_DSI2, "MIPI_DSI2"),
    RSTGEN_ITEM(RSTGEN_ID_MODULE_MIPI_DSI1, RES_MODULE_RST_SEC_MIPI_DSI1, "MIPI_DSI1"),
    RSTGEN_ITEM(RSTGEN_ID_MODULE_MIPI_CSI3, RES_MODULE_RST_SEC_MIPI_CSI3, "MIPI_CSI3"),
    RSTGEN_ITEM(RSTGEN_ID_MODULE_MIPI_CSI2, RES_MODULE_RST_SEC_MIPI_CSI2, "MIPI_CSI2"),
    RSTGEN_ITEM(RSTGEN_ID_MODULE_MIPI_CSI1, RES_MODULE_RST_SEC_MIPI_CSI1, "MIPI_CSI1"),
    RSTGEN_ITEM(RSTGEN_ID_MODULE_USB2, RES_MODULE_RST_SEC_USB2, "USB2"),
    RSTGEN_ITEM(RSTGEN_ID_MODULE_USB1, RES_MODULE_RST_SEC_USB1, "USB1"),
    RSTGEN_ITEM(RSTGEN_ID_MODULE_PCIEPHY, RES_MODULE_RST_SEC_PCIEPHY, "PCIEPHY"),
    RSTGEN_ITEM(RSTGEN_ID_MODULE_PCIE1, RES_MODULE_RST_SEC_PCIE1, "PCIE1"),
    RSTGEN_ITEM(RSTGEN_ID_MODULE_PCIE2, RES_MODULE_RST_SEC_PCIE2, "PCIE2"),
    RSTGEN_ITEM(RSTGEN_ID_MODULE_CPU2_SS, RES_MODULE_RST_SEC_CPU2_SS, "CPU2_SS"),
    RSTGEN_ITEM(RSTGEN_ID_MODULE_CPU1_SS, RES_MODULE_RST_SEC_CPU1_SS, "CPU1_SS"),
    RSTGEN_ITEM(RSTGEN_ID_MODULE_MJPEG, RES_MODULE_RST_SEC_MJPEG, "MJPEG"),
    RSTGEN_ITEM(RSTGEN_ID_MODULE_VPU2, RES_MODULE_RST_SEC_VPU2, "VPU2"),
    RSTGEN_ITEM(RSTGEN_ID_MODULE_VPU1, RES_MODULE_RST_SEC_VPU1, "VPU1"),
    RSTGEN_ITEM(RSTGEN_ID_MODULE_VDSP_DRESET, RES_MODULE_RST_SEC_VDSP_DRESET, "VDSP_DRESET"),
    RSTGEN_ITEM(RSTGEN_ID_MODULE_NNA, RES_MODULE_RST_SEC_NNA, "NNA"),
    RSTGEN_ITEM(RSTGEN_ID_MODULE_CSSYS_TRESET_N, RES_MODULE_RST_SEC_CSSYS_TRESET_N, "CSSYS_TRESET_N"),
    RSTGEN_ITEM(RSTGEN_ID_MODULE_GIC5, RES_MODULE_RST_SEC_GIC5, "GIC5"),
    RSTGEN_ITEM(RSTGEN_ID_MODULE_GIC4, RES_MODULE_RST_SEC_GIC4, "GIC4"),
    RSTGEN_ITEM(RSTGEN_ID_MODULE_DDR_SW_3, RES_MODULE_RST_SEC_DDR_SW_3, "DDR_SW_3"),
    RSTGEN_ITEM(RSTGEN_ID_MODULE_DDR_SW_2, RES_MODULE_RST_SEC_DDR_SW_2, "DDR_SW_2"),
    RSTGEN_ITEM(RSTGEN_ID_MODULE_DDR_SW_1, RES_MODULE_RST_SEC_DDR_SW_1, "DDR_SW_1"),
    RSTGEN_ITEM(RSTGEN_ID_MODULE_DDR_SS, RES_MODULE_RST_SEC_DDR_SS, "DDR_SS"),
    RSTGEN_ITEM(RSTGEN_ID_MODULE_CPU1_SCU_WARM, RES_MODULE_RST_SEC_CPU1_SCU_WARM, "CPU1_SCU_WARM"),
    RSTGEN_ITEM(RSTGEN_ID_MODULE_CPU1_CORE5_WARM, RES_MODULE_RST_SEC_CPU1_CORE5_WARM, "CPU1_CORE5_WARM"),
    RSTGEN_ITEM(RSTGEN_ID_MODULE_CPU1_CORE4_WARM, RES_MODULE_RST_SEC_CPU1_CORE4_WARM, "CPU1_CORE4_WARM"),
    RSTGEN_ITEM(RSTGEN_ID_MODULE_CPU1_CORE3_WARM, RES_MODULE_RST_SEC_CPU1_CORE3_WARM, "CPU1_CORE3_WARM"),
    RSTGEN_ITEM(RSTGEN_ID_MODULE_CPU1_CORE2_WARM, RES_MODULE_RST_SEC_CPU1_CORE2_WARM, "CPU1_CORE2_WARM"),
    RSTGEN_ITEM(RSTGEN_ID_MODULE_CPU1_CORE1_WARM, RES_MODULE_RST_SEC_CPU1_CORE1_WARM, "CPU1_CORE1_WARM"),
    RSTGEN_ITEM(RSTGEN_ID_MODULE_CPU1_CORE0_WARM, RES_MODULE_RST_SEC_CPU1_CORE0_WARM, "CPU1_CORE0_WARM"),
    RSTGEN_ITEM(RSTGEN_ID_MODULE_GIC3, RES_MODULE_RST_SEC_GIC3, "GIC3"),
    RSTGEN_ITEM(RSTGEN_ID_MODULE_GIC2, RES_MODULE_RST_SEC_GIC2, "GIC2"),
    RSTGEN_ITEM(RSTGEN_ID_MODULE_ADSP, RES_MODULE_RST_SEC_ADSP, "ADSP"),
    RSTGEN_ITEM(RSTGEN_ID_MODULE_MSHC4, RES_MODULE_RST_SEC_MSHC4, "MSHC4"),
    RSTGEN_ITEM(RSTGEN_ID_MODULE_MSHC3, RES_MODULE_RST_SEC_MSHC3, "MSHC3"),
    RSTGEN_ITEM(RSTGEN_ID_MODULE_MSHC2, RES_MODULE_RST_SEC_MSHC2, "MSHC2"),
    RSTGEN_ITEM(RSTGEN_ID_MODULE_MSHC1, RES_MODULE_RST_SEC_MSHC1, "MSHC1"),
    RSTGEN_ITEM(RSTGEN_ID_MODULE_ENET2, RES_MODULE_RST_SEC_ENET2, "ENET2"),
    RSTGEN_ITEM(RSTGEN_ID_MODULE_CANFD8, RES_MODULE_RST_SEC_CANFD8, "CANFD8"),
    RSTGEN_ITEM(RSTGEN_ID_MODULE_CANFD7, RES_MODULE_RST_SEC_CANFD7, "CANFD7"),
    RSTGEN_ITEM(RSTGEN_ID_MODULE_CANFD6, RES_MODULE_RST_SEC_CANFD6, "CANFD6"),
    RSTGEN_ITEM(RSTGEN_ID_MODULE_CANFD5, RES_MODULE_RST_SEC_CANFD5, "CANFD5"),
    RSTGEN_ITEM(RSTGEN_ID_MODULE_I2S_MC2, RES_MODULE_RST_SEC_I2S_MC2, "I2S_MC2"),
    RSTGEN_ITEM(RSTGEN_ID_MODULE_I2S_MC1, RES_MODULE_RST_SEC_I2S_MC1, "I2S_MC1"),
    RSTGEN_ITEM(RSTGEN_ID_MODULE_I2S_SC8, RES_MODULE_RST_SEC_I2S_SC8, "I2S_SC8"),
    RSTGEN_ITEM(RSTGEN_ID_MODULE_I2S_SC7, RES_MODULE_RST_SEC_I2S_SC7, "I2S_SC7"),
    RSTGEN_ITEM(RSTGEN_ID_MODULE_I2S_SC6, RES_MODULE_RST_SEC_I2S_SC6, "I2S_SC6"),
    RSTGEN_ITEM(RSTGEN_ID_MODULE_I2S_SC5, RES_MODULE_RST_SEC_I2S_SC5, "I2S_SC5"),
    RSTGEN_ITEM(RSTGEN_ID_MODULE_I2S_SC4, RES_MODULE_RST_SEC_I2S_SC4, "I2S_SC4"),
    RSTGEN_ITEM(RSTGEN_ID_MODULE_I2S_SC3, RES_MODULE_RST_SEC_I2S_SC3, "I2S_SC3"),
    RSTGEN_ITEM(RSTGEN_ID_MODULE_OSPI2, RES_MODULE_RST_SEC_OSPI2, "OSPI2"),
#endif
#if MODULE_HELPER_RSTGEN_CORE
    /*CORE*/

    RSTGEN_ITEM(RSTGEN_ID_CORE_SAF_CR5_SW, RES_CORE_RST_SAF_CR5_SAF_SW, "SAF_CR5_SW"),
    RSTGEN_ITEM(RSTGEN_ID_CORE_SAF_CR5_EN, RES_CORE_RST_SAF_CR5_SAF_EN, "SAF_CR5_EN"),
    RSTGEN_ITEM(RSTGEN_ID_CORE_ADSP_SW, RES_CORE_RST_SEC_ADSP_SW, "ADSP_SW"),
    RSTGEN_ITEM(RSTGEN_ID_CORE_ADSP_EN, RES_CORE_RST_SEC_ADSP_EN, "ADSP_EN"),
    RSTGEN_ITEM(RSTGEN_ID_CORE_CPU2_CORE_SW, RES_CORE_RST_SEC_CPU2_CORE_SW, "CPU2_CORE_SW"),
    RSTGEN_ITEM(RSTGEN_ID_CORE_CPU2_CORE_EN, RES_CORE_RST_SEC_CPU2_CORE_EN, "CPU2_CORE_EN"),
    RSTGEN_ITEM(RSTGEN_ID_CORE_CPU1_CORE_ALL_SW, RES_CORE_RST_SEC_CPU1_CORE_ALL_SW, "CPU1_CORE_ALL_SW"),
    RSTGEN_ITEM(RSTGEN_ID_CORE_CPU1_CORE_ALL_EN, RES_CORE_RST_SEC_CPU1_CORE_ALL_EN, "CPU1_CORE_ALL_EN"),
    RSTGEN_ITEM(RSTGEN_ID_CORE_CR5_MP_SW, RES_CORE_RST_SEC_CR5_MP_SW, "CR5_MP_SW"),
    RSTGEN_ITEM(RSTGEN_ID_CORE_CR5_MP_EN, RES_CORE_RST_SEC_CR5_MP_EN, "CR5_MP_EN"),
    RSTGEN_ITEM(RSTGEN_ID_CORE_CR5_SEC_SW, RES_CORE_RST_SEC_CR5_SEC_SW, "CR5_SEC_SW"),
    RSTGEN_ITEM(RSTGEN_ID_CORE_CR5_SEC_EN, RES_CORE_RST_SEC_CR5_SEC_EN, "CR5_SEC_EN"),
    RSTGEN_ITEM(RSTGEN_ID_CORE_VDSP_SW, RES_CORE_RST_SEC_VDSP_SW, "VDSP_SW"),
    RSTGEN_ITEM(RSTGEN_ID_CORE_VDSP_EN, RES_CORE_RST_SEC_VDSP_EN, "VDSP_EN"),
#endif
};

static bool is_res_belong_this_domain(uint32_t resid)
{
    int ret = 0;
    paddr_t phy_addr;
    int32_t slice_idx = -1;
    ret = res_get_info_by_id(resid, &phy_addr, &slice_idx);

    if (ret == -1) {
        //printf("res_glb_idx:0x%x is not belong this domain\n", resid);
        return false;
    }

    return true;
}

void register_res_rstgens(void)
{
    int i;
    int num = ARRAYSIZE(init_rstgens_table);
    struct rstgen *rst;
    list_initialize(&g_rstgen_root);

    for (i = 0; i < num; i++) {
        enum rstgen_param rst_status;
        rst = &init_rstgens_table[i];

        //dprintf(CRITICAL, "register rst %s \n", rst->name);
        if (!is_res_belong_this_domain(rst->resid)) {
            continue;
        }

        list_clear_node(&rst->node);
        //list_initialize(&rst->child);
        //init refcount
        rstgen_init_refcount_bitmap(rst);
        rst->is_released = rstgen_is_released;
        rst->release = rstgen_release;
        rst->hold = rstgen_hold;
        rst_status = rstgen_get_stat_from_hw(rst);

        if (rst_status == RST_RELEASE) {
            rst->release_cnt = 1;
        }
        else if (rst_status == RST_HOLD) {
            rst->hold_cnt = 1;
        }
        else { //invalid
            continue;
        }

        list_add_tail(&g_rstgen_root, &rst->node);
    }
}

static void print_prefix(int depth, const char *s)
{
    int i;

    for (i = 0; i < depth; i++) {
        dprintf(CRITICAL, "%s", s);
    }
}

void dump_rstgen_internal(struct rstgen *rst, int depth)
{
    int i;

    if (!rst) { //mean dump whole clk tree
        struct rstgen *n = NULL;

        if (list_is_empty(&g_rstgen_root)) {
            return;
        }

        list_for_every_entry(&g_rstgen_root, n, struct rstgen, node) {
            dump_rstgen_internal(n, depth);
        }
        return ;
    }

    int refcount = rstgen_get_refcount(rst);
    //format dump
    //print_prefix(depth, "\t");
    //content
    dprintf(CRITICAL, "%s", rst->name);

    if (depth > (int)strlen(rst->name) / 8) {
        print_prefix(depth - strlen(rst->name) / 8, "\t");
    }

    dprintf(CRITICAL, "ref:%d\t release:%d\t hold:%d\n", refcount,
            rst->release_cnt,
            rst->hold_cnt);

    //print peripheral reference
    for (i = 0; i < MAX_PER_ID; i++) {
        char perstr[100];

        if (rstgen_test_refcount_bit(rst, i)) {
            print_prefix(1, "\t");
            sprintf(perstr, "%s(%d)", module_get_per_name_by_id(i), i);
            dprintf(CRITICAL, "%s", perstr);

            if (depth > (int)strlen(perstr) / 8) {
                print_prefix(depth - strlen(perstr) / 8 - 1, "\t");
            }

            dprintf(CRITICAL, "ref:1\t\n");
        }
    }
}

void dump_rstgen()
{
    dump_rstgen_internal(NULL, 3);
}

#if defined(WITH_LIB_CONSOLE)
#include <lib/console.h>

int cmd_dumprstgen(int argc, const cmd_args *argv)
{
    dump_rstgen();
    return 0;
}

STATIC_COMMAND_START STATIC_COMMAND("dumprstgen", "dump rst gen",
                                    (console_cmd)&cmd_dumprstgen)
STATIC_COMMAND_END(dumprstgen);
#endif

