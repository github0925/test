/*
* app_csi.c
*
* Copyright (c) 2018 Semidrive Semiconductor.
* All rights reserved.
*
* Description: implement the app to auto jump to kernel.
*
* Revision History:
* -----------------
* 001, 10/20/2019 henglei create this file
*/
#include <app.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <debug.h>
#include <platform.h>
#include <lib/console.h>
#include <arch.h>

#if WITH_KERNEL_VM
#include <kernel/vm.h>
#endif


#if EMULATION_PLATFORM_FPGA
#include "sdm_display.h"
#include "disp_data_type.h"
#endif


#include "v4l2.h"
#include "csi_hal.h"

#include "ov5640.h"
#include "mipi_bridge.h"
#include "tca9539.h"
#include "max9286.h"
#include "max20086.h"


#define APP_CSI_LOG 0


#if WITH_KERNEL_VM
#define v2p(va)    (paddr_t)(vaddr_to_paddr(va))
#define p2v(pa)    (vaddr_t)(paddr_to_kvaddr(pa))
#else
#define v2p(va)    (paddr_t)(va)
#define p2v(pa)    (vaddr_t)(pa)
#endif

#if EMULATION_PLATFORM_FPGA
typedef struct {
    uint16_t x1;
    uint16_t y1;
    uint16_t x2;
    uint16_t y2;
    uint16_t w;
    uint16_t h;
} disp_area_t;

typedef struct {
    sdm_display_t *sdm;
    struct sdm_post_config post;
    disp_area_t da[2];
} csi_disp_info_t;
#endif


static void *g_csi[2] = {NULL, NULL};

static int log_index = 0;


#if EMULATION_PLATFORM_FPGA
static csi_disp_info_t g_cdinfo;

static int csi_display_thread(void *arg)
{
    printf("%s()\n", __func__);
    struct sdm_post_config post;
    int n_bufs = 2;
    int ret;
    int i;
    struct list_node *head = sdm_get_display_list();
    sdm_display_t *sdm;
    list_for_every_entry(head, sdm, sdm_display_t, node) {
        LOGD("disp->id, disp->handle->display_id (%d, %d)\n",
             sdm->id, sdm->handle->display_id);
        printf("sdm=%p, sdm->handle=%p\n", sdm, sdm->handle);
        break;
    }
    printf("sdm=%p, sdm->handle=%p\n", sdm, sdm->handle);

    post.bufs = (struct sdm_buffer *) malloc(sizeof(struct sdm_buffer) *
                n_bufs);

    if (!post.bufs) {
        LOGE("Error: malloc sdm_buffer failed\n");
        return 0;
    }

    void *bufs[2];
    bufs[0] = malloc(640 * 480 * 2);
    memset(bufs[0], 0xff, 640 * 480 * 2);
    bufs[1] = malloc(640 * 480 * 2);
    memset(bufs[1], 0x00, 640 * 480 * 2);

    //lv_area_t disp_area = {0, 0, 640, 480};
    disp_area_t da1 = {0, 0, 640, 480};
    disp_area_t da[2] = {{0, 0, 640, 480, 640, 480}, {640, 0, 1280, 480, 640, 480}};
    post.n_bufs = n_bufs;

    while (1) {
        static int idx;
        idx++;
        printf("%s(): idx=%d, n_bufs=%d\n", __func__, idx, n_bufs);

        for (i = 0; i < n_bufs; i++) {
            struct sdm_buffer *buf = &post.bufs[i];
            int flag = (idx + i) % 2;
            printf("i=%d, flag=%d\n", i, flag);
            buf->addr[0] = (unsigned long)v2p((void *)bufs[flag]);
            //if (i == 1)
            //    buf->alpha = opa;
            //else
            buf->alpha = 0xff;
            buf->alpha_en = 1;
            buf->ckey = 0;
            buf->ckey_en = 0;
            //buf->fmt = sd_get_color_format();
            buf->fmt = COLOR_RGB565;
            buf->layer_en = 1;
            buf->src.x = da[i].x1;
            buf->src.y = da[i].y1;
            buf->src.w = da[i].w;
            buf->src.h = da[i].h;

            buf->start.x = da[i].x1;
            buf->start.y = da[i].y1;
            buf->start.w = da[i].w;
            buf->start.h = da[i].h;
            buf->src_stride[0] = da[i].w * 2;

            // dc do not support scaling.
            buf->dst.x = da[i].x1;
            buf->dst.y = da[i].y1;
            buf->dst.w = da[i].w;
            buf->dst.h = da[i].h;
            buf->z_order = i;
            //buf->z_order = 0;
        }

        ret = sdm_post(sdm->handle, &post);
        //printf("sdm_post, ret=%d\n", ret);

        if (ret) {
            LOGD("post failed: %d\n", ret);
        }

        free(post.bufs);

        thread_sleep(20000);
    }

    return 0;
}


