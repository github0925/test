/*
 * SEMIDRIVE Copyright Statement
 * Copyright (c) SEMIDRIVE. All rights reserved
 *
 * This software and all rights therein are owned by SEMIDRIVE, and are
 * protected by copyright law and other relevant laws, regulations and
 * protection. Without SEMIDRIVE's prior written consent and/or related rights,
 * please do not use this software or any potion thereof in any form or by any
 * means. You may not reproduce, modify or distribute this software except in
 * compliance with the License. Unless required by applicable law or agreed to
 * in writing, software distributed under the License is distributed on
 * an "AS IS" basis, WITHOUT WARRANTIES OF ANY KIND, either express or implied.
 *
 * You should have received a copy of the License along with this program.
 * If not, see <http://www.semidrive.com/licenses/>.
 */
#include <app.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <debug.h>
#include <platform.h>
#include <lib/console.h>
#include <arch.h>
#include <scr_hal.h>
#include <stdbool.h>
#include "storage_device.h"
#include "spi_nor_hal.h"
#include "kernel/thread.h"
#include "rpmsg_rtos.h"
#include "res.h"
#include "rstgen_hal.h"
#include "dcf.h"
#include "ipcc_device.h"
#include "lib/cksum.h"

#define USE_IPCC 1
#define UPDATE_MONITOR_PRINT_MSG 0
#define UPDATE_MONITOR_EPT     81
#define UPDATE_MONITOR         "update-monitor"
#define MAX_IPCC_BUFF_SIZE     (492)
#define OTA_START_MAGIC        0x7E7E
#define OTA_END_MAGIC          0x7B7B
#define MAX_DATA_LENGTH        448
#define MSG_HEAD_SIZE          (sizeof(update_msg_head_t))
#define MAX_SEND_LENGTH (MAX_DATA_LENGTH + MSG_HEAD_SIZE)
#define MAX_RECV_LENGTH (MAX_DATA_LENGTH + MSG_HEAD_SIZE)
#ifndef MIN
#define MIN(a,b)  ((a) > (b) ? (b) : (a))
#endif

#define GET_ALIGN_ADDR(addr, erase_size) (((addr) / (erase_size)) * (erase_size))
#define GET_REL_ADDR(addr, sect_size)    ((addr) % (sect_size))
#define UPDATE_MONITOR_DEBUG INFO
//#define UPDATE_MONITOR_DEBUG CRITICAL

typedef enum update_cmd {
    OTA_CMD_START,
    OTA_CMD_START_OK,
    OTA_CMD_START_FAIL,
    OTA_CMD_SEEK,
    OTA_CMD_SEEK_OK,
    OTA_CMD_SEEK_FAIL,
    OTA_CMD_READ,
    OTA_CMD_READ_OK,
    OTA_CMD_READ_FAIL,
    OTA_CMD_WRITE,
    OTA_CMD_WRITE_OK,
    OTA_CMD_WRITE_FAIL,
    OTA_CMD_WRITE_INFO,
    OTA_CMD_WRITE_INFO_OK,
    OTA_CMD_WRITE_INFO_FAIL,
    OTA_CMD_GET_CAPACITY,
    OTA_CMD_GET_CAPACITY_OK,
    OTA_CMD_GET_CAPACITY_FAIL,
    OTA_CMD_GET_BLOCK,
    OTA_CMD_GET_BLOCK_OK,
    OTA_CMD_GET_BLOCK_FAIL,
    OTA_CMD_GET_ERASESIZE,
    OTA_CMD_GET_ERASESIZE_OK,
    OTA_CMD_GET_ERASESIZE_FAIL,
    OTA_CMD_COPY,
    OTA_CMD_COPY_OK,
    OTA_CMD_COPY_FAIL,
    OTA_CMD_HANDOVER,
    OTA_CMD_HANDOVER_OK,
    OTA_CMD_HANDOVER_FAIL,
    OTA_CMD_CLOSE,
    OTA_CMD_CLOSE_OK,
    OTA_CMD_CLOSE_FAIL,
    OTA_CMD_MAX,
} update_cmd_enum;

struct update_msg_head {
    uint16_t flag1;  //0x7e7e
    uint16_t len;    //just for data
    uint16_t cmd;
    uint32_t resv;
    uint32_t crc;
    uint16_t flag2;  //0x7b7b
} __attribute__((packed));

typedef struct update_msg_head update_msg_head_t;

typedef struct msg_reply {
    update_cmd_enum reply_cmd;
    uint16_t     reply_data_len;
} msg_reply_t;

