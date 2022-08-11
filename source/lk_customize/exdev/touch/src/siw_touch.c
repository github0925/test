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

#define IIC_CODE_START 0x24
#define IICCMD_GET_EVENTNUM 0xA6
#define IICCMD_GET_EVENT 0xA7

#define TS_EVENT_UNKNOWN    0x00
#define TS_EVENT_PRESS      0x01
#define TS_EVENT_MOVE       0x02
#define TS_EVENT_RELEASE    0x03

#define SIW_META_SIZE 8

static int siw_dbg_flag = 0;

/* Define vendor spcific touchscreen data */
struct siw_ts_data {
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

static int siw_i2c_read(struct siw_ts_data *siw_touch, u8 *buf, int len)
{
    int ret = 0;
    ret = hal_i2c_read_reg_data(siw_touch->i2c_handle,
                                siw_touch->conf->i2c_addr, buf, 2, buf + 2, len - 2);

    if (ret < 0) {
        dprintf(CRITICAL, "%s: read error, ret=%d.\n", __func__, ret);
        return ret;
    }

    return 0;
}

static int siw_i2c_write(struct siw_ts_data *siw_touch, u8 *buf, int len)
{
    int ret = 0;
    ret = hal_i2c_write_reg_data(siw_touch->i2c_handle,
                                 siw_touch->conf->i2c_addr, buf, 2, buf + 2, len - 2);

    if (ret < 0) {
        dprintf(CRITICAL, "%s: transmit error, ret=%d.\n", __func__, ret);
        return ret;
    }

    return ret;
}

static int siw_read_version(struct siw_ts_data *siw_touch)
{
    u8 data[42] = {0x24, 0xd0};
    int ret;
    ret = siw_i2c_read(siw_touch, data, 42);

    if (ret)
        return ret;

    if (siw_dbg_flag) {
        for (int i = 0; i < 4; i++)
            dprintf(ALWAYS,
                    "%s(): instance(%#x), %#x, %#x, %#x, %#x, %#x, %#x, %#x, %#x.\n",
                    __func__, siw_touch->instance,
                    data[i * 8 + 0], data[i * 8 + 1], data[i * 8 + 2], data[i * 8 + 3],
                    data[i * 8 + 4], data[i * 8 + 5], data[i * 8 + 6], data[i * 8 + 7]);
    }

    siw_touch->id = 0xaa;
    siw_touch->version = 0xaa;
    siw_touch->vendor = 0xaa;
    return 0;
}

static int siw_reset_device(struct siw_ts_data *siw_touch)
{
    if (siw_touch->conf->port_config.enable) {
        struct tca9539_device *pd = NULL;
        pd = tca9539_init(siw_touch->conf->port_config.port_i2c_bus,
                          siw_touch->conf->port_config.port_addr);

        if (pd == NULL) {
            dprintf(ALWAYS, "%s, init tca9359 error!\n", __func__);
            return -1;
        }

        pd->ops.output_enable(pd, siw_touch->conf->port_config.port_id);
        pd->ops.output_val(pd, siw_touch->conf->port_config.port_id, 0);
        thread_sleep(20);
        pd->ops.output_val(pd, siw_touch->conf->port_config.port_id, 1);
    }

    if (siw_touch->conf->serdes_config.enable) {
        uint16_t ser_addr = siw_touch->conf->serdes_config.ser_addr;
        uint16_t des_addr = siw_touch->conf->serdes_config.des_addr;
        uint16_t irq_channel = siw_touch->conf->serdes_config.irq_channel;
        uint16_t reset_channel = siw_touch->conf->serdes_config.reset_channel;

        if (du90ub948_gpio_output(siw_touch->i2c_handle, des_addr, reset_channel, 1))
            return -1;

        thread_sleep(10);
        du90ub948_gpio_output(siw_touch->i2c_handle, des_addr, reset_channel, 0);
        thread_sleep(10);
        du90ub948_gpio_output(siw_touch->i2c_handle, des_addr, reset_channel, 1);
        thread_sleep(100);
        du90ub948_gpio_input(siw_touch->i2c_handle, des_addr, irq_channel);
        du90ub941_or_947_gpio_input(siw_touch->i2c_handle, ser_addr, irq_channel);
    }

    return 0;
}

static int siw_ts_read_input_report(struct siw_ts_data *siw_touch, u8 *data)
{
    u8 cmd1[4] = {IIC_CODE_START, IICCMD_GET_EVENTNUM};
    u8 rcv[84] = {IIC_CODE_START, IICCMD_GET_EVENT};
    u8 *point;
    u8 nFingers = 0;
    int ret = -1;
    int cnt;

    ret = siw_i2c_read(siw_touch, cmd1, 4);
    if (ret < 0) {
        dprintf(ALWAYS, "%s, read failed: %d\n", __func__, ret);
        return ret;
    }

    nFingers = cmd1[2];

    if (nFingers > siw_touch->conf->coord_config.max_touch_num) {
        dprintf(ALWAYS, "%s, nFingers=%d invalid\n", __func__, nFingers);
        return -1;
    }
    else if (nFingers == 0) {
        dprintf(INFO, "%s, nFingers=%d\n", __func__, nFingers);
        return 0;
    }
    else {
        cnt = SIW_META_SIZE * nFingers;
        siw_i2c_read(siw_touch, rcv, cnt + 4);
        memcpy(data, rcv + 4, cnt);
    }

    point = rcv + 4;

    for (cnt = 0; cnt < nFingers; cnt++) {
        if (point[SIW_META_SIZE * cnt + 1] == TS_EVENT_PRESS
                || point[SIW_META_SIZE * cnt + 1] == TS_EVENT_MOVE)
            return nFingers;
    }

    return 0;
}

static int siw_ts_work_func(void *arg)
{
    struct siw_ts_data *siw_touch = arg;
    u8 pdata[80] = {0,};
    int len, touch_num;
    struct touch_report_data report_data;

    while (1) {
        event_wait(&siw_touch->event);
        touch_num = siw_ts_read_input_report(siw_touch, pdata);

        if (siw_dbg_flag) {
            dprintf(ALWAYS, "%s: [%d] %#x, %#x (%#x, %#x, %#x, %#x)\n", __func__,
                    siw_touch->instance, pdata[0], pdata[1], pdata[3], pdata[2], pdata[5], pdata[4]);
        }

        if (touch_num < 0) {
            unmask_gpio_interrupt(siw_touch->dev_irq);
            continue;
        }

        report_data.key_value = 0;
        report_data.touch_num = touch_num;

        for (int j = 0; j < touch_num; j++) {
            report_data.coord_data[j].id = *((u8 *)(pdata + 0 + SIW_META_SIZE * j));
            report_data.coord_data[j].x = *((u16 *)(pdata + 2 + SIW_META_SIZE * j));
            report_data.coord_data[j].y = *((u16 *)(pdata + 4 + SIW_META_SIZE * j));
            report_data.coord_data[j].w = *((u16 *)(pdata + 6 + SIW_META_SIZE * j));
            report_data.coord_data[j].x =
                report_data.coord_data[j].x * siw_touch->conf->coord_config.x_max / 32767;
            report_data.coord_data[j].x =
                siw_touch->conf->coord_config.x_max - report_data.coord_data[j].x;
            report_data.coord_data[j].y =
                report_data.coord_data[j].y * siw_touch->conf->coord_config.y_max / 32767;
            report_data.coord_data[j].y =
                siw_touch->conf->coord_config.y_max - report_data.coord_data[j].y;
        }

        len = 2 + touch_num * TS_COORD_METADATA_SIZE;
        safe_ts_report_data(siw_touch->safe_ts_dev, &report_data, len);
        unmask_gpio_interrupt(siw_touch->dev_irq);
    }

    return 0;
}

static enum handler_return siw_irq_handler(void *arg)
{
    struct siw_ts_data *siw_touch = arg;
    mask_gpio_interrupt(siw_touch->dev_irq);
    event_signal(&siw_touch->event, false);
    return INT_RESCHEDULE;
}

static int siw_config_device(struct siw_ts_data *siw_touch)
{
    event_init(&siw_touch->event, false, EVENT_FLAG_AUTOUNSIGNAL);
    thread_t *tp_thread = thread_create("siw_thread", siw_ts_work_func,
                                        siw_touch, DEFAULT_PRIORITY, DEFAULT_STACK_SIZE);
    thread_detach_and_resume(tp_thread);
    register_gpio_int_handler(siw_touch->dev_irq, IRQ_TYPE_EDGE_FALLING,
                              siw_irq_handler, siw_touch);
    unmask_gpio_interrupt(siw_touch->dev_irq);
    return 0;
}

static int siw_set_inited(void *arg)
{
    return 0;
}

static int siw_probe_device(const struct ts_board_config *conf)
{
    int ret;
    struct siw_ts_data *siw_touch = NULL;

    siw_touch = malloc(sizeof(struct siw_ts_data));
    if (!siw_touch) {
        dprintf(CRITICAL, "%s, alloc mem fail\n", __func__);
        return -1;
    }

    memset(siw_touch, 0, sizeof(struct siw_ts_data));
    siw_touch->conf = conf;
    siw_touch->instance = siw_touch->conf->ts_domain_support;
    siw_touch->dev_irq = siw_touch->conf->irq_pin.pin_num;

    if (!hal_i2c_creat_handle(&siw_touch->i2c_handle, siw_touch->conf->res_id)) {
        dprintf(CRITICAL, "%s, i2c handle fail,instance=%#x\n", __func__,
                siw_touch->instance);
        goto err;
    }

    if (siw_touch->conf->serdes_config.enable) {
        ret = du90ub941_or_947_enable_port(siw_touch->i2c_handle,
                                           siw_touch->conf->serdes_config.ser_addr,
                                           siw_touch->conf->serdes_config.serdes_type);

        if (ret) {
            dprintf(ALWAYS, "%s, enable port fail,instance=%#x\n", __func__,
                    siw_touch->instance);
            goto err1;
        }

        ret = du90ub941_or_947_enable_i2c_passthrough(siw_touch->i2c_handle,
                siw_touch->conf->serdes_config.ser_addr);

        if (ret) {
            dprintf(ALWAYS, "%s, enable i2c pass fail,instance=%#x\n", __func__,
                    siw_touch->instance);
            goto err1;
        }
    }

    int count = 0;

    do {
        siw_reset_device(siw_touch);

        if (!siw_read_version(siw_touch))
            break;
    } while (++count < 3);

    if (count >= 3) {
        dprintf(CRITICAL, "%s: read version fail, instance=%#x\n", __func__,
                siw_touch->instance);
        goto err1;
    }

    ret = siw_config_device(siw_touch);

    if (ret) {
        dprintf(CRITICAL, "%s:config device fail, instance=%#x\n", __func__,
                siw_touch->instance);
        goto err1;
    }

    siw_touch->safe_ts_dev = safe_ts_alloc_device();

    if (!siw_touch->safe_ts_dev) {
        dprintf(CRITICAL, "%s: safe_ts_alloc_device fail, instance=%#x\n",
                __func__, siw_touch->instance);
        goto err;
    }

    siw_touch->safe_ts_dev->instance = siw_touch->conf->ts_domain_support;
    siw_touch->safe_ts_dev->screen_id = siw_touch->conf->screen_id;
    siw_touch->safe_ts_dev->vinfo.id = siw_touch->id;
    siw_touch->safe_ts_dev->vinfo.version = siw_touch->version;
    siw_touch->safe_ts_dev->vinfo.vendor = siw_touch->vendor;
    siw_touch->safe_ts_dev->vinfo.name = siw_touch->conf->device_name;
    siw_touch->safe_ts_dev->cinfo.max_touch_num = siw_touch->conf->coord_config.max_touch_num;
    siw_touch->safe_ts_dev->cinfo.swapped_x_y = siw_touch->conf->coord_config.swapped_x_y;
    siw_touch->safe_ts_dev->cinfo.inverted_x = siw_touch->conf->coord_config.inverted_x;
    siw_touch->safe_ts_dev->cinfo.inverted_y = siw_touch->conf->coord_config.inverted_y;
    siw_touch->safe_ts_dev->set_inited = siw_set_inited;
    siw_touch->safe_ts_dev->vendor_priv = siw_touch;

    ret = safe_ts_register_device(siw_touch->safe_ts_dev);
    if (ret) {
        safe_ts_delete_device(siw_touch->safe_ts_dev);
        dprintf(CRITICAL, "%s: safe_ts_register_device fail, instance=%#x\n",
                __func__, siw_touch->instance);
        goto err;
    }

    dprintf(ALWAYS, "%s, instance=%#x, done ok\n", __func__, siw_touch->instance);
    return 0;
err1:
    hal_i2c_release_handle(siw_touch->i2c_handle);
err:
    free(siw_touch);
    return -1;
}

static struct touch_driver siw_driver = {
    "siw_touch",
    siw_probe_device,
};

register_touch_driver(siw_driver);

#if defined(WITH_LIB_CONSOLE)
#include <lib/console.h>
static int siw_dbg_en(int argc, const cmd_args *argv)
{
    siw_dbg_flag = atoui(argv[1].str);
    dprintf(ALWAYS, "siw_dbg_flag=%d\n", siw_dbg_flag);
    return 0;
}

STATIC_COMMAND_START STATIC_COMMAND("siw_dbg_en", "siw_dbg_en [0/1]",
                                    (console_cmd)&siw_dbg_en)
STATIC_COMMAND_END(siw_touch);
#endif

