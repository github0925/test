//*****************************************************************************
//
// rstgen_cmd_test.c - app for the rstgen test Module.
//
// Copyright (c) 2019-2029 SemiDrive Incorporated.  All rights reserved.
// Software License Agreement
//
//*****************************************************************************
#include <app.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <debug.h>
#include <platform.h>
#include <lib/console.h>
#include <rstgen_hal_ip_test.h>
#include "res.h"
#include "chip_res.h"

static void *g_handle;

/*globale res to rstgen res index*/
const RstgenResManageIdToIdx g_IsoResIdToIsoRstgenIdx[iso_idx_max] = {
    {RES_ISO_EN_SEC_PCIE,pcie_iso_b},
    {RES_ISO_EN_SEC_USB,usb_iso_b},
    {RES_ISO_EN_SEC_CPU1,cpu1_iso_b},
    {RES_ISO_EN_SEC_GPU1,gpu1_iso_b},
    {RES_ISO_EN_SEC_DDR,ddr_iso_b},
};

const RstgenResManageIdToIdx g_CoreResIdToCoreRstgenIdx[saf_core_max+ap_core_max] = {
    {RES_CORE_RST_SAF_CR5_SAF,saf_core_cr5_saf},
    {RES_CORE_RST_SEC_VDSP_SW,ap_core_vdsp},
    {RES_CORE_RST_SEC_CR5_SEC_SW,ap_core_cr5_sec},
    {RES_CORE_RST_SEC_CR5_MP_SW,ap_core_cr5_mp},
    {RES_CORE_RST_SEC_CPU1_CORE_ALL_SW,ap_core_cpu1_core_all},
    {RES_CORE_RST_SEC_CPU2_CORE_SW,ap_core_cpu2_core},
    {RES_CORE_RST_SEC_ADSP_SW,ap_core_adsp},
};

