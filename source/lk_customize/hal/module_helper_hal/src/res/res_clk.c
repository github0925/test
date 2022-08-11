/*
 * res_clk.c
 *
 * Copyright (c) 2019 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: MODULE HELPER CLK.
 *
 * Revision History:
 * -----------------
 */
#include <res/res_clk.h>
#include <chip_res.h>
#include "res.h"
#include <module_helper_hal_internal.h>
#include <string.h>
#include <macros.h>

#ifdef RES_CLK_DEBUG
#define CLK_TRACE   dprintf
#else
#define CLK_TRACE(x...) do { } while (0)
#endif

#define PLL_ROOT 0
#define PLL_DIVA 1
#define PLL_DIVB 2
#define PLL_DIVC 3
#define PLL_DIVD 4
#define PLL_DUMMY_ROOT 5

#define UUU_SEL0 0
#define UUU_SEL1 1
#define UUU_M 2
#define UUU_N 3
#define UUU_P 4
#define UUU_Q 5

static volatile int round_rate_filter_en = 1;
static volatile int set_rate_overwrite_allow = 0;
static volatile int dont_change_pll = 0;
static volatile enum module_per_id cur_per_id = 0;
static volatile enum clk_id cur_clkid = 0;

#define abs_clk(a, b)   \
    ((a > b) ? (a-b):(b-a))

struct clk_request {
    unsigned long request;
    unsigned long best_prate;
    int best_parent_index;
};

struct list_node g_clk_root = LIST_INITIAL_VALUE(g_clk_root);
static unsigned long res_clk_div_round_rate(struct clk *clk, struct clk *p,
        int div_i, unsigned long *prate, unsigned long freq);
static unsigned long res_clk_round_rate(struct clk *clk,
                                        struct clk_request *req);
static int clk_get_refcount(struct clk *clk, bool freqcare);
static unsigned long res_clk_get_rate(struct clk *clk, bool bymonitor);
static bool res_clk_is_valid_round_rate(struct clk *clk, int pindex,
                                        unsigned long rate, unsigned long prate);
static bool clk_is_hw_enable(struct clk *clk);

#ifdef RES_CLK_DEBUG
#define CLK_ITEM_FIXRATE(_clkid, _name, _rate)  \
    {   \
    .clkid = _clkid,    \
    .resid=0,   \
     .name = _name, \
     .rate = _rate, \
     .ptable_size = 0,  \
    }

#define CLK_ITEM_PLL(_clkid, _resid, _name, _ratetab)    \
    {   \
    .clkid = _clkid,    \
    .resid=_resid,  \
     .name = _name, \
     .ptable_size = 0,  \
     .pll.plldiv = PLL_DUMMY_ROOT,    \
     .pll.ratetable_size = ARRAYSIZE(_ratetab),   \
     .pll.ratetable = _ratetab,   \
    }

#define CLK_ITEM_PLL_ROOT(_clkid, _resid, _name, _ratetab)    \
    {   \
    .clkid = _clkid ##_ROOT,    \
    .resid=_resid,  \
     .name = _name"_ROOT", \
     .ptable_size = 0,  \
     .ptable = (void*)_clkid,   \
     .pll.plldiv = PLL_ROOT,    \
     .pll.ratetable_size = ARRAYSIZE(_ratetab),   \
     .pll.ratetable = _ratetab,   \
    }

#define CLK_ITEM_PLL_DIVA(_clkid, _resid, _name, _ratetab)    \
    {   \
    .clkid = _clkid ##_DIVA,    \
    .resid=_resid,  \
     .name = _name"_DIVA",  \
     .ptable_size = 0,  \
     .ptable = (void*)_clkid,   \
     .pll.plldiv = PLL_DIVA,    \
     .pll.ratetable_size = ARRAYSIZE(_ratetab),   \
     .pll.ratetable = _ratetab,   \
    }

#define CLK_ITEM_PLL_DIVB(_clkid, _resid, _name, _ratetab)    \
    {   \
    .clkid = _clkid ##_DIVB,    \
    .resid=_resid,  \
     .name = _name"_DIVB",  \
     .ptable_size = 0,  \
     .ptable = (void*)_clkid,   \
     .pll.plldiv = PLL_DIVB,    \
     .pll.ratetable_size = ARRAYSIZE(_ratetab),   \
     .pll.ratetable = _ratetab,   \
    }

#define CLK_ITEM_PLL_DIVC(_clkid, _resid, _name, _ratetab)    \
    {   \
    .clkid = _clkid ##_DIVC,    \
    .resid=_resid,  \
     .name = _name"_DIVC",  \
     .ptable_size = 0,  \
     .ptable = (void*)_clkid,  \
     .pll.plldiv = PLL_DIVC,    \
     .pll.ratetable_size = ARRAYSIZE(_ratetab),   \
     .pll.ratetable = _ratetab,   \
    }

#define CLK_ITEM_PLL_DIVD(_clkid, _resid, _name, _ratetab)    \
    {   \
    .clkid = _clkid ##_DIVD,    \
    .resid=_resid,  \
     .name = _name"_DIVD",  \
     .ptable_size = 0,  \
     .ptable = (void*)_clkid,  \
     .pll.plldiv = PLL_DIVD,    \
     .pll.ratetable_size = ARRAYSIZE(_ratetab),   \
     .pll.ratetable = _ratetab,   \
    }
