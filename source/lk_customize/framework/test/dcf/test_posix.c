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
#include <kernel/event.h>
#include <kernel/semaphore.h>
#include <dcf.h>

int do_posix_ls(int argc, const cmd_args *argv)
{
    dcf_file_list(0);

    return 0;
}

#define POSIX_TEST_PING_SIZE        (64)
int do_posix_ping(int argc, const cmd_args *argv)
{
    char send_buf[DCF_MSG_MAX_DLEN] = {0,};
    char rx_buf[DCF_MSG_MAX_DLEN] = {0,};
    int len = POSIX_TEST_PING_SIZE;
    lk_bigtime_t start_time, txdone_time, end_time;
    u32 count = 0;
    u32 round = 1;
    int ret = 0;
    int fd;

    if (len > DCF_MSG_MAX_DLEN)
        printf("len %d exceed MTU %d\n", len, DCF_MSG_MAX_DLEN);

    memset(send_buf, COMM_MSG_CCM_ECHO, len);
    printf("PING %s %d bytes of data\n", DEV_LOOPBACK, len);

    fd = posix_open(DEV_LOOPBACK, O_RDWR|O_NONBLOCK);
    if (fd < 0) {
        printf("%s: open failure %d\n", __func__, fd);
        return fd;
    }

    start_time = current_time_hires();
    while (round--) {
        ret = posix_write(fd, send_buf, len);
        if (ret < 0) {
            printf("%s: write failure %d\n", __func__, ret);
            break;
        }
        txdone_time = current_time_hires();

        ret = posix_read(fd, rx_buf, len);
        if (ret <= 0) {
            printf("%s: read failure\n", __func__);
            break;
        }
        count++;

        if (strncmp(rx_buf, send_buf, len)) {
            printf("%s: result unexpected %s\n", __func__, rx_buf);
            break;
        }
        rx_buf[0] = 0;

    }
    end_time = current_time_hires();

    if (ret > 0) {
        printf("%d packets transmitted, %d packets received, %d loss\n", count, count, 0);
        if (count == 1)
            printf("tx-trip avg. = %.2f us\n", (float)(txdone_time - start_time)/count);
        printf("round-trip avg. = %.2f us\n", (float)(end_time - start_time)/count);
    }
    posix_close(fd);

    return 0;
}

int do_posix_iperf(int argc, const cmd_args *argv)
{
    char send_buf[DCF_MSG_MAX_DLEN] = {0,};
    size_t len = DCF_MSG_MAX_DLEN;
    lk_bigtime_t start_time, end_time;
    u32 count = 0;
    u32 round = 100;
    int err_cnt = 0;
    int ret = 0;
    int fd;

    memset(send_buf, COMM_MSG_CCM_DROP, len);
    fd = posix_open(DEV_LOOPBACK, O_RDONLY|O_NONBLOCK);
    if (fd < 0) {
        printf("%s: failed", __func__);
        return fd;
    }

    start_time = current_time_hires();

    while (round--) {
        ret = posix_write(fd, send_buf, len);
        if (ret < 0) {
            printf("ipccping: send failed %d\n", ret);
            err_cnt++;
            break;
        }
        count++;
    }
    end_time = current_time_hires();

    if (ret > 0) {
        printf("%d packets transmitted, %d bytes, %d loss\n", count, count * len, err_cnt);
        printf("throughput avg. = %.2f MB/s\n", (float)(count * len)/(end_time - start_time));
        printf("tx latency avg. = %lld us\n", (end_time - start_time)/count);
    }

    posix_close(fd);

    return 0;
}

static thread_t* posix_echod;

int posix_echo_task(void *dev_name)
{
    char send_buf[DCF_MSG_MAX_DLEN] = {0,};
    size_t len = DCF_MSG_MAX_DLEN;
    u32 count = 0;
    int ret = 0;
    int fd;
    char *device;

    if (!dev_name)
        device = (char*)DEV_LOOPBACK;
    else
        device = dev_name;

    printf("try to open device %s\n", device);
    fd = posix_open(device, O_RDWR);
    if (fd < 0) {
        printf("%s: failed\n", __func__);
        return -1;
    }
    printf("open successfully\n");

    while (1) {
        ret = posix_read(fd, send_buf, len);
        if (ret <= 0) {
            printf("%s: read failure %d\n", __func__, ret);
            continue;
        }
        printf("read %d bytes\n", ret);

        if (strncmp("stop", send_buf, 4) == 0) {
            printf("receive stop message, quit\n", ret);
            break;
        }

        ret = posix_write(fd, send_buf, len);
        if (ret < 0) {
            printf("%s: read failure %d\n", __func__, ret);
            continue;
        }
        count++;
    }

    printf("closing device %s\n", device);
    posix_close(fd);

    if (dev_name)
        free(dev_name);

    printf("device closed\n");

    return 0;
}

