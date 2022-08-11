/*
 * Copyright (c) 2018  Semidrive
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
#include <lib/console.h>
#include <mbox_hal.h>
#include <kernel/event.h>

/* the MB address is mbox built-in for test */
#define DEFAULT_ECHO_MB_ADDR    (0x4)
#define DEFAULT_TEST_MB_ADDR    (0x5)

/* sending message wait for tx done timeout */
#define DEFAULT_TIMEOUT_MS      (100)

#ifdef DOMAIN_PROCESSOR
/* project should define this macro */
#define DEFAULT_RPROC       DOMAIN_PROCESSOR
#else
#define DEFAULT_RPROC       IPCC_RRPOC_AP1
#endif

inline static
hal_mb_chan_t* mb_test_request_channel(hal_mb_client_t cl, bool low_latency,
                                      hal_mb_rx_cb cb, hal_mb_proc_t remote)
{
    return hal_mb_request_channel_with_addr(
            cl, low_latency, cb, remote, DEFAULT_TEST_MB_ADDR);
}

inline static
hal_mb_chan_t* mb_test_request_echo_channel(hal_mb_client_t cl, bool low_latency,
                                    hal_mb_rx_cb cb, hal_mb_proc_t remote)
{
  return hal_mb_request_channel_with_addr(
          cl, low_latency, cb, remote, DEFAULT_ECHO_MB_ADDR);
}

#define MBT_PAYLOAD_SIZE        (64)

static u32 rx_msg_buf[HAL_MB_MTU/sizeof(u32)];
static event_t new_msg_event;
static lk_bigtime_t start_time, txdone, end_time;


void mbox_init(void);

static void receive_new_msg(hal_mb_client_t cl, void *mssg, u16 len)
{
    memcpy((void*)rx_msg_buf, mssg, len);

    event_signal(&new_msg_event, true);
//    printf("recieved mssg %s len: %d\n", mssg, len);
}

static status_t wait_for_new_msg(lk_time_t timeout)
{
    return event_wait_timeout(&new_msg_event, timeout);
}

static void receive_once(hal_mb_client_t cl, void *mssg, u16 len)
{
    printf("receive_once mssg %s len: %d\n", mssg, len);
    return;
}

static int create_remote_channel(int rproc, hal_mb_rx_cb cb)
{
    hal_mb_client_t cl;
    hal_mb_chan_t *mchan;
    int target = DEFAULT_RPROC;

    cl = hal_mb_get_client_with_addr(DEFAULT_TEST_MB_ADDR);
    if (!cl) {
        printf("get cl failed %d failed\n");
        return -1;
    }

    mchan = mb_test_request_channel(cl, false, cb, target);
    if (!mchan) {
        printf("request channel failed\n");
        goto fail1;
    }

    return 0;

fail1:
    hal_mb_put_client(cl);

    return -1;
}

static int do_mb_ipi(int argc, const cmd_args *argv)
{
    hal_mb_client_t cl;
    hal_mb_chan_t *mchan;
    status_t ret;
    int target = DEFAULT_RPROC;
    u32 count = 0;
    u32 round = 1;

    mbox_init();

    if (argc == 2) {
        target = argv[1].i;
    }

    if (argc >= 3) {
        target = argv[1].i;
        round = argv[2].u;
    }

    cl = hal_mb_get_client_with_addr(DEFAULT_TEST_MB_ADDR);
    if (!cl) {
        printf("get cl failed failed\n");
        return -1;
    }

    mchan = mb_test_request_channel(cl, true, NULL, target);
    if (!mchan) {
        printf("request channel failed\n");
        goto fail1;
    }

    start_time = current_time_hires();

    while (round--) {
        ret = hal_mb_send_data(mchan, 0, 0, DEFAULT_TIMEOUT_MS);
        if (ret != NO_ERROR) {
            printf("send_data failed %d\n", ret);
            goto fail2;
        }
        count++;
    }

    end_time = current_time_hires();
    printf("IPI TX avg = %.2f us round %d\n", (float)(end_time - start_time)/count, count);

fail2:
    hal_mb_free_channel(mchan);
fail1:
    hal_mb_put_client(cl);

    return 0;
}

