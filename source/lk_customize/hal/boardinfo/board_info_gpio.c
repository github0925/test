/*
 * Copyright (c) 2019 Semidrive Inc.
 *
 * Board distinguished by PIN level.
 */

#include <assert.h>
#include <debug.h>
#include "hal_port.h"
#include "hal_dio.h"
#include "boardinfo_hwid_hw.h"
#include "boardinfo_hwid_usr.h"
#include "board_info_gpio.h"

#define PIN_MODE(dir, pull, out_manner, mode)   {((uint32_t)PORT_PAD_POE__DISABLE | \
                                                        PORT_PAD_IS__##dir | \
                                                        PORT_PAD_SR__FAST | \
                                                        PORT_PAD_DS__MID1 | \
                                                        PORT_PIN_IN_##pull ), \
                                                ((uint32_t)PORT_PIN_MUX_FV__MIN | \
                                                        PORT_PIN_MUX_FIN__MIN | \
                                                        PORT_PIN_OUT_##out_manner | \
                                                        PORT_PIN_MODE_##mode)}

struct board_info_pin {
    void (*set_hwid)(struct sd_hwid_stor *hwid, uint32_t val);
    uint8_t num;
    struct {
        Port_PinType pin;
        Port_PinModeType mode;
        Port_PinDirectionType dir;
        Port_PinModeType restore_mode;
        Port_PinDirectionType restore_dir;
    } pin_cfg[];
};

extern const domain_res_t g_iomuxc_res;
extern const domain_res_t g_gpio_res;

#if TARGET_REFERENCE_G9 || TARGET_REFERENCE_G9Q
static void g9xref_set_hwid(struct sd_hwid_stor *hwid, uint32_t val)
{
    hwid->magic = 5;
    hwid->ver = 1;
    hwid->v.v1.board_type = BOARD_TYPE_REF;
    hwid->v.v1.board_id_major = BOARDID_MAJOR_G9A;
    hwid->v.v1.board_id_minor = val ? 2 : 3;
}

static struct board_info_pin g_board_info_pin = {
    .set_hwid = g9xref_set_hwid,
    .num = 1,
    .pin_cfg = {
        {
            .pin = PortConf_PIN_GPIO_B4,
            .mode = PIN_MODE(IN, PULL_UP, PUSHPULL, GPIO),
            .dir = PORT_PIN_IN,
            .restore_mode = PIN_MODE(OUT, PULL_DOWN, PUSHPULL, GPIO),
            .restore_dir = PORT_PIN_OUT
        }
    }
};
#else
#error Not support board info in GPIO.
#endif

static int init(storage_device_t *storage_dev, uint32_t res_idx,
                void *config)
{
    struct board_info_pin *pin =
                    (struct board_info_pin *)storage_dev->priv;
    void *port_handle;

    hal_port_creat_handle(&port_handle, g_iomuxc_res.res_id[0]);
    ASSERT(port_handle);

    for (int i = 0; i < pin->num; i++) {
        hal_port_set_pin_mode(port_handle, pin->pin_cfg[i].pin,
                              pin->pin_cfg[i].mode);
	hal_port_set_pin_direction(port_handle, pin->pin_cfg[i].pin,
                              pin->pin_cfg[i].dir);
    }

    hal_port_release_handle(port_handle);

    return 0;
}

static int read(storage_device_t *storage_dev, uint64_t src,
                uint8_t *dst, uint64_t data_len)
{
    struct board_info_pin *pin =
                    (struct board_info_pin *)storage_dev->priv;
    void *dio_handle;
    uint32_t val = 0;
    struct sd_hwid_stor hwid = {0};

    hal_dio_creat_handle(&dio_handle, g_gpio_res.res_id[0]);
    ASSERT(dio_handle);

    for (int i = 0; i < pin->num; i++) {
        val <<= 1;
        val |= hal_dio_read_channel(dio_handle, pin->pin_cfg[i].pin);
    }

    hal_dio_release_handle(dio_handle);

    dprintf(INFO, "%s: read gpio val %d\n", __func__, val);
    pin->set_hwid(&hwid, val);
    if (dst) {
        memcpy(dst, &hwid, data_len);
        return 0;
    }

    return -1;
}

static int write(storage_device_t *storage_dev, uint64_t offset,
                 const uint8_t *buf, uint64_t data_len)
{
    /* Write not supported. */
    return -1;
}

static int release(storage_device_t *storage_dev)
{
    struct board_info_pin *pin =
                    (struct board_info_pin *)storage_dev->priv;
    void *port_handle;

    hal_port_creat_handle(&port_handle, g_iomuxc_res.res_id[0]);
    ASSERT(port_handle);

    for (int i = 0; i < pin->num; i++) {
        hal_port_set_pin_mode(port_handle, pin->pin_cfg[i].pin,
                              pin->pin_cfg[i].restore_mode);
        hal_port_set_pin_direction(port_handle, pin->pin_cfg[i].pin,
                              pin->pin_cfg[i].restore_dir);
    }

    hal_port_release_handle(port_handle);

    return 0;
}

static storage_device_t g_board_info_pin_dev = {
    .priv = &g_board_info_pin,
    .init = init,
    .read = read,
    .write = write,
    .release = release
};

storage_device_t *get_board_info_pin_dev(int index)
{
    if (index)
        return NULL;

    return &g_board_info_pin_dev;
}
