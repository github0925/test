/*
 * func_cpt.c
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: ii4 Test App.
 *
 * Revision History:
 * -----------------
 */
#include <string.h>
#include <timer_hal.h>
#undef HAL_ASSERT_PARAMETER
#include <semaphore.h>
#include "board_start.h"
#include "func_can.h"
#include "remote_test.h"

//24M - 1 HZ
#define CPT_DBG       ALWAYS
#define CPT_TMR_FREQ  24000000
#define CPT_TMR_DIV   0

#define SAMPLING_CHANNEL    HAL_TIMER_FUNC_CH_A
#define S2US               (1000*1000)
#define FIFO_WML    8

#define get_read_timers_value(x) (x < ARRAY_SIZE(cpt_chn_table) ? (x++):(x=0))

typedef struct {
    semaphore_t sem;
    uint32_t ovf_cnt;
    uint32_t *p;
    uint32_t *base;
    uint32_t  items;
} cpt_result_t;

typedef struct {
    hal_timer_func_ch_t channel;
    hal_timer_sub_t     ticker;
    cpt_result_t        res;
    uint32_t            items;
} icu_instance_t;

typedef struct {
    uint32_t timer_res_id;
    void *icu_timer_handle;
    icu_instance_t inst[HAL_TIMER_FUNC_CH_TOTAL];
} icu_ctrl_block_t;

struct {
    float freq;
    float duty1;
    float duty2;
} final_res[(FIFO_WML * 10 - 2) / 2];

static icu_ctrl_block_t ICU = {
    .timer_res_id = RES_TIMER_TIMER2,
    .inst =
    {
        {
            .channel = HAL_TIMER_FUNC_CH_A,
            .ticker = HAL_TIMER_LOCAL_A,
        },
        {
            .channel = HAL_TIMER_FUNC_CH_B,
            .ticker = HAL_TIMER_LOCAL_B,
        },
        {
            .channel = HAL_TIMER_FUNC_CH_C,
            .ticker = HAL_TIMER_LOCAL_C,
        },
        {
            .channel = HAL_TIMER_FUNC_CH_D,
            .ticker = HAL_TIMER_LOCAL_D,
        },
    },
};

static void icu_result_init(icu_ctrl_block_t *icu, hal_timer_func_ch_t channel,
                            void *buf, uint32_t items)
{
    icu->inst[channel].res.base = buf;
    icu->inst[channel].res.p = buf;
    icu->inst[channel].res.items = items;
    icu->inst[channel].res.ovf_cnt = 0;

    sem_init(&icu->inst[channel].res.sem, 0);
}

static enum handler_return icu_toggle_event_cbk_core(icu_ctrl_block_t *icu,
        hal_timer_func_ch_t channel)
{
    if (hal_timer_cpt_get_fifo_items_num(icu->icu_timer_handle,
                                         channel) >= FIFO_WML) {
        for (int i = 0; i < FIFO_WML; i++) {
            *icu->inst[channel].res.p = hal_timer_timer_cpt_value_get(icu->icu_timer_handle,
                                        channel);
            icu->inst[channel].res.p++;
        }

        if (icu->inst[channel].res.p >= icu->inst[channel].res.base +
                icu->inst[channel].res.items) {
            icu->inst[channel].res.p = icu->inst[channel].res.base;
            sem_post(&icu->inst[channel].res.sem, false);
        }
    }

    return 0;
}

static enum handler_return icu_toggle_event_cbk_A(void)
{
    return icu_toggle_event_cbk_core(&ICU, HAL_TIMER_FUNC_CH_A);
}
static enum handler_return icu_toggle_event_cbk_B(void)
{
    return icu_toggle_event_cbk_core(&ICU, HAL_TIMER_FUNC_CH_B);
}
static enum handler_return icu_toggle_event_cbk_C(void)
{
    return icu_toggle_event_cbk_core(&ICU, HAL_TIMER_FUNC_CH_C);
}
static enum handler_return icu_toggle_event_cbk_D(void)
{
    return icu_toggle_event_cbk_core(&ICU, HAL_TIMER_FUNC_CH_D);
}

static enum handler_return cpt_ovf_chA_int_cbk(void)
{
    ICU.inst[HAL_TIMER_FUNC_CH_A].res.ovf_cnt++;

    return 0;
}

static enum handler_return cpt_ovf_chB_int_cbk(void)
{
    ICU.inst[HAL_TIMER_FUNC_CH_B].res.ovf_cnt++;

    return 0;
}

static enum handler_return cpt_ovf_chC_int_cbk(void)
{
    ICU.inst[HAL_TIMER_FUNC_CH_C].res.ovf_cnt++;

    return 0;
}

