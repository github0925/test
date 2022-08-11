#if !DDR_ENTER_SELF && !STATIC_HANDOVER
#include "image_cfg.h"
enum STR_MASTER {
    STR_AP1,
    STR_AP2
};

#if CONFIG_DCF_HAS_AP1
#define STR_VOTE_AP1    (1<<STR_AP1)
#define STR_AP1_TO_SAF  (SAF_AP1_MEMBASE + 0x100000)
#else
#define STR_VOTE_AP1    (0)
#define STR_AP1_TO_SAF  (0)
#endif

#if CONFIG_DCF_HAS_AP2
#define STR_VOTE_AP2    (1<<STR_AP2)
#define STR_AP2_TO_SAF  (SAF_AP2_MEMBASE + 0x100000)
#else
#define STR_VOTE_AP2    (0)
#define STR_AP2_TO_SAF  (0)
#endif

#define STR_VOTE_ALL ( STR_VOTE_AP1 | STR_VOTE_AP2 )
#define STR_RTC_FLAG    0xf1890050

bool is_str_enter(enum STR_MASTER master);
bool is_str_resume(enum STR_MASTER master);
uint32_t get_str_resume_entry(enum STR_MASTER master);
void clr_str_flag(enum STR_MASTER master);
void set_str_flag_to_rtc(enum STR_MASTER master);
void str_vote(enum STR_MASTER master);
bool check_str_all(void);
void str_clr_vote(void);
__WEAK void power_config_before_str(void) {};
#endif
