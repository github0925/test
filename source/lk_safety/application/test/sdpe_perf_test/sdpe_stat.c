/**
 * @file stat.c
 *
 * Copyright (c) 2019 Semidrive Semiconductor.
 * All rights reserved.
 */

#include <debug.h>
#include <assert.h>
#include "image_cfg.h"
#include "sdpe_stat.h"

#define full_barrier()  __asm__ volatile("dmb" : : : "memory")

#define PORT_MAX 8U

static stat_ring_t *g_stat_ring = (stat_ring_t *)SDPE_STAT_MEMBASE;

static uint32_t stat_ring_used(void);
static bool stat_ring_empty(void);
static void port_stat_parse(void);
static void show_stat_result(float cnt_freq);

void stat_flush(void)
{
    uint8_t cnt = 0U;

    if (likely(g_stat_ring->magic == 0x73746174U)) {
        /* Flush the last recorded data. */
        while (!stat_ring_empty() && (cnt++ < g_stat_ring->size)) {
            /* Move read index ahead. */
            g_stat_ring->rd_idx++;
        }
    }
}

void stat_dump(float cnt_freq)
{
    uint8_t dump_cnt = 0U;
    bool show_result = false;

    if (likely(g_stat_ring->magic == 0x73746174U)) {
        /* Read the last recorded data.
         * Due to the ring buffer store strategy that
         * drop data when ring full, Only the data before
         * or after dump is contiguous.
         */
        while (!stat_ring_empty() && (dump_cnt++ < g_stat_ring->size)) {
            port_stat_parse();

            /* In our scenario, the barrier here is used to avoid
             * AXI (4 and higher) reorder on transactions with different
             * ID and make sure read ring data has been read out before
             * updated read index observed.
             */
            full_barrier();

            /* Move read index ahead. */
            g_stat_ring->rd_idx++;

            show_result = true;
        }

        if (show_result) {
            /* Flush statistics data recorded during dump,
             * because the speed of dump may affect the speed
             * of data recording when ring full.
             */
            stat_flush();

            show_stat_result(cnt_freq);
        }
    }
}

static uint32_t stat_ring_used(void)
{
    return g_stat_ring->wr_idx - g_stat_ring->rd_idx;
}

static bool stat_ring_empty(void)
{
    return !stat_ring_used();
}

struct stat_time
{
    uint32_t    min;
    uint32_t    max;
    uint32_t    aver;

    uint32_t    cnt;
    uint32_t    sum;
};

struct stat_data
{
    bool valid;
    bool svalid;
    struct stat_time isr_time;
    struct stat_time route_time;
    uint64_t s;
    uint64_t e;
    uint32_t rx_cnt;
    uint32_t tx_cnt;
    uint32_t rx_lost_cnt;
    uint32_t tx_lost_cnt;
};

static struct stat_data g_route_stat[PORT_MAX];

static void port_stat_parse(void)
{
    uint32_t rd_offset = g_stat_ring->rd_idx % g_stat_ring->size;
    stat_info_t *buf = &g_stat_ring->data[rd_offset];
    uint8_t port_id = buf->port;

    ASSERT(port_id < PORT_MAX);

    g_route_stat[port_id].valid = true;

    if (!g_route_stat[port_id].svalid) {
        g_route_stat[port_id].svalid = true;
        g_route_stat[port_id].s = buf->timestamp;
    }
    g_route_stat[port_id].e = buf->timestamp;

    if ((buf->route_event == PDU_ISR) ||
        (buf->route_event == PDU_ROUTE)) {
        struct stat_time *t;

        if (buf->route_event == PDU_ISR) {
            t = &g_route_stat[port_id].isr_time;
        }
        else {
            t = &g_route_stat[port_id].route_time;
            g_route_stat[port_id].tx_cnt++;
        }

        if ((t->min == 0U) || (t->min > buf->time)) {
            t->min = buf->time;
        }
        if (t->max < buf->time) {
            t->max = buf->time;
        }
        t->cnt++;
        t->sum += buf->time;
        t->aver = t->sum / t->cnt;
    }
    else if (buf->route_event == PDU_REQUIRE) {
        g_route_stat[port_id].rx_cnt++;
    }
    else if (buf->route_event == PDU_LOST) {
        g_route_stat[port_id].rx_lost_cnt++;
    }
    else if (buf->route_event == PDU_TX_FAIL) {
        g_route_stat[port_id].tx_lost_cnt++;
    }
    else {
        /* Do nothing. */
    }
}

static void show_stat_result(float cnt_freq)
{
    static uint32_t print_line;

    /* Move cursor upward by the number of line has printed. */
    dprintf(ALWAYS, "\033[%dA", print_line);
    /* Clear contents from cursor to the end. */
    dprintf(ALWAYS, "\033[K");
    dprintf(ALWAYS, "========================================");
    dprintf(ALWAYS, "SDPE statistics data");
    dprintf(ALWAYS, "========================================\n");
    dprintf(ALWAYS, "%-15s%-17s%-17s%-17s%-17s%-10s%-10s%-10s%-10s\n", "",
            "min(us)", "max(us)", "average(us)", "throughput(fps)", "rx", "tx",
            "rx lost", "tx lost");
    print_line = 2U;
    for (size_t i = 0; i < PORT_MAX; i++) {
        const char *port_name[PORT_MAX] = {"CAN", "CAN TP", "LIN", "LIN TP",
                                          "VCAN", "VLIN", "DoIP", "Eth"};
        if (g_route_stat[i].valid) {
            dprintf(ALWAYS, "%s.isr:%-7s%-17f%-17f%-17f\n",
                    port_name[i], "",
                    (double)g_route_stat[i].isr_time.min / cnt_freq,
                    (double)g_route_stat[i].isr_time.max / cnt_freq,
                    (double)g_route_stat[i].isr_time.aver / cnt_freq);
            dprintf(ALWAYS, "%s.route:%-5s%-17f%-17f%-17f%-17lld%-10d%-10d%-10d%-10d\n",
                    port_name[i], "",
                    (double)g_route_stat[i].route_time.min / cnt_freq,
                    (double)g_route_stat[i].route_time.max / cnt_freq,
                    (double)g_route_stat[i].route_time.aver / cnt_freq,
                    (uint64_t)g_route_stat[i].tx_cnt * (uint64_t)(cnt_freq *1000000U) /
                                    (uint32_t)(g_route_stat[i].e - g_route_stat[i].s),
                    g_route_stat[i].rx_cnt,
                    g_route_stat[i].tx_cnt,
                    g_route_stat[i].rx_lost_cnt,
                    g_route_stat[i].tx_lost_cnt);

            print_line += 2U;
        }
    }

    for (size_t i = 0; i < PORT_MAX; i++) {
        struct stat_data *stat_data = &g_route_stat[i];
        stat_data->valid = stat_data->svalid = false;
        stat_data->isr_time.cnt = stat_data->isr_time.sum = 0U;
        stat_data->route_time.cnt = stat_data->route_time.sum = 0U;
        stat_data->s = stat_data->e = 0U;
        stat_data->rx_cnt = stat_data->tx_cnt = 0U;
        stat_data->rx_lost_cnt = stat_data->tx_lost_cnt = 0U;
    }
}