typedef struct update_monitor_dev_struct {
#if USE_IPCC
    struct ipcc_device     *ipcc_dev;
    struct ipcc_channel    *chan;
#else
    struct rpmsg_channel   *chan;
#endif
    struct thread          *recv_handler;
    struct storage_device  *storage;
    event_t                 new_msg_event;
    update_msg_head_t      *recv_head;
    update_msg_head_t      *send_head;
    uint8_t                 recv_msg[MAX_RECV_LENGTH];
    uint8_t                 send_msg[MAX_RECV_LENGTH];
    uint64_t                seek_addr;
    uint64_t                write_len_total;
    uint8_t                *write_buf;
    uint8_t                *copy_buf;
    uint64_t                capacity;
    uint32_t                erase_size;
    uint32_t                block_size;
    uint8_t                 write_buf_flag;
    msg_reply_t             reply;
    uint8_t                 load_storage;
} update_monitor_dev_t;


static update_monitor_dev_t update_monitor_dev;

//extern
extern int ipcc_channel_set_mtu(struct ipcc_channel *ichan, unsigned int mtu);

static struct spi_nor_cfg ospi_cfg = {
    .cs = SPI_NOR_CS0,
    .bus_clk = SPI_NOR_CLK_25MHZ,
    .octal_ddr_en = 1,
};

static uint16_t getUint16(uint8_t *data, int *offset)
{
    uint16_t value;
    value = data[(*offset)++];
    value |= data[(*offset)++] << 8;
    return value;
}

static uint32_t getUint32(uint8_t *data, int *offset)
{
    uint32_t value;
    uint32_t tmp = getUint16(data, offset);
    value = getUint16(data, offset);
    value <<= 16;
    value += tmp;
    return value;
}

static uint64_t getUint64(uint8_t *data, int *offset)
{
    uint64_t value;
    uint64_t tmp = getUint32(data, offset);
    value = getUint32(data, offset);
    value <<= 32;
    value += tmp;
    return value;
}


static void msg_printer(const void *ptr, size_t len)
{
    unsigned long address = (unsigned long)ptr;
    size_t count;
    size_t i;

    for (count = 0 ; count < len; count += 16) {
        for (i = 0; i < MIN(len - count, 16); i++) {
            dprintf(CRITICAL, "%02hhx ", *(const uint8_t *)(address + i));
        }

        dprintf(CRITICAL, "\n");
        address += 16;
    }
}


static void update_dev_calloc(update_monitor_dev_t *dev)
{
    if (dev->write_buf_flag == 0) {
        dev->write_buf = (uint8_t *)calloc(1, dev->erase_size);
    }

    dev->write_buf_flag = 1;
}

static void update_dev_free(update_monitor_dev_t *dev)
{
    if (dev->write_buf_flag != 0) {
        free(dev->write_buf);
    }

    dev->write_buf_flag = 0;
}

static int ospi1_reset(void)
{
    uint32_t ret;
    void *handle = NULL;
    ret = hal_rstgen_creat_handle(&handle, RES_MODULE_RST_SAF_OSPI1);

    if (!ret) {
        dprintf(CRITICAL, "update monitor: ospi1 creat_handle fail \n");
        return -1;
    }

    ret = hal_rstgen_init(handle);

    if (!ret) {
        dprintf(CRITICAL, "update monitor: ospi1 init fail \n");
        hal_rstgen_release_handle(handle);
        return -1;
    }

    ret = hal_rstgen_module_reset(handle, RES_MODULE_RST_SAF_OSPI1);

    if (!ret) {
        dprintf(CRITICAL, "update monitor: ospi1 reset fail \n");
        hal_rstgen_release_handle(handle);
        return -1;
    }

    hal_rstgen_release_handle(handle);
    return 0;
}

static void ospi1_handover(void)
{
    scr_handle_t handle;

    handle = hal_scr_create_handle(SCR_SAFETY__L16__apbmux2_ospi1_select);
    ASSERT(handle);
    hal_scr_set(handle, 1);
    hal_scr_delete_handle(handle);

    handle = hal_scr_create_handle(SCR_SAFETY__L16__axislvmux_ospi1_select);
    ASSERT(handle);
    hal_scr_set(handle, 1);
    hal_scr_delete_handle(handle);
}

static int ospi1_reset_and_handover(void)
{
    static bool handover_done = 0;
    int ret;

    if (0 == handover_done) {
        /* reset OSPI1 */
        ret = ospi1_reset();

        if (ret) {
            dprintf(CRITICAL, "update monitor:ospi1 reset fail \n");
            return -1;
        }

        /* Handover OSPI */
        ospi1_handover();
        handover_done = 1;
    }

    return 0;
}

static int update_monitor_free_storage(update_monitor_dev_t *dev)
{
    int ret = -1;
    ret = storage_dev_destroy(dev->storage);
    dev->load_storage = 0;
    return ret;
}

