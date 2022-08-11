/**
 *@file refa03_path.c
 *@author yi shao (yi.shao@semidrive.com)
 *@brief
 *@version 0.1
 *@date 2021-05-12
 *
 *@copyright Copyright (c) 2021 Semidrive Semiconductor
 *
 */
#include "refa03_path.h"

#include "ak7738.h"
#include "am.h"
#include "am_tca9539.h"
#include "chip_res.h"
#include "i2c_hal.h"
#include "tas6424.h"
#include "tca9539.h"
#include "xf6020.h"
#include "am_tas5404.h"
// extern const am_board_t g_am_board;
/* GPIO pin definition */
#define AUDIO_ANA_AMP_FAULT_N TCA9539_00_01_P00
#define AUDIO_ANA_AMP_CLIP_OTW_N TCA9539_01_01_P01
#define AUDIO_DIG_AMP_STANDBY_N TCA9539_02_01_P02
#define AUDIO_DIG_AMP_MUTE_N TCA9539_03_01_P03
#define AUDIO_DIG_AMP_FAULT_N TCA9539_04_01_P04
#define AUDIO_DIG_AMP_WARN_N TCA9539_05_01_P05
#define AUDIO_DSP_RST_N TCA9539_07_01_P07
#define AUDIO_4MIC_RST_N TCA9539_0F_01_P15

#define REFA03_TAS6424_CH_VOLUME_MAX (207)
#define REFA03_TAS6424_CH_VOLUME_MIN (107)
/**
 * here handle
 */
