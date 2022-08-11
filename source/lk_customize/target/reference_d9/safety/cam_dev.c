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
#include "tca9539.h"
#include "tca6408.h"
#include "max9286.h"
#include "max20086.h"
#include "boardinfo_hwid_usr.h"

extern struct cam_device g_cdev[MAX_CAM_INST];
//int avm_initstatus = 0;

#if CSI_BOARD_507
static void config_camera507_pins(void)
{
    struct tca9539_device *pd;
    struct max20086_device *poc;
    dprintf(AVM_SRV_LOG, "%s: \n", __func__);

    pd = tca9539_init(10, 0x74);

    if (pd == NULL) {
        printf("init tca9359 error!\n");
        return ;
    }

    pd->ops.output_enable(pd, 3);

    //a9286 pwdn_n
    pd->ops.output_val(pd, 3, 0);
    thread_sleep(20);
    pd->ops.output_val(pd, 3, 1);

    tca9539_deinit(pd);

    poc = max20086_init(10, 0x28);

    if (poc == NULL) {
        printf("init max20086 error!\n");
        return ;
    }

    max20086_deinit(poc);
    dprintf(AVM_SRV_LOG, "%s: end\n", __func__);
}
#endif

#if CSI_BOARD_507_A02
static void card507_a02_init(int ip)
{
    struct tca6408_device *pd;
    struct tca9539_device *pd9;
    struct max20086_device *poc;
    dprintf(AVM_SRV_LOG, "\n%s: \n", __func__);

    if (ip == 0) {
        pd = tca6408_init(9, 0x20);

        if (pd == NULL) {
            printf("init tca6408 error!\n");
            return ;
        }

        tca6408_output_enable(pd, TCA6408_P2);

        //6408 pwdn_n
        tca6408_output_val(pd, TCA6408_P2, 0);
        thread_sleep(20);

        tca6408_output_val(pd, TCA6408_P2, 1);
        tca6408_deinit(pd);

        poc = max20086_init(9, 0x28);

        if (poc == NULL) {
            printf("init max20086 error!\n");
            return ;
        }

        max20086_deinit(poc);
    }
    else if (ip == 1) {
        pd9 = tca9539_init(10, 0x74);

        if (pd9 == NULL) {
            printf("init tca6408 error!\n");
            return ;
        }

        pd9->ops.output_enable(pd9, TCA9539_P02);

        //a9286 pwdn_n
        pd9->ops.output_val(pd9, TCA9539_P02, 0);
        thread_sleep(20);
        pd9->ops.output_val(pd9, TCA9539_P02, 1);
        tca9539_deinit(pd9);

        poc = max20086_init(10, 0x29);

        if (poc == NULL) {
            printf("init max20086 error!\n");
            return ;
        }

        max20086_deinit(poc);
    }

    dprintf(AVM_SRV_LOG, "%s: end\n", __func__);
}
#endif

#if CSI_BOARD_507_A02P
static void config_camera507_a02p_pins(void)
{
    struct tca6408_device *pd;
    struct max20086_device *poc;
    dprintf(0, "%s: enter\n", __func__);
    pd = tca6408_init(9, 0x20);

    if (pd == NULL) {
        printf("init tca6408 error!\n");
        return ;
    }

    tca6408_output_enable(pd, TCA6408_P2);

    //a9286 pwdn_n
    tca6408_output_val(pd, TCA6408_P2, 0);
    thread_sleep(20);
    tca6408_output_val(pd, TCA6408_P2, 1);
    tca6408_deinit(pd);

    poc = max20086_init(9, 0x28);

    if (poc == NULL) {
        printf("init max20086 error!\n");
        return ;
    }
    max20086_deinit(poc);
    thread_sleep(20);
    dprintf(0, "%s: end\n", __func__);
}
#endif

