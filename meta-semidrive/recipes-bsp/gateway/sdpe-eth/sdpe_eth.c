/*
 * sdpe_test.c
 *
 * Copyright (c) 2020 SemiDrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */
/*
 * Test application that data integraty of inter processor
 * communication from linux userspace to a remote software
 * context. The application sends chunks of data to the
 * remote processor. The remote side echoes the data back
 * to application which then validates the data returned.
 */

#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <time.h>
#include <fcntl.h>
#include <string.h>
#include <linux/rpmsg.h>
#include <pthread.h>
#include <dirent.h>
#include <sys/mman.h>
#include <signal.h>
// used for UDP socket
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "../common/ring_buffer.h"

struct eth_ring {
    ring_t *udp_ring;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
};

struct eth_stat {
    uint64_t min;
    uint64_t max;
    double   aver;
    uint64_t sum_time;
    uint64_t packet_nr;
    double   throughput;
};

struct eth_msg_head {
    uint16_t msg_id;
    uint16_t msg_type;
    uint32_t cookie;
} __attribute__((packed));

static int rpmsg_fd = -1, err_cnt;
static int udp_server_fd;

static socklen_t client_addr_len;
static struct sockaddr_in udp_client_addr;

static uint8_t *eth_packet;

static struct eth_ring eth_ring;

static int mem_fd;
static uint64_t *sys_cnt_base;
static struct eth_stat g_route_stat, g_udp_stat;

//#define DEBUG 1

#define RPMSG_DEV_NAME      "/dev/sdpe-eth"

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define in_range(c, lo, up)  ((unsigned char)c >= lo && (unsigned char)c <= up)
#define isprint(c)  in_range(c, 0x20, 0x7f)

#define UDP_SERVER_PORT 8888
#define UDP_SERVER_PORT2 8889

#define ETH_BUF_TIMESTAMP_VALID_POS     0
#define ETH_BUF_TIMESTAMP_POS           4
#define ETH_BUF_TIMESTAMP_LEN           12
#define ETH_BUF_TIMESTAMP_VALID         0x73746174 /* "stat" */

#define SYS_CNT_FREQ                    3 /* MHz */

#define SDPE_MSG_HEAD                   sizeof(struct eth_msg_head)
#define SoAdSocketnPduUdpTxBufferMin    (1472 + ETH_BUF_TIMESTAMP_LEN + SDPE_MSG_HEAD)

#define UDP_RING_SIZE                   4096
#define ETH_MTU                         1500

#define SDPE_RPC_ETH1_SERVICE           3
#define SDPE_RPC_ETH2_SERVICE           4
#define ETH_SEND_FRAME_MSG_ID           1

#define SDPE_RPC_REQ                    0
#define SDPE_RPC_RSP                    1
#define SDPE_RPC_IND                    2

static void hexdump8_ex(const void *ptr, size_t len, u_int64_t disp_addr)
{
    caddr_t address = (caddr_t)ptr;
    size_t count;
    size_t i;
    const char *addr_fmt = ((disp_addr + len) > 0xFFFFFFFF)
                           ? "0x%016llx: "
                           : "0x%08llx: ";

    for (count = 0 ; count < len; count += 16) {
        printf(addr_fmt, disp_addr + count);

        for (i = 0; i < MIN(len - count, 16); i++) {
            printf("%02hhx ", *(const unsigned char *)(address + i));
        }

        for (; i < 16; i++) {
            printf("   ");
        }

        printf("|");

        for (i = 0; i < MIN(len - count, 16); i++) {
            char c = ((const char *)address)[i];
            printf("%c", isprint(c) ? c : '.');
        }

        printf("\n");
        address += 16;
    }
}

static inline void hexdump8(const void *ptr, size_t len)
{
    hexdump8_ex(ptr, len, (u_int64_t)((caddr_t)ptr));
}

static inline uint64_t get_cur_time(void)
{
    return *sys_cnt_base;
}

static int get_eth_packet_timestamp(uint8_t *packet, uint64_t *t)
{
    if (*(uint32_t *)&packet[ETH_BUF_TIMESTAMP_VALID_POS] ==
            ETH_BUF_TIMESTAMP_VALID) {
        *t = *(uint64_t *)&packet[ETH_BUF_TIMESTAMP_POS];
        return 0;
    }
    else {
        return 1;
    }
}

