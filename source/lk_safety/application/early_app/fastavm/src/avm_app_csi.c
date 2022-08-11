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

#include <ext_data.h>

#include "v4l2.h"
#include "csi_hal.h"

#include "ov5640.h"
#include "mipi_bridge.h"
#include "tca9539.h"
#include "tca6408.h"
#include "max9286.h"
#include "max20086.h"
#include "n4.h"

#include "avm_app_csi.h"

#include <hal_port.h>
#include "hal_dio.h"

#include "boardinfo_hwid_usr.h"

#define APP_CSI_LOG 4


#if WITH_KERNEL_VM
#define v2p(va)    (paddr_t)(vaddr_to_paddr(va))
#define p2v(pa)    (vaddr_t)(paddr_to_kvaddr(pa))
#else
#define v2p(va)    (paddr_t)(va)
#define p2v(pa)    (vaddr_t)(pa)
#endif

static void *g_csi[2] = {NULL, NULL};

static int log_index = 0;

#if CSI_BOARD_507
static struct tca9539_device *pin_dev;
static struct max20086_device *poc_dev;

static void config_camera507_pins(void)
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
    tca9539_deinit(pd);

    poc = max20086_init(10, 0x28);

    if (poc == NULL) {
        printf("init max20086 error!\n");
        return ;
    }

    poc_dev = poc;
    max20086_deinit(poc);
    printf("\n%s: end\n", __func__);
}
#endif

#if CSI_BOARD_507_A02
static struct tca6408_device *pin_dev;
static struct max20086_device *poc_dev;

static void config_camera507_a02_pins(void)
{
    struct tca6408_device *pd;
    struct max20086_device *poc;
    //printf("\n%s: \n", __func__);

    pd = tca6408_init(9, 0x20);

    if (pd == NULL) {
        printf("init tca6408 error!\n");
        return ;
    }

	tca6408_output_enable(pd, TCA6408_P2);

	//a9286 pwdn_n
    tca6408_output_val(pd, TCA6408_P2, 1);


    pin_dev = pd;
    tca6408_deinit(pd);
    poc = max20086_init(9, 0x28);

    if (poc == NULL) {
        printf("init max20086 error!\n");
        return ;
    }
    thread_sleep(20);
    poc_dev = poc;
    max20086_deinit(poc);
    //printf("\n%s: end\n", __func__);
}
#endif

#if CSI_BOARD_507_A02P
static struct tca6408_device *pin_dev;
static struct max20086_device *poc_dev;

static void config_camera507_a02p_pins(void)
{
    struct tca6408_device *pd;
    struct max20086_device *poc;
    //printf("\n%s: \n", __func__);

    pd = tca6408_init(9, 0x20);

    if (pd == NULL) {
        printf("init tca6408 error!\n");
        return ;
    }

	tca6408_output_enable(pd, TCA6408_P2);

    //a9286 pwdn_n
	tca6408_output_val(pd, TCA6408_P2, 1);


    pin_dev = pd;

    poc = max20086_init(9, 0x28);

    if (poc == NULL) {
        printf("init max20086 error!\n");
        return ;
    }
    thread_sleep(20);
    poc_dev = poc;

    //printf("\n%s: end\n", __func__);
}
#endif


#if CSI_BOARD_508
static struct tca9539_device *pin_dev;

void config_camera508_pins(void)
{
    struct tca9539_device *pd;
    //printf("\n%s: \n", __func__);
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
    thread_sleep(20);
    //pd->ops.output_val(1, 1);
    pin_dev = pd;
}
#endif

#if CSI_BOARD_ICL02
extern const domain_res_t g_iomuxc_res;
extern const domain_res_t g_gpio_res;
static void config_camera_icl02_pins(void)
{
    struct tca6408_device *pd;
    struct max20086_device *poc;
    void *g_handle = NULL;
    Port_PinType pin = PortConf_PIN_I2S_SC6_SCK;
    Port_PinModeType mode = {//pin113
        ((uint32_t)PORT_PAD_POE__DISABLE | PORT_PAD_IS__OUT | PORT_PAD_SR__FAST | PORT_PAD_DS__MID1 | PORT_PIN_IN_PULL_UP),
        ((uint32_t)PORT_PIN_MUX_FV__MIN | PORT_PIN_MUX_FIN__MIN | PORT_PIN_OUT_PUSHPULL | PORT_PIN_MODE_GPIO),
    };

    pd = tca6408_init(9, 0x20);

    if (pd == NULL) {
        printf("init tca6408 error!\n");
        return ;
    }

    tca6408_output_enable(pd, TCA6408_P5);

    // 6408 pwdn_n
    tca6408_output_val(pd, TCA6408_P5, 1);

    // pmu
    hal_port_creat_handle(&g_handle, g_iomuxc_res.res_id[0]);
    hal_port_set_pin_mode(g_handle, pin, mode);
    hal_port_release_handle(&g_handle);

    hal_dio_creat_handle(&g_handle, g_gpio_res.res_id[0]);
    hal_dio_write_channel(g_handle, pin, 1);
    hal_dio_release_handle(&g_handle);
    thread_sleep(10);

    // poc
    tca6408_output_enable(pd, TCA6408_P7);
    tca6408_output_val(pd, TCA6408_P7, 1);
    tca6408_deinit(pd);
    thread_sleep(10);

    poc = max20086_init(9, 0x28);

    if (poc == NULL) {
        printf("init max20086 error!\n");
        return ;
    }
    thread_sleep(20);
    max20086_deinit(poc);
}
#endif

