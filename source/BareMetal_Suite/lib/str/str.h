#ifndef STR_MODE_H
#define STR_MODE_H

#include <types_def.h>
#include "partition_load_configs.h"

enum STR_MASTER {
    STR_AP1,
    STR_AP2
};

#ifdef SAF_AP1_MEMBASE
#define STR_AP1_TO_SAF  (SAF_AP1_MEMBASE + 0x100000)
#else
#define STR_AP1_TO_SAF  (0)
#endif
#ifdef SAF_AP2_MEMBASE
#define STR_AP2_TO_SAF  (SAF_AP2_MEMBASE + 0x100000)
#else
#define STR_AP2_TO_SAF  (0)
#endif
#define STR_RTC_FLAG    0xf1890050
#define STR_SAVED_FREQ  0xf841b000

struct pt_load_config *str_get_pt_configs(void);
uint32_t str_get_pt_configs_cnt(void);
bool is_str_enter(void);
bool is_str_resume();
void set_str_resume(enum STR_MASTER master, bool flag);
void str_save_ddr_freq(uint32_t freq);
void ddr_self_exit(void);
#endif
