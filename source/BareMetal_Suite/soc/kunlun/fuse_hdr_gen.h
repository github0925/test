/********************************************************
 *          Copyright(c) 2019   Semidrive               *
 *******************************************************/

/* Generated code! Do not modify manually. */

#ifndef __FUSE_HDR_GEN_H__
#define __FUSE_HDR_GEN_H__

#define FUSE_BOOT_PINS() \
    ((soc_read_fuse(173) >> 26) & 0x3fu)

#define FUSE_INNER_BOOT() \
    ((soc_read_fuse(173) >> 25) & 0x1u)

#define FUSE_USB_PRVSN_BT_DIS() \
    ((soc_read_fuse(173) >> 24) & 0x1u)

#define FUSE_BOOT_FLAG_PIN_DIS() \
    ((soc_read_fuse(173) >> 23) & 0x1u)

#define FUSE_EVENT_LOG_DIS() \
    ((soc_read_fuse(173) >> 22) & 0x1u)

#define FUSE_SAFETY_WDOG_EN() \
    ((soc_read_fuse(173) >> 21) & 0x1u)

#define FUSE_SEC_WDOG_EN() \
    ((soc_read_fuse(173) >> 20) & 0x1u)

#define FUSE_SAFETY_HANDORVER_DIS() \
    ((soc_read_fuse(173) >> 18) & 0x1u)

#define FUSE_DONOT_PRVSN_BOOT_ON_FAIL() \
    ((soc_read_fuse(173) >> 17) & 0x1u)

#define FUSE_KICK_SEC_CORE() \
    ((soc_read_fuse(173) >> 16) & 0x1u)

#define FUSE_USR_SAFETY_ROMC_SELFTEST() \
    ((soc_read_fuse(173) >> 15) & 0x1u)

#define FUSE_USR_SEC_ROMC_SELFTEST() \
    ((soc_read_fuse(173) >> 14) & 0x1u)

#define FUSE_CAK_INSTALLED_1() \
    ((soc_read_fuse(173) >> 7) & 0x1u)

#define FUSE_SAFETY_IMG_PRVSN() \
    ((soc_read_fuse(174) >> 31) & 0x1u)

#define FUSE_EMMC_CMD1_ARG_STATUS_BIT() \
    ((soc_read_fuse(174) >> 30) & 0x1u)

#define FUSE_EMMC_CMD1_ARG_SINGLE_VOL() \
    ((soc_read_fuse(174) >> 29) & 0x1u)

#define FUSE_TOGGLE_SD_RST() \
    ((soc_read_fuse(174) >> 28) & 0x1u)

#define FUSE_TOGGLE_EMMC_RST() \
    ((soc_read_fuse(174) >> 27) & 0x1u)

#define FUSE_MMC_3P3V() \
    ((soc_read_fuse(174) >> 26) & 0x1u)

#define FUSE_SDMMC_HIGH_SPEED_BT() \
    ((soc_read_fuse(174) >> 25) & 0x1u)

#define FUSE_MMC_4BITS() \
    ((soc_read_fuse(174) >> 24) & 0x1u)

#define FUSE_OSPI2_DQS_MUXING() \
    ((soc_read_fuse(174) >> 23) & 0x1u)

#define FUSE_SPINOR_TOGGLE_RST() \
    ((soc_read_fuse(174) >> 22) & 0x1u)

#define FUSE_HYPERFLASH() \
    ((soc_read_fuse(174) >> 21) & 0x1u)

#define FUSE_MSHC_PAD_DRIVE_STRENGTH() \
    ((soc_read_fuse(174) >> 16) & 0xfu)

#define FUSE_USB_REF_CLK_FRM_PAD() \
    ((soc_read_fuse(174) >> 8) & 0x1u)

#define FUSE_CAK_INSTALLED_2() \
    ((soc_read_fuse(174) >> 7) & 0x1u)

#define FUSE_USR_SAFE_FREQ_BOOT() \
    ((soc_read_fuse(174) >> 0) & 0x1u)

#define FUSE_XTAL_SAF_XGAIN() \
    ((soc_read_fuse(20) >> 12) & 0x3u)

#define FUSE_XTAL_AP_XGAIN() \
    ((soc_read_fuse(20) >> 14) & 0x3u)

