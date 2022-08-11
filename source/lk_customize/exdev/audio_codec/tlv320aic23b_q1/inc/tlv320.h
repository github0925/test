/*
 * tlv320.h
 *
 * Copyright (c) 2018 Semidrive Semiconductor.
 * All rights reserved.
 *
 *
 */

#ifndef __TLV320_H__
#define __TLV320_H__

/**
 * @brief get tlv320aic23b_q1 codec controller driver interface
 *
 * @return driver interface pointer
*/
const struct au_codec_dev_ctrl_interface
*sdrv_tlv320_get_controller_interface(void);

/**
 * @brief get tlv320aic23b_q1 codec dev info
 *
 * @return a codec dev instance
*/
struct audio_codec_dev
*sdrv_tlv320_get_dev(int codec_id);

#endif