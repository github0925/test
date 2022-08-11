/*
 * verifyboot.h
 *
 * Copyright (c) 2020 SemiDrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */

#ifndef _SD_VERIFYBOOT_H_
#define _SD_VERIFYBOOT_H_

#include <list.h>
#include "libavb.h"
#include "partition_parser.h"
#include "storage_device.h"

#define VBMETA_PARTITION_NAME "vbmeta"
#define BPT_SIZE           0x800

struct image_load_info {
    struct list_node node;
    addr_t addr;
    uint64_t size;
    const char *name;
};

struct public_key_blob {
    uint8_t *blob;
    uint32_t blob_len;
};

typedef struct AvbOpsUserData {
    partition_device_t  *ptdev;
    struct public_key_blob *pk_blob;
    bool pk_blob_preloaded;
    struct list_node preload_head;
} AvbOpsUserData;

AvbOps *avb_ops_new(partition_device_t  *ptdev,
                    struct public_key_blob *pk_blob);
void avb_ops_free(AvbOps *ops);
uint32_t sd_crc32(uint32_t crc, const uint8_t *buffer, uint32_t len);
bool avb_add_preload_image_info(AvbOps *ops, addr_t addr,
                                size_t size, const char *name);
struct public_key_blob *avb_get_public_key_blob_from_bpt(const uint8_t *buffer,
                                                        uint32_t len);

/* Get  footer in the parition
 * There must be a footer which is avb2.0 format in the last 64 bytes of the partition
 * */
bool avb_get_footer_from_partition(partition_device_t  *ptdev,
                                   const char *partition, AvbFooter *footer);

bool avb_get_footer_from_buffer(const uint8_t *buffer, uint32_t buf_len,
                                AvbFooter *footer);
uint32_t avb_get_image_size_from_bpt(const uint8_t *buffer, uint32_t len);

/* If out_slot_data is not null and return value is true,
 * the caller needs to free AvbSlotVerifyData.
 * */
bool verify_loaded_images(partition_device_t  *ptdev,
                         struct list_node *head, AvbSlotVerifyData **out_slot_data);

/* If out_slot_data is not null and return value is true,
 * the caller needs to free AvbSlotVerifyData.
 * */
bool verify_single_image(partition_device_t  *ptdev, uint8_t *buffer,
                      uint32_t buf_len, const char *partition, AvbSlotVerifyData **out_slot_data);

struct image_load_info *image_info_node_new(addr_t addr, size_t size,
                                            const char *name);

void image_info_node_free(struct image_load_info *img_info);

bool get_device_locked(void);

bool add_verified_image_list(struct list_node *head, void *buf, size_t size, const char *name);

void free_image_info_list(struct list_node *head);
#endif

