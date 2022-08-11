/*
  SEMIDRIVE Copyright Statement
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

#ifndef SDRV_OSPI_MIGRATE_H
#define SDRV_OSPI_MIGRATE_H
#include <stdbool.h>
struct rpmsg_endpoint_info {
    char name[32];
    __u32 src;
    __u32 dst;
};

typedef enum ota_cmd {
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
} ota_cmd_enum;


struct ota_msg_head_struct {
    uint16_t flag1;
    uint16_t len;
    uint16_t cmd;
    uint32_t resv;
    uint32_t crc;
    uint16_t flag2;
} __attribute__((packed));

typedef struct ota_msg_head_struct ota_msg_head_struct_t;

typedef struct ota_op {
    ota_cmd_enum cmd;
    int fd;
} ota_op_t;

#define OTA_START_MAGIC 0x7E7E
#define OTA_END_MAGIC   0x7B7B
#define MAX_DATA_LENGTH 448
#define MSG_HEAD_SIZE (sizeof(ota_msg_head_struct_t))
#define MAX_SEND_LENGTH (MAX_DATA_LENGTH + MSG_HEAD_SIZE)
#define MAX_RECV_LENGTH (MAX_DATA_LENGTH + MSG_HEAD_SIZE)
#define OSPI_HANDOVER_MODE 0xAA55
#define OSPI_REMOTE_MODE   0x55AA




extern int remote_ospi_init(storage_device_t *storage_dev, char const *dev_name,
                            char const *rpmsg_name);
extern int remote_ospi_seek(int fd, off_t offset, int whence);
extern int remote_ospi_read(storage_device_t *storage_dev, uint64_t src,
                            uint8_t *dst, uint64_t size);
extern uint64_t remote_ospi_get_capacity(storage_device_t *storage_dev);
extern uint32_t remote_ospi_get_erase_size(storage_device_t *storage_dev);
extern uint32_t remote_ospi_get_block_size(storage_device_t *storage_dev);
extern int remote_ospi_write(storage_device_t *storage_dev, uint64_t dst,
                             const uint8_t *buf, uint64_t data_len);
extern int remote_ospi_copy(storage_device_t *storage_dev, uint64_t src,
                            uint64_t dst, uint64_t size);
extern int remote_ospi_release(storage_device_t *storage_dev);
extern bool ospi_use_handover;
#endif
