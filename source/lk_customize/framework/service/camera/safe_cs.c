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
#include <lib/console.h>
#include <lib/bytes.h>
#include <lib/reg.h>
#include <dcf.h>
#include <app.h>
#include <thread.h>
#include <event.h>
#include <list.h>

#include "safe_cs.h"

#include "cam_conf.h"


#include "v4l2.h"
#include "csi_hal.h"



struct cam_device g_cdev[MAX_CAM_INST];

static bool binitialized;
//int avm_initstatus = 0;

int get_format_size(u32 format)
{
    int val = 0;

    switch (format) {
        case V4L2_PIX_FMT_YUV444:
        case V4L2_PIX_FMT_RGB24:
            val = 6;
            break;

        case V4L2_PIX_FMT_UYVY:
        case V4L2_PIX_FMT_UYVYSP:
        case V4L2_PIX_FMT_YUYV:
        case V4L2_PIX_FMT_YUYVSP:
        case V4L2_PIX_FMT_RGB565:
            val = 4;
            break;

        case V4L2_PIX_FMT_YUV420SP:
        case V4L2_PIX_FMT_YUV420XP:
            val = 3;
            break;

        default:
            break;
    }

    return val;
}


static rpc_call_result_t cam_svr_ioctl_cb(rpc_server_handle_t hserver,
        rpc_call_request_t *request)
{
    rpc_call_result_t result = {0,};
    struct scs_ioctl_cmd *ctl = (struct scs_ioctl_cmd *) &request->param[0];
    struct scs_ioctl_result *r = (struct scs_ioctl_result *) &result.result[0];
    struct cam_device *cam;
    int instance;
    int ret = 0;
    result.ack = request->cmd;

    if (!request) {
        result.retcode = ERR_INVALID_ARGS;
        return result;
    }
    result.retcode = 0;

    instance = ctl->instance;
    dprintf(AVM_SRV_LOG, "\n%s_%d(): [%d] enter:\n", __func__, instance,
            current_time());
    cam = &g_cdev[instance];

    mutex_acquire(&cam->dev_lock);

    switch (ctl->op) {
        case SCS_OP_DEV_OPEN:
            dprintf(AVM_SRV_LOG, "[%d-%d]: cam_dev_init()\n", cam->ip, cam->channel);
            ret = cam_dev_init(cam);
            if(ret<0){
                dprintf(0, "[%d-%d]: cam_dev_init() fail\n", cam->ip, cam->channel);
                result.retcode = ERR_BUSY;
            }
            break;

        case SCS_OP_ENUM_FORMAT: {
            struct v4l2_fmtdesc fe;
            fe.index = ctl->msg.fmt.index;
            cam_dev_enum_fmt(cam, &fe);
            r->msg.fmt.fmt = fe.pixelformat;    //V4L2_PIX_FMT_YUYV
            r->msg.fmt.index = fe.index;
            dprintf(AVM_SRV_LOG, "[%d-%d]: enum format: r->msg.fmt.fmt=0x%x\n",
                    cam->ip, cam->channel, r->msg.fmt.fmt);
            break;
        }

        case SCS_OP_ENUM_FRAMESIZE:
            dprintf(AVM_SRV_LOG, "[%d-%d]: enum framesize: ctl->msg.fsz.index=%d\n",
                    cam->ip, cam->channel, ctl->msg.fsz.index);
            struct v4l2_frame_size_enum fse;
            fse.index = ctl->msg.fsz.index;
            cam_dev_enum_framesize(cam, &fse);
            r->msg.fsz.type = V4L2_FRMSIZE_TYPE_DISCRETE;
            r->msg.fsz.width = fse.max_width;
            r->msg.fsz.height = fse.max_height;
            r->msg.fsz.index = fse.index;
            break;

        case SCS_OP_GET_FORMAT:
            r->msg.g_fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
            r->msg.g_fmt.width = cam->csi_config.fmt.width;
            r->msg.g_fmt.height = cam->csi_config.fmt.height;
            r->msg.g_fmt.pixelformat = cam->csi_config.fmt.code;
            break;

        case SCS_OP_SET_FORMAT:
            dprintf(AVM_SRV_LOG, "[%d-%d]: set format: (%d-%d, 0x%x)\n", cam->ip,
                    cam->channel, ctl->msg.s_fmt.width,
                    ctl->msg.s_fmt.height, ctl->msg.s_fmt.pixelformat);
            cam->csi_config.fmt.width = ctl->msg.s_fmt.width;
            cam->csi_config.fmt.height = ctl->msg.s_fmt.height;
            cam->csi_config.fmt.code = ctl->msg.s_fmt.pixelformat;

            ret = cam_dev_config(cam);

            if (ret < 0)
                goto rt;

            r->msg.g_fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
            r->msg.g_fmt.width = cam->csi_config.fmt.width;
            r->msg.g_fmt.height = cam->csi_config.fmt.height;
            r->msg.g_fmt.pixelformat = cam->csi_config.fmt.code;

            break;

        case SCS_OP_QUEUE_SETUP:
            dprintf(AVM_SRV_LOG, "[%d-%d]: queue setup: \n", cam->ip, cam->channel);
            r->msg.queue.bufs = CAMERA_MAX_BUF;
            r->msg.queue.planes = 1;
            r->msg.queue.size = cam->csi_config.fmt.width *
                                cam->csi_config.fmt.height * get_format_size(cam->csi_config.fmt.code) / 2;
            break;

        case SCS_OP_GET_BUFINFO:
            dprintf(AVM_SRV_LOG, "[%d-%d]: get bufinfo: index=%d\n", cam->ip,
                    cam->channel, ctl->msg.s_bufinfo.index);
            r->msg.g_bufinfo.index = ctl->msg.s_bufinfo.index;
            uint8_t *addr = NULL;
            r->msg.g_bufinfo.len = cam_dev_get_bufinfo(cam->csi_handle,
                                   ctl->msg.s_bufinfo.index, addr);
            r->msg.g_bufinfo.addr = (u32)p2ap((paddr_t)addr);
            break;

        case SCS_OP_SET_BUFINFO:
            dprintf(AVM_SRV_LOG, "[%d-%d]: set bufinfo: index=%d, len=%d, addr=0x%x\n",
                    cam->ip, cam->channel, ctl->msg.s_bufinfo.index, ctl->msg.s_bufinfo.len,
                    ctl->msg.s_bufinfo.addr);
            cam_dev_init_buf(cam, ctl->msg.s_bufinfo.index,
                             (uint8_t *)(ap2p((paddr_t)ctl->msg.s_bufinfo.addr)));
            break;

        case SCS_OP_QBUF:
            dprintf(AVM_SRV_LOG, "[%d-%d]: qbuf: ctl->msg.s_bufinfo.index=%d\n",
                    cam->ip, cam->channel, ctl->msg.s_bufinfo.index);
            cam_dev_qbuf(cam, ctl->msg.s_bufinfo.index);
            r->msg.g_bufinfo.index = ctl->msg.s_bufinfo.index;
            break;

        case SCS_OP_STREAM_ON:
            dprintf(AVM_SRV_LOG, "[%d-%d]: stream on: ...\n", cam->ip, cam->channel);
            cam_dev_start(cam);
            break;

        case SCS_OP_STREAM_OFF:
            dprintf(AVM_SRV_LOG, "[%d-%d]: stream off: \n", cam->ip, cam->channel);
            cam_dev_stop(cam);
            break;

        case SCS_OP_DEV_CLOSE:
            dprintf(AVM_SRV_LOG, "[%d-%d]: cam_dev_close: \n", cam->ip, cam->channel);
            cam_dev_close(cam);
            break;

        default:
            break;
    }

rt:
    //result.retcode = 0;
    mutex_release(&cam->dev_lock);

    dprintf(AVM_SRV_LOG, "%s cmd=%d[%d] exit\n", __func__, ctl->op,
            current_time());

    return result;
}

