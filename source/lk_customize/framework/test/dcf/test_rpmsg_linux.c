/*
 * Copyright (c) 2020  Semidrive
 *
 */

#include <app.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <err.h>
#include <debug.h>
#include <platform.h>
#include <platform/debug.h>
#include <kernel/semaphore.h>
#include <lib/console.h>
#include <lib/reg.h>
#include <kernel/event.h>
#include <rpmsg_rtos.h>
#include <dcf.h>

#define RPMSG_ECHO_EPT          (IPCC_ECHO_EPT)
#define RPMSG_TEST_EPT          (18)
#define RPMSG_MTU_BYTES         (RL_BUFFER_PAYLOAD_SIZE)
#define RPMSG_PING_PAYLOAD_SIZE (64)
#define RPMSG_TEST_EPT_NAME_    "rpmsg-test"
#define RPMSG_TEST_PING_TIMEOUT (2000)
int do_rpmsg_open_close(int argc, const cmd_args *argv)
{
    int ept_addr = RPMSG_TEST_EPT;
    int rproc = DP_CA_AP1;
    struct rpmsg_channel *chan;

    if (argc == 2) {
        ept_addr = argv[1].i;
    }

    if (argc == 3) {
        ept_addr = argv[1].i;
        rproc = argv[2].i;
    }

    chan = rpmsg_channel_create(rproc, ept_addr, RPMSG_TEST_EPT_NAME_);
    if (!chan) {
        printf("open channel failed\n");
        return -1;
    }

    rpmsg_channel_start(chan, NULL);

    thread_sleep(1000);

    rpmsg_channel_stop(chan);
    rpmsg_channel_destroy(chan);

    return 0;
}

static int rpmsg_echo_test_loop(struct rpmsg_channel *chan)
{
    int recved = 0;
    unsigned long src;
    char txbuf[RPMSG_MTU_BYTES];
    char rxbuf[RPMSG_MTU_BYTES];
    int err_cnt = 0;
    int count = 1;

    memset(txbuf, COMM_MSG_CCM_ECHO, RPMSG_MTU_BYTES);

    while (count < RPMSG_MTU_BYTES) {
        rpmsg_channel_sendto(chan, RPMSG_ECHO_EPT, txbuf, count, RL_BLOCK);

        memset(rxbuf, 0, RPMSG_MTU_BYTES);
        rpmsg_channel_recvfrom(chan, &src,
                        (char*)rxbuf, RPMSG_MTU_BYTES, &recved, RL_BLOCK);
        if (recved == 0 && (src == RL_ADDR_ANY)) {
            dprintf(INFO, "\n\nFrom received stop msg\n");
            break;
        }
        if ((recved != count) || memcmp(txbuf, rxbuf, recved)) {
            printf("\n\n ERROR Data corruption");
            err_cnt++;
            break;
        }
        count++;
    }

    printf("\r\n **********************************");
    printf("****\r\n");
    printf("\r\n Echo Test Round %d Test Results: Error count = %d\r\n",
            count, err_cnt);
    printf("\r\n **********************************");
    printf("****\r\n");

    return 0;
}

static void start_rpmsg_echo_test(int rproc, int addr)
{
    struct rpmsg_channel *chan;

    chan = rpmsg_channel_create(rproc, addr, RPMSG_TEST_EPT_NAME_);
    if (!chan) {
        printf("open channel failed\n");
        return;
    }

    rpmsg_channel_start(chan, NULL);

    rpmsg_echo_test_loop(chan);

    rpmsg_channel_stop(chan);
    rpmsg_channel_destroy(chan);

}

int do_rpmsg_echo_test(int argc, const cmd_args *argv)
{
    int ept_addr = RPMSG_TEST_EPT;
    int rproc = -1;

    if (argc == 1) {
        printf("usage:\n");
        printf("rpmsg_echo_test [rproc], use default endpoint %d\n", ept_addr);
        printf("rpmsg_echo_test [rproc] [ept_addr]\n");
        return -1;
    }
    if (argc == 2) {
        rproc = argv[1].i;
    }

    if (argc == 3) {
        rproc = argv[1].i;
        ept_addr = argv[2].i;
    }

    start_rpmsg_echo_test(rproc, ept_addr);

    return 0;
}

int do_rpmsg_listdev(int argc, const cmd_args *argv)
{
    show_rpmsg_service();
    return 0;
}