const RstgenResManageIdToIdx g_ModuleResIdToModuleRstgenIdx[saf_module_max+ap_module_max] = {
    {RES_MODULE_RST_SAF_GIC1,saf_module_gic1},
    {RES_MODULE_RST_SAF_OSPI1,saf_module_ospi1},
    {RES_MODULE_RST_SAF_ENET1,saf_module_enet1},
    {RES_MODULE_RST_SAF_I2S_SC1,saf_module_i2s_sc1},
    {RES_MODULE_RST_SAF_I2S_SC2,saf_module_i2s_sc2},
    {RES_MODULE_RST_SAF_CANFD1,saf_module_canfd1},
    {RES_MODULE_RST_SAF_CANFD2,saf_module_canfd2},
    {RES_MODULE_RST_SAF_CANFD3,saf_module_canfd3},
    {RES_MODULE_RST_SAF_CANFD4,saf_module_canfd4},
    {RES_MODULE_RST_SAF_SEM1,saf_module_sem1},
    {RES_MODULE_RST_SAF_SEM2,saf_module_sem2},
    {RES_MODULE_RST_SEC_OSPI2,ap_module_ospi2},
    {RES_MODULE_RST_SEC_I2S_SC3,ap_module_i2s_sc3},
    {RES_MODULE_RST_SEC_I2S_SC4,ap_module_i2s_sc4},
    {RES_MODULE_RST_SEC_I2S_SC5,ap_module_i2s_sc5},
    {RES_MODULE_RST_SEC_I2S_SC6,ap_module_i2s_sc6},
    {RES_MODULE_RST_SEC_I2S_SC7,ap_module_i2s_sc7},
    {RES_MODULE_RST_SEC_I2S_SC8,ap_module_i2s_sc8},
    {RES_MODULE_RST_SEC_I2S_MC1,ap_module_i2s_mc1},
    {RES_MODULE_RST_SEC_I2S_MC2,ap_module_i2s_mc2},
    {RES_MODULE_RST_SEC_CANFD5,ap_module_canfd5},
    {RES_MODULE_RST_SEC_CANFD6,ap_module_canfd6},
    {RES_MODULE_RST_SEC_CANFD7,ap_module_canfd7},
    {RES_MODULE_RST_SEC_CANFD8,ap_module_canfd8},
    {RES_MODULE_RST_SEC_ENET2,ap_module_enet2},
    {RES_MODULE_RST_SEC_MSHC1,ap_module_mshc1},
    {RES_MODULE_RST_SEC_MSHC2,ap_module_mshc2},
    {RES_MODULE_RST_SEC_MSHC3,ap_module_mshc3},
    {RES_MODULE_RST_SEC_MSHC4,ap_module_mshc4},
    {RES_MODULE_RST_SEC_ADSP,ap_module_adsp},
    {RES_MODULE_RST_SEC_GIC2,ap_module_gic2},
    {RES_MODULE_RST_SEC_GIC3,ap_module_gic3},
    {RES_MODULE_RST_SEC_CPU1_CORE0_WARM,ap_module_cpu1_core0_warm},
    {RES_MODULE_RST_SEC_CPU1_CORE1_WARM,ap_module_cpu1_core1_warm},
    {RES_MODULE_RST_SEC_CPU1_CORE2_WARM,ap_module_cpu1_core2_warm},
    {RES_MODULE_RST_SEC_CPU1_CORE3_WARM,ap_module_cpu1_core3_warm},
    {RES_MODULE_RST_SEC_CPU1_CORE4_WARM,ap_module_cpu1_core4_warm},
    {RES_MODULE_RST_SEC_CPU1_CORE5_WARM,ap_module_cpu1_core5_warm},
    {RES_MODULE_RST_SEC_CPU1_SCU_WARM,ap_module_cpu1_scu_warm},
    {RES_MODULE_RST_SEC_DDR_SS,ap_module_ddr_ss},
    {RES_MODULE_RST_SEC_DDR_SW_1,ap_module_ddr_sw0},
    {RES_MODULE_RST_SEC_DDR_SW_2,ap_module_ddr_sw1},
    {RES_MODULE_RST_SEC_DDR_SW_3,ap_module_ddr_sw2},
    {RES_MODULE_RST_SEC_GIC4,ap_module_gic4},
    {RES_MODULE_RST_SEC_GIC5,ap_module_gic5},
    {RES_MODULE_RST_SEC_CSSYS_TRESET_N,ap_module_cssys_treset_n},
    {RES_MODULE_RST_SEC_NNA,ap_module_nna},
    {RES_MODULE_RST_SEC_VDSP_DRESET,ap_module_vdsp_DReset},
    {RES_MODULE_RST_SEC_VPU1,ap_module_vpu1},
    {RES_MODULE_RST_SEC_VPU2,ap_module_vpu2},
    {RES_MODULE_RST_SEC_MJPEG,ap_module_mjpeg},
    {RES_MODULE_RST_SEC_CPU1_SS,ap_module_cpu1_ss},
    {RES_MODULE_RST_SEC_CPU2_SS,ap_module_cpu2_ss},
    {RES_MODULE_RST_SEC_MIPI_CSI1,ap_module_mipi_csi1},
    {RES_MODULE_RST_SEC_MIPI_CSI2,ap_module_mipi_csi2},
    {RES_MODULE_RST_SEC_MIPI_CSI3,ap_module_mipi_csi3},
    {RES_MODULE_RST_SEC_MIPI_DSI1,ap_module_mipi_dsi1},
    {RES_MODULE_RST_SEC_MIPI_DSI2,ap_module_mipi_dsi2},
    {RES_MODULE_RST_SEC_DC1,ap_module_dc1},
    {RES_MODULE_RST_SEC_DC2,ap_module_dc2},
    {RES_MODULE_RST_SEC_DC3,ap_module_dc3},
    {RES_MODULE_RST_SEC_DC4,ap_module_dc4},
    {RES_MODULE_RST_SEC_DC5,ap_module_dc5},
    {RES_MODULE_RST_SEC_DP1,ap_module_dp1},
    {RES_MODULE_RST_SEC_DP2,ap_module_dp2},
    {RES_MODULE_RST_SEC_DP3,ap_module_dp3},
    {RES_MODULE_RST_SEC_LVDS_SS,ap_module_lvds_ss},
    {RES_MODULE_RST_SEC_CSI1,ap_module_csi1},
    {RES_MODULE_RST_SEC_CSI2,ap_module_csi2},
    {RES_MODULE_RST_SEC_CSI3,ap_module_csi3},
    {RES_MODULE_RST_SEC_DISP_MUX,ap_module_disp_mux},
    {RES_MODULE_RST_SEC_G2D1,ap_module_g2d1},
    {RES_MODULE_RST_SEC_G2D2,ap_module_g2d2},
    {RES_MODULE_RST_SEC_GPU1_CORE,ap_module_gpu1_core},
    {RES_MODULE_RST_SEC_GPU1_SS,ap_module_gpu1_ss},
    {RES_MODULE_RST_SEC_GPU2_CORE,ap_module_gpu2_core},
    {RES_MODULE_RST_SEC_GPU2_SS,ap_module_gpu2_ss},
    {RES_MODULE_RST_SEC_DBG_REQ,ap_module_dbg_req},
    {RES_MODULE_RST_SEC_CANFD9,ap_module_canfd9},
    {RES_MODULE_RST_SEC_CANFD10,ap_module_canfd10},
    {RES_MODULE_RST_SEC_CANFD11,ap_module_canfd11},
    {RES_MODULE_RST_SEC_CANFD12,ap_module_canfd12},
    {RES_MODULE_RST_SEC_CANFD13,ap_module_canfd13},
    {RES_MODULE_RST_SEC_CANFD14,ap_module_canfd14},
    {RES_MODULE_RST_SEC_CANFD15,ap_module_canfd15},
    {RES_MODULE_RST_SEC_CANFD16,ap_module_canfd16},
    {RES_MODULE_RST_SEC_CANFD17,ap_module_canfd17},
    {RES_MODULE_RST_SEC_CANFD18,ap_module_canfd18},
    {RES_MODULE_RST_SEC_CANFD19,ap_module_canfd19},
    {RES_MODULE_RST_SEC_CANFD20,ap_module_canfd20},
};

