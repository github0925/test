/********************************************************
 *          Copyright(c) 2018   Semidrive               *
 *******************************************************/

#include <common_hdr.h>
#include <soc.h>
#include <arch.h>
#include "rstgen_reg.h"

void rg_module_reset(U32 base, U32 id, U8 val)
{
    U32 a = base + SOC_RSTGEN_REG_MAP(RSTGEN_MODULE_RST_OFF(id));
    U32 v = readl(a);
    U32 tms = 0;

    if (!(v & BM_RSTGEN_MODULE_RST_MODULE_RST_LOCK)) {
        writel(v | BM_RSTGEN_MODULE_RST_MODULE_RST_EN, a);

        while ((!(readl(a) & BM_RSTGEN_MODULE_RST_MODULE_RST_EN))
               && (tms++ < 1000));

        if (val) {
            v |= BM_RSTGEN_MODULE_RST_MODULE_RST_N;
        } else {
            v &= ~BM_RSTGEN_MODULE_RST_MODULE_RST_N;
        }

        writel(v | BM_RSTGEN_MODULE_RST_MODULE_RST_EN, a);
        tms = 0;

        if (val) {
            while ((!(readl(a) & BM_RSTGEN_MODULE_RST_MODULE_RST_STA))
                   && (tms++ < 1000));
        } else {
            while ((readl(a) & BM_RSTGEN_MODULE_RST_MODULE_RST_STA)
                   && (tms++ < 1000));
        }
    } else {
        DBG("%s: Opps, locked already\n", __FUNCTION__);
    }
}

U32 rg_rd_gpr(U32 base, U32 id)
{
    return readl(base + SOC_RSTGEN_REG_MAP(RSTGEN_GENERAL_REG_OFF(id)));
}

void rg_wr_gpr(U32 base, U32 id, U32 v)
{
    writel(v, base + SOC_RSTGEN_REG_MAP(RSTGEN_GENERAL_REG_OFF(id)));
}

U32 rg_get_reset_source(U32 base)
{
    U32 a = base + SOC_RSTGEN_REG_MAP(RSTGEN_RST_STA_OFF);

    return (readl(a) & 0xFFFFu);
}

static inline void
inline_rg_core_reset(U32 base, U32 id) __attribute__((always_inline));
static inline void inline_rg_core_reset(U32 base, U32 id)
{
    U32 r_en = base + SOC_RSTGEN_REG_MAP(RSTGEN_CORE_RST_EN_OFF(id));
    U32 r_rst = base + SOC_RSTGEN_REG_MAP(RSTGEN_CORE_SW_RST_OFF(id));

    U32 v = readl(r_en);

    if (!(v & BM_RSTGEN_CORE_RST_EN_CORE_RST_LOCK)) {
        /* de-assert RST_B before enabling RST */
        v |= BM_RSTGEN_CORE_SW_RST_STATIC_RST_B;
        writel(v, r_rst);

        /* 3 clks nedded for RST_B de-assertion being sync-ed into 24MHz
         * clock domain */
        while (!(readl(r_rst) & BM_RSTGEN_CORE_SW_RST_STATIC_RST_B_STA));

        v = readl(r_en);
        v |= BM_RSTGEN_CORE_RST_EN_SW_RST_EN;
        writel(v, r_en);

        while (!(readl(r_en) & BM_RSTGEN_CORE_RST_EN_SW_RST_EN_STA));

        v = readl(r_rst);
        v |= BM_RSTGEN_CORE_SW_RST_AUTO_CLR_RST;

#if (__ARM_ARCH == 7)
        /* Invalidate entire branch predictor array */
        /* The branch predictor maintenance operations must be used to invalidate entries in the branch predictor after any of the following events:
           enabling or disabling the MMU
           writing new data to instruction locations
           writing new mappings to the translation tables
           changes to the TTBR0, TTBR1, or TTBCR registers, unless accompanied by a change to the ContextID or the FCSE ProcessID.*/
        __asm__ volatile("mcr p15 , 0 , %0, c7 , c5 , 6" :: "r" (0): "memory");
        __asm__ volatile("isb" ::: "memory");
#endif
        writel(v, r_rst);

        while (!(readl(r_rst) & BM_RSTGEN_CORE_SW_RST_CORE_RST_STA));
    }
}

void ramfunc_rg_core_reset(U32 base, U32 id) __RAM_FUNC__;
void ramfunc_rg_core_reset(U32 base, U32 id)
{
    inline_rg_core_reset(base, id);
}

void rg_core_reset(U32 base, U32 id)
{
    inline_rg_core_reset(base, id);
}

U32 rg_glb_reset_en(U32 base, U32 msk)
{
    U32 a = base + SOC_RSTGEN_REG_MAP(RSTGEN_GLOBAL_RST_EN_OFF(0));
    U32 t = readl(a);

    if ((t & msk) == msk) {
        return 0;
    } else if (!(t & (0x01u << 31))) {
        t |= msk;
        writel(t, a);
        return 0;
    } else {
        return -1;
    }
}

#if defined(CFG_DRV_RSTGEN_API_rg_glb_reset_dis_)
U32 rg_glb_reset_dis(U32 base, U32 msk)
{
    U32 a = base + SOC_RSTGEN_REG_MAP(RSTGEN_GLOBAL_RST_EN_OFF(0));
    U32 t = readl(a);

    if ((t & msk) == 0) {
        return 0;
    } else if (!(t & (0x01u << 31))) {
        t &= ~msk;
        writel(t, a);
        return 0;
    } else {
        DBG("%s: failed\n", __FUNCTION__);
        return -1;
    }
}
#endif

void rg_glb_self_reset(U32 base, U8 v)
{
    U32 a = base + SOC_RSTGEN_REG_MAP(RSTGEN_SW_SELF_RST_OFF);
    U32 t = readl(a);

    if (!(t & BM_RSTGEN_SW_SELF_RST_SW_GLB_RST_LOCK)) {
        if (v) {
            t |= BM_RSTGEN_SW_SELF_RST_SW_GLB_RST;
        } else {
            t &= ~BM_RSTGEN_SW_SELF_RST_SW_GLB_RST;
        }

        /* to make sure no uncompleted transfer on fabric */
        dsb();
        isb();
        writel(t, a);
    }
}

#if defined(CFG_DRV_RSTGEN_API_rg_glb_other_reset)
void rg_glb_other_reset(U32 base, U8 v)
{
    U32 a = base + SOC_RSTGEN_REG_MAP(RSTGEN_SW_OTH_RST_OFF);
    U32 t = readl(a);

    if (!(t & BM_RSTGEN_SW_OTH_RST_SW_GLB_RST_LOCK)) {
        if (v) {
            t |= BM_RSTGEN_SW_OTH_RST_SW_GLB_RST;
        } else {
            t &= ~BM_RSTGEN_SW_OTH_RST_SW_GLB_RST;
        }

        writel(t, a);
    }
}
#endif

#if defined(CFG_DRV_RSTGEN_API_rg_en_isolation)
void rg_en_isolation(U32 base, U8 id)
{
    U32 a = base + SOC_RSTGEN_REG_MAP(RSTGEN_ISO_EN_OFF(id));
    U32 v = readl(a);
    v &= ~BM_RSTGEN_ISO_EN_ISO_B;
    writel(v, a);
}
#endif

void rg_dis_isolation(U32 base, U8 id)
{
    U32 a = base + SOC_RSTGEN_REG_MAP(RSTGEN_ISO_EN_OFF(id));
    U32 v = readl(a);
    v |= BM_RSTGEN_ISO_EN_ISO_B;
    writel(v, a);
}
