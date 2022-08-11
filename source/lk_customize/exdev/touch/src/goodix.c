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
#include <kernel/thread.h>
#include <kernel/event.h>
#include <lib/bytes.h>
#include <lib/reg.h>
#include <thread.h>
#include <event.h>
#include <i2c_hal.h>
#include <hal_port.h>
#include <tca9539.h>

#include <safe_ts.h>
#include "touch_driver.h"
#include "boardinfo_hwid_usr.h"
#include "hal_dio.h"
#include "gpioirq.h"
#include "serdes_9xx.h"

#define GOODIX_CONTACT_SIZE         8
#define GOODIX_MAX_CONTACTS         10
#define GOODIX_CFG_NAME             "sts_cfg.bin"
#define GOODIX_CONFIG_MAX_LENGTH    240
#define GOODIX_CONFIG_911_LENGTH    186
#define GOODIX_CONFIG_967_LENGTH    228
#define GOODIX_VENDOR_ID            0x0416
#define GTP_READ_COOR_ADDR          0x814E
#define GOODIX_REG_ID               0x8140

#define GPIO_IRQ 0

static int gdx_dbg_flag = 0;
extern const domain_res_t g_iomuxc_res;

/* Define vendor spcific touchscreen data */
struct goodix_ts_data {
    u16 id;
    u16 version;
    u16 vendor;
    u16 instance;
    int cfg_len;
    const char *cfg_data;
    unsigned int dev_irq;
    void *i2c_handle;
    event_t event;
    const struct ts_board_config *conf;
    struct safe_ts_device *safe_ts_dev;
};

#if GPIO_IRQ
struct goodix_ts_data *g_goodix[TS_DO_MAX];
static bool use_gpio_flag;
static bool core_board;
static event_t event_gtc;
static int irq_mask;

static void config_gpio_irq(struct goodix_ts_data *goodix)
{
    u32 val;

    if (gdx_dbg_flag) {
        dprintf(INFO, "readl(0xf0002a40)=%#x, readl(0xf0002640)=%#x\n",
                readl(0xf0002a40), readl(0xf0002640));
    }

    if (goodix->conf->ts_domain_support == TS_SUPPORT_ANRDOID_MAIN) {
        writel(0x10000, 0xf0002a40);
        writel(0x10000, 0xf0002640);
        g_goodix[TS_DO_ANRDOID_MAIN] = goodix;
    }
    else if (goodix->conf->ts_domain_support == TS_SUPPORT_ANRDOID_AUX1) {
        val = readl(0xf0002a40);
        val |= 1 << 17;
        writel(val, 0xf0002a40);
        val = readl(0xf0002640);
        val |= 1 << 17;
        writel(val, 0xf0002640);
        g_goodix[TS_DO_ANRDOID_AUX1] = goodix;
    }
    else if (goodix->conf->ts_domain_support == TS_SUPPORT_CTRLPANEL_MAIN) {
        if (core_board) {
            val = readl(0xf0002a40);
            val |= 1 << 2;
            writel(val, 0xf0002a40);
            val = readl(0xf0002640);
            val |= 1 << 2;
            writel(val, 0xf0002640);
        }
        else {
            val = readl(0xf0002a40);
            val |= 1 << 3;
            writel(val, 0xf0002a40);
            val = readl(0xf0002640);
            val |= 1 << 3;
            writel(val, 0xf0002640);
        }

        g_goodix[TS_DO_CTRLPANEL_MAIN] = goodix;
    }
    else if (goodix->conf->ts_domain_support == (TS_SUPPORT_ANRDOID_MAIN |
             TS_SUPPORT_CTRLPANEL_MAIN)) {
        writel(0x10000, 0xf0002a40);
        writel(0x10000, 0xf0002640);
        g_goodix[TS_DO_ANRDOID_MAIN] = goodix;
        g_goodix[TS_DO_CTRLPANEL_MAIN] = goodix;
    }
}