static int update_monitor_load_storage(update_monitor_dev_t *dev)
{
    if (0 == dev->load_storage) {
        /*load storage*/
        dev->storage = setup_storage_dev(OSPI, RES_OSPI_REG_OSPI1, &ospi_cfg);

        if (NULL == dev->storage) {
            dprintf(CRITICAL, "update monitor init: failed to load storage\n");
            return -1;
        }

        dev->capacity = dev->storage->get_capacity(dev->storage);
        dev->erase_size = dev->storage->get_erase_group_size(dev->storage);
        dev->block_size = dev->storage->get_block_size(dev->storage);

        if ((0 == dev->capacity) || (0 == dev->erase_size) || (0 == dev->block_size)) {
            dprintf(CRITICAL, "update monitor init: failed to load info from storage\n");
            return -1;
        }

        /*load storage OK*/
        dev->load_storage = 1;
    }

    return 0;
}

static int update_msg_sender(update_monitor_dev_t *dev)
{
    update_msg_head_t   *send_head;
    update_cmd_enum cmd = dev->reply.reply_cmd;
    uint16_t data_len = dev->reply.reply_data_len;
    int ret = -1;
    uint16_t total_len;

    if ( data_len > MAX_DATA_LENGTH) {
        dprintf(CRITICAL, "update monitor msg_sender: length error, data_len =%d\n",
                data_len);
        return -1;
    }

    total_len = data_len + MSG_HEAD_SIZE;
    send_head = dev->send_head;
    /*clean the buffer*/
    memset(dev->send_msg, 0, MSG_HEAD_SIZE);
    /*head*/
    send_head->flag1    =   OTA_START_MAGIC;
    send_head->len      =   data_len;
    send_head->cmd      =   cmd;
    send_head->crc      =   0;
    send_head->flag2    =   OTA_END_MAGIC;
    /*data is already put in the send_msg*/
    /*crc*/
    send_head->crc      =   crc32(0, dev->send_msg, total_len);

#if UPDATE_MONITOR_PRINT_MSG
    dprintf(CRITICAL, "update monitor sendmsg:\n");
    msg_printer(dev->send_msg, total_len);
#endif
    /*send msg*/
#if USE_IPCC
    ret = ipcc_channel_sendto( dev->chan, UPDATE_MONITOR_EPT, (char *)dev->send_msg,
                               total_len, 1000);
#else
    ret = rpmsg_channel_sendmsg(dev->chan, (struct dcf_message *)dev->send_msg,
                                total_len, 1000);
#endif

    if (ret) {
        dprintf(CRITICAL, "update monitor msg_sender: send data error, ret=%d\n", ret);
        return -1;
    }

    return 0;
}

static int update_msg_checker(update_monitor_dev_t *dev)
{
    uint32_t crc_val;
    update_msg_head_t *recv_head =  dev->recv_head;

    if (recv_head->len > MAX_DATA_LENGTH) {
        dprintf(CRITICAL, "uptate_monitor msg_checker:recv msg len error");
        return -1;
    }

    if (recv_head->flag1 != OTA_START_MAGIC) {
        dprintf(CRITICAL,
                "uptate_monitor msg_checker:recv msg flag1 is 0x%04x, expect 0x%04x \n",
                recv_head->flag1, OTA_START_MAGIC);
        return -1;
    }

    if (recv_head->flag2 != OTA_END_MAGIC) {
        dprintf(CRITICAL,
                "uptate_monitor msg_checker:recv msg flag2 is 0x%04x, expect 0x%04x \n",
                recv_head->flag2, OTA_END_MAGIC);
        return -1;
    }

    crc_val = recv_head->crc;
    recv_head->crc = 0;
    recv_head->crc = crc32(0, dev->recv_msg, (recv_head->len + MSG_HEAD_SIZE));

    if (crc_val != recv_head->crc) {
        dprintf(CRITICAL,
                "uptate_monitor msg_checker:recv msg crc = 0x%08x error, expect 0x%08x \n",
                crc_val, recv_head->crc);
        return -1;
    }

    return 0;
}

static inline void update_msg_handover(update_monitor_dev_t *dev)
{
#if PLATFORM_G9X || PLATFORM_D9LITE
    int ret = 0;
    ret = ospi1_reset_and_handover();

    if (ret < 0) {
        dev->reply = (msg_reply_t) {OTA_CMD_HANDOVER_FAIL, 0};
        dprintf(CRITICAL, "uptate_monitor update_msg_handover error\n");
    }
    else {
        dev->reply = (msg_reply_t) {OTA_CMD_HANDOVER_OK, 0};
        dprintf(INFO, "uptate_monitor: update_msg_handover done\n");
    }

#else
    dev->reply = (msg_reply_t) {OTA_CMD_HANDOVER_FAIL, 0};
    dprintf(INFO, "uptate_monitor: do not support ospi handover\n");
#endif
}