static int csi_display_init(void)
{
    printf("%s()\n", __func__);
    struct sdm_post_config *post;
    int n_bufs = 2;
    int ret;
    int i;
    struct list_node *head = sdm_get_display_list();
    sdm_display_t *sdm;
    list_for_every_entry(head, sdm, sdm_display_t, node) {
        LOGD("disp->id, disp->handle->display_id (%d, %d)\n",
             sdm->id, sdm->handle->display_id);
        printf("sdm=%p, sdm->handle=%p\n", sdm, sdm->handle);
        break;
    }
    printf("sdm=%p, sdm->handle=%p\n", sdm, sdm->handle);
    g_cdinfo.sdm = sdm;
    post = &g_cdinfo.post;
    post->bufs = (struct sdm_buffer *) malloc(sizeof(struct sdm_buffer) *
                 n_bufs);

    if (!post->bufs) {
        LOGE("Error: malloc sdm_buffer failed\n");
        return -1;
    }

    post->n_bufs = n_bufs;

    g_cdinfo.da[0].x1 = 0;
    g_cdinfo.da[0].y1 = 0;
    g_cdinfo.da[0].x2 = 640;
    g_cdinfo.da[0].x2 = 480;
    g_cdinfo.da[0].w = 640;
    g_cdinfo.da[0].h = 480;
    g_cdinfo.da[1].x1 = 640;
    g_cdinfo.da[1].y1 = 0;
    g_cdinfo.da[1].x2 = 1280;
    g_cdinfo.da[1].x2 = 640;
    g_cdinfo.da[1].w = 640;
    g_cdinfo.da[1].h = 480;

    return 0;

}

static int csi_display_flush(uint32_t img_id, addr_t rgby_paddr)
{
    sdm_display_t *sdm;
    struct sdm_post_config *post;
    int i;
    int ret;
    //int idx;

    post = &g_cdinfo.post;
    sdm = g_cdinfo.sdm;

    for (i = 0; i < post->n_bufs; i++) {
        struct sdm_buffer *buf = &post->bufs[i];
        //int flag = (idx + i) % 2;
        //printf("i=%d, flag=%d\n", i, flag);
        buf->addr[0] = (unsigned long)rgby_paddr;
        //if (i == 1)
        //    buf->alpha = opa;
        //else
        buf->alpha = 0xff;
        buf->alpha_en = 1;
        buf->ckey = 0;
        buf->ckey_en = 0;
        //buf->fmt = sd_get_color_format();
        buf->fmt = COLOR_RGB565;
        buf->layer_en = 1;
        buf->src.x = g_cdinfo.da[i].x1;
        buf->src.y = g_cdinfo.da[i].y1;
        buf->src.w = g_cdinfo.da[i].w;
        buf->src.h = g_cdinfo.da[i].h;

        buf->start.x = g_cdinfo.da[i].x1;
        buf->start.y = g_cdinfo.da[i].y1;
        buf->start.w = g_cdinfo.da[i].w;
        buf->start.h = g_cdinfo.da[i].h;
        buf->src_stride[0] = g_cdinfo.da[i].w * 2;

        // dc do not support scaling.
        buf->dst.x = g_cdinfo.da[i].x1;
        buf->dst.y = g_cdinfo.da[i].y1;
        buf->dst.w = g_cdinfo.da[i].w;
        buf->dst.h = g_cdinfo.da[i].h;
        buf->z_order = i;
    }

    ret = sdm_post(sdm->handle, post);
    //printf("sdm_post, ret=%d\n", ret);
    return ret;
}
#endif