static void config_ms_camera(int ip)
{
    struct tca6408_device *pd;
    struct tca9539_device *pd9;
    struct max20086_device *poc;
    dprintf(AVM_SRV_LOG, "\n%s: \n", __func__);

    if (ip == 0) {
        pd = tca6408_init(9, 0x20);

        if (pd == NULL) {
            printf("init tca6408 error!\n");
            return ;
        }

        //POC
        tca6408_output_enable(pd, TCA6408_P7);
        tca6408_output_val(pd, TCA6408_P7, 1);

        tca6408_output_enable(pd, TCA6408_P2);
        //6408 pwdn_n
        tca6408_output_val(pd, TCA6408_P2, 0);
        thread_sleep(20);
        tca6408_output_val(pd, TCA6408_P2, 1);

        tca6408_deinit(pd);

        poc = max20086_init(9, 0x28);

        if (poc == NULL) {
            printf("init max20086 error!\n");
            return ;
        }

        max20086_deinit(poc);
    }
    else if (ip == 1) {
        pd9 = tca9539_init(10, 0x74);

        if (pd9 == NULL) {
            printf("init tca6408 error!\n");
            return ;
        }

        pd9->ops.output_enable(pd9, TCA9539_P02);

        //a9286 pwdn_n
        pd9->ops.output_val(pd9, TCA9539_P02, 0);
        thread_sleep(20);
        pd9->ops.output_val(pd9, TCA9539_P02, 1);
        tca9539_deinit(pd9);

        poc = max20086_init(10, 0x29);

        if (poc == NULL) {
            printf("init max20086 error!\n");
            return ;
        }

        max20086_deinit(poc);
    }

    dprintf(AVM_SRV_LOG, "%s: end\n", __func__);
}


static void cam_dev_default_configs(struct v4l2_mbus_framefmt *fmt,
                                    struct v4l2_fract *fi,
                                    struct v4l2_fwnode_endpoint *ep, bool *crop_enable)
{
    /* default value YUYV 1280x2880 25fps */
    fmt->code = V4L2_PIX_FMT_YUYV;
    fmt->field = V4L2_FIELD_NONE;
    fmt->colorspace = V4L2_COLORSPACE_SRGB;
    fmt->width = 1280;
    fmt->height = 2880;
    fmt->field = V4L2_FIELD_NONE;
    fi->numerator = 1;
    fi->denominator = 25;
    ep->bus_type = V4L2_MBUS_CSI2;
    *crop_enable = false;
}

static int cam_dev_get_configs(struct cam_device *cam)
{
    struct v4l2_device *vdev;
    void *handle;
    int ret;

    dprintf(AVM_SRV_LOG, "%s():\n", __func__);
    handle = cam->csi_handle;

    vdev = hal_csi_get_vdev(handle);

    if (vdev == NULL) {
        dprintf(0, "%s(): get vdev error\n", __func__);
        return -1;
    }

    ret = vdev->ops.get_interface(vdev, &cam->csi_config.ep);

    if (ret < 0) {
        dprintf(0, "%s(): vdev get interface error\n", __func__);
        return -1;
    }

    dprintf(AVM_SRV_LOG, "cam->csi_config.ep.bus_type=%d\n",
            cam->csi_config.ep.bus_type);

    ret = vdev->ops.get_fmt(vdev, &cam->csi_config.fmt);

    if (ret < 0) {
        dprintf(0, "%s(): vdev get fmt error\n", __func__);
        return -1;
    }

    dprintf(AVM_SRV_LOG, "cam->csi_config.fmt.code=0x%x\n",
            cam->csi_config.fmt.code);

    vdev->ops.g_frame_interval(vdev, &cam->csi_config.fi);

    if (ret < 0) {
        dprintf(0, "%s(): vdev get frame interval error\n", __func__);
        return -1;
    }

    dprintf(AVM_SRV_LOG, "cam->csi_config.fi.denominator=%d\n",
            cam->csi_config.fi.denominator);

    dprintf(AVM_SRV_LOG, "%s(): end\n\n", __func__);
    return ret;

}

