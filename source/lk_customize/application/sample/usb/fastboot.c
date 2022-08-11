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
#include <kernel/thread.h>
#include <app.h>
#include "class_fastboot.h"
#include "chip_res.h"


fastboot_t fb;

char manufacturer[] = "Semidrive";
char product[] = "X9";
char serialno[] = "0123456789abcdef";
char cfg[] = "spl";

int app_fb_init(void)
{
	thread_t *thr;

	memset(&fb, 0, sizeof(fastboot_t));

	fb.priv.download_base = (void *)0xffff000050000000;
	fb.priv.download_max = 0x10000000;

	fb.usb.global_base_id = RES_USB_USB1;
	fb.usb.global_phys_id = RES_USBPHY_USBPHY1;
	fb.usb.priv.vendor_id = 0x18d1;
	fb.usb.priv.product_id = 0x4d00;
	fb.usb.priv.version_id = 0x0100;
	fb.usb.priv.manufacturer = manufacturer;
	fb.usb.priv.product = product;
	fb.usb.priv.serialno = serialno;
	fb.usb.priv.cfg = cfg;

	fb.fb_init        = fastboot_init;
	fb.fb_stop        = fastboot_stop;
	fb.cb.online      = fastboot_online;
	fb.cb.offline     = fastboot_offline;
	fb.usb_read       = fastboot_usb_read;
	fb.usb_write      = fastboot_usb_write;

	fastboot_register_cmd("getvar:", cmd_getvar);
	fastboot_register_cmd("download:", cmd_download);
	fastboot_register_cmd("flash", cmd_flash);

	fastboot_register_var("version", "0.5");
	fastboot_register_var("name", "fastboot");
	fastboot_register_var("max-download-size", "0x1000000");

	fb.fb_init(&fb);

	thr = thread_create("fastboot", fastboot_handler, (void *)&fb, DEFAULT_PRIORITY, DEFAULT_STACK_SIZE);
	thread_resume(thr);
	return 0;
}

void app_fb_stop(void)
{
	fb.fb_stop(&fb);
}

static void usb_entry(const struct app_descriptor *app, void *args)
{
	printf("usb entry\n");
	app_fb_init();
}

APP_START(usb)
.flags = 0,
.entry=usb_entry,
APP_END