char rstgen_core_readonlyreg_check_test_help[]= {
    "rstgen_core_readonlyreg_check_test:start_idx\n" \
    "" \
};
int do_rstgen_core_readonlyreg_check_test(int argc, const cmd_args *argv)
{
    bool ret = true;
    uint8_t start_idx = argv[1].u;

    printf("do_rstgen_read_only_reg_test str:%s   u:%d\n",argv[1].str,argv[1].u);
    ret = hal_rstgen_test_creat_handle(&g_handle,RES_GLOBAL_RST_SEC_RST_EN);
    if(!ret){
        ret = hal_rstgen_test_creat_handle(&g_handle,RES_GLOBAL_RST_SAF_RST_EN);
    }

    if(ret){
        for(uint16_t i = start_idx;i<(saf_core_max+ap_core_max);i++)
        {
            ret = hal_rstgen_core_readonlyreg_check_test(g_handle,g_CoreResIdToCoreRstgenIdx[i].res_glb_idx);
            if(!ret){
                break;
            }
        }
    }

    hal_rstgen_test_release_handle(g_handle);
    printf("do_rstgen_core_readonlyreg_check_test ret:%d:\n",ret);
    return ret;
}


char rstgen_module_readonlyreg_check_test_help[]= {
    "rstgen_module_readonlyreg_check_test:start_idx\n" \
    "" \
};
int do_rstgen_module_readonlyreg_check_test(int argc, const cmd_args *argv)
{
    bool ret = true;
    uint8_t start_idx = argv[1].u;

    printf("do_rstgen_read_only_reg_test str:%s   u:%d\n",argv[1].str,argv[1].u);
    ret = hal_rstgen_test_creat_handle(&g_handle,RES_GLOBAL_RST_SEC_RST_EN);
    if(!ret){
        ret = hal_rstgen_test_creat_handle(&g_handle,RES_GLOBAL_RST_SAF_RST_EN);
    }

    if(ret){
        for(uint16_t i = start_idx;i<(saf_module_max+ap_module_max);i++){
            ret = hal_rstgen_module_readonlyreg_check_test(g_handle,g_ModuleResIdToModuleRstgenIdx[i].res_glb_idx);
            if(!ret){
                break;
            }
        }
    }

    hal_rstgen_test_release_handle(g_handle);
    printf("do_rstgen_module_readonlyreg_check_test ret:%d:\n",ret);
    return ret;
}