static bool refa03_path_idle_start(unsigned int vol)
{
    /*  set all ctl and sync*/
    bool ret;
    int vol_index;
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_00_E0_REFSEL, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_00_1F_REFMODE, 0xf);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_01_80_CKRESETN, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_01_1F_FSMODE, 0x7);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_02_02_PMMB1, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_02_01_PMMB2, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_03_80_MSN1, 0x1);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_03_70_CKS1, 0x1);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_03_07_SDV1, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_04_FF_BDV1, 0x27);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_05_80_MSN2, 0x1);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_05_70_CKS2, 0x1);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_05_07_SDV2, 0x4);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_06_FF_BDV2, 0x9);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_07_80_MSN3, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_07_70_CKS3, 0x1);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_07_07_SDV3, 0x2);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_08_FF_BDV3, 0xef);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_09_80_MSN4, 0x1);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_09_70_CKS4, 0x1);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_09_07_SDV4, 0x4);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_0A_FF_BDV4, 0x9);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_0B_80_MSN5, 0x1);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_0B_70_CKS5, 0x1);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_0B_07_SDV5, 0x4);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_0C_FF_BDV5, 0x9);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_0D_80_MSN6, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_0D_70_CKS6, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_0D_07_SDV6, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_0E_FF_BDV6, 0x27);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_0F_80_MSN7, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_0F_70_CKS7, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_0F_07_SDV7, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_10_FF_BDV7, 0x27);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_11_70_CKSB, 0x1);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_11_0C_MDIV, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_12_FF_MDIVB, 0x4);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_13_08_CLKOE, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_13_07_CLKOSEL, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_15_70_SDBCK1, 0x2);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_15_07_SDBCK2, 0x2);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_16_70_SDBCK3, 0x3);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_16_07_SDBCK4, 0x2);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_17_70_SDBCK5, 0x2);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_17_07_SDDSP1, 0x2);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_18_70_SDDSP2, 0x2);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_18_07_SDDSP1O1, 0x2);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_19_70_SDDSP1O2, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_19_07_SDDSP1O3, 0x2);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_1A_70_SDDI1, 0x2);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_1A_07_SDDI2, 0x2);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_1B_70_SDDI3, 0x3);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_1B_07_SDDI4, 0x2);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_1C_70_SDDI5, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_1C_07_SDDI6, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_1D_70_SDSRCO1, 0x2);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_1D_07_SDSRCO2, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_1E_70_SDSRCO3, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_1E_07_SDSRCO4, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_1F_70_SDFSCO1, 0x3);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_1F_07_SDFSCO2, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_20_70_SDMIXA, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_20_07_SDMIXB, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_21_70_SDADC1, 0x2);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_21_07_SDCODEC, 0x2);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_23_3F_SELDO1, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_24_3F_SELDO2, 0xd);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_25_3F_SELDO3, 0x22);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_26_3F_SELDO4, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_27_3F_SELDO5, 0x11);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_28_3F_SELDO6, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_29_3F_SELDA1, 0xf);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_2A_3F_SELDA2, 0x10);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_2B_3F_D1SELDI1, 0x1e);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_2C_3F_D1SELDI2, 0x2);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_2D_3F_D1SELDI3, 0x3);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_2E_3F_D1SELDI4, 0x4);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_2F_3F_D1SELDI5, 0x5);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_30_3F_D1SELDI6, 0x1);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_31_3F_D2SELDI1, 0x7);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_32_3F_D2SELDI2, 0x8);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_33_3F_D2SELDI3, 0x9);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_34_3F_D2SELDI4, 0xa);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_35_3F_D2SELDI5, 0x19);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_36_3F_D2SELDI6, 0x1a);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_37_3F_SELSRCI1, 0x6);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_38_3F_SELSRCI2, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_39_3F_SELSRCI3, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_3A_3F_SELSRCI4, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_3B_3F_SELFSCI1, 0x14);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_3C_3F_SELFSCI2, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_3D_3F_SELMIXAI1, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_3E_3F_SELMIXAI2, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_3F_3F_SELMIXBI1, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_40_3F_SELMIXBI2, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_41_3F_SELDIT, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_44_80_BCKP1, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_44_70_DCF1, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_44_08_BCKP2, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_44_07_DCF2, 0x6);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_45_80_BCKP3, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_45_70_DCF3, 0x6);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_45_08_BCKP4, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_45_07_DCF4, 0x6);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_46_08_BCKP5, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_46_07_DCF5, 0x6);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_48_80_DIEDGEN1, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_48_30_DISL1, 0x3);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_48_08_DILSBE1, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_48_07_DIDL1, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_49_80_DIEDGEN2, 0x1);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_49_30_DISL2, 0x3);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_49_08_DILSBE2, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_49_07_DIDL2, 0x2);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_4A_80_DIEDGEN3, 0x1);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_4A_30_DISL3, 0x2);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_4A_08_DILSBE3, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_4A_07_DIDL3, 0x2);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_4B_80_DIEDGEN4, 0x1);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_4B_30_DISL4, 0x3);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_4B_08_DILSBE4, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_4B_07_DIDL4, 0x3);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_4C_80_DIEDGEN5, 0x1);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_4C_30_DISL5, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_4C_08_DILSBE5, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_4C_07_DIDL5, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_4D_80_DIEDGEN6, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_4D_30_DISL6, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_4D_08_DILSBE6, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_4D_07_DIDL6, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_4E_80_DOEDGEN1, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_4E_30_DOSL1, 0x3);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_4E_08_DOLSBE1, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_4E_07_DODL1, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_4F_80_DOEDGEN2, 0x1);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_4F_30_DOSL2, 0x3);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_4F_08_DOLSBE2, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_4F_07_DODL2, 0x2);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_50_80_DOEDGEN3, 0x1);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_50_30_DOSL3, 0x2);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_50_08_DOLSBE3, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_50_07_DODL3, 0x2);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_51_80_DOEDGEN4, 0x1);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_51_30_DOSL4, 0x3);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_51_08_DOLSBE4, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_51_07_DODL4, 0x3);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_52_80_DOEDGEN5, 0x1);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_52_30_DOSL5, 0x3);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_52_08_DOLSBE5, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_52_07_DODL5, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_53_80_DOEDGEN6, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_53_30_DOSL6, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_53_08_DOLSBE6, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_53_07_DODL6, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_54_20_SDOPH6, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_54_10_SDOPH5, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_54_08_SDOPH4, 0x1);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_54_04_SDOPH3, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_54_02_SDOPH2, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_54_01_SDOPH1, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_55_C0_TDMO2, 0x1);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_55_30_TDMO3, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_55_0C_TDMO4, 0x1);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_55_03_TDMO5, 0x1);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_57_80_DO2SEL, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_57_40_DO3SEL, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_57_30_DO4SEL, 0x2);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_57_08_DO5SEL, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_57_03_DO6SEL, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_58_20_SDOUT1E, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_58_10_SDOUT2E, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_58_08_SDOUT3E, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_58_04_SDOUT4E, 0x1);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_58_02_SDOUT5E, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_58_01_SDOUT6E, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_59_10_DI2SEL, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_59_08_DI3SEL, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_59_04_DI5SEL, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_59_02_LCK3SEL, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_59_01_BCK3SEL, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_5B_C0_SFTA2, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_5B_30_SFTA1, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_5B_0C_SWPA2, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_5B_03_SWPA1, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_5C_C0_SFTB2, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_5C_30_SFTB1, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_5C_0C_SWPB2, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_5C_03_SWPB1, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_5D_F0_MGNL, 0xf);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_5D_0F_MGNR, 0xf);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_5E_08_ADRCLE, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_5E_04_ADRCRE, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_5E_02_MICLZCE, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_5E_01_MICRZCE, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_5F_FF_VOLAD1L, 0x30);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_60_FF_VOLAD1R, 0x30);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_61_FF_VOLAD2L, 0x30);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_62_FF_VOLAD2R, 0x30);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_63_FF_VOLADM, 0x30);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_66_80_ADSD, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_66_40_ADSL, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_66_10_ADMSEL, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_66_08_AD1LSEL, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_66_04_AD1RSEL, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_66_03_AD2SEL, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_67_80_ATSPAD, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_67_40_AD1MUTE, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_67_20_AD2MUTE, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_67_10_ADMMUTE, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_67_04_AD1HPFN, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_67_02_AD2HPFN, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_67_01_ADMHPFN, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_68_FF_VOLDA1L, 0x19);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_69_FF_VOLDA1R, 0x19);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_6A_FF_VOLDA2L, 0x19);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_6B_FF_VOLDA2R, 0x19);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_6E_80_ATSPDA, 0x1);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_6E_40_DA1MUTE, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_6E_20_DA2MUTE, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_6E_04_DSMN, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_6E_02_DASD, 0x1);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_6E_01_DASL, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_6F_80_SRCMM1, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_6F_40_SRCMM2, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_6F_20_SRCMM3, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_6F_10_SRCFAUD, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_70_80_SMUTE1, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_70_40_SMUTE2, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_70_20_SMUTE3, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_70_10_SMUTE4, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_70_08_SAUTO1, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_70_04_SAUTO2, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_70_02_SAUTO3, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_70_01_SAUTO4, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_71_20_FMUTE1, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_71_10_FMUTE2, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_71_08_FAUTO1, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_71_04_FAUTO2, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_71_02_FSCLRS1, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_71_01_FSCLRS2, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_73_30_PRAMDIV, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_73_0C_CRAMDIV, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_73_03_DRAMDIV, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_74_40_D3RAMCLRN, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_74_20_D2RAMCLRN, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_74_10_D1RAMCLRN, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_74_07_DLRAMDIV, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_75_C0_D1DRMA, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_75_30_D1DRMBK, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_75_0C_D2DRMA, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_75_03_D2DRMBK, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_76_C0_D1SS, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_76_20_D1DLRMA, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_76_0F_D1DLRMBK, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_77_C0_D2SS, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_77_20_D2DLRMA, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_77_0F_D2DLRMBK, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_78_80_D1DLP0, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_78_70_D1WAVP, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_78_08_D2DLP0, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_78_07_D2WAVP, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_7A_80_D1JX0E, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_7A_40_D1JX1E, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_7A_20_D1JX2E, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_7A_10_D1JX3E, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_7A_08_D2JX0E, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_7A_04_D2JX1E, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_7A_02_D2JX2E, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_7A_01_D2JX3E, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_7B_80_CRCE, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_7B_04_D1WDTEN, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_7B_02_D2WDTEN, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_7B_01_D3WDTEN, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_7C_80_SRCMM4, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_7C_40_PLLLOCKE, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_7C_20_SRCLOCKE1, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_7C_10_SRCLOCKE2, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_7C_08_SRCLOCKE3, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_7C_04_SRCLOCKE4, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_7C_02_FSCLOCKE1, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_7C_01_FSCLOCKE2, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_7E_80_V, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_7E_40_DITDTH, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_7E_20_CS41, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_7E_10_CS40, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_7E_08_CS3, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_7E_04_CS2, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_7E_02_CS1, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_7E_01_CHN, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_7F_80_CS15, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_7F_40_CS14, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_7F_20_CS13, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_7F_10_CS12, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_7F_08_CS11, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_7F_04_CS10, 0x1);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_7F_02_CS9, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_7F_01_CS8, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_80_20_CS29, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_80_10_CS28, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_80_08_CS27, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_80_04_CS26, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_80_02_CS25, 0x1);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_80_01_CS24, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_81_80_CS39, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_81_40_CS38, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_81_20_CS37, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_81_10_CS36, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_81_08_CS35, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_81_04_CS34, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_81_02_CS33, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_81_01_CS32, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_83_20_PMAD1, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_83_10_PMAD2, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_83_08_PMADM, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_83_04_PMDA1, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_83_02_PMDA2, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_84_80_PMSRC1, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_84_40_PMSRC2, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_84_20_PMSRC3, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_84_10_PMSRC4, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_84_08_PMFSC1, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_84_04_PMFSC2, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_84_02_PMDIT, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_85_80_DLRDY, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_85_10_CRESETN, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_85_08_D1RESETN, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_85_04_D2RESETN, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_85_02_D3RESETN, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_85_01_HRESETN, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_91_C0_SDOUT4DS, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_91_30_SDOUT3DS, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_91_0C_SDOUT2DS, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_91_03_SDOUT1DS, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_91_C0_BICK4DS, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_91_30_BICK3DS, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_91_0C_BICK2DS, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_91_03_BICK1DS, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_92_0C_LRCK4DS, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_92_03_LRCK3DS, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_93_C0_LRCK2DS, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_93_30_LRCK1DS, 0x0);
    _CHECK_RET(ret);

    /**TAS6424 setting */
    ret = write_ctl(REFA03_CHIP_TAS6424, TAS6424_00_80_RESET, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_TAS6424, TAS6424_00_20_PBTL_CH34, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_TAS6424, TAS6424_00_10_PBTL_CH12, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_TAS6424, TAS6424_00_08_CH1_LO_MODE, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_TAS6424, TAS6424_00_04_CH2_LO_MODE, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_TAS6424, TAS6424_00_02_CH3_LO_MODE, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_TAS6424, TAS6424_00_01_CH4_LO_MODE, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_TAS6424, TAS6424_01_80_HPF_BYPASS, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_TAS6424, TAS6424_01_60_OTW_CONTROL, 0x1);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_TAS6424, TAS6424_01_10_OC_CONTROL, 0x1);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_TAS6424, TAS6424_01_0C_VOLUME_RATE, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_TAS6424, TAS6424_01_01_GAIN, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_TAS6424, TAS6424_02_70_PWM_FREQUENCY, 0x6);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_TAS6424, TAS6424_02_04_SDM_OSR, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_TAS6424, TAS6424_02_03_OUTPUT_PHASE, 0x2);
    _CHECK_RET(ret);
    ret =
        write_ctl(REFA03_CHIP_TAS6424, TAS6424_03_C0_INPUT_SAMPLING_RATE, 0x0);
    _CHECK_RET(ret);
    ret =
        write_ctl(REFA03_CHIP_TAS6424, TAS6424_03_20_8_CH_TDM_SLOT_SELECT, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_TAS6424, TAS6424_03_10_TDM_SLOT_SIZE, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_TAS6424, TAS6424_03_08_TDM_SLOT_SELECT_2, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_TAS6424, TAS6424_03_07_INPUT_FORMAT, 0x4);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_TAS6424, TAS6424_04_C0_CH1_STATE_CONTROL, 0x1);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_TAS6424, TAS6424_04_30_CH2_STATE_CONTROL, 0x1);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_TAS6424, TAS6424_04_0C_CH3_STATE_CONTROL, 0x1);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_TAS6424, TAS6424_04_03_CH4_STATE_CONTROL, 0x1);
    _CHECK_RET(ret);
    /** Here caluate volume index for tas6424 */
    vol_index =
        get_linear_vol(vol, REFA03_TAS6424_CH_VOLUME_MAX, REFA03_TAS6424_CH_VOLUME_MIN);
    printf("vol_index is %d \n", vol_index); // debug line

    ret = write_ctl(REFA03_CHIP_TAS6424, TAS6424_05_FF_CH1_VOLUME, vol_index);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_TAS6424, TAS6424_06_FF_CH2_VOLUME, vol_index);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_TAS6424, TAS6424_07_FF_CH3_VOLUME, vol_index);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_TAS6424, TAS6424_08_FF_CH4_VOLUME, vol_index);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_TAS6424, TAS6424_09_80_DC_LDG_ABORT, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_TAS6424, TAS6424_09_40_2X_RAMP, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_TAS6424, TAS6424_09_20_2X_SETTLE, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_TAS6424, TAS6424_09_02_LDG_LO_ENABLE, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_TAS6424, TAS6424_09_01_LDG_BYPASS, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_TAS6424, TAS6424_0A_F0_CH1_DC_LDG_SL, 0x1);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_TAS6424, TAS6424_0A_0F_CH2_DC_LDG_SL, 0x1);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_TAS6424, TAS6424_0B_F0_CH3_DC_LDG_SL, 0x1);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_TAS6424, TAS6424_0B_0F_CH4_DC_LDG_SL, 0x1);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_TAS6424, TAS6424_14_80_MASK_OC, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_TAS6424, TAS6424_14_40_MASK_OTSD, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_TAS6424, TAS6424_14_20_MASK_UV, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_TAS6424, TAS6424_14_10_MASK_OV, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_TAS6424, TAS6424_14_08_MASK_DC, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_TAS6424, TAS6424_14_04_MASK_ILIMIT, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_TAS6424, TAS6424_14_02_MASK_CLIP, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_TAS6424, TAS6424_14_01_MASK_OTW, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_TAS6424, TAS6424_15_80_CH12_PBTL12_GAIN, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_TAS6424, TAS6424_15_20_CH34_PBTL34_GAIN, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_TAS6424, TAS6424_15_08_CH1_ENABLE, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_TAS6424, TAS6424_15_04_CH2_ENABLE, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_TAS6424, TAS6424_15_02_CH3_ENABLE, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_TAS6424, TAS6424_15_01_CH4_ENABLE, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_TAS6424, TAS6424_16_80_AC_DIAGS_LOOPBACK, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_TAS6424, TAS6424_16_10_AC_TIMING, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_TAS6424, TAS6424_16_0C_AC_CURRENT, 0x0);
    _CHECK_RET(ret);
    /**tca9539 setting */
    ret = write_ctl(REFA03_CHIP_TCA9539, AUDIO_DIG_AMP_MUTE_N, 0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_TCA9539, AUDIO_DIG_AMP_STANDBY_N, 0);
    _CHECK_RET(ret);
    /**tas5404 setting */

    ret = write_ctl(REFA03_CHIP_TAS5404, TAS5404_08_03_GAIN_CH1, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_TAS5404, TAS5404_08_0C_GAIN_CH2, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_TAS5404, TAS5404_08_30_GAIN_CH3, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_TAS5404, TAS5404_08_C0_GAIN_CH4, 0x0);
    _CHECK_RET(ret);

    ret = write_ctl(REFA03_CHIP_TAS5404, TAS5404_09_01_THERMAL_FOLDBACK, 0x00);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_TAS5404, TAS5404_09_10_OC_LEVEL_CH1, 0x01);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_TAS5404, TAS5404_09_20_OC_LEVEL_CH2, 0x01);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_TAS5404, TAS5404_09_40_OC_LEVEL_CH3, 0x01);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_TAS5404, TAS5404_09_80_OC_LEVEL_CH4, 0x01);
    _CHECK_RET(ret);

    ret = write_ctl(REFA03_CHIP_TAS5404, TAS5404_0A_03_FREQ_SEL_, 0x01);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_TAS5404, TAS5404_0A_0C_CONF_CLIP_OTW_REPORT,
                    0x03);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_TAS5404, TAS5404_0A_10_HARD_STOP_MODE, 0x00);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_TAS5404, TAS5404_0A_20_PHASE_DIFF, 0x00);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_TAS5404, TAS5404_0A_40_SYNC_PULSE, 0x00);
    _CHECK_RET(ret);
    ret =
        write_ctl(REFA03_CHIP_TAS5404, TAS5404_0A_80_CONF_THERMAL_REPORT, 0x00);
    _CHECK_RET(ret);

    ret = write_ctl(REFA03_CHIP_TAS5404, TAS5404_0B_01_LOADDIAG_CH1, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_TAS5404, TAS5404_0B_02_LOADDIAG_CH2, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_TAS5404, TAS5404_0B_04_LOADDIAG_CH3, 0x00);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_TAS5404, TAS5404_0B_08_LOADDIAG_CH4, 0x00);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_TAS5404, TAS5404_0B_10_DC_DETECTION, 0x01);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_TAS5404, TAS5404_0B_20_TWEETER_DETECT, 0x00);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_TAS5404, TAS5404_0B_40_SLAVE_MODE, 0x01);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_TAS5404, TAS5404_0B_80_CFG_OSC_SYNC, 0x00);
    _CHECK_RET(ret);

    ret = write_ctl(REFA03_CHIP_TAS5404, TAS5404_0C_01_UNMUTE_CH1, 0x1);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_TAS5404, TAS5404_0C_02_UNMUTE_CH2, 0x1);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_TAS5404, TAS5404_0C_04_UNMUTE_CH3, 0x01);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_TAS5404, TAS5404_0C_08_UNMUTE_CH4, 0x01);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_TAS5404, TAS5404_0C_10_UNMUTE, 0x01);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_TAS5404, TAS5404_0C_20_DC_SD, 0x00);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_TAS5404, TAS5404_0C_80_RESET, 0x0);
    _CHECK_RET(ret);
    // Set to low-low state to all channels
    ret = write_ctl(REFA03_CHIP_TAS5404, TAS5404_0D_01_LOW_LOW_CH1, 0x01);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_TAS5404, TAS5404_0D_02_LOW_LOW_CH2, 0x01);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_TAS5404, TAS5404_0D_04_LOW_LOW_CH3, 0x01);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_TAS5404, TAS5404_0D_08_LOW_LOW_CH4, 0x01);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_TAS5404, TAS5404_0D_10_CH12_PBTL, 0x00);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_TAS5404, TAS5404_0D_20_CH34_PBTL, 0x00);
    _CHECK_RET(ret);

    ret = write_ctl(REFA03_CHIP_TAS5404, TAS5404_10_03_DC_DETECT_VAL, 0x01);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_TAS5404, TAS5404_10_04_CROSSTALK, 0x0);
    _CHECK_RET(ret);
    ret =
        write_ctl(REFA03_CHIP_TAS5404, TAS5404_10_08_DELAY20MS_LOAD_DIAG, 0x00);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_TAS5404, TAS5404_10_10_S2P_S2G_4X, 0x00);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_TAS5404, TAS5404_10_20_CM_RAMP_SPEED, 0x00);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_TAS5404, TAS5404_10_80_SLOW_CM_MUTE, 0x00);
    _CHECK_RET(ret);

    ret = write_ctl(REFA03_CHIP_TAS5404, TAS5404_10_03_DC_DETECT_VAL, 0x00);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_TAS5404, TAS5404_10_04_CROSSTALK, 0x0);
    _CHECK_RET(ret);
    ret =
        write_ctl(REFA03_CHIP_TAS5404, TAS5404_10_08_DELAY20MS_LOAD_DIAG, 0x00);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_TAS5404, TAS5404_10_10_S2P_S2G_4X, 0x00);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_TAS5404, TAS5404_10_20_CM_RAMP_SPEED, 0x00);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_TAS5404, TAS5404_10_80_SLOW_CM_MUTE, 0x00);
    _CHECK_RET(ret);
    return true;
}