int do_posix_start_echo(int argc, const cmd_args *argv)
{
    static char *dev_name;

    printf("use device %s\n", argv[1].str);
    dev_name = strdup(argv[1].str);
    posix_echod = thread_create("posixecho", posix_echo_task, dev_name, LOW_PRIORITY, DEFAULT_STACK_SIZE);
    thread_detach_and_resume(posix_echod);

    return 0;
}


int posix_select_task(void *dev_name)
{
    char send_buf[DCF_MSG_MAX_DLEN] = {0,};
    size_t len = DCF_MSG_MAX_DLEN;
    struct timeval tv;
    u32 count = 0;
    int ret = 0;
    fd_set readfd;
    int loopback;
    int cluster;
    int fd = -1;

    loopback = posix_open(DEV_LOOPBACK, O_RDWR);
    if (loopback < 0) {
        printf("%s: failed\n", __func__);
        return -1;
    }
    count++;

    cluster = posix_open(DEV_DISP_C, O_RDWR);
    if ((loopback < 0) || (cluster < 0)) {
        printf("%s: failed\n", __func__);
        return -1;
    }
    count++;

    FD_ZERO(&readfd);
    tv.tv_sec = 10;
    tv.tv_usec = 0;
    printf("%s: selecting fd %d,%d\n", __func__, loopback, cluster);

    while (1) {

        FD_SET(loopback, &readfd);
        FD_SET(cluster, &readfd);

        ret = posix_select(1 + MAX(loopback, cluster), &readfd, 0, 0, &tv);
        if (ret < 0) {
            dprintf(0, "select failed\n");
            break;
        }

        if (ret == 0) {
            dprintf(1, "select timed out\n");
            continue;
        }

        if (FD_ISSET(loopback, &readfd))
            fd = loopback;
        if (FD_ISSET(cluster, &readfd))
            fd = cluster;

        ret = posix_read(fd, send_buf, len);
        if (ret <= 0) {
            printf("%s: fd %d read err=%d\n", __func__, fd, ret);
            continue;
        }
//        printf("read %d bytes\n", ret);

        if (strncmp("stop", send_buf, 4) == 0) {
            printf("fd %d receive stop message, quit\n", fd, ret);
            printf("closing fd %d\n", fd);
            posix_close(fd);
            if (--count == 0)
                break;
        }

        ret = posix_write(fd, send_buf, len);
        if (ret < 0) {
            printf("%s: fd %d write err=%d\n", __func__, fd, ret);
            continue;
        }
    }

    printf("test exit\n");

    return 0;
}

int do_posix_start_select(int argc, const cmd_args *argv)
{
    posix_echod = thread_create("selectd", posix_select_task, NULL, LOW_PRIORITY, DEFAULT_STACK_SIZE);
    thread_detach_and_resume(posix_echod);

    return 0;
}

#if CONFIG_HAS_DEVFS
int do_posix_register(int argc, const cmd_args *argv)
{
    printf("Not support yet\n");
    return 0;
}
#endif
static void posix_test_usage(const char *cmd)
{
    cmd_args argv;

    printf("POSIX unit test toolbox\n");
    printf("%s commands:\n\n", cmd);

    printf("%s ls", cmd);
    printf("\t\t:list device files\n");

    printf("%s ping", cmd);
    printf("\t\t:ping loopback device\n");

    printf("%s iperf", cmd);
    printf("\t\t:test loopback performance\n");

    printf("%s start", cmd);
    printf("\t\t:start an echo deamon\n");

    printf("%s select", cmd);
    printf("\t\t:select test\n");

#if CONFIG_HAS_DEVFS
    printf("%s register", cmd);
    printf("\t\t:register device file\n");
#endif
}

int do_posix_cmd(int argc, const cmd_args *argv)
{
    if (argc < 2) {
        posix_test_usage(argv[0].str);
        goto out;
    }

    dcf_file_init();
    if (!strcmp(argv[1].str, "ls")) {
        do_posix_ls(argc-1, &argv[1]);
    }
    if (!strcmp(argv[1].str, "ping")) {
        do_posix_ping(argc-1, &argv[1]);
    }
    if (!strcmp(argv[1].str, "iperf")) {
        do_posix_iperf(argc-1, &argv[1]);
    }
#if CONFIG_HAS_DEVFS
    if (!strcmp(argv[1].str, "register")) {
        do_posix_register(argc-1, &argv[1]);
    }
#endif
    if (!strcmp(argv[1].str, "start")) {
        do_posix_start_echo(argc-1, &argv[1]);
    }
    if (!strcmp(argv[1].str, "select")) {
        do_posix_start_select(argc-1, &argv[1]);
    }

out:
    return 0;
}

#if defined(WITH_LIB_CONSOLE)
STATIC_COMMAND_START
STATIC_COMMAND("toybox", "Use posix API to test device file", do_posix_cmd)
STATIC_COMMAND_END(test_posix);
#endif

APP_START(test_posix)
.flags = 0
         APP_END
