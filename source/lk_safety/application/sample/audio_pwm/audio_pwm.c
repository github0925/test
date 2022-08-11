/*
 * Copyright (c) 2019 Semidrive Semiconductor.
 * All rights reserved.
 *
 */

#include <app.h>
#include <debug.h>
#include <kernel/event.h>
#include <kernel/thread.h>
#include <lib/console.h>
#include <string.h>

#include <chip_res.h>
#include <hal_port.h>
#include <pwm_hal.h>
#include <res.h>
#include <res_loader.h>
#include <heap.h>
// #define PCM_USE_H
#ifdef PCM_USE_H
#include "sweep.h"
#endif
// Config on X9_REF board
#define APP_AUDIO_DEBUG_LEVEL ALWAYS
#define APP_AUDIO_REPEAT_NUM 5
#define APP_DATA_TEST_LEN 16
#define AUDIO_SOURCE_1_SIZE 0xd040
#define AUDIO_SOURCE_1_OFFSET 0x0
#define AUDIO_SOURCE_2_SIZE 0x4860
#define AUDIO_SOURCE_2_OFFSET 0xd040

uint8_t fade_out[4 * 1024] = {};
uint8_t fade_in[16 * 1024] = {};

typedef struct {
    uint32_t res;
    Port_PinType pin;
    uint8_t Alt;
} pwm_port_cfg_t;

static void *app_pcm_pwm_handle;
static uint32_t app_audio_repeat_cnt = 0;
static void *port_handle;
static event_t play_complete_event;
static bool app_audio_inited = false;

uint8_t *play_back_data_p = NULL;
int32_t play_back_data_len = 0;
#define PWM_PORT_CFG_NUM 5

const pwm_port_cfg_t pwm_port_cfg[PWM_PORT_CFG_NUM] = {
    {RES_PWM_PWM1, PortConf_PIN_GPIO_A10, PORT_PIN_MODE_ALT4},
    {RES_PWM_PWM3, PortConf_PIN_GPIO_C8, PORT_PIN_MODE_ALT6},
    {RES_PWM_PWM3, PortConf_PIN_GPIO_C9, PORT_PIN_MODE_ALT6},
    {RES_PWM_PWM3, PortConf_PIN_GPIO_C10, PORT_PIN_MODE_ALT6},
    {RES_PWM_PWM3, PortConf_PIN_GPIO_C11, PORT_PIN_MODE_ALT6},
};

extern const domain_res_t g_iomuxc_res;

static Port_PinModeType PIN_MODE_PWM = {
    ((uint32_t)PORT_PAD_POE__ENABLE | PORT_PAD_IS__OUT | PORT_PAD_SR__FAST |
     PORT_PAD_DS__MID1 | PORT_PIN_IN_PULL_UP),
    ((uint32_t)PORT_PIN_MUX_FV__MIN | PORT_PIN_MUX_FIN__MIN |
     PORT_PIN_OUT_PUSHPULL /*| PORT_PIN_MODE_ALT4*/),
};

static void audio_pwm_port_set(uint32_t pwm_res)
{
    uint32_t i;
    Port_PinModeType pin_mode;

    hal_port_creat_handle(&port_handle, g_iomuxc_res.res_id[0]);
    for (i = 0; i < PWM_PORT_CFG_NUM; i++) {
        if (pwm_res == pwm_port_cfg[i].res) {
            pin_mode = PIN_MODE_PWM;
            pin_mode.pin_mux_config |= pwm_port_cfg[i].Alt;
            hal_port_set_pin_mode(port_handle, pwm_port_cfg[i].pin, pin_mode);
        }
    }
    hal_port_release_handle(&port_handle);
}

static void audio_pwm_cmp_int_cbk(void *arg)
{
    pwm_instance_t *pwm_inst = (pwm_instance_t *)arg;
    dprintf(APP_AUDIO_DEBUG_LEVEL, "PCM PWM %d cmp!\n", pwm_inst->phy_num);
}

static void audio_pwm_play_complete_cbk(void *arg)
{
    pwm_instance_t *pwm_inst = (pwm_instance_t *)arg;
    dprintf(APP_AUDIO_DEBUG_LEVEL, "PCM PWM %d play cpmplete!\n",
            pwm_inst->phy_num);
    event_signal(&play_complete_event, false);
}

