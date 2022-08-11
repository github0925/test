/*
 * cpu_hal.h
 *
 * Copyright (c) 2020 SemiDrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */

#ifndef _CPU_HAL_H
#define _CPU_HAL_H

#include <stdbool.h>

typedef enum sd_cpu_id{
    CPU_ID_MIN = 0x0,
    CPU_ID_AP1,
    CPU_ID_AP2,
    CPU_ID_MP,
    CPU_ID_SEC,
    CPU_ID_SAF,
    CPU_ID_MAX
}sd_cpu_id;

bool hal_cpu_create_handle(void** handle_p);
bool hal_cpu_boot(void* handle, sd_cpu_id id, uint64_t entry);
bool hal_cpu_release_handle(void* handle);
#endif