#if CSI_BOARD_507
static struct tca9539_device *pin_dev;
static struct max20086_device *poc_dev;

void config_camera507_pins(void)
{
    struct tca9539_device *pd;
    struct max20086_device *poc;
    printf("\n%s: \n", __func__);

    pd = tca9539_init(10, 0x74);

    if (pd == NULL) {
        printf("init tca9359 error!\n");
        return ;
    }

    pd->ops.output_enable(pd, 3);
    pd->ops.output_enable(pd, 6);
    pd->ops.output_enable(pd, 9);
    pd->ops.output_enable(pd, 10);


    //a9286 pwdn_n
    pd->ops.output_val(pd, 3, 1);
    //b9286 pwdn_n
    pd->ops.output_val(pd, 6, 1);
    //9: ov5640 pwdn, need low
    //10, ov5640 reset_n, need high
    pd->ops.output_val(pd, 9, 0);
    pd->ops.output_val(pd, 10, 1);

    pin_dev = pd;

    poc = max20086_init(10, 0x28);

    if (poc == NULL) {
        printf("init max20086 error!\n");
        return ;
    }

    poc_dev = poc;

    printf("\n%s: end\n", __func__);
}
#endif

#if CSI_BOARD_508
static struct tca9539_device *pin_dev;

void config_camera508_pins(void)
{
    struct tca9539_device *pd;
    printf("\n%s: \n", __func__);
    pd = tca9539_init(10, 0x74);

    if (pd == NULL) {
        printf("init tca9359 error!\n");
        return ;
    }

    pd->ops.output_enable(pd, 1);
    //pd->ops.output_enable(11);
    //pd->ops.output_enable(12);

    pd->ops.output_val(pd, 1, 0);
    //pd->ops.output_val(11, 1);
    //pd->ops.output_val(12, 0);

    //pd->ops.output_val(1, 1);
    pin_dev = pd;
}
#endif


static int csi_init(int argc, const cmd_args *argv)
{
    struct v4l2_device *vdev;
    unsigned int id;
    void *csi_handle;

#if EMULATION_PLATFORM_FPGA
    csi_display_init();
    config_ov5640_pin();
#else
#if CSI_BOARD_507
    config_camera507_pins();
#endif
#if CSI_BOARD_508
    config_camera508_pins();
#endif
#endif

    if (argc == 1) {
        printf("need argv[1]=0\n");
        return -1;
    }

    id = atoui(argv[1].str);

    if ((id != 0)) {
        printf("%s: invalid id %u\n", __func__, id);
        return -1;
    }

    dprintf(APP_CSI_LOG, "%s(): id=%d\n", __func__, id);

    if (g_csi[id] != NULL) {
        printf("csi %d already initialized!\n", id);
        return -1;
    }


#if EMULATION_PLATFORM_FPGA
    vdev = ov5640_init(id);

    if (id == 0) {
        hal_csi_creat_handle(&csi_handle, RES_CSI_CSI1);
    }
    else if (id == 1) {
        hal_csi_creat_handle(&csi_handle, RES_CSI_CSI2);
    }

#else

    if (id == 0) {
        vdev = max9286_init(9, 0x2c);
        hal_csi_creat_handle(&csi_handle, CAMERA_AVM);
    }
    else {
        printf("%s: invalid id %u\n", __func__, id);
        return -1;
    }

#endif

    hal_csi_set_vdev(csi_handle, vdev);
    g_csi[id] = csi_handle;

    dprintf(APP_CSI_LOG, "%s() end.\n", __func__);
    return 0;
}