static bool refa03_path_hifi_playback_to_main_spk_48k_start(unsigned int vol)
{
    bool ret;
    /** ak7738 setting*/
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_01_80_CKRESETN, 0x1);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_58_02_SDOUT5E, 0x1);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_83_04_PMDA1, 0x1);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_83_02_PMDA2, 0x1);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_85_10_CRESETN, 0x1);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_85_08_D1RESETN, 0x1);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_85_01_HRESETN, 0x1);
    _CHECK_RET(ret);

    /**tas9539 setting*/
    ret =
        write_ctl(REFA03_CHIP_TAS6424, TAS6424_03_C0_INPUT_SAMPLING_RATE, 0x1);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_TAS6424, TAS6424_03_07_INPUT_FORMAT, 0x6);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_TAS6424, TAS6424_04_C0_CH1_STATE_CONTROL, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_TAS6424, TAS6424_04_30_CH2_STATE_CONTROL, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_TAS6424, TAS6424_04_0C_CH3_STATE_CONTROL, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_TAS6424, TAS6424_04_03_CH4_STATE_CONTROL, 0x0);
    _CHECK_RET(ret);

    /**tas6424 setting*/
    ret = write_ctl(REFA03_CHIP_TCA9539, AUDIO_DIG_AMP_MUTE_N, 1);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_TCA9539, AUDIO_DIG_AMP_STANDBY_N, 1);
    _CHECK_RET(ret);

    /**tas5404 setting*/
    ret = write_ctl(REFA03_CHIP_TAS5404, TAS5404_0C_10_UNMUTE, 0x00);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_TAS5404, TAS5404_0C_01_UNMUTE_CH1, 0x00);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_TAS5404, TAS5404_0C_02_UNMUTE_CH2, 0x00);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_TAS5404, TAS5404_0C_04_UNMUTE_CH3, 0x00);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_TAS5404, TAS5404_0C_08_UNMUTE_CH4, 0x00);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_TAS5404, TAS5404_0D_01_LOW_LOW_CH1, 0x00);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_TAS5404, TAS5404_0D_02_LOW_LOW_CH2, 0x00);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_TAS5404, TAS5404_0D_04_LOW_LOW_CH3, 0x00);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_TAS5404, TAS5404_0D_08_LOW_LOW_CH4, 0x00);
    _CHECK_RET(ret);
    return true;
}
static bool refa03_path_hifi_capture_from_main_mic_48k_start(unsigned int vol)
{
    bool ret;
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_01_80_CKRESETN, 0x1);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_02_02_PMMB1, 0x1);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_02_01_PMMB2, 0x1);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_58_10_SDOUT2E, 0x1);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_58_04_SDOUT4E, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_83_20_PMAD1, 0x1);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_83_10_PMAD2, 0x1);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_85_10_CRESETN, 0x1);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_85_08_D1RESETN, 0x1);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_85_04_D2RESETN, 0x1);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_85_01_HRESETN, 0x1);
    _CHECK_RET(ret);
    /**xf6020 setting*/
    ret = write_ctl(REFA03_CHIP_XF6020, XF6020_00_VOICE_MODE, TWO_SND_AREAS);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_XF6020, XF6020_01_FF_PARAM, DUAL_MAE);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_XF6020, XF6020_02_FF_BYPASS, 0x0);
    _CHECK_RET(ret);
    return true;
}
static bool refa03_path_safety_playback_to_main_spk_48k_start(unsigned int vol)
{
    bool ret;

    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_01_80_CKRESETN, 0x1); //Clock release
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_1A_70_SDDI1, 0x2);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_58_02_SDOUT5E, 0x1);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_85_10_CRESETN, 0x1);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_85_08_D1RESETN, 0x1);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_85_01_HRESETN, 0x1);
    _CHECK_RET(ret);
    ret =
        write_ctl(REFA03_CHIP_TAS6424, TAS6424_03_C0_INPUT_SAMPLING_RATE, 0x1);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_TAS6424, TAS6424_03_07_INPUT_FORMAT, 0x6);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_TAS6424, TAS6424_04_C0_CH1_STATE_CONTROL, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_TAS6424, TAS6424_04_30_CH2_STATE_CONTROL, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_TAS6424, TAS6424_04_0C_CH3_STATE_CONTROL, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_TAS6424, TAS6424_04_03_CH4_STATE_CONTROL, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_TCA9539, AUDIO_DIG_AMP_MUTE_N, 1);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_TCA9539, AUDIO_DIG_AMP_STANDBY_N, 1);
    _CHECK_RET(ret);
    return true;
}