static inline void update_msg_close(update_monitor_dev_t *dev)
{
    int ret = -1;
    ret =  update_monitor_free_storage(dev);

    if (ret < 0) {
        dev->reply = (msg_reply_t) {OTA_CMD_CLOSE_FAIL, 0};
        dprintf(CRITICAL,
                "uptate_monitor msg_close: load_storage failed ,start error\n");
    }
    else {
        dev->reply = (msg_reply_t) {OTA_CMD_CLOSE_OK, 0};
        dprintf(INFO, "uptate_monitor msg_close: ok\n");
    }
}

static inline void update_msg_start(update_monitor_dev_t *dev)
{
    int ret = -1;
    ret =  update_monitor_load_storage(dev);

    if (ret < 0) {
        dev->reply = (msg_reply_t) {OTA_CMD_START_FAIL, 0};
        dprintf(CRITICAL, "uptate_monitor: load_storage failed ,start error\n");
    }
    else {
        dev->reply = (msg_reply_t) {OTA_CMD_START_OK, 0};
        dprintf(INFO, "uptate_monitor: start ok\n");
    }
}

static inline void update_msg_seek(update_monitor_dev_t *dev)
{
    int offset = MSG_HEAD_SIZE;
    dev->seek_addr = getUint64(dev->recv_msg, &offset);

    if ( (dev->seek_addr) > (dev->capacity)) {
        dev->reply = (msg_reply_t) {OTA_CMD_SEEK_FAIL, 0};
        dprintf(CRITICAL,
                "uptate_monitor msg_seek: seek=%lld bigger than capacity=%lld \n",
                dev->seek_addr, dev->capacity);
    }
    else {
        dev->reply = (msg_reply_t) {OTA_CMD_SEEK_OK, 0};
        dprintf(INFO, "uptate_monitor msg_seek:%lld\n", dev->seek_addr);
    }
}

static inline void update_msg_read(update_monitor_dev_t *dev)
{
    int ret = -1;
    uint16_t read_len;
    int offset = MSG_HEAD_SIZE;
    struct storage_device *storage = dev->storage;

    /*storage check*/
    if (dev->storage == NULL) {
        dev->reply = (msg_reply_t) {OTA_CMD_READ_FAIL, 0};
        dprintf(CRITICAL, "uptate_monitor msg_read: storage==NULL\n");
        return;
    }

    /*get read length*/
    read_len = getUint16(dev->recv_msg, &offset);

    if ((read_len > MAX_DATA_LENGTH) ) {
        dev->reply = (msg_reply_t) {OTA_CMD_READ_FAIL, 0};
        dprintf(CRITICAL, "uptate_monitor msg_read: length error\n");
        return;
    }

    /*read from storage*/
    ret = storage->read(storage, dev->seek_addr, (dev->send_msg + MSG_HEAD_SIZE),
                        read_len);

    if (ret) {
        dev->reply = (msg_reply_t) {OTA_CMD_READ_FAIL, 0};
        dprintf(CRITICAL, "uptate_monitor msg_read :storage read error, ret=%d\n", ret);
        return;
    }

    /*seek_addr increase*/
    dev->seek_addr = dev->seek_addr + read_len;
    dev->reply = (msg_reply_t) {OTA_CMD_READ_OK, read_len};
    return;
}

static inline void update_msg_write_info(update_monitor_dev_t *dev)
{
    int ret = -1;
    struct storage_device *storage = dev->storage;
    int offset = MSG_HEAD_SIZE;
    uint32_t align_addr = 0;

    /*storage check*/
    if (dev->storage == NULL) {
        dev->reply = (msg_reply_t) {OTA_CMD_WRITE_INFO_FAIL, 0};
        dprintf(CRITICAL, "uptate_monitor msg_write_info: storage==NULL\n");
        return;
    }

    /*get write total length*/
    dev->write_len_total = getUint64(dev->recv_msg, &offset);

    /*calloc a erase size buffer*/
    if (!dev->write_buf_flag) {
        update_dev_calloc(dev);
    }

    if (!dev->write_buf) {
        dev->reply = (msg_reply_t) {OTA_CMD_WRITE_INFO_FAIL, 0};
        dprintf(CRITICAL, "uptate_monitor msg_write_info: calloc fail\n");
        return;
    }

    align_addr = GET_ALIGN_ADDR((uint32_t)(dev->seek_addr), (dev->erase_size));
    /*read*/
    ret = storage->read(storage, align_addr, dev->write_buf, dev->erase_size);

    if (ret) {
        dev->reply = (msg_reply_t) {OTA_CMD_WRITE_INFO_FAIL, 0};
        dprintf(CRITICAL, "msg_write_info: storage read error1, ret=%d\n", ret);
        return;
    }

    dprintf(UPDATE_MONITOR_DEBUG, "uptate_monitor msg_write_info: seek_addr=%lld\n",
            dev->seek_addr);
    dprintf(UPDATE_MONITOR_DEBUG,
            "uptate_monitor msg_write_info: write_len_total=%lld\n", dev->write_len_total);
    dev->reply = (msg_reply_t) {OTA_CMD_WRITE_INFO_OK, 0};
    return;
}

