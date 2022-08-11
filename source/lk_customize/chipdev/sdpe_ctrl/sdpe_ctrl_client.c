/*
 * sdpe_ctrl_client.c
 *
 * Copyright (c) 2019 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: sdpe client driver provides remote function calls
 * to safety.
 *
 * Revision History:
 * -----------------
 */
#include <debug.h>

#ifdef SUPPORT_SDPE_RPC
#include "sdpe_ctrl_service.h"
#else
#include "sdpe_cb.h"
#endif

void sdpe_event_cb(uint32_t event_id, uint32_t arg)
{
    dprintf(ALWAYS, "%s: %d %d\n", __FUNCTION__, event_id, arg);
}