int rpmsg_ping_once(struct rpmsg_channel *chan, int payload_size)
{
    int recved = 0;
    unsigned long max_size = 0;
    unsigned long src;
    char txbuf[RPMSG_MTU_BYTES];
    char *txbufp;
    char rxbuf[RPMSG_MTU_BYTES];
    lk_bigtime_t start_time, txdone_time, end_time;
    int k, ret;

    if (payload_size > RPMSG_MTU_BYTES) {
        printf("len %d exceed MTU %d, trim\n", payload_size, RPMSG_MTU_BYTES);
        payload_size = RPMSG_MTU_BYTES;
    }

    memset(txbuf, COMM_MSG_CCM_ECHO, RPMSG_MTU_BYTES);
    memset(rxbuf, 0, RPMSG_MTU_BYTES);

    dprintf(2, "Sending %d bytes to ept %d:%d:\n", payload_size, chan->rproc, RPMSG_ECHO_EPT);

    printf("PING rproc%d %d(%d) bytes of data\n", chan->rproc, payload_size, payload_size + 20);

    start_time = current_time_hires();
    rpmsg_channel_sendto(chan, RPMSG_ECHO_EPT, txbuf, payload_size, 15000);
    txdone_time = current_time_hires();

    ret = rpmsg_channel_recvfrom(chan, &src,
                    (char*)rxbuf, payload_size, &recved, RPMSG_TEST_PING_TIMEOUT);
    if (recved == 0 && (src == RL_ADDR_ANY)) {
        dprintf(INFO, "\n\nFrom received stop msg\n");
        return ERR_CANCELLED;
    }
    if (ret < 0) {
        dprintf(0, "\n\nPing timeout\n");
        return ret;
    }

    end_time = current_time_hires();

    dprintf(2, "From ept %d:%d received %d bytes\n", chan->rproc, (int)src, recved);
    if ((recved != payload_size) || memcmp(txbuf, rxbuf, recved)) {
        printf("\n\n ERROR recved %d != payload_size %d\n", recved, payload_size);
        return -2;
    }
    for (k = 0; k < recved; k++) {
        if (rxbuf[k] != COMM_MSG_CCM_ECHO) {
            printf("\n\n ERROR Data corruption at index %d \r\n", k);
            return ERR_PARTIAL_WRITE;
        }
    }

    printf("tx-trip avg. = %lld us\n", txdone_time - start_time);
    printf("round-trip avg. = %lld us\n", end_time - start_time);

    return 0;
}

void start_rpmsg_ping(int rproc, int test_round, int size)
{
    struct rpmsg_channel *chan;
    int payload_size = size;
    int err_cnt = 0;
    int ret = 0;
    int count = 0;

    chan = rpmsg_channel_create(rproc, RPMSG_TEST_EPT, RPMSG_TEST_EPT_NAME_);
    if (!chan) {
        printf("open channel failed\n");
        return;
    }

    rpmsg_channel_start(chan, NULL);

    while(test_round--) {
        count++;
        ret = rpmsg_ping_once(chan, payload_size);
        if (ret < 0) {
            err_cnt++;
        }
    }

    dprintf(0, "%d packets transmitted, %d received, %d loss\n", count, count - err_cnt, err_cnt);

    rpmsg_channel_stop(chan);
    rpmsg_channel_destroy(chan);
}

int do_rpmsg_ping(int argc, const cmd_args *argv)
{
    int rproc = -1;
    int test_round = 1;
    int size = RPMSG_PING_PAYLOAD_SIZE;

    if (argc == 1) {
        printf("usage:\n");
        printf("rpmsg ping [rproc], test once\n");
        printf("rpmsg ping [rproc] [times]\n");
        printf("rpmsg ping [rproc] [times] [paysize]\n");
        return -1;
    }

    if (argc == 2) {
        rproc = argv[1].i;
    }

    if (argc == 3) {
        rproc = argv[1].i;
        test_round = argv[2].i;
    }

    if (argc == 4) {
        rproc = argv[1].i;
        test_round = argv[2].i;
        size = argv[3].i;
    }

    start_rpmsg_ping(rproc, test_round, size);

    return 0;
}

