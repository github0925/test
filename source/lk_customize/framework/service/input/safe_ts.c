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
#include <sys/types.h>
#include <lib/bytes.h>
#include <lib/reg.h>
#include <dcf.h>
#include <app.h>
#include <thread.h>
#include <event.h>
#include <lk/init.h>

#include "safe_ts.h"
#include "indev_local.h"
#include "avm_touch.h"

bool avm_enable_flag = false;
mutex_t safe_ts_lock;
static bool binitialized;
static struct safe_ts_device *safe_ts_dev[TS_DO_MAX];
static struct xy_range_info xy_range[TS_DO_MAX] = {
#if SERDES_TP_X9H
#if !TOUCH_SERDES_DIVIDED
    {1920, 720, 0, 0},
    {1920, 720, 0, 0},
    {0, 0, 0, 0},
    {0, 0, 0, 0},
    {1920, 720, 0, 0},
    {0, 0, 0, 0},
#else
#ifdef S3SIN1_1920X1080
    {1280, 960, 640, 0},
    {0, 0, 0, 0},
    {0, 0, 0, 0},
    {0, 0, 0, 0},
    {1920, 120, 0, 960},
    {0, 0, 0, 0},
#elif S3IN1
    {1280, 600, 640, 0},
    {0, 0, 0, 0},
    {0, 0, 0, 0},
    {0, 0, 0, 0},
    {1920, 120, 0, 600},
    {0, 0, 0, 0},
#endif
#endif
#elif SERDES_TP_X9M
#if !TOUCH_SERDES_DIVIDED
    {1920, 720, 0, 0},
    {0, 0, 0, 0},
    {0, 0, 0, 0},
    {0, 0, 0, 0},
    {1920, 720, 0, 0},
    {0, 0, 0, 0},
#else
    {0, 0, 0, 0},
    {0, 0, 0, 0},
    {0, 0, 0, 0},
    {0, 0, 0, 0},
    {0, 0, 0, 0},
    {0, 0, 0, 0},
#endif
#elif SERDES_TP_X9U_B
    {1920, 720, 0, 0},
    {1920, 720, 0, 0},
    {1920, 720, 0, 0},
    {1920, 720, 0, 0},
    {1920, 720, 0, 0},
    {0, 0, 0, 0},
#elif SERDES_TP_X9U_A
    {1920, 720, 0, 0},
    {1920, 720, 0, 0},
    {0, 0, 0, 0},
    {0, 0, 0, 0},
    {1920, 720, 0, 0},
    {0, 0, 0, 0},
#endif
};

static int safe_ts_get_remote_domain(u16 instance)
{
    enum ts_domain_type ts_domain;

    if (instance & TS_SUPPORT_ANRDOID_MAIN)
        ts_domain = TS_DO_ANRDOID_MAIN;
    else if (instance & TS_SUPPORT_ANRDOID_AUX1)
        ts_domain = TS_DO_ANRDOID_AUX1;
    else if (instance & TS_SUPPORT_ANRDOID_AUX2)
        ts_domain = TS_DO_ANRDOID_AUX2;
    else if (instance & TS_SUPPORT_ANRDOID_AUX3)
        ts_domain = TS_DO_ANRDOID_AUX3;
    else if (instance & TS_SUPPORT_CTRLPANEL_MAIN)
        ts_domain = TS_DO_CTRLPANEL_MAIN;
    else if (instance & TS_SUPPORT_CTRLPANEL_AUX1)
        ts_domain = TS_DO_CTRLPANEL_AUX1;
    else
        ts_domain = TS_DO_MAX;

    return ts_domain;
}

static struct dcf_notifier *safe_ts_add_notifier(u32 rproc, u32 mbox_addr)
{
    struct dcf_notifier_attribute attr;
    dcf_notify_attr_init(&attr, rproc, mbox_addr);
    return dcf_create_notifier(&attr);
}

