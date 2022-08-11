
#ifndef __RSTGEN_DRV_TEST_H__
#define __RSTGEN_DRV_TEST_H__


/*rstgen ap core index*/
typedef enum _rstgen_rst_type
{
    drv_rst_type_gic = 0U,
    drv_rst_type_cpu = 1U,
    drv_rst_type_oth = 2U,
} rstgen_rst_type;


/*rstgen ap core index*/
typedef enum _rstgen_drv_core_idx
{
    drv_core_vdsp = 0U,
    drv_core_cr5_sec = 1U,
    drv_core_cr5_mp = 2U,
    drv_core_cpu1_core_all = 3U,
    drv_core_cpu2_core = 4U,
    drv_core_adsp = 5U,
} rstgen_drv_core_idx;

/*rstgen safety module index*/
typedef enum _rstgen_drv_saf_module_idx
{
    drv_saf_module_gic1 = 0U,
    drv_saf_module_ospi1 = 1U,
    drv_saf_module_enet1 = 2U,
    drv_saf_module_i2s_sc1 = 3U,
    drv_saf_module_i2s_sc2 = 4U,
    drv_saf_module_canfd1 = 5U,
    drv_saf_module_canfd2 = 6U,
    drv_saf_module_canfd3 = 7U,
    drv_saf_module_canfd4 = 8U,
    drv_saf_module_sem1 = 9U,
    drv_saf_module_sem2 = 10U,
} rstgen_drv_saf_module_idx;

/*rstgen ap modue index*/
typedef enum _rstgen_drv_ap_module_idx
{
    drv_ap_module_ospi2 = 0U,
    drv_ap_module_i2s_sc3 = 1U,
    drv_ap_module_i2s_sc4 = 2U,
    drv_ap_module_i2s_sc5 = 3U,
    drv_ap_module_i2s_sc6 = 4U,
    drv_ap_module_i2s_sc7 = 5U,
    drv_ap_module_i2s_sc8 = 6U,
    drv_ap_module_i2s_mc1 = 7U,
    drv_ap_module_i2s_mc2 = 8U,
    drv_ap_module_canfd5 = 9U,
    drv_ap_module_canfd6 = 10U,
    drv_ap_module_canfd7 = 11U,
    drv_ap_module_canfd8 = 12U,
    drv_ap_module_enet2 = 13U,
    drv_ap_module_mshc1 = 14U,
    drv_ap_module_mshc2 = 15U,
    drv_ap_module_mshc3 = 16U,
    drv_ap_module_mshc4 = 17U,
    drv_ap_module_adsp = 18U,
    drv_ap_module_gic2 = 19U,
    drv_ap_module_gic3 = 20U,
    drv_ap_module_cpu1_core0_warm = 21U,
    drv_ap_module_cpu1_core1_warm = 22U,
    drv_ap_module_cpu1_core2_warm = 23U,
    drv_ap_module_cpu1_core3_warm = 24U,
    drv_ap_module_cpu1_core4_warm = 25U,
    drv_ap_module_cpu1_core5_warm = 26U,
    drv_ap_module_cpu1_scu_warm = 27U,
    drv_ap_module_ddr_ss = 29U,
    drv_ap_module_ddr_sw0 = 30U,
    drv_ap_module_ddr_sw1 = 31U,
    drv_ap_module_ddr_sw2 = 32U,
    drv_ap_module_gic4 = 33U,
    drv_ap_module_gic5 = 34U,
    drv_ap_module_cssys_treset_n = 36U,
    drv_ap_module_nna = 37U,
    drv_ap_module_vdsp_DReset = 38U,
    drv_ap_module_vpu1 = 39U,
    drv_ap_module_vpu2 = 40U,
    drv_ap_module_mjpeg = 41U,
    drv_ap_module_cpu1_ss = 45U,
    drv_ap_module_cpu2_ss = 46U,
    drv_ap_module_mipi_csi1 = 56U,
    drv_ap_module_mipi_csi2 = 57U,
    drv_ap_module_mipi_csi3 = 58U,
    drv_ap_module_mipi_dsi1 = 59U,
    drv_ap_module_mipi_dsi2 = 60U,
    drv_ap_module_dc1 = 61U,
    drv_ap_module_dc2 = 62U,
    drv_ap_module_dc3 = 63U,
    drv_ap_module_dc4 = 64U,
    drv_ap_module_dc5 = 65U,
    drv_ap_module_dp1 = 66U,
    drv_ap_module_dp2 = 67U,
    drv_ap_module_dp3 = 68U,
    drv_ap_module_lvds_ss = 69U,
    drv_ap_module_csi1 = 70U,
    drv_ap_module_csi2 = 71U,
    drv_ap_module_csi3 = 72U,
    drv_ap_module_disp_mux = 73U,
    drv_ap_module_g2d1 = 74U,
    drv_ap_module_g2d2 = 75U,
    drv_ap_module_gpu1_core = 80U,
    drv_ap_module_gpu1_ss = 81U,
    drv_ap_module_gpu2_core = 82U,
    drv_ap_module_gpu2_ss = 83U,
    drv_ap_module_dbg_req = 89U,
    drv_module_canfd9 = 90U,
    drv_module_canfd10 = 91U,
    drv_module_canfd11 = 92U,
    drv_module_canfd12 = 93U,
    drv_module_canfd13 = 94U,
    drv_module_canfd14 = 95U,
    drv_module_canfd15 = 96U,
    drv_module_canfd16 = 97U,
    drv_module_canfd17 = 98U,
    drv_module_canfd18 = 99U,
    drv_module_canfd19 = 100U,
    drv_module_canfd20 = 101U,
    drv_module_max,
} rstgen_drv_ap_module_idx;

bool rstgen_dump_all_reg_for_test(vaddr_t base,uint32_t core_idx,uint32_t module_idx,uint32_t iso_idx);
bool rstgen_core_readonlyreg_check_test(vaddr_t base,uint32_t core_idx);
bool rstgen_module_readonlyreg_check_test(vaddr_t base,uint32_t module_idx);
bool rstgen_global_rw_reg_check_test(vaddr_t base);
bool rstgen_core_rw_reg_check_test(vaddr_t base,uint32_t core_idx);
bool rstgen_module_rw_reg_check_test(vaddr_t base,uint32_t module_idx);
bool rstgen_iso_rw_reg_check_test(vaddr_t base,uint32_t iso_idx);
bool rstgen_general_rw_reg_check_test(vaddr_t base,uint32_t general_idx);
bool rstgen_module_rst_test(vaddr_t base,uint32_t module_idx);
bool rstgen_clear_rst_sta(vaddr_t base);
#endif
