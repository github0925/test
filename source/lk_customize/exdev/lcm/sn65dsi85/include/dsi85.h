/*
* dsi85.h
*
* Copyright (c) 2019 Semidrive Semiconductor.
* All rights reserved.
*
* Description:
*
* Revision History:
* -----------------
* 011, 06/28/2019 BI create this file
*/
#ifndef __SN65DSI85_H__
#define __SN65DSI85_H__

void sn65dsi85_regs_init(void* i2c_handle, uint32_t slave_addr);
void sn65dsi85_dump(void* i2c_handle, uint32_t slave_addr);
void sn65dsi85_enable(void* i2c_handle, uint32_t slave_addr);
void sn65dsi85_lock_check(void* i2c_handle, uint32_t slave_addr);

#endif //__SN65DSI85_H__
