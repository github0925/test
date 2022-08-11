#ifndef _BACKLIGH_CFG_H_
#define _BACKLIGH_CFG_H_

#include <chip_res.h>
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

#ifdef SVC_BACKLIGHT_LOG
#undef SVC_BACKLIGHT_LOG
#endif
#define SVC_BACKLIGHT_LOG 1

#define BACKLIGHT_PWM_FREQ 10000
#define BACKLIGHT_PWM_RES RES_PWM_PWM3
#define BLCKLIGHT_PWM_GRP HAL_PWM_CHN_A_B_WORK
#define BL_SCREEN_0_CH HAL_PWM_CHN_A
#define BL_SCREEN_1_CH HAL_PWM_CHN_B

#define BACKLIGHT_PWM_MAX_DUTY 100
#define BACKLIGHT_PWM_DUTY 70 //0~100

enum backlight_op_type {
    BL_OP_GET_BRIGHT,
    BL_OP_SET_BRIGHT,
};

/* Do not exceed 16 bytes so far */
struct bl_ioctl_cmd {
    u32 op;
    union {
        struct {
            u8 screenid;
            u8 brightness;
        } ctl;
        u8 data[8];
    }u;
};

struct backlight_device {
    bool binitialized;
    mutex_t bl_lock;

    /* communication level stuff */
    struct ipcc_channel *bl_agent;

    /* TODO: operation stuff */
    int screenid;
    int brightness;
    bool power;

    /* PWM */
    void *pwm_handle;
    int resid;
    int pwm_ch;
    int pwm_grp_num;
};

struct backlight_board_config {
    int screenid;
    int resid;
    unsigned long freq;
    int grp_num;
    int pwm_ch;
    int mux_duty;
    int def_duty;
    int phase;
    int single_mode;
    int align_mode;
    bool initialized;
    uint32_t i2c_res;
    uint8_t ser_i2c_addr;
    uint8_t des_i2c_addr;
};

void backlight_service_init(int displayid);
int backlight_duty_set(int screenid, int duty);

#endif //_BACKLIGH_CFG_H_