static bool refa03_path_bt_playback_to_main_spk_16k_start(unsigned int vol)
{
    bool ret;
    int vol_index;
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_01_80_CKRESETN, 0x1);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_03_80_MSN1, 0x1);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_03_70_CKS1, 0x1);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_08_FF_BDV3, 0xef);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_15_70_SDBCK1, 0x2);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_1A_70_SDDI1, 0x2);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_30_3F_D1SELDI6, 0x1);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_48_30_DISL1, 0x3);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_49_07_DIDL2, 0x2);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_4A_07_DIDL3, 0x2);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_4B_07_DIDL4, 0x3);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_4E_30_DOSL1, 0x3);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_4F_07_DODL2, 0x2);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_50_07_DODL3, 0x2);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_51_07_DODL4, 0x3);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_58_02_SDOUT5E, 0x1);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_84_80_PMSRC1, 0x1);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_85_08_D1RESETN, 0x1);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_85_01_HRESETN, 0x1);
    _CHECK_RET(ret);

    ret =
        write_ctl(REFA03_CHIP_TAS6424, TAS6424_03_C0_INPUT_SAMPLING_RATE, 0x1);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_TAS6424, TAS6424_03_07_INPUT_FORMAT, 0x6);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_TAS6424, TAS6424_04_C0_CH1_STATE_CONTROL, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_TAS6424, TAS6424_04_30_CH2_STATE_CONTROL, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_TAS6424, TAS6424_04_0C_CH3_STATE_CONTROL, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_TAS6424, TAS6424_04_03_CH4_STATE_CONTROL, 0x0);
    _CHECK_RET(ret);
    /** Here caluate volume index for tas6424 */
    vol_index =
        get_linear_vol(vol, REFA03_TAS6424_CH_VOLUME_MAX, REFA03_TAS6424_CH_VOLUME_MIN);
    printf("vol_index is %d \n", vol_index);

    ret = write_ctl(REFA03_CHIP_TAS6424, TAS6424_05_FF_CH1_VOLUME, vol_index);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_TAS6424, TAS6424_06_FF_CH2_VOLUME, vol_index);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_TAS6424, TAS6424_07_FF_CH3_VOLUME, vol_index);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_TAS6424, TAS6424_08_FF_CH4_VOLUME, vol_index);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_TCA9539, AUDIO_DIG_AMP_MUTE_N, 1);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_TCA9539, AUDIO_DIG_AMP_STANDBY_N, 1);
    _CHECK_RET(ret);
    return true;
}