int safe_cam_add_notifier(struct cam_device *cam)
{
    struct dcf_notifier_attribute attr;

    dcf_notify_attr_init(&attr, cam->rproc, cam->mbox_addr);
    cam->notifier = dcf_create_notifier(&attr);

    if (!cam->notifier) {
        dprintf(0, "create notify failed\n");
        return -1;
    }

    return 0;
}

int safe_cam_notify_agent(struct cam_device *cam, u8 *data, u16 len)
{
    return dcf_do_notify(cam->notifier, data, len);
}

void cam_service_init(void)
{
    struct cam_device *cam;
    rpc_server_impl_t devfuncs[] = {
        {MOD_RPC_REQ_SCS_IOCTL, cam_svr_ioctl_cb, IPCC_RPC_NO_FLAGS},
    };
    int i, ret;

    if (binitialized)
        return ;

    for (int j = 0; j < MAX_CAM_INST; j++) {
        ret = cam_service_probe_device(&g_cdev[j], j);

        if (ret)
            continue;

        cam = &g_cdev[j];

        cam->cb = malloc(sizeof(struct csi_buffer) * 3);
        if(!cam->cb){
            dprintf(0, "malloc fail!\n");
            continue;
        }
        for (i = 0; i < 3; i++) {
            cam->cb[i].index = i;
        }

        mutex_init(&cam->dev_lock);
        mutex_acquire(&cam->dev_lock);
        safe_cam_add_notifier(cam);
        mutex_release(&cam->dev_lock);

    }

    start_ipcc_rpc_service(devfuncs, ARRAY_SIZE(devfuncs));

    binitialized = true;
}

void cam_service_entry(const struct app_descriptor *app, void *args)
{
    cam_service_init();
}

APP_START(cam_srv)
.entry = (app_entry)cam_service_entry,
.stack_size = 512,
APP_END