static int audio_pwm_play_thread(void *arg)
{
    while (1) {
        event_wait(&play_complete_event);
        dprintf(APP_AUDIO_DEBUG_LEVEL, "%s\n", __func__);
        app_audio_repeat_cnt++;
        pwm_instance_t *pwm_inst = (pwm_instance_t *)app_pcm_pwm_handle;
        if (app_audio_repeat_cnt < APP_AUDIO_REPEAT_NUM - 1) {
            hal_pwm_pcm_play_start(pwm_inst, (uint8_t *)play_back_data_p,
                                   play_back_data_len); // 16000 8
        } else if (app_audio_repeat_cnt == APP_AUDIO_REPEAT_NUM - 1) {
            hal_pwm_pcm_play_start(pwm_inst, (uint8_t *)fade_out,
                                   sizeof(fade_out) /
                                       sizeof(uint8_t)); // 16000 8
        } else {
            app_audio_repeat_cnt = APP_AUDIO_REPEAT_NUM;
            if (play_back_data_p) {
                free(play_back_data_p);
                play_back_data_p = NULL;
                play_back_data_len = 0;
            }
        }
    }

    return 0;
}

static void init_fade_in_data(void)
{
    int i = 0;
    uint8_t value = 0x1;
    for (i = 0; i < 128; i++) {
        memset(fade_in + i * 128, value++, 128);
    }
}
static void init_fade_out_data(void)
{
    int i = 0;
    uint8_t value = 0X80;
    for (i = 0; i < 128; i++) {
        memset(fade_out + i * 32, value--, 32);
    }
}
static void audio_pwm_play_test_init(int argc, const cmd_args *argv)
{
    hal_pwm_pcm_cfg_t hal_cfg;
    uint32_t pwm_res;
    uint32_t sample_rate;
    uint32_t data_bits;

    if (app_audio_inited) {
        dprintf(APP_AUDIO_DEBUG_LEVEL, "pcm pwm have already inited\n");
        return;
    }

    if (argc != 4) {
        dprintf(APP_AUDIO_DEBUG_LEVEL, "argc must be 4\n");
        return;
    }

    if ((argv[1].u > 8) || (argv[1].u == 0)) {
        dprintf(APP_AUDIO_DEBUG_LEVEL, "argv[1] must be 1-8\n");
        return;
    }

    if (argv[1].u == 1)
        pwm_res = RES_PWM_PWM1;
    if (argv[1].u == 2)
        pwm_res = RES_PWM_PWM2;
    if (argv[1].u == 3)
        pwm_res = RES_PWM_PWM3;
    if (argv[1].u == 4)
        pwm_res = RES_PWM_PWM4;
    if (argv[1].u == 5)
        pwm_res = RES_PWM_PWM5;
    if (argv[1].u == 6)
        pwm_res = RES_PWM_PWM6;
    if (argv[1].u == 7)
        pwm_res = RES_PWM_PWM7;
    if (argv[1].u == 8)
        pwm_res = RES_PWM_PWM8;

    sample_rate = argv[2].u;
    data_bits = argv[3].u;

    app_audio_repeat_cnt = 0;
    app_audio_inited = true;

    audio_pwm_port_set(pwm_res);

    event_init(&play_complete_event, false, EVENT_FLAG_AUTOUNSIGNAL);

    thread_t *t = thread_create("pcmpwm_play_test", audio_pwm_play_thread, NULL,
                                DEFAULT_PRIORITY, DEFAULT_STACK_SIZE);
    thread_resume(t);

    hal_pwm_creat_handle(&app_pcm_pwm_handle, pwm_res);

    if (app_pcm_pwm_handle != NULL) {
        hal_cfg.sample_freq = sample_rate;
        hal_cfg.data_bits = data_bits;
        hal_cfg.drive_mode = HAL_PWM_PCM_DRIVE_MONO_CHANNEL;
        hal_cfg.play_complete_cbk = audio_pwm_play_complete_cbk;
        hal_pwm_pcm_init(app_pcm_pwm_handle, &hal_cfg);
        pwm_instance_t *pwm_inst = (pwm_instance_t *)app_pcm_pwm_handle;
        init_fade_in_data();
        init_fade_out_data();
        dprintf(APP_AUDIO_DEBUG_LEVEL,
                "PCM PWM%d inited, sample_freq:%d data_bits:%d!\n",
                pwm_inst->phy_num, hal_cfg.sample_freq, hal_cfg.data_bits);
    } else {
        dprintf(ALWAYS, "%s() creat handle fail\n", __func__);
    }
}