int cam_dev_init(struct cam_device *cam)
{
    //avm_initstatus = 1;
    struct v4l2_device *vdev;
    void *handle;
    int i;

    if (cam->csi_handle != NULL) {
        dprintf(0, "csi already initialized!\n");
        //return -1;
        goto done;
    }

    for (i = 0; i < MAX_CAM_INST; i++) {
        if ((g_cdev[i].enabled) && (g_cdev[i].csi_handle)
                && (g_cdev[i].ip == cam->ip)
                && (g_cdev[i].channel != cam->channel)) {
            dprintf(0, "same as g_cdev[%d]\n", i);
            cam->csi_handle = g_cdev[i].csi_handle;
            vdev = hal_csi_get_vdev(cam->csi_handle);
            if (vdev->ops.vc_enable)
                vdev->ops.vc_enable(vdev, true, 0x1 << cam->channel);
            goto done;
        }
    }

#if 1
    if (cam->ip == 0){
        hal_csi_creat_handle(&handle, CAMERA_AVM);
    } else if (cam->ip == 1) {
        hal_csi_creat_handle(&handle, RES_CSI_CSI2);
    } else {
        return -1;
    }

    if(!handle){
        dprintf(0, "on using\n");
        return -1;
    }
#endif

    if ((get_part_id(PART_BOARD_TYPE)==BOARD_TYPE_MS) && (get_part_id(PART_BOARD_ID_MAJ)==BOARDID_MAJOR_TI_A02)){
        dprintf(0, "%s: call ms_a02 csi\n", __func__);
        config_ms_camera(cam->ip);
    } else {
#if CSI_BOARD_507
    config_camera507_pins();
#endif

#if CSI_BOARD_507_A02
    card507_a02_init(cam->ip);
#endif
#if CSI_BOARD_507_A02P
     config_camera507_a02p_pins();
#endif
    }

    if (cam->ip == 0)
        vdev = max9286_init(9, 0x2c);
    else if (cam->ip == 1)
        vdev = max9286_init(10, 0x2c);
    else
        return -1;

    if (vdev == NULL) {
        dprintf(0, "%s(): get vdev error\n", __func__);
        return -1;
    }

    if (cam->sync) {
        if (vdev->ops.sync_enable)
            vdev->ops.sync_enable(vdev, true);
        if (vdev->ops.vc_enable)
            vdev->ops.vc_enable(vdev, true, 0xf);
    }
    else {
        if (vdev->ops.vc_enable)
            vdev->ops.vc_enable(vdev, true, 0x1 << cam->channel);
    }
#if 0
    if (cam->ip == 0)
        hal_csi_creat_handle(&handle, CAMERA_AVM);
    else if (cam->ip == 1)
        hal_csi_creat_handle(&handle, RES_CSI_CSI2);
    else
        return -1;
#endif
    hal_csi_set_vdev(handle, vdev);
    cam->csi_handle = handle;

    //avm_initstatus = 2;

done:

#if 0
    cam_dev_default_configs(&cam->csi_config.fmt, &cam->csi_config.fi,
                            &cam->csi_config.ep, &cam->csi_config.crop_enable);
#else
    cam_dev_get_configs(cam);
#endif
    dprintf(AVM_SRV_LOG, "%s() end.\n", __func__);

    return 0;
}

int cam_dev_close(struct cam_device *cam)
{
    struct v4l2_device *vdev;
    void *handle;

    dprintf(AVM_SRV_LOG, "%s()\n", __func__);
    handle = cam->csi_handle;

    vdev = hal_csi_get_vdev(handle);

    if (vdev == NULL) {
        dprintf(0, "%s(): get vdev error\n", __func__);
        return -1;
    }

    if (cam->sync) {
        if (vdev->ops.vc_enable)
            vdev->ops.vc_enable(vdev, false, 0xf);
    }
    else {
        if (vdev->ops.vc_enable)
            vdev->ops.vc_enable(vdev, false, 0x1 << cam->channel);
    }

    if (vdev->ops.close)
        vdev->ops.close(vdev);
    hal_csi_release_handle(handle);
    cam->csi_handle = NULL;

    return 0;
}

void *cam_dev_get_handle(struct cam_device *cam)
{
    void *handle;
    dprintf(AVM_SRV_LOG, "%s()\n", __func__);
    handle = cam->csi_handle;
    return handle;
}

int cam_dev_enum_fmt(struct cam_device *cam,
                     struct v4l2_fmtdesc *fe)
{
    struct v4l2_device *vdev;
    void *handle;
    int ret;
    dprintf(AVM_SRV_LOG, "%s():\n", __func__);
    handle = cam->csi_handle;

    vdev = hal_csi_get_vdev(handle);