static void eth_route_stat(uint64_t route_s, uint64_t udp_s,
                           uint64_t udp_e, uint32_t len)
{
    uint64_t interval;

    /*=================routing latency=================*/
    interval = udp_e - route_s;

    if ((g_route_stat.min > interval) ||
            (g_route_stat.min == 0)) {
        g_route_stat.min = interval;
    }

    if (g_route_stat.max < interval) {
        g_route_stat.max = interval;
    }

    g_route_stat.sum_time += interval;
    g_route_stat.packet_nr++;
    g_route_stat.aver = g_route_stat.sum_time / g_route_stat.packet_nr;
    g_route_stat.throughput = (double)len * SYS_CNT_FREQ * 1000000 / interval;

    /*=================udp sending latency=================*/
    interval = udp_e - udp_s;

    if ((g_udp_stat.min > interval) ||
            (g_udp_stat.min == 0)) {
        g_udp_stat.min = interval;
    }

    if (g_udp_stat.max < interval) {
        g_udp_stat.max = interval;
    }

    g_udp_stat.sum_time += interval;
    g_udp_stat.aver = g_udp_stat.sum_time / g_route_stat.packet_nr;
}

static int eth_ring_init(void)
{
    eth_ring.udp_ring = ring_alloc(UDP_RING_SIZE);

    if (eth_ring.udp_ring ) {
        pthread_mutex_init(&eth_ring.mutex, NULL);
        pthread_cond_init(&eth_ring.cond, NULL);

        return 0;
    }

    return 1;
}

static void eth_ring_free(void)
{
    pthread_mutex_destroy(&eth_ring.mutex);
    pthread_cond_destroy(&eth_ring.cond);
    ring_free(eth_ring.udp_ring);
}

static void eth_packet_notify(void)
{
    pthread_cond_signal(&eth_ring.cond);
}

static void eth_packet_rx(uint8_t *buf, uint16_t len)
{
    /* Add 2 bytes header that indicates the length of
     * the next payload.
     */
    while (0 == ring_in(eth_ring.udp_ring, (uint8_t *)&len, 2));

    /* Copy packet to udp fifo. */
    if (0 == ring_in(eth_ring.udp_ring, buf, len)) {
        /*
         * Ring is full or almost full,
         * nofify udp send thread and retry inserting.
         */
        eth_packet_notify();

        while (0 == ring_in(eth_ring.udp_ring, buf, len));
    }
}

static void eth_packet_tx(void)
{
    static uint8_t buf[ETH_MTU];
    uint32_t attempt_len;
    uint32_t rd_len;

    pthread_mutex_lock(&eth_ring.mutex);
    pthread_cond_wait(&eth_ring.cond, &eth_ring.mutex);
    pthread_mutex_unlock(&eth_ring.mutex);

    while (1) {
        rd_len = ring_out(eth_ring.udp_ring, buf, 2);

        if (rd_len == 2) {
            attempt_len = *(uint16_t *)buf;

            do {
                rd_len = ring_out(eth_ring.udp_ring, buf, attempt_len);
            } while (!rd_len);
        } else if (!rd_len) {
            /* Ring empty. */
            break;
        }

        uint64_t udp_s = get_cur_time();

#if DEBUG
        printf("\n======A55 send data to PC======\n");
        hexdump8(buf, len);
#endif
        sendto(udp_server_fd, buf, rd_len, MSG_DONTROUTE | MSG_DONTWAIT,
               (struct sockaddr *)&udp_client_addr, client_addr_len);

        uint64_t udp_e = get_cur_time();
        uint64_t packet_born_time;

        if (0 == get_eth_packet_timestamp(buf, &packet_born_time)) {
            eth_route_stat(packet_born_time, udp_s, udp_e, rd_len);
        }
    }
}

