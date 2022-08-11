/*
 * Copyright (c) 2020 Semidrive Semiconductor, Inc.
 * All rights reserved.
 */

#include <pwm_hal.h>
#include <ds90ub9xx.h>
#include <disp_hal.h>
#include <i2c_hal.h>

#include "bl_conf.h"

static struct backlight_device backlight_dev[SCREEN_MAX];

static struct backlight_board_config blbc[] = {
    {//screen 1
        0,
        RES_PWM_PWM5,
        25000,
        HAL_PWM_CHN_A_B_WORK,
        HAL_PWM_CHN_A,
        100,
        70,
        HAL_PWM_PHASE_POLARITY_NEG,
        HAL_PWM_CONTINUE_CMP,
        HAL_PWM_EDGE_ALIGN_MODE,
        false,
        RES_I2C_I2C14,
        0x1A,
        0x2C,
    },
#if (TARGET_REFERENCE_D9 || TARGET_REFERENCE_D9P)
	{//screen 2
		0,
		RES_PWM_PWM5,
		25000,
		HAL_PWM_CHN_A_B_WORK,
		HAL_PWM_CHN_A,
		100,
		70,
		HAL_PWM_PHASE_POLARITY_NEG,
		HAL_PWM_CONTINUE_CMP,
		HAL_PWM_EDGE_ALIGN_MODE,
		false,
		RES_I2C_I2C14,
		0x1A,
		0x2C,
	},
#else
    { //screen 2
        1,
        RES_PWM_PWM3,
        25000,
        HAL_PWM_CHN_A_B_WORK,
        HAL_PWM_CHN_A,
        100,
        70,
        HAL_PWM_PHASE_POLARITY_NEG,
        HAL_PWM_CONTINUE_CMP,
        HAL_PWM_EDGE_ALIGN_MODE,
        false,
        RES_I2C_I2C16,
        0x0C,
        0x2C,
    },
#endif
    {//screen 3
        2,
        RES_PWM_PWM3,
        25000,
        HAL_PWM_CHN_A_B_WORK,
        HAL_PWM_CHN_B,
        100,
        70,
        HAL_PWM_PHASE_POLARITY_NEG,
        HAL_PWM_CONTINUE_CMP,
        HAL_PWM_EDGE_ALIGN_MODE,
        false,
        RES_I2C_I2C16,
        0x1A,
        0x3C,
    },
    {//screen 4
        3,
        RES_PWM_PWM4,
        25000,
        HAL_PWM_CHN_A_B_C_D_WORK,
        HAL_PWM_CHN_D,
        100,
        70,
        HAL_PWM_PHASE_POLARITY_NEG,
        HAL_PWM_CONTINUE_CMP,
        HAL_PWM_EDGE_ALIGN_MODE,
        false,
        RES_I2C_I2C15,
        0x1A,
        0x2C,
    },
    {//screen 5
        4,
        RES_PWM_PWM5,
        25000,
        HAL_PWM_CHN_A_B_WORK,
        HAL_PWM_CHN_B,
        100,
        70,
        HAL_PWM_PHASE_POLARITY_NEG,
        HAL_PWM_CONTINUE_CMP,
        HAL_PWM_EDGE_ALIGN_MODE,
        false,
        RES_I2C_I2C15,
        0x0C,
        0x2C,
    },
};


static int panel_check_phase(struct backlight_board_config *blc)
{
    int ret = 0;
    uint8_t value;
    void *i2c_handle = NULL;
    uint32_t i2c_res = blc->i2c_res;

    ret = hal_i2c_creat_handle(&i2c_handle, i2c_res);
    if (!ret) {
        dprintf(0, "%s creat handle err! ret:%d\n", __func__, ret);
        return ret;
    }

    ds90ub9xx_write_reg(i2c_handle, blc->ser_i2c_addr, 0x03, 0x08); //I2C Pass-through Port0/Port1
    ds90ub9xx_write_reg(i2c_handle, blc->ser_i2c_addr, 0x17, 0x9e); //config I2C Pass All

    ret = ds90ub94x_read_gpio_reg(i2c_handle, blc->des_i2c_addr, GPIO6_REG);

    dprintf(SVC_BACKLIGHT_LOG, "read gpio%d -> 0x%x\n", GPIO6_REG, ret);

    hal_i2c_release_handle(i2c_handle);

    return ret;
}