static void mask_gpio_irq(void)
{
    u32 val = 0;
    val = readl(0xf0003040);

    if (gdx_dbg_flag) {
        dprintf(0, "%s: val=%#x\n", __func__, val);
    }

    if (val & (0x1 << 16)) {
        writel(0x10000, 0xf0003440);
        irq_mask |= 1 << TS_DO_ANRDOID_MAIN;
    }

    if (val & (0x1 << 17)) {
        writel(0x20000, 0xf0003440);
        irq_mask |= 1 << TS_DO_ANRDOID_AUX1;
    }

    if (core_board) {
        if (val & (0x1 << 2)) {
            writel(0x4, 0xf0003440);
            irq_mask |= 1 << TS_DO_CTRLPANEL_MAIN;
        }
    }
    else {
        if (val & (0x1 << 3)) {
            writel(0x8, 0xf0003440);
            irq_mask |= 1 << TS_DO_CTRLPANEL_MAIN;
        }
    }
}
#endif

static int gtp_i2c_read(struct goodix_ts_data *goodix, u8 *buf, int len)
{
    int ret = 0;
    ret = hal_i2c_read_reg_data(goodix->i2c_handle, goodix->conf->i2c_addr,
                                buf, 2, buf + 2, len - 2);

    if (ret < 0) {
        dprintf(CRITICAL, "%s: read error, ret=%d.\n", __func__, ret);
        return ret;
    }

    return 0;
}

static int gtp_i2c_write(struct goodix_ts_data *goodix, u8 *buf, int len)
{
    int ret = 0;
    ret = hal_i2c_write_reg_data(goodix->i2c_handle, goodix->conf->i2c_addr,
                                 buf, 2, buf + 2, len - 2);

    if (ret < 0) {
        dprintf(CRITICAL, "%s: transmit error, ret=%d.\n", __func__, ret);
        return ret;
    }

    return ret;
}

static int goodix_get_cfg_len(u16 id)
{
    switch (id) {
        case 911:
        case 9271:
        case 9110:
        case 927:
        case 928:
            return GOODIX_CONFIG_911_LENGTH;

        case 912:
        case 967:
            return GOODIX_CONFIG_967_LENGTH;

        default:
            return GOODIX_CONFIG_MAX_LENGTH;
    }
}

static int goodix_firmware_upgrade(struct goodix_ts_data *goodix)
{
    goodix->cfg_len = goodix_get_cfg_len(goodix->id);
    //goodix->cfg_data = malloc(goodix->cfg_len);
    return 0;
}

static int goodix_read_version(struct goodix_ts_data *goodix)
{
    u8 data[8] = {GOODIX_REG_ID >> 8, GOODIX_REG_ID & 0xFF};
    char id_str[5];
    int id, version;
    gtp_i2c_read(goodix, data, 8);

    if (gdx_dbg_flag) {
        dprintf(ALWAYS, "%s(): instance(%#x), %#x, %#x, %#x, %#x, %#x, %#x.\n",
            __func__, goodix->instance, data[2], data[3], data[4], data[5], data[6], data[7]);
    }

    memcpy(id_str, data + 2, 4);
    id = atoui(id_str);
    version = data[6] + (data[7] << 8);

    if (id == 0)
        return -1;
    else {
        goodix->id = id;
        goodix->version = version;
        goodix->vendor = GOODIX_VENDOR_ID;
    }

    if (gdx_dbg_flag) {
        dprintf(ALWAYS, "%s(): id:%#x, version:%#x, instance=%#x\n", __func__, id,
                version, goodix->instance);
    }

    return 0;
}

static int goodix_reset_device(struct goodix_ts_data *goodix)
{
    struct tca9539_device *pd = NULL;

    if (goodix->conf->port_config.enable) {
        pd = tca9539_init(goodix->conf->port_config.port_i2c_bus,
                          goodix->conf->port_config.port_addr);

        if (pd == NULL) {
            dprintf(ALWAYS, "%s, init tca9359 error!\n", __func__);
            return -1;
        }

        pd->ops.output_enable(pd, goodix->conf->port_config.port_id);
        pd->ops.output_val(pd, goodix->conf->port_config.port_id, 0);
        thread_sleep(10);
        pd->ops.output_val(pd, goodix->conf->port_config.port_id, 1);
    }

    if (goodix->conf->serdes_config.enable) {
        uint16_t ser_addr = goodix->conf->serdes_config.ser_addr;
        uint16_t des_addr = goodix->conf->serdes_config.des_addr;
        uint16_t irq_channel = goodix->conf->serdes_config.irq_channel;
        uint16_t reset_channel = goodix->conf->serdes_config.reset_channel;

        if (du90ub948_gpio_output(goodix->i2c_handle, des_addr, reset_channel, 0))
            return -1;

        du90ub948_gpio_output(goodix->i2c_handle, des_addr, irq_channel, 0);
        du90ub948_gpio_output(goodix->i2c_handle, des_addr,irq_channel,
            (goodix->conf->i2c_addr == 0x14));

        thread_sleep(1);
        du90ub948_gpio_output(goodix->i2c_handle, des_addr, reset_channel, 1);
        thread_sleep(5);
        du90ub948_gpio_output(goodix->i2c_handle, des_addr, irq_channel, 0);
        thread_sleep(50);
        du90ub948_gpio_input(goodix->i2c_handle, des_addr, irq_channel);
        du90ub941_or_947_gpio_input(goodix->i2c_handle, ser_addr, irq_channel);
    }

    return 0;
}

