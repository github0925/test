#ifndef __RTC_DRV_H__
#define __RTC_DRV_H__

#include <stdint.h>
#include <stdbool.h>
#include <sys/types.h>

/*********** API Declaration ***********/

/* rtc_switch_osc : switch clock source to RTC_XTAL.
(wait) : whether waiting for rtc_xtal to be locked.
return : switch result. */
bool rtc_switch_osc(bool wait);

/* rtc_init : initialize rtc module.
Any rtc api must be invoked after initialization.
(base) : module register base.
return : none. */
void rtc_init(addr_t base);

/* rtc_enable : enable rtc module and start rtc counter.
(base) : module register base.
(en) : enable or not.
return : true if manipulation succ or false in locked case. */
void rtc_enable(addr_t base);

/* rtc_lock : lock rtc ctrl manipualtion. if invoked, any enabling state
/privileged state/secure access state could not be modified untill next hardware
power reset.
(base) : module register base.
return : true if manipulation succ or false in locked case. */
void rtc_lock(addr_t base);

/* rtc_prv_access_enable : enable privileged access mode. if enable, all access
to rtc module must be in privileged mode(except user mode)
(base) : module register base.
(en) : enable or not
return : true if manipulation succ or false in locked case. */
bool rtc_prv_access_enable(addr_t base,bool en);

/* rtc_sec_access_enable : enable secure access mode. if enable, all access
to rtc module must be in secure world(NS = 0)
(base) : module register base.
(en) : enable or not
return : true if manipulation succ or false in locked case. */
bool rtc_sec_access_enable(addr_t base,bool en);

/* rtc_get_tick : get tick count from rtc counter.
(base) : module register base.
return : counter value (48-bit). */
uint64_t rtc_get_tick(addr_t base);

/* rtc_config_adjust : configure auto adjust function.
this function configure increment or decrement of cycle of rtc
clock to tune rtc accuraccy, which is max to ±0.7% drift. If
enabled, rtc will auto do increase or decrease cycle value to counter
per 256 rtc clocks.
(base) : module register base.
(cycle) : cycles to be adjusted
(dir) : 0 - increase, 1- decrease
return : none. */
void rtc_config_adjust(addr_t base, uint8_t cycle, uint8_t dir);

/* rtc_adjust_enable : enable auto adjust function.
(base) : module register base.
(en) : enable or not
return : none. */
void rtc_adjust_enable(addr_t base, bool en);

/* rtc_update_cmp_value : update compare value.
rtc will do interrupt or wakeup request (if enable)
while rtc counter equals to compare value.
(base) : module register base.
(increment) : increment ticks from current counter,
the final compare value = rtc counter + increment
return : none. */
void rtc_update_cmp_value(addr_t base, uint64_t increment);

/* rtc_wakeup_enable : enable wakeup feature
rtc will do interrupt or wakeup request (if enable)
while rtc counter equals to compare value.
(base) : module register base.
(en) : enable wakeup feature or not
(irq_en) : enable interrupt dispatch to cpu
(req_en) : enable request dispatch to pmu
return : none. */
void rtc_wakeup_enable(addr_t base, bool en, bool irq_en, bool req_en);

/* rtc_wakeup_enable : config periodic interrupt feature
rtc will dispatch interrupt periodically from specific divider.
(base) : module register base.
(div) : periodic freq = 32768/(2^(div+1)) which div shall be from 0~15
return : true if manipulation succ or false with invalid divider value. */
bool rtc_config_periodic(addr_t base, uint8_t div);

/* rtc_periodical_interrupt_enable : enable periodic interrupt feature
rtc will dispatch interrupt periodically from specific divider.
(base) : module register base.
(en) : enable periodic interrupt feautre or not
return : none
***NOTE*** : Periodic interrupt to GIC must be set to edge-sensitive mode. */
void rtc_periodical_interrupt_enable(addr_t base, bool en);

/* rtc_vio_dis_interrupt_enable : enable disabling violation
 interrupt feature
rtc will dispatch interrupt if rtc was disabled (invoke rtc_enable())
(base) : module register base.
(en) : enable disabling violation interrupt feautre or not
return : none */
void rtc_vio_dis_interrupt_enable(addr_t base, bool en);

/* rtc_vio_ovf_interrupt_enable : enable overflow violation
 interrupt feature
rtc will dispatch interrupt if rtc counter was overflowed
(base) : module register base.
(en) : enable overflow violation interrupt feautre or not
return : none */
void rtc_vio_ovf_interrupt_enable(addr_t base, bool en);

/* rtc_check_wakeup_status : check wakeup state
this function only valid while wakeup feature enable.
(base) : module register base.
return : wakeup happened(counter >= compare) or not */
bool rtc_check_wakeup_status(addr_t base);

/* rtc_check_wakeup_status : check disabling violation state
this function only valid while disabling violation feature enable.
(base) : module register base.
return : disabling violation or not */
bool rtc_check_vio_dis_status(addr_t base);

/* rtc_check_wakeup_status : check overflow violation state
this function only valid while overflow violation feature enable.
(base) : module register base.
return : overflow violation or not */
bool rtc_check_vio_ovf_status(addr_t base);

/* rtc_clr_wakeup_status : clear wakeup state
this function clear wakeup state in wakeup control register.
Note : Clear wakeup state will NOT prevent next interrupt generation
in next rtc clock unless compare value was updated.
If user need one-shot interrupt, please use rtc_wakeup_enable function to
disable wakeup feature while the interrupt generation after this function invoked.
(base) : module register base.
return : none */
void rtc_clr_wakeup_status(addr_t base);

/* rtc_clr_vio_dis_status : clear disabling violation state
this function clear disabling violation state in disabling
violation control register.
Note : Clear disabling violation state will NOT prevent next
interrupt generation in next rtc clock tick.
If user need one-shot interrupt, please use rtc_vio_dis_interrupt_enable
 function to disable disabling violation feature while the interrupt
generation after this function invoked.
(base) : module register base.
return : none */
void rtc_clr_vio_dis_status(addr_t base);

/* rtc_clr_wakeup_status : clear overflow violation state
this function clear overflow violation state in overflow
violation control register.
Note : Clear overflow violation state will NOT prevent next
interrupt generation in next rtc clock tick.
If user need one-shot interrupt, please use rtc_vio_dis_interrupt_enable
 function to disable overflow violation feature while the interrupt
generation after this function invoked.
(base) : module register base.
return : none */
void rtc_clr_vio_ovf_status(addr_t base);

uint32_t rtc_set_tick(addr_t base, uint64_t tick);

#define RTC_CLK_HZ              32768
/* RTC ticks to micro-seconds helper macro. */
#define RtcTick2Ms(tick)        ((uint64_t)tick * (uint64_t)1000 / RTC_CLK_HZ)
/* RTC ticks to seconds helper macro. */
#define RtcTick2S(tick)         ((uint64_t)tick / RTC_CLK_HZ)
/* micro-seconds to rtc ticks helper macro. */
#define MS2RtcTick(ms)          ((uint64_t)(ms * RTC_CLK_HZ / (uint64_t)1000))
/* seconds to rtc ticks helper macro. */
#define S2RtcTick(s)            ((uint64_t)(s * RTC_CLK_HZ))

#define RTC_TICK_MAX    0xffffffffffffull

#endif