char rstgen_global_rw_reg_check_help[]= {
    "rstgen_global_rw_reg_check_test:null\n" \
    "" \
};
int do_rstgen_global_rw_reg_check_test(int argc, const cmd_args *argv)
{
    bool ret = true;

    ret = hal_rstgen_test_creat_handle(&g_handle,RES_GLOBAL_RST_SEC_RST_EN);
    if(!ret){
        ret = hal_rstgen_test_creat_handle(&g_handle,RES_GLOBAL_RST_SAF_RST_EN);
    }

    if(ret){
        ret = hal_rstgen_global_rw_reg_check_test(g_handle);
    }

    hal_rstgen_test_release_handle(g_handle);
    printf("do_rstgen_global_rw_reg_check_test ret:%d:\n",ret);
    return ret;
}

char rstgen_core_rw_reg_check_test_help[]= {
    "rstgen_core_rw_reg_check_test:start_idx\n" \
    "" \
};
int do_rstgen_core_rw_reg_check_test(int argc, const cmd_args *argv)
{
    bool ret = true;
    uint8_t start_idx = argv[1].u;

    ret = hal_rstgen_test_creat_handle(&g_handle,RES_GLOBAL_RST_SEC_RST_EN);
    if(!ret){
        ret = hal_rstgen_test_creat_handle(&g_handle,RES_GLOBAL_RST_SAF_RST_EN);
    }

    if(ret){
        for(uint16_t i = start_idx;i<(saf_core_max+ap_core_max);i++){
            ret = hal_rstgen_core_rw_reg_check_test(g_handle,g_CoreResIdToCoreRstgenIdx[i].res_glb_idx);
            if(!ret){
                break;
            }
        }
    }

    hal_rstgen_test_release_handle(g_handle);
    printf("do_rstgen_core_rw_reg_check_test ret:%d:\n",ret);
    return ret;
}

char rstgen_module_rw_reg_check_test_help[]= {
    "rstgen_module_rw_reg_check_test:start_idx\n" \
    "" \
};
int do_rstgen_module_rw_reg_check_test(int argc, const cmd_args *argv)
{
    bool ret = true;
    uint8_t start_idx = argv[1].u;

    ret = hal_rstgen_test_creat_handle(&g_handle,RES_GLOBAL_RST_SEC_RST_EN);
    if(!ret){
        ret = hal_rstgen_test_creat_handle(&g_handle,RES_GLOBAL_RST_SAF_RST_EN);
    }

    if(ret){
        for(uint16_t i = start_idx;i<(saf_module_max+ap_module_max);i++)
        {
            ret = hal_rstgen_module_rw_reg_check_test(g_handle,g_ModuleResIdToModuleRstgenIdx[i].res_glb_idx);
            if(!ret){
                break;
            }
        }
    }

    hal_rstgen_test_release_handle(g_handle);
    printf("do_rstgen_module_rw_reg_check_test ret:%d:\n",ret);
    return ret;
}

