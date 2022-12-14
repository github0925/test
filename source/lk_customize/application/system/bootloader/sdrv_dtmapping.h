/*
 * sdrv,dt_mapping.h
 *
 * Copyright(c); 2020 Semidrive
 *
 * Author: Yujin <jin.yu@semidrive.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#ifndef __SDRV_DT_MAPPING_H__
#define __SDRV_DT_MAPPING_H__

/* HW ID */
#define SDRV_CHIPID_ALL 0
#define SDRV_CHIPID_X9E 1
#define SDRV_CHIPID_X9M 2
#define SDRV_CHIPID_X9H 3
#define SDRV_CHIPID_X9P 4
#define SDRV_CHIPID_G9S 5
#define SDRV_CHIPID_G9X 6
#define SDRV_CHIPID_G9E 7
#define SDRV_CHIPID_V9L 8
#define SDRV_CHIPID_V9T 9
#define SDRV_CHIPID_V9F 10
#define SDRV_CHIPID_D9A 11

/* board type bit 8 ~ 7 */
#define SDRV_BOARD_TYPE_ALL 0
#define SDRV_BOARD_TYPE_EVB 1
#define SDRV_BOARD_TYPE_REF 2
#define SDRV_BOARD_TYPE_CUS 3

/* board id major bit 6 ~ 4 */
#define SDRV_BOARDID_MAJOR_ALL 0
#define SDRV_BOARDID_MAJOR_A 1
#define SDRV_BOARDID_MAJOR_B 2
#define SDRV_BOARDID_MAJOR_C 3

/* board id minor bit 3 ~ 0 */
#define SDRV_BOARDID_MINOR_ALL 0
#define SDRV_BOARDID_MINOR_1 1
#define SDRV_BOARDID_MINOR_2 2
#define SDRV_BOARDID_MINOR_3 3
#define SDRV_BOARDID_MINOR_4 4
#define SDRV_BOARDID_MINOR_5 5

#define SDRV_BOARDID_MAJOR_OFFSET 4
#define SDRV_BOARDID_ALL 0
#define SDRV_BOARDID_A01 ((SDRV_BOARDID_MAJOR_A << SDRV_BOARDID_MAJOR_OFFSET) | SDRV_BOARDID_MINOR_1)
#define SDRV_BOARDID_A02 ((SDRV_BOARDID_MAJOR_A << SDRV_BOARDID_MAJOR_OFFSET) | SDRV_BOARDID_MINOR_2)
#define SDRV_BOARDID_A03 ((SDRV_BOARDID_MAJOR_A << SDRV_BOARDID_MAJOR_OFFSET) | SDRV_BOARDID_MINOR_3)
#define SDRV_BOARDID_A04 ((SDRV_BOARDID_MAJOR_A << SDRV_BOARDID_MAJOR_OFFSET) | SDRV_BOARDID_MINOR_4)
#define SDRV_BOARDID_A05 ((SDRV_BOARDID_MAJOR_A << SDRV_BOARDID_MAJOR_OFFSET) | SDRV_BOARDID_MINOR_5)
#define SDRV_BOARDID_A06 ((SDRV_BOARDID_MAJOR_A << SDRV_BOARDID_MAJOR_OFFSET) | SDRV_BOARDID_MINOR_6)

#define SDRV_BOARDID_B01 ((SDRV_BOARDID_MAJOR_B << SDRV_BOARDID_MAJOR_OFFSET) | SDRV_BOARDID_MINOR_1)
#define SDRV_BOARDID_B02 ((SDRV_BOARDID_MAJOR_B << SDRV_BOARDID_MAJOR_OFFSET) | SDRV_BOARDID_MINOR_2)
#define SDRV_BOARDID_B03 ((SDRV_BOARDID_MAJOR_B << SDRV_BOARDID_MAJOR_OFFSET) | SDRV_BOARDID_MINOR_3)
#define SDRV_BOARDID_B04 ((SDRV_BOARDID_MAJOR_B << SDRV_BOARDID_MAJOR_OFFSET) | SDRV_BOARDID_MINOR_4)
#define SDRV_BOARDID_B05 ((SDRV_BOARDID_MAJOR_B << SDRV_BOARDID_MAJOR_OFFSET) | SDRV_BOARDID_MINOR_5)
#define SDRV_BOARDID_B06 ((SDRV_BOARDID_MAJOR_B << SDRV_BOARDID_MAJOR_OFFSET) | SDRV_BOARDID_MINOR_6)

/* Feature ID */
#define SDRV_FEAT_ID_NO_FEAT 0
/* TODO */

#endif
