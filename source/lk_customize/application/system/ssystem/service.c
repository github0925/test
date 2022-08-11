/*
 * Copyright (c) 2020 Semidrive Semiconductor, Inc.
 * All rights reserved.
 */

#include <stdlib.h>
#include <assert.h>
#include <bits.h>
#include <debug.h>
#include <stdio.h>
#include <err.h>
#include <lib/console.h>
#include <lib/bytes.h>
#include <lib/reg.h>
#include <dcf.h>
#include <image_cfg.h>

#if CONFIG_SSYSTEM_SERVER

#include <rpmsg_rtos.h>
#include "ssystem.h"

#define SSYSTEMD_STACK_SZ 4096
#define HSM_HEAD_BEGIN (R5_SEC_TCMB_BASE + 0x40)

#define HSM_MAIN_CMD  0xffff1234
#define HSM_HEAD_MAGIC  0x61706930

struct ssystem_server_info {
    bool binitialized;
    thread_t *main_thread;

    /* communication channel */
    int dcf_fd;

    /* service stuff */

};

/* This is a template of ssystem ioctl command body
 * To fill this
 */

struct ssystem_ioctl_cmd {
    u32 op   : 8;
    u32 rsvd : 24;
    union {
        /* to extend cmd body */
        u8 data[28];
    } u;

} __attribute__((__packed__));

struct ssystem_hsm_cmd {
    u32 cmd;
    u8 data[28];

} __attribute__((__packed__));

static const uint32_t hsm_share_mem_table[1][2] = {
    {SAF_SEC_MEMBASE + SAF_SEC_MEMSIZE - 0x4000, 0x4000}, /*sec to safe: address,size*/
};

struct hsm_head_table {
    uint32_t hsm_header_num;                   /* 0x61706930 ascii:'api0'*/
    void (* hsm_init)(uint32_t *, uint32_t);   /* hsm_init function address. param:sharemem table,table item size*/
    void (* hsm_main)(uint32_t, void *);       /* hsm_main function address. param:from cpu id, data*/
    uint32_t response_pack_size;               /* response package size */
    void *response_pack_addr;              /* response package address */
};

static struct hsm_head_table *hsm_head = (struct hsm_head_table *)HSM_HEAD_BEGIN;
static struct ssystem_server_info ssystem_server;
int ssystem_process_task(void *token)
{
    struct ssystem_server_info *s = token;
    char send_buf[DCF_MSG_MAX_DLEN] = {0,};
    struct ssystem_ioctl_cmd *cmd_msg = (struct ssystem_ioctl_cmd *) send_buf;
    struct ssystem_hsm_cmd *hsm_cmd_msg = (struct ssystem_hsm_cmd *) send_buf;
    size_t len = DCF_MSG_MAX_DLEN;
    u32 count = 0;
    int ret = 0;

    if (hsm_head->hsm_header_num == HSM_HEAD_MAGIC) {
        hsm_head->hsm_init((uint32_t *)hsm_share_mem_table, 1);    /*hsm_init*/
    }

    while (1) {
        ret = posix_read(s->dcf_fd, send_buf, len);
        if (ret <= 0) {
            printf("%s: read failure %d\n", __func__, ret);
            continue;
        }

        if (cmd_msg->op == COMM_MSG_CCM_ECHO) {
            printf("ssystem: command=ECHO\n");
            ret = posix_write(s->dcf_fd, send_buf, ret);
            if (ret < 0) {
                printf("%s: read failure %d\n", __func__, ret);
                continue;
            }
        }
        if (cmd_msg->op == COMM_MSG_CCM_DROP) {
            printf("ssystem: command=DROP\n");
            continue;
        }

        if (hsm_cmd_msg->cmd == HSM_MAIN_CMD) {
            if (hsm_head->hsm_header_num == HSM_HEAD_MAGIC) {
                hsm_head->hsm_main(0, (void *)(hsm_cmd_msg->data));
                memcpy((void *)(hsm_cmd_msg->data), (hsm_head->response_pack_addr), hsm_head->response_pack_size);
                ret = posix_write(s->dcf_fd, send_buf, len);
                if (ret < 0) {
                    printf("%s: hsm service write failure %d\n", __func__, ret);
                }
            }
            continue;
        }

        count++;
    }
    posix_close(s->dcf_fd);

    return 0;
}

int ssystem_send_msg(struct ssystem_ioctl_cmd *msg, lk_time_t timeMs)
{
    struct ssystem_server_info *s = &ssystem_server;
    int ret;

    if (!s->binitialized)
        return ERR_BAD_HANDLE;

    ret = posix_write(s->dcf_fd, msg, sizeof(*msg));
    if (ret < 0) {
        dprintf(0, "%s failed %d\n", __func__, ret);
    }

    return ret;
}

#endif //CONFIG_SSYSTEM_SERVER

void start_rpmsg_service(void);
void rpmsg_net_init_hook(void);

void ssystem_server_init(void)
{
    /* rpmsg core init */
    start_rpmsg_service();
    /* virtual eth interface based on rpmsg */
    rpmsg_net_init_hook();

#if CONFIG_SSYSTEM_SERVER
    struct ssystem_server_info *s = &ssystem_server;

    if (s->binitialized)
        return;

    /* rpmsg core init */

    /* daemon thread */
    s->dcf_fd = posix_open(DEV_SSYSTEM, O_RDWR);
    if (s->dcf_fd < 0) {
        printf("%s: failed", __func__);
        return;
    }

    s->main_thread = thread_create("ssystemd", ssystem_process_task, s, HIGH_PRIORITY, SSYSTEMD_STACK_SZ);
    thread_detach_and_resume(s->main_thread);

    s->binitialized = true;
#endif
}