    if (vdev == NULL) {
        dprintf(0, "%s(): get vdev error\n", __func__);
        return -1;
    }

    ret = vdev->ops.enum_format(vdev, fe);

    dprintf(AVM_SRV_LOG, "%s(): end\n\n", __func__);
    return ret;
}


int cam_dev_enum_framesize(struct cam_device *cam,
                           struct v4l2_frame_size_enum *fse)
{
    struct v4l2_device *vdev;
    void *handle;
    int ret;
    dprintf(AVM_SRV_LOG, "%s():\n", __func__);
    handle = cam->csi_handle;

    vdev = hal_csi_get_vdev(handle);

    if (vdev == NULL) {
        dprintf(0, "%s(): get vdev error\n", __func__);
        return -1;
    }

    ret = vdev->ops.enum_frame_size(fse);

    dprintf(AVM_SRV_LOG, "%s(): end\n\n", __func__);
    return ret;
}

bool cam_dev_init_buf(struct cam_device *cam, int index,
                      uint8_t *pin)
{
    void *handle;
    csi_instance_t *instance = NULL;
    dprintf(AVM_SRV_LOG, "%s(): index=%d\n", __func__, index);

    handle = cam->csi_handle;
    CSI_HAL_ASSERT_PARAMETER(handle);
    instance = (csi_instance_t *)handle;

    int i, j = 0;
    struct csi_image *img;
    addr_t baddr;
    size_t mem_size;

    for (i = 0; i < IMG_COUNT; i++) {
        img = instance->dev->ops.get_image(instance->dev, i);
        dprintf(AVM_SRV_LOG,
                "%s(): cam->sync=%d, cam->ip=%d, cam->channel=%d, img->enable=%d\n",
                __func__, cam->sync, cam->ip, cam->channel, img->enable);

        if (!img || !img->enable)
            continue;

        if ((!cam->sync) && (cam->channel != img->id))
            continue;

        img->id = i;
        dprintf(AVM_SRV_LOG, "i=%d, img->rgby_stride=%d, img->height=%d\n", i,
                img->rgby_stride, img->height);

        if (img->rgby_stride && img->height) {
            mem_size = img->rgby_stride * img->height;

            if (index >= CAMERA_MAX_BUF)
                return false;

            j = index;

            if (cam->sync)
                baddr = (addr_t)(pin + (img->rgby_stride * img->height) * i);
            else
                baddr = (addr_t)(pin);

            memset((void *)baddr, 0xAA, mem_size);
            arch_clean_invalidate_cache_range(baddr, mem_size);
            img->rgby_baddr[j] = baddr;
            dprintf(AVM_SRV_LOG, "CSI RGBY ADDR %d: 0x%lx,stride:%u\n", j,
                    img->rgby_baddr[j], img->rgby_stride);
        }
        else {
            dprintf(AVM_SRV_LOG, "only support packed!\n");
            return false;
        }

        instance->dev->ops.set_image(instance->dev, img);
    }



    return true;
}

int cam_dev_get_bufinfo(void *handle, int index, uint8_t *pin)
{
    csi_instance_t *instance = NULL;
    dprintf(AVM_SRV_LOG, "%s(): line %d, index=%d*\n", __func__,
            __LINE__, index);

    CSI_HAL_ASSERT_PARAMETER(handle);
    instance = (csi_instance_t *)handle;
    struct csi_image *img;
    int mem_size = 0;

    img = instance->dev->ops.get_image(instance->dev, 0);

    if (!img || !img->enable)
        return 0;

    if (img->rgby_stride && img->height) {
        mem_size = img->rgby_stride * img->height * IMG_COUNT;

        pin = (uint8_t *)img->rgby_baddr[index];
        dprintf(AVM_SRV_LOG, "len=%d, addr=%p\n", mem_size, pin);
    }

    return mem_size;
}