static void *sdpe_recv_thread_routine(void *arg)
{
    // SDPE ------rpmsg----->[A55]--------UDP----->Eth
    int bytes_rcvd;
    bool first = true;
    int flags;

    eth_packet = (uint8_t *)malloc(SoAdSocketnPduUdpTxBufferMin);

    if (eth_packet == 0) {
        printf("ERROR: Failed to allocate memory.\n");
        return NULL;
    }

    flags = fcntl(rpmsg_fd, F_GETFL, NULL);

    while (1) {
        flags |= O_NONBLOCK;
        fcntl(rpmsg_fd, F_SETFL, flags);
        bytes_rcvd = read(rpmsg_fd, eth_packet, SoAdSocketnPduUdpTxBufferMin);

        if ((bytes_rcvd <= 0) && !first) {
            /*
             * Read all RPMsg out, notify udp send thread to send
             * packets out.
             */
            eth_packet_notify();
        }

        flags &= ~O_NONBLOCK;
        fcntl(rpmsg_fd, F_SETFL, flags);
        while (bytes_rcvd <= 0) {
            bytes_rcvd = read(rpmsg_fd, eth_packet, SoAdSocketnPduUdpTxBufferMin);
        }

#if DEBUG
        printf("\n======A55 recv data from SDPE======\n");
        hexdump8(eth_packet + SDPE_MSG_HEAD,
                 bytes_rcvd - SDPE_MSG_HEAD);
#endif
        eth_packet_rx(eth_packet + SDPE_MSG_HEAD,
                      bytes_rcvd - SDPE_MSG_HEAD);
        first = false;
    }

    return NULL;
}

static void *udp_send_thread(void *arg)
{
    while (1) {
        eth_packet_tx();
    }

    return NULL;
}

static void input_handle(int signum)
{
    /* Do nothing. Just avoid stopped by input. */
}

static void *stat_dump_thread(void *arg)
{
    signal(SIGTTIN, input_handle);

    while (1) {
        if (getchar() == 'd') {
            printf("\n##########################################\n");
            printf("\n======Eth routing latency statistics======\n");
            printf("\tmin  = %fus\n", (double)g_route_stat.min / SYS_CNT_FREQ);
            printf("\tmax  = %fus\n", (double)g_route_stat.max / SYS_CNT_FREQ);
            printf("\taver = %fus\n", (double)g_route_stat.aver / SYS_CNT_FREQ);
            printf("\tthroughput = %fB/s\n", g_route_stat.throughput);
            printf("\t====================\n");
            printf("\tudp send min  = %fus\n", (double)g_udp_stat.min / SYS_CNT_FREQ);
            printf("\tudp send max  = %fus\n", (double)g_udp_stat.max / SYS_CNT_FREQ);
            printf("\tudp send aver = %fus\n", (double)g_udp_stat.aver / SYS_CNT_FREQ);
            printf("\n##########################################\n");
        }
    }
    return NULL;
}

static void eth_fill_msg_head(char *data, int service_id)
{
    struct eth_msg_head *head = (struct eth_msg_head *)data;

    head->msg_id = (service_id << 8) + ETH_SEND_FRAME_MSG_ID;
    head->msg_type = SDPE_RPC_IND;
}