char rstgen_iso_rw_reg_check_test_help[]= {
    "rstgen_iso_rw_reg_check_test:iso_idx\n" \
    "" \
};
int do_rstgen_iso_rw_reg_check_test(int argc, const cmd_args *argv)
{
    bool ret = true;
    uint8_t iso_idx = argv[1].u;

    ret = hal_rstgen_test_creat_handle(&g_handle,RES_GLOBAL_RST_SEC_RST_EN);
    if(!ret){
        ret = hal_rstgen_test_creat_handle(&g_handle,RES_GLOBAL_RST_SAF_RST_EN);
    }

    if(ret){
        for(uint16_t i = iso_idx;i<(iso_idx_max);i++)
        {
            ret = hal_rstgen_iso_rw_reg_check_test(g_handle,g_IsoResIdToIsoRstgenIdx[i].res_glb_idx);
            if(!ret){
                break;
            }
        }
    }

    hal_rstgen_test_release_handle(g_handle);
    printf("do_rstgen_terninal_from_fuse_test ret:%d:\n",ret);
    return ret;
}

char rstgen_general_rw_reg_check_test_help[]= {
    "rstgen_general_rw_reg_check_test:general_idx\n" \
    "" \
};
int do_rstgen_general_rw_reg_check_test(int argc, const cmd_args *argv)
{
    bool ret = true;
    uint8_t general_idx = argv[1].u;

    ret = hal_rstgen_test_creat_handle(&g_handle,RES_GLOBAL_RST_SEC_RST_EN);
    if(!ret){
        ret = hal_rstgen_test_creat_handle(&g_handle,RES_GLOBAL_RST_SAF_RST_EN);
    }

    if(ret){
        for(uint32_t general_idx = RES_GENERAL_REG_SEC_GENERAL_REG1;general_idx<=(RES_GENERAL_REG_SEC_GENERAL_REG8);general_idx++)
        {
            ret = hal_rstgen_general_rw_reg_check_test(g_handle,general_idx);
            if(!ret){
                break;
            }
        }
    }

    hal_rstgen_test_release_handle(g_handle);
    printf("do_rstgen_general_rw_reg_check_test ret:%d:\n",ret);
    return ret;
}

char rstgen_global_sw_self_rst_test_help[]= {
    "rstgen_global_sw_self_rst_test:null\n" \
    "" \
};
int do_rstgen_global_sw_self_rst_test(int argc, const cmd_args *argv)
{
    bool ret = true;

    ret = hal_rstgen_test_creat_handle(&g_handle,RES_GLOBAL_RST_SEC_RST_EN);
    if(!ret){
        ret = hal_rstgen_test_creat_handle(&g_handle,RES_GLOBAL_RST_SAF_RST_EN);
    }

    if(ret){
        ret = hal_rstgen_global_sw_self_rst_test(g_handle);
    }

    hal_rstgen_test_release_handle(g_handle);
    printf("do_rstgen_global_sw_self_rst_test ret:%d:\n",ret);
    return ret;
}

char rstgen_global_sem_rst_test_help[]= {
    "rstgen_global_sem_rst_test:rstgen really number\n" \
    "" \
};
int do_rstgen_global_sem_rst_test(int argc, const cmd_args *argv)
{
    bool ret = true;

    ret = hal_rstgen_test_creat_handle(&g_handle,RES_GLOBAL_RST_SEC_RST_EN);
    if(!ret){
        ret = hal_rstgen_test_creat_handle(&g_handle,RES_GLOBAL_RST_SAF_RST_EN);
    }

    if(ret){
        ret = hal_rstgen_global_sem_rst_test(g_handle);
    }

    hal_rstgen_test_release_handle(g_handle);
    printf("do_rstgen_global_sem_rst_test ret:%d:\n",ret);
    return ret;
}