#if CSI_BOARD_510
extern const domain_res_t g_gpio_res;
extern const domain_res_t g_iomuxc_res;

static void config_camera510_pins(void)
{
    static void *dio_handle;
    static void *port_handle;
    bool ioret;

    const Port_PinModeType MODE_GPIO_OSPI2_DATA3 = {
        ((uint32_t)PORT_PAD_POE__DISABLE | PORT_PAD_IS__OUT | PORT_PAD_SR__FAST | PORT_PAD_DS__MID1 | PORT_PIN_IN_NO_PULL),
        ((uint32_t)PORT_PIN_MUX_FV__MIN | PORT_PIN_MUX_FIN__MIN  | PORT_PIN_OUT_PUSHPULL | PORT_PIN_MODE_GPIO),
    };

    /* Port setup */
    ioret = hal_port_creat_handle(&port_handle, g_iomuxc_res.res_id[0]);
    if (!ioret) {
        return;
    }
    ioret = hal_port_set_pin_mode(port_handle, PortConf_PIN_OSPI2_DATA3,
                                MODE_GPIO_OSPI2_DATA3);
    hal_port_release_handle(&port_handle);

    //RSTB Active 12uSec Pin: OSPI_DATA3 GPIO.IO85
    ioret = hal_dio_creat_handle(&dio_handle, g_gpio_res.res_id[0]);
    if (!ioret) {
        return;
    }
    hal_dio_set_channel_direction(dio_handle, PortConf_PIN_OSPI2_DATA3, DIO_CHANNEL_OUT);
    hal_dio_write_channel(dio_handle, PortConf_PIN_OSPI2_DATA3, 0);
    thread_sleep(5);//5000 uSec
    hal_dio_write_channel(dio_handle, PortConf_PIN_OSPI2_DATA3, 1);
    hal_dio_release_handle(&dio_handle);
}
#endif

static void config_ms_camera(void)
{
    struct tca6408_device *pd;
    struct max20086_device *poc;

    pd = tca6408_init(9, 0x20);

    if (pd == NULL) {
        printf("init tca6408 error!\n");
        return ;
    }

    //P7, POC
    tca6408_output_enable(pd, TCA6408_P7);
    tca6408_output_val(pd, TCA6408_P7, 1);
    thread_sleep(100);

    //9286 pwdn_n
    tca6408_output_enable(pd, TCA6408_P2);
    tca6408_output_val(pd, TCA6408_P2, 1);

    tca6408_deinit(pd);
    poc = max20086_init(9, 0x28);

    if (poc == NULL) {
        printf("init max20086 error!\n");
        return ;
    }
    thread_sleep(20);

    max20086_deinit(poc);
}


int initstatus = 0;

