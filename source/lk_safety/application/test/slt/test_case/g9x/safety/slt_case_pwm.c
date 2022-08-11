/*
 * Copyright (c) 2019, SemiDrive, Inc. All rights reserved
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include <stdlib.h>
#include <assert.h>
#include <bits.h>
#include <debug.h>
#include <stdio.h>
#include <err.h>
#include <lib/console.h>
#include <lib/bytes.h>
#include <lib/reg.h>
#include <dcf.h>
#include <app.h>
#include <thread.h>
#include <event.h>
#include <dcf.h>
#include <pwm_hal.h>
#include <lib/slt_module_test.h>

#define PWM_DUTY 51

typedef enum {
    PWM1 = RES_PWM_PWM1,
    PWM2 = RES_PWM_PWM2,
    PWM_MAX = 2
} pwm_ip_e;

typedef struct {
    bool *result;
    uint8_t channel;
    int resid;
    uint32_t irq_num;
    void *handle;
    hal_pwm_int_func_cbk pwm_cbk;
} pwm_device_t;

typedef struct {
    mutex_t mutex;
    pwm_device_t *dev;
} pwm_contex_t;

static bool pwm1_result = false;
static bool pwm2_result = false;

void slt_pwm1_ovf_cbk(void *arg)
{
    pwm1_result = true;
    dprintf(ALWAYS, "slt_pwm1 ovf cbk sucessfully!\n");
}

void slt_pwm2_ovf_cbk(void *arg)
{
    pwm2_result = true;
    dprintf(ALWAYS, "slt_pwm2 ovf cbk sucessfully!\n");
}

static void slt_pwm_duty_set(pwm_device_t *pwm, u8 duty)
{
    hal_pwm_simple_duty_set(pwm->handle, HAL_PWM_CHN_A, duty);
}

static int slt_pwm_init(pwm_contex_t *pwm_contex)
{
    int ret = -1;
    hal_pwm_simple_cfg_t hal_cfg;
    pwm_device_t *pwm = pwm_contex->dev;

    if (pwm == NULL) {
        dprintf(ALWAYS, "pwm handle is null\n");
        goto out;
    }

    mutex_acquire(&pwm_contex->mutex);
    hal_pwm_creat_handle(&pwm->handle, pwm->resid);
    hal_pwm_int_cbk_register(pwm->handle, HAL_PWM_INT_SRC_CNT_G0_OVF, pwm->pwm_cbk);
    register_int_handler(pwm->irq_num, hal_pwm_irq_handle, pwm->handle);
    hal_pwm_int_enable(pwm->handle, HAL_PWM_INT_SRC_CNT_G0_OVF);
    unmask_interrupt(pwm->irq_num);

    hal_cfg.freq = 10000;
    hal_cfg.grp_num = HAL_PWM_CHN_A_WORK;
    hal_cfg.single_mode = HAL_PWM_ONE_SHOT_CMP;
    hal_cfg.align_mode = HAL_PWM_EDGE_ALIGN_MODE;
    hal_cfg.cmp_cfg[HAL_PWM_CHN_A].phase = HAL_PWM_PHASE_POLARITY_POS;
    hal_cfg.cmp_cfg[HAL_PWM_CHN_A].duty = PWM_DUTY;

    if (pwm->handle != NULL) {
        hal_pwm_simple_init(pwm->handle, &hal_cfg);
        hal_pwm_simple_start(pwm->handle);
    }
    else {
        dprintf(ALWAYS, "hal_pwm_creat_handle fail\n");
        mutex_release(&pwm_contex->mutex);
        goto out;
    }

    mutex_release(&pwm_contex->mutex);

    ret = 1;
out:
    return ret;
}

static int slt_pwm_checkout_result(pwm_contex_t *pwm_contex)
{
    int ret = -1;
    mutex_acquire(&pwm_contex->mutex);
    ret =  *pwm_contex->dev->result ? 0 : -1;
    mutex_release(&pwm_contex->mutex);

    return ret;
}

int TEST_SAFE_SS_11(uint times, uint timeout, char *result_string)
{
    int ret = -1;
    pwm_contex_t pwm_contex;
    pwm_device_t pwm_dev[PWM_MAX] = {
        [0] = {
            .result = &pwm1_result,
            .channel = 1,
            .resid = PWM1,
            .pwm_cbk = slt_pwm1_ovf_cbk,
            .irq_num = PWM1_PWM_IRQ_NUM,
        },
        [1] = {
            .result = &pwm2_result,
            .channel = 2,
            .resid = PWM2,
            .pwm_cbk = slt_pwm2_ovf_cbk,
            .irq_num = PWM2_PWM_IRQ_NUM,
        }
    };

    mutex_init(&pwm_contex.mutex);

    /* TODO: read pwm resource from system config file */
    for (uint8_t idx = 0; idx < PWM_MAX; idx++) {
        pwm_contex.dev = &pwm_dev[idx];

        if (slt_pwm_init(&pwm_contex) < 0) {
            dprintf(ALWAYS, "slt_pwm_init fail\n");
            goto out;
        }
    }

    thread_sleep(5);

    for (uint8_t idx = 0; idx < PWM_MAX; idx++) {
        pwm_contex.dev = &pwm_dev[idx];

        if (slt_pwm_checkout_result(&pwm_contex) < 0) {
            dprintf(ALWAYS, "slt_pwm%d check fail\n", pwm_dev[idx].channel);
            goto out;
        }
    }

    ret = 0;
out:

    for (uint8_t idx = 0; idx < 1; idx++) {
        if (pwm_dev[idx].handle != NULL) {
            hal_pwm_int_disable(pwm_dev[idx].handle, HAL_PWM_INT_SRC_CNT_G0_OVF);
            mask_interrupt(pwm_dev[idx].irq_num);
            hal_pwm_release_handle(pwm_dev[idx].handle);
        }
    }

    *pwm_dev[0].result = false;
    *pwm_dev[1].result = false;

    return ret;
}

SLT_MODULE_TEST_HOOK_DETAIL(safe_ss_11, TEST_SAFE_SS_11,
                            SLT_MODULE_TEST_LEVEL_SAMPLE_1, 0, 1, 5000, 0, 0);