#define FUSE_MFG_DISABLE() \
    ((soc_read_fuse(10) >> 24) & 0x1u)

#define FUSE_CR5_SAF_DISABLE() \
    ((soc_read_fuse(10) >> 28) & 0x1u)

#define FUSE_GLB_ECC_DIS() \
    ((soc_read_fuse(11) >> 15) & 0x1u)

#define FUSE_USB1_DISABLE() \
    ((soc_read_fuse(11) >> 6) & 0x1u)

#define FUSE_FA_ENABLE() \
    ((soc_read_fuse(168) >> 0) & 0x1u)

#define FUSE_SAF_DSEL_EN() \
    ((soc_read_fuse(169) >> 5) & 0x1u)

#define FUSE_PROD_ENABLE() \
    ((soc_read_fuse(169) >> 7) & 0x1u)

#define FUSE_IRAM1_ECC_DIS() \
    ((soc_read_fuse(169) >> 8) & 0x1u)

#define FUSE_IRAM2_ECC_DIS() \
    ((soc_read_fuse(169) >> 9) & 0x1u)

#define FUSE_IRAM3_ECC_DIS() \
    ((soc_read_fuse(169) >> 10) & 0x1u)

#define FUSE_IRAM4_ECC_DIS() \
    ((soc_read_fuse(169) >> 11) & 0x1u)

#define FUSE_OSPI1_DSEL() \
    ((soc_read_fuse(169) >> 17) & 0x1u)

#define FUSE_XTAL_SAFETY_DSEL() \
    ((soc_read_fuse(170) >> 8) & 0x1u)

#define FUSE_IRAM1_DSEL() \
    ((soc_read_fuse(170) >> 9) & 0x1u)

#define FUSE_PLL1_DSEL() \
    ((soc_read_fuse(170) >> 10) & 0x1u)

#define FUSE_PLL2_DSEL() \
    ((soc_read_fuse(170) >> 11) & 0x1u)

#define FUSE_RC_24M_DSEL() \
    ((soc_read_fuse(170) >> 12) & 0x1u)

#define FUSE_WDT1_DSEL() \
    ((soc_read_fuse(170) >> 15) & 0x1u)

#define FUSE_RPC_SAF_DSEL() \
    ((soc_read_fuse(170) >> 17) & 0x1u)

#define FUSE_EIC_SAF_DSEL() \
    ((soc_read_fuse(170) >> 18) & 0x1u)

#define FUSE_TIMER1_DSEL() \
    ((soc_read_fuse(170) >> 20) & 0x1u)

#define FUSE_CE1_DSEL() \
    ((soc_read_fuse(171) >> 3) & 0x1u)

#define FUSE_USER_CR5_SAF_DISABLE() \
    ((soc_read_fuse(172) >> 14) & 0x1u)

#define FUSE_PARALLEL_BOOT_DIS() \
    ((soc_read_fuse(172) >> 15) & 0x1u)

#define FUSE_WDT1_DEFUALT_EN() \
    ((soc_read_fuse(172) >> 21) & 0x1u)

#define FUSE_USB_PID() \
    ((soc_read_fuse(16) >> 0) & 0xffffu)

#define FUSE_USB_VID() \
    ((soc_read_fuse(16) >> 16) & 0xffffu)

#define FUSE_PROD_MAJOR_ID() \
    ((soc_read_fuse(10) >> 8) & 0xffu)

#define FUSE_PROD_MINOR_ID() \
    ((soc_read_fuse(10) >> 0) & 0xffu)

#define FUSE_PROD_FAMILY_ID() \
    ((soc_read_fuse(10) >> 12) & 0xfu)

#define FUSE_PROD_SERIES_CODE() \
    ((soc_read_fuse(10) >> 8) & 0xfu)

#define FUSE_PROD_FEATURE_CODE() \
    ((soc_read_fuse(10) >> 4) & 0xfu)

#define FUSE_PROD_SPEED_GRADE() \
    ((soc_read_fuse(10) >> 0) & 0xfu)

#define FUSE_UUID_START     8

#define FUSE_DID_START      (0x48/4)

#define FUSE_RMV_START      (0x2D4/4)

#define FUSE_FUSE_SENSE_n_CHK_1() \
    ((soc_read_fuse(163) >> 31) & 0x1u)