int avm_csi_init(uint32_t id)
{
    if(0==initstatus)
    {
        initstatus = 1;
        struct v4l2_device *vdev;
        void *csi_handle;

        if ((get_part_id(PART_BOARD_TYPE)==BOARD_TYPE_MS) && (get_part_id(PART_BOARD_ID_MAJ)==BOARDID_MAJOR_TI_A02)){
            dprintf(APP_CSI_LOG, "ms_a02_csi\n");
            config_ms_camera();
        } else {
#if CSI_BOARD_507
        config_camera507_pins();
#endif
#if CSI_BOARD_508
        config_camera508_pins();
#endif
#if CSI_BOARD_507_A02
        config_camera507_a02_pins();
#endif

#if CSI_BOARD_507_A02P
        config_camera507_a02p_pins();
#endif

#if CSI_BOARD_ICL02
        config_camera_icl02_pins();
#endif

#if CSI_BOARD_510
        config_camera510_pins();
#endif
        }

        if ((id != 0)) {
            printf("%s: invalid id %u\n", __func__, id);
            return -1;
        }

        dprintf(APP_CSI_LOG, "%s(): id=%d\n", __func__, id);

        if (g_csi[id] != NULL) {
            printf("csi %d already initialized!\n", id);
            return -1;
        }

        if (id == 0) {
#if !CSI_BOARD_510
            vdev = max9286_init(9, 0x2c);
            if(vdev == NULL){
                printf("max9286_init fail! no vdev\n");
                return -1;
            }
            max9286_set_sync_mode(vdev, true);
            max9286_vc_channel_enable(vdev, true, 0xf);
#else
            vdev = n4_init(9, 0x30);
            if(vdev == NULL){
                printf("next_chip_n4_init fail! no vdev\n");
                return -1;
            }
            n4_set_sync_mode(vdev, true);
            n4_vc_channel_enable(vdev, true, 0xf);
#endif
            hal_csi_creat_handle(&csi_handle, CAMERA_AVM);
        }
        else {
            printf("%s: invalid id %u\n", __func__, id);
            return -1;
        }

        hal_csi_set_vdev(csi_handle, vdev);
        g_csi[id] = csi_handle;

        initstatus = 2;

        dprintf(APP_CSI_LOG, "%s() end.\n", __func__);
    }
    return 0;
}

int csi_default_configs(struct v4l2_mbus_framefmt *fmt, struct v4l2_fract *fi,
                             struct v4l2_fwnode_endpoint *ep, bool *crop_enable)
{
    /* default value UYVY 1280x720 25fps */
    fmt->code = V4L2_PIX_FMT_UYVY;
    fmt->field = V4L2_FIELD_NONE;
    fmt->colorspace = V4L2_COLORSPACE_SRGB;
    fmt->width = 1280;
    fmt->height = 720*4;
    fmt->field = V4L2_FIELD_NONE;
    fi->numerator = 1;
    fi->denominator = 25;
    ep->bus_type = V4L2_MBUS_CSI2;
    *crop_enable = false;