static int do_mb_echo_test(int argc, const cmd_args *argv)
{
    hal_mb_client_t cl;
    hal_mb_chan_t *mchan;
    u8 msg_buf[MBT_PAYLOAD_SIZE];
    status_t ret;
    int target = DEFAULT_RPROC;
    int i;
    u32 count = 1;

    mbox_init();

    if (argc == 2) {
        target = argv[1].i;
    }

    if (argc >= 3) {
        target = argv[1].i;
        count = argv[2].u;
    }

    printf("PING R:%d %d(%d) bytes of data\n", target, MBT_PAYLOAD_SIZE, MBT_PAYLOAD_SIZE + 4);

    memset(msg_buf, 0xA5, MBT_PAYLOAD_SIZE);

    while (count--) {

        cl = hal_mb_get_client_with_addr(DEFAULT_TEST_MB_ADDR);
        if (!cl) {
            printf("get cl failed %d failed\n");
            break;
        }

        mchan = mb_test_request_channel(cl, false, NULL, target);
        if (!mchan) {
            hal_mb_put_client(cl);
            printf("request channel failed\n");
            break;
        }

        for (i = 1; i < HAL_MB_MTU;i++) {
            ret = hal_mb_send_data(mchan, msg_buf, i, DEFAULT_TIMEOUT_MS);
            if (ret != NO_ERROR) {
                hal_mb_free_channel(mchan);
                hal_mb_put_client(cl);
                printf("send_data failed %d\n", ret);
                break;
            }
        }

        hal_mb_free_channel(mchan);
        hal_mb_put_client(cl);
    }

    return 0;
}

static int do_mb_stats(int argc, const cmd_args *argv)
{
    printf("MB stats:\n");
    printf("\tMTU:%d Total:%d\n", HAL_MB_MTU, HAL_MB_MTU + 4);
    printf("\tTX packets:%d error:%d overruns:%d\n", 0, 0, 0);
    printf("\tRX packets:%d error:%d overruns:%d\n", 0, 0, 0);
    printf("\tRX bytes:%d Tx bytes:%d\n", 0, 0);

    return 0;
}

static int do_mb_ping(int argc, const cmd_args *argv)
{
    hal_mb_client_t cl;
    hal_mb_chan_t *mchan;
    u32 msg_buf[HAL_MB_MTU/sizeof(u32)];
    status_t ret;
    int target = DEFAULT_RPROC;
    u32 count = 0;
    u32 round = 1;
    int len = MBT_PAYLOAD_SIZE;

    mbox_init();

    if (argc == 2) {
        target = argv[1].i;
    }

    if (argc == 3) {
        target = argv[1].i;
        round = argv[2].u;
    }

    if (argc == 4) {
        target = argv[1].i;
        round = argv[2].u;
        len = argv[3].i;
    }

    cl = hal_mb_get_client_with_addr(DEFAULT_TEST_MB_ADDR);
    if (!cl) {
        printf("get cl failed %d failed\n");
        return -1;
    }

    printf("PING rproc:%d %d(%d) bytes of data\n", target, len, len + 4);
    mchan = mb_test_request_echo_channel(cl, false, receive_new_msg, target);
    if (!mchan) {
        printf("request channel failed\n");
        goto fail1;
    }

    event_init(&new_msg_event, false, EVENT_FLAG_AUTOUNSIGNAL);

    memset((void*)msg_buf, 0xA5, len);
    start_time = current_time_hires();

    while (round--) {
        ret = hal_mb_send_data(mchan, (u8*)msg_buf, len, DEFAULT_TIMEOUT_MS);
        if (ret != NO_ERROR) {
            printf("send_data failed %d\n", ret);
            goto fail2;
        }

        txdone = current_time_hires();

        ret = wait_for_new_msg(1000);
        if (ret < 0) {
            printf("wait for echo timeout\n");
            goto fail2;
        }
        if (memcmp(msg_buf, rx_msg_buf, len)) {
            printf("wait for echo timeout\n");
            goto fail2;
        }
        count++;
        rx_msg_buf[0] = 0;
    }

    end_time = current_time_hires();
    printf("tx-trip avg %lld us in %d times\n", (txdone - start_time)/count, count);
    printf("round-trip avg %lld us in %d times\n", (end_time - start_time)/count, count);

fail2:
    hal_mb_free_channel(mchan);
fail1:
    hal_mb_put_client(cl);

    return 0;
}

struct peer_load_msg {
    uint8_t tag;
    uint8_t size;
    uint8_t command;
    uint8_t version;
    uint32_t parameter;
};