/* To make sure the full use of ospi nor flash ,
write one erase size each time*/
static inline void update_msg_write(update_monitor_dev_t *dev)
{
    struct storage_device *storage = dev->storage;
    uint16_t write_len;    /*length of write data this time*/
    uint32_t rel_addr = 0; /*relative addr in one erase sector*/
    uint32_t align_addr = 0; /*addr align to erase_size*/
    uint32_t msg_part1 = 0;
    uint32_t msg_part2 = 0;
    int ret = -1;

    write_len = dev->recv_head->len;
    rel_addr =     GET_REL_ADDR((uint32_t)dev->seek_addr, (dev->erase_size));
    align_addr = GET_ALIGN_ADDR((uint32_t)(dev->seek_addr), (dev->erase_size));

    /*storage check*/
    if (dev->storage == NULL) {
        dev->reply = (msg_reply_t) {OTA_CMD_WRITE_FAIL, 0};
        dprintf(CRITICAL, "uptate_monitor msg_write: storage==NULL\n");
        update_dev_free(dev);
        return;
    }

    /*write_buf not null first*/
    if (!dev->write_buf_flag) {
        dprintf(CRITICAL, "uptate_monitor msg_write: write_buf is null\n");
        dev->reply = (msg_reply_t) {OTA_CMD_WRITE_FAIL, 0};
        return;
    }

    /*copy data from recv msg to buf*/
    if ((rel_addr + write_len) < (dev->erase_size)) {
        memcpy((dev->write_buf) + rel_addr, (dev->recv_msg) + MSG_HEAD_SIZE, write_len);
        dprintf(UPDATE_MONITOR_DEBUG, "uptate_monitor msg_write:not full\n");
    }
    else {
        msg_part1 = (dev->erase_size) - rel_addr;
        msg_part2 = write_len - msg_part1;
        memcpy((dev->write_buf) + rel_addr, (dev->recv_msg) + MSG_HEAD_SIZE, msg_part1);
        /*erase now sector*/
        ret = storage->erase(storage, align_addr, dev->erase_size);

        if (ret < 0) {
            dprintf(CRITICAL, "msg_write:storage erase addr 0x%08x fail\n", align_addr);
            dev->reply = (msg_reply_t) {OTA_CMD_WRITE_FAIL, 0};
            update_dev_free(dev);
            return;
        }

        /*write to flash*/
        ret = storage->write(storage, align_addr, dev->write_buf, dev->erase_size);

        if (ret < 0) {
            dprintf(CRITICAL, "msg_write:storage write addr 0x%08x fail\n", align_addr);
            dev->reply = (msg_reply_t) {OTA_CMD_WRITE_FAIL, 0};
            update_dev_free(dev);
            return;
        }

        /*clear buf*/
        memset(dev->write_buf, 0, dev->erase_size);

        if (msg_part2 > 0) {
            memcpy((dev->write_buf), (dev->recv_msg) + MSG_HEAD_SIZE + msg_part1,
                   msg_part2);
        }

        dprintf(UPDATE_MONITOR_DEBUG, "uptate_monitor msg_write: full\n");
    }

    /*seek_addr increase*/
    dev->seek_addr += write_len;

    /*update relative addr adn align addr */
    rel_addr = GET_REL_ADDR((uint32_t)dev->seek_addr, (dev->erase_size));
    align_addr = GET_ALIGN_ADDR((uint32_t)(dev->seek_addr), (dev->erase_size));


    /*calc the msg total length left */
    if (dev->write_len_total >= write_len)
        dev->write_len_total = dev->write_len_total - write_len;
    else {
        dprintf(CRITICAL,
                "uptate_monitor msg_write: len_total=%d left is shorter than write_len=%d, fail\n",
                (uint32_t)dev->write_len_total, write_len);
        dev->reply = (msg_reply_t) {OTA_CMD_WRITE_FAIL, 0};
        update_dev_free(dev);
        return;
    }

    dprintf(UPDATE_MONITOR_DEBUG, "ptate_monitor msg_write: this write_len=%d\n",
            write_len);
    dprintf(UPDATE_MONITOR_DEBUG,
            "ptate_monitor msg_write: left write_len_total=%lld\n", dev->write_len_total);

    /*last msg */
    if (dev->write_len_total == 0) {
        dprintf(UPDATE_MONITOR_DEBUG, "ptate_monitor msg_write: last msg\n");

        if (rel_addr != 0) {
            dprintf(INFO, "uptate_monitor msg_write: last copy\n");
            //copy the rest of data
            ret = storage->read(storage, dev->seek_addr, (dev->write_buf) + rel_addr,
                                (dev->erase_size) - rel_addr);

            if (ret) {
                dev->reply = (msg_reply_t) {OTA_CMD_WRITE_FAIL, 0};
                dprintf(CRITICAL, "uptate_monitor msg_write: storage read error2, ret=%d\n",
                        ret);
                update_dev_free(dev);
                return;
            }

            /*erase now sector*/
            ret = storage->erase(storage, align_addr, dev->erase_size);

            if (ret < 0) {
                dev->reply = (msg_reply_t) {OTA_CMD_WRITE_FAIL, 0};
                dprintf(CRITICAL, "uptate_monitor msg_write: storage erase addr 0x%08x fail\n",
                        align_addr);
                update_dev_free(dev);
                return;
            }

            /*write to flash*/
            ret = storage->write(storage, align_addr, dev->write_buf, dev->erase_size);

            if (ret < 0) {
                dev->reply = (msg_reply_t) {OTA_CMD_WRITE_FAIL, 0};
                dprintf(CRITICAL, "uptate_monitor msg_write: storage write addr 0x%08x fail\n",
                        align_addr);
                update_dev_free(dev);
                return;
            }

#if 0
            /*cached write*/
            ret = storage->cached_write(storage, align_addr, dev->write_buf, rel_addr);

            if (ret < 0) {
                dev->reply = (msg_reply_t) {OTA_CMD_WRITE_FAIL, 0};
                dprintf(CRITICAL, "uptate_monitor msg_write: storage write addr 0x%08x fail\n",
                        align_addr);
                update_dev_free(dev);
                return;
            }

#endif

        }

        update_dev_free(dev);
    }

    dev->reply = (msg_reply_t) {OTA_CMD_WRITE_OK, 0};
    return;
}

