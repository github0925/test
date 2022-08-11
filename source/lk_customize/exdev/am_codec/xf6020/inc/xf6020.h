/**
 *@file xf6020.h
 *@author liang lou (liang.lou@semidrive.com)
 *@brief xf6020 audio manager codec dev driver
 *@version 0.1
 *@date 2021-05-17
 *
 *@copyright Copyright (c) 2021 Semidrive Semiconductor
 *
 */
#ifndef __XF6020_H__
#define __XF6020_H__
/**
 * @brief XF6020 Register Definition
 *
 */
#define XF6020_00_VOICE_MODE 0x0
#define XF6020_01_PARAM 0x1
#define XF6020_02_BYPASS 0x2
#define XF6020_0C_VERSION 0xc

/**
 *@brief XF6020 control definition
 *
 */
enum {
    XF6020_00_FF_VOICEMODE,
    XF6020_01_FF_PARAM,
    XF6020_02_FF_BYPASS,
    XF6020_0C_FF_VERSION,
    XF6020_CTL_NUMB, // special ctl for last ctl

};

/**
 * @brief voice mode definition.
 *
 */
typedef enum {
    /*! four sound areas*/
    FOUR_SND_AREAS = 0,
    /*! two sound areas*/
    TWO_SND_AREAS = 1,
} XF6020VoiceMode;
/**
 * @brief xf6020's parameters
 *
 */
typedef enum {
    AEC_TEL = 0, /*!< telecom mode*/
    AEC_MUSIC = 1,
    USE_BEAM_0 = 2,
    USE_BEAM_1 = 3,
    USE_BEAM_2 = 4,
    USE_BEAM_3 = 5,
    USE_BEAM_OFF = 6,

    WAKEUP_TWO = 7,
    WAKEUP_FOUR = 8,
    WAKEUP_OFF = 9,

    REC_BEAM = 10,
    REC_MAE = 11,
    REC_OFF = 12,

    DUAL_MAE = 13,
    DUAL_MAB = 14,
    DUAL_MAE_MICL = 15,
    DUAL_MAE_MICR = 16,
    DUAL_MAE_REFL = 17,
    DUAL_MAE_REFR = 18,
    DUAL_MAE_REFMIX = 19,
    DUAL_PHONE = 20,
    PARAME_VALUE_MAX
} XF6020ParamValue;

/**
 * @brief get xf6020 codec ctl interface
 *
 * @return struct am_ctl_interface*
 */
struct am_ctl_interface *sdrv_xf6020_get_ctl_interface(void);

#endif