static enum handler_return cpt_ovf_chD_int_cbk(void)
{
    ICU.inst[HAL_TIMER_FUNC_CH_D].res.ovf_cnt++;

    return 0;
}

static bool icu_start_capture(icu_ctrl_block_t *icu,
                              hal_timer_func_ch_t channel)
{
    hal_timer_int_src_t int_src = 0;
    bool level = 0;

    switch (channel) {
        case HAL_TIMER_FUNC_CH_A:
            int_src = HAL_TIMER_CPT_A_INT_SRC;
            break;

        case HAL_TIMER_FUNC_CH_B:
            int_src = HAL_TIMER_CPT_B_INT_SRC;
            break;

        case HAL_TIMER_FUNC_CH_C:
            int_src = HAL_TIMER_CPT_C_INT_SRC;
            break;

        case HAL_TIMER_FUNC_CH_D:
            int_src = HAL_TIMER_CPT_D_INT_SRC;
            break;

        default:
            break;
    }

    hal_timer_int_src_enable(icu->icu_timer_handle, int_src);
    hal_timer_func_cpt_enable(icu->icu_timer_handle, channel);

    return level;
}

static void cpt_timer_init(icu_ctrl_block_t *icu)
{
    hal_timer_glb_cfg_t glb_cfg;
    hal_timer_ovf_cfg_t ovf_cfg;
    hal_timer_fun_cfg_t fun_cfg;

    hal_timer_creat_handle(&icu->icu_timer_handle, icu->timer_res_id);

    if (icu->icu_timer_handle == NULL) {
        dprintf(ALWAYS, "%s() create handle fail!\n", __func__);
        return;
    }

    glb_cfg.cascade = false;
    glb_cfg.clk_frq = CPT_TMR_FREQ;
    glb_cfg.clk_div = CPT_TMR_DIV;
    glb_cfg.clk_sel = HAL_TIMER_SEL_LF_CLK;

    hal_timer_global_init(icu->icu_timer_handle, &glb_cfg);

    ovf_cfg.periodic = true;
    ovf_cfg.cnt_val = 0;
    ovf_cfg.ovf_val = 0xFFFFFFFF;

    fun_cfg.func.dma_ctrl.dma_enable = false;

    fun_cfg.func.func_type = HAL_TIMER_FUNC_TYPE_CPT; //select cpature sub function
    fun_cfg.func.sub_func.cpt_cfg.cpt_cnt_sel =
        HAL_TIMER_CPT_CNT_LOCAL; //use local channel timer
    fun_cfg.func.sub_func.cpt_cfg.trig_mode =
        HAL_TIMER_CPT_TOGGLE_EDGE; //select edge toggle trigger
    fun_cfg.func.sub_func.cpt_cfg.dual_cpt_mode = 0; //disable dual mode
    fun_cfg.func.sub_func.cpt_cfg.single_mode = 0;
    fun_cfg.func.sub_func.cpt_cfg.filter_dis = 1;
    fun_cfg.func.sub_func.cpt_cfg.filter_width = 0;
    fun_cfg.func.sub_func.cpt_cfg.first_cpt_rst_en = 0;

    for (uint32_t i = 0; i < HAL_TIMER_FUNC_CH_TOTAL; i++) {
        hal_timer_ovf_init(icu->icu_timer_handle, icu->inst[i].ticker, &ovf_cfg);
        hal_timer_func_init(icu->icu_timer_handle, icu->inst[i].channel, &fun_cfg);
        hal_timer_func_cpt_disable(icu->icu_timer_handle, icu->inst[i].ticker);
    }

    hal_timer_int_cbk_register(icu->icu_timer_handle, HAL_TIMER_CNT_LA_OVF_INT_SRC,
                               cpt_ovf_chA_int_cbk);
    hal_timer_int_cbk_register(icu->icu_timer_handle, HAL_TIMER_CNT_LB_OVF_INT_SRC,
                               cpt_ovf_chB_int_cbk);
    hal_timer_int_cbk_register(icu->icu_timer_handle, HAL_TIMER_CNT_LC_OVF_INT_SRC,
                               cpt_ovf_chC_int_cbk);
    hal_timer_int_cbk_register(icu->icu_timer_handle, HAL_TIMER_CNT_LD_OVF_INT_SRC,
                               cpt_ovf_chD_int_cbk);

    hal_timer_int_cbk_register(icu->icu_timer_handle, HAL_TIMER_CPT_A_INT_SRC,
                               icu_toggle_event_cbk_A);
    hal_timer_int_cbk_register(icu->icu_timer_handle, HAL_TIMER_CPT_B_INT_SRC,
                               icu_toggle_event_cbk_B);
    hal_timer_int_cbk_register(icu->icu_timer_handle, HAL_TIMER_CPT_C_INT_SRC,
                               icu_toggle_event_cbk_C);
    hal_timer_int_cbk_register(icu->icu_timer_handle, HAL_TIMER_CPT_D_INT_SRC,
                               icu_toggle_event_cbk_D);

    hal_timer_int_src_enable(icu->icu_timer_handle, HAL_TIMER_CNT_LA_OVF_INT_SRC);
    hal_timer_int_src_enable(icu->icu_timer_handle, HAL_TIMER_CNT_LB_OVF_INT_SRC);
    hal_timer_int_src_enable(icu->icu_timer_handle, HAL_TIMER_CNT_LC_OVF_INT_SRC);
    hal_timer_int_src_enable(icu->icu_timer_handle, HAL_TIMER_CNT_LD_OVF_INT_SRC);

    dprintf(debug_show_dg, "%s()\n", __func__);
}

