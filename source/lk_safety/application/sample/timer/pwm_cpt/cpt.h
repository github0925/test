
#ifndef __H_CPT_H__
#define __H_CPT_H__

typedef struct {
    void **timer_handle;
    hal_timer_sub_t tmr_sub_chn;
    hal_timer_func_ch_t tmr_func_chn;
    Port_PinType pin_type;
    uint8_t cpt_chn;
} cpt_table_t;

typedef struct {
    uint32_t high_us;
    uint32_t low_us;
    uint32_t freq;
    uint32_t duty;
    uint32_t rise_cnt;
    uint32_t fall_cnt;
    uint32_t rise_detect;
    uint32_t fall_detect;
    bool valid;
} cpt_result_t;

void cpt_timer_init(void);
void cpt_timer_start(void);
cpt_result_t *cpt_timer_result_get(uint8_t channel);

#endif


