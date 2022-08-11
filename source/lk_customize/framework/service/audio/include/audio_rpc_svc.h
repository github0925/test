/*
 * Copyright (c) 2020 Semidrive Semiconductor, Inc.
 * All rights reserved.
 */
#ifndef __AUDIO_RPC_SERVICE_H__
#define __AUDIO_RPC_SERVICE_H__

#include <dcf.h>
#include <debug.h>
#include <kernel/event.h>
#include <kernel/semaphore.h>
#include <lib/reg.h>
#include <stdlib.h>
#include <sys/types.h>

#if ENABLE_AUDIO_AGENT
#include "animation_config.h"
#include "audio_agent.h"

#endif
#include "i2s_hal.h"

#define AUDIO_SERVICE_MBOX_ADDR (0xd0)

struct i2s_buffer {
    u8 data[16];
};

struct audio_server {
    mutex_t mutex;
    struct i2s_buffer *buffer;
    struct dcf_notifier *notifier;
    u32 rproc;
    u32 mbox_addr;
    event_t event;
    bool initialized;
#if ENABLE_AUDIO_AGENT
    au_agent_handle_t audio_handle;
#endif
};

struct audio_rpc_cmd {
    u16 op;
    union {
        u8 data[16];
        struct {
            u32 data;
        } send_msg;
        struct {
            u32 data;
            u32 val0;
            u32 val1;
        } recv_msg;
    } msg;
};

struct audio_rpc_result {
    u16 op;
    union {
        u8 data[14];
        struct {
            u32 data;
        } fmt;
        struct {
            u32 data;
        } buffer;
    } msg;
};

#endif