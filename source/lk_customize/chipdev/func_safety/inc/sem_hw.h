#ifndef _SEM_HW_H
#define _SEM_HW_H

/**********************************************************
 SEM signals generated from c:\projects\x9\function_safety\hw\dma_irq_v0.5.4.xlsx.
 DO NOT modify!
**********************************************************/
enum sem_signal {
    IRAM1____sig_err_int = 0,        // Level High.
    IRAM1____mul_err_int = 1,        // Level High.
    IRAM2____sig_err_int = 2,        // Level High.
    IRAM2____mul_err_int = 3,        // Level High.
    IRAM3____sig_err_int = 4,        // Level High.
    IRAM3____mul_err_int = 5,        // Level High.
    IRAM4____sig_err_int = 6,        // Level High.
    IRAM4____mul_err_int = 7,        // Level High.
    ROMC1____sig_err_int = 8,        // Level High.
    ROMC1____mul_err_int = 9,        // Level High.
    ROMC2____sig_err_int = 10,       // Level High.
    ROMC2____mul_err_int = 11,       // Level High.
    ENET1____sbd_sfty_ce_intr_o = 12,        // Level High.
    ENET1____sbd_sfty_ue_intr_o = 13,        // Level High.
    ENET2____sbd_sfty_ce_intr_o = 14,        // Level High.
    ENET2____sbd_sfty_ue_intr_o = 15,        // Level High.
    CANFD1____ipi_int_ce = 16,       // Level High. Correctable error interrupt
    CANFD1____ipi_int_nceha = 17,        // Level High. Non correctable error int host
    CANFD1____ipi_int_ncefa = 18,        // Level High. Non correctable error int internal
    CANFD2____ipi_int_ce = 19,       // Level High. Correctable error interrupt
    CANFD2____ipi_int_nceha = 20,        // Level High. Non correctable error int host
    CANFD2____ipi_int_ncefa = 21,        // Level High. Non correctable error int internal
    CANFD3____ipi_int_ce = 22,       // Level High. Correctable error interrupt
    CANFD3____ipi_int_nceha = 23,        // Level High. Non correctable error int host
    CANFD3____ipi_int_ncefa = 24,        // Level High. Non correctable error int internal
    CANFD4____ipi_int_ce = 25,       // Level High. Correctable error interrupt
    CANFD4____ipi_int_nceha = 26,        // Level High. Non correctable error int host
    CANFD4____ipi_int_ncefa = 27,        // Level High. Non correctable error int internal
    CANFD5____ipi_int_ce = 28,       // Level High. Correctable error interrupt
    CANFD5____ipi_int_nceha = 29,        // Level High. Non correctable error int host
    CANFD5____ipi_int_ncefa = 30,        // Level High. Non correctable error int internal
    CANFD6____ipi_int_ce = 31,       // Level High. Correctable error interrupt
    CANFD6____ipi_int_nceha = 32,        // Level High. Non correctable error int host
    CANFD6____ipi_int_ncefa = 33,        // Level High. Non correctable error int internal
    CANFD7____ipi_int_ce = 34,       // Level High. Correctable error interrupt
    CANFD7____ipi_int_nceha = 35,        // Level High. Non correctable error int host
    CANFD7____ipi_int_ncefa = 36,        // Level High. Non correctable error int internal
    CANFD8____ipi_int_ce = 37,       // Level High. Correctable error interrupt
    CANFD8____ipi_int_nceha = 38,        // Level High. Non correctable error int host
    CANFD8____ipi_int_ncefa = 39,        // Level High. Non correctable error int internal
    NOC_SEC____PD_SEC_mainLatentInt = 40,        // Level High.
    NOC_SEC_____PD_SEC_mainLatentIntBar = 40,        // Level Low.
    NOC_SEC____PD_SEC_mainMissionInt = 41,       // Level High.
    NOC_SEC_____PD_SEC_mainMissionIntBar = 41,       // Level Low.
    NOC_SEC____m_0_mainSglEccErr = 42,       // Pulse High. Actually m_2_mainSglEccErr
    NOC_SEC____m_0_I_mainInitiator_Timeout_Fault = 43,       // Level High.
    NOC_SEC____m_1_mainSglEccErr = 44,       // Pulse High. Actually m_3_mainSglEccErr
    NOC_SEC____m_1_I_mainInitiator_Timeout_Fault = 45,       // Level High.
    NOC_SEC____m_2_mainSglEccErr = 46,       // Pulse High. Actually m_0_mainSglEccErr
    NOC_SEC____m_2_I_mainInitiator_Timeout_Fault = 47,       // Level High.
    NOC_SEC____m_3_mainSglEccErr = 48,       // Pulse High. Actually m_1_mainSglEccErr
    NOC_SEC____m_3_I_mainInitiator_Timeout_Fault = 49,       // Level High.
    NOC_SEC____m_5_mainSglArAddrEccErr = 50,     // Pulse High.
    NOC_SEC____m_5_mainSglAwAddrEccErr = 51,     // Pulse High.
    NOC_SEC____m_5_mainSglDataEccErr = 52,       // Pulse High.
    NOC_SEC____m_5_I_mainInitiator_Timeout_Fault = 53,       // Level High.
    NOC_SEC____m_6_I_mainInitiator_Timeout_Fault = 54,       // Level High.
    NOC_SEC____m_7_I_mainInitiator_Timeout_Fault = 55,       // Level High.
    NOC_SEC____m_8_I_mainInitiator_Timeout_Fault = 56,       // Level High.
    NOC_SEC____m_9_I_mainInitiator_Timeout_Fault = 57,       // Level High.
    NOC_SEC____s_0_mainSglEccErr = 58,       // Pulse High.
    NOC_SEC____s_1_mainSglEccErr = 59,       // Pulse High.
    NOC_SEC____s_2_mainSglDataEccErr = 60,       // Pulse High.
    NOC_SEC____s_3_mainSglDataEccErr = 61,       // Pulse High.
    NOC_SEC____s_13_mainSglDataEccErr = 62,      // Pulse High.
    NOC_SEC____s_14_mainSglDataEccErr = 63,      // Pulse High.
    NOC_SEC____s_17_mainSglDataEccErr = 64,      // Pulse High.
    NOC_VSN____PD_VSN_mainLatentInt = 65,        // Level High.
    NOC_VSN_____PD_VSN_mainLatentIntBar = 65,        // Level Low.
    NOC_VSN____PD_VSN_mainMissionInt = 66,       // Level High.
    NOC_VSN_____PD_VSN_mainMissionIntBar = 66,       // Level Low.
    NOC_VSN____m_0_mainSglArAddrEccErr = 67,     // Pulse High.
    NOC_VSN____m_0_mainSglAwAddrEccErr = 67,     // Pulse High.
    NOC_VSN____m_0_mainSglDataEccErr = 67,       // Pulse High.
    NOC_VSN____m_0_I_mainInitiator_Timeout_Fault = 68,       // Level High.
    NOC_VSN____m_1_mainSglArAddrEccErr = 69,     // Pulse High.
    NOC_VSN____m_1_mainSglAwAddrEccErr = 69,     // Pulse High.
    NOC_VSN____m_1_mainSglDataEccErr = 69,       // Pulse High.
    NOC_VSN____m_1_I_mainInitiator_Timeout_Fault = 70,       // Level High.
    NOC_VSN____m_2_mainSglArAddrEccErr = 71,     // Pulse High.
    NOC_VSN____m_2_mainSglAwAddrEccErr = 71,     // Pulse High.
    NOC_VSN____m_2_mainSglDataEccErr = 71,       // Pulse High.
    NOC_VSN____m_2_I_mainInitiator_Timeout_Fault = 72,       // Level High.
    NOC_VSN____s_0_mainSglDataEccErr = 73,       // Pulse High.
    VDSP____PFaultInfoValid = 73,        // Pulse High.
    NOC_VSN____s_3_mainSglDataEccErr = 74,       // Pulse High.
    NOC_MAIN____SOC_mainLatentInt = 75,      // Level High.
    NOC_MAIN_____SOC_mainLatentIntBar = 75,      // Level Low.
    NOC_MAIN____SOC_mainMissionInt = 76,     // Level High.
    NOC_MAIN_____SOC_mainMissionIntBar = 76,     // Level Low.
    NOC_MAIN____m_0_I_mainInitiator_Timeout_Fault = 77,      // Level High.
    NOC_MAIN____m_1_I_mainInitiator_Timeout_Fault = 78,      // Level High.
    NOC_MAIN____m_2_I_mainInitiator_Timeout_Fault = 79,      // Level High.
    NOC_MAIN____m_3_I_mainInitiator_Timeout_Fault = 80,      // Level High.
    NOC_MAIN____m_4_I_mainInitiator_Timeout_Fault = 81,      // Level High.
    NOC_MAIN____m_5_I_mainInitiator_Timeout_Fault = 82,      // Level High.
    NOC_MAIN____m_6_I_mainInitiator_Timeout_Fault = 83,      // Level High.
    NOC_MAIN____m_7_I_mainInitiator_Timeout_Fault = 84,      // Level High.
    NOC_MAIN____m_8_I_mainInitiator_Timeout_Fault = 85,      // Level High.
    NOC_MAIN____m_9_I_mainInitiator_Timeout_Fault = 86,      // Level High.
    NOC_MAIN____m_10_I_mainInitiator_Timeout_Fault = 87,     // Level High.
    NOC_MAIN____m_11_mainSglArAddrEccErr = 88,       // Pulse High.
    NOC_MAIN____m_11_mainSglAwAddrEccErr = 88,       // Pulse High.
    NOC_MAIN____m_11_mainSglDataEccErr = 88,     // Pulse High.
    NOC_MAIN____m_11_I_mainInitiator_Timeout_Fault = 89,     // Level High.
    NOC_MAIN____m_12_mainSglArAddrEccErr = 90,       // Pulse High.
    NOC_MAIN____m_12_mainSglAwAddrEccErr = 90,       // Pulse High.
    NOC_MAIN____m_12_mainSglDataEccErr = 90,     // Pulse High.
    NOC_MAIN____m_12_I_mainInitiator_Timeout_Fault = 91,     // Level High.
    NOC_MAIN____m_13_I_mainInitiator_Timeout_Fault = 92,     // Level High.
    NOC_MAIN____m_14_I_mainInitiator_Timeout_Fault = 93,     // Level High.
    NOC_MAIN____m_17_I_mainInitiator_Timeout_Fault = 94,     // Level High.
    NOC_MAIN____m_18_I_mainInitiator_Timeout_Fault = 95,     // Level High.
    NOC_MAIN____m_19_I_mainInitiator_Timeout_Fault = 96,     // Level High.
    NOC_MAIN____m_20_I_mainInitiator_Timeout_Fault = 97,     // Level High.
    NOC_MAIN____m_21_I_mainInitiator_Timeout_Fault = 98,     // Level High.
    NOC_MAIN____m_22_I_mainInitiator_Timeout_Fault = 99,     // Level High.
    NOC_MAIN____m_23_I_mainInitiator_Timeout_Fault = 100,        // Level High.
    NOC_MAIN____m_24_mainSglArAddrEccErr = 101,      // Pulse High.
    NOC_MAIN____m_24_mainSglAwAddrEccErr = 101,      // Pulse High.
    NOC_MAIN____m_24_mainSglDataEccErr = 101,        // Pulse High.
    NOC_MAIN____m_16_I_mainInitiator_Timeout_Fault = 102,        // Level High.
    NOC_MAIN____m_24_I_mainInitiator_Timeout_Fault = 102,        // Level High.
    NOC_MAIN____s_0_sbp_mainSglDataEccErr = 103,     // Pulse High.
    NOC_MAIN____s_3_mainSglDataEccErr = 104,     // Pulse High.
    NOC_MAIN____s_4_mainSglDataEccErr = 105,     // Pulse High.
    NOC_MAIN____s_11_mainSglDataEccErr = 106,        // Pulse High.
    NOC_MAINB____HPIb_mainLatentInt = 107,       // Level High.
    NOC_MAINB_____SAF_mainLatentIntBar = 107,        // Level Low.
    NOC_MAINB____HPIb_mainMissionInt = 108,      // Level High.
    NOC_MAINB_____SAF_mainMissionIntBar = 108,       // Level Low.
    NOC_MAINB____m_0_mainSglArAddrEccErr = 109,      // Pulse High.
    NOC_MAINB____m_0_mainSglAwAddrEccErr = 109,      // Pulse High.
    NOC_MAINB____m_0_mainSglDataEccErr = 109,        // Pulse High.
    NOC_MAINB____m_0_I_mainInitiator_Timeout_Fault = 110,        // Level High.
    NOC_MAINB____m_1_I_mainInitiator_Timeout_Fault = 111,        // Level High.
    NOC_MAINB____m_2_I_mainInitiator_Timeout_Fault = 112,        // Level High.
    NOC_SAF____PD_SAF_mainLatentInt = 113,       // Level High.
    NOC_SAF_____PD_SAF_mainLatentIntBar = 113,       // Level Low.
    NOC_SAF____PD_SAF_mainMissionInt = 114,      // Level High.
    NOC_SAF_____PD_SAF_mainMissionIntBar = 114,      // Level Low.
    NOC_SAF____m_0_mainDupl_SglEccErr = 115,     // Pulse High.
    NOC_SAF____m_0_mainMain_SglEccErr = 116,     // Pulse High.
    NOC_SAF____m_0_I_mainInitiator_Timeout_Fault = 117,      // Level High.
    NOC_SAF____m_1_mainDupl_SglEccErr = 118,     // Pulse High.
    NOC_SAF____m_1_mainMain_SglEccErr = 119,     // Pulse High.
    NOC_SAF____m_1_I_mainInitiator_Timeout_Fault = 120,      // Level High.
    NOC_SAF____m_2_I_mainInitiator_Timeout_Fault = 121,      // Level High.
    NOC_SAF____m_3_I_mainInitiator_Timeout_Fault = 122,      // Level High.
    NOC_SAF____m_4_mainMain_SglArAddrEccErr = 123,       // Pulse High.
    NOC_SAF____m_4_mainMain_SglAwAddrEccErr = 123,       // Pulse High.
    NOC_SAF____m_4_mainMain_SglDataEccErr = 123,     // Pulse High.
    NOC_SAF____m_4_mainDupl_SglArAddrEccErr = 124,       // Pulse High.
    NOC_SAF____m_4_mainDupl_SglAwAddrEccErr = 124,       // Pulse High.
    NOC_SAF____m_4_mainDupl_SglDataEccErr = 124,     // Pulse High.
    NOC_SAF____m_4_I_mainInitiator_Timeout_Fault = 125,      // Level High.
    NOC_SAF____m_5_I_mainInitiator_Timeout_Fault = 126,      // Level High.
    NOC_SAF____m_6_I_mainInitiator_Timeout_Fault = 127,      // Level High.
    NOC_SAF____s_0_mainDupl_SglEccErr = 128,     // Pulse High.
    NOC_SAF____s_0_mainMain_SglEccErr = 129,     // Pulse High.
    NOC_SAF____s_1_mainDupl_SglDataEccErr = 130,     // Pulse High.
    NOC_SAF____s_1_mainMain_SglDataEccErr = 131,     // Pulse High.
    NOC_SAF____s_2_mainDupl_SglDataEccErr = 132,     // Pulse High.
    NOC_SAF____s_2_mainMain_SglDataEccErr = 133,     // Pulse High.
    NOC_SAF____s_4_mainDupl_SglDataEccErr = 134,     // Pulse High.
    NOC_SAF____s_4_mainMain_SglDataEccErr = 135,     // Pulse High.
    MPC_IRAM1____cor_err_int = 136,      // Level High.
    MPC_IRAM1____unc_err_int = 137,      // Level High.
    MPC_IRAM2____cor_err_int = 138,      // Level High.
    MPC_IRAM2____unc_err_int = 139,      // Level High.
    MPC_IRAM3____cor_err_int = 140,      // Level High.
    MPC_IRAM3____unc_err_int = 141,      // Level High.
    MPC_IRAM4____cor_err_int = 142,      // Level High.
    MPC_IRAM4____unc_err_int = 143,      // Level High.
    MPC_ROMC2____cor_err_int = 144,      // Level High.
    MPC_ROMC2____unc_err_int = 145,      // Level High.
    MPC_DDR____cor_err_int = 146,        // Level High.
    MPC_DDR____unc_err_int = 147,        // Level High.
    BIPC_DDR____ecc_err_single = 148,        // Level High.
    BIPC_DDR____ecc_err_multiple = 149,      // Level High.
    BIPC_VDSP____ecc_err_single = 150,       // Level High.
    BIPC_VDSP____ecc_err_multiple = 151,     // Level High.
    BIPC_ENET1____ecc_err_single = 152,      // Level High.
    BIPC_ENET1____ecc_err_multiple = 153,        // Level High.
    DDR_SS____ddr_ss_func_saf_fatl_int = 154,        // Level High.
    DDR_SS____ddr_ss_func_saf_nonfatl_int = 155,     // Level High.
    DC1____dc_ch_underrun = 156,     // Level High.
    DC1____crc32_irq = 157,      // Level High.
    DC2____dc_ch_underrun = 158,     // Level High.
    DC2____crc32_irq = 159,      // Level High.
    DC3____dc_ch_underrun = 160,     // Level High.
    DC3____crc32_irq = 161,      // Level High.
    DC4____dc_ch_underrun = 162,     // Level High.
    DC4____crc32_irq = 163,      // Level High.
    DC5____dc_ch_underrun = 164,     // Level High.
    DC5____crc32_irq = 165,      // Level High.
    DP1____dp_ch_underrun = 166,     // Level High.
    DP2____dp_ch_underrun = 167,     // Level High.
    DP3____dp_ch_underrun = 168,     // Level High.
    VDSP____ErrorCorrected = 169,        // Level High.
    VDSP____PFatalError = 170,       // Level High.
    VDSP____DoubleExceptionError = 171,      // Pulse High.
    ADSP____ErrorCorrected = 172,        // Level High.
    ADSP____PFatalError = 173,       // Level High.
    ADSP____PrefetchRamErrorUncorrected = 174,       // Pulse High.
    ADSP____PrefetchRamErrorCorrected = 175,     // Pulse High.
    ADSP____DoubleExceptionError = 176,      // Pulse High.
    PCIE_SS____o_pciex2_safety_uncorr = 177,     // Level High. PCIE non correctable error
    PCIE_SS____o_pciex2_safety_corr = 178,       // Level High. PCIE correctable error
    PCIE_SS____o_pciex1_safety_uncorr = 179,     // Level High. PCIE non correctable error
    PCIE_SS____o_pciex1_safety_corr = 180,       // Level High. PCIE correctable error
    EFUSEC____normal_err_int = 181,      // Level High.
    EFUSEC____fatal_err_int = 182,       // Level High.
    OSPI1____o_irq_corr = 183,       // Level High.
    OSPI1____o_irq_uncorr = 184,     // Level High.
    OSPI2____o_irq_corr = 185,       // Level High.
    OSPI2____o_irq_uncorr = 186,     // Level High.
    CR5_SAF____aximcorr0 = 187,      // Pulse High. data errors on reads by the AXI Master
    CR5_SAF____axiscorr0 = 188,      // Pulse High. data errors on writes to the AXI Slave
    CR5_SAF____ppxcorr0 = 189,       // Pulse High. data errors on reads by the AXI Peripheral Port
    CR5_SAF____pphcorr0 = 190,       // Pulse High. data errors on reads by the AHB Peripheral Port
    CR5_SAF____aximfatal0_0_ = 191,      // Pulse High. data errors and any parity errors on the AXI master, on a per-channel basis
    CR5_SAF____aximfatal0_1_ = 192,      // Pulse High. data errors and any parity errors on the AXI master, on a per-channel basis
    CR5_SAF____aximfatal0_2_ = 193,      // Pulse High. data errors and any parity errors on the AXI master, on a per-channel basis
    CR5_SAF____aximfatal0_3_ = 194,      // Pulse High. data errors and any parity errors on the AXI master, on a per-channel basis
    CR5_SAF____aximfatal0_4_ = 195,      // Pulse High. data errors and any parity errors on the AXI master, on a per-channel basis
    CR5_SAF____axisfatal0_0_ = 196,      // Pulse High. data errors and any parity errors on the AXI slave, on a per-channel basis
    CR5_SAF____axisfatal0_1_ = 197,      // Pulse High. data errors and any parity errors on the AXI slave, on a per-channel basis
    CR5_SAF____axisfatal0_2_ = 198,      // Pulse High. data errors and any parity errors on the AXI slave, on a per-channel basis
    CR5_SAF____axisfatal0_3_ = 199,      // Pulse High. data errors and any parity errors on the AXI slave, on a per-channel basis
    CR5_SAF____axisfatal0_4_ = 200,      // Pulse High. data errors and any parity errors on the AXI slave, on a per-channel basis
    CR5_SAF____ppxfatal0_0_ = 201,       // Pulse High. data errors and any parity errors on the AXI Peripheral Port, on a per-channel basis
    CR5_SAF____ppxfatal0_1_ = 202,       // Pulse High. data errors and any parity errors on the AXI Peripheral Port, on a per-channel basis
    CR5_SAF____ppxfatal0_2_ = 203,       // Pulse High. data errors and any parity errors on the AXI Peripheral Port, on a per-channel basis
    CR5_SAF____ppxfatal0_3_ = 204,       // Pulse High. data errors and any parity errors on the AXI Peripheral Port, on a per-channel basis
    CR5_SAF____ppxfatal0_4_ = 205,       // Pulse High. data errors and any parity errors on the AXI Peripheral Port, on a per-channel basis
    CR5_SAF____dccmout_0_ = 206,     // Pulse High. LockStep compare fail
    CR5_SAF____dccmout2_0_ = 207,        // Pulse High. LockStep compare fail
    CR5_SEC____aximcorr0 = 208,      // Pulse High. data errors on reads by the AXI Master
    CR5_SEC____axiscorr0 = 209,      // Pulse High. data errors on writes to the AXI Slave
    CR5_SEC____ppxcorr0 = 210,       // Pulse High. data errors on reads by the AXI Peripheral Port
    CR5_SEC____pphcorr0 = 211,       // Pulse High. data errors on reads by the AHB Peripheral Port
    CR5_SEC____aximfatal0_0_ = 212,      // Pulse High. data errors and any parity errors on the AXI master, on a per-channel basis
    CR5_SEC____aximfatal0_1_ = 213,      // Pulse High. data errors and any parity errors on the AXI master, on a per-channel basis
    CR5_SEC____aximfatal0_2_ = 214,      // Pulse High. data errors and any parity errors on the AXI master, on a per-channel basis
    CR5_SEC____aximfatal0_3_ = 215,      // Pulse High. data errors and any parity errors on the AXI master, on a per-channel basis
    CR5_SEC____aximfatal0_4_ = 216,      // Pulse High. data errors and any parity errors on the AXI master, on a per-channel basis
    CR5_SEC____axisfatal0_0_ = 217,      // Pulse High. data errors and any parity errors on the AXI slave, on a per-channel basis
    CR5_SEC____axisfatal0_1_ = 218,      // Pulse High. data errors and any parity errors on the AXI slave, on a per-channel basis
    CR5_SEC____axisfatal0_2_ = 219,      // Pulse High. data errors and any parity errors on the AXI slave, on a per-channel basis
    CR5_SEC____axisfatal0_3_ = 220,      // Pulse High. data errors and any parity errors on the AXI slave, on a per-channel basis
    CR5_SEC____axisfatal0_4_ = 221,      // Pulse High. data errors and any parity errors on the AXI slave, on a per-channel basis
    CR5_SEC____ppxfatal0_0_ = 222,       // Pulse High. data errors and any parity errors on the AXI Peripheral Port, on a per-channel basis
    CR5_SEC____ppxfatal0_1_ = 223,       // Pulse High. data errors and any parity errors on the AXI Peripheral Port, on a per-channel basis
    CR5_SEC____ppxfatal0_2_ = 224,       // Pulse High. data errors and any parity errors on the AXI Peripheral Port, on a per-channel basis
    CR5_SEC____ppxfatal0_3_ = 225,       // Pulse High. data errors and any parity errors on the AXI Peripheral Port, on a per-channel basis
    CR5_SEC____ppxfatal0_4_ = 226,       // Pulse High. data errors and any parity errors on the AXI Peripheral Port, on a per-channel basis
    CR5_SEC____dccmout_0_ = 227,     // Pulse High. LockStep compare fail
    CR5_SEC____dccmout2_0_ = 228,        // Pulse High. LockStep compare fail
    CR5_MP____aximcorr0 = 229,       // Pulse High. data errors on reads by the AXI Master
    CR5_MP____axiscorr0 = 230,       // Pulse High. data errors on writes to the AXI Slave
    CR5_MP____ppxcorr0 = 231,        // Pulse High. data errors on reads by the AXI Peripheral Port
    CR5_MP____pphcorr0 = 232,        // Pulse High. data errors on reads by the AHB Peripheral Port
    CR5_MP____aximfatal0_0_ = 233,       // Pulse High. data errors and any parity errors on the AXI master, on a per-channel basis
    CR5_MP____aximfatal0_1_ = 234,       // Pulse High. data errors and any parity errors on the AXI master, on a per-channel basis
    CR5_MP____aximfatal0_2_ = 235,       // Pulse High. data errors and any parity errors on the AXI master, on a per-channel basis
    CR5_MP____aximfatal0_3_ = 236,       // Pulse High. data errors and any parity errors on the AXI master, on a per-channel basis
    CR5_MP____aximfatal0_4_ = 237,       // Pulse High. data errors and any parity errors on the AXI master, on a per-channel basis
    CR5_MP____axisfatal0_0_ = 238,       // Pulse High. data errors and any parity errors on the AXI slave, on a per-channel basis
    CR5_MP____axisfatal0_1_ = 239,       // Pulse High. data errors and any parity errors on the AXI slave, on a per-channel basis
    CR5_MP____axisfatal0_2_ = 240,       // Pulse High. data errors and any parity errors on the AXI slave, on a per-channel basis
    CR5_MP____axisfatal0_3_ = 241,       // Pulse High. data errors and any parity errors on the AXI slave, on a per-channel basis
    CR5_MP____axisfatal0_4_ = 242,       // Pulse High. data errors and any parity errors on the AXI slave, on a per-channel basis
    CR5_MP____ppxfatal0_0_ = 243,        // Pulse High. data errors and any parity errors on the AXI Peripheral Port, on a per-channel basis
    CR5_MP____ppxfatal0_1_ = 244,        // Pulse High. data errors and any parity errors on the AXI Peripheral Port, on a per-channel basis
    CR5_MP____ppxfatal0_2_ = 245,        // Pulse High. data errors and any parity errors on the AXI Peripheral Port, on a per-channel basis
    CR5_MP____ppxfatal0_3_ = 246,        // Pulse High. data errors and any parity errors on the AXI Peripheral Port, on a per-channel basis
    CR5_MP____ppxfatal0_4_ = 247,        // Pulse High. data errors and any parity errors on the AXI Peripheral Port, on a per-channel basis
    CR5_MP____dccmout_0_ = 248,      // Pulse High. LockStep compare fail
    CR5_MP____dccmout2_0_ = 249,     // Pulse High. LockStep compare fail
    REMAP_CR5_SAF_AW____irq = 250,       // Level High. Remap Address field check error
    REMAP_CR5_SAF_AR____irq = 251,       // Level High. Remap Address field check error
    REMAP_CR5_SEC_AW____irq = 252,       // Level High. Remap Address field check error
    REMAP_CR5_SEC_AR____irq = 253,       // Level High. Remap Address field check error
    REMAP_CR5_MP_AW____irq = 254,        // Level High. Remap Address field check error
    REMAP_CR5_MP_AR____irq = 255,        // Level High. Remap Address field check error
    CPU1_____nFAULTIRQ_0_ = 256,     // Level Low. Fault indicator for a detected 1-bit or 2-bit ECC or Parity error in the L3 RAMs
    CPU1_____nFAULTIRQ_1_ = 257,     // Level Low. Fault indicator for a detected 1-bit or 2-bit ECC or Parity error in the Core0 L1 or L2 RAMs
    CPU1_____nFAULTIRQ_2_ = 258,     // Level Low. Fault indicator for a detected 1-bit or 2-bit ECC or Parity error in the Core1 L1 or L2 RAMs
    CPU1_____nFAULTIRQ_3_ = 259,     // Level Low. Fault indicator for a detected 1-bit or 2-bit ECC or Parity error in the Core2 L1 or L2 RAMs
    CPU1_____nFAULTIRQ_4_ = 260,     // Level Low. Fault indicator for a detected 1-bit or 2-bit ECC or Parity error in the Core3 L1 or L2 RAMs
    CPU1_____nFAULTIRQ_5_ = 261,     // Level Low. Fault indicator for a detected 1-bit or 2-bit ECC or Parity error in the Core2 L1 or L2 RAMs
    CPU1_____nFAULTIRQ_6_ = 262,     // Level Low. Fault indicator for a detected 1-bit or 2-bit ECC or Parity error in the Core3 L1 or L2 RAMs
    CPU1_____nERRIRQ_0_ = 263,       // Level Low. Error indicator for an ECC error that causes potential data corruption or loss of coherency in the L3 RAMs
    CPU1_____nERRIRQ_1_ = 264,       // Level Low. Error indicator for an ECC error that causes potential data corruption or loss of coherency in the Core0 L1 or L2 RAMs
    CPU1_____nERRIRQ_2_ = 265,       // Level Low. Error indicator for an ECC error that causes potential data corruption or loss of coherency in the Core1 L1 or L2 RAMs
    CPU1_____nERRIRQ_3_ = 266,       // Level Low. Error indicator for an ECC error that causes potential data corruption or loss of coherency in the Core2 L1 or L2 RAMs
    CPU1_____nERRIRQ_4_ = 267,       // Level Low. Error indicator for an ECC error that causes potential data corruption or loss of coherency in the Core3 L1 or L2 RAMs
    CPU1_____nERRIRQ_5_ = 268,       // Level Low. Error indicator for an ECC error that causes potential data corruption or loss of coherency in the Core2 L1 or L2 RAMs
    CPU1_____nERRIRQ_6_ = 269,       // Level Low. Error indicator for an ECC error that causes potential data corruption or loss of coherency in the Core3 L1 or L2 RAMs
    CPU2_____nFAULTIRQ_0_ = 270,     // Level Low. Fault indicator for a detected 1-bit or 2-bit ECC or Parity error in the L3 RAMs
    CPU2_____nFAULTIRQ_1_ = 271,     // Level Low. Fault indicator for a detected 1-bit or 2-bit ECC or Parity error in the Core0 L1 or L2 RAMs
    CPU2_____nERRIRQ_0_ = 272,       // Level Low. Error indicator for an ECC error that causes potential data corruption or loss of coherency in the L3 RAMs
    CPU2_____nERRIRQ_1_ = 273,       // Level Low. Error indicator for an ECC error that causes potential data corruption or loss of coherency in the Core0 L1 or L2 RAMs
    RC_24M____saf_osc_unst_int = 274,        // Level High. osc un-stable according to saf xtal
    RC_24M____saf_xtal_unst_int = 275,       // Level High. safety xtal un-stable
    RC_24M____saf_xtal_lost_int = 276,       // Level High. safety xtal lost
    RC_24M____soc_osc_unst_int = 277,        // Level High. osc un-stable according to soc xtal
    RC_24M____soc_xtal_unst_int = 278,       // Level High. soc xtal un-stable
    RC_24M____soc_xtal_lost_int = 279,       // Level High. soc xtal lost
    XTAL_SAF____fr_unst_int = 280,       // Level High. xtal_saf un-stable
    XTAL_SAF____to_unst_int = 281,       // Level High. xtal_ap un-stable
    XTAL_SAF____to_lost_int = 282,       // Level High. xtal_ap lost
    XTAL_AP____fr_unst_int = 283,        // Level High. xtal_ap un-stable
    XTAL_AP____to_unst_int = 284,        // Level High. xtal_saf un-stable
    XTAL_AP____to_lost_int = 285,        // Level High. xtal_saf lost
    PWR_LAT_SAF____rtc_power_lat = 286,      // Level High. RTC domain power fail
    PWR_LAT_SAF____ap_power_lat = 287,       // Level High. AP domain power fail
    PWR_LAT_SAF____saf_power_lat = 288,      // Level High. SAF power fail
    PWR_LAT_RTC____saf_power_lat = 289,      // Level High. SAF domain power fail
    PWR_LAT_RTC____ap_power_lat = 290,       // Level High. AP domain power fail
    WDT1____ovflow_int = 291,        // Level High.
    WDT1____ill_win_refr_int = 292,      // Level High.
    WDT1____ill_seq_refr_int = 293,      // Level High.
    WDT2____ovflow_int = 294,        // Level High.
    WDT2____ill_win_refr_int = 295,      // Level High.
    WDT2____ill_seq_refr_int = 296,      // Level High.
    WDT3____ovflow_int = 297,        // Level High.
    WDT3____ill_win_refr_int = 298,      // Level High.
    WDT3____ill_seq_refr_int = 299,      // Level High.
    WDT4____ovflow_int = 300,        // Level High.
    WDT4____ill_win_refr_int = 301,      // Level High.
    WDT4____ill_seq_refr_int = 302,      // Level High.
    WDT5____ovflow_int = 303,        // Level High.
    WDT5____ill_win_refr_int = 304,      // Level High.
    WDT5____ill_seq_refr_int = 305,      // Level High.
    WDT6____ovflow_int = 306,        // Level High.
    WDT6____ill_win_refr_int = 307,      // Level High.
    WDT6____ill_seq_refr_int = 308,      // Level High.
    WDT7____ovflow_int = 309,        // Level High.
    WDT7____ill_win_refr_int = 310,      // Level High.
    WDT7____ill_seq_refr_int = 311,      // Level High.
    WDT8____ovflow_int = 312,        // Level High.
    WDT8____ill_win_refr_int = 313,      // Level High.
    WDT8____ill_seq_refr_int = 314,      // Level High.
    PVT_SNS_SEC____pvt_int_0 = 315,      // Level High.
    PVT_SNS_SEC____pvt_int_1 = 316,      // Level High.
    PVT_SNS_SAF____pvt_int_0 = 317,      // Level High.
    PVT_SNS_SAF____pvt_int_1 = 318,      // Level High.
};