static int audio_pwm_play_test_start(int argc, const cmd_args *argv)
{
    pwm_instance_t *pwm_inst = (pwm_instance_t *)app_pcm_pwm_handle;
    if (argc != 2) {
        dprintf(APP_AUDIO_DEBUG_LEVEL, "Input parameter number must be 2!\n");
        return -1;
    }
    hal_pwm_pcm_play_start(pwm_inst, (uint8_t *)fade_in,
                           sizeof(fade_in) / sizeof(uint8_t)); // 16000 8
    app_audio_repeat_cnt = 0;
    play_back_data_len = 0;
    if (play_back_data_p) {
        free(play_back_data_p);
        play_back_data_p = NULL;
    }
    switch (argv[1].u) {
    case 1:
#ifdef PCM_USE_H
        play_back_data_p = seat_belt;
        play_back_data_len = sizeof(seat_belt) / sizeof(uint8_t);
#else
        play_back_data_len = AUDIO_SOURCE_1_SIZE;
        play_back_data_p = (uint8_t *)memalign(32, play_back_data_len);
        res_load("res", play_back_data_p, AUDIO_SOURCE_1_SIZE,
                 AUDIO_SOURCE_1_OFFSET);
#endif
        break;
    case 2:
#ifdef PCM_USE_H
        play_back_data_p = turn_signal;
        play_back_data_len = sizeof(turn_signal) / sizeof(uint8_t);
#else
        play_back_data_len = AUDIO_SOURCE_2_SIZE;
        play_back_data_p = (uint8_t *)memalign(32, play_back_data_len);
        res_load("res", play_back_data_p, AUDIO_SOURCE_2_SIZE,
                 AUDIO_SOURCE_2_OFFSET);
#endif
        break;
    default:
        break;
    }

#if 0
    hal_pwm_int_enable(app_pcm_pwm_handle, HAL_PWM_INT_SRC_CMP_EVENT);
    hal_pwm_int_cbk_register(app_pcm_pwm_handle, HAL_PWM_INT_SRC_CMP_EVENT, audio_pwm_cmp_int_cbk);
    register_int_handler(pwm_inst->irq_num, hal_pwm_irq_handle, app_pcm_pwm_handle);
    unmask_interrupt(pwm_inst->irq_num);
#endif

    return 1;
}

static int audio_pwm_play_test_pause(int argc, const cmd_args *argv)
{
    pwm_instance_t *pwm_inst = (pwm_instance_t *)app_pcm_pwm_handle;
    dprintf(APP_AUDIO_DEBUG_LEVEL, "PCM PWM %d play pause!\n",
            pwm_inst->phy_num);
    hal_pwm_pcm_play_pause(app_pcm_pwm_handle);
    return 1;
}

static int audio_pwm_play_test_resume(int argc, const cmd_args *argv)
{
    pwm_instance_t *pwm_inst = (pwm_instance_t *)app_pcm_pwm_handle;
    dprintf(APP_AUDIO_DEBUG_LEVEL, "PCM PWM %d play resume!\n",
            pwm_inst->phy_num);
    hal_pwm_pcm_play_resume(app_pcm_pwm_handle);
    return 1;
}

#if defined(WITH_LIB_CONSOLE)
STATIC_COMMAND_START
STATIC_COMMAND("pcm_pwm_init", "audio pcm pwm init play test",
               (console_cmd)&audio_pwm_play_test_init)
STATIC_COMMAND("pcm_pwm_play", "audio pcm pwm start play test",
               (console_cmd)&audio_pwm_play_test_start)
STATIC_COMMAND("pcm_pwm_pause", "audio pcm pwm pause play test",
               (console_cmd)&audio_pwm_play_test_pause)
STATIC_COMMAND("pcm_pwm_resume", "audio pcm pwm resume play test",
               (console_cmd)&audio_pwm_play_test_resume)
STATIC_COMMAND_END(audio_pwm_test);
#endif

APP_START(audio_pwm).flags = 0,
    //.init = audio_pwm_play_test_init,
    APP_END