static bool refa03_path_bt_playback_to_main_spk_8k_start(unsigned int vol)
{
    bool ret;
    int vol_index;
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_01_80_CKRESETN, 0x1);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_03_80_MSN1, 0x1);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_03_70_CKS1, 0x1);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_08_FF_BDV3, 0xff);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_15_70_SDBCK1, 0x2);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_1A_70_SDDI1, 0x2);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_30_3F_D1SELDI6, 0x1);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_48_30_DISL1, 0x3);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_49_07_DIDL2, 0x2);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_4A_07_DIDL3, 0x2);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_4B_07_DIDL4, 0x3);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_4E_30_DOSL1, 0x3);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_4F_07_DODL2, 0x2);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_50_07_DODL3, 0x2);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_51_07_DODL4, 0x3);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_58_02_SDOUT5E, 0x1);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_84_80_PMSRC1, 0x1);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_85_08_D1RESETN, 0x1);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_85_01_HRESETN, 0x1);
    _CHECK_RET(ret);
    ret =
        write_ctl(REFA03_CHIP_TAS6424, TAS6424_03_C0_INPUT_SAMPLING_RATE, 0x1);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_TAS6424, TAS6424_03_07_INPUT_FORMAT, 0x6);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_TAS6424, TAS6424_04_C0_CH1_STATE_CONTROL, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_TAS6424, TAS6424_04_30_CH2_STATE_CONTROL, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_TAS6424, TAS6424_04_0C_CH3_STATE_CONTROL, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_TAS6424, TAS6424_04_03_CH4_STATE_CONTROL, 0x0);
    _CHECK_RET(ret);
    /** Here caluate volume index for tas6424 */
    vol_index =
        get_linear_vol(vol, REFA03_TAS6424_CH_VOLUME_MAX, REFA03_TAS6424_CH_VOLUME_MIN);

    ret = write_ctl(REFA03_CHIP_TAS6424, TAS6424_05_FF_CH1_VOLUME, vol_index);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_TAS6424, TAS6424_06_FF_CH2_VOLUME, vol_index);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_TAS6424, TAS6424_07_FF_CH3_VOLUME, vol_index);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_TAS6424, TAS6424_08_FF_CH4_VOLUME, vol_index);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_TCA9539, AUDIO_DIG_AMP_MUTE_N, 1);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_TCA9539, AUDIO_DIG_AMP_STANDBY_N, 1);
    _CHECK_RET(ret);
    return true;
}

static bool refa03_path_bt_capture_from_main_mic_16k_start(unsigned int vol)
{
    bool ret;
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_01_80_CKRESETN, 0x1);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_02_02_PMMB1, 0x1);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_02_01_PMMB2, 0x1);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_58_08_SDOUT3E, 0x1);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_58_04_SDOUT4E, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_83_20_PMAD1, 0x1);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_83_10_PMAD2, 0x1);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_84_08_PMFSC1, 0x1);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_85_10_CRESETN, 0x1);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_85_04_D2RESETN, 0x1);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_85_01_HRESETN, 0x1);
    _CHECK_RET(ret);
    /**xf6020 setting*/
    ret = write_ctl(REFA03_CHIP_XF6020, XF6020_00_VOICE_MODE, TWO_SND_AREAS);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_XF6020, XF6020_01_FF_PARAM, AEC_TEL);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_XF6020, XF6020_02_FF_BYPASS, 0x0);
    _CHECK_RET(ret);

    return true;
}

