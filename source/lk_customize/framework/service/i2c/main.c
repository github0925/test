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
#include <app.h>
#include <thread.h>
#include <event.h>
#include "i2c_hal.h"

#ifndef SA_VI2C_EPT_NAME
/* Check DCF endpoint management document for allocation */
#define SA_VI2C_EPT_NAME             "safety,vi2c"
#endif

#ifndef SA_VI2C_EPT
#define SA_VI2C_EPT                  (73)
#endif

#define I2C_M_RD                     0x1
#define MAX_I2C_NUM                  16

//#define PRINT_I2C_MSG
#ifdef PRINT_I2C_MSG
static int print_i2c_msg = -1;
#endif

#define PAYLOAD_HEAD_LEN             sizeof(common_head_t)
#define PAYLOAD_MAX_LEN              120
#define PAYLOAD_DATA_LEN             (PAYLOAD_MAX_LEN - PAYLOAD_HEAD_LEN)

static uint8_t I2C_HEAD = 0;
static uint8_t I2C_DATA = 1;
static uint8_t I2C_END = 2;

const uint32_t i2c_res_idx[MAX_I2C_NUM] = {
    RES_I2C_I2C1,
    RES_I2C_I2C2,
    RES_I2C_I2C3,
    RES_I2C_I2C4,
    RES_I2C_I2C5,
    RES_I2C_I2C6,
    RES_I2C_I2C7,
    RES_I2C_I2C8,
    RES_I2C_I2C9,
    RES_I2C_I2C10,
    RES_I2C_I2C11,
    RES_I2C_I2C12,
    RES_I2C_I2C13,
    RES_I2C_I2C14,
    RES_I2C_I2C15,
    RES_I2C_I2C16,
};

typedef struct __common_head_t {
    uint8_t adap_num;
    uint8_t msg_type;
    uint16_t time_flag;
} __attribute__((packed)) common_head_t;

typedef struct __head_info_t {
    common_head_t common_head;
    uint16_t msg_len;
    uint16_t msg_num;
} __attribute__((packed)) head_info_t;

typedef struct __data_info_t {
    common_head_t common_head;
    char i2c_data[PAYLOAD_DATA_LEN];
} __attribute__((packed)) data_info_t;

typedef struct __end_info_t {
    common_head_t common_head;
    int8_t status;
} __attribute__((packed)) end_info_t;

typedef struct __payload_t {
    common_head_t common_head;
    char buf[PAYLOAD_DATA_LEN];
} __attribute__((packed)) payload_t;

struct r_i2c_msg_info {
    uint8_t *r_msg;
    uint8_t *r_msg_p;
    uint16_t htime_flag;
    uint16_t dtime_flag;
    uint16_t etime_flag;
    uint16_t r_len;
    uint16_t r_num;
};

struct t_i2c_msg_info {
    uint16_t time_flag;
};

struct vi2c_service_info_t {
    bool binitialized;
    event_t initial_event;
    int fd;
    payload_t *payload;
    int payload_len;
    struct r_i2c_msg_info r_info[MAX_I2C_NUM];
    struct t_i2c_msg_info t_info[MAX_I2C_NUM];
};

static struct vi2c_service_info_t vi2c_service_info;

struct i2c_msg_head {
    uint16_t addr;
    uint16_t flags;
    uint16_t len;
};

struct i2c_msg_data {
    uint8_t *buf;
};

struct vi2c_msg {
    struct i2c_msg_head head;
    struct i2c_msg_data data;
};

void switch_to_vi2c_msg(uint8_t *r_msg, struct vi2c_msg msgs[], uint16_t num)
{
    struct i2c_msg_head *msgs_h = (struct i2c_msg_head *)r_msg;

    for (int i = 0; i < num; i++) {
        msgs[i].head.addr = msgs_h->addr;
        msgs[i].head.flags = msgs_h->flags;
        msgs[i].head.len = msgs_h->len;
        msgs[i].data.buf = (uint8_t *)++msgs_h;

        if (!(msgs[i].head.flags & I2C_M_RD))
            msgs_h = (struct i2c_msg_head *)((uint8_t *)msgs_h + msgs[i].head.len);
    }
}