    return 0;
}
static bool avm_csi_init_ddr_anyRes(void *handle, uint8_t *pin, uint16_t width, uint16_t height)
{
    csi_instance_t *instance = NULL;
    dprintf(APP_CSI_LOG, "%s(): line %d\n", __func__, __LINE__);

    CSI_HAL_ASSERT_PARAMETER(handle);
    instance = (csi_instance_t *)handle;

    int i, j;
    struct csi_image *img;
    addr_t baddr;
    static int mem_align = 0;
    size_t mem_size;

    mem_align &= 0x0F;

    for (i = 0; i < IMG_COUNT; i++)
    {
        img = instance->dev->ops.get_image(instance->dev, i);

        if (!img || !img->enable)
        continue;

        img->id = i;
        dprintf(APP_CSI_LOG, "i=%d, img->rgby_stride=%d, img->height=%d\n", i, img->rgby_stride,img->height);

        if (img->rgby_stride && img->height) {
            //mem_size = img->rgby_stride * img->height + 0x10;
            mem_size = img->rgby_stride * img->height;

            for (j = 0; j < CAMERA_MAX_BUF; j++) {
                baddr = (addr_t)(pin+j*width*height*2*4+width*height*2*i);
                memset((void *)baddr, 0xAA, mem_size);
                arch_clean_invalidate_cache_range(baddr, mem_size);
                //img->rgby_baddr[j] = p2ap(baddr);
                img->rgby_baddr[j] = baddr;
                dprintf(APP_CSI_LOG, "CSI RGBY ADDR %d: 0x%lx,stride:%u\n",
                j, img->rgby_baddr[j], img->rgby_stride);
            }
        } else {
            dprintf(APP_CSI_LOG, "only support packed!\n");
            continue;
        }
        instance->dev->ops.set_image(instance->dev, img);
        instance->dev->ops.cfg_mem(instance->dev, 1 << i);
    }

    // mem_align++;

    return true;
}
static bool avm_csi_init_ddr(void *handle, uint8_t (*pin)[IMG_COUNT][1280*720*2])
{
    csi_instance_t *instance = NULL;
    dprintf(APP_CSI_LOG, "hal_csi_init_mem()\n");

    CSI_HAL_ASSERT_PARAMETER(handle);
    instance = (csi_instance_t *)handle;

    int i, j;
    struct csi_image *img;
    addr_t baddr;
    static int mem_align = 0;
    size_t mem_size;

    mem_align &= 0x0F;

    for (i = 0; i < IMG_COUNT; i++) {
        img = instance->dev->ops.get_image(instance->dev, i);

        if (!img || !img->enable)
            continue;

        img->id = i;
        //printf("i=%d, img->rgby_stride=%d, img->height=%d\n", i, img->rgby_stride,
        //       img->height);

        if (img->rgby_stride && img->height) {
            //mem_size = img->rgby_stride * img->height + 0x10;
            mem_size = img->rgby_stride * img->height;

            for (j = 0; j < CAMERA_MAX_BUF; j++) {
                baddr = (addr_t)(*(*(pin + j) + i));
                memset((void *)baddr, 0xAA, mem_size);
                arch_clean_invalidate_cache_range(baddr, mem_size);
                //img->rgby_baddr[j] = p2ap(baddr);
                img->rgby_baddr[j] = baddr;
                dprintf(APP_CSI_LOG, "CSI RGBY ADDR %d: 0x%lx,stride:%u\n",
                        j, img->rgby_baddr[j], img->rgby_stride);
            }
        } else {
            dprintf(APP_CSI_LOG, "only support packed!\n");
            continue;
        }

        instance->dev->ops.set_image(instance->dev, img);
        instance->dev->ops.cfg_mem(instance->dev, 1 << i);
    }

    // mem_align++;

    return true;
}

 int avm_csi_config_anyRes(int id, uint8_t *pin, uint16_t width, uint16_t height, struct v4l2_fract anyframe_interval)
{
    struct v4l2_mbus_framefmt fmt;
    struct v4l2_fract frame_interval;
    struct v4l2_fwnode_endpoint endpoint;
    bool crop_enable;

    int ret = 0;
    void *csi_handle;
    struct v4l2_device *vdev;

    struct v4l2_ctrl ctrl = {.val = 1, .id = V4L2_CID_HFLIP,};
//  struct v4l2_ctrl ctrl = {.val = 1, .id = V4L2_CID_TEST_PATTERN,};

    int i;
    csi_instance_t *instance = NULL;
    struct csi_image *img;

    dprintf(APP_CSI_LOG, "\n%s(): \n", __func__);
    frame_interval = anyframe_interval;

    csi_default_configs(&fmt, &frame_interval, &endpoint,
                      &crop_enable);//////change camera setting here

    fmt.width = width;
    fmt.height = height*4;

    if ((id != 0)) {
        printf("%s: invalid id %u\n", __func__, id);
        return -1;
    }

    dprintf(APP_CSI_LOG, "%s(): id=%d\n", __func__, id);
    csi_handle = g_csi[id];
    vdev = hal_csi_get_vdev(csi_handle);


    vdev->ops.s_power(vdev, 1);

    vdev->ops.set_interface(vdev, endpoint);
    dprintf(APP_CSI_LOG, "fmt.width=%d, fmt.height=%d\n", fmt.width, fmt.height);
    ret = vdev->ops.set_fmt(vdev, fmt);

    if (ret < 0) {
        printf("%s(): vdev set fmt error\n", __func__);
        return -1;
    }

    vdev->ops.s_frame_interval(vdev, frame_interval);
    vdev->ops.s_ctrl(vdev, &ctrl);

    //avm_csi_expose_adjust(id,2,10);
    //avm_csi_expose_adjust(id,3,10000);
    //avm_csi_blacklevel_adjust(id,0,128);
    //avm_csi_blacklevel_adjust(id,1,128);
    //avm_csi_blacklevel_adjust(id,2,128);
    //avm_csi_blacklevel_adjust(id,3,128);

    hal_csi_cfg_interface(csi_handle, endpoint);

    instance = (csi_instance_t *)csi_handle;
    for (i = 0; i < IMG_COUNT; i++) {
        img = instance->dev->ops.get_image(instance->dev, i);
        img->enable=true;
    }

    ret = hal_csi_init(csi_handle);

    if (ret < 0) {
        printf("hal csi init error.\n");
        return ret;
    }

    dprintf(APP_CSI_LOG, "\n------------------%s(): call init mem\n", __func__);
    avm_csi_init_ddr_anyRes(csi_handle, pin, width, height);

    dprintf(APP_CSI_LOG, "\n------------------%s() end\n", __func__);


    return ret;
}