static bool refa03_path_bt_capture_from_main_mic_8k_start(unsigned int vol)
{
    bool ret;
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_01_80_CKRESETN, 0x1);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_02_02_PMMB1, 0x1);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_02_01_PMMB2, 0x1);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_08_FF_BDV3, 0xff);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_58_08_SDOUT3E, 0x1);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_58_04_SDOUT4E, 0x0);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_83_20_PMAD1, 0x1);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_83_10_PMAD2, 0x1);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_84_08_PMFSC1, 0x1);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_85_10_CRESETN, 0x1);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_85_04_D2RESETN, 0x1);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_AK7738, AK7738_85_01_HRESETN, 0x1);
    _CHECK_RET(ret);
    /**xf6020 setting*/
    ret = write_ctl(REFA03_CHIP_XF6020, XF6020_00_VOICE_MODE, TWO_SND_AREAS);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_XF6020, XF6020_01_FF_PARAM, AEC_TEL);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_XF6020, XF6020_02_FF_BYPASS, 0x0);
    _CHECK_RET(ret);
    return true;
}
static bool refa03_path_cluster_playback_to_main_spk_48k_start(unsigned int vol)
{
    return true;
}

static bool refa03_path_start(unsigned int path_id, unsigned int vol)
{
    switch (path_id)
    {
    case IDLE_PATH:
        return refa03_path_idle_start(vol);
    case HIFI_PLAYBACK_TO_MAIN_SPK_48K:
        return refa03_path_hifi_playback_to_main_spk_48k_start(vol);
    case HIFI_CAPTURE_FROM_MAIN_MIC_48K:
        return refa03_path_hifi_capture_from_main_mic_48k_start(vol);
    case SAFETY_PLAYBACK_TO_MAIN_SPK_48K:
        return refa03_path_safety_playback_to_main_spk_48k_start(vol);
    case BT_PLAYBACK_TO_MAIN_SPK_16K:
        return refa03_path_bt_playback_to_main_spk_16k_start(vol);
    case BT_PLAYBACK_TO_MAIN_SPK_8K:
        return refa03_path_bt_playback_to_main_spk_8k_start(vol);
    case BT_CAPTURE_FROM_MAIN_MIC_16K:
        return refa03_path_bt_capture_from_main_mic_16k_start(vol);
    case BT_CAPTURE_FROM_MAIN_MIC_8K:
        return refa03_path_bt_capture_from_main_mic_8k_start(vol);
    case CLUSTER_PLAYBACK_TO_MAIN_SPK_48K:
        return refa03_path_cluster_playback_to_main_spk_48k_start(vol);
    }
    return false;
}
/** not implement , just return to idle state.  */
static bool refa03_path_stop(unsigned int path_id) { return true; }

static bool refa03_path_bt_capture_from_main_mic_8k_mute(unsigned int mute)
{
    bool ret;
    if (UNMUTE_PATH == mute)
    {
        ret = write_ctl(REFA03_CHIP_AK7738, AK7738_67_40_AD1MUTE, 0x0);
        _CHECK_RET(ret);
        ret = write_ctl(REFA03_CHIP_AK7738, AK7738_67_20_AD2MUTE, 0x0);
        _CHECK_RET(ret);
    }
    else
    {
        ret = write_ctl(REFA03_CHIP_AK7738, AK7738_67_40_AD1MUTE, 0x1);
        _CHECK_RET(ret);
        ret = write_ctl(REFA03_CHIP_AK7738, AK7738_67_20_AD2MUTE, 0x1);
        _CHECK_RET(ret);
    }
    return true;
}

static bool refa03_path_bt_capture_from_main_mic_16k_mute(unsigned int mute)
{
    bool ret;
    if (UNMUTE_PATH == mute)
    {
        ret = write_ctl(REFA03_CHIP_AK7738, AK7738_67_40_AD1MUTE, 0x0);
        _CHECK_RET(ret);
        ret = write_ctl(REFA03_CHIP_AK7738, AK7738_67_20_AD2MUTE, 0x0);
        _CHECK_RET(ret);
    }
    else
    {
        ret = write_ctl(REFA03_CHIP_AK7738, AK7738_67_40_AD1MUTE, 0x1);
        _CHECK_RET(ret);
        ret = write_ctl(REFA03_CHIP_AK7738, AK7738_67_20_AD2MUTE, 0x1);
        _CHECK_RET(ret);
    }
    return true;
}

static bool refa03_path_hifi_capture_from_main_mic_48k_mute(unsigned int mute)
{
    bool ret;
    if (UNMUTE_PATH == mute)
    {
        _FUNC_LINE_PRT_
        ret = write_ctl(REFA03_CHIP_AK7738, AK7738_67_40_AD1MUTE, 0x0);
        _CHECK_RET(ret);
        ret = write_ctl(REFA03_CHIP_AK7738, AK7738_67_20_AD2MUTE, 0x0);
        _CHECK_RET(ret);
    }
    else
    {
        _FUNC_LINE_PRT_
        ret = write_ctl(REFA03_CHIP_AK7738, AK7738_67_40_AD1MUTE, 0x1);
        _CHECK_RET(ret);
        ret = write_ctl(REFA03_CHIP_AK7738, AK7738_67_20_AD2MUTE, 0x1);
        _CHECK_RET(ret);
    }
    return true;
}

static bool refa03_path_mute(unsigned int path_id, unsigned int mute)
{
    switch (path_id)
    {
    case HIFI_CAPTURE_FROM_MAIN_MIC_48K:
        return refa03_path_hifi_capture_from_main_mic_48k_mute(mute);
    case BT_CAPTURE_FROM_MAIN_MIC_16K:
        return refa03_path_bt_capture_from_main_mic_16k_mute(mute);
    case BT_CAPTURE_FROM_MAIN_MIC_8K:
        return refa03_path_bt_capture_from_main_mic_8k_mute(mute);
    }
    return false;
}

static bool refa03_path_bt_playback_to_main_spk_16k_setvol(unsigned int vol)
{
    bool ret;
    int vol_index;
    vol_index =
        get_linear_vol(vol, REFA03_TAS6424_CH_VOLUME_MAX, REFA03_TAS6424_CH_VOLUME_MIN);
    printf("vol_index is %d \n", vol_index);
    ret = write_ctl(REFA03_CHIP_TAS6424, TAS6424_05_FF_CH1_VOLUME, vol_index);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_TAS6424, TAS6424_06_FF_CH2_VOLUME, vol_index);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_TAS6424, TAS6424_07_FF_CH3_VOLUME, vol_index);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_TAS6424, TAS6424_08_FF_CH4_VOLUME, vol_index);
    _CHECK_RET(ret);
    return true;
}

static bool refa03_path_bt_playback_to_main_spk_8k_setvol(unsigned int vol)
{
    bool ret;
    int vol_index;
    vol_index =
        get_linear_vol(vol, REFA03_TAS6424_CH_VOLUME_MAX, REFA03_TAS6424_CH_VOLUME_MIN);
    printf("vol_index is %d \n", vol_index);
    ret = write_ctl(REFA03_CHIP_TAS6424, TAS6424_05_FF_CH1_VOLUME, vol_index);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_TAS6424, TAS6424_06_FF_CH2_VOLUME, vol_index);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_TAS6424, TAS6424_07_FF_CH3_VOLUME, vol_index);
    _CHECK_RET(ret);
    ret = write_ctl(REFA03_CHIP_TAS6424, TAS6424_08_FF_CH4_VOLUME, vol_index);
    _CHECK_RET(ret);
    return true;
}