#define CLK_ITEM_WITH_PARENT(_clkid, _resid, _name) \
    {   \
    .clkid = _clkid,    \
    .resid=_resid,  \
     .name = _name, \
     .ptable_size = ARRAYSIZE(_clkid ##_tab),   \
     .ptable = _clkid ##_tab,   \
    }

#define CLK_ITEM_WITH_TABLE(_clkid, _resid, _name, _table)  \
    {   \
    .clkid = _clkid,    \
    .resid=_resid,  \
     .name = _name, \
     .ptable_size = ARRAYSIZE(_table),  \
     .ptable = _table,  \
    }

#define CLK_ITEM_WITH_TABLE_AND_GATE(_clkid, _resid, _gateid, _name, _table)  \
    {   \
    .clkid = _clkid,    \
    .resid=_resid,  \
    .gate_resid = _gateid,  \
     .name = _name, \
     .ptable_size = ARRAYSIZE(_table),  \
     .ptable = _table,  \
    }

#define CLK_UUU_ITEM_WITH_TABLE(_clkid, _resid, _type, _name, _table)   \
    {   \
    .clkid = _clkid,    \
    .resid=_resid,  \
     .name = _name, \
     .ptable_size = ARRAYSIZE(_table),  \
     .ptable = _table,  \
     .uuu.uuu_type = _type  ,\
    }

#else
#define CLK_ITEM_FIXRATE(_clkid, _name, _rate)  \
    {   \
    .clkid = _clkid,    \
    .resid=0,   \
     .rate = _rate, \
     .ptable_size = 0,  \
    }

#define CLK_ITEM_PLL(_clkid, _resid, _name, _ratetab)    \
    {   \
    .clkid = _clkid,    \
    .resid=_resid,  \
     .ptable_size = 0,  \
     .pll.plldiv = PLL_DUMMY_ROOT,    \
    .pll.ratetable_size = ARRAYSIZE(_ratetab),   \
    .pll.ratetable = _ratetab,   \
    }

#define CLK_ITEM_PLL_ROOT(_clkid, _resid, _name, _ratetab)    \
    {   \
    .clkid = _clkid ##_ROOT,    \
    .resid=_resid,  \
     .ptable_size = 0,  \
     .pll.plldiv = PLL_ROOT,    \
     .ptable = (void*)_clkid,   \
     .pll.ratetable_size = ARRAYSIZE(_ratetab),   \
     .pll.ratetable = _ratetab, \
    }

#define CLK_ITEM_PLL_DIVA(_clkid, _resid, _name, _ratetab)    \
    {   \
    .clkid = _clkid ##_DIVA,    \
    .resid=_resid,  \
     .ptable_size = 0,  \
     .pll.plldiv = PLL_DIVA,    \
     .ptable = (void*)_clkid,   \
     .pll.ratetable_size = ARRAYSIZE(_ratetab),   \
     .pll.ratetable = _ratetab, \
    }

#define CLK_ITEM_PLL_DIVB(_clkid, _resid, _name, _ratetab)    \
    {   \
    .clkid = _clkid ##_DIVB,    \
    .resid=_resid,  \
     .ptable_size = 0,  \
     .pll.plldiv = PLL_DIVB,    \
     .ptable = (void*)_clkid,   \
     .pll.ratetable_size = ARRAYSIZE(_ratetab),   \
     .pll.ratetable = _ratetab, \
    }

#define CLK_ITEM_PLL_DIVC(_clkid, _resid, _name, _ratetab)    \
    {   \
    .clkid = _clkid ##_DIVC,    \
    .resid=_resid,  \
     .ptable_size = 0,  \
     .pll.plldiv = PLL_DIVC,    \
    .ptable = (void*)_clkid,  \
    .pll.ratetable_size = ARRAYSIZE(_ratetab),   \
     .pll.ratetable = _ratetab, \
    }

#define CLK_ITEM_PLL_DIVD(_clkid, _resid, _name, _ratetab)    \
    {   \
    .clkid = _clkid ##_DIVD,    \
    .resid=_resid,  \
     .ptable_size = 0,  \
     .pll.plldiv = PLL_DIVD,    \
    .ptable = (void*)_clkid,  \
    .pll.ratetable_size = ARRAYSIZE(_ratetab),   \
     .pll.ratetable = _ratetab, \
    }
#define CLK_ITEM_WITH_PARENT(_clkid, _resid, _name) \
    {   \
    .clkid = _clkid,    \
    .resid=_resid,  \
     .ptable_size = ARRAYSIZE(_clkid ##_tab),   \
     .ptable = _clkid ##_tab,   \
    }

#define CLK_ITEM_WITH_TABLE(_clkid, _resid, _name, _table)  \
    {   \
    .clkid = _clkid,    \
    .resid=_resid,  \
     .ptable_size = ARRAYSIZE(_table),  \
     .ptable = _table,  \
    }

#define CLK_ITEM_WITH_TABLE_AND_GATE(_clkid, _resid, _gateid, _name, _table)  \
    {   \
    .clkid = _clkid,    \
    .resid=_resid,  \
    .gate_resid = _gateid,  \
     .ptable_size = ARRAYSIZE(_table),  \
     .ptable = _table,  \
    }


#define CLK_UUU_ITEM_WITH_TABLE(_clkid, _resid, _type, _name, _table)   \
    {   \
    .clkid = _clkid,    \
    .resid=_resid,  \
     .ptable_size = ARRAYSIZE(_table),  \
     .ptable = _table,  \
     .uuu.uuu_type = _type  ,\
    }

#endif

#define CLK_ITEM_PLL_ABCD(_clkid, _resid, _name, _ratetab)    \
    CLK_ITEM_PLL(_clkid, _resid, _name, _ratetab),   \
    CLK_ITEM_PLL_ROOT(_clkid, _resid, _name, _ratetab),   \
    CLK_ITEM_PLL_DIVA(_clkid, _resid, _name, _ratetab),   \
    CLK_ITEM_PLL_DIVB(_clkid, _resid, _name, _ratetab),   \
    CLK_ITEM_PLL_DIVC(_clkid, _resid, _name, _ratetab),   \
    CLK_ITEM_PLL_DIVD(_clkid, _resid, _name, _ratetab)

#define CLK_ITEM_PLL_AB(_clkid, _resid, _name, _ratetab)  \
    CLK_ITEM_PLL(_clkid, _resid, _name, _ratetab),   \
    CLK_ITEM_PLL_ROOT(_clkid, _resid, _name, _ratetab),   \
    CLK_ITEM_PLL_DIVA(_clkid, _resid, _name, _ratetab),   \
    CLK_ITEM_PLL_DIVB(_clkid, _resid, _name, _ratetab)

#define CLK_UUU_RATIO_LIMIT(_pre_id, m, n, p, q)    \
    clk_set_ratio_limit(_pre_id ##_M, m, INT16_MAX);    \
    clk_set_ratio_limit(_pre_id ##_1, n, INT16_MAX);    \
    clk_set_ratio_limit(_pre_id ##_2, p, INT16_MAX);    \
    clk_set_ratio_limit(_pre_id ##_3, q, INT16_MAX);

#define SET_CLK_PLL_DEPENDS_AB(_pre_id, _depend)    \
    clk_set_depends_on(_pre_id, _depend);   \
    clk_set_depends_on(_pre_id ##_ROOT, _depend);   \
    clk_set_depends_on(_pre_id ##_DIVA, _depend);   \
    clk_set_depends_on(_pre_id ##_DIVB, _depend);


#include "clk/res_pll.h"
#include "clk/res_ckgen.h"
static struct clk init_clks_table[] = {
    /*XTAL*/
    CLK_ITEM_FIXRATE(CLK_ID_RC_24M, "RC_24M", 24000000),
    CLK_ITEM_FIXRATE(CLK_ID_RC_RTC, "RC_RTC", 2000000),
    CLK_ITEM_FIXRATE(CLK_ID_XTAL_24M, "XTAL_24M", 24000000),
    CLK_ITEM_FIXRATE(CLK_ID_XTAL_RTC, "XTAL_RTC", 32000),
#if 0
    /*ext aud*/
    CLK_ITEM_FIXRATE(CLK_ID_EXT_AUD1, "EXT_AUD1", 207900000),
    CLK_ITEM_FIXRATE(CLK_ID_EXT_AUD2, "EXT_AUD2", 207900000),
    CLK_ITEM_FIXRATE(CLK_ID_EXT_AUD3, "EXT_AUD3", 207900000),
    CLK_ITEM_FIXRATE(CLK_ID_EXT_AUD4, "EXT_AUD4", 207900000),
#else
// will come from disp
#endif
    /*PLL*/
    PLL_RES_ITEMS
#if MODULE_HELPER_CKGEN_SEC
    /*CKGEN sec*/
    CKGEN_SEC_ITEMS
#endif
#if MODULE_HELPER_CKGEN_DISP
    /*CKGEN disp*/
    CKGEN_DISP_ITEMS
#endif
#if MODULE_HELPER_CKGEN_SAF
    /*CKGEN saf*/
    CKGEN_SAF_ITEMS
#endif
#if MODULE_HELPER_CKGEN_SOC
    /*CKGEN soc*/
    CKGEN_SOC_ITEMS
#endif
#if MODULE_HELPER_CKGEN_UUU
    /*CKGEN uuu*/
    CKGEN_UUU_ITEMS
#endif
};

static inline bool is_rc(struct clk *clk)
{
    if (clk->clkid >= CLK_ID_FIXRATE_FIRST
            && clk->clkid <= CLK_ID_FIXRATE_LAST) {
        return true;
    }

    return false;
}

static inline bool is_pll(struct clk *clk)
{
    if (clk->clkid >= CLK_ID_PLL_FIRST && clk->clkid <= CLK_ID_PLL_LAST) {
        return true;
    }

    return false;
}
static inline bool is_ckgen_clk(struct clk *clk)
{
    if (clk->clkid >= CLK_ID_CKGEN_FIRST && clk->clkid <= CLK_ID_CKGEN_LAST) {
        return true;
    }

    return false;
}
static inline bool is_ckgen_bus(struct clk *clk)
{
    if (clk->clkid >= CLK_ID_CKGEN_BUS_FIRST
            && clk->clkid <= CLK_ID_CKGEN_BUS_LAST) {
        return true;
    }

    return false;
}
static inline bool is_ckgen_core(struct clk *clk)
{
    if (clk->clkid >= CLK_ID_CKGEN_CORE_FIRST
            && clk->clkid <= CLK_ID_CKGEN_CORE_LAST) {
        return true;
    }

    return false;
}

static inline bool is_ckgen_ip(struct clk *clk)
{
    if (clk->clkid >= CLK_ID_CKGEN_IP_FIRST
            && clk->clkid <= CLK_ID_CKGEN_IP_LAST) {
        return true;
    }

    return false;
}

static inline bool is_ckgen_uuu(struct clk *clk)
{
    if (clk->clkid >= CLK_ID_CKGEN_UUU_FIRST
            && clk->clkid <= CLK_ID_CKGEN_UUU_LAST) {
        return true;
    }

    return false;
}
static bool is_res_belong_this_domain(uint32_t resid)
{
    int ret = 0;
    paddr_t phy_addr;
    int32_t slice_idx = -1;
    ret = res_get_info_by_id(resid, &phy_addr, &slice_idx);

    if (ret == -1) {
        CLK_TRACE(DBGV, "res_glb_idx:0x%x is not belong this domain\n", resid);
        return false;
    }

    return true;
}
static int clk_is_enable(struct clk *clk);
static bool is_clk_can_access(struct clk *clk)
{
    if (!is_pll(clk)) {
        return true;
    }

    if (!clk->pll.dependson || clk_is_hw_enable(clk->pll.dependson)) {
        return true;
    }

    return false;
}

//internal for pll & ckgen
static bool clk_is_hw_enable(struct clk *clk)
{
    if (is_pll(clk)) {
        return clk->enable_cnt > clk->disable_cnt;
    }

    if (is_ckgen_bus(clk)) {
        return get_ckgen_bus_endis(clk->resid);
    }
    else if (is_ckgen_core(clk)) {
        return get_ckgen_core_endis(clk->resid);
    }
    else if (is_ckgen_ip(clk)) {
        return get_ckgen_ip_endis(clk->resid);
    }
    else if (is_ckgen_uuu(clk)) {
        return get_ckgen_uuu_endis(clk->resid, clk->uuu.uuu_type);
    }
    else {
        return true;
    }

    return false;
}

static int clk_is_enable(struct clk *clk)
{
    if (is_pll(clk)) {
        return clk->enable_cnt > clk->disable_cnt;
    }

    if (is_ckgen_bus(clk)) {
        return clk->enable_cnt > clk->disable_cnt;
    }
    else if (is_ckgen_core(clk)) {
        return clk->enable_cnt > clk->disable_cnt;
    }
    else if (is_ckgen_ip(clk)) {
        return clk->enable_cnt > clk->disable_cnt;
    }
    else if (is_ckgen_uuu(clk)) {
        return clk->enable_cnt > clk->disable_cnt;
    }
    else {
        ASSERT(0);
    }

    return 0;
}
static int clk_endis(struct clk *clk, bool isenable)
{
    int ret = 0;

    if (!is_clk_can_access(clk)) {
        return 0;
    }

    if (is_pll(clk)) {
        ret = set_pll_endis(clk->resid, clk->pll.plldiv, isenable);
        goto out;
    }

    if (is_ckgen_clk(clk) && (clk->gate_resid != 0)) {
        ret = set_ckgen_gate_endis(clk->gate_resid, isenable);
        goto out;
    }

    if (is_ckgen_bus(clk)) {
        ret = set_ckgen_bus_endis(clk->resid, isenable);
    }
    else if (is_ckgen_core(clk)) {
        ret = set_ckgen_core_endis(clk->resid, isenable);
    }
    else if (is_ckgen_ip(clk)) {
        ret = set_ckgen_ip_endis(clk->resid, isenable);
    }
    else if (is_ckgen_uuu(clk)) {
        ret = set_ckgen_uuu_endis(clk->resid, isenable);
    }
    else {
        ASSERT(0);
    }

out:

    if (!ret) {
        isenable ? clk->enable_cnt++ : clk->disable_cnt++;
    }

    return ret;
}

static int clk_enable(struct clk *clk)
{
    return clk_endis(clk, true);
}
static int clk_disable(struct clk *clk)
{
    return clk_endis(clk, false);
}
static int clk_set_rate(struct clk *clk, unsigned long prate,
                        unsigned long freq)
{
    if (!is_clk_can_access(clk)) {
        return 0;
    }

    if (is_pll(clk)) {
        return set_pll_rate(clk, prate, freq);
    }

    if (is_ckgen_bus(clk)) {
        return set_ckgen_bus_rate(clk->resid, prate, freq);
    }
    else if (is_ckgen_core(clk)) {
        return set_ckgen_core_rate(clk->resid, prate, freq);
    }
    else if (is_ckgen_ip(clk)) {
        return set_ckgen_ip_rate(clk->resid, prate, freq);
    }
    else if (is_ckgen_uuu(clk)) {
        return set_ckgen_uuu_rate(clk->resid, clk->uuu.uuu_type, prate, freq);
    }
    else {
        ASSERT(0);
    }

    return 0;
}

static unsigned long clk_get_rate(struct clk *clk, unsigned long prate,
                                  bool bymonitor)
{
    if (!is_clk_can_access(clk)) {
        return 0;
    }

    if (is_pll(clk)) {
        unsigned long rate ;
        rate = get_pll_rate(clk, prate, bymonitor);

        if (rate == 0) { //may be not belong this domain, so return fake rate
            rate = clk->rate;
        }

        return rate;
    }
    else if (is_ckgen_bus(clk)) {
        return get_ckgen_bus_rate(clk->resid, prate, bymonitor);
    }
    else if (is_ckgen_core(clk)) {
        return get_ckgen_core_rate(clk->resid, prate, bymonitor);
    }
    else if (is_ckgen_ip(clk)) {
        return get_ckgen_ip_rate(clk->resid, prate, bymonitor);
    }
    else if (is_ckgen_uuu(clk)) {
        return get_ckgen_uuu_rate(clk->resid, clk->uuu.uuu_type, prate, bymonitor);
    }
    else {
        ASSERT(0);
    }

    return 0;
}
static unsigned long clk_round_rate(struct clk *clk, int pindex,
                                    unsigned long *prate, unsigned long freq)
{
    if (!is_clk_can_access(clk)) {
        return 0;
    }

    if (is_pll(clk)) {
        if (dont_change_pll) {
            return get_pll_rate(clk, *prate, false);
        }
        else {
            return get_pll_round_rate_clk(clk, pindex, prate, freq);
        }
    }
    else if (is_ckgen_bus(clk)) {
        return get_ckgen_bus_ip_round_rate_clk(clk, pindex, prate, freq);
    }
    else if (is_ckgen_core(clk)) {
        return get_ckgen_core_round_rate_clk(clk, pindex, prate, freq);
    }
    else if (is_ckgen_ip(clk)) {
        return get_ckgen_bus_ip_round_rate_clk(clk, pindex, prate, freq);
    }
    else if (is_ckgen_uuu(clk)) {
        return get_ckgen_uuu_round_rate_clk(clk, pindex, prate, freq);
    }
    else {
        ASSERT(0);
    }

    return 0;
}
static int clk_set_parent(struct clk *clk, int parent_index)
{
    if (!is_clk_can_access(clk)) {
        return 0;
    }

    if (is_pll(clk)) {
        CLK_TRACE(CRITICAL, "pll %s no parent or fixed parent\n", clk->name);
        ASSERT(0);
        return -1;
    }

    if (is_ckgen_bus(clk)) {
        return set_ckgen_bus_parent(clk->resid, parent_index);
    }
    else if (is_ckgen_core(clk)) {
        return set_ckgen_core_parent(clk->resid, parent_index);
    }
    else if (is_ckgen_ip(clk)) {
        return set_ckgen_ip_parent(clk->resid, parent_index);
    }
    else if (is_ckgen_uuu(clk)) {
        return set_ckgen_uuu_parent(clk->resid, clk->uuu.uuu_type, parent_index);
    }
    else {
        ASSERT(0);
    }

    return 0;
}
static int clk_get_current_parent_from_hw(struct clk *clk)
{
    if (!is_clk_can_access(clk)) {
        return 0;
    }

    if (is_ckgen_bus(clk)) {
        return get_ckgen_bus_parent(clk->resid);
    }
    else if (is_ckgen_core(clk)) {
        return get_ckgen_core_parent(clk->resid);
    }
    else if (is_ckgen_ip(clk)) {
        return get_ckgen_ip_parent(clk->resid);
    }
    else if (is_ckgen_uuu(clk)) {
        return get_ckgen_uuu_parent(clk->resid, clk->uuu.uuu_type);
    }
    else {
        ASSERT(0);
    }

    return -1;
}

//
static struct clk *get_clk_by_id_internal(struct list_node *list,
        unsigned long clkid)
{
    struct clk *clk;

    if (!list) {
        return get_clk_by_id_internal(&g_clk_root, clkid);
    }

    if (list_is_empty(list)) {
        return NULL;
    }

    list_for_every_entry(list, clk, struct clk, node) {
        if (clk->clkid == clkid) {
            return clk;
        }

        if (list_is_empty(&clk->child)) {
            continue;
        }

        struct clk *child = get_clk_by_id_internal(&clk->child, clkid);

        if (child) {
            return child;
        }
    }
    return NULL;
}
#ifdef RES_CLK_DEBUG
static struct clk *get_clk_by_name_internal(struct list_node *list,
        const char *str)
{
    struct clk *clk;

    if (!list) {
        return get_clk_by_name_internal(&g_clk_root, str);
    }

    if (list_is_empty(list)) {
        return NULL;
    }

    list_for_every_entry(list, clk, struct clk, node) {
        if (strcmp(clk->name, str) == 0) {
            return clk;
        }

        if (list_is_empty(&clk->child)) {
            continue;
        }

        struct clk *child = get_clk_by_name_internal(&clk->child, str);

        if (child) {
            return child;
        }
    }
    return NULL;
}

static struct clk *get_clk_by_name(const char *str)
{
    return get_clk_by_name_internal(NULL, str);
}
#endif
static struct clk *get_clk_by_id_from_array(unsigned long clkid)
{
    int i;
    int num = ARRAYSIZE(init_clks_table);

    for (i = 0; i < num; i++) {
        if (init_clks_table[i].clkid == clkid) {
            return &init_clks_table[i];
        }
    }

    return NULL;
}

static struct clk *get_clk_by_id(unsigned long clkid)
{
    return get_clk_by_id_internal(NULL, clkid);
}

static int clk_set_ratio_limit(unsigned long clkid, int mindiv, int maxdiv)
{
    struct clk *clk = get_clk_by_id(clkid);

    if (!clk) {
        return -1;
    }

    clk->mindiv = mindiv;
    clk->maxdiv = maxdiv;
    return 0;
}

static int clk_set_depends_on(unsigned long clkid, unsigned long depends)
{
    struct clk *clk = get_clk_by_id(clkid);
    struct clk *dp = get_clk_by_id(depends);

    if (!clk || !dp) {
        return -1;
    }

    ASSERT(is_pll(clk));
    clk->pll.dependson = dp;
    return 0;
}

static int clk_set_moreprecise(unsigned long clkid, bool need,
                               uint32_t frac)
{
    struct clk *clk = get_clk_by_id(clkid);

    if (!clk) {
        return -1;
    }

    ASSERT(is_pll(clk) && clk->pll.plldiv == PLL_DUMMY_ROOT);
    clk->pll.moreprecise = need;

    if (need) {
        if (frac == 0) {
            clk->pll.config.integer = true;
        }
        else {
            clk->pll.config.integer = false;
        }

        clk->pll.config.frac = frac;
    }

    return 0;
}

static int clk_init_parents_from_table(struct clk *clk)
{
    int *table = clk->ptable;

    if (is_pll(clk)) {
        if (clk->pll.plldiv == PLL_DUMMY_ROOT) {
            clk->cur_parent_index = -1; //root clk
            clk->parent_nums = 0;
        }
        else {
            clk->cur_parent_index = 0;
            clk->parent_nums = 1;
            clk->parents[0] = get_clk_by_id_from_array((unsigned long)table);
        }
    }
    else if (!clk->ptable_size) {
        clk->cur_parent_index = -1; //root clk
        clk->parent_nums = 0;
    }
    else {
        int i;

        for (i = 0; i < clk->ptable_size; i++) {
            clk->parents[i] = get_clk_by_id_from_array((unsigned long)table[i]);
        }

        clk->parent_nums = clk->ptable_size;
        clk->cur_parent_index = 0;
    }

    return 0;
}

static int clk_init_refcount_bitmap(struct clk *clk)
{
    int i;

    for (i = 0; i < REFCNT_NUM; i++) {
        clk->refcount[i] = 0;
        clk->refcount_en[i] = 0;
    }

    return 0;
}
static int clk_get_refcount_bits(struct clk *clk, bool freqcare)
{
    int i;
    uint32_t n;
    int count = 0;
    uint32_t *ref = NULL;

    if (freqcare) {
        ref = &clk->refcount[0];
    }
    else {
        ref = &clk->refcount_en[0];
    }

    for (i = 0; i < REFCNT_NUM; i++) {
        int j;
        n = ref[i];

        for (j = 0; j < 32; j++) {
            if ((1 << j)&n) {
                count++;
            }
        }
    }

    return count;
}

static int clk_set_refcount_bit(struct clk *clk, int per_id, bool freqcare)
{
    int i = per_id / 32;
    uint32_t *ref = NULL;

    if (freqcare) {
        ref = &clk->refcount[0];
    }
    else {
        ref = &clk->refcount_en[0];
    }

    ASSERT(i < REFCNT_NUM);
    ref[i] |= 1 << (per_id % 32);
    return 0;
}

static int clk_clear_refcount_bit(struct clk *clk, int per_id,
                                  bool freqcare)
{
    int i = per_id / 32;
    uint32_t *ref = NULL;

    if (freqcare) {
        ref = &clk->refcount[0];
    }
    else {
        ref = &clk->refcount_en[0];
    }

    ASSERT(i < REFCNT_NUM);
    ref[i] &= ~(1 << (per_id % 32));
    return 0;
}

static int clk_test_refcount_bit(struct clk *clk, int per_id,
                                 bool freqcare)
{
    int i = per_id / 32;
    uint32_t *ref = NULL;

    if (freqcare) {
        ref = &clk->refcount[0];
    }
    else {
        ref = &clk->refcount_en[0];
    }

    ASSERT(i < REFCNT_NUM);
    return (ref[i] & (1 << (per_id % 32)));
}
static int clk_get_refcount_by_perid_level1(struct clk *clk, int per_id,
        bool freqcare)
{
    int count = 0;
    //count refcount bitmap
    count += clk_test_refcount_bit(clk, per_id, freqcare);

    //count refcount en bitmap
    if (!freqcare) {
        count += clk_test_refcount_bit(clk, per_id, false);
    }

    return count;
}

static int clk_get_refcount_by_perid(struct clk *clk, int per_id,
                                     bool freqcare)
{
    int count = 0;
    struct clk *child = NULL;
    //count refcount bitmap
    count += clk_test_refcount_bit(clk, per_id, freqcare);

    //count refcount en bitmap
    if (!freqcare) {
        count += clk_test_refcount_bit(clk, per_id, false);
    }

    //cound child
    if (list_is_empty(&clk->child)) {
        return count;
    }

    list_for_every_entry(&clk->child, child, struct clk, node) {
        count += clk_get_refcount_by_perid(child, per_id, freqcare);
    }
    return count;
}


static int clk_get_refcount(struct clk *clk, bool freqcare)
{
    int count = 0;
    struct clk *child = NULL;
    //count refcount bitmap
    count += clk_get_refcount_bits(clk, true);

    //count refcount en bitmap
    if (!freqcare) {
        count += clk_get_refcount_bits(clk, false);
    }

    //cound child
    if (list_is_empty(&clk->child)) {
        return count;
    }

    list_for_every_entry(&clk->child, child, struct clk, node) {
        count += clk_get_refcount(child, freqcare);
    }
    return count;
}


static bool res_clk_is_valid_request(struct clk *clk, int pindex,
                                     unsigned long rate, unsigned long prate)
{
    struct clk *p = NULL;

    if (pindex != -1) {
        p = clk->parents[pindex];
    }

    if (rate != res_clk_get_rate(clk, false)) {
        int refcnt = clk_get_refcount(clk, true);

        if (refcnt > 1) {
            return false;
        }

        if (refcnt == 1 && clk_get_refcount_by_perid(clk, cur_per_id, true) == 0) {
            return false;
        }

        if (cur_clkid != clk->clkid
                && clk_get_refcount_by_perid_level1(clk, cur_per_id, true) == 1) {
            return false;
        }
    }

    if (p && prate != res_clk_get_rate(p, false)) {
        int rfcnt = clk_get_refcount(p, true);

        if (rfcnt > 1) {
            return false;
        }
        else if (rfcnt == 1
                 && ((clk->cur_parent_index != pindex)
                     || clk_get_refcount_by_perid(p, cur_per_id, true) == 0)) {
            return false;
        }
        else if (rfcnt == 1 && (clk->cur_parent_index == pindex)
                 && clk_get_refcount(clk, true) == 0) {
            return false;
        }
    }

    return true;
}

static bool res_clk_is_valid_round_rate(struct clk *clk, int pindex,
                                        unsigned long rate, unsigned long prate)
{
    if (round_rate_filter_en) {
        return res_clk_is_valid_request(clk, pindex, rate, prate);
    }
    else {
        return true;
    }
}

static unsigned long res_clk_get_rate(struct clk *clk, bool bymonitor)
{
    struct clk *p = NULL;
    unsigned long prate;

    if (clk->cur_parent_index == -1) {
        if (clk->get_rate) {
            return clk->get_rate(clk, 0, bymonitor);
        }
        else {
            return clk->rate;
        }
    }

    p = clk->parents[clk->cur_parent_index];
    prate = res_clk_get_rate(p, bymonitor);
    return clk->get_rate(clk, prate, bymonitor);
}

unsigned long res_clk_get_rate_by_id(enum clk_id clkid, bool bymonitor)
{
    struct clk *clk = get_clk_by_id(clkid);

    if (clk) {
        return res_clk_get_rate(clk, bymonitor);
    }
    else {
        return 0;
    }
}

static unsigned long res_clk_div_round_rate(struct clk *clk, struct clk *p,
        int div_i, unsigned long *prate, unsigned long freq)
{
    unsigned long now;
    unsigned long parent_rate_saved = *prate;
    struct clk_request req;

    if (freq * div_i == parent_rate_saved) {
        *prate = parent_rate_saved;
        return freq;
    }

    req.request = freq * div_i;
    *prate = res_clk_round_rate(p, &req);
    now = DIV_ROUND_UP(*prate, div_i);
    return now;
}

static unsigned long res_clk_round_rate(struct clk *clk,
                                        struct clk_request *req)
{
    int i;
    struct clk *p = NULL;
    unsigned long prate = 0;
    unsigned long bestrate = 0, bestprate = 0;
    int best_parent_index = 0;
    unsigned long diffrate = UINT32_MAX;

    if (!clk->parent_nums) {
        req->best_parent_index = clk->cur_parent_index;

        if (clk->round_rate) {
            req->best_prate = clk->round_rate(clk, -1, 0, req->request);
        }
        else {
            req->best_prate = res_clk_get_rate(clk, false);
        }

        return req->best_prate;
    }

    bestrate = res_clk_get_rate(clk, false);
    best_parent_index = clk->cur_parent_index;
    bestprate = res_clk_get_rate(clk->parents[clk->cur_parent_index], false);
    //prefer to rount rate from current parent.
    {
        unsigned long diff;
        unsigned long rate;
        int i = clk->cur_parent_index;
        p = clk->parents[i];
        prate = res_clk_get_rate(p, false);

        if (clk->round_rate) {
            rate = clk->round_rate(clk, i, &prate, req->request);
        }
        else {
            rate = res_clk_get_rate(clk, false);
        }

        if (!res_clk_is_valid_round_rate(clk, i, rate, prate)) { goto notvalid; }

        diff = abs_clk(rate, req->request);
        //dprintf(CRITICAL,"current clk %s req %lu get %lu  prate %lu pindex %d diff %lu\n",clk->name, req->request, rate, prate, i, diff);

        if (diff < diffrate) {
            diffrate = diff;
            bestrate = rate;
            best_parent_index = i;
            bestprate = prate;

            if (diff == 0) {
                goto found;
            }
        }
    }
notvalid:

    for (i = 0; i < clk->parent_nums; i++) {
        unsigned long diff;
        unsigned long rate;

        if (i == clk->cur_parent_index) { continue; }

        p = clk->parents[i];
        prate = res_clk_get_rate(p, false);

        if (clk->round_rate) {
            rate = clk->round_rate(clk, i, &prate, req->request);
        }
        else {
            rate = res_clk_get_rate(clk, false);
        }

        if (!res_clk_is_valid_round_rate(clk, i, rate, prate)) { continue; }

        diff = abs_clk(rate, req->request);
        //dprintf(CRITICAL,"p %d clk %s req %lu get %lu  prate %lu pindex %d diff %lu\n", i,clk->name, req->request, rate,prate, i, diff);

        if (diff < diffrate) {
            diffrate = diff;
            bestrate = rate;
            best_parent_index = i;
            bestprate = prate;

            if (diff == 0) {
                break;
            }
        }
    }

found:
    req->best_prate = bestprate;
    req->best_parent_index = best_parent_index;
    //dprintf(CRITICAL, "clk %s req %lu get %lu  bestprate %lu pindex %d\n",clk->name, req->request, bestrate,bestprate, best_parent_index);
    return bestrate;
}

/* child must call this function after add itself refcount,
*  and must change from 0 ->1.
*/
static int res_clk_inc_refcount(struct clk *clk, unsigned long per_id,
                                bool freqcare)
{
    int new_refcount;
    int ret = 0;

    if (per_id != INVALID_PER_ID) {
        clk_clear_refcount_bit(clk, per_id, !freqcare);
        clk_set_refcount_bit(clk, per_id, freqcare);
    }

    new_refcount = clk_get_refcount(clk, false);

    if (new_refcount) {
        bool is_enabled = false;

        if (clk->cur_parent_index != -1) {
            ret = res_clk_inc_refcount(clk->parents[clk->cur_parent_index],
                                       INVALID_PER_ID, freqcare);
        }

        if (clk->is_enable) {
            is_enabled = clk->is_enable(clk);
        }

        if (!is_enabled && clk->enable) {
            ret |= clk->enable(clk);
        }
    }

    return ret;
}
static int res_clk_dec_refcount(struct clk *clk, unsigned long per_id)
{
    int new_refcount;
    int ret = 0;

    if (per_id != INVALID_PER_ID) {
        clk_clear_refcount_bit(clk, per_id, true);
        clk_clear_refcount_bit(clk, per_id, false);
    }

    new_refcount = clk_get_refcount(clk, false);

    if (new_refcount == 0) {
        bool is_enabled = true;

        if (clk->is_enable) {
            is_enabled = clk->is_enable(clk);
        }

        if (is_enabled && clk->disable) {
            ret = clk->disable(clk);
        }

        if (clk->cur_parent_index != -1) {
            ret |= res_clk_dec_refcount(clk->parents[clk->cur_parent_index],
                                        INVALID_PER_ID);
        }
    }

    return ret;
}

static int res_clk_set_rate(struct clk *clk, enum module_per_id per_id,
                            unsigned long freq)
{
    int ret = 0;

    if (!freq) {
        return res_clk_dec_refcount(clk, per_id);
    }
    else if (freq == 1) {
        return res_clk_inc_refcount(clk, per_id, false);
    }
    else {
        unsigned long rate;
        struct clk_request req;
        int oldrefcnt;

        if (freq == res_clk_get_rate(clk, false)) {
            res_clk_inc_refcount(clk, per_id, true);
            return ret;
        }

        req.request = freq;
        rate = res_clk_round_rate(clk, &req);
        CLK_TRACE(DBGV, "%s req is %lu round rate %lu, best p %d, prate %lu\n",
                  clk->name, freq, rate, req.best_parent_index, req.best_prate);
        //may be can check if the round rate is match the requirement.
        oldrefcnt = clk_get_refcount(clk, false);

        if (!res_clk_is_valid_request(clk, req.best_parent_index, rate,
                                      req.best_prate)) { //rate != res_clk_get_rate(clk) && clk_get_refcount(clk) > 1)
            CLK_TRACE(CRITICAL,
                      "clk %s refcount is >1, don't suggest to change the freq from %lu to %lu\n",
                      clk->name, res_clk_get_rate(clk, false), rate);

            if (!set_rate_overwrite_allow) {
                //res_clk_dec_refcount(clk, per_id);
                ASSERT(0);
                return -1;
            }
        }

        //request the same freq
        if (rate == res_clk_get_rate(clk, false)) {
            res_clk_inc_refcount(clk, per_id, true);
            return ret;
        }

        //allowed to change rate and parent
        if (req.best_parent_index != -1
                && req.best_parent_index != clk->cur_parent_index) {
            struct clk *new_parent = clk->parents[req.best_parent_index];
            int oldpindex = clk->cur_parent_index;
            //delete node from old parent.
            list_delete(&clk->node);
            //add node to new parent.
            list_add_head(&(new_parent->child), &clk->node);
            res_clk_inc_refcount(new_parent, INVALID_PER_ID,
                                 false);

            if (req.best_prate != res_clk_get_rate(new_parent,
                                                   false)) {
                ret |= res_clk_set_rate(new_parent,
                                        INVALID_PER_ID, req.best_prate);
            }

            if (clk->set_rate) {
                ret |= clk->set_rate(clk, req.best_prate, rate);
            }
            else {
                ASSERT(0);
            }

            ret |= clk->set_parent(clk, req.best_parent_index);
            clk->cur_parent_index = req.best_parent_index;

            //change parent, need dec refcount from old parent.

            if (oldrefcnt != 0) {
                res_clk_dec_refcount(clk->parents[oldpindex], INVALID_PER_ID);
            }
        }
        else if (clk->cur_parent_index != -1
                 && req.best_prate != res_clk_get_rate(clk->parents[clk->cur_parent_index],
                         false)) {//only need change parent rate
            ret |= res_clk_set_rate(clk->parents[clk->cur_parent_index],
                                    INVALID_PER_ID, req.best_prate);

            if (clk->set_rate) {
                ret |= clk->set_rate(clk, req.best_prate, rate);
            }
            else {
                ASSERT(0);
            }
        }
        else {
            if (clk->set_rate) {
                ret |= clk->set_rate(clk, req.best_prate, rate);
            }
            else {
                ASSERT(0);
            }
        }

        res_clk_inc_refcount(clk, per_id, true);
    }

    return ret;
}

int res_clk_request(enum module_per_id per_id, enum clk_id clkid,
                    unsigned long param)
{
    struct clk *clk=NULL;
    unsigned long freq = param;
    int ret;

    module_helper_init();
    clk = get_clk_by_id(clkid);
    //TODO:can check if this clk allowed to be accessed by user
    CLK_TRACE(DBGV, "req from %d : clk %p id %d param %lu\n", per_id, clk,
              clkid, param);

    if (!clk) {
        CLK_TRACE(CRITICAL, "no such clk %d\n", clkid);
        return -1;
    }

    spin_lock_irqsave(&clk->lock, clk->lockstate);
    cur_per_id = per_id;
    cur_clkid = clkid;
    ret = res_clk_set_rate(clk, per_id, freq);
    spin_unlock_irqrestore(&clk->lock, clk->lockstate);
    return ret;
}
//TODO

static int config_dummy_pll(int clkid, unsigned long rate)
{
    struct clk *clk = get_clk_by_id(clkid);

    if (!clk) { return -1; }

    clk->rate = rate;
    clk->round_rate = NULL;
    return 0;
}


void register_res_clks(void)
{
    int i;
    int num = ARRAYSIZE(init_clks_table);
    list_initialize(&g_clk_root);
    struct clk *clk;

    for (i = 0; i < num; i++) {
        clk = &init_clks_table[i];
        list_clear_node(&clk->node);
        list_initialize(&clk->child);
        //init spin lock
        spin_lock_init(&clk->lock);
        //init refcount
        clk_init_refcount_bitmap(clk);
        //init parents from table
        clk_init_parents_from_table(clk);

        //init ops
        if (is_rc(clk)) {
        }
        else if (is_pll(clk)) {
            clk->pll.dependson = NULL;
            clk->is_enable = clk_is_enable;
            clk->enable = clk_enable;
            clk->disable = clk_disable;
            clk->set_rate = clk_set_rate;
            clk->get_rate = clk_get_rate;
            clk->round_rate = clk_round_rate;
            clk->set_parent = clk_set_parent;
        }
        else if (is_ckgen_clk(clk)) {//ckgen
            clk->is_enable = clk_is_enable;
            clk->enable = clk_enable;
            clk->disable = clk_disable;
            clk->set_rate = clk_set_rate;
            clk->get_rate = clk_get_rate;
            clk->round_rate = clk_round_rate;
            clk->set_parent = clk_set_parent;
        }
        else {
            ASSERT(0);
        }

        //mindiv maxdiv limit
        clk->mindiv = 1;
        clk->maxdiv = INT16_MAX;
        //freq limit
        //clk->minfreq =0;
        //clk->maxfreq =UINT32_MAX;

        //TODO , if not belong this domain, will using dummy rate
        if (!is_rc(clk) && !is_res_belong_this_domain(clk->resid)) {
            if (clk->clkid == CLK_ID_PLL1 || clk->clkid == CLK_ID_PLL1_ROOT
                    || clk->clkid == CLK_ID_PLL1_DIVA || clk->clkid == CLK_ID_PLL1_DIVB
                    || clk->clkid == CLK_ID_PLL1_DIVC || clk->clkid == CLK_ID_PLL1_DIVD
               ) {
                clk->rate = 0;
                clk->round_rate = NULL;
            }
            else if (clk->clkid == CLK_ID_PLL2 || clk->clkid == CLK_ID_PLL2_ROOT
                     || clk->clkid == CLK_ID_PLL2_DIVA || clk->clkid == CLK_ID_PLL2_DIVB
                     || clk->clkid == CLK_ID_PLL2_DIVC || clk->clkid == CLK_ID_PLL2_DIVD
                    ) {
                clk->rate = 0;
                clk->round_rate = NULL;
            }
            else {
                continue;
            }
        }

        list_add_tail(&g_clk_root, &clk->node);
        //init enable status
        clk->enable_cnt = clk_is_hw_enable(clk);
    }

    //init some special clk
    CLK_UUU_RATIO_LIMIT(CLK_ID_CPU1A, 1, 2, 4, 8)
    CLK_UUU_RATIO_LIMIT(CLK_ID_CPU1B, 1, 2, 4, 8)
    CLK_UUU_RATIO_LIMIT(CLK_ID_CPU2, 1, 2, 4, 8)
    CLK_UUU_RATIO_LIMIT(CLK_ID_GPU1, 1, 1, 4, 1)
    CLK_UUU_RATIO_LIMIT(CLK_ID_GPU2, 1, 1, 4, 1)
    CLK_UUU_RATIO_LIMIT(CLK_ID_VPU_BUS, 1, 4, 1, 1)
    CLK_UUU_RATIO_LIMIT(CLK_ID_VSN_BUS, 1, 4, 1, 1)
    CLK_UUU_RATIO_LIMIT(CLK_ID_DDR, 1, 2, 4, 1)
    CLK_UUU_RATIO_LIMIT(CLK_ID_HIS_BUS, 1, 10, 2, 4)
    //init dependson
    SET_CLK_PLL_DEPENDS_AB(CLK_ID_PLL_CPU1A, CLK_ID_CPU1A_2)
    SET_CLK_PLL_DEPENDS_AB(CLK_ID_PLL_CPU1B, CLK_ID_CPU1A_2)
    SET_CLK_PLL_DEPENDS_AB(CLK_ID_PLL_CPU2, CLK_ID_CPU2_2)
    SET_CLK_PLL_DEPENDS_AB(CLK_ID_PLL_GPU1, CLK_ID_GPU1_2)
    SET_CLK_PLL_DEPENDS_AB(CLK_ID_PLL_GPU2, CLK_ID_GPU2_2)
    SET_CLK_PLL_DEPENDS_AB(CLK_ID_PLL_VPU, CLK_ID_VPU_BUS_1)
    SET_CLK_PLL_DEPENDS_AB(CLK_ID_PLL_DDR, CLK_ID_DDR_2)
    SET_CLK_PLL_DEPENDS_AB(CLK_ID_PLL_VSN, CLK_ID_VSN_BUS_1)
    //init some special pll
    clk_set_moreprecise(CLK_ID_PLL_DDR, true, 0);
    clk_set_moreprecise(CLK_ID_PLL_DISP, true, 0);
    clk_set_moreprecise(CLK_ID_PLL_LVDS1, true, 0);
    clk_set_moreprecise(CLK_ID_PLL_LVDS2, true, 0);
    clk_set_moreprecise(CLK_ID_PLL_LVDS3, true, 0);
    clk_set_moreprecise(CLK_ID_PLL_LVDS4, true, 0);

    for (i = 0; i < num; i++) {
        clk = get_clk_by_id(init_clks_table[i].clkid);

        if (!clk) { continue; }

        //get current parent index from hw
        if (clk->parent_nums > 1) {
            clk->cur_parent_index = clk_get_current_parent_from_hw(clk);

            if (clk->cur_parent_index == -1) { //not belong this domain
                CLK_TRACE(DBGV, " remove %s because it doesn't belong this domain\n",
                          clk->name);
                list_delete(&clk->node);
                continue;
            }
        }
        else if (clk->parent_nums == 1) {
            clk->cur_parent_index = 0;
        }
        else {
            clk->cur_parent_index = -1;
        }

        // add to global list or child of parent
        if (clk->cur_parent_index != -1) {
            struct clk *p = clk->parents[clk->cur_parent_index];
            list_delete(&clk->node);
            list_add_tail(&(p->child), &clk->node);
        }
    }
}
#ifdef RES_CLK_DEBUG
//debug code
static void print_prefix(int depth, const char *s)
{
    int i;

    for (i = 0; i < depth; i++) {
        dprintf(CRITICAL, "%s", s);
    }
}

void dump_clktree_internal(struct clk *clk, int depth, const int maxdepth,
                           bool bymonitor)
{
    int i;

    if (!clk) { //dump whole clk tree
        struct clk *child = NULL;

        if (list_is_empty(&g_clk_root)) {
            return;
        }

        list_for_every_entry(&g_clk_root, child, struct clk, node) {
            dump_clktree_internal(child, depth, maxdepth, bymonitor);
        }
        return ;
    }

    int refcount = clk_get_refcount(clk, false);
    unsigned long rate = res_clk_get_rate(clk, bymonitor);
    //format dump
    print_prefix(depth, "\t");
    //content
    dprintf(CRITICAL, "%s", clk->name);

    if ((maxdepth - depth) > (int)strlen(clk->name) / 8) {
        print_prefix(maxdepth - depth - strlen(clk->name) / 8, "\t");
    }

    dprintf(CRITICAL, "ref:%d\t en:%d\t dis:%d\t %lu\n", refcount,
            clk->enable_cnt, clk->disable_cnt, rate);

    //print peripheral reference
    for (i = 0; i < MAX_PER_ID; i++) {
        char perstr[100];

        if (clk_test_refcount_bit(clk, i, true)) {
            print_prefix(depth + 1, "\t");
            sprintf(perstr, "(rate)%s(%d)", module_get_per_name_by_id(i), i);
            dprintf(CRITICAL, "%s", perstr);

            if ((maxdepth - depth - 1) > (int)strlen(perstr) / 8) {
                print_prefix(maxdepth - depth - 1 - strlen(perstr) / 8, "\t");
            }

            dprintf(CRITICAL, "ref:1\t\n");
        }

        if (clk_test_refcount_bit(clk, i, false)) {
            print_prefix(depth + 1, "\t");
            sprintf(perstr, "(en)%s(%d)", module_get_per_name_by_id(i), i);
            dprintf(CRITICAL, "%s", perstr);

            if ((maxdepth - depth - 1) > (int)strlen(perstr) / 8) {
                print_prefix(maxdepth - depth - 1 - strlen(perstr) / 8, "\t");
            }

            dprintf(CRITICAL, "ref:1\t\n");
        }
    }

    //subtree
    if (!list_is_empty(&clk->child)) {
        struct clk *child = NULL;
        list_for_every_entry(&clk->child, child, struct clk, node) {
            dump_clktree_internal(child, depth + 1, maxdepth, bymonitor);
        }
    }
}
int calculate_maxdepths(struct clk *clk, int depth)
{
    int maxdepths = depth;
    int namedepth, perdepth = 0;
    int tmp;

    if (!clk) { //mean dump whole clk tree
        struct clk *child = NULL;

        if (list_is_empty(&g_clk_root)) {
            return 0;
        }

        list_for_every_entry(&g_clk_root, child, struct clk, node) {
            tmp = calculate_maxdepths(child, depth);

            if (tmp > maxdepths) {
                maxdepths = tmp;
            }
        }
        return maxdepths;
    }

    //name
    namedepth = strlen(clk->name) / 8 + 1;

    //perid
    if (clk_get_refcount_bits(clk, true) != 0
            || clk_get_refcount_bits(clk, false) != 0) {
        perdepth = 3; /*per str is need 2 indent*/
    }

    maxdepths = MAX(namedepth, perdepth);
    maxdepths += depth;

    //subtree
    if (!list_is_empty(&clk->child)) {
        struct clk *child = NULL;
        list_for_every_entry(&clk->child, child, struct clk, node) {
            tmp = calculate_maxdepths(child, depth + 1);

            if (tmp > maxdepths) {
                maxdepths = tmp;
            }
        }
    }

    return maxdepths;
}

void dump_clktree(struct clk *clk, bool bymonitor)
{
    int maxdepths = calculate_maxdepths(clk, 0);
    //printf("maxdepths %d\n", maxdepths);
    dprintf(CRITICAL, "get rate%s by monitor\n", bymonitor ? "" : " NOT");
    dump_clktree_internal(clk, 0, maxdepths, bymonitor);
}

#if defined(WITH_LIB_CONSOLE)
#include <lib/console.h>
int cmd_dumpclk(int argc, const cmd_args *argv)
{
    bool bymonitor = false;
    struct clk *clk = NULL;
    const char *name = NULL;

    if (argc == 2) {
        bymonitor = argv[1].b;
    }

    if (argc == 3) {
        name = argv[1].str;
        bymonitor = argv[2].b;
    }

    if (name) {
        clk = get_clk_by_name(name);
    }

    dump_clktree(clk, bymonitor);
    return 0;
}
int cmd_setclk(int argc, const cmd_args *argv)
{
    struct clk *clk = NULL;
    unsigned long per_id;

    if (argc != 4) {
        dprintf(CRITICAL, "change clock freq : perid name freq(hz)\n");
        return -1;
    }

    per_id = argv[1].u;
    clk = get_clk_by_name(argv[2].str);

    if (!clk) {
        dprintf(CRITICAL, "no such clk %s\n", argv[2].str);
        return -1;
    }

    return res_clk_request(per_id, clk->clkid, argv[3].u);
}

STATIC_COMMAND_START
STATIC_COMMAND("dumpclk", "dump clock tree: [name] [bymonitor]", (console_cmd)&cmd_dumpclk)
STATIC_COMMAND("setclk", "change clock freq : name freq(hz)", (console_cmd)&cmd_setclk)
STATIC_COMMAND_END(dumpclk);
#endif
#endif