static int do_mb_peer_load(int argc, const cmd_args *argv)
{
    hal_mb_client_t cl;
    hal_mb_chan_t *mchan;
    status_t ret;
    struct peer_load_msg msg;
    int target = DEFAULT_RPROC;

    mbox_init();

    if (argc == 2) {
        target = argv[1].i;
    }

    msg.tag = 0xfc;
    msg.size = 8;
    msg.command = 0xf1;
    msg.version = 1;
    msg.parameter = 0xcdcdcdcd;

    cl = hal_mb_get_client_with_addr(DEFAULT_TEST_MB_ADDR);
    if (!cl) {
        printf("get cl failed %d failed\n");
        return -1;
    }

    mchan = mb_test_request_channel(cl, true, receive_new_msg, target);
    if (!mchan) {
        printf("request channel failed\n");
        goto fail1;
    }

    event_init(&new_msg_event, false, 0);
    start_time = current_time_hires();

    ret = hal_mb_send_data_rom(mchan, (u8*)&msg, 8);
    if (ret != NO_ERROR) {
        printf("test peer load failed %d\n", ret);
        goto fail2;
    }

    wait_for_new_msg(1);

    end_time = current_time_hires();
    struct peer_load_msg *recv_msg =
        (struct peer_load_msg *) rx_msg_buf;

    printf("peer load cmd %x %x, takes %d us\n",
                recv_msg->command, recv_msg->tag,
                (long)(end_time - start_time));

fail2:
    hal_mb_free_channel(mchan);
fail1:
    hal_mb_put_client(cl);

    return 0;
}
#if 0
int do_mb_server(int argc, const cmd_args *argv)
{
    hal_mb_client_t cl;
    hal_mb_chan_t *mchan;
    u8 msg_buf[16];
    status_t ret;
    unsigned long round = 0;
    int target = DEFAULT_RPROC;

    if (argc == 2) {
        target = argv[1].i;
    }

    event_init(&new_msg_event, false, 0);

    cl = hal_mb_get_client_with_addr(DEFAULT_TEST_MB_ADDR);
    if (!cl) {
        printf("get cl failed %d failed\n");
        return -1;
    }

    mchan = mb_test_request_channel(cl, false, receive_new_msg, target);
    if (!mchan) {
        printf("request channel failed\n");
        goto fail1;
    }

    while (1) {
    ret = wait_for_new_msg(1000);
        if (ret != NO_ERROR) {
            continue;
        }

        if (0 == strncmp("exit", rx_msg_buf, 4)) {
            break;
        }

        printf("%s: new msg received, reply ok!\n", __func__);
        round++;
        strcpy((char *)msg_buf, "ok!");
        msg_buf[3] = '\0';
        ret = hal_mb_send_data(mchan, msg_buf, 4, DEFAULT_TIMEOUT_MS);
        if (ret != NO_ERROR) {
            printf("send ok failed %d\n", ret);
            continue;
        }
    }

    printf("going to quit, round %ld\n", round);

    event_destroy(&new_msg_event);
fail2:
    hal_mb_free_channel(mchan);
fail1:
    hal_mb_put_client(cl);

    return 0;
}
#endif
static void mbt_usage(const char *cmd)
{
    printf("Usage: here's a mailbox debug and unit test toolbox\n");
    printf("%s commands:\n\n", cmd);

    printf("%s ipi <rproc>", cmd);
    printf("\t\t:send a ipi interrupt with low latency\n");

    printf("%s echo_test <rproc>", cmd);
    printf("\t:send different size of payload\n");

    printf("%s ping <rproc>", cmd);
    printf("\t:ping a remote processor and wait for echo\n");
    printf("    ping <rproc> <times>\n");
    printf("    ping <rproc> <times> <len>\n");

    printf("%s pl <rproc>", cmd);
    printf("\t\t:send a peer load message\n");

    printf("%s stats", cmd);
    printf("\t\t:show h/w statistics\n");
}

static int do_mb_cmd(int argc, const cmd_args *argv)
{
    if (argc < 2) {
        mbt_usage(argv[0].str);
        goto out;
    }

    if (!strcmp(argv[1].str, "ping")) {
        do_mb_ping(argc-1, &argv[1]);
    }
    if (!strcmp(argv[1].str, "ipi")) {
        do_mb_ipi(argc-1, &argv[1]);
    }
    if (!strcmp(argv[1].str, "echo_test")) {
        do_mb_echo_test(argc-1, &argv[1]);
    }
    if (!strcmp(argv[1].str, "pl")) {
        do_mb_peer_load(argc-1, &argv[1]);
    }
    if (!strcmp(argv[1].str, "stats")) {
        do_mb_stats(argc-1, &argv[1]);
    }

out:
    return 0;
}

#if defined(WITH_LIB_CONSOLE)
STATIC_COMMAND_START
STATIC_COMMAND("mbt", "mailbox toolbox and unit test", (console_cmd)&do_mb_cmd)
STATIC_COMMAND_END(mb_test);
#endif

APP_START(mb_test)
.flags = 0
APP_END