void pack_readback(uint8_t adap_num, uint16_t time_flag, uint8_t *t_data,
                   uint16_t t_data_len, head_info_t *head, end_info_t *end,
                   uint8_t *t_payload, int8_t status)
{
    int quotient_size = t_data_len / PAYLOAD_DATA_LEN;
    int remainder_size = t_data_len % PAYLOAD_DATA_LEN;
    data_info_t *data = (data_info_t *)t_payload;

    for (int i = 0; i < quotient_size; i++) {
        data->common_head.adap_num = adap_num;
        data->common_head.msg_type = I2C_DATA;
        data->common_head.time_flag = time_flag;
        memcpy(data->i2c_data, t_data, PAYLOAD_DATA_LEN);
        t_data += PAYLOAD_DATA_LEN;
        data++;
    }

    if (remainder_size) {
        data->common_head.adap_num = adap_num;
        data->common_head.msg_type = I2C_DATA;
        data->common_head.time_flag = time_flag;
        memcpy(data->i2c_data, t_data, remainder_size);
    }

    head->common_head.adap_num = adap_num;
    head->common_head.msg_type = I2C_HEAD;
    head->common_head.time_flag = time_flag;
    head->msg_len = t_data_len;

    end->common_head.adap_num = adap_num;
    end->common_head.msg_type = I2C_END;
    end->common_head.time_flag = time_flag;
    end->status = status;
}

void send_readback(int fd, head_info_t *head, end_info_t *end,
                   uint8_t *t_payload, uint16_t t_payload_len)
{
    int ret;
    int quotient_size = t_payload_len / PAYLOAD_MAX_LEN;
    int remainder_size = t_payload_len % PAYLOAD_MAX_LEN;
    data_info_t *data = (data_info_t *)t_payload;

    ret = posix_write(fd, head, sizeof(head_info_t));
    if (ret < 0) {
        printf("vi2c[%u] send head fail\n", head->common_head.adap_num);
    }

    for (int i = 0; i < quotient_size; i++) {
        ret = posix_write(fd, data, PAYLOAD_MAX_LEN);
        if (ret < 0) {
            printf("vi2c[%u] send data1 fail\n", data->common_head.adap_num);
            break;
        }
        data++;
    }

    if (remainder_size) {
        ret = posix_write(fd, data, remainder_size);
        if (ret < 0) {
            printf("vi2c[%u] send data2 fail\n", data->common_head.adap_num);
        }
    }

    ret = posix_write(fd, end, sizeof(end_info_t));
    if (ret < 0) {
        printf("vi2c[%u] send end fail\n", end->common_head.adap_num);
    }
}