int cam_dev_config(struct cam_device *cam)
{
    //bool crop_enable;
    int ret = 0;
    void *handle;
    struct v4l2_device *vdev;
    struct v4l2_ctrl ctrl = {.val = 1, .id = V4L2_CID_HFLIP,};
    csi_instance_t *instance = NULL;
    struct csi_image *img;
    int i;

    dprintf(AVM_SRV_LOG, "%s(): \n", __func__);

    handle = cam->csi_handle;
    instance = (csi_instance_t *)handle;
    vdev = hal_csi_get_vdev(handle);

    if (vdev == NULL) {
        dprintf(0, "%s(): get vdev error\n", __func__);
        return -1;
    }

    vdev->ops.s_power(vdev, 1);

    vdev->ops.set_interface(vdev, cam->csi_config.ep);

    dprintf(AVM_SRV_LOG, "fmt.width=%d, fmt.height=%d\n",
            cam->csi_config.fmt.width, cam->csi_config.fmt.height);
    ret = vdev->ops.set_fmt(vdev, cam->csi_config.fmt);

    if (ret < 0) {
        dprintf(0, "%s(): vdev set fmt error\n", __func__);
        return -1;
    }

    vdev->ops.s_frame_interval(vdev, cam->csi_config.fi);
    vdev->ops.s_ctrl(vdev, &ctrl);

    hal_csi_cfg_interface(handle, cam->csi_config.ep);

    for (i = 0; i < IMG_COUNT; i++) {
        img = instance->dev->ops.get_image(instance->dev, i);

        if (cam->sync || ((cam->channel == (uint32_t)i)))
            img->enable = true;
    }

    ret = hal_csi_init(handle);

    if (ret < 0) {
        dprintf(0, "hal csi init error.\n");
        return ret;
    }

    return ret;
}

int safe_cam_notify_agent(struct cam_device *cam, u8 *data, u16 len);

static void data_callback(struct data_callback_info *cbi)
{
    u8 pdata[10] = {0};
    u32 len = 0;
    int i;

    pdata[0] = cbi->img_id;
    pdata[1] = cbi->index;
    len = 2;
    dprintf(AVM_SRV_LOG, "%s(): img_id=%d, index=%d, paddr=0x%x, [%d]\n",
            __func__,
            cbi->img_id, cbi->index, (int)cbi->rgby_paddr, current_time());

    for (i = 0; i < MAX_CAM_INST; i++) {
        if ((g_cdev[i].enabled) && (g_cdev[i].ip == cbi->ip)
                && (g_cdev[i].channel == cbi->img_id)) {
            safe_cam_notify_agent(&g_cdev[i], pdata, len);
        }
    }

    //dprintf(AVM_SRV_LOG, "%s: [%d] end.\n", __func__, current_time());
}


int cam_dev_qbuf(struct cam_device *cam, uint32_t index)
{
    struct v4l2_device *vdev;
    void *handle;

    dprintf(AVM_SRV_LOG, "%s(): index=%d\n", __func__, index);
    handle = cam->csi_handle;

    csi_instance_t *instance = NULL;

    instance = (csi_instance_t *)handle;

    vdev = hal_csi_get_vdev(handle);

    if (vdev == NULL) {
        dprintf(0, "%s(): get vdev error\n", __func__);
        return -1;
    }

    instance->dev->ops.qbuf(instance->dev, cam->channel, &cam->cb[index]);

    dprintf(AVM_SRV_LOG, "%s(): end\n\n", __func__);
    return 0;
}


int cam_dev_start(struct cam_device *cam)
{
    struct v4l2_device *vdev;
    void *handle;
    csi_instance_t *instance = NULL;

    dprintf(AVM_SRV_LOG, "%s()\n", __func__);
    handle = cam->csi_handle;

    vdev = hal_csi_get_vdev(handle);

    if (vdev == NULL) {
        dprintf(0, "%s(): get vdev error\n", __func__);
        return -1;
    }


    instance = (csi_instance_t *)handle;

    if (cam->sync) {
        instance->dev->ops.cfg_mem(instance->dev, 0xf);
        if (vdev->ops.vc_enable)
            vdev->ops.vc_enable(vdev, true, 0xf);
        hal_csi_start(handle, 0xf);
    }
    else {
        instance->dev->ops.cfg_mem(instance->dev, 0x1 << cam->channel);
        if (vdev->ops.vc_enable)
            vdev->ops.vc_enable(vdev, true, 0x1 << cam->channel);
        hal_csi_start(handle, 0x1 << cam->channel);
    }

    dprintf(AVM_SRV_LOG, "%s(): set cbs\n", __func__);
    hal_csi_set_callback_sync(handle, cam->channel, data_callback);
    vdev->ops.s_stream(vdev, 1);

    dprintf(AVM_SRV_LOG, "%s(): end\n\n", __func__);
    return 0;
}