static rpc_call_result_t safe_ts_ioctl_cb(rpc_server_handle_t hserver,
        rpc_call_request_t *request)
{
    rpc_call_result_t result = {0,};
    struct sts_ioctl_cmd *ctl = (struct sts_ioctl_cmd *) &request->param[0];
    struct sts_ioctl_result *r = (struct sts_ioctl_result *) &result.result[0];
    struct safe_ts_device *dev;
    int ts_remote_domain;
    result.ack = request->cmd;

    if (!request) {
        result.retcode = ERR_INVALID_ARGS;
        dprintf(ALWAYS, "%s:instance=%#x, request err\n", __func__, ctl->instance);
        return result;
    }

    ts_remote_domain = safe_ts_get_remote_domain(ctl->instance);

    if (ts_remote_domain >= TS_DO_MAX) {
        result.retcode = ERR_INVALID_ARGS;
        dprintf(ALWAYS, "%s:instance=%#x, instance err\n", __func__, ctl->instance);
        return result;
    }

    dev = safe_ts_dev[ts_remote_domain];

    if (!dev) {
        result.retcode = ERR_NOT_IMPLEMENTED;
        dprintf(ALWAYS, "%s:instance=%#x, dev err\n", __func__, ctl->instance);
        return result;
    }

    switch (ctl->op) {
        case STS_OP_GET_VERSION:
            r->msg.v.version = dev->vinfo.version;
            r->msg.v.id = dev->vinfo.id;
            r->msg.v.vendor = dev->vinfo.vendor;
            result.retcode = 0;
            break;

        case STS_OP_GET_CONFIG:
            r->msg.cg.abs_x_max = xy_range[ts_remote_domain].x_max;
            r->msg.cg.abs_y_max = xy_range[ts_remote_domain].y_max;
            r->msg.cg.x_offset = xy_range[ts_remote_domain].x_offset;
            r->msg.cg.y_offset = xy_range[ts_remote_domain].y_offset;
            r->msg.cg.swapped_x_y = dev->cinfo.swapped_x_y;
            r->msg.cg.inverted_x = dev->cinfo.inverted_x;
            r->msg.cg.inverted_y = dev->cinfo.inverted_y;
            r->msg.cg.max_touch_num = dev->cinfo.max_touch_num;
            result.retcode = 0;
            break;

        case STS_OP_SET_INITED:
            if (!dev->set_inited) {
                result.retcode = ERR_NOT_IMPLEMENTED;
                dprintf(ALWAYS, "%s:instance=%#x, set inited none\n", __func__, ctl->instance);
            }
            else {
                mutex_acquire(&dev->dev_lock);

                if (!dev->inited_flag) {
                    dev->set_inited(dev->vendor_priv);
                    dev->inited_flag = true;
                    result.retcode = 0;
                }
                else {
                    result.retcode = 0;
                }

                mutex_release(&dev->dev_lock);
            }

            break;

        default:
            result.retcode = ERR_INVALID_ARGS;
            dprintf(ALWAYS, "%s:instance=%#x, op err\n", __func__, ctl->instance);
            break;
    }

    dprintf(ALWAYS, "%s:instance=%#x, cmd=%d done\n", __func__, ctl->instance, ctl->op);
    return result;
}

static void safe_ts_init(void)
{
    rpc_server_impl_t devfuncs[] = {
        {MOD_RPC_REQ_STS_IOCTL, safe_ts_ioctl_cb, IPCC_RPC_NO_FLAGS},
    };

    if (binitialized)
        return;

    start_ipcc_rpc_service(devfuncs, ARRAY_SIZE(devfuncs));
    mutex_init(&safe_ts_lock);

    indev_local_init(
            xy_range[TS_DO_CTRLPANEL_MAIN].x_offset,
            xy_range[TS_DO_CTRLPANEL_MAIN].y_offset,
            xy_range[TS_DO_CTRLPANEL_MAIN].x_max,
            xy_range[TS_DO_CTRLPANEL_MAIN].y_max);
    binitialized = true;
}

static void safe_ts_entry(uint level)
{
    safe_ts_init();
}

LK_INIT_HOOK(safe_ts, safe_ts_entry, LK_INIT_LEVEL_PLATFORM);

struct safe_ts_device *safe_ts_alloc_device(void)
{
    struct safe_ts_device *dev = NULL;

    dev = malloc(sizeof(struct safe_ts_device));
    if (!dev)
        return dev;

    memset(dev, 0, sizeof(struct safe_ts_device));
    return dev;
}

void safe_ts_delete_device(struct safe_ts_device *dev)
{
    if (dev)
        free(dev);
}

