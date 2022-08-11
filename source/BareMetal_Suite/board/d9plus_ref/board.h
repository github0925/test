/********************************************************
 *          Copyright(c) 2020   Semidrive               *
 ********************************************************/

#ifndef __BOARD_H__
#define __BOARD_H__

#include "soc.h"

int32_t change_vdd_ap_voltage(uint32_t mv);

/*
 * @brief   change DDR voltages
 * @para    vdd_id
 *                  1 - VDD2(1.1v by default), SW_B1
 *                  2 - VDDQ(0.6v by default), SW_B2
 *                  3 - VDD1(1.8v by default), SW_B3
 * @para    mv  voltage value to be set, in mv.
 */
int setup_ddr_voltage(uint32_t vdd_id, uint32_t mv);

int32_t board_setup(uint32_t rev, uint8_t type, uint8_t maj, uint8_t min);
int board_info(void * buf, uint32_t len);

#endif