static inline void update_msg_get_capacity(update_monitor_dev_t *dev)
{
    if ((NULL == dev->storage) || (0 == dev->capacity)) {
        dev->reply = (msg_reply_t) {OTA_CMD_GET_CAPACITY_FAIL, 0};
        dprintf(CRITICAL, "uptate_monitor get_capacity fail\n");
    }
    else {
        *(uint64_t *)(dev->send_msg + MSG_HEAD_SIZE) = dev->capacity;
        dev->reply = (msg_reply_t) {OTA_CMD_GET_CAPACITY_OK, sizeof(uint64_t)};
        dprintf(INFO, "uptate_monitor get_capacity ok\n");
    }
}

static inline void update_msg_get_erasesize(update_monitor_dev_t *dev)
{
    if ((NULL == dev->storage) || (0 == dev->erase_size)) {
        dev->reply = (msg_reply_t) {OTA_CMD_GET_ERASESIZE_FAIL, 0};
        dprintf(CRITICAL, "uptate_monitor get erase_size fail\n");
    }
    else {
        *(uint32_t *)(dev->send_msg + MSG_HEAD_SIZE) = dev->erase_size;
        dev->reply = (msg_reply_t) {OTA_CMD_GET_ERASESIZE_OK, sizeof(uint32_t)};
        dprintf(INFO, "uptate_monitor get erase_size ok\n");
    }
}

static inline void update_msg_get_blocksize(update_monitor_dev_t *dev)
{
    if ((NULL == dev->storage) || (0 == dev->block_size)) {
        dev->reply = (msg_reply_t) {OTA_CMD_GET_BLOCK_FAIL, 0};
        dprintf(CRITICAL, "uptate_monitor get block_size fail\n");
    }
    else {
        *(uint32_t *)(dev->send_msg + MSG_HEAD_SIZE) = dev->block_size;
        dev->reply = (msg_reply_t) {OTA_CMD_GET_BLOCK_OK, sizeof(uint32_t)};
        dprintf(INFO, "uptate_monitor get block_size ok\n");
    }
}