static int goodix_ts_read_input_report(struct goodix_ts_data *goodix,
                                       u8 *data)
{
    int count = 0;
    int touch_num = 0;
    u8 end_cmd[3] = {GTP_READ_COOR_ADDR >> 8, GTP_READ_COOR_ADDR & 0xFF, 0};
    /* store first finger point */
    u8 point_data[11] = {GTP_READ_COOR_ADDR >> 8, GTP_READ_COOR_ADDR & 0xFF};
    /* store second~tenth finger point, if do have */
    u8 point_data1[74] = {GTP_READ_COOR_ADDR >> 8, (GTP_READ_COOR_ADDR + 1 + GOODIX_CONTACT_SIZE) & 0xFF};

    if (!goodix)
        return -1;

    while (count < 10) {
        gtp_i2c_read(goodix, point_data, 11);

        if (point_data[2] & (0x1 << 7)) {
            touch_num = point_data[2] & 0xf;
            memcpy(data, point_data + 2, 9);

            if (touch_num > 1) {
                gtp_i2c_read(goodix, point_data1, 2 + GOODIX_CONTACT_SIZE * (touch_num - 1));
                memcpy(data + 9, point_data1 + 2, GOODIX_CONTACT_SIZE * (touch_num - 1));
            }

            gtp_i2c_write(goodix, end_cmd, 3);
            return touch_num;
        }

        count++;
        thread_sleep(2);
    }

    gtp_i2c_write(goodix, end_cmd, 3);
    return 0;
}

static enum handler_return goodix_irq_handler(void *arg)
{
    struct goodix_ts_data *goodix = arg;

#if GPIO_IRQ
    mask_interrupt(goodix->dev_irq);
    mask_gpio_irq();
    event_signal(&event_gtc, false);
#else
    mask_gpio_interrupt(goodix->dev_irq);
    event_signal(&goodix->event, false);
#endif
    return INT_RESCHEDULE;
}