static void pwm_init(struct backlight_device *bl, struct backlight_board_config *blc)
{
    hal_pwm_simple_cfg_t hal_cfg;
    int ret = 0;
    bl->resid = blc->resid;
    bl->pwm_ch = blc->pwm_ch;
    bl->pwm_grp_num = blc->grp_num;
    bl->screenid = blc->screenid;
    hal_cfg.freq = blc->freq;
    hal_cfg.grp_num = blc->grp_num;
    hal_cfg.single_mode = blc->single_mode;
    hal_cfg.align_mode = blc->align_mode;

    ret = panel_check_phase(blc);
    if (ret == 1) {
        blc->phase = HAL_PWM_PHASE_POLARITY_POS;
        dprintf(SVC_BACKLIGHT_LOG, "blc->screenid[%d] blc->phase[%d] \n", blc->screenid, blc->phase);
    }

    hal_cfg.cmp_cfg[blc->pwm_ch].phase = blc->phase;
    hal_cfg.cmp_cfg[blc->pwm_ch].duty = blc->def_duty;
    hal_pwm_creat_handle(&bl->pwm_handle, bl->resid);

    for (int i = 0; i < sizeof(blbc) / sizeof(blbc[0]); i++) {
        if ((bl->resid == blbc[i].resid) && (blbc[i].initialized)) {
            hal_cfg.cmp_cfg[blbc[i].pwm_ch].phase = blbc[i].phase;
            hal_cfg.cmp_cfg[blbc[i].pwm_ch].duty = blbc[i].def_duty;
            hal_cfg.grp_num = (blbc[i].grp_num > blc->grp_num) ? blbc[i].grp_num : blc->grp_num;
            dprintf(SVC_BACKLIGHT_LOG, "hal_cfg.cmp_cfg[blbc[%d].pwm_ch].phase = %d \n", i, hal_cfg.cmp_cfg[blbc[i].pwm_ch].phase);
            dprintf(SVC_BACKLIGHT_LOG, "hal_cfg.cmp_cfg[blbc[%d].pwm_ch].duty = %d \n", i, hal_cfg.cmp_cfg[blbc[i].pwm_ch].duty);
            dprintf(SVC_BACKLIGHT_LOG, "hal_cfg.grp_num = %d \n", hal_cfg.grp_num);
        }
    }

    if(bl->pwm_handle != NULL) {
        hal_pwm_simple_init(bl->pwm_handle, &hal_cfg);
        hal_pwm_simple_start(bl->pwm_handle);
        blc->initialized = true;
        dprintf(SVC_BACKLIGHT_LOG, "pwm bl->screenid[%d] phase[%d] inited freq=%d, duty=%d!\n", bl->screenid,
            hal_cfg.cmp_cfg[blc->pwm_ch].phase, hal_cfg.freq, hal_cfg.cmp_cfg[blc->pwm_ch].duty);
    }
}

static int pwm_duty_set(struct backlight_device *bl, u8 duty)
{

    if(duty > BACKLIGHT_PWM_MAX_DUTY || duty < 0) {
        dprintf(0, "err: backlight pwm duty set value inval! input duty = %d \n", duty);
        return 0;
    }

    hal_pwm_simple_duty_set(bl->pwm_handle, bl->pwm_ch, duty);
    dprintf(SVC_BACKLIGHT_LOG, "backlight pwm bl->screenid[%d] duty set: %d !\n", bl->screenid, duty);

    return 1;
}

static rpc_call_result_t backlight_ioctl_cb(rpc_server_handle_t hserver, rpc_call_request_t *request)
{
    rpc_call_result_t result = {0,};
    struct bl_ioctl_cmd *ctl = (struct bl_ioctl_cmd *) &request->param[0];
    struct backlight_device *bl = &backlight_dev[ctl->u.ctl.screenid];
    u32 ret = 0;

    if (!bl->binitialized)
        return result;

    result.ack = request->cmd;
    if (!request) {
        result.retcode = ERR_INVALID_ARGS;
        return result;
    }

    mutex_acquire(&bl->bl_lock);
    switch(ctl->op) {
    case BL_OP_SET_BRIGHT:
        ret = pwm_duty_set(bl, ctl->u.ctl.brightness);
        break;
    case BL_OP_GET_BRIGHT:
    default:
        break;
    }
    if (ret)
        result.retcode = 0;
    else
        result.retcode = ERR_INVALID_ARGS;
    mutex_release(&bl->bl_lock);

    dprintf(SVC_BACKLIGHT_LOG, "%s op=%d\n", __func__, ctl->op);

    return result;
}

static void backlight_rpc_server_init(void)
{
    static int rpc_server_initialized = 0;
    static rpc_server_impl_t s_blfuncs[] = {
        {MOD_RPC_REQ_BL_IOCTL, backlight_ioctl_cb, IPCC_RPC_NO_FLAGS},
    };

    if (rpc_server_initialized)
        return;

    start_ipcc_rpc_service(s_blfuncs, ARRAY_SIZE(s_blfuncs));
    rpc_server_initialized = 1;
}

int backlight_duty_set(int screenid, int duty)
{
    int ret = -1;
    struct backlight_device *bl = &backlight_dev[screenid];

    if (!bl->binitialized)
        return ret;

    if (pwm_duty_set(bl, duty))
        ret = 0;

    return ret;
}

void backlight_service_init(int displayid)
{
    struct backlight_device *bl = &backlight_dev[displayid];

    if (bl->binitialized)
        return;

    mutex_init(&bl->bl_lock);
    mutex_acquire(&bl->bl_lock);

    /* TODO: read pwm resource from system config file */
    pwm_init(bl, &blbc[displayid]);

    backlight_rpc_server_init();

    mutex_release(&bl->bl_lock);

    bl->binitialized = true;
}

static int test_backlight_duty_set(int argc, const cmd_args *argv)
{
    uint8_t duty;
    uint8_t screenid;

    if(argc != 3) {
        dprintf(0, "Input parameter number must be 3!\n");
        return -1;
    }

	screenid = argv[1].u;
	duty = argv[2].u;
	dprintf(0, "backlight pwm duty set screenid[%d] duty[%d]\n", screenid, duty);
    backlight_duty_set(screenid, duty);

    return 1;
}

#if defined(WITH_LIB_CONSOLE)
STATIC_COMMAND_START
STATIC_COMMAND("bl_duty_set", "backlight pwm duty set",  (console_cmd)&test_backlight_duty_set)
STATIC_COMMAND_END(app_backlight);
#endif