/**
 * @brief Thread to handle cpt tests.
 */
int cpt_start_thread(void *arg)
{

    uint32_t ring_buffer[FIFO_WML * 10]; //10 times of gathering
    uint32_t parse[FIFO_WML * 10];
    uint32_t period = 0;
    uint32_t duty1 = 0;
    uint32_t duty2 = 0;

    cpt_timer_init(&ICU);

    icu_result_init(&ICU, SAMPLING_CHANNEL, ring_buffer, FIFO_WML * 10);

    icu_start_capture(&ICU, SAMPLING_CHANNEL);

    while (1) {
        thread_sleep(100);

        sem_wait(&ICU.inst[SAMPLING_CHANNEL].res.sem);

        memcpy(parse, ring_buffer, sizeof(parse));

        for (int i = 1; i < FIFO_WML * 10 - 1; i += 2) {
            if (parse[i + 1] < parse[i - 1]) {
                period =  0xFFFFFFFF - parse[i - 1] + parse[i + 1];
            }
            else {
                period = parse[i + 1] - parse[i - 1];
            }

            if (parse[i] < parse[i - 1]) {
                duty1 = 0xFFFFFFFF - parse[i - 1] + parse[i];
            }
            else {
                duty1 = parse[i] - parse[i - 1];
            }

            duty2 = period - duty1;

            final_res[i / 2].freq = S2US / (hal_timer_cntr_to_us(ICU.icu_timer_handle,
                                            period * 10000) / 10000.0);
            final_res[i / 2].duty1 = (duty1 * 100) / period;
            final_res[i / 2].duty2 = (duty2 * 100) / period;
            //dprintf(debug_show_dg, "freq :%f duty1 %f duty2 %f\n", final_res[i/2].freq,final_res[i/2].duty1,final_res[i/2].duty2);
        }
    }
}

const static com_chn_table_t cpt_chn_table[] = {
    {0x02, CPT_CHN_2},
};

static bool _capt_single_read(board_test_exec_t *exec)
{
    bool ret = false;

    can_cmd_t *capt_cmd = (can_cmd_t *)exec->cmd;

    if (capt_cmd->dev_id == g_step_case_table[CAPT_SERIAL_ID].cmd_id) {

        for (uint8_t num = 0; num < ARRAY_SIZE(cpt_chn_table); num++) {

            if (capt_cmd->route_channel_id != cpt_chn_table[num].pin_num) {
                continue;
            }
            else {
                remote_test_send_req(capt_cmd);

                if (remote_test_wait_resp(xTIME_OUT_TICKS, exec) == true) {
                    if (exec->resp[0] == NORMAL_DEAL) {
                        set_para_value(ret, true);
                    }
                }
            }
        }
    }

    return ret;
}

static bool _capt_period_read(board_test_exec_t *exec)
{
    bool ret = false;

    set_para_value(exec->cmd[0], g_step_case_table[CAPT_SERIAL_ID].cmd_id);
    set_para_value(exec->cmd[1], CPT_CHN_2);
    set_para_value(exec->peridic_resp_id, PERIODIC_RESP_CAPT);

    ret = _capt_single_read(exec);
    return ret;
}

bool board_capt_reply_deal(board_test_exec_t *exec, board_test_state_e state)
{
    bool ret = false;
    uint8_t cmdStatus = CMD_PARA_ERR;

    if (state == STATE_SINGLE) {
        ret = _capt_single_read(exec);
    }
    else if (state == STATE_PERIODIC) {
        ret = _capt_period_read(exec);
    }

    if (ret)
        set_para_value(cmdStatus, NORMAL_DEAL);

    set_para_value(exec->resp[0], cmdStatus);

    set_para_value(exec->board_response, can_common_response);
    return ret;
}