void vi2c_message_process2(struct vi2c_service_info_t *vi2c,
                           uint8_t adap_num)
{
    void *handle = NULL;
    spin_lock_saved_state_t states;
    int ret = 0;
    uint16_t i2c_msg_num = 0;
    struct vi2c_msg *msgs = NULL;
    uint8_t *t_data = NULL;
    uint8_t *t_data_p = NULL;
    uint16_t t_data_len = 0;
    uint8_t *t_payload = NULL;
    uint16_t t_payload_len = 0;
    head_info_t head_info = {0};
    end_info_t end_info = {0};
    int8_t status = -1;

    if (!hal_i2c_creat_handle(&handle, i2c_res_idx[adap_num])) {
        printf("vi2c[%u] creat handle fail\n", adap_num);
        return;
    }

    /*
    ** analyze i2c msg data send from remote
    ** basic format from remote :
    ** struct i2c_msg {
    **  __u16 addr; //slave addr
    **  __u16 flags; //read(1) or write(0) flag
    **  __u16 len; // len of buf
    **  __u8 *buf; //buf with data to send or buf empty to receive
    ** };
    **
    ** if falgs = read no need buf from remote
    ** i2c msg data received format :
    ** [addr][flags][len][buf]...[addr][flags][len]...[addr][flags][len][buf]
    **
    ** define the same struct compatible with remote struct i2c_msg
    ** struct vi2c_msg {
    **  struct i2c_msg_head head;
    **  struct i2c_msg_data data;
    ** };
    */
    i2c_msg_num = vi2c->r_info[adap_num].r_num;
    msgs = malloc(sizeof(struct vi2c_msg) * i2c_msg_num);

    if (!msgs) {
        printf("vi2c[%u] alloc mem for msgs fail\n", adap_num);
        hal_i2c_release_handle(handle);
        return;
    }

    switch_to_vi2c_msg(vi2c->r_info[adap_num].r_msg, msgs, i2c_msg_num);

    for (int i = 0; i < i2c_msg_num; i++) {
        if (msgs[i].head.flags & I2C_M_RD) {
            t_data_len += msgs[i].head.len;
        }
    }

    if (t_data_len > 0) {
        t_data = malloc(t_data_len);
        if (!t_data) {
            printf("vi2c[%u] alloc mem for t_data fail\n", adap_num);
            free(msgs);
            hal_i2c_release_handle(handle);
            return;
        }
    }

    t_data_p = t_data;

    for (int i = 0; i < i2c_msg_num; i++) {
        if (msgs[i].head.flags & I2C_M_RD) {
            msgs[i].data.buf = t_data_p;
            t_data_p = t_data_p + msgs[i].head.len;
        }
    }

    status = hal_i2c_common_xfer(handle, (struct i2c_msg *)msgs, i2c_msg_num);
    if(status >= 0)
        status = i2c_msg_num;

    t_payload_len = t_data_len + (t_data_len / PAYLOAD_DATA_LEN + !!
                                  (t_data_len % PAYLOAD_DATA_LEN)) * PAYLOAD_HEAD_LEN;

    if (t_payload_len > 0) {
        t_payload = malloc(t_payload_len);
        if (!t_payload) {
            printf("vi2c[%u] alloc mem for t_payload fail\n", adap_num);
            free(msgs);
            free(t_data);
            hal_i2c_release_handle(handle);
            return;
        }
    }

    vi2c->t_info[adap_num].time_flag++;

    pack_readback(adap_num, vi2c->t_info[adap_num].time_flag, t_data, t_data_len,
                  &head_info, &end_info, t_payload, status);

    send_readback(vi2c->fd, &head_info, &end_info, t_payload, t_payload_len);

#ifdef PRINT_I2C_MSG

    if (print_i2c_msg == adap_num) {
        for (int i = 0; i < vi2c->r_info[adap_num].r_len; i++) {
            printf("vi2c[%u] receive data[%d]=%#x\n",
                   adap_num, i, vi2c->r_info[adap_num].r_msg[i]);
        }

        for (int i = 0; i < t_payload_len; i++) {
            printf("vi2c[%u] payload data[%d]=%#x\n",
                   adap_num, i, t_payload[i]);
        }
    }

#endif
    free(msgs);
    free(t_data);
    free(t_payload);
    hal_i2c_release_handle(handle);
}