enum sem_monitor_sig {
    EFUSE_MISC_CFG_0_ = 0,      // SAFE_DID_0
    EFUSE_MISC_CFG_1_ = 1,      // SAFE_DID_1
    EFUSE_MISC_CFG_2_ = 2,      // SAFE_DID_2
    EFUSE_MISC_CFG_3_ = 3,      // SAFE_DID_3
    EFUSE_MISC_CFG_4_ = 4,      // SAFE_DID_OE
    EFUSE_MISC_CFG_5_ = 5,      // SAF_DSEL_EN
    EFUSE_MISC_CFG_6_ = 6,      // GPU_ASTC_DISABLE
    EFUSE_MISC_CFG_7_ = 7,      // PROD_ENABLE
    EFUSE_MISC_CFG_8_ = 8,      // IRAM1_ECC_DISABLE
    EFUSE_MISC_CFG_9_ = 9,      // IRAM2_ECC_DISABLE
    EFUSE_MISC_CFG_10_ = 10,        // IRAM3_ECC_DISABLE
    EFUSE_MISC_CFG_11_ = 11,        // IRAM4_ECC_DISABLE
    EFUSE_MISC_CFG_12_ = 12,        // EFUSE_MISC_CFG[12]
    EFUSE_MISC_CFG_13_ = 13,        // EFUSE_MISC_CFG[13]
    EFUSE_MISC_CFG_14_ = 14,        // EFUSE_MISC_CFG[14]
    EFUSE_MISC_CFG_15_ = 15,        // EFUSE_MISC_CFG[15]
    EFUSE_MISC_CFG_16_ = 16,        // ENET1_DSEL
    EFUSE_MISC_CFG_17_ = 17,        // OSPI1_DSEL
    EFUSE_MISC_CFG_18_ = 18,        // I2S_SC1_DSEL
    EFUSE_MISC_CFG_19_ = 19,        // I2S_SC2_DSEL
    EFUSE_MISC_CFG_20_ = 20,        // CAN1_DSEL
    EFUSE_MISC_CFG_21_ = 21,        // CAN2_DSEL
    EFUSE_MISC_CFG_22_ = 22,        // CAN3_DSEL
    EFUSE_MISC_CFG_23_ = 23,        // CAN4_DSEL
    EFUSE_MISC_CFG_24_ = 24,        // I2C1_DSEL
    EFUSE_MISC_CFG_25_ = 25,        // I2C2_DSEL
    EFUSE_MISC_CFG_26_ = 26,        // I2C3_DSEL
    EFUSE_MISC_CFG_27_ = 27,        // I2C4_DSEL
    EFUSE_MISC_CFG_28_ = 28,        // SPI1_DSEL
    EFUSE_MISC_CFG_29_ = 29,        // SPI2_DSEL
    EFUSE_MISC_CFG_30_ = 30,        // SPI3_DSEL
    EFUSE_MISC_CFG_31_ = 31,        // SPI4_DSEL
    EFUSE_MISC_CFG_32_ = 32,        // UART1_DSEL
    EFUSE_MISC_CFG_33_ = 33,        // UART2_DSEL
    EFUSE_MISC_CFG_34_ = 34,        // UART3_DSEL
    EFUSE_MISC_CFG_35_ = 35,        // UART4_DSEL
    EFUSE_MISC_CFG_36_ = 36,        // UART5_DSEL
    EFUSE_MISC_CFG_37_ = 37,        // UART6_DSEL
    EFUSE_MISC_CFG_38_ = 38,        // UART7_DSEL
    EFUSE_MISC_CFG_39_ = 39,        // UART8_DSEL
    EFUSE_MISC_CFG_40_ = 40,        // XTAL_SAFTY_DSEL
    EFUSE_MISC_CFG_41_ = 41,        // IRAM1_DSEL
    EFUSE_MISC_CFG_42_ = 42,        // PLL1_DSEL
    EFUSE_MISC_CFG_43_ = 43,        // PLL2_DSEL
    EFUSE_MISC_CFG_44_ = 44,        // RC_24M_DSEL
    EFUSE_MISC_CFG_45_ = 45,        // PWRCTRL_DSEL
    EFUSE_MISC_CFG_46_ = 46,        // BIPC_ENET_DSEL
    EFUSE_MISC_CFG_47_ = 47,        // WDT1_DSEL
    EFUSE_MISC_CFG_48_ = 48,        // DMA1_DSEL
    EFUSE_MISC_CFG_49_ = 49,        // RPC_SAF_DSEL
    EFUSE_MISC_CFG_50_ = 50,        // EIC_SAF_DSEL
    EFUSE_MISC_CFG_51_ = 51,        // SEM1_DSEL
    EFUSE_MISC_CFG_52_ = 52,        // TIMER1_DSEL
    EFUSE_MISC_CFG_53_ = 53,        // TIMER2_DSEL
    EFUSE_MISC_CFG_54_ = 54,        // PWM1_DSEL
    EFUSE_MISC_CFG_55_ = 55,        // PWM2_DSEL
    EFUSE_MISC_CFG_56_ = 56,        // PVT_SNS_SAF_DSEL
    EFUSE_MISC_CFG_57_ = 57,        // TM_DSEL
    EFUSE_MISC_CFG_58_ = 58,        // IOMUXC_RTC_DSEL
    EFUSE_MISC_CFG_59_ = 59,        // RC_RTC_DSEL
    EFUSE_MISC_CFG_60_ = 60,        // XTAL_RTC_DSEL
    EFUSE_MISC_CFG_61_ = 61,        // RTC1_DSEL
    EFUSE_MISC_CFG_62_ = 62,        // RTC2_DSEL
    EFUSE_MISC_CFG_63_ = 63,        // PMU_DSEL
    EFUSE_MISC_CFG_64_ = 64,        // RSTGEN_RTC_DSEL
    EFUSE_MISC_CFG_65_ = 65,        // SEC_STORAGE1_DSEL
    EFUSE_MISC_CFG_66_ = 66,        // SEC_STORAGE2_DSEL
    EFUSE_MISC_CFG_67_ = 67,        // CE1_DSEL
    EFUSE_MISC_CFG_68_ = 68,        // EFUSE_MISC_CFG[68]
    EFUSE_MISC_CFG_69_ = 69,        // EFUSE_MISC_CFG[69]
    EFUSE_MISC_CFG_70_ = 70,        // EFUSE_MISC_CFG[70]
    EFUSE_MISC_CFG_71_ = 71,        // EFUSE_MISC_CFG[71]
    EFUSE_MISC_CFG_72_ = 72,        // EFUSE_MISC_CFG[72]
    EFUSE_MISC_CFG_73_ = 73,        // EFUSE_MISC_CFG[73]
    EFUSE_MISC_CFG_74_ = 74,        // EFUSE_MISC_CFG[74]
    EFUSE_MISC_CFG_75_ = 75,        // EFUSE_MISC_CFG[75]
    EFUSE_MISC_CFG_76_ = 76,        // EFUSE_MISC_CFG[76]
    EFUSE_MISC_CFG_77_ = 77,        // EFUSE_MISC_CFG[77]
    EFUSE_MISC_CFG_78_ = 78,        // EFUSE_MISC_CFG[78]
    EFUSE_MISC_CFG_79_ = 79,        // EFUSE_MISC_CFG[79]
    EFUSE_MISC_CFG_80_ = 80,        // EFUSE_MISC_CFG[80]
    EFUSE_MISC_CFG_81_ = 81,        // EFUSE_MISC_CFG[81]
    EFUSE_MISC_CFG_82_ = 82,        // EFUSE_MISC_CFG[82]
    EFUSE_MISC_CFG_83_ = 83,        // EFUSE_MISC_CFG[83]
    EFUSE_MISC_CFG_84_ = 84,        // EFUSE_MISC_CFG[84]
    EFUSE_MISC_CFG_85_ = 85,        // EFUSE_MISC_CFG[85]
    EFUSE_MISC_CFG_86_ = 86,        // EFUSE_MISC_CFG[86]
    EFUSE_MISC_CFG_87_ = 87,        // EFUSE_MISC_CFG[87]
    EFUSE_MISC_CFG_88_ = 88,        // EFUSE_MISC_CFG[88]
    EFUSE_MISC_CFG_89_ = 89,        // EFUSE_MISC_CFG[89]
    EFUSE_MISC_CFG_90_ = 90,        // EFUSE_MISC_CFG[90]
    EFUSE_MISC_CFG_91_ = 91,        // EFUSE_MISC_CFG[91]
    EFUSE_MISC_CFG_92_ = 92,        // EFUSE_MISC_CFG[92]
    EFUSE_MISC_CFG_93_ = 93,        // EFUSE_MISC_CFG[93]
    EFUSE_MISC_CFG_94_ = 94,        // EFUSE_MISC_CFG[94]
    EFUSE_MISC_CFG_95_ = 95,        // EFUSE_MISC_CFG[95]
    EFUSE_MISC_CFG_96_ = 96,        // EFUSE_MISC_CFG[96]
    EFUSE_MISC_CFG_97_ = 97,        // EFUSE_MISC_CFG[97]
    EFUSE_MISC_CFG_98_ = 98,        // EFUSE_MISC_CFG[98]
    EFUSE_MISC_CFG_99_ = 99,        // EFUSE_MISC_CFG[99]
    EFUSE_MISC_CFG_100_ = 100,      // EFUSE_MISC_CFG[100]
    EFUSE_MISC_CFG_101_ = 101,      // EFUSE_MISC_CFG[101]
    EFUSE_MISC_CFG_102_ = 102,      // EFUSE_MISC_CFG[102]
    EFUSE_MISC_CFG_103_ = 103,      // EFUSE_MISC_CFG[103]
    EFUSE_MISC_CFG_104_ = 104,      // PVT_SAF_SENS_EN
    EFUSE_MISC_CFG_105_ = 105,      // PVT_SEC_SENS_EN
    EFUSE_MISC_CFG_106_ = 106,      // SDBG_MODE
    EFUSE_MISC_CFG_107_ = 107,      // XTAL_XCEHCK_DISABLE
    EFUSE_MISC_CFG_108_ = 108,      // USER_MAC_DOMAIN_DISABLE
    EFUSE_MISC_CFG_109_ = 109,      // USER_MAC_DISABLE
    EFUSE_MISC_CFG_110_ = 110,      // USER_CR5_SAF_DISABLE
    EFUSE_MISC_CFG_111_ = 111,      // PARALLEL_BOOT_DISABLE
    EFUSE_MISC_CFG_112_ = 112,      // VDSP_DBG_DISABLE
    EFUSE_MISC_CFG_113_ = 113,      // ADSP_DBG_DISABLE
    EFUSE_MISC_CFG_114_ = 114,      // GPU1_DBG_DISABLE
    EFUSE_MISC_CFG_115_ = 115,      // GPU2_DBG_DISABLE
    EFUSE_MISC_CFG_116_ = 116,      // EFUSE_SE_AP_SEP
    EFUSE_MISC_CFG_117_ = 117,      // WDT1_DEFAULT_ENABLE
    EFUSE_MISC_CFG_118_ = 118,      // SEC_SAFETY_DISABLE
    EFUSE_MISC_CFG_119_ = 119,      // DFM_DISABLE
    EFUSE_MISC_CFG_120_ = 120,      // AP1_DBG_DISABLE
    EFUSE_MISC_CFG_121_ = 121,      // AP2_DBG_DISABLE
    EFUSE_MISC_CFG_122_ = 122,      // R5_SAF_DBG_DISABLE
    EFUSE_MISC_CFG_123_ = 123,      // R5_SEC_DBG_DISABLE
    EFUSE_MISC_CFG_124_ = 124,      // R5_MP_DBG_DISABLE
    EFUSE_MISC_CFG_125_ = 125,      // ALL_DBG_DISABLE
    EFUSE_MISC_CFG_126_ = 126,      // PTB_DISABLE
    EFUSE_MISC_CFG_127_ = 127,      // JTAG_DISABLE
    APBMUX_SEM1_SEL = 128,      // APBMUX selector control of SEM1
    APBMUX_CE1_SEL = 129,       // APBMUX selector control of CE1
    APBMUX_PWRCTRL_SEL = 130,       // APBMUX selector control of PWRCTRL
    APBMUX_OSPI1_SEL = 131,     // APBMUX selector control of OSPI1
    APBMUX_CANFD1_SEL = 132,        // APBMUX selector control of CANFD1
    APBMUX_CANFD2_SEL = 133,        // APBMUX selector control of CANFD2
    APBMUX_CANFD3_SEL = 134,        // APBMUX selector control of CANFD3
    APBMUX_CANFD4_SEL = 135,        // APBMUX selector control of CANFD4
    APBMUX_I2C1_SEL = 136,      // APBMUX selector control of I2C1
    APBMUX_I2C2_SEL = 137,      // APBMUX selector control of I2C2
    APBMUX_I2C3_SEL = 138,      // APBMUX selector control of I2C3
    APBMUX_I2C4_SEL = 139,      // APBMUX selector control of I2C4
    APBMUX_UART1_SEL = 140,     // APBMUX selector control of UART1
    APBMUX_UART2_SEL = 141,     // APBMUX selector control of UART2
    APBMUX_UART3_SEL = 142,     // APBMUX selector control of UART3
    APBMUX_UART4_SEL = 143,     // APBMUX selector control of UART4
    APBMUX_UART5_SEL = 144,     // APBMUX selector control of UART5
    APBMUX_UART6_SEL = 145,     // APBMUX selector control of UART6
    APBMUX_UART7_SEL = 146,     // APBMUX selector control of UART7
    APBMUX_UART8_SEL = 147,     // APBMUX selector control of UART8
    APBMUX_ENET1_SEL = 148,     // APBMUX selector control of ENET1
    APBMUX_TMR1_SEL = 149,      // APBMUX selector control of TMR1
    APBMUX_TMR2_SEL = 150,      // APBMUX selector control of TMR2
    APBMUX_PWM1_SEL = 151,      // APBMUX selector control of PWM1
    APBMUX_PWM2_SEL = 152,      // APBMUX selector control of PWM2
    APBMUX_WDT1_SEL = 153,      // APBMUX selector control of WDT1
    APBMUX_DMA1_SEL = 154,      // APBMUX selector control of DMA1
    APBMUX_RC24_SEL = 155,      // APBMUX selector control of RC24
    APBMUX_PLL1_SEL = 156,      // APBMUX selector control of PLL1
    APBMUX_PLL2_SEL = 157,      // APBMUX selector control of PLL2
    APBMUX_PVT_SNS_SAF_SEL = 158,       // APBMUX selector control of PVT_SNS_SAF
    APBMUX_BIPC_ENET1_SEL = 159,        // APBMUX selector control of BIPC_ENET1
    APBMUX_I2SSC1_SEL = 160,        // APBMUX selector control of I2SSC1
    APBMUX_I2SSC2_SEL = 161,        // APBMUX selector control of I2SSC2
    APBMUX_IRAM1_SEL = 162,     // APBMUX selector control of IRAM1
    APBMUX_SPI1_SEL = 163,      // APBMUX selector control of SPI1
    APBMUX_SPI2_SEL = 164,      // APBMUX selector control of SPI2
    APBMUX_SPI3_SEL = 165,      // APBMUX selector control of SPI3
    APBMUX_SPI4_SEL = 166,      // APBMUX selector control of SPI4
    APBMUX_RPC_SAF_SEL = 167,       // APBMUX selector control of RPC_SAF
    APBMUX_XTAL_SAF_SEL = 168,      // APBMUX selector control of XTAL_SAF
    APBMUX_EIC_SAF_SEL = 169,       // APBMUX selector control of EIC_SAF
    AXIMUX_IRAM1_SEL = 170,     // AXIMUX selector control of IRAM1
    AXIMUX_OSPI1_SEL = 171,     // AXIMUX selector control of OSPI1
    AXIMUX_DMA1_SEL = 172,      // AXIMUX selector control of DMA1
    AXIMUX_CE1_SEL = 173,       // AXIMUX selector control of CE1
    APBMUX_TM_SEL = 174,        // APBMUX selector control of TM
    APBMUX_RC_RTC_SEL = 175,        // APBMUX selector control of RC_RTC
    APBMUX_IOMUXC_RTC_SEL = 176,        // APBMUX selector control of IOMUXC_RTC
    APBMUX_XTAL_RTC_SEL = 177,      // APBMUX selector control of XTAL_RTC
    APBMUX_RTC1_SEL = 178,      // APBMUX selector control of RTC1
    APBMUX_RTC2_SEL = 179,      // APBMUX selector control of RTC2
    APBMUX_PMU_SEL = 180,       // APBMUX selector control of PMU
    APBMUX_RSTGEN_RTC_SEL = 181,        // APBMUX selector control of RSTGEN_RTC
    APBMUX_SEC_STORAGE1_SEL = 182,      // APBMUX selector control of SEC_STORAGE1
    APBMUX_SEC_STORAGE2_SEL = 183,      // APBMUX selector control of SEC_STORAGE2
};
#endif