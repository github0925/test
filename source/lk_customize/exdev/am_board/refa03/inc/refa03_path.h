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
#ifndef __REF03_PATH_H__
#define __REF03_PATH_H__
/**
 *@brief chip id define must different and match to
 *
 */
typedef enum {
    REFA03_CHIP_AK7738,
    REFA03_CHIP_TCA9539,
    REFA03_CHIP_TAS6424,
    REFA03_CHIP_XF6020,
    REFA03_CHIP_TAS5404,
    REFA03_CHIP_NUMB,
} REFA03_CHIPS;

struct am_board_interface *get_refa03_board_interface(void);
#endif