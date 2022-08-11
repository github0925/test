/**
 * @file am_ctl.h
 * @author shao yi
 * @brief
 * @version 0.1
 * @date 2021-01-15
 *
 * @copyright Copyright (c) 2021 Semidrive Semiconductor
 *
 */
#ifndef __AM_CTL_H__
#define __AM_CTL_H__
#include <stdio.h>
bool am_reset(unsigned int chip_id);
bool am_burn_fw(unsigned int chip_id, int fw_no);
bool am_sync_ctl(unsigned int chip_id);
bool am_read_ctl(unsigned int chip_id, unsigned int ctl_name,
                 unsigned int *val);
bool am_write_ctl(unsigned int chip_id, unsigned int ctl_name,
                  unsigned int val);
bool am_write_ctl_nocache(unsigned int chip_id, unsigned int ctl_name,
                          unsigned int val);

bool am_read_reg(unsigned int chip_id, unsigned int reg_addr,
                 unsigned int *val);
bool am_write_reg(unsigned int chip_id, unsigned int reg_addr,
                  unsigned int val);
bool am_write_reg_nocache(unsigned int chip_id, unsigned int reg_addr,
                          unsigned int val);

/*TODO: No tested*/
bool am_make_dirty_ctl(unsigned int chip_id, unsigned int ctl_name);
/*TODO: No tested*/
bool am_make_dirty_chip(unsigned int chip_id);
bool am_make_dirty_reg(unsigned int chip_id, unsigned int reg_addr);
bool am_dump_regs(int chip_id);
#endif