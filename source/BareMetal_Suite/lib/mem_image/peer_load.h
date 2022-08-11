/*
 * Copyright (c) 2019 Semidrive Semiconductor.
 * All rights reserved.
 */

#ifndef __PEER_LOAD_H_
#define __PEER_LOAD_H_

#include <stdint.h>

#define PEER_LOAD_MSG_TAG      0xFC
#define PEER_LOAD_MSG_CMD      0xF1
#define PEER_LOAD_MSG_VERSION     1
#define PEER_LOAD_MSG_LEN         8

struct peer_boot_message {
    uint8_t tag;
    uint8_t size;
    uint8_t command;
    uint8_t version;
    uint32_t parameter;
};

#define MK_PEER_LOAD_MSG(addr) \
                    {\
                        .tag = PEER_LOAD_MSG_TAG,\
                        .size = sizeof(struct peer_boot_message),\
                        .command = PEER_LOAD_MSG_CMD,\
                        .version = PEER_LOAD_MSG_VERSION,\
                        .parameter = addr\
                    }
#endif
