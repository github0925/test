/*
 * sdpe_stat.h
 *
 * Copyright (c) 2020 SemiDrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */
#ifndef _SDPE_STAT_H
#define _SDPE_STAT_H

#include <stdint.h>

#define PDU_ISR             0U
#define PDU_GET_RX_PDU_ID   1U
#define PDU_REQUIRE         2U
#define PDU_LOST            3U
#define PDU_RELEASE         4U
#define PDU_ROUTE           5U
#define PDU_COPY            6U
#define PDU_DO_ROUTE        7U

typedef struct stat_info {
    uint64_t            timestamp;
    uint8_t             port;
    uint8_t             bus;
    uint32_t            time;
    uint8_t             route_event;
} __attribute__((aligned(8))) stat_info_t;

struct ring {
    uint32_t            size;
    uint32_t            offset;
    uint32_t            head;
    uint32_t            tail;
} __attribute__((aligned(4)));

typedef struct stat_buf {
    /**
     * Magic number should be 0x73746174
     * "stat".
     */
    uint32_t            magic;

    /**
     * Use two ring to implement ping-pong
     * buffer.
     */
    struct ring         ring[2];

    /**
     * read & write index are used
     * for indexing reading & writing ring.
     * The two variables are used to implement
     * lock free access to stat buffer
     * between 2 different threads or cores
     * in the way that both are only written
     * by one thread or core.
     */
    uint8_t             read_idx;
    uint8_t             write_idx;

    uint32_t            reserved0;
    uint32_t            reserved1;
    uint32_t            reserved2;
    uint32_t            reserved3;
    uint32_t            reserved4;
    uint32_t            reserved5;
    stat_info_t         data[0];
} __attribute__((aligned(8))) stat_buf_t;

#endif
