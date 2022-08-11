/**
 * @file stat.h
 *
 * Copyright (c) 2019 Semidrive Semiconductor.
 * All rights reserved.
 */

#include <stdint.h>

#define PDU_ISR             0U
#define PDU_GET_RX_PDU_ID   1U
#define PDU_REQUIRE         2U
#define PDU_LOST            3U
#define PDU_RELEASE         4U
#define PDU_ROUTE           5U
#define PDU_TX_FAIL         6U

typedef struct stat_info {
    uint64_t            timestamp;
    uint8_t             port;
    uint8_t             bus;
    uint8_t             route_event;
    uint32_t            time;
} __attribute__((aligned(8))) stat_info_t;

typedef struct stat_ring {
    /*
     * Magic number should be 0x73746174
     * "stat".
     */
    uint32_t            magic;

    uint32_t            size;

    /*
     * read & write index are used
     * for indexing reading & writing ring.
     * The two variables are used to implement
     * lock free reading stat ring in the way
     * that first access (r/w) buffer then update
     * index, and only one thread in one core update
     * rd_idx.
     */
    uint32_t            rd_idx;
    uint32_t            wr_idx;

    uint32_t            reserved;
    stat_info_t         data[0];
} __attribute__((aligned(8))) stat_ring_t;

/**
 * @brief Flush bad SDPE statistics data.
 */
void stat_flush(void);

/**
 * @brief Dump SDPE statistics data.
 * @param cnt_freq Counter frequency, unit MHz.
 */
void stat_dump(float cnt_freq);