#define FUSE_SEC_EARLY_FIREWALL_1() \
    ((soc_read_fuse(163) >> 30) & 0x1u)

#define FUSE_DCACHE_DISABLE() \
    ((soc_read_fuse(163) >> 23) & 0x1u)

#define FUSE_ICACHE_DISABLE() \
    ((soc_read_fuse(163) >> 22) & 0x1u)

#define FUSE_SAFETY_LOG_DUMP_DIS() \
    ((soc_read_fuse(163) >> 21) & 0x1u)

#define FUSE_SAFETY_ROMC_SELFTEST() \
    ((soc_read_fuse(163) >> 20) & 0x1u)

#define FUSE_SEC_ROMC_SELFTEST() \
    ((soc_read_fuse(163) >> 19) & 0x1u)

#define FUSE_HANDOVER_FLAG_TO_SEC() \
    ((soc_read_fuse(163) >> 18) & 0x1u)

#define FUSE_USB_OPEN_TIMEOUT() \
    ((soc_read_fuse(163) >> 16) & 0x3u)

#define FUSE_OSPI_PHY_TRAINING_FIX() \
    ((soc_read_fuse(163) >> 15) & 0x1u)

#define FUSE_PEER_BT_TMO_CHK() \
    ((soc_read_fuse(163) >> 14) & 0x1u)

#define FUSE_USB_MS_OS_DESC() \
    ((soc_read_fuse(163) >> 13) & 0x1u)

#define FUSE_SAFE_FREQ_BOOT() \
    ((soc_read_fuse(163) >> 12) & 0x1u)

#define FUSE_SEC_EARLY_FIREWALL_2() \
    ((soc_read_fuse(163) >> 1) & 0x1u)

#define FUSE_FUSE_SENSE_n_CHK_2() \
    ((soc_read_fuse(163) >> 0) & 0x1u)

#define FUSE_TEST_SKIP_SDMMC_CARD_INIT() \
    ((soc_read_fuse(99) >> 31) & 0x1)

#define FUSE_DONOT_SKIP_BOOT_FLOW_EVEN_UT() \
    ((soc_read_fuse(99) >> 30) & 0x1)

#define FUSE_TEST_BPT_CHK_IGNORE_CRC() \
    ((soc_read_fuse(99) >> 29) & 0x1)

#define FUSE_TEST_IGNORE_DABORT() \
    ((soc_read_fuse(99) >> 28) & 0x1)

#define FUSE_TEST_SEC_BT_MEM_DEV() \
    ((soc_read_fuse(99) >> 27) & 0x1)

#define FUSE_TEST_SAFE_BT_MEM_DEV() \
    ((soc_read_fuse(99) >> 26) & 0x1)

#define FUSE_TEST_SEC_BT_PIN_OVERRIDE_VAL() \
    ((soc_read_fuse(99) >> 16) & 0x1fu)

#define FUSE_TEST_OSPI_PHY_DLY_START() \
    ((soc_read_fuse(99) >> 8) & 0xffu)

#define FUSE_TEST_OSPI_PHY_DLY_END() \
    ((soc_read_fuse(99) >> 0) & 0xffu)

#define FUSE_CR5_SAF_LOCKSTEP_DISABLE() \
    ((soc_read_fuse(22) >> 1) & 0x1)

#define FUSE_CR5_SEC_LOCKSTEP_DISABLE() \
    ((soc_read_fuse(22) >> 2) & 0x1)

#define FUSE_CR5_MP_LOCKSTEP_DISABLE() \
    ((soc_read_fuse(22) >> 3) & 0x1)

#define FUSE_SEC_JUMP_ENTRY_OVERRIDE_EN() \
    ((soc_read_fuse(15) >> 31) & 0x1)

#define FUSE_SEC_JUMP_ENTRY_OVERRIDE_VAL() \
    ((soc_read_fuse(15) >> 16) & 0x7fff)

#define FUSE_SAFE_JUMP_ENTRY_OVERRIDE_EN() \
    ((soc_read_fuse(15) >> 15) & 0x1)

#define FUSE_SAFE_JUMP_ENTRY_OVERRIDE_VAL() \
    ((soc_read_fuse(15) >> 0) & 0x7fff)

#endif    /* __FUSE_HDR_GEN_H__ */
