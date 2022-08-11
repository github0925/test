/*
* Copyright (c) 2019 Semidrive Semiconductor.
* All rights reserved.
*
*/

#include <debug.h>
#include <string.h>
#include <app.h>
#include <sys/types.h>
#include <lib/console.h>
#include <timer_hal.h>
#undef HAL_ASSERT_PARAMETER
#include <dma_hal.h>
#include "hal_port.h"
#include "hal_dio.h"
#include "res.h"
#include "chip_res.h"
#include "cpt.h"

//24M - 1 HZ
#define CPT_DBG       ALWAYS
#define CPT_TMR_FREQ  24000000
#define CPT_TMR_DIV   0
#define CPT_CHN_TOTAL 4
#define CPT_DEBOUNCE  1

#define CPT_WAIT_INIT      0
#define CPT_WAIT_RISE_EDGE 1
#define CPT_WAIT_FALL_EDGE 2

#define CPT_BUF_WORDS_SIZE 4

static void *cpt_tmr = NULL;

static struct dma_chan *dma_tx_chan[CPT_CHN_TOTAL] = {NULL};
static struct dma_desc *dma_tx_desc[CPT_CHN_TOTAL] = {NULL};
static cpt_result_t cpt_result[CPT_CHN_TOTAL] = {0};
static uint32_t cpt_buf[CPT_CHN_TOTAL][CPT_BUF_WORDS_SIZE] __attribute__ ((
            aligned(CACHE_LINE)));
static uint32_t cpt_wait_edge[CPT_CHN_TOTAL] = {0};

//#define CPT_LOOPBACK_TEST
#define RES_TIMER_CPT RES_TIMER_TIMER3

#if (RES_TIMER_CPT == RES_TIMER_TIMER2)
const cpt_table_t cpt_table[CPT_CHN_TOTAL] = {
    {&cpt_tmr, HAL_TIMER_LOCAL_A, HAL_TIMER_FUNC_CH_A, PortConf_PIN_GPIO_A8,  0},
    {&cpt_tmr, HAL_TIMER_LOCAL_B, HAL_TIMER_FUNC_CH_B, PortConf_PIN_GPIO_A9,  1},
    {&cpt_tmr, HAL_TIMER_LOCAL_C, HAL_TIMER_FUNC_CH_C, PortConf_PIN_GPIO_A10, 2},
    {&cpt_tmr, HAL_TIMER_LOCAL_D, HAL_TIMER_FUNC_CH_D, PortConf_PIN_GPIO_A11, 3},
};
#elif (RES_TIMER_CPT == RES_TIMER_TIMER3)
const cpt_table_t cpt_table[CPT_CHN_TOTAL] = {
    {&cpt_tmr, HAL_TIMER_LOCAL_A, HAL_TIMER_FUNC_CH_A, PortConf_PIN_GPIO_C0, 0},
    {&cpt_tmr, HAL_TIMER_LOCAL_B, HAL_TIMER_FUNC_CH_B, PortConf_PIN_GPIO_C1, 1},
    {&cpt_tmr, HAL_TIMER_LOCAL_C, HAL_TIMER_FUNC_CH_C, PortConf_PIN_GPIO_C2, 2},
    {&cpt_tmr, HAL_TIMER_LOCAL_D, HAL_TIMER_FUNC_CH_D, PortConf_PIN_GPIO_C3, 3},
};
#endif

static void cpt_timer_dma_start(const cpt_table_t *cpt_item);

static void cpt_timer_data_init(void)
{
    uint32_t i;

    for (i = 0; i < CPT_CHN_TOTAL; i++) {
        cpt_result[i].valid = false;
        cpt_result[i].rise_detect = 0;
        cpt_result[i].fall_detect = 0;
    }
}

