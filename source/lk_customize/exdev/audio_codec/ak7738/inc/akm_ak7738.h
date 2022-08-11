/*
 * ak7738.h
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 *
 */

#ifndef __AK7738_H__
#define __AK7738_H__

/**
 * @brief get ak7738 codec controller driver interface
 *
 * @return driver interface pointer
*/
const struct au_codec_dev_ctrl_interface
*sdrv_ak7738_get_controller_interface(void);

/**
 * @brief get ak7738 codec dev info
 *
 * @return a codec dev instance
*/
struct audio_codec_dev
*sdrv_ak7738_get_dev(int codec_id);

#endif