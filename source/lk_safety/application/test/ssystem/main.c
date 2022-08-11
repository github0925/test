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

static void ssystem_test_usage(const char *cmd)
{
    cmd_args argv;

    printf("ssystem remote test toolbox\n");
    printf("%s commands:\n\n", cmd);

    printf("%s info", cmd);
    printf("\t\t:list device files\n");

    printf("%s ping", cmd);
    printf("\t\t:ping loopback device\n");
}

#define POSIX_TEST_PING_SIZE        (64)
int do_ssystem_ping(int argc, const cmd_args *argv)
{
    char send_buf[POSIX_TEST_PING_SIZE] = {0,};
    char rx_buf[POSIX_TEST_PING_SIZE] = {0,};
    int len = POSIX_TEST_PING_SIZE;
    lk_bigtime_t start_time, txdone_time, end_time;
    u32 count = 0;
    u32 round = 1;
    int ret = 0;
    int fd;

    memset(send_buf, COMM_MSG_CCM_ECHO, len);
    printf("PING %s %d bytes of data\n", DEV_SSYSTEM, len);

    fd = posix_open(DEV_SSYSTEM, O_RDWR);
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
            printf("%s: read failure %d\n", __func__, ret);
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

int do_ssystem_info(int argc, const cmd_args *argv)
{
    char send_buf[POSIX_TEST_PING_SIZE] = {0,};
    char rx_buf[POSIX_TEST_PING_SIZE] = {0,};
    int len = POSIX_TEST_PING_SIZE;
    int ret = 0;
    int fd;

    fd = posix_open(DEV_SSYSTEM, O_RDWR);
    if (fd < 0) {
        printf("%s: open failure %d\n", __func__, fd);
        return fd;
    }

    memcpy(send_buf, "getinfo", len);
    ret = posix_write(fd, send_buf, len);
    if (ret < 0) {
        printf("%s: write failure %d\n", __func__, ret);
        goto out;
    }

    ret = posix_read(fd, rx_buf, len);
    if (ret <= 0) {
        printf("%s: read failure %d\n", __func__, ret);
        goto out;
    }

    printf("read %d bytes %s \n", rx_buf, len);

out:
    posix_close(fd);

    return 0;
}

int do_ssystem_test(int argc, const cmd_args *argv)
{
    if (argc < 2) {
        ssystem_test_usage(argv[0].str);
        goto out;
    }

    dcf_file_init();

    if (!strcmp(argv[1].str, "ping")) {
        do_ssystem_ping(argc-1, &argv[1]);
    }
    if (!strcmp(argv[1].str, "info")) {
        do_ssystem_info(argc-1, &argv[1]);
    }

out:
    return 0;
}

#if defined(WITH_LIB_CONSOLE)
STATIC_COMMAND_START
STATIC_COMMAND("ssystem", "test ssystem from safety", do_ssystem_test)
STATIC_COMMAND_END(ssystem_test);
#endif

APP_START(ssystem_test)
.flags = 0
         APP_END

