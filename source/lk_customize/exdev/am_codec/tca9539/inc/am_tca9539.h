/**
 *@file am_tca9539.h
 *@author yi shao (yi.shao@semidrive.com)
 *@brief GPIO Driver for audio manager.
 *@version 0.1
 *@date 2021-05-10
 *
 *@copyright Copyright (c) 2021 Semidrive Semiconductor
 *
 */
#ifndef __AM_TCA9539_H__
#define __AM_TCA9539_H__
/**
 * @brief virtual reg one for one GPIO PIN
 *
 */
#define TCA9539_00_PIN_REG0 0x0
#define TCA9539_01_PIN_REG1 0x1
#define TCA9539_02_PIN_REG2 0x2
#define TCA9539_03_PIN_REG3 0x3
#define TCA9539_04_PIN_REG4 0x4
#define TCA9539_05_PIN_REG5 0x5
#define TCA9539_06_PIN_REG6 0x6
#define TCA9539_07_PIN_REG7 0x7
#define TCA9539_08_PIN_REG8 0x8
#define TCA9539_09_PIN_REG9 0x9
#define TCA9539_0A_PIN_REG10 0xa
#define TCA9539_0B_PIN_REG11 0xb
#define TCA9539_0C_PIN_REG12 0xc
#define TCA9539_0D_PIN_REG13 0xd
#define TCA9539_0E_PIN_REG14 0xe
#define TCA9539_0F_PIN_REG15 0xf
/**
 * @brief TCA9539 i2c io expander control enum
 *
 */
enum {
    TCA9539_00_01_P00, ///< AUDIO_ANA_AMP_FAULT_N
    TCA9539_01_01_P01, ///< AUDIO_ANA_AMP_CLIP_OTW_N
    TCA9539_02_01_P02, ///< AUDIO_DIG_AMP_STANDBY_N
    TCA9539_03_01_P03, ///< AUDIO_DIG_AMP_MUTE_N
    TCA9539_04_01_P04, ///< AUDIO_DIG_AMP_FAULT_N
    TCA9539_05_01_P05, ///< AUDIO_DIG_AMP_WARN_N
    TCA9539_06_01_P06, ///<
    TCA9539_07_01_P07, ///< AUDIO_DSP_RST_N
    TCA9539_08_01_P08,
    TCA9539_09_01_P09,
    TCA9539_0A_01_P10,
    TCA9539_0B_01_P11,
    TCA9539_0C_01_P12,
    TCA9539_0D_01_P13,
    TCA9539_0E_01_P14,
    TCA9539_0F_01_P15, ///< AUDIO_4MIC_RST_N
    TCA9539_CTL_NUMB   // special ctl for last ctl
};
/**
 * @brief get tca9539 control interface.
 *
 * @return struct am_ctl_interface*
 */
struct am_ctl_interface *sdrv_tca9539_get_ctl_interface(void);
#endif