#if GPIO_IRQ
static int goodix_ts_work_func(void *arg)
{
    u8 pdata[81] = {0,};
    int len, touch_num;
    struct goodix_ts_data *goodix = arg;
    struct touch_report_data report_data;

    while (1) {
        event_wait(&event_gtc);

        for (int i = 0; i < TS_DO_MAX; i++) {
            if (irq_mask & 1 << i) {
                irq_mask &= ~(1 << i);
                touch_num = goodix_ts_read_input_report(g_goodix[i], pdata);

                if (gdx_dbg_flag) {
                    dprintf(ALWAYS, "%s: [%d] 0x%x, (0x%x, 0x%x, 0x%x, 0x%x)\n", __func__,
                            g_goodix[i]->instance, pdata[0], pdata[3], pdata[2], pdata[5], pdata[4]);
                }

                if ((touch_num < 0) || (pdata[0] == 0)) {
                    continue;
                }

                report_data.key_value = pdata[0] & (1 << 4);
                report_data.touch_num = touch_num;

                for (int j = 0; j < touch_num; j++) {
                    report_data.coord_data[j].id = *((u8 *)(pdata + 1 + GOODIX_CONTACT_SIZE * j));
                    report_data.coord_data[j].x = *((u16 *)(pdata + 2 + GOODIX_CONTACT_SIZE * j));
                    report_data.coord_data[j].y = *((u16 *)(pdata + 4 + GOODIX_CONTACT_SIZE * j));
                    report_data.coord_data[j].w = *((u16 *)(pdata + 6 + GOODIX_CONTACT_SIZE * j));
                }

                len = 2 + touch_num * TS_COORD_METADATA_SIZE;
                safe_ts_report_data(g_goodix[i]->safe_ts_dev, &report_data, len);
            }
        }

        unmask_interrupt(goodix->dev_irq);
    }

    return 0;
}
#else
static int goodix_ts_work_func(void *arg)
{
    struct goodix_ts_data *goodix = arg;
    u8 pdata[81] = {0,};
    int len, touch_num;
    struct touch_report_data report_data;

    while (1) {
        event_wait(&goodix->event);
        touch_num = goodix_ts_read_input_report(goodix, pdata);

        if (gdx_dbg_flag) {
            dprintf(ALWAYS, "%s: [%d] 0x%x, (0x%x, 0x%x, 0x%x, 0x%x)\n", __func__,
                    goodix->instance, pdata[0], pdata[3], pdata[2], pdata[5], pdata[4]);
        }

        if ((touch_num < 0) || (pdata[0] == 0)) {
            unmask_gpio_interrupt(goodix->dev_irq);
            continue;
        }

        report_data.key_value = pdata[0] & (1 << 4);
        report_data.touch_num = touch_num;

        for (int j = 0; j < touch_num; j++) {
            report_data.coord_data[j].id = *((u8 *)(pdata + 1 + GOODIX_CONTACT_SIZE * j));
            report_data.coord_data[j].x = *((u16 *)(pdata + 2 + GOODIX_CONTACT_SIZE * j));
            report_data.coord_data[j].y = *((u16 *)(pdata + 4 + GOODIX_CONTACT_SIZE * j));
            report_data.coord_data[j].w = *((u16 *)(pdata + 6 + GOODIX_CONTACT_SIZE * j));
        }

        len = 2 + touch_num * TS_COORD_METADATA_SIZE;
        safe_ts_report_data(goodix->safe_ts_dev, &report_data, len);
        unmask_gpio_interrupt(goodix->dev_irq);
    }

    return 0;
}
#endif

static int goodix_config_device(struct goodix_ts_data *goodix)
{
#if GPIO_IRQ
    if (!use_gpio_flag) {
        if (get_part_id(PART_BOARD_TYPE) == BOARD_TYPE_MS)
            core_board = true;

        event_init(&event_gtc, false, EVENT_FLAG_AUTOUNSIGNAL);
        thread_t *tp_thread = thread_create("gt9xx_thread", goodix_ts_work_func,
                                            goodix, DEFAULT_PRIORITY, DEFAULT_STACK_SIZE);
        thread_detach_and_resume(tp_thread);
        register_int_handler(goodix->dev_irq, goodix_irq_handler, goodix);
        unmask_interrupt(goodix->dev_irq);
        use_gpio_flag = true;
    }

    config_gpio_irq(goodix);
#else
    event_init(&goodix->event, false, EVENT_FLAG_AUTOUNSIGNAL);
    thread_t *tp_thread = thread_create("gt9xx_thread", goodix_ts_work_func,
                                        goodix, DEFAULT_PRIORITY, DEFAULT_STACK_SIZE);
    thread_detach_and_resume(tp_thread);
    register_gpio_int_handler(goodix->dev_irq, IRQ_TYPE_EDGE_FALLING, goodix_irq_handler, goodix);
    unmask_gpio_interrupt(goodix->dev_irq);
#endif
    return 0;
}

static int goodix_set_inited(void *arg)
{
    //struct goodix_ts_data *goodix = arg;
    return 0;
}

