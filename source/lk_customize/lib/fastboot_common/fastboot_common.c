/*
 * Copyright (c) 2009, Google Inc.
 * All rights reserved.
 *
 * Copyright (c) 2013-2015, 2018, The Linux Foundation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <string.h>
#include <assert.h>
#include <kernel/thread.h>
#include <malloc.h>
#include "class_fastboot.h"
#include "chip_res.h"

#define SD_VENDOR_ID  0x18d1
#define SD_PRODUCT_ID 0x4d00
#define SD_VERSION_ID 0x0100
#define SD_VID_INDEX  0x10
#define SD_UUID_INDEX 0x8

#define APB_EFUSEC_BASE (0xF0010000)
#define FUSE0_OFFSET    0x1000

static const char manufacturer[] = "Semidrive";
static const char product[] = "X9";
static const char cfg_str[] = "spl";

__WEAK uint32_t fuse_read(uint32_t index)
{
    uint32_t base = APB_EFUSEC_BASE;

    return readl(base + FUSE0_OFFSET + index * 4);
}

static const char *get_serialno(void)
{
    static  char serialno[17] = "";
    uint32_t serial_fuse[2] = {0};
    uint8_t *buf = NULL;

    serial_fuse[0] = fuse_read(SD_UUID_INDEX);
    serial_fuse[1] = fuse_read(SD_UUID_INDEX + 1);

    buf = (uint8_t *)serial_fuse;
    snprintf(serialno, sizeof(serialno), "%02X%02X%02X%02X%02X%02X%02X%02X",
             buf[0],buf[1],buf[2],buf[3],buf[4],buf[5],buf[6],buf[7]);
    dprintf(ALWAYS, "%s serial:%s\n", __func__, serialno);
    return serialno;
}

static void get_vid_pid(uint16_t *vid, uint16_t *pid)
{
    uint32_t v = 0;

    v = fuse_read(SD_VID_INDEX);

    *pid = v & 0xFFFFU;
    *vid = (v >> 16) & 0xFFFFU;

    if (*pid == 0)
        *pid = SD_PRODUCT_ID;

    if (*vid == 0)
        *vid = SD_VENDOR_ID;

    dprintf(ALWAYS, "%s vid:0x%0x pid:0x%0x\n", __func__, *vid, *pid);
}

fastboot_t *fastboot_common_init(void *addr, uint32_t sz)
{
    thread_t *thr;
    fastboot_t *fb;
    uint16_t vid = 0, pid = 0;

    fb = calloc(1, sizeof(fastboot_t));
    ASSERT(fb != NULL);

    get_vid_pid(&vid, &pid);

    fb->priv.download_base = (void *)addr;
    fb->priv.download_max  = sz;

    fb->usb.global_base_id    = RES_USB_USB1;
    fb->usb.global_phys_id    = RES_USBPHY_USBPHY1;
    fb->usb.priv.vendor_id    = vid;
    fb->usb.priv.product_id   = pid;
    fb->usb.priv.version_id   = SD_VERSION_ID;
    fb->usb.priv.manufacturer = manufacturer;
    fb->usb.priv.product      = product;
    fb->usb.priv.serialno     = get_serialno();
    fb->usb.priv.cfg          = cfg_str;

    fb->fb_init    = fastboot_init;
    fb->fb_stop    = fastboot_stop;
    fb->cb.online  = fastboot_online;
    fb->cb.offline = fastboot_offline;
    fb->usb_read   = fastboot_usb_read;
    fb->usb_write  = fastboot_usb_write;

    fastboot_register_cmd("getvar:", cmd_getvar);
    fastboot_register_cmd("download:", cmd_download);

    fastboot_register_var("version", "0.5");
    fastboot_register_var("name", "fastboot");

    fb->fb_init(fb);

    thr = thread_create("fastboot", fastboot_handler, (void *)fb,
                        DEFAULT_PRIORITY, DEFAULT_STACK_SIZE);
    thread_resume(thr);
    return fb;
}

void fastboot_common_okay(fastboot_t *fb, const char *reason)
{
    if (fb) {
        fastboot_okay(fb, reason);
    }
}

void fastboot_common_fail(fastboot_t *fb, const char *reason)
{
    if (fb) {
        fastboot_fail(fb, reason);
    }
}

void fastboot_common_info(fastboot_t *fb, const char *reason)
{
    if (fb) {
        fastboot_info(fb, reason);
    }
}

void fastboot_common_stop(fastboot_t *fb)
{
    if (fb) {
        fb->fb_stop(fb);
        free(fb);
    }
}