int cam_dev_stop(struct cam_device *cam)
{
    struct v4l2_device *vdev;
    void *handle;

    dprintf(AVM_SRV_LOG, "%s()\n", __func__);
    handle = cam->csi_handle;

    vdev = hal_csi_get_vdev(handle);

    if (vdev == NULL) {
        dprintf(0, "%s(): get vdev error\n", __func__);
        return -1;
    }

    if (cam->sync) {
        if (vdev->ops.vc_enable)
            vdev->ops.vc_enable(vdev, false, 0xf);
        hal_csi_stop(handle, 0xf);
    }
    else {
        if (vdev->ops.vc_enable)
            vdev->ops.vc_enable(vdev, false, 0x1 << cam->channel);
        hal_csi_stop(handle, 0x1 << cam->channel);
    }

    vdev->ops.s_stream(vdev, 0);

    dprintf(AVM_SRV_LOG, "%s(): end\n", __func__);
    return 0;
}

struct cam_config cam_confs[] = {
    {true, DP_CA_AP1, MBOX_INDEX_CAMERA_CSI0,},
    {false, DP_CA_AP1, MBOX_INDEX_CAMERA_CSI1_0,},
    {false, DP_CA_AP1, MBOX_INDEX_CAMERA_CSI1_1,},
    {false, DP_CA_AP1, MBOX_INDEX_CAMERA_CSI1_2,},
    {false, DP_CA_AP1, MBOX_INDEX_CAMERA_CSI1_3,},
};

int cam_service_probe_device(struct cam_device *cam, int instance)
{
    if ((uint32_t)instance >= (sizeof(cam_confs) / sizeof(struct cam_config))) {
        dprintf(0, "%s: out of range.\n", __func__);
        return -1;
    }

    if (cam_confs[instance].enabled) {
        cam->enabled = true;
        cam->rproc = cam_confs[instance].rproc;
        cam->mbox_addr = cam_confs[instance].mbox_addr;
        cam->sync = false;

        switch (cam->mbox_addr) {
            case MBOX_INDEX_CAMERA_CSI0:
                cam->sync = true;

            case MBOX_INDEX_CAMERA_CSI0_0:
                cam->channel = 0;
                cam->ip = 0;
                break;

            case MBOX_INDEX_CAMERA_CSI0_1:
                cam->channel = 1;
                cam->ip = 0;
                break;

            case MBOX_INDEX_CAMERA_CSI0_2:
                cam->channel = 2;
                cam->ip = 0;
                break;

            case MBOX_INDEX_CAMERA_CSI0_3:
                cam->channel = 3;
                cam->ip = 0;
                break;

            case MBOX_INDEX_CAMERA_CSI1:
                cam->sync = true;

            case MBOX_INDEX_CAMERA_CSI1_0:
                cam->channel = 0;
                cam->ip = 1;
                break;

            case MBOX_INDEX_CAMERA_CSI1_1:
                cam->channel = 1;
                cam->ip = 1;
                break;

            case MBOX_INDEX_CAMERA_CSI1_2:
                cam->channel = 2;
                cam->ip = 1;
                break;

            case MBOX_INDEX_CAMERA_CSI1_3:
                cam->channel = 3;
                cam->ip = 1;
                break;

            case MBOX_INDEX_CAMERA_CSI2:
            case MBOX_INDEX_CAMERA_CSI2_0:
                cam->channel = 0;
                cam->ip = 2;
                break;

            case MBOX_INDEX_CAMERA_CSI2_1:
                cam->channel = 1;
                cam->ip = 2;
                break;

            default:
                break;
        }

        dprintf(0, "%s:[%d], ip=%d, channel=%d, ok\n", __func__,
                instance, cam->ip,
                cam->channel);
        return 0;
    }
    else {
        dprintf(0, "%s:[%d] err\n", __func__, instance);
        cam->enabled = false;
        return -1;
    }
}