static int csi_parse_configs(int argc, const cmd_args *argv,
                             struct v4l2_mbus_framefmt *fmt, struct v4l2_fract *fi,
                             struct v4l2_fwnode_endpoint *ep, bool *crop_enable)
{
    /* default value YUYV 1280x720 25fps */
    fmt->code = V4L2_PIX_FMT_YUYV;
    fmt->field = V4L2_FIELD_NONE;
    fmt->colorspace = V4L2_COLORSPACE_SRGB;
    fmt->width = 1280;
    fmt->height = 720;
    fmt->field = V4L2_FIELD_NONE;
    fi->numerator = 1;
    fi->denominator = 25;
    ep->bus_type = V4L2_MBUS_CSI2;
    *crop_enable = false;

    if ((argc != 2) && (argc != 5) && (argc != 6) && (argc != 7)) {
        printf("usage:\n");
        printf("    csi_config bus_id (resolution) (fps) (format) (interface) (crop)\n");
        printf("\tbus_id:\n");
        printf("\t\t 0:bus 0      1:bus 1\n");
        printf("\tresolution:\n");
        printf("\t\t 1:320x240    2:640x480    3:720x480   4:720x576\n");
        printf("\t\t 5:1024x768   6:1280x720   7:1920x1080 8:2592x1944\n");
        printf("\tfps:\n");
        printf("\t\t 1:15fps    2:30fps\n");
        printf("\tformat:\n");
        printf("\t\t 1:UYVY       2:YUYV       3:RGB565    4:RGB565 BE\n");
        printf("\t\t 5:UYVYSP     6:YUYVSP     7:YUV420SP  8:YUV420XP\n");
        printf("\t\t 9:YUV444\n");
        printf("\tinterface:\n");
        printf("\t\t 0: PARALLEL  1:BT656\n");
        printf("\tcrop:\n");
        printf("\t\t 0: disable   1:enable\n");
        goto out;
    }

    if (argc == 2) {
        printf("use default value rgb565 640x480 15fps \n");
        return 0;
    }

    switch (atoui(argv[2].str)) {
        case 1:
            fmt->width = 320, fmt->height = 240;
            break;

        case 2:
            fmt->width = 640, fmt->height = 480;
            break;

        case 3:
            fmt->width = 720, fmt->height = 480;
            break;

        case 4:
            fmt->width = 720, fmt->height = 576;
            break;

        case 5:
            fmt->width = 1024, fmt->height = 768;
            break;

        case 6:
            fmt->width = 1280, fmt->height = 720;
            break;

        case 7:
            fmt->width = 1920, fmt->height = 1080;
            break;

        case 8:
            fmt->width = 2592, fmt->height = 1944;
            break;

        default:
            printf("Unsupport resolution!\n");
            break;
    }

    switch (atoui(argv[3].str)) {
        case 1:
            fi->denominator = 15;
            break;

        case 2:
            fi->denominator = 30;
            break;

        default:
            printf("Unsupport fps!\n");
            break;
    }

    switch (atoui(argv[4].str)) {
        case 1:
            fmt->code = V4L2_PIX_FMT_UYVY;
            break;

        case 2:
            fmt->code = V4L2_PIX_FMT_YUYV;
            break;

        case 3:
            fmt->code = V4L2_PIX_FMT_RGB565;
            break;

        case 4:
            fmt->code = V4L2_PIX_FMT_RGB565X;
            break;

        case 5:
            fmt->code = V4L2_PIX_FMT_UYVYSP;
            break;

        case 6:
            fmt->code = V4L2_PIX_FMT_YUYVSP;
            break;

        case 7:
            fmt->code = V4L2_PIX_FMT_YUV420SP;
            break;

        case 8:
            fmt->code = V4L2_PIX_FMT_YUV420XP;
            break;

        case 9:
            fmt->code = V4L2_PIX_FMT_YUV444;
            break;

        default:
            printf("Unsupport format!\n");
            break;
    }

