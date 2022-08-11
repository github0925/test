/*
 * gpio-ops.c
 *
 * Copyright (c) 2020 SemiDrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */

#include <gpiod.h>
#include <stdio.h>
#include <string.h>

#include "board_diag.h"
#include "debug.h"

#define GPIOCHIP_BANK_CAP   32

#define GPIOCHIP_CNT 5
#define PIN_NUM_MIN  0
#define PIN_NUM_MAX 155

static struct gpiod_chip *gpiochip[GPIOCHIP_CNT];

void gpio_init(void)
{
    for (uint32_t i = 0; i < GPIOCHIP_CNT; i++) {
        gpiochip[i] = gpiod_chip_open_by_number(i);
    }
}

void gpio_deinit(void)
{
    for (uint32_t i = 0; i < GPIOCHIP_CNT; i++) {
        if (gpiochip[i])
            gpiod_chip_close(gpiochip[i]);

        gpiochip[i] = NULL;
    }
}

static bool is_valid_pin(uint8_t pin)
{
    return (pin <= PIN_NUM_MAX && pin >= PIN_NUM_MIN);
}

static struct gpiod_line *get_gpioline_by_pin(uint8_t pin)
{
    struct gpiod_chip *chip = NULL;

    if (!is_valid_pin(pin))
        return NULL;

    chip = gpiochip[pin / GPIOCHIP_BANK_CAP];

    if (chip)
        return gpiod_chip_get_line(chip, pin % GPIOCHIP_BANK_CAP);

    ERROR("cann't find gpiochip pin:%u\n", pin);
    return NULL;
}

bool gpio_read(uint8_t pin, uint8_t *out_val)
{
    int req;
    int val;
    bool ret = false;
    char gpio_lable[32] = {};
    struct gpiod_line *line;

    line = get_gpioline_by_pin(pin);

    if (!line) {
        ERROR("cann't get gpio line\n");
        return ret;
    }

    snprintf(gpio_lable, sizeof(gpio_lable), "%s-%u", "gpio-pin", pin);
    req = gpiod_line_request_input(line, gpio_lable);

    if (req) {
        ERROR("request line outpu fail! req:%d\n", req);
        goto out;
    }

    val = gpiod_line_get_value(line);

    if (val < 0) {
        ERROR("read input val fail!\n");
        goto out;

    }

    *out_val = !!val;
    ret = true;
out:
    gpiod_line_release(line);

    return ret;
}

bool gpio_write(uint8_t pin, uint8_t val)
{
    int req;
    bool ret = false;
    char gpio_lable[32] = {};
    struct gpiod_line *line;

    line = get_gpioline_by_pin(pin);

    if (!line) {
        ERROR("cann't get gpio line\n");
        return ret;
    }

    snprintf(gpio_lable, sizeof(gpio_lable), "%s-%u", "gpio-pin", pin);
    req = gpiod_line_request_output(line, gpio_lable, val);

    if (req) {
        ERROR("request line outpu fail! req:%d\n", req);
        goto out;
    }

    req = gpiod_line_set_value(line, val);

    if (req) {
        ERROR("outpu val:%u fail! req:%d\n",
              val, req);
        goto out;
    }

    DBG("req:%d  line:%p val:%u\n", req, line, val);

    ret = true;
out:
    gpiod_line_release(line);

    return ret;
}
