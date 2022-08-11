/*
* sms_monitor.c
*
* Copyright (c) 2020 Semidrive Semiconductor.
* All rights reserved.
*
* -----------------
*/
#include <app.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <debug.h>
#include <platform.h>
#include <lib/console.h>
#include <arch.h>
#include <dcf.h>
#include "res.h"
#include <ipcc_device.h>
#include "sms_monitor.h"
#include "sms_storage.h"
#include <assert.h>
#include <bits.h>
#include <err.h>
#include <kernel/thread.h>
#include <kernel/event.h>
#include <lk/init.h>
#include <lib/console.h>
#include <lib/bytes.h>
#include <lib/reg.h>
#include <app.h>

#include <clkgen_hal.h>
#include <rstgen_hal.h>
#include <scr_hal.h>
#include <target_port.h>
#include "hal_port.h"
#include "pll_hal.h"
#include <sdrpc.h>
#include "dcf.h"
#include "ckgen_cfg.h"
#include <common/arm/platform.h>


int ipcc_channel_set_mtu(struct ipcc_channel *ichan, unsigned int mtu);

static char flash_succes[] =  "flash_success";
static char flash_failed[] =  "flash_failed";
static char recive_failed[] =  "revice_failed";

uint8_t *buffer;
uint8_t check = 0x00;
uint32_t count = 0;
uint32_t bin_len = 0;
uint32_t offset = 0;

static void createCheckSum(uint8_t *data, int len)
{
    if (data && len > 0) {
        for (int i = 0; i < len; i++) {
            check = check ^ data[i];
        }
    }
}

static int sms_send_msg(char *buf, int len)
{
    int slen = 0;
    int pos = 0;
    int ret;

    while (pos < len) {
        if (MAX_IPCC_BUFF_SIZE + pos < len)
            slen = MAX_IPCC_BUFF_SIZE;
        else
            slen = len - pos;

        ret = ipcc_channel_sendto(sms_chan, SMS_MONITORP_EPT, (buf + pos), slen, 500);

        if (ret < 0)
            return ret;

        pos += slen;
    }

    return ret;
}


static int sms_monitor_task(void *token)
{
    init_avm_storage();
    flash_avm_partition((uint8_t *)token, bin_len + 4);
    sms_send_msg(flash_succes, 12);
    deinit_avm_storage();
    free(buffer);
    return 0;
}

static void decode(uint8_t *msg, int len)
{
    if (msg && len > 3) {
        if (msg[0] == 0xaa && msg[1] == 0x55) {
            int cmd = msg[2];

            if (cmd == SMS_CMD_START) {
                //recive file info
                bin_len = (msg[6] & 0xFF)
                          | ((msg[5] << 8) & 0xFF00)
                          | ((msg[4] << 16) & 0xFF0000)
                          | ((msg[3] << 24) & 0xFF000000);

                if (bin_len > 0 && bin_len < MAX_AVM_SIZE) {
                    dprintf(INFO, "recive file info bin_len %d start recive\n", bin_len);
                    buffer = malloc(bin_len + 4);
                    memset(buffer, 0, bin_len + 4);
                    memcpy(buffer, msg + 3, 4);
                    offset = 4;
                }
                else {
                    dprintf(INFO, "recive file len error\n");
                    return;
                }
            }
            else if (cmd == SMS_CMD_PROCESSING) {
                //receive data
                if (offset + len - 3 <= bin_len + 4) {
                    memcpy(buffer + offset, msg + 3, len - 3);
                    offset += (len - 3);
                }
                else {
                    sms_send_msg(recive_failed, 13);
                    dprintf(CRITICAL, "recive data error\n");
                }
            }
            else if (cmd == SMS_CMD_END) {
                //recive check
                //createCheckSum(buffer,bin_len);
                dprintf(INFO, "recive check 0x%x recive_check 0x%x stop\n", check, msg[3]);
                /*creat recv msg thread handler*/
                thread_t *thread = thread_create("sms", sms_monitor_task, buffer,
                                                 THREAD_PRI_SAMPLE, SAMPLE_SERVICE_STACK_SIZE);

                if (!thread) {
                    dprintf(CRITICAL, "sms monitor:failed to create a thread\n");
                    return;
                }

                thread_detach_and_resume(thread);
            }
        }
        else {
            sms_send_msg(flash_failed, 12);
            dprintf(CRITICAL, "decode data error\n");
        }
    }
}


static void sms_event_handle(struct ipcc_channel *chan, struct dcf_message *msg,
                             int len, int src)
{
    if (chan)
        decode((uint8_t *)msg, len);
}

static void monitor_entry(const struct app_descriptor *app, void *args)
{
    dprintf(INFO, "monitor_entry ####################################\n");
    struct ipcc_device *sms_dev = NULL;
    sms_dev = ipcc_device_gethandle(IPCC_RRPOC_AP1, 1000);

    if (!sms_dev)
        return;

    sms_chan = ipcc_channel_create(sms_dev, SMS_MONITORP_EPT, SMS_MONITOR_NAME,
                                   true);

    if (sms_chan) {
        ipcc_channel_set_mtu(sms_chan, 492);
        ipcc_channel_start(sms_chan, sms_event_handle);
    }
    else {
        dprintf(CRITICAL, "failed to create rpmsg channel\n");
    }
}

APP_START(sms_monitor)
.flags = 0,
.stack_size = 2048,
.entry = monitor_entry,
APP_END