static inline void update_msg_copy(update_monitor_dev_t *dev)
{
    int offset = MSG_HEAD_SIZE;
    int ret = -1;
    struct storage_device *storage = dev->storage;
    uint64_t src_cp_addr = getUint64(dev->recv_msg, &offset);
    uint64_t dst_cp_addr = getUint64(dev->recv_msg, &offset);
    uint64_t cp_cnt      = getUint64(dev->recv_msg, &offset) / (dev->erase_size);

    /*storage check*/
    if (dev->storage == NULL) {
        dev->reply = (msg_reply_t) {OTA_CMD_COPY_FAIL, 0};
        dprintf(CRITICAL, "uptate_monitor msg_copy storage==NULL\n");
        return;
    }

    dprintf(INFO, "uptate_monitor msg_copy: src_addr=%d\n", (uint32_t)src_cp_addr);
    dprintf(INFO, "uptate_monitor msg_copy: dst_addr=%d\n", (uint32_t)dst_cp_addr);
    dprintf(INFO, "uptate_monitor msg_copy: size=%d\n",
            (uint32_t)(cp_cnt * dev->erase_size));

    if ((src_cp_addr > dst_cp_addr)
            || ((src_cp_addr + cp_cnt * (dev->erase_size)) <= dst_cp_addr)) {
        update_dev_calloc(dev);

        for (uint64_t n = 0; n < cp_cnt; n++) {
            /*read*/
            ret = storage->read(storage, src_cp_addr, dev->write_buf, dev->erase_size);

            if (ret) {
                dev->reply = (msg_reply_t) {OTA_CMD_COPY_FAIL, 0};
                dprintf(CRITICAL, "uptate_monitor msg_copy: storage read error, addr 0x%08x\n",
                        (uint32_t)src_cp_addr);
                update_dev_free(dev);
                return;
            }

            /*erase*/
            ret = storage->erase(storage, dst_cp_addr, dev->erase_size);

            if (ret < 0) {
                dprintf(CRITICAL, "uptate_monitor msg_copy: storage erase addr 0x%08x fail\n",
                        (uint32_t)dst_cp_addr);
                dev->reply = (msg_reply_t) {OTA_CMD_COPY_FAIL, 0};
                update_dev_free(dev);
                return;
            }

            /*write to flash*/
            ret = storage->write(storage, dst_cp_addr, dev->write_buf, dev->erase_size);

            if (ret < 0) {
                dprintf(CRITICAL, "uptate_monitor msg_copy: 0x%08x fail\n",
                        (uint32_t)dst_cp_addr);
                dev->reply = (msg_reply_t) {OTA_CMD_COPY_FAIL, 0};
                update_dev_free(dev);
                return;
            }

            /*increase addr*/
            src_cp_addr += dev->erase_size;
            dst_cp_addr += dev->erase_size;
        }

        update_dev_free(dev);
        dev->reply = (msg_reply_t) {OTA_CMD_COPY_OK, 0};
        dprintf(INFO, "uptate_monitor msg_copy: ok\n");
    }
    else {
        dev->reply = (msg_reply_t) {OTA_CMD_COPY_FAIL, 0};
        dprintf(CRITICAL, "uptate_monitor msg_copy: para error\n");
    }
}

static int update_msg_processer(update_monitor_dev_t *dev)
{
    /*clear send data*/
    memset(dev->send_msg, 0, MAX_RECV_LENGTH);

    /*chose action from cmd*/
    switch (dev->recv_head->cmd) {
        case OTA_CMD_START :
            update_msg_start(dev);
            break;

        case OTA_CMD_SEEK :
            update_msg_seek(dev);
            break;

        case OTA_CMD_READ :
            update_msg_read(dev);
            break;

        case OTA_CMD_WRITE_INFO :
            update_msg_write_info(dev);
            break;

        case OTA_CMD_WRITE :
            update_msg_write(dev);
            break;

        case OTA_CMD_GET_CAPACITY :
            update_msg_get_capacity(dev);
            break;

        case OTA_CMD_GET_ERASESIZE :
            update_msg_get_erasesize(dev);
            break;

        case OTA_CMD_GET_BLOCK :
            update_msg_get_blocksize(dev);
            break;

        case OTA_CMD_COPY :
            update_msg_copy(dev);
            break;

        case OTA_CMD_HANDOVER :
            update_msg_handover(dev);
            break;

        case OTA_CMD_CLOSE :
            update_msg_close(dev);
            break;

        default:
            dprintf(CRITICAL, "uptate_monitor msg_processer: error, unknown cmd: %d\n",
                    dev->recv_head->cmd);
            return -1;
    }

    return 0;
}

static int update_msg_handler(void *arg)
{
    update_monitor_dev_t *dev = (update_monitor_dev_t *)arg;
    int ret = -1;

    while (true) {
        /*wait recv event happen*/
        event_wait(&dev->new_msg_event);
#if UPDATE_MONITOR_PRINT_MSG
        dprintf(CRITICAL, "uptate_monitor,recv msg:\n");
        msg_printer(dev->recv_msg, recv_head->len + MSG_HEAD_SIZE);
#endif
        /*check msg*/
        ret = update_msg_checker(dev);

        if (ret < 0) {
            dprintf(CRITICAL, "uptate_monitor msg_handler: msg format error");
            continue;
        }

        /*process msg*/
        ret = update_msg_processer(dev);

        if (ret) {
            dprintf(CRITICAL, "uptate_monitor msg_handler: msg process error\n");
            continue;
        }

        /*send reply msg*/
        ret = update_msg_sender(dev);

        if (ret) {
            dprintf(CRITICAL, "uptate_monitor msg_handler: msg send error\n");
            continue;
        }
    }

    return 0;
}
static update_monitor_dev_t *update_monitor_create(void)
{
    return &update_monitor_dev;
}