static uint32_t cpt_timer_dma_type_get(void *handle)
{
    timer_instance_t *timer_inst = (timer_instance_t *)handle;

    if ((addr_t)(timer_inst->timer) == APB_TIMER1_BASE) {
        return DMA_PERI_TIMER1;
    }
    else if ((addr_t)(timer_inst->timer) == APB_TIMER2_BASE) {
        return DMA_PERI_TIMER2;
    }
    else if ((addr_t)(timer_inst->timer) == APB_TIMER3_BASE) {
        return DMA_PERI_TIMER3;
    }
    else if ((addr_t)(timer_inst->timer) == APB_TIMER4_BASE) {
        return DMA_PERI_TIMER4;
    }
    else if ((addr_t)(timer_inst->timer) == APB_TIMER5_BASE) {
        return DMA_PERI_TIMER5;
    }
    else if ((addr_t)(timer_inst->timer) == APB_TIMER6_BASE) {
        return DMA_PERI_TIMER6;
    }
    else if ((addr_t)(timer_inst->timer) == APB_TIMER7_BASE) {
        return DMA_PERI_TIMER7;
    }
    else if ((addr_t)(timer_inst->timer) == APB_TIMER8_BASE) {
        return DMA_PERI_TIMER8;
    }

    return 0xFFFFFFFF;
}

static struct dma_chan *cpt_timer_dma_cfg(const cpt_table_t *cpt_item)
{
    struct dma_dev_cfg dma_cfg;
    enum dma_chan_tr_type ch_type;
    struct dma_chan *tx_chan = NULL;

    if (*cpt_item->timer_handle == NULL) {
        dprintf(ALWAYS, "%s() handle is null\n", __func__);
        return NULL;
    }

    timer_instance_t *timer_inst = (timer_instance_t *)(
                                       *cpt_item->timer_handle);
    ch_type = cpt_timer_dma_type_get(*cpt_item->timer_handle);

    dma_cfg.direction = DMA_DEV2MEM;
    dma_cfg.src_addr_width = DMA_DEV_BUSWIDTH_4_BYTES;
    dma_cfg.dst_addr_width = DMA_DEV_BUSWIDTH_4_BYTES;
    dma_cfg.src_maxburst = DMA_BURST_TR_1ITEM;
    dma_cfg.dst_maxburst = DMA_BURST_TR_1ITEM;

    if (cpt_item->tmr_func_chn == HAL_TIMER_FUNC_CH_A) {
        dma_cfg.src_addr = (addr_t)(&(timer_inst->timer->fifo_a));
    }

    if (cpt_item->tmr_func_chn == HAL_TIMER_FUNC_CH_B) {
        dma_cfg.src_addr = (addr_t)(&(timer_inst->timer->fifo_b));
    }

    if (cpt_item->tmr_func_chn == HAL_TIMER_FUNC_CH_C) {
        dma_cfg.src_addr = (addr_t)(&(timer_inst->timer->fifo_c));
    }

    if (cpt_item->tmr_func_chn == HAL_TIMER_FUNC_CH_D) {
        dma_cfg.src_addr = (addr_t)(&(timer_inst->timer->fifo_d));
    }

    tx_chan = hal_dma_chan_req(ch_type);

    if (tx_chan) {
        hal_dma_dev_config(tx_chan, &dma_cfg);
    }
    else {
        dprintf(ALWAYS, "%s() dma chan req fail\n", __func__);
    }

    return tx_chan;
}

static void cpt_edge_update(cpt_table_t *cpt_item,
                            hal_timer_cpt_edge_t edge)
{
    hal_timer_fun_cfg_t fun_cfg;

    fun_cfg.func.dma_ctrl.dma_enable = true;
    fun_cfg.func.dma_ctrl.dma_sel = HAL_TIMER_DMA_SEL_CPT;
    fun_cfg.func.dma_ctrl.fifo_wml = 1;
    fun_cfg.func.func_type = HAL_TIMER_FUNC_TYPE_CPT;
    fun_cfg.func.sub_func.cpt_cfg.cpt_cnt_sel = HAL_TIMER_CPT_CNT_LOCAL;
    fun_cfg.func.sub_func.cpt_cfg.trig_mode = edge;
    fun_cfg.func.sub_func.cpt_cfg.dual_cpt_mode = 0;
    fun_cfg.func.sub_func.cpt_cfg.single_mode = 0;
    fun_cfg.func.sub_func.cpt_cfg.filter_dis = 1;
    fun_cfg.func.sub_func.cpt_cfg.filter_width = 0;
    fun_cfg.func.sub_func.cpt_cfg.first_cpt_rst_en = 0;

    hal_timer_func_cpt_disable(*cpt_item->timer_handle,
                               cpt_item->tmr_func_chn);
    hal_timer_func_init(*cpt_item->timer_handle, cpt_item->tmr_func_chn,
                        &fun_cfg);
}

