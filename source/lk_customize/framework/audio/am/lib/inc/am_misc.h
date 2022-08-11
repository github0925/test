/**
 *@file am_misc.h
 *@author yi shao (yi.shao@semidrive.com)
 *@brief audio manager misc functions set
 *@version 0.1
 *@date 2021-05-19
 *
 *@copyright Copyright (c) 2021 Semidrive Semiconductor
 *
 */
#ifndef __AM_MISC_H__
#define __AM_MISC_H__
/**
 *@brief Get the linear vol object, this function is used to
 *       calculate volume index by linear mapping.
 *
 *@param vol input vol index , from 0 ~100
 *@param max_val  max volume index
 *@param min_val  min volume index
 *@return int     volume index result for vol.
 */
int am_get_linear_vol(int vol, int max_val, int min_val);
#endif