#if USE_IPCC
static void update_msg_callback(struct ipcc_channel *chan,
                                struct dcf_message *dmsg, int len, int src)
#else
static void update_msg_callback(struct rpmsg_channel *chan,
                                struct dcf_message *dmsg, int len)
#endif
{
    update_monitor_dev_t *dev = update_monitor_create();

    //int ret;
    if ((!dev->chan) || (!dmsg) || (len <= 0)) {
        dprintf(CRITICAL, "update monitor msg_callback: para error\n");
        return;
    }

    memset(dev->recv_msg, 0, sizeof(MAX_RECV_LENGTH));
    memcpy(dev->recv_msg, (char *)dmsg, MIN((unsigned int)len, MAX_RECV_LENGTH));
    event_signal(&dev->new_msg_event, true);

    return;
}


static void update_monitor_init(update_monitor_dev_t *dev)
{
#if USE_IPCC
    /*creat a ipcc dev*/
#if PLATFORM_G9X || PLATFORM_D9LITE
    dev->ipcc_dev = ipcc_device_gethandle(IPCC_RRPOC_AP2, 1000);
#else
    dev->ipcc_dev = ipcc_device_gethandle(IPCC_RRPOC_AP1, 1000);
#endif

    if (!dev->ipcc_dev) {
        dprintf(CRITICAL, "update monitor init:failed to create an ipcc device\n");
        return;
    }

    /*creat a ipcc channel*/
    dev->chan = ipcc_channel_create(dev->ipcc_dev, UPDATE_MONITOR_EPT,
                                    UPDATE_MONITOR, true);
#else
#if RPMSG_MASTER_DEVICE
    dprintf(INFO, "update monitor init:start rpmsg service ...\n");
    start_rpmsg_service();
#endif
    /*creat a ipcc channel*/
#if PLATFORM_G9X || PLATFORM_D9LITE
    dev->chan = rpmsg_channel_create(DP_CA_AP2, UPDATE_MONITOR_EPT, UPDATE_MONITOR);
#else
    dev->chan = rpmsg_channel_create(DP_CA_AP1, UPDATE_MONITOR_EPT, UPDATE_MONITOR);
#endif
#endif

    if (!dev->chan) {
        dprintf(CRITICAL, "update monitor init:failed to create an ipcc channel\n");
        return;
    }

    /*creat recv msg thread handler*/
    dev->recv_handler = thread_create("update_msg_handler", update_msg_handler,
                                      (void *)dev, HIGHEST_PRIORITY, DEFAULT_STACK_SIZE);

    if (!dev->recv_handler) {
        dprintf(CRITICAL, "update monitor init:failed to create a thread\n");
        return;
    }

    /*mark the msg head at the beginning of msg buffer*/
    dev->recv_head = (update_msg_head_t *)dev->recv_msg;
    dev->send_head = (update_msg_head_t *)dev->send_msg;
    /*init para*/
    dev->seek_addr = 0;
    dev->write_len_total = 0;
    dev->write_buf = NULL;
    dev->write_buf_flag = 0;
    dev->reply = (msg_reply_t) {0, 0};
    dev->storage = NULL;
    dev->capacity = 0;
    dev->erase_size = 0;
    dev->block_size = 0;
    dev->load_storage = 0;

    /*recv msg event*/
    event_init(&dev->new_msg_event, false, EVENT_FLAG_AUTOUNSIGNAL);
    /*start msg recv_handler*/
    thread_detach_and_resume(dev->recv_handler);
#if USE_IPCC
    /*start ipcc_channel*/
    ipcc_channel_set_mtu(dev->chan, MAX_IPCC_BUFF_SIZE);
    ipcc_channel_start(dev->chan, update_msg_callback);
#else
    /*start rpmsg_channel*/
    rpmsg_channel_set_mtu(dev->chan, MAX_IPCC_BUFF_SIZE);
    rpmsg_channel_start(dev->chan, update_msg_callback);
#endif
}

static void update_monitor_entry(const struct app_descriptor *app, void *args)
{
    thread_sleep(1000);
    update_monitor_dev_t *dev = update_monitor_create();
    update_monitor_init(dev);
}

APP_START(update_monitor)
.flags = 0,
.entry = update_monitor_entry,
APP_END
