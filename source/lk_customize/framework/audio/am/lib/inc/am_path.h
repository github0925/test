/**
 * @file am_path.h
 * @author shao yi
 * @brief audio path functions
 * @version 0.1
 * @date 2021-01-14
 *
 * @copyright Copyright (c) 2021 Semidrive Semiconductor
 *
 */
#ifndef __AM_PATH_H__
#define __AM_PATH_H__
#include <stdio.h>
int init_path_status_tbl(unsigned int path_number);
unsigned int get_path_number(void);
unsigned int get_active_path_number(void);
int am_path_release(void);
void dump_path_status(void);

int add_path_by_id(unsigned int path_id, int vol);
int remove_path_by_id(unsigned int path_id);
/**
 *@brief reset audio path
 *
 *@return int
 */
int am_reset_active_path(void);
int am_check_task(void);

int am_get_path_vol_changed(unsigned int path_id, int vol);
int am_get_path_vol(unsigned int path_id);
int am_get_path_mute(unsigned int path_id);
int am_set_path_vol(unsigned int path_id, int vol);
int am_set_path_mute(unsigned int path_id, int mute);
int am_get_path_active_status(unsigned int path_id);

#endif