int avm_csi_config(int id, uint8_t (*pin)[IMG_COUNT][1280*720*2])
{
    struct v4l2_mbus_framefmt fmt;
    struct v4l2_fract frame_interval;
    struct v4l2_fwnode_endpoint endpoint;
    bool crop_enable;

    int ret = 0;
    void *csi_handle;
    struct v4l2_device *vdev;

    struct v4l2_ctrl ctrl = {.val = 1, .id = V4L2_CID_HFLIP,};
//  struct v4l2_ctrl ctrl = {.val = 1, .id = V4L2_CID_TEST_PATTERN,};

    int i;
    csi_instance_t *instance = NULL;
    struct csi_image *img;

    dprintf(APP_CSI_LOG, "\n%s(): \n", __func__);

    csi_default_configs(&fmt, &frame_interval, &endpoint,
                      &crop_enable);

    if ((id != 0)) {
        printf("%s: invalid id %u\n", __func__, id);
        return -1;
    }

    dprintf(APP_CSI_LOG, "%s(): id=%d\n", __func__, id);
    csi_handle = g_csi[id];
    vdev = hal_csi_get_vdev(csi_handle);


    vdev->ops.s_power(vdev, 1);

    vdev->ops.set_interface(vdev, endpoint);
    //printf("fmt.width=%d, fmt.height=%d\n", fmt.width, fmt.height);
    ret = vdev->ops.set_fmt(vdev, fmt);

    if (ret < 0) {
        printf("%s(): vdev set fmt error\n", __func__);
        return -1;
    }

    vdev->ops.s_frame_interval(vdev, frame_interval);
    vdev->ops.s_ctrl(vdev, &ctrl);


    hal_csi_cfg_interface(csi_handle, endpoint);

    instance = (csi_instance_t *)csi_handle;
    for (i = 0; i < IMG_COUNT; i++) {
        img = instance->dev->ops.get_image(instance->dev, i);
        img->enable=true;
    }
    ret = hal_csi_init(csi_handle);

    if (ret < 0) {
        printf("hal csi init error.\n");
        return ret;
    }

    dprintf(APP_CSI_LOG, "\n%s(): call init mem\n", __func__);
    avm_csi_init_ddr(csi_handle, pin);

    dprintf(APP_CSI_LOG, "\n%s() end\n", __func__);


    return ret;
}


static void display_callback(uint32_t img_id, addr_t rgby_paddr)
{
    if (log_index < 20) {
        dprintf(APP_CSI_LOG, "%s(): img_id=%d, paddr=0x%x\n", __func__, img_id,
                rgby_paddr);
        log_index++;
    }
}

int avm_csi_start(int id)
{
    struct csi_device *dev;
    struct v4l2_device *vdev;
    void *csi_handle;

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

    hal_csi_start(csi_handle, 0xf);
    vdev->ops.s_stream(vdev, 1);

    //hal_csi_set_display_info(csi_handle, display_callback);

    dprintf(APP_CSI_LOG, "%s(): end\n\n", __func__);
    return 0;
}

void *avm_csi_get_handle(int id) {
    void *csi_handle;

    if ((id != 0)) {
        printf("%s: invalid id %u\n", __func__, id);
        return NULL;
    }

    dprintf(APP_CSI_LOG, "%s(): id=%d\n", __func__, id);
    csi_handle = g_csi[id];
    return csi_handle;
}

int avm_csi_stop(int id)
{
    struct csi_device *dev;
    struct v4l2_device *vdev;
    void *csi_handle;

    if ((id != 0)) {
        printf("%s: invalid id %u\n", __func__, id);
        return -1;
    }

    dprintf(0, "%s(): id=%d\n", __func__, id);
    csi_handle = g_csi[id];

    vdev = hal_csi_get_vdev(csi_handle);

    if (vdev == NULL) {
        printf("%s(): get vdev error\n", __func__);
        return -1;
    }
#if !CSI_BOARD_510
    max9286_vc_channel_enable(vdev, false, 0xf);
#else
    n4_vc_channel_enable(vdev, false, 0xf);
#endif
    hal_csi_stop(csi_handle, 0xf);

    vdev->ops.s_stream(vdev, 0);

    dprintf(APP_CSI_LOG, "%s(): end\n\n", __func__);
    return 0;
}

int avm_csi_close(int id)
{
    struct v4l2_device *vdev;
    void *csi_handle;

    if ((id != 0)) {
        printf("%s: invalid id %u\n", __func__, id);
        return -1;
    }

    dprintf(0, "%s(): id=%d\n", __func__, id);
    csi_handle = g_csi[id];

    vdev = hal_csi_get_vdev(csi_handle);
#if !CSI_BOARD_510
    max9286_vc_channel_enable(vdev, false, 0xf);
    max9286_deinit(vdev);
#else
    n4_vc_channel_enable(vdev, false, 0xf);
    n4_deinit(vdev);
#endif
    hal_csi_release_handle(csi_handle);
    csi_handle = NULL;

    return 0;
}