extern const domain_res_t g_iomuxc_res;
extern const domain_res_t g_gpio_res;

static void *port_handle;
static void *dio_handle;

const Port_PinModeType CPT_MODE_GPIO = {
    ((uint32_t)PORT_PAD_POE__DISABLE | PORT_PAD_IS__IN | PORT_PAD_SR__FAST | PORT_PAD_DS__MID1 | PORT_PIN_IN_PULL_DOWN ),
    ((uint32_t)PORT_PIN_MUX_FV__MIN | PORT_PIN_MUX_FIN__MIN  | PORT_PIN_OUT_OPENDRAIN | PORT_PIN_MODE_GPIO),
};

const Port_PinModeType CPT_MODE_TIMER = {
    ((uint32_t)PORT_PAD_POE__ENABLE | PORT_PAD_IS__OUT | PORT_PAD_SR__FAST | PORT_PAD_DS__MID1 | PORT_PIN_IN_PULL_UP ),
    ((uint32_t)PORT_PIN_MUX_FV__MIN | PORT_PIN_MUX_FIN__MIN  | PORT_PIN_OUT_PUSHPULL | PORT_PIN_MODE_ALT5),
};

static void cpt_dma_tx_irq_evt_handle(enum dma_status status, uint32_t err,
                                      void *ctx)
{
    cpt_table_t *cpt_item = (cpt_table_t *)ctx;
    uint32_t cpt_chn = cpt_item->cpt_chn;
    timer_instance_t *instance = (timer_instance_t *)(*cpt_item->timer_handle);
    uint32_t high_dura_cnt, low_dura_cnt, cycle_dura_cnt;

    if (status == DMA_COMP) {
        if (cpt_wait_edge[cpt_chn] == CPT_WAIT_INIT) {
            uint8_t level = 0;
            hal_port_creat_handle(&port_handle, g_iomuxc_res.res_id[0]);
            hal_dio_creat_handle(&dio_handle, g_gpio_res.res_id[0]);

            hal_port_set_pin_mode(port_handle, cpt_item->pin_type, CPT_MODE_GPIO);
            level = hal_dio_read_channel(dio_handle, cpt_item->pin_type);
            hal_port_set_pin_mode(port_handle, cpt_item->pin_type, CPT_MODE_TIMER);

            hal_port_release_handle(&port_handle);
            hal_dio_release_handle(&dio_handle);

            if (level) {
                cpt_wait_edge[cpt_chn] = CPT_WAIT_RISE_EDGE;
            }
            else {
                cpt_wait_edge[cpt_chn] = CPT_WAIT_FALL_EDGE;
            }
        }

        if (cpt_wait_edge[cpt_chn] == CPT_WAIT_RISE_EDGE) {
            if ((cpt_result[cpt_chn].rise_detect > CPT_DEBOUNCE)
                    && (cpt_result[cpt_chn].fall_detect > CPT_DEBOUNCE)) {

                low_dura_cnt = cpt_buf[cpt_chn][0] - cpt_result[cpt_chn].fall_cnt;
                high_dura_cnt = cpt_result[cpt_chn].fall_cnt -
                                cpt_result[cpt_chn].rise_cnt;
                cycle_dura_cnt = cpt_buf[cpt_chn][0] - cpt_result[cpt_chn].rise_cnt;

                cpt_result[cpt_chn].low_us = low_dura_cnt / instance->cnt_per_us;

                if (cycle_dura_cnt) {
                    cpt_result[cpt_chn].freq =
                        (CPT_TMR_FREQ / (CPT_TMR_DIV + 1) + (cycle_dura_cnt >> 1)) /
                        (cycle_dura_cnt);
                    cpt_result[cpt_chn].duty =
                        (high_dura_cnt * 100 + (cycle_dura_cnt >> 1)) / (cycle_dura_cnt);
                }

                cpt_result[cpt_chn].valid = true;
                cpt_result[cpt_chn].rise_detect = CPT_DEBOUNCE;
                //dprintf(CPT_DBG, "%d, %d\n", cpt_result[cpt_chn].freq, cpt_result[cpt_chn].duty);
                //dprintf(CPT_DBG, "r: %d\n", cpt_buf[cpt_chn][0]);
            }

            cpt_result[cpt_chn].rise_cnt = cpt_buf[cpt_chn][0];
            cpt_result[cpt_chn].rise_detect++;
            cpt_wait_edge[cpt_chn] = CPT_WAIT_FALL_EDGE;
        }
        else if (cpt_wait_edge[cpt_chn] == CPT_WAIT_FALL_EDGE) {
            if ((cpt_result[cpt_chn].rise_detect > CPT_DEBOUNCE)
                    && (cpt_result[cpt_chn].fall_detect > CPT_DEBOUNCE)) {

                high_dura_cnt = cpt_buf[cpt_chn][0] - cpt_result[cpt_chn].rise_cnt;
                low_dura_cnt = cpt_result[cpt_chn].rise_cnt - cpt_result[cpt_chn].fall_cnt;
                cycle_dura_cnt = cpt_buf[cpt_chn][0] - cpt_result[cpt_chn].fall_cnt;

                cpt_result[cpt_chn].high_us = high_dura_cnt / instance->cnt_per_us;

                if (cycle_dura_cnt) {
                    cpt_result[cpt_chn].freq =
                        (CPT_TMR_FREQ / (CPT_TMR_DIV + 1) + (cycle_dura_cnt >> 1)) /
                        (cycle_dura_cnt);
                    cpt_result[cpt_chn].duty =
                        (high_dura_cnt * 100 + (cycle_dura_cnt >> 1)) / (cycle_dura_cnt);
                }

                cpt_result[cpt_chn].valid = true;
                cpt_result[cpt_chn].fall_detect = CPT_DEBOUNCE;
                //dprintf(CPT_DBG, "%d, %d\n", cpt_result[cpt_chn].freq, cpt_result[cpt_chn].duty);
                //dprintf(CPT_DBG, "f: %d\n", cpt_buf[cpt_chn][0]);
            }

            cpt_result[cpt_chn].fall_cnt = cpt_buf[cpt_chn][0];
            cpt_result[cpt_chn].fall_detect++;
            cpt_wait_edge[cpt_chn] = CPT_WAIT_RISE_EDGE;
        }

        cpt_timer_dma_start(cpt_item);
    }
    else {
        dprintf(ALWAYS, "DMA transfer err, status:%d!\n", status);
    }
}