int safe_ts_register_device(struct safe_ts_device *dev)
{
    if (!dev)
        return -1;

    mutex_acquire(&safe_ts_lock);

    if (dev->instance & TS_SUPPORT_ANRDOID_MAIN) {
        if (safe_ts_dev[TS_DO_ANRDOID_MAIN]) {
            dprintf(ALWAYS, "%s: ts android main already registered\n", __func__);
            return -1;
        }

        dev->notifier[TS_DO_ANRDOID_MAIN] =
            safe_ts_add_notifier(DP_CA_AP1, TS_ANRDOID_MAIN_MBOX_ADDR);

        if (!dev->notifier[TS_DO_ANRDOID_MAIN]) {
            dprintf(ALWAYS, "%s: ts android main add notifier fail\n", __func__);
            return -1;
        }

        safe_ts_dev[TS_DO_ANRDOID_MAIN] = dev;
    }

    if (dev->instance & TS_SUPPORT_ANRDOID_AUX1) {
        if (safe_ts_dev[TS_DO_ANRDOID_AUX1]) {
            dprintf(ALWAYS, "%s: ts android aux1 already registered\n", __func__);
            return -1;
        }

        dev->notifier[TS_DO_ANRDOID_AUX1] =
            safe_ts_add_notifier(DP_CA_AP1, TS_ANRDOID_AUX1_MBOX_ADDR);

        if (!dev->notifier[TS_DO_ANRDOID_AUX1]) {
            dprintf(ALWAYS, "%s: ts android aux1 add notifier fail\n", __func__);
            return -1;
        }

        safe_ts_dev[TS_DO_ANRDOID_AUX1] = dev;
    }

    if (dev->instance & TS_SUPPORT_ANRDOID_AUX2) {
        if (safe_ts_dev[TS_DO_ANRDOID_AUX2]) {
            dprintf(ALWAYS, "%s: ts android aux2 already registered\n", __func__);
            return -1;
        }

        dev->notifier[TS_DO_ANRDOID_AUX2] =
            safe_ts_add_notifier(DP_CA_AP1, TS_ANRDOID_AUX2_MBOX_ADDR);

        if (!dev->notifier[TS_DO_ANRDOID_AUX2]) {
            dprintf(ALWAYS, "%s: ts android aux2 add notifier fail\n", __func__);
            return -1;
        }

        safe_ts_dev[TS_DO_ANRDOID_AUX2] = dev;
    }

    if (dev->instance & TS_SUPPORT_ANRDOID_AUX3) {
        if (safe_ts_dev[TS_DO_ANRDOID_AUX3]) {
            dprintf(ALWAYS, "%s: ts android aux3 already registered\n", __func__);
            return -1;
        }

        dev->notifier[TS_DO_ANRDOID_AUX3] =
            safe_ts_add_notifier(DP_CA_AP1, TS_ANRDOID_AUX3_MBOX_ADDR);

        if (!dev->notifier[TS_DO_ANRDOID_AUX3]) {
            dprintf(ALWAYS, "%s: ts android aux3 add notifier fail\n", __func__);
            return -1;
        }

        safe_ts_dev[TS_DO_ANRDOID_AUX3] = dev;
    }

    if (dev->instance & TS_SUPPORT_CTRLPANEL_MAIN) {
        if (safe_ts_dev[TS_DO_CTRLPANEL_MAIN]) {
            dprintf(ALWAYS, "%s: ts controlpanel main already registered\n", __func__);
            return -1;
        }

        safe_ts_dev[TS_DO_CTRLPANEL_MAIN] = dev;
    }

    mutex_init(&dev->dev_lock);
    mutex_release(&safe_ts_lock);
    return 0;
}

void safe_ts_report_data(struct safe_ts_device *dev, void *data, u16 len)
{
    if (!dev)
        return;

    if (dev->instance & TS_SUPPORT_ANRDOID_MAIN) {
        if (dev->notifier[TS_DO_ANRDOID_MAIN]) {
            if (avm_enable_flag) {
                indev_local_process_touch_report(data, len, dev->screen_id);
            }
            else if (dev->inited_flag) {
                dcf_do_notify(dev->notifier[TS_DO_ANRDOID_MAIN], data, len);
            }
        }
    }

    if (dev->instance & TS_SUPPORT_ANRDOID_AUX1) {
        if (dev->notifier[TS_DO_ANRDOID_AUX1] && dev->inited_flag)
            dcf_do_notify(dev->notifier[TS_DO_ANRDOID_AUX1], data, len);
    }

    if (dev->instance & TS_SUPPORT_ANRDOID_AUX2) {
        if (dev->notifier[TS_DO_ANRDOID_AUX2] && dev->inited_flag)
            dcf_do_notify(dev->notifier[TS_DO_ANRDOID_AUX2], data, len);
    }

    if (dev->instance & TS_SUPPORT_ANRDOID_AUX3) {
        if (dev->notifier[TS_DO_ANRDOID_AUX3] && dev->inited_flag)
            dcf_do_notify(dev->notifier[TS_DO_ANRDOID_AUX3], data, len);
    }

    if (dev->instance & TS_SUPPORT_CTRLPANEL_MAIN) {
        indev_local_process_touch_report(data, len, dev->screen_id);
    }
}