static int goodix_probe_device(const struct ts_board_config *conf)
{
    int ret;
    struct goodix_ts_data *goodix = NULL;
    goodix = malloc(sizeof(struct goodix_ts_data));

    if (!goodix) {
        dprintf(CRITICAL, "%s, alloc mem fail\n", __func__);
        return -1;
    }

    memset(goodix, 0, sizeof(struct goodix_ts_data));
    goodix->conf = conf;
    goodix->instance = goodix->conf->ts_domain_support;

#if GPIO_IRQ
    goodix->dev_irq = GPIO1_GPIO_INT4_NUM;
#else
    goodix->dev_irq = goodix->conf->irq_pin.pin_num;
#endif

    if (!hal_i2c_creat_handle(&goodix->i2c_handle, goodix->conf->res_id)) {
        dprintf(CRITICAL, "%s, i2c handle fail,instance=%#x\n", __func__,
                goodix->instance);
        goto err;
    }

    if (goodix->conf->serdes_config.enable) {
        ret = du90ub941_or_947_enable_port(goodix->i2c_handle,
            goodix->conf->serdes_config.ser_addr, goodix->conf->serdes_config.serdes_type);
        if (ret) {
            dprintf(ALWAYS, "%s, enable port fail,instance=%#x\n", __func__,
                    goodix->instance);
            goto err1;
        }

        ret = du90ub941_or_947_enable_i2c_passthrough(goodix->i2c_handle,
            goodix->conf->serdes_config.ser_addr);
        if (ret) {
            dprintf(ALWAYS, "%s, enable i2c pass fail,instance=%#x\n", __func__,
                    goodix->instance);
            goto err1;
        }
    }

    int count = 0;

    do {
        goodix_reset_device(goodix);

        if (!goodix_read_version(goodix))
            break;
    } while (++count < 3);

    if (count >= 3) {
        dprintf(CRITICAL, "%s: read version fail, instance=%#x\n", __func__,
                goodix->instance);
        goto err1;
    }

    ret = goodix_firmware_upgrade(goodix);
    if (ret) {
        dprintf(CRITICAL, "%s: updata firmware fail, instance=%#x\n", __func__,
                goodix->instance);
        goto err1;
    }

    ret = goodix_config_device(goodix);
    if (ret) {
        dprintf(CRITICAL, "%s:config device fail, instance=%#x\n", __func__,
                goodix->instance);
        goto err1;
    }

    goodix->safe_ts_dev = safe_ts_alloc_device();
    if (!goodix->safe_ts_dev) {
        dprintf(CRITICAL, "%s: safe_ts_alloc_device fail, instance=%#x\n",
                __func__, goodix->instance);
        goto err;
    }

    goodix->safe_ts_dev->instance = goodix->conf->ts_domain_support;
    goodix->safe_ts_dev->screen_id = goodix->conf->screen_id;
    goodix->safe_ts_dev->vinfo.id = goodix->id;
    goodix->safe_ts_dev->vinfo.version = goodix->version;
    goodix->safe_ts_dev->vinfo.vendor = goodix->vendor;
    goodix->safe_ts_dev->vinfo.name = goodix->conf->device_name;
    goodix->safe_ts_dev->cinfo.max_touch_num = goodix->conf->coord_config.max_touch_num;
    goodix->safe_ts_dev->cinfo.swapped_x_y = goodix->conf->coord_config.swapped_x_y;
    goodix->safe_ts_dev->cinfo.inverted_x = goodix->conf->coord_config.inverted_x;
    goodix->safe_ts_dev->cinfo.inverted_y = goodix->conf->coord_config.inverted_y;
    goodix->safe_ts_dev->set_inited = goodix_set_inited;
    goodix->safe_ts_dev->vendor_priv = goodix;

    ret = safe_ts_register_device(goodix->safe_ts_dev);
    if (ret) {
        safe_ts_delete_device(goodix->safe_ts_dev);
        dprintf(CRITICAL, "%s: safe_ts_register_device fail, instance=%#x\n",
                __func__, goodix->instance);
        goto err;
    }

    //dprintf(ALWAYS, "%s, instance=%#x, done ok\n", __func__, goodix->instance);
    return 0;

err1:
    hal_i2c_release_handle(goodix->i2c_handle);
err:
    free(goodix);
    return -1;
}

static struct touch_driver goodix_driver = {
    "goodix",
    goodix_probe_device,
};

register_touch_driver(goodix_driver);

#if defined(WITH_LIB_CONSOLE)
#include <lib/console.h>
static int gdx_dbg_en(int argc, const cmd_args *argv)
{
    gdx_dbg_flag = atoui(argv[1].str);
    dprintf(ALWAYS, "gdx_dbg_flag=%d\n", gdx_dbg_flag);
    return 0;
}

STATIC_COMMAND_START STATIC_COMMAND("gdx_dbg_en", "gdx_dbg_en [0/1]",
                                    (console_cmd)&gdx_dbg_en)
STATIC_COMMAND_END(goodix);
#endif

