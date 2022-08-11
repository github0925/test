/*
* mipi_lt9611_to_hdmi_1920x1080_lcd.c
*
* Copyright (c) 2019 Semidrive Semiconductor.
* All rights reserved.
*
* Description:
*
* Revision History:
* -----------------
* 011, 06/28/2019 BI create this file
*/

#include "sdm_panel.h"
#include "disp_panels.h"
#include <lt9611.h>
#include <hal_port.h>
#include <hal_dio.h>
#include <thread.h>
#include <event.h>
#include "gpioirq.h"

#include "tca9539.h"

#define LCM_MIPI_LT9611_LOG 1

#define udelay(x) spin(x)
#define mdelay(x) spin(x * 1000)

#if (TARGET_REFERENCE_D9 || TARGET_REFERENCE_D9P)
#define DSI_TO_HDMI_TIMING 1080
#else
#define DSI_TO_HDMI_TIMING 720
#endif

#define LT9611_HPD_IRQ_GPIO PortConf_PIN_GPIO_D13

static event_t event_hdmi_hpd;

static uint8_t reset_timing[] = {
    5, 20, 5,
};
#if (DSI_TO_HDMI_TIMING == 1080) //1080p
static struct display_timing timing = {
    .hactive = 1920,
    .hfront_porch = 88,
    .hback_porch = 148,
    .hsync_len = 44,
    .vactive = 1080,
    .vfront_porch = 4,
    .vback_porch = 36,
    .vsync_len = 5,
};

static struct info_mipi mipi_data = {
    .burst_mode = 2,
    .video_bus_width = 24,
    .lane_number = 4,
    .phy_freq = 1069200,
};
#elif (DSI_TO_HDMI_TIMING == 1920720)//1920x720
static struct display_timing timing = {
    .hactive = 1920,
    .hfront_porch = 88,
    .hback_porch = 148,
    .hsync_len = 44,
    .vactive = 720,
    .vfront_porch = 5,
    .vback_porch = 20,
    .vsync_len = 5,
};

static struct info_mipi mipi_data = {
    .burst_mode = 2,
    .video_bus_width = 24,
    .lane_number = 4,
    .phy_freq = 693000,
};
#elif (DSI_TO_HDMI_TIMING == 720) //720p
static struct display_timing timing = {
    .hactive = 1280,
    .hfront_porch = 110,
    .hback_porch = 220,
    .hsync_len = 40,
    .vactive = 720,
    .vfront_porch = 5,
    .vback_porch = 20,
    .vsync_len = 5,
};

static struct info_mipi mipi_data = {
    .burst_mode = 2,
    .video_bus_width = 24,
    .lane_number = 4,
    .phy_freq = 693000,
};

#elif (DSI_TO_HDMI_TIMING == 576)
static struct display_timing timing = {
    .hactive = 720,
    .hfront_porch = 12,
    .hback_porch = 68,
    .hsync_len = 64,
    .vactive = 576,
    .vfront_porch = 5,
    .vback_porch = 39,
    .vsync_len = 5,
};

static struct info_mipi mipi_data = {
    .burst_mode = 2,
    .video_bus_width = 24,
    .lane_number = 4,
    .phy_freq = 693000,
};
#elif (DSI_TO_HDMI_TIMING == 480)
    static struct display_timing timing = {
        .hactive = 720,
        .hfront_porch = 16,
        .hback_porch = 60,
        .hsync_len = 62,
        .vactive = 480,
        .vfront_porch = 9,
        .vback_porch = 30,
        .vsync_len = 6,
    };

    static struct info_mipi mipi_data = {
        .burst_mode = 2,
        .video_bus_width = 24,
        .lane_number = 4,
        .phy_freq = 693000,
    };

#endif

struct lt9611_data {
    unsigned int dev_irq;
};


extern const domain_res_t g_iomuxc_res;
extern const domain_res_t g_gpio_res;

//PWR_EN2 1.8V
static void config_1_8v_power_pin(int level)
{
    struct tca9539_device *pd;
    pd = tca9539_init(12, 0x76);

    if (pd == NULL) {
        printf("init tca9359 error!\n");
        return ;
    }
    tca9539_enable_i2cpoll(pd);

    pd->ops.output_enable(pd, TCA9539_P02);
    pd->ops.output_val(pd, TCA9539_P02, level);
    tca9539_deinit(pd);
}

static void dsi_power_port_config(void)
{
    static void *dio_handle;
    bool ioret;

#if (TARGET_REFERENCE_D9 || TARGET_REFERENCE_D9P)
// PWR_1V8 EN
    ioret = hal_dio_creat_handle(&dio_handle, g_gpio_res.res_id[0]);
    if (!ioret) {
        return;
    }
    hal_dio_write_channel(dio_handle, PortConf_PIN_I2S_SC8_SCK, 1);
    dprintf(LCM_MIPI_LT9611_LOG, "%s(): config PortConf_PIN_I2S_SC8_SCK pull high for PWR_1V8 EN\n",
        __func__);
    hal_dio_release_handle(&dio_handle);
#else
    //PWR_EN1 pull high 10ms earlier than PWR_EN2.
    // PWR_EN1 3V3 port en
    ioret = hal_dio_creat_handle(&dio_handle, g_gpio_res.res_id[0]);
    if (!ioret) {
        return;
    }
    hal_dio_write_channel(dio_handle, PortConf_PIN_GPIO_C9, 1);
    hal_dio_release_handle(&dio_handle);

    //PWR_EN2 1.8V en
    //mdelay(10);
    config_1_8v_power_pin(1);
#endif
}