static void cpt_timer_dma_start(const cpt_table_t *cpt_item)
{
    uint8_t cpt_chn = cpt_item->cpt_chn;

    if (dma_tx_desc[cpt_chn]) {
        hal_dma_free_desc(dma_tx_desc[cpt_chn]);
        dma_tx_chan[cpt_chn] = cpt_timer_dma_cfg(cpt_item);
    }

    if (dma_tx_chan[cpt_chn] == NULL) {
        dprintf(ALWAYS, "dma_tx_chan is null!\n");
        return;
    }

    dma_tx_desc[cpt_chn] = hal_prep_dma_dev(dma_tx_chan[cpt_chn],
                                            (void *) & (cpt_buf[cpt_chn]), 4, DMA_INTERRUPT);

    if (dma_tx_desc[cpt_chn] == NULL) {
        dprintf(ALWAYS, "dma_tx_desc is null!\n");
        return;
    }

    dma_tx_desc[cpt_chn]->dmac_irq_evt_handle = cpt_dma_tx_irq_evt_handle;
    dma_tx_desc[cpt_chn]->context = (void *)cpt_item;
    arch_clean_cache_range((addr_t) & (cpt_buf[cpt_chn]),
                           CPT_BUF_WORDS_SIZE * 4);
    arch_invalidate_cache_range((addr_t) & (cpt_buf[cpt_chn]),
                                CPT_BUF_WORDS_SIZE * 4);
    hal_dma_submit(dma_tx_desc[cpt_chn]);
}

static enum handler_return cpt_ovf_ch0_int_cbk(void)
{
    cpt_result[0].duty = 0;
    cpt_result[0].freq = 0;
    cpt_result[0].rise_detect = 0;
    cpt_result[0].fall_detect = 0;
    cpt_result[0].high_us = 0xFFFFFFFF;
    cpt_result[0].low_us = 0xFFFFFFFF;

    return INT_NO_RESCHEDULE;
}

static enum handler_return cpt_ovf_ch1_int_cbk(void)
{
    cpt_result[1].duty = 0;
    cpt_result[1].freq = 0;
    cpt_result[1].rise_detect = 0;
    cpt_result[1].fall_detect = 0;
    cpt_result[1].high_us = 0xFFFFFFFF;
    cpt_result[1].low_us = 0xFFFFFFFF;

