/**
 *@file refa04_path.h
 *@author yi shao (yi.shao@semidrive.com)
 *@brief
 *@version 0.1
 *@date 2021-05-10
 *
 *@copyright Copyright (c) 2021 Semidrive Semiconductor
 *
 */
#ifndef __REF04_PATH_H__
#define __REF04_PATH_H__
/**
 *@brief chip id define must different and match to
 *
 */
typedef enum {
    REFA04_CHIP_AK7738,
    REFA04_CHIP_TCA9539,
    REFA04_CHIP_TAS6424,
    REFA04_CHIP_XF6020,
    REFA04_CHIP_TAS5404,
    REFA04_CHIP_NUMB,
} REFA04_CHIPS;


struct am_board_interface *get_refa04_board_interface(void);
#endif