#if defined(WITH_LIB_CONSOLE)
#include <lib/console.h>
static int do_touch_listdevice(void)
{
    for (int i = 0; i < TS_DO_MAX; i++) {
        if (safe_ts_dev[i]) {
            printf("name:%s, id:%u, version:%u, vendor:%u, domain=%#x\n",
                   safe_ts_dev[i]->vinfo.name,
                   safe_ts_dev[i]->vinfo.id,
                   safe_ts_dev[i]->vinfo.version,
                   safe_ts_dev[i]->vinfo.vendor,
                   safe_ts_dev[i]->instance);
        }
    }

    return 0;
}

static int do_touch_send(const char *domain, const char *xp,
                         const char *yp, const char *xplus)
{
    struct safe_ts_device *dev = NULL;
    struct touch_report_data report_data;

    if (!strcmp(domain, "anmain") && safe_ts_dev[TS_DO_ANRDOID_MAIN])
        dev = safe_ts_dev[TS_DO_ANRDOID_MAIN];
    else if (!strcmp(domain, "anaux1") && safe_ts_dev[TS_DO_ANRDOID_AUX1])
        dev = safe_ts_dev[TS_DO_ANRDOID_AUX1];
    else if (!strcmp(domain, "anaux2") && safe_ts_dev[TS_DO_ANRDOID_AUX2])
        dev = safe_ts_dev[TS_DO_ANRDOID_AUX2];
    else if (!strcmp(domain, "anaux3") && safe_ts_dev[TS_DO_ANRDOID_AUX3])
        dev = safe_ts_dev[TS_DO_ANRDOID_AUX3];
    else if (!strcmp(domain, "cpmain") && safe_ts_dev[TS_DO_CTRLPANEL_MAIN])
        dev = safe_ts_dev[TS_DO_CTRLPANEL_MAIN];
    else if (!strcmp(domain, "cpaux1") && safe_ts_dev[TS_DO_CTRLPANEL_AUX1])
        dev = safe_ts_dev[TS_DO_CTRLPANEL_AUX1];
    else
        return 0;

    uint32_t x = atoui(xp);
    uint32_t y = atoui(yp);
    report_data.key_value = 0;
    report_data.touch_num = 1;
    report_data.coord_data[0].id = 0;
    report_data.coord_data[0].x = x;
    report_data.coord_data[0].y = y;
    report_data.coord_data[0].w = 0;
    printf("%s, x=%u, y=%u\n", __func__,
           report_data.coord_data[0].x, report_data.coord_data[0].y);

    if (!strcmp(xplus, "++")) {
        for (int i = 0; i < 100; i++) {
            safe_ts_report_data(dev, &report_data, 9);
            report_data.coord_data[0].x++;
        }
    }
    else {
        safe_ts_report_data(dev, &report_data, 9);
    }

    report_data.touch_num = 0;
    safe_ts_report_data(dev, &report_data, 9);
    return 0;
}

static void touch_cmd_usage(const char *cmd)
{
    printf("%s support below commands:\n\n", cmd);
    printf("%s devices\n", cmd);
    printf("<list all touch devices>\n\n");
    printf("%s send xx1 xx2 xx3 xx4\n", cmd);
    printf("<xx1:anmain/anaux1/anaux2/anaux3/cpmain/cpaux1>\n");
    printf("<android main,android aux1,android aux2,android aux3,controlpanel main,controlpanel aux1>\n");
    printf("<xx2: x point>\n");
    printf("<xx3: y point>\n");
    printf("<xx4: ++(x point will auto plus 100 unit)>\n\n");
    printf("example: touch send anmain 50 50\n");
    printf("example: touch send anmain 50 50 ++\n");
}

static int do_touch_cmd(int argc, const cmd_args *argv)
{
    if (!strcmp(argv[1].str, "devices")) {
        do_touch_listdevice();
    }
    else if (!strcmp(argv[1].str, "send")) {
        do_touch_send(argv[2].str, argv[3].str, argv[4].str, argv[5].str);
    }
    else {
        touch_cmd_usage(argv[0].str);
    }

    return 0;
}

STATIC_COMMAND_START STATIC_COMMAND("touch", "touch cmdline func",
                                    (console_cmd)&do_touch_cmd)
STATIC_COMMAND_END(safe_touch);
#endif