int do_rpmsg_iperf(int argc, const cmd_args *argv)
{
    struct rpmsg_channel *chan;
    int err_cnt = 0;
    int ret = 0;
    char send_buf[RPMSG_MTU_BYTES] = {0,};
    size_t len = RPMSG_PING_PAYLOAD_SIZE;
    unsigned long remote = 0;
    lk_bigtime_t start_time, txdone_time, end_time;
    u32 count = 0;
    u32 round = 100;

    if (argc == 0)
        return -1;

    if (argc == 1) {
        remote = 1;
    } else {
        remote = argv[1].u;

        if (argc == 3) {
            round = argv[2].u;
        }

        if (argc == 4) {
            round = argv[2].u;
            len = argv[3].u;
        }
    }

    if (len > RPMSG_MTU_BYTES) {
        printf("len %d exceed MTU %d, trim\n", len, RPMSG_MTU_BYTES);
        len = RPMSG_MTU_BYTES;
    }

    chan = rpmsg_channel_create(remote, RPMSG_TEST_EPT, RPMSG_TEST_EPT_NAME_);
    if (!chan) {
        printf("open channel failed\n");
        return ERR_NO_RESOURCES;
    }

    rpmsg_channel_start(chan, NULL);

    memset(send_buf, COMM_MSG_CCM_DROP, len);
    start_time = current_time_hires();
    while (round--) {
        ret = rpmsg_channel_sendto(chan, RPMSG_ECHO_EPT, send_buf, len, 15000);
        if (ret < 0) {
            err_cnt++;
            printf("iperf: send failed %d\n", ret);
        }
        count++;
    }
    end_time = current_time_hires();

    printf("%d packets transmitted, %d bytes, %d loss\n", count, count * len, err_cnt);
    printf("throughput avg. = %.2f MB/s\n", (float)count * len/(end_time - start_time));
    printf("tx latency avg. = %lld us\n", (end_time - start_time)/count);

    rpmsg_channel_stop(chan);
    rpmsg_channel_destroy(chan);

    return 0;
}

int do_start_rpmsg_service(int argc, const cmd_args *argv)
{
    struct rpmsg_dev_config cfg = {0};

    if (argc == 1) {
        start_rpmsg_service();
        return 0;
    }

    if (argc > 2) {
        cfg.remote_proc = argv[1].i;
        cfg.shm_phys_base = argv[2].u;
        cfg.shm_size = argv[3].u;
        rpmsg_device_probe(&cfg);
    }

    return 0;
}

void reset_rpmsg_service(void);

int do_reset_rpmsg_service(int argc, const cmd_args *argv)
{
    reset_rpmsg_service();

    return 0;
}

static void rpmsg_tool_usage(const char *cmd)
{
    printf("Usage: here's a rpmsg virtio debug and unit test toolbox (default is AP1)\n");
    printf("%s sub-commands:\n\n", cmd);

    printf("%s ping", cmd);
    printf("\t\t:ping rpmsg channel with remote processor\n");
    printf("\t\t\t:ping  <rproc>\n");
    printf("\t\t\t:ping  <rproc> <times>\n");
    printf("\t\t\t:ping  <rproc> <times> <paysize>\n");

    printf("%s iperf", cmd);
    printf("\t\t:benchmark throughput like iperf\n");
    printf("\t\t\t:iperf <rproc>\n");
    printf("\t\t\t:iperf <rproc> <times>\n");
    printf("\t\t\t:iperf <rproc> <times> <len>\n");

    printf("%s start", cmd);
    printf("\t\t:Start a rpmsg service, should be called before open any channel\n");
    printf("\t\t\t:start                     => with default settings\n");
    printf("\t\t\t:start <rproc> <pa> <size> => with specified settings\n");

    printf("%s reset", cmd);
    printf("\t\t:Reset rpmsg service after AP watchdog event\n");
    printf("\t\t\t:reset                     => with default settings\n");

    printf("%s open", cmd);
    printf("\t\t:open then close a rpmsg channel to remote processor\n");
    printf("\t\t\t:open <addr>\n");
    printf("\t\t\t:open <rproc> <addr>\n");

    printf("%s list", cmd);
    printf("\t\t:list all existing rpmsg channel\n");

    printf("%s payload", cmd);
    printf("\t\t:payload rpmsg channel with remote processor\n");
    printf("\t\t\t:payload <rproc>\n");
    printf("\t\t\t:payload <rproc> <addr>\n");

}

int do_rpmsg_cmd(int argc, const cmd_args *argv)
{
    if (argc < 2) {
        rpmsg_tool_usage(argv[0].str);
        goto out;
    }
    if (!strcmp(argv[1].str, "start")) {
        do_start_rpmsg_service(argc-1, &argv[1]);
    }
    if (!strcmp(argv[1].str, "reset")) {
        do_reset_rpmsg_service(argc-1, &argv[1]);
    }
    if (!strcmp(argv[1].str, "open")) {
        do_rpmsg_open_close(argc-1, &argv[1]);
    }
    if (!strcmp(argv[1].str, "list")) {
        do_rpmsg_listdev(argc-1, &argv[1]);
    }
    if (!strcmp(argv[1].str, "ping")) {
        do_rpmsg_ping(argc-1, &argv[1]);
    }
    if (!strcmp(argv[1].str, "echo_test")) {
        do_rpmsg_echo_test(argc-1, &argv[1]);
    }
    if (!strcmp(argv[1].str, "iperf")) {
        do_rpmsg_iperf(argc-1, &argv[1]);
    }

out:
    return 0;
}

STATIC_COMMAND_START
STATIC_COMMAND("rpmsg", "rpmsg toolbox and test", do_rpmsg_cmd)
STATIC_COMMAND_END(test_rpmsg_linux);