int main(int argc, char *argv[])
{
    int ret;
    err_cnt = 0;
    int opt;
    int ntimes = 1000;  // 1
    char rpmsg_dev_name[32];
    // UDP Server
    struct sockaddr_in ser_addr;
    char udp_buf[SoAdSocketnPduUdpTxBufferMin];
    int udp_recv_count;
    short udp_port = UDP_SERVER_PORT;
    int rpmsg_ept = 1024;
    int eth_id = 0;
    int service_id = SDPE_RPC_ETH1_SERVICE;
    int dump_flag = 0;

    while ((opt = getopt(argc, argv, "n:p:e:d")) != -1) {
        switch (opt) {
            case 'n':
                ntimes = atoi(optarg);
                printf("getopt n  %d\n", ntimes);
                break;

            case 'p':
                udp_port = atoi(optarg);
                printf("getopt p for UDP Port %d\n", udp_port);
                break;

            case 'e':
                rpmsg_ept = atoi(optarg);
                printf("getopt e for rpmsg endpoint %d\n", rpmsg_ept);
                break;

            case 'd':
                dump_flag = 1;
                printf("getopt d for start dump thread\n");
                break;

            default:
                printf("getopt return unsupported option: -%c\n", opt);
                break;
        }
    }

    printf("\r\n sdpe test start \r\n");

    if (rpmsg_ept == 1024) {
        eth_id = 0;
        udp_port = UDP_SERVER_PORT;
        service_id = SDPE_RPC_ETH1_SERVICE;
    }
    else {
        eth_id = 1;
        udp_port = UDP_SERVER_PORT2;
        service_id = SDPE_RPC_ETH2_SERVICE;
    }

    sprintf(rpmsg_dev_name, "%s%d", RPMSG_DEV_NAME, eth_id);

    rpmsg_fd = open(rpmsg_dev_name, O_RDWR);

    if (rpmsg_fd < 0) {
        perror("Failed to open rpmsg device.");
        return -1;
    }

    /* init UDP server */
    udp_server_fd = socket(AF_INET, SOCK_DGRAM, 0); //AF_INET:IPV4;SOCK_DGRAM:UDP

    if (udp_server_fd < 0) {
        printf("create socket fail!\n");
        return -1;
    }

    memset(&ser_addr, 0, sizeof(ser_addr));
    ser_addr.sin_family = AF_INET;
    ser_addr.sin_addr.s_addr = htonl(INADDR_ANY); // IP addr transform, INADDR_ANY

    ser_addr.sin_port = htons(udp_port);  // port number transform
    printf("UDP Port %d \n", udp_port);

    ret = bind(udp_server_fd, (struct sockaddr *)&ser_addr, sizeof(ser_addr));

    if (ret < 0) {
        printf("socket bind fail!\n");
        return -1;
    }

    mem_fd = open("/dev/mem", O_RDONLY | O_SYNC);

    if (mem_fd < 0) {
        printf("Open /dev/mem failed\n");
        return -1;
    }

    sys_cnt_base = mmap(NULL, 0x20000, PROT_READ, MAP_SHARED, mem_fd, 0x31400000);

    if (sys_cnt_base == NULL) {
        printf("MMap sys cnt failed\n");
        return -1;
    }

    if (eth_ring_init()) {
        printf("Allocate udp ring failed\n");
        return -1;
    }

    pthread_t dump_thread_id;
    if (dump_flag == 1) {
        pthread_create(&dump_thread_id, NULL, stat_dump_thread, NULL);
    }

    /* create rpmsg receive thread */
    pthread_t recv_thread_id;
    pthread_create(&recv_thread_id, NULL, sdpe_recv_thread_routine, NULL);

    pthread_t udp_send_thread_id;
    pthread_create(&udp_send_thread_id, NULL, udp_send_thread, NULL);

    /* recv UDP data and forward to SDPE by rpmsg */
    while (1) {
        // Eth-----UDP---->[A55]------rpmsg----->SDPE
        client_addr_len = sizeof(udp_client_addr);
        udp_recv_count = recvfrom(udp_server_fd, udp_buf + SDPE_MSG_HEAD,
                                  SoAdSocketnPduUdpTxBufferMin, MSG_WAITALL,
                                  (struct sockaddr *)&udp_client_addr, &client_addr_len);

        if (udp_recv_count == -1) {
#if DEBUG
            printf("\nUDP recieve data fail!\n");
#endif
            continue;
        }

#if DEBUG
        printf("\nUDP: %d bytes from UDP_C\n", udp_recv_count);
        hexdump8(udp_buf + SDPE_MSG_HEAD, udp_recv_count);
#endif

        /* send udp data to SDPE by rpmsg. */
        eth_fill_msg_head(udp_buf, service_id);
        write(rpmsg_fd, udp_buf, udp_recv_count + SDPE_MSG_HEAD);
    }

    if (dump_flag == 1) {
        pthread_join(dump_thread_id, NULL);
    }
    pthread_join(recv_thread_id, NULL);
    pthread_join(udp_send_thread_id, NULL);

    free(eth_packet);

    eth_ring_free();

    close(rpmsg_fd);

    close(udp_server_fd);

    return 0;
}