    if (argc == 6) {
        switch (atoui(argv[5].str)) {
            case 0:
                ep->bus_type = V4L2_MBUS_PARALLEL;
                break;

            case 1:
                ep->bus_type = V4L2_MBUS_BT656;
                break;

            default:
                printf("Unsupport interface!\n");
                break;
        }
    }

    if (argc == 7) {
        switch (atoui(argv[6].str)) {
            case 0:
                *crop_enable = false;
                break;

            case 1:
                *crop_enable = true;
                break;

            default:
                printf("crop setting error!\n");
                break;
        }
    }

out:
    return 0;
}


static int csi_config(int argc, const cmd_args *argv)
{
    struct v4l2_mbus_framefmt fmt;
    struct v4l2_fract frame_interval;
    struct v4l2_fwnode_endpoint endpoint;
    bool crop_enable;

    int id;
    int ret = 0;
    void *csi_handle;
    struct v4l2_device *vdev;

    struct v4l2_ctrl ctrl = {.val = 1, .id = V4L2_CID_HFLIP,};
//  struct v4l2_ctrl ctrl = {.val = 1, .id = V4L2_CID_TEST_PATTERN,};

    dprintf(APP_CSI_LOG, "\n%s(): \n", __func__);

    if (argc == 1) {
        printf("usage:\n");
        printf("    csi_config bus_id (resolution) (fps) (format) (interface) (crop)\n");
        printf("\tbus_id:\n");
        printf("\t\t 0:bus 0      1:bus 1\n");
        printf("\tresolution:\n");
        printf("\t\t 1:320x240    2:640x480    3:720x480   4:720x576\n");
        printf("\t\t 5:1024x768   6:1280x720   7:1920x1080 8:2592x1944\n");
        printf("\tfps:\n");
        printf("\t\t 1:15fps    2:30fps\n");
        printf("\tformat:\n");
        printf("\t\t 1:UYVY       2:YUYV       3:RGB565    4:RGB565 BE\n");
        printf("\t\t 5:UYVYSP     6:YUYVSP     7:YUV420SP  8:YUV420XP\n");
        printf("\t\t 9:YUV444\n");
        printf("\tinterface:\n");
        printf("\t\t 0: PARALLEL  1:BT656\n");
        printf("\tcrop:\n");
        printf("\t\t 0: disable   1:enable\n");
        return -1;
    }


    csi_parse_configs(argc, argv, &fmt, &frame_interval, &endpoint,
                      &crop_enable);

    if (argc == 1) {
        printf("need argv[1]=0,1\n");
        return -1;
    }

    id = atoui(argv[1].str);

    if ((id != 0)) {
        printf("%s: invalid id %u\n", __func__, id);
        return -1;
    }

    dprintf(APP_CSI_LOG, "%s(): id=%d\n", __func__, id);
    csi_handle = g_csi[id];
    vdev = hal_csi_get_vdev(csi_handle);



    vdev->ops.s_power(vdev, 1);

    vdev->ops.set_interface(vdev, endpoint);
    printf("fmt.width=%d, fmt.height=%d\n", fmt.width, fmt.height);
    ret = vdev->ops.set_fmt(vdev, fmt);

    if (ret < 0) {
        printf("%s(): vdev set fmt error\n", __func__);
        return -1;
    }

    vdev->ops.s_frame_interval(vdev, frame_interval);
    vdev->ops.s_ctrl(vdev, &ctrl);


    hal_csi_cfg_interface(csi_handle, endpoint);

    ret = hal_csi_init(csi_handle);

    if (ret < 0) {
        printf("hal csi init error.\n");
        return ret;
    }

    dprintf(APP_CSI_LOG, "\n%s(): call init mem\n", __func__);
    hal_csi_init_mem(csi_handle);

    dprintf(APP_CSI_LOG, "\n%s() end\n", __func__);


    return ret;
}




static void display_callback(uint32_t img_id, addr_t rgby_paddr)
{
#if EMULATION_PLATFORM_FPGA
    csi_display_flush(img_id, rgby_paddr);
#else

    if (log_index < 20) {
        dprintf(APP_CSI_LOG, "%s(): img_id=%d, paddr=0x%x\n", __func__, img_id,
                rgby_paddr);
        log_index++;
    }

#endif
}

