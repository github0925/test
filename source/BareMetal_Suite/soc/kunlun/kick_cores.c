/********************************************************
 *          Copyright(c) 2020   Semidrive               *
 *******************************************************/

#include <common_hdr.h>
#include <soc.h>
#include "rstgen/rstgen.h"
#include "scr/scr.h"
#include "pll/sc_ss_pfpll.h"
#include "clk/ckgen.h"

static void kick_ap1(void)
{
    soc_dis_isolation(CPU1_SS);

    rg_module_reset(APB_RSTGEN_SEC_BASE, RSTGEN_SEC_MODULE_RST_B_PLL_CPU1A_INDEX, 1);
    sc_pfpll_program(APB_PLL_CPU1A_BASE, PLL_FBDIV(207),
                     PLL_REFDIV(3), PLL_FRAC(0), PLL_POSTDIV1(1), DIV_ABCD(2, 4, 0, 0));
    sc_pfpll_program(APB_PLL_CPU1B_BASE, PLL_FBDIV(166),
                     PLL_REFDIV(3), PLL_FRAC(0), PLL_POSTDIV1(1), DIV_ABCD(2, 4, 0, 0));

    /* setup uuu of cpu1a/cpu1b */
    ckgen_uuu_slice_cfg(APB_CKGEN_SOC_BASE, UUU_ID(0),
                        UUU_SEL_PLL, DIV_MNPQ(0, 1, 3, 7));
    ckgen_uuu_slice_cfg(APB_CKGEN_SOC_BASE, UUU_ID(1),
                        UUU_SEL_PLL, DIV_MNPQ(0, 1, 3, 7));

    rg_core_reset(APB_RSTGEN_SEC_BASE, RSTGEN_SEC_CORE_RST_B_CPU1_INDEX);
    rg_module_reset(APB_RSTGEN_SEC_BASE,
                    RSTGEN_SEC_MODULE_RST_B_CPU1_NSRESET_INDEX, 1);
}

static void kick_ap2(void)
{
    soc_dis_isolation(CPU2_SS);

    rg_module_reset(APB_RSTGEN_SEC_BASE, RSTGEN_SEC_MODULE_RST_B_PLL_CPU2_INDEX, 1);
    sc_pfpll_program(APB_PLL_CPU2_BASE, PLL_FBDIV(207),
                     PLL_REFDIV(3), PLL_FRAC(0), PLL_POSTDIV1(1), DIV_ABCD(2, 4, 0, 0));

    ckgen_uuu_slice_cfg(APB_CKGEN_SOC_BASE, UUU_ID(2),
                        UUU_SEL_PLL, DIV_MNPQ(0, 1, 3, 7));

}

void soc_stop_secondary_core(module_e m, uint32_t core)
{
    if (CPU_AP1 == m) {
        switch (core) {
        case 1:
            rg_module_reset(APB_RSTGEN_SEC_BASE, RSTGEN_SEC_MODULE_RST_B_CPU1_NCORERESET_1_INDEX, 0);
            break;

        case 2:
            rg_module_reset(APB_RSTGEN_SEC_BASE, RSTGEN_SEC_MODULE_RST_B_CPU1_NCORERESET_2_INDEX, 0);
            break;

        case 3:
            rg_module_reset(APB_RSTGEN_SEC_BASE, RSTGEN_SEC_MODULE_RST_B_CPU1_NCORERESET_3_INDEX, 0);
            break;

        case 4:
            rg_module_reset(APB_RSTGEN_SEC_BASE, RSTGEN_SEC_MODULE_RST_B_CPU1_NCORERESET_4_INDEX, 0);
            break;

        case 5:
            rg_module_reset(APB_RSTGEN_SEC_BASE, RSTGEN_SEC_MODULE_RST_B_CPU1_NCORERESET_5_INDEX, 0);
            break;

        default :
            break;
        }
    }
}