int vi2c_message_process(struct vi2c_service_info_t *vi2c)
{
    /*
    ** receive rpmsg payload format
    **
    ** head payload : [adap_num /1byte] [msg_type /1byte] [time_flag /2bytes] [msg_len /2bytes] [msg_num /2bytes]
    ** data payload : [adap_num /1byte] [msg_type /1byte] [time_flag /2bytes] [msg /nbytes]
    ** end payload : [adap_num /1byte] [msg_type /1byte] [time_flag /2bytes]
    **
    ** adap_num : the physical i2c adapter num
    ** msg_type : head or data or end flag
    ** time_flag : a time flag check if head/msg/end payload is the same time send from remote
    ** msg_len : the valid i2c_msg len
    ** msg_num : the valid i2c_msg num
    ** msg : the i2c_msg data
    **
    */
    uint8_t adap_num = vi2c->payload->common_head.adap_num;
    uint8_t msg_type = vi2c->payload->common_head.msg_type;
    uint16_t time_flag = vi2c->payload->common_head.time_flag;

    if (msg_type == I2C_HEAD) {
        vi2c->r_info[adap_num].htime_flag = time_flag;
        head_info_t *r_head = (head_info_t *)(vi2c->payload);

        if (vi2c->r_info[adap_num].r_msg) {
            free(vi2c->r_info[adap_num].r_msg);
            vi2c->r_info[adap_num].r_msg = NULL;
            vi2c->r_info[adap_num].r_msg_p = NULL;
        }

        vi2c->r_info[adap_num].r_len = r_head->msg_len;
        vi2c->r_info[adap_num].r_num = r_head->msg_num;
        vi2c->r_info[adap_num].r_msg = malloc(vi2c->r_info[adap_num].r_len);
        vi2c->r_info[adap_num].r_msg_p = vi2c->r_info[adap_num].r_msg;
    }
    else if (msg_type == I2C_DATA) {
        vi2c->r_info[adap_num].dtime_flag = time_flag;
        data_info_t *r_data = (data_info_t *)(vi2c->payload);

        if (vi2c->r_info[adap_num].r_msg
                && (vi2c->r_info[adap_num].htime_flag == vi2c->r_info[adap_num].dtime_flag)) {
            memcpy(vi2c->r_info[adap_num].r_msg_p, r_data->i2c_data,
                   vi2c->payload_len - PAYLOAD_HEAD_LEN);
            vi2c->r_info[adap_num].r_msg_p =
                vi2c->r_info[adap_num].r_msg_p + vi2c->payload_len - PAYLOAD_HEAD_LEN;
        }
    }
    else if (msg_type == I2C_END) {
        vi2c->r_info[adap_num].etime_flag = time_flag;
        end_info_t *r_end = (end_info_t *)(vi2c->payload);
        uint16_t len = vi2c->r_info[adap_num].r_msg_p - vi2c->r_info[adap_num].r_msg;

        if (vi2c->r_info[adap_num].r_msg
                && (len == vi2c->r_info[adap_num].r_len)
                && (vi2c->r_info[adap_num].htime_flag == vi2c->r_info[adap_num].dtime_flag)
                && (vi2c->r_info[adap_num].htime_flag == vi2c->r_info[adap_num].etime_flag)) {
            vi2c_message_process2(vi2c, adap_num);
        }
        else {
            printf("vi2c[%u] err:r_msg=%p, len=%u, r_len=%u, htime=%u, dtime=%u, etime=%u\n",
                   adap_num, vi2c->r_info[adap_num].r_msg, len, vi2c->r_info[adap_num].r_len,
                   vi2c->r_info[adap_num].htime_flag, vi2c->r_info[adap_num].dtime_flag,
                   vi2c->r_info[adap_num].etime_flag);
        }

        if (vi2c->r_info[adap_num].r_msg) {
            free(vi2c->r_info[adap_num].r_msg);
            vi2c->r_info[adap_num].r_msg = NULL;
            vi2c->r_info[adap_num].r_msg_p = NULL;
        }
    }

    return 0;
}

static int vi2c_main_task(void *token)
{
    struct vi2c_service_info_t *vi2c = token;

    vi2c->payload = malloc(sizeof(payload_t));
    if (!vi2c->payload) {
        printf("vi2c alloc mem for payload fail\n");
        return -1;
    }

    vi2c->fd = posix_open(DEV_SA_VI2C, O_RDWR);
    if (vi2c->fd < 0) {
        printf("vi2c device open fail\n");
        return -1;
    }

    event_signal(&vi2c->initial_event, false);

    while (1) {
        int recved = 0;

        memset(vi2c->payload, 0, PAYLOAD_MAX_LEN);
        vi2c->payload_len = posix_read(vi2c->fd, vi2c->payload, PAYLOAD_MAX_LEN);
        if (vi2c->payload_len <= 0) {
            printf("vi2c read file fail\n");
            continue;
        }

        vi2c_message_process(vi2c);
    }

    free(vi2c->payload);
    posix_close(vi2c->fd);
    return 0;
}

void virtual_i2c_init(void)
{
    struct vi2c_service_info_t *vi2c = &vi2c_service_info;
    thread_t *mthread;

    if (vi2c->binitialized)
        return;

    event_init(&vi2c->initial_event, false, EVENT_FLAG_AUTOUNSIGNAL);
    mthread = thread_create("vi2cd", vi2c_main_task, vi2c, THREAD_PRI_VI2C, DEFAULT_STACK_SIZE);
    thread_detach_and_resume(mthread);
    event_wait(&vi2c->initial_event);
    vi2c->binitialized = true;
}

static void vi2c_entry(uint level)
{
    virtual_i2c_init();
}

LK_INIT_HOOK(vi2c_service, vi2c_entry, LK_INIT_LEVEL_PLATFORM + 1);

#ifdef PRINT_I2C_MSG
void vi2c_enable_msg_entry(int argc, const cmd_args *argv)
{
    print_i2c_msg = atoi(argv[1].str);
    printf("vi2c print_i2c_msg=%d\n", print_i2c_msg);
}

#if defined(WITH_LIB_CONSOLE)
STATIC_COMMAND_START STATIC_COMMAND("vi2c", "enable print i2c msg",
                                    (console_cmd)&vi2c_enable_msg_entry)
STATIC_COMMAND_END(vi2c);
#endif
#endif