char rstgen_global_dbg_rst_test_help[]= {
    "rstgen_global_dbg_rst_test:null\n" \
    "" \
};
int do_rstgen_global_dbg_rst_test(int argc, const cmd_args *argv)
{
    bool ret = true;

    ret = hal_rstgen_test_creat_handle(&g_handle,RES_GLOBAL_RST_SEC_RST_EN);
    if(!ret){
        ret = hal_rstgen_test_creat_handle(&g_handle,RES_GLOBAL_RST_SAF_RST_EN);
    }

    if(ret){
        ret = hal_rstgen_global_dbg_rst_test(g_handle);
    }

    hal_rstgen_test_release_handle(g_handle);
    printf("do_rstgen_global_dbg_rst_test ret:%d:\n",ret);
    return ret;
}

char rstgen_global_wdg_rst_test_help[]= {
    "rstgen_global_wdg_rst_test:watchdog really number\n" \
    "" \
};
int do_rstgen_global_wdg_rst_test(int argc, const cmd_args *argv)
{
    bool ret = true;
    uint8_t watchdog_really_number = argv[1].u;

    if((0== watchdog_really_number) || (watchdog_really_number > 8)){
        ret = false;
        printf("do_rstgen_global_wdg_rst_test ret:%d:\n",ret);
        return ret;
    }

    ret = hal_rstgen_test_creat_handle(&g_handle,RES_GLOBAL_RST_SEC_RST_EN);
    if(!ret){
        ret = hal_rstgen_test_creat_handle(&g_handle,RES_GLOBAL_RST_SAF_RST_EN);
    }

    if(ret){
        ret = hal_rstgen_global_wdg_rst_test(g_handle,watchdog_really_number);
    }

    hal_rstgen_test_release_handle(g_handle);
    printf("do_rstgen_global_wdg_rst_test ret:%d:\n",ret);
    return ret;
}

char rstgen_global_pre_rst_test_help[]= {
    "rstgen_global_pre_rst_test:null\n" \
    "" \
};
int do_rstgen_global_pre_rst_test(int argc, const cmd_args *argv)
{
    bool ret = true;

    ret = hal_rstgen_test_creat_handle(&g_handle,RES_GLOBAL_RST_SEC_RST_EN);
    if(!ret){
        ret = hal_rstgen_test_creat_handle(&g_handle,RES_GLOBAL_RST_SAF_RST_EN);
    }

    if(ret){
        ret = hal_rstgen_global_pre_rst_test(g_handle);
    }

    hal_rstgen_test_release_handle(g_handle);
    printf("do_rstgen_global_pre_rst_test ret:%d:\n",ret);
    return ret;
}

char rstgen_self_rst_test_help[]= {
    "rstgen_self_rst_test:null\n" \
    "" \
};
int do_rstgen_self_rst_test(int argc, const cmd_args *argv)
{
    bool ret = true;

    ret = hal_rstgen_test_creat_handle(&g_handle,RES_GLOBAL_RST_SEC_RST_EN);
    if(!ret){
        ret = hal_rstgen_test_creat_handle(&g_handle,RES_GLOBAL_RST_SAF_RST_EN);
    }

    if(ret){
        ret = hal_rstgen_self_rst_test(g_handle);
    }

    hal_rstgen_test_release_handle(g_handle);
    printf("do_rstgen_self_rst_test ret:%d:\n",ret);
    return ret;
}

char rstgen_other_rst_test_help[]= {
    "rstgen_other_rst_test:null\n" \
    "" \
};
int do_rstgen_other_rst_test(int argc, const cmd_args *argv)
{
    bool ret = true;

    ret = hal_rstgen_test_creat_handle(&g_handle,RES_GLOBAL_RST_SEC_RST_EN);
    if(!ret){
        ret = hal_rstgen_test_creat_handle(&g_handle,RES_GLOBAL_RST_SAF_RST_EN);
    }

    if(ret){
        ret = hal_rstgen_other_rst_test(g_handle);
    }

    hal_rstgen_test_release_handle(g_handle);
    printf("do_rstgen_other_rst_test ret:%d:\n",ret);
    return ret;
}

