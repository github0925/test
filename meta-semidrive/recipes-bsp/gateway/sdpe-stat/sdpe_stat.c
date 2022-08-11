/*
 * sdpe_stat.c
 *
 * Copyright (c) 2020 SemiDrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include "sdpe_stat.h"

#define PORT_MAX 11U
#define SYS_CNT_FREQ 800

#define SDPE_STAT_ADDR      0x140000
#define SDPE_STAT_SIZE      0x20000

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef unlikely
#define unlikely(x) __builtin_expect(!!(x), 0)
#endif

#ifndef likely
#define likely(x) __builtin_expect(!!(x), 1)
#endif

#define full_barrier()  asm volatile("dmb sy" : : : "memory")

struct stat_time {
    uint32_t    not_empty;
    uint32_t    min_raw;
    uint32_t    max_raw;
    double      min;
    double      max;
    double      aver;
};

static void port_stat_parse(stat_buf_t *stat_buf)
{
    struct ring *rring = &stat_buf->ring[stat_buf->read_idx];
    uint32_t roff = rring->offset;
    static struct stat_time isr_time[PORT_MAX];
    static struct stat_time get_pdu_time[PORT_MAX];
    static struct stat_time route_time[PORT_MAX];
    static struct stat_time pdu_copy_time[PORT_MAX];
    static struct stat_time pdu_do_route_time[PORT_MAX];

    uint64_t start = stat_buf->data[rring->head + roff].timestamp;
    uint64_t end;

    if (rring->tail == 0U) {
        end = stat_buf->data[rring->size - 1U + roff].timestamp;
    }
    else {
        end = stat_buf->data[rring->tail - 1U + roff].timestamp;
    }

    uint32_t isr_counter[PORT_MAX] = {0U};
    uint32_t rx_pdu_counter[PORT_MAX] = {0U};
    uint32_t tx_pdu_counter[PORT_MAX] = {0U};
    uint32_t lost_pdu_counter[PORT_MAX] = {0U};
    uint32_t do_route_pdu_counter[PORT_MAX] = {0U};
    uint64_t isr_total_time[PORT_MAX] = {0U};
    uint64_t route_total_time[PORT_MAX] = {0U};
    uint64_t do_route_total_time[PORT_MAX] = {0U};

    uint32_t i = rring->head;

    do {
        //printf("[%ld] Port %d, bus %d, event %d, cost time %d\n",
        //stat_buf->data[i + roff].timestamp, stat_buf->data[i + roff].port, stat_buf->data[i + roff].bus,
        //stat_buf->data[i + roff].route_event, stat_buf->data[i + roff].time);

        uint8_t port = stat_buf->data[i + roff].port;
        route_time[port].not_empty = TRUE;

        switch (stat_buf->data[i + roff].route_event) {
            case PDU_ISR:
                isr_counter[port]++;
                isr_total_time[port] += stat_buf->data[i + roff].time;

                if ((isr_time[port].min_raw > stat_buf->data[i + roff].time) ||
                        unlikely(isr_time[port].min_raw == 0U)) {
                    isr_time[port].min_raw = stat_buf->data[i + roff].time;
                    isr_time[port].min = (double)isr_time[port].min_raw / SYS_CNT_FREQ;
                }

                if (isr_time[port].max_raw < stat_buf->data[i + roff].time) {
                    isr_time[port].max_raw = stat_buf->data[i + roff].time;
                    isr_time[port].max = (double)isr_time[port].max_raw / SYS_CNT_FREQ;
                }

                break;

            case PDU_GET_RX_PDU_ID:
                if ((get_pdu_time[port].min_raw > stat_buf->data[i + roff].time) ||
                        unlikely(get_pdu_time[port].min_raw == 0U)) {
                    get_pdu_time[port].min_raw = stat_buf->data[i + roff].time;
                    get_pdu_time[port].min = (double)get_pdu_time[port].min_raw / SYS_CNT_FREQ;
                }

                if (get_pdu_time[port].max_raw < stat_buf->data[i + roff].time) {
                    get_pdu_time[port].max_raw = stat_buf->data[i + roff].time;
                    get_pdu_time[port].max = (double)get_pdu_time[port].max_raw / SYS_CNT_FREQ;
                }

                break;

            case PDU_REQUIRE:
                rx_pdu_counter[port]++;
                break;

            case PDU_LOST:
                lost_pdu_counter[port]++;
                break;

            case PDU_ROUTE:
                tx_pdu_counter[port]++;
                route_total_time[port] += stat_buf->data[i + roff].time;

                if ((route_time[port].min_raw > stat_buf->data[i + roff].time) ||
                        unlikely(route_time[port].min_raw == 0U)) {
                    route_time[port].min_raw = stat_buf->data[i + roff].time;
                    route_time[port].min = (double)route_time[port].min_raw / SYS_CNT_FREQ;
                }

                if (route_time[port].max_raw < stat_buf->data[i + roff].time) {
                    route_time[port].max_raw = stat_buf->data[i + roff].time;
                    route_time[port].max = (double)route_time[port].max_raw / SYS_CNT_FREQ;
                }

                break;

            case PDU_COPY:
                if ((pdu_copy_time[port].min_raw > stat_buf->data[i + roff].time) ||
                        unlikely(pdu_copy_time[port].min_raw == 0U)) {
                    pdu_copy_time[port].min_raw = stat_buf->data[i + roff].time;
                    pdu_copy_time[port].min = (double)pdu_copy_time[port].min_raw / SYS_CNT_FREQ;
                }

                if (pdu_copy_time[port].max_raw < stat_buf->data[i + roff].time) {
                    pdu_copy_time[port].max_raw = stat_buf->data[i + roff].time;
                    pdu_copy_time[port].max = (double)pdu_copy_time[port].max_raw / SYS_CNT_FREQ;
                }

                break;

            case PDU_DO_ROUTE:
                do_route_pdu_counter[port]++;
                do_route_total_time[port] += stat_buf->data[i + roff].time;

                if ((pdu_do_route_time[port].min_raw > stat_buf->data[i + roff].time) ||
                        unlikely(pdu_do_route_time[port].min_raw == 0U)) {
                    pdu_do_route_time[port].min_raw = stat_buf->data[i + roff].time;
                    pdu_do_route_time[port].min = (double)pdu_do_route_time[port].min_raw /
                                                  SYS_CNT_FREQ;
                }

                if (pdu_do_route_time[port].max_raw < stat_buf->data[i + roff].time) {
                    pdu_do_route_time[port].max_raw = stat_buf->data[i + roff].time;
                    pdu_do_route_time[port].max = (double)pdu_do_route_time[port].max_raw /
                                                  SYS_CNT_FREQ;
                }

                break;

            default:
                break;
        }

        i = (i + 1U) % rring->size;
    }
    while (i != rring->tail);

    for (size_t i = 0; (i < PORT_MAX) && route_time[i].not_empty; i++) {
        isr_time[i].aver = ((double)isr_total_time[i] / SYS_CNT_FREQ) / isr_counter[i];
        route_time[i].aver = ((double)route_total_time[i] / SYS_CNT_FREQ) /
                             tx_pdu_counter[i];
        pdu_do_route_time[i].aver = ((double)do_route_total_time[i] / SYS_CNT_FREQ) /
                                    do_route_pdu_counter[i];
        printf("\nPort [%ld] time:\n", i);
        printf("\t Measuer time: %fus\n", (double)(end - start) / SYS_CNT_FREQ);
        printf("\t ISR     : min = %fus, max = %fus\n", isr_time[i].min,
               isr_time[i].max);
        printf("\t Get pdu : min = %fus, max = %fus\n", get_pdu_time[i].min,
               get_pdu_time[i].max);
        printf("\t PDU copy: min = %fus, max = %fus\n", pdu_copy_time[i].min,
               pdu_copy_time[i].max);
        printf("\t Route   : min = %fus, max = %fus\n", route_time[i].min,
               route_time[i].max);
        printf("\t Do route: min = %fus, max = %fus\n", pdu_do_route_time[i].min,
               pdu_do_route_time[i].max);
        printf("\t Interrupt counter = %d\n", isr_counter[i]);
        printf("\t Rx frame = %d, Tx frame = %d, lost frame = %d\n", rx_pdu_counter[i],
               tx_pdu_counter[i], lost_pdu_counter[i]);
        printf("\t ISR average time: %fus\n", isr_time[i].aver);
        printf("\t Routing average time: %fus\n", route_time[i].aver);
        printf("\t Do route average time: %fus\n", pdu_do_route_time[i].aver);
        printf("\t Routing fps: %ld\n",
               (uint64_t)tx_pdu_counter[i] * (SYS_CNT_FREQ * 1000000) / (end - start));
    }
}

static uint8_t next_ring_to_be_swapped(uint8_t current)
{
    return (current == 0U) ? 1U : 0U;
}

void stat_dump(stat_buf_t *stat_buf)
{
    if (likely(stat_buf->magic == 0x73746174U) &&
            (stat_buf->read_idx != stat_buf->write_idx)) {
        port_stat_parse(stat_buf);

        /**
         * In our scenario, the barrier here is used to avoid
         * AXI reorder on transactions with different ID and
         * make sure read ring data has been read out before
         * update the read pointer.
         */
        full_barrier();


        /**
         * Update reading buffer for the next dump.
         */
        stat_buf->read_idx = next_ring_to_be_swapped(stat_buf->read_idx);
    }
}

int main(int argc, char *argv[])
{
    void *map_base, *stat_buf_addr;
    off_t offset, pa_offset;
    size_t length;
    int fd;

    offset = SDPE_STAT_ADDR;
    length = SDPE_STAT_SIZE;

    pa_offset = offset & ~(sysconf(_SC_PAGE_SIZE) - 1);

    fd = open("/dev/mem", (O_RDWR | O_SYNC));

    if (!fd) {
        printf("/dev/mem open fail\n");
        return -ENXIO;
    }

    map_base = mmap(NULL,
                    offset + length - pa_offset,
                    (PROT_READ | PROT_WRITE),
                    MAP_SHARED, fd, pa_offset);

    if (map_base == MAP_FAILED) {
        printf("sdpe stat mmap fail\n");
        return -EFAULT;
    }

    stat_buf_addr = (char *)map_base + offset - pa_offset;

    stat_dump((stat_buf_t *)stat_buf_addr);

    munmap(map_base, offset + length - pa_offset);
    close(fd);

    return 0;
}