static bool refa03_path_setvol(unsigned int path_id, unsigned int vol)
{
    switch (path_id)
    {
    case BT_PLAYBACK_TO_MAIN_SPK_16K:
        return refa03_path_bt_playback_to_main_spk_16k_setvol(vol);
    case BT_PLAYBACK_TO_MAIN_SPK_8K:
        return refa03_path_bt_playback_to_main_spk_8k_setvol(vol);
    }
    return false;
}

static bool refa03_path_switch(unsigned int from_path_id,
                               unsigned int to_path_id)
{
    return true;
}
/* GPIO will defined by board,so need overwrite readable and writeable function here*/
static bool refa03_tca9539_readable_reg(am_codec_dev_t *dev, unsigned int reg)
{
    if (dev->codec_type != CODEC_TYPE_TCA9539) {
        _ERR_FUNC_LINE_
        return false;
    }
    /* readable return true */
    switch (reg) {
    case TCA9539_00_PIN_REG0:
    case TCA9539_01_PIN_REG1:
    case TCA9539_04_PIN_REG4:
    case TCA9539_05_PIN_REG5:
        return true;
    default:
        return false;
    }
    return true;
}
/* GPIO will defined by board,so need overwrite readable and writeable function here*/
static bool refa03_tca9539_writeable_reg(am_codec_dev_t *dev, unsigned int reg)
{
    if (dev->codec_type != CODEC_TYPE_TCA9539) {
        _ERR_FUNC_LINE_
        return false;
    }
    /* readable return true */
    switch (reg) {
    case TCA9539_02_PIN_REG2:
    case TCA9539_03_PIN_REG3:
    case TCA9539_07_PIN_REG7:
    case TCA9539_0F_PIN_REG15:
        return true;
    default:
        return false;
    }
    return true;
}

static bool refa03_path_chip_initialize(int chip_id)
{
    bool ret = false;
    _FUNC_LINE_PRT_
    void * handle;
    if (REFA03_CHIP_AK7738 == chip_id)
    {
        /** create ak7738 i2c handle */
        ret = hal_i2c_creat_handle(&handle, RES_I2C_I2C6);
        if (ret == false)
        {
            _ERR_FUNC_LINE_PRT_
            return false;
        }
        set_chip_handle(REFA03_CHIP_AK7738,handle,0x1c);
        set_chip_info(REFA03_CHIP_AK7738, PROTOCOL_TYPE_I2C, CODEC_TYPE_AK7738);
        return true;
    }
    else if (REFA03_CHIP_TCA9539 == chip_id)
    {
        /** create tca9539  gpio*/
        handle = tca9539_init(12, 0x74);
        if (handle == NULL)
        {
            _ERR_FUNC_LINE_PRT_
            return false;
        }
        tca9539_enable_i2cpoll(handle);
        set_chip_handle(REFA03_CHIP_TCA9539,handle,0x74);
        /* It is i2c io expander so protocol is i2c */
        set_chip_info(REFA03_CHIP_TCA9539, PROTOCOL_TYPE_I2C, CODEC_TYPE_TCA9539);
        _FUNC_LINE_PRT_
        /*Only gpio need set writeable and readable function in codec interface*/
        set_chip_writeable_func(REFA03_CHIP_TCA9539,  refa03_tca9539_writeable_reg);
        set_chip_readable_func(REFA03_CHIP_TCA9539,  refa03_tca9539_readable_reg);
        return true;
    }
    else if (REFA03_CHIP_TAS6424 == chip_id)
    {
        /** create tas6424 i2c handle */
        ret = hal_i2c_creat_handle(&handle, RES_I2C_I2C6);
        if (ret == false)
        {
            _ERR_FUNC_LINE_PRT_
            return false;
        }

        set_chip_handle(REFA03_CHIP_TAS6424, handle, 0x6a);
        set_chip_info(REFA03_CHIP_TAS6424, PROTOCOL_TYPE_I2C, CODEC_TYPE_TAS6424);
        return true;
    }
    else if (REFA03_CHIP_XF6020 == chip_id)
    {
        /** create xf6020 i2c handle */
        ret = hal_i2c_creat_handle(&handle, RES_I2C_I2C6);
        if (ret == false)
        {
            _ERR_FUNC_LINE_PRT_
            return false;
        }
        set_chip_handle(REFA03_CHIP_XF6020, handle, 0x47);
        set_chip_info(REFA03_CHIP_XF6020, PROTOCOL_TYPE_I2C, CODEC_TYPE_XF6020);
        return true;
    }
    else if (REFA03_CHIP_TAS5404 == chip_id)
    {
        /** create tas5404 i2c handle */
        ret = hal_i2c_creat_handle(&handle, RES_I2C_I2C6);
        if (ret == false)
        {
            _ERR_FUNC_LINE_PRT_
            return false;
        }
        set_chip_handle(REFA03_CHIP_TAS5404, handle, 0x6c);
        set_chip_info(REFA03_CHIP_TAS5404, PROTOCOL_TYPE_I2C, CODEC_TYPE_TAS5404);
        return true;
    }
    _ERR_FUNC_LINE_PRT_
    return false;
}

static bool refa03_path_chip_release(int chip_id)
{
    _FUNC_LINE_PRT_
    am_codec_dev_t * dev = get_chip_dev(chip_id);
    if ((dev->codec_type == CODEC_TYPE_AK7738)||
        (dev->codec_type == CODEC_TYPE_TAS6424)||
        (dev->codec_type == CODEC_TYPE_XF6020)||
        (dev->codec_type == CODEC_TYPE_TAS5404)) {
        hal_i2c_release_handle(dev->dev_handle);
    }
    if (dev->codec_type == CODEC_TYPE_TCA9539) {
        tca9539_deinit(dev->dev_handle);
    }
    return true;
}

static bool refa03_path_op(unsigned int path_id, unsigned int op_code,
                           unsigned int param)
{
    switch (op_code)
    {
    case OP_START:
        return refa03_path_start(path_id, param);
    case OP_STOP:
        return refa03_path_stop(path_id);
    case OP_MUTE:
        return refa03_path_mute(path_id, param);
    case OP_SETVOL:
        return refa03_path_setvol(path_id, param);
    case OP_SWITCH:
        return refa03_path_switch(path_id, param);
    }
    return false;
}