static int csi_start(int argc, const cmd_args *argv)
{
    struct csi_device *dev;
    struct v4l2_device *vdev;
    int id;
    void *csi_handle;

    if (argc == 1) {
        printf("need argv[1]=0,1\n");
        return -1;
    }

    id = atoui(argv[1].str);

    if ((id != 0)) {
        printf("%s: invalid id %u\n", __func__, id);
        return -1;
    }

    dprintf(APP_CSI_LOG, "%s(): id=%d\n", __func__, id);
    csi_handle = g_csi[id];

    csi_handle = g_csi[id];
    vdev = hal_csi_get_vdev(csi_handle);

    if (vdev == NULL) {
        printf("%s(): get vdev error\n", __func__);
        return -1;
    }

    hal_csi_start(csi_handle, true);
    vdev->ops.s_stream(vdev, 1);

    hal_csi_set_display_info(csi_handle, display_callback);


    dprintf(APP_CSI_LOG, "%s(): end\n\n", __func__);
    return 0;
}


static int csi_stop(int argc, const cmd_args *argv)
{
    struct csi_device *dev;
    struct v4l2_device *vdev;
    int id;
    void *csi_handle;

    if (argc == 1) {
        printf("need argv[1]=0\n");
    }

    id = atoui(argv[1].str);

    if ((id != 0)) {
        printf("%s: invalid id %u\n", __func__, id);
        return -1;
    }

    dprintf(APP_CSI_LOG, "%s(): id=%d", __func__, id);
    csi_handle = g_csi[id];

    vdev = hal_csi_get_vdev(csi_handle);

    if (vdev == NULL) {
        printf("%s(): get vdev error\n", __func__);
        return -1;
    }

    hal_csi_stop(csi_handle);

    vdev->ops.s_stream(vdev, 0);

    dprintf(APP_CSI_LOG, "%s(): end\n\n", __func__);
    return 0;
}


static void csi_entry(const struct app_descriptor *app, void *args)
{
    struct v4l2_device *vdev;
    struct v4l2_mbus_framefmt fmt;
    struct v4l2_fract frame_interval;
    struct v4l2_fwnode_endpoint endpoint;
    int ret = 0;
    struct v4l2_ctrl ctrl = {.val = 1, .id = V4L2_CID_VFLIP,};
    void *csi_handle;

    dprintf(APP_CSI_LOG, "%s()\n", __func__);

#if 1
    cmd_args argv[2];
    argv[0].str = "csi_init";
    argv[1].str = "0";
    ret = csi_init(2, argv);

    if (ret < 0) {
        printf("%s(): init error\n", __func__);
        return ;
    }

#endif

#if 1
    argv[0].str = "csi_config";
    argv[1].str = "0";
    ret = csi_config(2, argv);

    if (ret < 0) {
        printf("%s(): config error\n", __func__);
        return ;
    }

#endif


#if 1
    argv[0].str = "csi_start";
    argv[1].str = "0";
    ret = csi_start(2, argv);

    if (ret < 0) {
        printf("%s(): start error\n", __func__);
        return ;
    }

#endif

    dprintf(APP_CSI_LOG, "%s(): end\n\n", __func__);
}


#if defined(WITH_LIB_CONSOLE)
#include <lib/console.h>
STATIC_COMMAND_START
STATIC_COMMAND("csi_init", "csi init", (console_cmd)&csi_init)
STATIC_COMMAND("csi_config", "csi config", (console_cmd)&csi_config)
STATIC_COMMAND("csi_start", "csi start", (console_cmd)&csi_start)
STATIC_COMMAND("csi_stop", "csi stop", (console_cmd)&csi_stop)
STATIC_COMMAND_END(csi);
#endif

#if 0
APP_START(csi_example)
.flags = 0,
.entry = csi_entry,
APP_END
#endif