    return INT_NO_RESCHEDULE;
}

static enum handler_return cpt_ovf_ch2_int_cbk(void)
{
    cpt_result[2].duty = 0;
    cpt_result[2].freq = 0;
    cpt_result[2].rise_detect = 0;
    cpt_result[2].fall_detect = 0;
    cpt_result[2].high_us = 0xFFFFFFFF;
    cpt_result[2].low_us = 0xFFFFFFFF;

    return INT_NO_RESCHEDULE;
}

static enum handler_return cpt_ovf_ch3_int_cbk(void)
{
    cpt_result[3].duty = 0;
    cpt_result[3].freq = 0;
    cpt_result[3].rise_detect = 0;
    cpt_result[3].fall_detect = 0;
    cpt_result[3].high_us = 0xFFFFFFFF;
    cpt_result[3].low_us = 0xFFFFFFFF;

    return INT_NO_RESCHEDULE;
}

void cpt_timer_init(void)
{
    hal_timer_glb_cfg_t glb_cfg;
    hal_timer_ovf_cfg_t ovf_cfg;
    hal_timer_fun_cfg_t fun_cfg;

    cpt_timer_data_init();

    hal_timer_creat_handle(&cpt_tmr, RES_TIMER_CPT);

    if (cpt_tmr == NULL) {
        dprintf(ALWAYS, "%s() create handle fail!\n", __func__);
        return;
    }

    glb_cfg.cascade = false;
    glb_cfg.clk_frq = CPT_TMR_FREQ;
    glb_cfg.clk_div = CPT_TMR_DIV;
    glb_cfg.clk_sel = HAL_TIMER_SEL_LF_CLK;

    hal_timer_global_init(cpt_tmr, &glb_cfg);

    ovf_cfg.periodic = true;
    ovf_cfg.cnt_val = 0;
    ovf_cfg.ovf_val = 0xFFFFFFFF;

    fun_cfg.func.dma_ctrl.dma_enable = true;
    fun_cfg.func.dma_ctrl.dma_sel = HAL_TIMER_DMA_SEL_CPT;
    fun_cfg.func.dma_ctrl.fifo_wml = 1;
    fun_cfg.func.func_type = HAL_TIMER_FUNC_TYPE_CPT;
    fun_cfg.func.sub_func.cpt_cfg.cpt_cnt_sel = HAL_TIMER_CPT_CNT_LOCAL;
    fun_cfg.func.sub_func.cpt_cfg.trig_mode = HAL_TIMER_CPT_TOGGLE_EDGE;
    fun_cfg.func.sub_func.cpt_cfg.dual_cpt_mode = 0;
    fun_cfg.func.sub_func.cpt_cfg.single_mode = 0;
    fun_cfg.func.sub_func.cpt_cfg.filter_dis = 1;
    fun_cfg.func.sub_func.cpt_cfg.filter_width = 0;
    fun_cfg.func.sub_func.cpt_cfg.first_cpt_rst_en = 0;

#ifdef CPT_LOOPBACK_TEST
    hal_timer_ovf_init(*cpt_table[2].timer_handle, cpt_table[2].tmr_sub_chn,
                       &ovf_cfg);
    hal_timer_func_init(*cpt_table[2].timer_handle, cpt_table[2].tmr_func_chn,
                        &fun_cfg);
    dma_tx_chan[2] = cpt_timer_dma_cfg(&cpt_table[2]);

    ovf_cfg.periodic = true;
    ovf_cfg.cnt_val = 0;
    ovf_cfg.ovf_val = 2400 - 1;

    fun_cfg.func.dma_ctrl.dma_enable = false;
    fun_cfg.func.dma_ctrl.dma_sel = HAL_TIMER_DMA_SEL_CMP;
    fun_cfg.func.dma_ctrl.fifo_wml = 1;
    fun_cfg.func.func_type = HAL_TIMER_FUNC_TYPE_CMP;
    fun_cfg.func.sub_func.cmp_cfg.cmp_cnt_sel = HAL_TIMER_CMP_CNT_LOCAL;
    fun_cfg.func.sub_func.cmp_cfg.cmp0_out_mode = HAL_TIMER_CMP_LEVEL_HIGH;
    fun_cfg.func.sub_func.cmp_cfg.cmp1_out_mode = HAL_TIMER_CMP_LEVEL_LOW;
    fun_cfg.func.sub_func.cmp_cfg.single_mode = 0;
    fun_cfg.func.sub_func.cmp_cfg.dual_cmp_mode = 1;
    fun_cfg.func.sub_func.cmp_cfg.cmp_rst_en = 0;
    fun_cfg.func.sub_func.cmp_cfg.cmp0_val = 100;
    fun_cfg.func.sub_func.cmp_cfg.cmp1_val = 700;

    hal_timer_ovf_init(*cpt_table[3].timer_handle, cpt_table[3].tmr_sub_chn,
                       &ovf_cfg);
    hal_timer_func_init(*cpt_table[3].timer_handle, cpt_table[3].tmr_func_chn,
                        &fun_cfg);
#else

    for (uint32_t i = 0; i < CPT_CHN_TOTAL; i++) {
        hal_timer_ovf_init(*cpt_table[i].timer_handle, cpt_table[i].tmr_sub_chn,
                           &ovf_cfg);
        hal_timer_func_init(*cpt_table[i].timer_handle, cpt_table[i].tmr_func_chn,
                            &fun_cfg);
        dma_tx_chan[i] = cpt_timer_dma_cfg(&cpt_table[i]);
    }

#endif

    hal_timer_int_src_enable(cpt_tmr, HAL_TIMER_CNT_LA_OVF_INT_SRC);
    hal_timer_int_src_enable(cpt_tmr, HAL_TIMER_CNT_LB_OVF_INT_SRC);
    hal_timer_int_src_enable(cpt_tmr, HAL_TIMER_CNT_LC_OVF_INT_SRC);
    hal_timer_int_src_enable(cpt_tmr, HAL_TIMER_CNT_LD_OVF_INT_SRC);
    hal_timer_int_cbk_register(cpt_tmr, HAL_TIMER_CNT_LA_OVF_INT_SRC,
                               cpt_ovf_ch0_int_cbk);
    hal_timer_int_cbk_register(cpt_tmr, HAL_TIMER_CNT_LB_OVF_INT_SRC,
                               cpt_ovf_ch1_int_cbk);
    hal_timer_int_cbk_register(cpt_tmr, HAL_TIMER_CNT_LC_OVF_INT_SRC,
                               cpt_ovf_ch2_int_cbk);
    hal_timer_int_cbk_register(cpt_tmr, HAL_TIMER_CNT_LD_OVF_INT_SRC,
                               cpt_ovf_ch3_int_cbk);

    dprintf(CPT_DBG, "%s()\n", __func__);
}