char rstgen_core_wdg_rst_test_help[]= {
    "rstgen_core_wdg_rst_test:watchdog really number\n" \
    "                                        :start_idx\n" \
    "" \
};
int do_rstgen_core_wdg_rst_test(int argc, const cmd_args *argv)
{
    bool ret = true;
    uint8_t watchdog_really_number = argv[1].u;
    uint8_t start_idx = argv[2].u;

    if((0== watchdog_really_number) || (watchdog_really_number > 8)){
        ret = false;
        printf("do_rstgen_core_wdg_rst_test ret:%d:\n",ret);
        return ret;
    }

    ret = hal_rstgen_test_creat_handle(&g_handle,RES_GLOBAL_RST_SEC_RST_EN);
    if(!ret){
        ret = hal_rstgen_test_creat_handle(&g_handle,RES_GLOBAL_RST_SAF_RST_EN);
    }

    if(ret){
        for(uint16_t i = start_idx;i<(saf_core_max+ap_core_max);i++)
        {
            ret = hal_rstgen_core_wdg_rst_test(g_handle,watchdog_really_number,g_CoreResIdToCoreRstgenIdx[i].res_glb_idx);
            if(!ret){
                break;
            }
        }
    }

    hal_rstgen_test_release_handle(g_handle);
    printf("do_rstgen_core_wdg_rst_test ret:%d:\n",ret);
    return ret;
}

char rstgen_core_dbg_rst_test_help[]= {
    "rstgen_core_dbg_rst_test:start_idx\n" \
    "" \
};
int do_rstgen_core_dbg_rst_test(int argc, const cmd_args *argv)
{
    bool ret = true;
    uint8_t start_idx = argv[1].u;

    ret = hal_rstgen_test_creat_handle(&g_handle,RES_GLOBAL_RST_SEC_RST_EN);
    if(!ret){
        ret = hal_rstgen_test_creat_handle(&g_handle,RES_GLOBAL_RST_SAF_RST_EN);
    }

    if(ret){
        for(uint16_t i = start_idx;i<(saf_core_max+ap_core_max);i++)
        {
            ret = hal_rstgen_core_dbg_rst_test(g_handle,g_CoreResIdToCoreRstgenIdx[i].res_glb_idx);
            if(!ret){
                break;
            }
        }
    }

    hal_rstgen_test_release_handle(g_handle);
    printf("do_rstgen_core_dbg_rst_test ret:%d:\n",ret);
    return ret;
}


char rstgen_core_rst_test_help[]= {
    "rstgen_core_rst_test:start_idx\n" \
    "" \
};
int do_rstgen_core_rst_test(int argc, const cmd_args *argv)
{
    bool ret = true;
    uint8_t start_idx = argv[1].u;

    ret = hal_rstgen_test_creat_handle(&g_handle,RES_GLOBAL_RST_SEC_RST_EN);
    if(!ret){
        ret = hal_rstgen_test_creat_handle(&g_handle,RES_GLOBAL_RST_SAF_RST_EN);
    }

    if(ret){
        for(uint16_t i = start_idx;i<(saf_core_max+ap_core_max);i++)
        {
            ret = hal_rstgen_core_rst_test(g_handle,g_CoreResIdToCoreRstgenIdx[i].res_glb_idx);
            if(!ret){
                break;
            }
        }
    }

    hal_rstgen_test_release_handle(g_handle);
    printf("do_rstgen_core_dbg_rst_test ret:%d:\n",ret);
    return ret;
}

