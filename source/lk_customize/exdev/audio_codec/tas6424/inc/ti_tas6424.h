/*
 * tas6424.h
 *
 * Copyright (c) 2018 Semidrive Semiconductor.
 * All rights reserved.
 *
 *
 */

#ifndef __TAS6424_H__
#define __TAS6424_H__

/**
 * @brief get tas6424 codec controller driver interface
 *
 * @return driver interface pointer
*/
const struct au_codec_dev_ctrl_interface
*sdrv_tas6424_get_controller_interface(void);

/**
 * @brief get tas6424 codec dev info
 *
 * @return a codec dev instance
 */
struct audio_codec_dev *sdrv_tas6424_get_dev(int codec_id);

#endif