//RST_N
static void config_lt9611_reset_pin(int level)
{
#ifdef PROJECT_BF200
    static void *dio_handle;
    bool ioret;

    //PWR_EN1 pull high 10ms earlier than PWR_EN2.
    // PWR_EN1 3V3 port en
    ioret = hal_dio_creat_handle(&dio_handle, g_gpio_res.res_id[0]);
    if (!ioret) {
        return;
    }
    hal_dio_write_channel(dio_handle, PortConf_PIN_GPIO_D1, level);
    hal_dio_release_handle(&dio_handle);
#else
    struct tca9539_device *pd;
    dprintf(LCM_MIPI_LT9611_LOG, "%s: enter\n", __func__);
    pd = tca9539_init(12, 0x76);

    if (pd == NULL) {
        printf("init tca9359 error!\n");
        return ;
    }
    tca9539_enable_i2cpoll(pd);

    pd->ops.output_enable(pd, TCA9539_P00);
    pd->ops.output_val(pd, TCA9539_P00, level);
    tca9539_deinit(pd);
    dprintf(LCM_MIPI_LT9611_LOG, "%s: end\n", __func__);

#endif
}

static void lt9611_chip_reset(void)
{
    config_lt9611_reset_pin(0);
    //mdelay(10);
    config_lt9611_reset_pin(1);
}

static int panel_post_begin(void)
{
    //gpio config
#if PROJECT_BF200
    //for BF200 Not needed, control by hw
#else
    dsi_power_port_config();
#endif
    return 0;
}

static int lt9611_work_func(void *arg)
{
    while (1) {
        event_wait(&event_hdmi_hpd);
        LT9611_IRQ_Task();
        printf("lt9611_work_func hpd attch !\n");
        unmask_gpio_interrupt(LT9611_HPD_IRQ_GPIO);
    }
    return 0;
}

static enum handler_return lt9611_irq_handler(void *arg)
{
    mask_gpio_interrupt(LT9611_HPD_IRQ_GPIO);
    event_signal(&event_hdmi_hpd, false);
    printf("lt9611 LT9611_HPD_IRQ_GPIO(D13(77) irq handler attch !\n");
    return INT_RESCHEDULE;
}

static thread_t *lt9611_thread;
static int panel_post_end(void)
{
    int ret;
    printf("%s enter\n", __func__);

    //config lt9611
    const uint8_t i2c_addr = 0x39;
#if (TARGET_REFERENCE_D9 || TARGET_REFERENCE_D9P)
    lt9611_i2c_init(RES_I2C_I2C11, i2c_addr);
#else
    lt9611_i2c_init(RES_I2C_I2C16, i2c_addr);
#endif
    //LT9611_Reset
    lt9611_chip_reset();

    //LT9611 init
    ret = LT9611_Init();
    if (ret < 0) {
        printf("%s(): lt9611 init falied \n", __func__);
        return -1;
    }

    //HPD deal thread
    event_init(&event_hdmi_hpd, false, EVENT_FLAG_AUTOUNSIGNAL);
    lt9611_thread = thread_create("lt9611", lt9611_work_func, NULL, DEFAULT_PRIORITY, DEFAULT_STACK_SIZE);
    thread_detach_and_resume(lt9611_thread);

    //register hpd gpio(PortConf_PIN_GPIO_D13(77)) irq
    register_gpio_int_handler(LT9611_HPD_IRQ_GPIO, IRQ_TYPE_EDGE_FALLING,
                              lt9611_irq_handler, NULL);
    unmask_gpio_interrupt(LT9611_HPD_IRQ_GPIO);
    dprintf(LCM_MIPI_LT9611_LOG, "register lt9611 irq\n");
    printf("%s down\n", __func__);

    return 0;
}

struct sdm_panel mipi_lt9611_to_hdmi_1920x1080_lcd = {
    .panel_name = "mipi_lt9611_to_hdmi_1920x1080_lcd",
    .if_type = IF_TYPE_DSI,
    .cmd_intf = IF_TYPE_NONE, // or IF_TYPE_I2C
    .width_mm = 240,
    .height_mm = 160,
    .fps = 60,
    .reset_timing = reset_timing,
    .display_timing = &timing,
    .mipi = &mipi_data,
    .sis_num = 0,
    .sis = NULL,
    .rtos_screen = NULL,
    .panel_post_begin = panel_post_begin,
    .panel_post_end = panel_post_end,
};

