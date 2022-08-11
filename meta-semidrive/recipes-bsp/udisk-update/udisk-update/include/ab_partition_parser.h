/*
 * Copyright (c) 2017, The Linux Foundation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of The Linux Foundation nor
 *       the names of its contributors may be used to endorse or promote
 *       products derived from this software without specific prior written
 *       permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NON-INFRINGEMENT ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
#ifndef __AB_PART_H__
#define __AB_PART_H__
//#include <fastboot.h>
#include <err.h>
#include <stdio.h>
#include "partition_parser.h"

extern const char *suffix_slot[];
extern const char *suffix_delimiter;

#define SUFFIX_SLOT(part_slot) suffix_slot[(part_slot)]
#define MAX_SLOT_SUFFIX_SZ      3
#define BOOT_DEV_NAME_SIZE_MAX  10
#define MAX_RSP_SIZE            64
#define MAX_GET_VAR_NAME_SIZE   256
#define MAX_NR_SCAN_FOR_SLOT    16

typedef struct {
    char Suffix[MAX_SLOT_SUFFIX_SZ];
} Slot;
#define SET_BIT(p,n) ((p) |= ((uint64_t)0x1 << (n)))
#define CLR_BIT(p,n) ((p) &= (~(((uint64_t)0x1) << (n))))

enum {
    SLOT_A = 0,
    SLOT_B = 1,
    AB_SUPPORTED_SLOTS = 2,
    INVALID = -1
};


typedef enum part_attr {
    ATTR_UNBOOTABLE,
    ATTR_ACTIVE,
    ATTR_SUCCESSFUL,
    ATTR_RETRY,
    ATTR_NUM
} part_attr_t;

/* Structure to print get var info */
struct ab_slot_info {
    char slot_is_unbootable[MAX_GET_VAR_NAME_SIZE];
    char slot_is_unbootable_rsp[MAX_RSP_SIZE];
    char slot_is_active[MAX_GET_VAR_NAME_SIZE];
    char slot_is_active_rsp[MAX_RSP_SIZE];
    char slot_is_succesful[MAX_GET_VAR_NAME_SIZE];
    char slot_is_succesful_rsp[MAX_RSP_SIZE];
    char slot_retry_count[MAX_GET_VAR_NAME_SIZE];
    char slot_retry_count_rsp[MAX_RSP_SIZE];
};

/* A/B support API(s) */
bool ptdev_multislot_is_supported(partition_device_t
                                  *part_dev);/* Check Multislot is supported */
bool ptdev_scan_for_multislot(partition_device_t
                              *part_dev);    /* Calling to scan part. table. */
void ptdev_mark_slot_attr(partition_device_t *part_dev,
                          int slot, int attr);    /* Marking slot active */
void ptdev_clean_slot_attr(partition_device_t *part_dev,
                           int slot, int attr);
void ptdev_reset_attributes(partition_device_t *part_dev,
                            unsigned index);  /* Resetting slot attr. */
void ptdev_fill_slot_meta(partition_device_t *part_dev,
                          struct ab_slot_info *slot_info);    /* Fill slot meta infomation */
void ptdev_deactivate_slot(partition_device_t *part_dev,
                           int slot); /* Mark slot inactive and reset other attributes*/
void ptdev_activate_slot(partition_device_t *part_dev,
                         int slot);    /* Mark slot active and set other attributes*/
int ptdev_find_boot_slot(partition_device_t
                         *part_dev);     /* Find bootable partition */
int ptdev_find_active_slot(partition_device_t
                           *part_dev);   /* Find current active partition*/
int get_inverse_slot(partition_device_t *part_dev, int slot);
int ptdev_mark_part_attr(partition_device_t *part_dev,
                         unsigned slot, char *partName, int attr);
int ptdev_clean_part_attr(partition_device_t *part_dev,
                          unsigned slot, char *partName, int attr);
int ptdev_find_successfull_slot(partition_device_t *part_dev);
int ptdev_fill_partition_meta(partition_device_t *part_dev,
                              char has_slot_pname[][MAX_GET_VAR_NAME_SIZE],
                              char has_slot_reply[][MAX_RSP_SIZE],
                              int array_size);   /* Fill partition slot info meta*/

void ptdev_mark_slot_attr_noUpdate(partition_device_t *part_dev, unsigned slot,
                                   int attr);
void ptdev_clean_slot_attr_noUpdate(partition_device_t *part_dev, unsigned slot,
                                    int attr);
void ptdev_mark_slot_temp(partition_device_t *part_dev, unsigned slot,
                          int attr);
int  ptdev_attributes_update(partition_device_t *part_dev);
void ptdev_reset_all_attributes(partition_device_t *part_dev);

#endif /* __AB_PART_H__ */
