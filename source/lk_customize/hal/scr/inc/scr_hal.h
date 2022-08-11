/*
 * scr_hal.h
 *
 * Copyright (c) 2019 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: SCR HAL header.
 *
 * Platform and driver codes call scr chip driver directly. Applications
 * need to use HAL SCR interfaces, which provide resource control.
 *
 * Revision History:
 * -----------------
 */
#ifndef _SCR_HAL_H
#define _SCR_HAL_H

#include "scr.h"

typedef uint64_t scr_handle_t;

scr_handle_t hal_scr_create_handle(scr_signal_t scr_signal);
void hal_scr_delete_handle(scr_handle_t);
uint32_t hal_scr_get(scr_handle_t handle);
bool hal_scr_set(scr_handle_t handle, uint32_t value);
bool hal_scr_lock(scr_handle_t handle);
bool hal_scr_is_locked(scr_handle_t handle);

#endif /* _SCR_HAL_H */