void cpt_timer_start(void)
{
    for (uint8_t i = 0; i < CPT_CHN_TOTAL; i++) {
        cpt_timer_dma_start(&cpt_table[i]);
    }

    dprintf(CPT_DBG, "%s()\n", __func__);
}

cpt_result_t *cpt_result_get(uint8_t channel)
{
    return &(cpt_result[channel]);
}

int cpt_start_test(int argc, const cmd_args *argv)
{
    cpt_timer_init();
    cpt_timer_start();
    return 0;
}

int cpt_get_result_test(int argc, const cmd_args *argv)
{
    if (argc != 2) {
        dprintf(CPT_DBG, "argc must be 2\n");
        return -1;
    }

    if (argv[1].u >= CPT_CHN_TOTAL) {
        dprintf(CPT_DBG, "argv[1] exceed range\n");
        return -1;
    }

    cpt_result_t *cpt_result;
    cpt_result = cpt_result_get(argv[1].u);
    dprintf(ALWAYS, "freq: %d, duty: %d, high_us: %d, low_us: %d\n",
            cpt_result->freq, cpt_result->duty, cpt_result->high_us,
            cpt_result->low_us);

    return 0;
}

#if defined(WITH_LIB_CONSOLE)
STATIC_COMMAND_START
STATIC_COMMAND("cpt_start",   "cpt test", (console_cmd)&cpt_start_test)
STATIC_COMMAND("cpt_result",  "cpt test",
               (console_cmd)&cpt_get_result_test)
STATIC_COMMAND_END(cpt_test);
#endif


APP_START(cpt)
.flags = 0,
APP_END