char rstgen_module_rst_test_help[]= {
    "rstgen_module_rst_test:start_idx\n" \
    "" \
};
int do_rstgen_module_rst_test(int argc, const cmd_args *argv)
{
    bool ret = true;
    uint8_t start_idx = argv[1].u;

    ret = hal_rstgen_test_creat_handle(&g_handle,RES_GLOBAL_RST_SEC_RST_EN);
    if(!ret){
        ret = hal_rstgen_test_creat_handle(&g_handle,RES_GLOBAL_RST_SAF_RST_EN);
    }

    if(ret){
        for(uint16_t i = start_idx;i<(saf_module_max+ap_module_max);i++)
        {
            ret = hal_rstgen_module_rst_test(g_handle,g_ModuleResIdToModuleRstgenIdx[i].res_glb_idx);
            if(!ret){
                break;
            }
        }
    }

    hal_rstgen_test_release_handle(g_handle);
    printf("do_rstgen_module_rst_test ret:%d:\n",ret);
    return ret;
}


#if defined(WITH_LIB_CONSOLE)
STATIC_COMMAND_START
STATIC_COMMAND("rstgen_test1", rstgen_core_readonlyreg_check_test_help, (console_cmd)&do_rstgen_core_readonlyreg_check_test)
STATIC_COMMAND("rstgen_test2", rstgen_module_readonlyreg_check_test_help, (console_cmd)&do_rstgen_module_readonlyreg_check_test)
STATIC_COMMAND("rstgen_test3", rstgen_global_rw_reg_check_help, (console_cmd)&do_rstgen_global_rw_reg_check_test)
STATIC_COMMAND("rstgen_test4", rstgen_core_rw_reg_check_test_help, (console_cmd)&do_rstgen_core_rw_reg_check_test)
STATIC_COMMAND("rstgen_test5", rstgen_module_rw_reg_check_test_help, (console_cmd)&do_rstgen_module_rw_reg_check_test)
STATIC_COMMAND("rstgen_test6", rstgen_iso_rw_reg_check_test_help, (console_cmd)&do_rstgen_iso_rw_reg_check_test)
STATIC_COMMAND("rstgen_test7", rstgen_general_rw_reg_check_test_help, (console_cmd)&do_rstgen_general_rw_reg_check_test)
STATIC_COMMAND("rstgen_test8", rstgen_global_sw_self_rst_test_help, (console_cmd)&do_rstgen_global_sw_self_rst_test)
STATIC_COMMAND("rstgen_test9", rstgen_global_sem_rst_test_help, (console_cmd)&do_rstgen_global_sem_rst_test)
STATIC_COMMAND("rstgen_test10", rstgen_global_dbg_rst_test_help, (console_cmd)&do_rstgen_global_dbg_rst_test)
STATIC_COMMAND("rstgen_test11", rstgen_global_wdg_rst_test_help, (console_cmd)&do_rstgen_global_wdg_rst_test)
STATIC_COMMAND("rstgen_test12", rstgen_global_pre_rst_test_help, (console_cmd)&do_rstgen_global_pre_rst_test)
STATIC_COMMAND("rstgen_test13", rstgen_self_rst_test_help, (console_cmd)&do_rstgen_self_rst_test)
STATIC_COMMAND("rstgen_test14", rstgen_other_rst_test_help, (console_cmd)&do_rstgen_other_rst_test)
STATIC_COMMAND("rstgen_test15", rstgen_core_wdg_rst_test_help, (console_cmd)&do_rstgen_core_wdg_rst_test)
STATIC_COMMAND("rstgen_test16", rstgen_core_dbg_rst_test_help, (console_cmd)&do_rstgen_core_dbg_rst_test)
STATIC_COMMAND("rstgen_test17", rstgen_core_rst_test_help, (console_cmd)&do_rstgen_core_rst_test)
STATIC_COMMAND("rstgen_test18", rstgen_module_rst_test_help, (console_cmd)&do_rstgen_module_rst_test)
STATIC_COMMAND_END(rstgentest);
#endif
APP_START(rstgen)
.flags = 0
         APP_END