static bool refa03_path_reset(void)
{

    /* here reset ref a04 board*/
    bool ret;
    _FUNC_LINE_PRT_
    /* Ak7738 reset by GPIO*/
    ret = write_ctl_nocache(REFA03_CHIP_TCA9539, AUDIO_DSP_RST_N, 0);
    _CHECK_RET(ret)
    delay_ctl(3);
    ret = write_ctl_nocache(REFA03_CHIP_TCA9539, AUDIO_DSP_RST_N, 1);
    _CHECK_RET(ret)
    delay_ctl(200);
    /*Clean software cache*/
    ret = reset(REFA03_CHIP_AK7738);
    _CHECK_RET(ret)
    ret = write_ctl_nocache(REFA03_CHIP_TCA9539, AUDIO_4MIC_RST_N, 0);
    _CHECK_RET(ret)
    delay_ctl(10);
    ret = write_ctl_nocache(REFA03_CHIP_TCA9539, AUDIO_4MIC_RST_N, 1);
    _CHECK_RET(ret)
    delay_ctl(10);

    ret = write_ctl_nocache(REFA03_CHIP_TCA9539, AUDIO_DIG_AMP_STANDBY_N, 1);
    _CHECK_RET(ret)
    delay_ctl(10);
    /* load firmware*/
    ret = burn_fw(REFA03_CHIP_AK7738, AK7738_FW_NO2_DSP1);
    _CHECK_RET(ret)
    ret = burn_fw(REFA03_CHIP_AK7738, AK7738_FW_NO2_DSP2);
    _CHECK_RET(ret)
    ret = reset(REFA03_CHIP_TAS6424);
    _CHECK_RET(ret)
    ret = reset(REFA03_CHIP_XF6020);
    _CHECK_RET(ret)
    ret = reset(REFA03_CHIP_TAS5404);
    _CHECK_RET(ret)
    ret = write_ctl_nocache(REFA03_CHIP_TCA9539, AUDIO_DIG_AMP_STANDBY_N, 0);
    _CHECK_RET(ret)
    ret = write_ctl_nocache(REFA03_CHIP_TCA9539, AUDIO_DIG_AMP_MUTE_N, 0);
    _CHECK_RET(ret)
    ret = write_ctl_nocache(REFA03_CHIP_TAS6424, TAS6424_00_80_RESET, 1);
    _CHECK_RET(ret)
    delay_ctl(10);
    return true;
}
/**
 *@brief sync path status by chip id.
 * it will excute read/write register value to codec.`
 *
 *@return true
 *@return false
 */
static bool refa03_path_sync(void)
{
    bool ret;
    _FUNC_LINE_PRT_
    ret = sync_ctl(REFA03_CHIP_AK7738);
    _CHECK_RET(ret);
    ret = sync_ctl(REFA03_CHIP_TAS6424);
    _CHECK_RET(ret);
    ret = sync_ctl(REFA03_CHIP_XF6020);
    _CHECK_RET(ret);
    ret = sync_ctl(REFA03_CHIP_TCA9539);
    _CHECK_RET(ret);
    ret = sync_ctl(REFA03_CHIP_TAS5404);
    _CHECK_RET(ret);
    return true;
}

/**
 * @brief here check codec exception and do some actions
 *
 * @return true
 * @return false
 */
static bool refa03_path_exception_check(void)
{
    bool ret;
    unsigned int val = 0;
    /* make reg 10, 11,12 cache dirty to read for check exception*/
    make_dirty_reg(REFA03_CHIP_TAS6424,TAS6424_10_CHANNEL_FAULTS);
    make_dirty_reg(REFA03_CHIP_TAS6424,TAS6424_11_GLOBAL_FAULTS_1);
    make_dirty_reg(REFA03_CHIP_TAS6424, TAS6424_12_GLOBAL_FAULTS_2);

    ret = read_ctl(REFA03_CHIP_TAS6424, TAS6424_10_80_CH1_OC, &val);
    _CHECK_RET_AND_VAL(ret, val, 0)
    ret = read_ctl(REFA03_CHIP_TAS6424, TAS6424_10_40_CH2_OC, &val);
    _CHECK_RET_AND_VAL(ret, val, 0)
    ret = read_ctl(REFA03_CHIP_TAS6424, TAS6424_10_20_CH3_OC, &val);
    _CHECK_RET_AND_VAL(ret, val, 0)
    ret = read_ctl(REFA03_CHIP_TAS6424, TAS6424_10_10_CH4_OC, &val);
    _CHECK_RET_AND_VAL(ret, val, 0)
    ret = read_ctl(REFA03_CHIP_TAS6424, TAS6424_10_08_CH1_DC, &val);
    _CHECK_RET_AND_VAL(ret, val, 0)
    ret = read_ctl(REFA03_CHIP_TAS6424, TAS6424_10_04_CH2_DC, &val);
    _CHECK_RET_AND_VAL(ret, val, 0)
    ret = read_ctl(REFA03_CHIP_TAS6424, TAS6424_10_02_CH3_DC, &val);
    _CHECK_RET_AND_VAL(ret, val, 0)
    ret = read_ctl(REFA03_CHIP_TAS6424, TAS6424_10_01_CH4_DC, &val);
    _CHECK_RET_AND_VAL(ret, val, 0)

    ret = read_ctl(REFA03_CHIP_TAS6424, TAS6424_11_08_PVDD_OV, &val);
    _CHECK_RET_AND_VAL(ret, val, 0)
    ret = read_ctl(REFA03_CHIP_TAS6424, TAS6424_11_04_VBAT_OV, &val);
    _CHECK_RET_AND_VAL(ret, val, 0)
    ret = read_ctl(REFA03_CHIP_TAS6424, TAS6424_11_02_PVDD_UV, &val);
    _CHECK_RET_AND_VAL(ret, val, 0)
    ret = read_ctl(REFA03_CHIP_TAS6424, TAS6424_11_01_VBAT_UV, &val);
    _CHECK_RET_AND_VAL(ret, val, 0)
    ret = read_ctl(REFA03_CHIP_TAS6424, TAS6424_12_10_OTSD, &val);
    _CHECK_RET_AND_VAL(ret, val, 0)

    make_dirty_ctl(REFA03_CHIP_TAS5404,TAS5404_13_STAT_REG5);
    ret = read_ctl(REFA03_CHIP_TAS5404, TAS5404_13_10_OTSHUTDOWN_CH1, &val);
    _CHECK_RET_AND_VAL(ret, val, 0)
    ret = read_ctl(REFA03_CHIP_TAS5404, TAS5404_13_20_OTSHUTDOWN_CH2, &val);
    _CHECK_RET_AND_VAL(ret, val, 0)
    ret = read_ctl(REFA03_CHIP_TAS5404, TAS5404_13_40_OTSHUTDOWN_CH3, &val);
    _CHECK_RET_AND_VAL(ret, val, 0)
    ret = read_ctl(REFA03_CHIP_TAS5404, TAS5404_13_80_OTSHUTDOWN_CH4, &val);
    _CHECK_RET_AND_VAL(ret, val, 0)

    return true;
}
static struct am_board_interface refa03_path = {
    refa03_path_chip_initialize, ///< refa03 board initialize
    refa03_path_chip_release,
    refa03_path_reset,
    refa03_path_op, ///< refa03 board path op code
    refa03_path_sync,
    refa03_path_exception_check,
    NULL, ///< user func
};

struct am_board_interface *get_refa03_board_interface(void)
{
    return &refa03_path;
}