void soc_start_cpu_core(module_e m, uint32_t core, addr_t rvba)
{
    if (CPU_AP1 == m) {
        switch (core) {
        case 0:
            /* setup rvbar */
            scr_bits_wr(APB_SCR_SEC_BASE, L31,
                        SCR_SEC_CPU1_RVBARADDR0_29_2_L31_START_BIT, 28, (uint32_t)rvba >> 2);
            scr_bits_wr(APB_SCR_SEC_BASE, L31,
                        SCR_SEC_CPU1_RVBARADDR0_39_30_L31_START_BIT, 10, (uint32_t)(rvba >> 30));
            /* disable clock gating */
            ckgen_cg_en(APB_CKGEN_SOC_BASE, CKGEN_SOC_CKGATE_CPU1_CORE0_INDEX);
            /* release resets */
            rg_module_reset(APB_RSTGEN_SEC_BASE, RSTGEN_SEC_MODULE_RST_B_CPU1_NCORERESET_0_INDEX, 1);
            break;

        case 1:
            scr_bits_wr(APB_SCR_SEC_BASE, L31,
                        SCR_SEC_CPU1_RVBARADDR1_29_2_L31_START_BIT, 28, (uint32_t)rvba >> 2);
            scr_bits_wr(APB_SCR_SEC_BASE, L31,
                        SCR_SEC_CPU1_RVBARADDR1_39_30_L31_START_BIT, 10, (uint32_t)(rvba >> 30));
            ckgen_cg_en(APB_CKGEN_SOC_BASE, CKGEN_SOC_CKGATE_CPU1_CORE1_INDEX);
            rg_module_reset(APB_RSTGEN_SEC_BASE, RSTGEN_SEC_MODULE_RST_B_CPU1_NCORERESET_1_INDEX, 1);
            break;

        case 2:
            scr_bits_wr(APB_SCR_SEC_BASE, L31,
                        SCR_SEC_CPU1_RVBARADDR2_29_2_L31_START_BIT, 28, (uint32_t)rvba >> 2);
            scr_bits_wr(APB_SCR_SEC_BASE, L31,
                        SCR_SEC_CPU1_RVBARADDR2_39_30_L31_START_BIT, 10, (uint32_t)(rvba >> 30));    
            ckgen_cg_en(APB_CKGEN_SOC_BASE, CKGEN_SOC_CKGATE_CPU1_CORE2_INDEX);
            rg_module_reset(APB_RSTGEN_SEC_BASE, RSTGEN_SEC_MODULE_RST_B_CPU1_NCORERESET_2_INDEX, 1);
            break;

        case 3:
            scr_bits_wr(APB_SCR_SEC_BASE, L31,
                        SCR_SEC_CPU1_RVBARADDR3_29_2_L31_START_BIT, 28, (uint32_t)rvba >> 2);
            scr_bits_wr(APB_SCR_SEC_BASE, L31,
                        SCR_SEC_CPU1_RVBARADDR3_39_30_L31_START_BIT, 10, (uint32_t)(rvba >> 30));                        
            ckgen_cg_en(APB_CKGEN_SOC_BASE, CKGEN_SOC_CKGATE_CPU1_CORE3_INDEX);
            rg_module_reset(APB_RSTGEN_SEC_BASE, RSTGEN_SEC_MODULE_RST_B_CPU1_NCORERESET_3_INDEX, 1);
            break;

        case 4:
            scr_bits_wr(APB_SCR_SEC_BASE, L31,
                        SCR_SEC_CPU1_RVBARADDR4_29_2_L31_START_BIT, 28, (uint32_t)rvba >> 2);
            scr_bits_wr(APB_SCR_SEC_BASE, L31,
                        SCR_SEC_CPU1_RVBARADDR4_39_30_L31_START_BIT, 10, (uint32_t)(rvba >> 30));                        
            ckgen_cg_en(APB_CKGEN_SOC_BASE, CKGEN_SOC_CKGATE_CPU1_CORE4_INDEX);
            rg_module_reset(APB_RSTGEN_SEC_BASE, RSTGEN_SEC_MODULE_RST_B_CPU1_NCORERESET_4_INDEX, 1);
            break;

        case 5:
            scr_bits_wr(APB_SCR_SEC_BASE, L31,
                        SCR_SEC_CPU1_RVBARADDR5_29_2_L31_START_BIT, 28, (uint32_t)rvba >> 2);
            scr_bits_wr(APB_SCR_SEC_BASE, L31,
                        SCR_SEC_CPU1_RVBARADDR5_39_30_L31_START_BIT, 10, (uint32_t)(rvba >> 30));                        
            ckgen_cg_en(APB_CKGEN_SOC_BASE, CKGEN_SOC_CKGATE_CPU1_CORE5_INDEX);
            rg_module_reset(APB_RSTGEN_SEC_BASE, RSTGEN_SEC_MODULE_RST_B_CPU1_NCORERESET_5_INDEX, 1);
            break;

        default :
            break;
        }
    } else if ((CPU_AP2 == m) && (core == 0)) {
        /* setup rvbar */
        scr_bits_wr(APB_SCR_SEC_BASE, L31,
                    SCR_SEC_CPU2_RVBARADDR0_29_2_L31_START_BIT, 28, (uint32_t)rvba >> 2);
        scr_bits_wr(APB_SCR_SEC_BASE, L31,
                    SCR_SEC_CPU2_RVBARADDR0_39_30_L31_START_BIT, 10, (uint32_t)(rvba >> 30));                    
        /* disable clock gating */
        ckgen_cg_en(APB_CKGEN_SOC_BASE, CKGEN_SOC_CKGATE_CPU2_INDEX);
        /* release resets */
        rg_core_reset(APB_RSTGEN_SEC_BASE, RSTGEN_SEC_CORE_RST_B_CPU2_INDEX);
    }
}

void soc_kick_cpu(module_e m)
{
    switch (m) {
    case CPU_AP1:
        kick_ap1();
        break;

    case CPU_AP2:
        kick_ap2();
        break;

    default:
        break;
    }
}
