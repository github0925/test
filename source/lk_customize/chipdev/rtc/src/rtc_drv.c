/*
* rtc_drv.c
*
* Copyright (c) 2018 Semidrive Semiconductor.
* All rights reserved.
*
* Description: real-time clock driver implementation.
*
*/

//*****************************************************************************
//
// If building with a C++ compiler, make all of the definitions in this header
// have a C binding.
//
//*****************************************************************************
#ifdef __cplusplus
extern "C"
{
#endif

#include <rtc_regs_def.h>
#include <rtc_drv.h>
#include <reg.h>
#include <__regs_base.h>
#include <spinlock.h>
#include <stdlib.h>
#include <debug.h>

#define CLK_SYNC    7

#define RTC_LOCK(base) spin_lock_saved_state_t state; do{\
        if(APB_RTC1_BASE == base)\
            spin_lock_irqsave(&rtc1_lock, state);\
        else\
            spin_lock_irqsave(&rtc2_lock, state);\
    }while(0)

#define RTC_UNLOCK(base) do{\
        if(APB_RTC1_BASE == base)\
            spin_unlock_irqrestore(&rtc1_lock, state);\
        else\
            spin_unlock_irqrestore(&rtc2_lock, state);\
    }while(0)

static spin_lock_t rtc1_lock = SPIN_LOCK_INITIAL_VALUE;
static spin_lock_t rtc2_lock = SPIN_LOCK_INITIAL_VALUE;

static void rtc_clk_sync(addr_t base, uint32_t rtc_clk);
bool rtc_switch_osc(bool wait)
{
    bool done = false;

    do{
        /* xtal mask bit:31 | 1 | 0 - lock | EN |power down */
        if (((readl(APB_XTAL_RTC_BASE)) & 0x80000003) == 0x80000002)
        {
            done = true;
            /* xtal is stable now. select to xtal as source. */
            if (!(readl(APB_RC_RTC_BASE + 4) & 1ul))
                writel(1ul, APB_RC_RTC_BASE + 4);
            break;
        }

    } while(wait);

    return done;
}

void rtc_init(addr_t base)
{
    rtc_switch_osc(false);

    if(true == rtc_check_wakeup_status(base))
    {
        rtc_clr_wakeup_status(base);
        rtc_wakeup_enable(base, false, false, false);
    }
}

static bool rtc_is_lock(uint32_t var)
{
    if (var & (1ul << RTCBS(RTC_CTRL, LOCK))) {
        return true;
    }
    else {
        return false;
    }
}

static bool rtc_is_en(uint32_t var)
{
    if (var & (1ul << RTCBS(RTC_CTRL, EN))) {
        return true;
    }
    else {
        return false;
    }
}

void rtc_enable(addr_t base)
{
    RTC_LOCK(base);

    uint32_t var = readl(base + RTCROFF(RTC_CTRL));

    if(rtc_is_en(var)) {
        /* enabled */
        RTC_UNLOCK(base);
        dprintf(INFO, "rtc 0x%lx was already enabled\n", base);
        return;
    }

    dprintf(INFO, "rtc 0x%lx was reset to be off\n", base);

    if(rtc_is_lock(var)) {
        printf("Error: rtc 0x%lx was locked beforehand!\n", base);
        return;
    }

    /* init tick counter with zero */
    writel(0, base + RTCROFF(RTC_L));
    writel(0, base + RTCROFF(RTC_H));

    var |= (1ul << RTCBS(RTC_CTRL, EN));
    writel(var, base + RTCROFF(RTC_CTRL));
    rtc_clk_sync(base, CLK_SYNC);

    RTC_UNLOCK(base);
}

void rtc_lock(addr_t base)
{
    RTC_LOCK(base);

    uint32_t var = readl(base + RTCROFF(RTC_CTRL));

    if(rtc_is_lock(var))
    {
        /* already locked */
        RTC_UNLOCK(base);
        return;
    }

    var |= (1ul << RTCBS(RTC_CTRL, LOCK));
    writel(var, base + RTCROFF(RTC_CTRL));
    rtc_clk_sync(base, CLK_SYNC);

    RTC_UNLOCK(base);

    return;
}

bool rtc_prv_access_enable(addr_t base,bool en)
{
    bool ret = true;

    RTC_LOCK(base);

    uint32_t var = readl(base + RTCROFF(RTC_CTRL));

    if(var & (1ul << RTCBS(RTC_CTRL,LOCK)))
    {
        /* locked, non-accessable */
        ret = false;
    }


    if(en)
    {
        var |= (1ul << RTCBS(RTC_CTRL,PRV_EN));
    }
    else
    {
        var &= ~(1ul << RTCBS(RTC_CTRL,PRV_EN));
    }

    ret = true;

    writel(var,base + RTCROFF(RTC_CTRL));

    rtc_clk_sync(base,CLK_SYNC);

    RTC_UNLOCK(base);

    return ret;
}

bool rtc_sec_access_enable(addr_t base,bool en)
{
    bool ret = true;

    RTC_LOCK(base);

    uint32_t var = readl(base + RTCROFF(RTC_CTRL));

    if(var & (1ul << RTCBS(RTC_CTRL,LOCK)))
    {
        /* locked, non-accessable */
        ret = false;
    }

    if(en)
    {
        var |= (1ul << RTCBS(RTC_CTRL,SEC_EN));
    }
    else
    {
        var &= ~(1ul << RTCBS(RTC_CTRL,SEC_EN));
    }

    ret = true;

    writel(var,base + RTCROFF(RTC_CTRL));

    rtc_clk_sync(base,CLK_SYNC);

    RTC_UNLOCK(base);

    return ret;
}

uint64_t rtc_get_tick(addr_t base)
{
    uint32_t tickl = readl(base + RTCROFF(RTC_L));
    uint32_t tickh = readl(base + RTCROFF(RTC_H));

    uint64_t tick = ((uint64_t)tickh << 32) | (uint64_t)tickl;

    return tick;
}

uint32_t rtc_set_tick(addr_t base, uint64_t tick)
{
    if(tick >= RTC_TICK_MAX)
        return 1;

    uint32_t ctrl = readl(base + RTCROFF(RTC_CTRL));

    if((ctrl & (1<<RTCBS(RTC_CTRL,EN))) && (ctrl & (1<<RTCBS(RTC_CTRL,LOCK))))
        return (uint32_t)(-2);
    else if(ctrl & (1<<RTCBS(RTC_CTRL,EN))){
        uint32_t v = ctrl & ~(1<<RTCBS(RTC_CTRL,EN));
        writel(v,base+ RTCROFF(RTC_CTRL));
        spin(300);
    }else if(ctrl & (1<<RTCBS(RTC_CTRL,LOCK)))
        return (uint32_t)(-3);

    writel((uint32_t)tick,base + RTCROFF(RTC_L));
    writel((uint32_t)(tick>>32),base+ RTCROFF(RTC_H));

    writel(ctrl,base + RTCROFF(RTC_CTRL));
    rtc_clk_sync(base, CLK_SYNC);

     return 0;
}

static void rtc_clk_sync(addr_t base, uint32_t rtc_clk)
{
    uint64_t rtc_sync_point = rtc_get_tick(base) + rtc_clk;

    while(rtc_get_tick(base) <= rtc_sync_point);
}

void rtc_config_adjust(addr_t base, uint8_t cycle, uint8_t dir)
{
    RTC_LOCK(base);

    uint32_t var = readl(base + RTCROFF(AUTO_ADJUST));

    if(var & (1ul << RTCBS(AUTO_ADJUST,EN)))
    {
        //disable if enable before
        writel(var &~ (1ul << RTCBS(AUTO_ADJUST,EN)), base + RTCROFF(AUTO_ADJUST));
        rtc_clk_sync(base,CLK_SYNC);
    }

    var &= ~(0xFFul << RTCBS(AUTO_ADJUST,CYCLE));
    var |= (((uint32_t)cycle) << RTCBS(AUTO_ADJUST,CYCLE));

    var &= ~(1ul << RTCBS(AUTO_ADJUST,DIR));
    var |= ((dir == 0 ? 0ul : 1ul) << RTCBS(AUTO_ADJUST,DIR));

    writel(var,base + RTCROFF(AUTO_ADJUST));
    rtc_clk_sync(base,CLK_SYNC);

    RTC_UNLOCK(base);
}

void rtc_adjust_enable(addr_t base, bool en)
{
    uint32_t var = readl(base + RTCROFF(AUTO_ADJUST));

    var &= ~(1ul << RTCBS(AUTO_ADJUST,EN));
    var |= ((en == false ? 0ul : 1ul) << RTCBS(AUTO_ADJUST,EN));

    RTC_LOCK(base);

    writel(var,base + RTCROFF(AUTO_ADJUST));
    rtc_clk_sync(base,CLK_SYNC);

    RTC_UNLOCK(base);
}


void rtc_wakeup_enable(addr_t base, bool en, bool irq_en, bool req_en)
{
    uint32_t var = readl(base + RTCROFF(WAKEUP_CTRL));
    uint32_t tmp = 0;

    var &= ~( (1ul << RTCBS(WAKEUP_CTRL, REQ_EN)) |
              (1ul << RTCBS(WAKEUP_CTRL, IRQ_EN)) |
              (1ul << RTCBS(WAKEUP_CTRL, EN)) |
              (1ul << RTCBS(WAKEUP_CTRL, STATUS)) |
              (1ul << RTCBS(WAKEUP_CTRL, CLEAR)) );

    RTC_LOCK(base);

    if(!en)
    {
        tmp = var;
        tmp |= (1ul << RTCBS(WAKEUP_CTRL, CLEAR));

        writel(tmp, base + RTCROFF(WAKEUP_CTRL));
        rtc_clk_sync(base, CLK_SYNC);
        writel(var, base + RTCROFF(WAKEUP_CTRL));
        rtc_clk_sync(base, CLK_SYNC);
    }
    else
    {
        var |= ( (1ul << RTCBS(WAKEUP_CTRL, EN)) |
                 ( (irq_en == true ? 1ul : 0ul) << RTCBS(WAKEUP_CTRL, IRQ_EN)) |
                 ( (req_en == true ? 1ul : 0ul) << RTCBS(WAKEUP_CTRL, REQ_EN)) );
        tmp = var;
        tmp |= (1ul << RTCBS(WAKEUP_CTRL, CLEAR));
    }

    writel(tmp, base + RTCROFF(WAKEUP_CTRL));
    rtc_clk_sync(base, CLK_SYNC);
    writel(var, base + RTCROFF(WAKEUP_CTRL));
    rtc_clk_sync(base, CLK_SYNC);

    RTC_UNLOCK(base);
}

void rtc_update_cmp_value(addr_t base, uint64_t increment)
{
    uint32_t var = readl(base + RTCROFF(WAKEUP_CTRL));
    uint32_t tmp = var;

    var &= ~(1ul << RTCBS(WAKEUP_CTRL, CLEAR));
    var &= ~(1ul << RTCBS(WAKEUP_CTRL, STATUS));

    /* disable irq/req/en bits. */
    tmp &= ~(1ul << RTCBS(WAKEUP_CTRL, REQ_EN));
    tmp &= ~(1ul << RTCBS(WAKEUP_CTRL, IRQ_EN));
    tmp &= ~(1ul << RTCBS(WAKEUP_CTRL, EN));
    /* write 1 to clear */
    tmp |= (1ul << RTCBS(WAKEUP_CTRL, CLEAR));

    RTC_LOCK(base);

    writel(tmp, base + RTCROFF(WAKEUP_CTRL));
    rtc_clk_sync(base, CLK_SYNC);

    uint64_t tick = rtc_get_tick(base);

    tick += increment;

    /* now we disable the wakeup control, safe to update. */
    writel((uint32_t)tick, base + RTCROFF(TIMER_L));
    writel((uint32_t)(tick >> 32), base + RTCROFF(TIMER_H));
    writel(var, base + RTCROFF(WAKEUP_CTRL));
    rtc_clk_sync(base, CLK_SYNC);

    RTC_UNLOCK(base);
}

/* period = 32768/(2^(div+1)) which div shall be from 0~15 */
bool rtc_config_periodic(addr_t base, uint8_t div)
{
    if(div > 0xF)
    {
        return false;
    }

    RTC_LOCK(base);

    uint32_t var = readl(base + RTCROFF(PERIODICAL_CTRL));

    if(var & (1ul << RTCBS(PERIODICAL_CTRL,IRQ_EN)))
    {
        /* disable first */
        writel(var &~ (1ul << RTCBS(PERIODICAL_CTRL,IRQ_EN)), base + RTCROFF(PERIODICAL_CTRL));
        rtc_clk_sync(base,CLK_SYNC);
    }

    var &= ~(0xFul << RTCBS(PERIODICAL_CTRL,FREQ));
    var |= (((uint32_t)div) << RTCBS(PERIODICAL_CTRL,FREQ));

    writel(var,base + RTCROFF(PERIODICAL_CTRL));
    rtc_clk_sync(base,CLK_SYNC);

    RTC_UNLOCK(base);

    return true;

}

void rtc_periodical_interrupt_enable(addr_t base, bool en)
{
    uint32_t var = readl(base + RTCROFF(PERIODICAL_CTRL));

    var &= ~(1ul << RTCBS(PERIODICAL_CTRL,IRQ_EN));
    var |= ((en == false ? 0ul : 1ul) << RTCBS(PERIODICAL_CTRL,IRQ_EN));

    RTC_LOCK(base);

    writel(var,base + RTCROFF(PERIODICAL_CTRL));
    rtc_clk_sync(base,CLK_SYNC);

    RTC_UNLOCK(base);
}

void rtc_vio_dis_interrupt_enable(addr_t base, bool en)
{
    uint32_t var = readl(base + RTCROFF(VIOLATION_INT));

    var &= ~(1ul << RTCBS(VIOLATION_INT,DIS_VIO_MASK));
    var |= ((en == false ? 1ul : 0ul) << RTCBS(VIOLATION_INT,DIS_VIO_MASK));

    RTC_LOCK(base);

    writel(var,base + RTCROFF(VIOLATION_INT));
    rtc_clk_sync(base,CLK_SYNC);

    RTC_UNLOCK(base);
}

void rtc_vio_ovf_interrupt_enable(addr_t base, bool en)
{
    uint32_t var = readl(base + RTCROFF(VIOLATION_INT));

    var &= ~(1ul << RTCBS(VIOLATION_INT,OVF_VIO_MASK));
    var |= ((en == false ? 1ul : 0ul) << RTCBS(VIOLATION_INT,OVF_VIO_MASK));

    RTC_LOCK(base);

    writel(var,base + RTCROFF(VIOLATION_INT));
    rtc_clk_sync(base,CLK_SYNC);

    RTC_UNLOCK(base);
}

bool rtc_check_wakeup_status(addr_t base)
{
    uint32_t var = readl(base + RTCROFF(WAKEUP_CTRL));

    if(var & (1ul << RTCBS(WAKEUP_CTRL,STATUS)))
    {
        return true;
    }
    else
    {
        return false;
    }

}

bool rtc_check_vio_dis_status(addr_t base)
{
    uint32_t var = readl(base + RTCROFF(VIOLATION_INT));

    if(var & (1ul << RTCBS(VIOLATION_INT,DIS_VIO_STATUS)))
    {
        return true;
    }
    else
    {
        return false;
    }

}

bool rtc_check_vio_ovf_status(addr_t base)
{
    uint32_t var = readl(base + RTCROFF(VIOLATION_INT));

    if(var & (1ul << RTCBS(VIOLATION_INT,OVF_VIO_STATUS)))
    {
        return true;
    }
    else
    {
        return false;
    }
}

void rtc_clr_wakeup_status(addr_t base)
{
    uint32_t var = readl(base + RTCROFF(WAKEUP_CTRL));

    var |= (1ul << RTCBS(WAKEUP_CTRL,CLEAR));

    RTC_LOCK(base);

    writel(var, base + RTCROFF(WAKEUP_CTRL));
    rtc_clk_sync(base,CLK_SYNC);

    var &= ~( (1ul << RTCBS(WAKEUP_CTRL,CLEAR)) | (1ul << RTCBS(WAKEUP_CTRL,STATUS)) );

    writel(var, base + RTCROFF(WAKEUP_CTRL));
    rtc_clk_sync(base,CLK_SYNC);

    RTC_UNLOCK(base);
}


void rtc_clr_vio_dis_status(addr_t base)
{
    uint32_t var1 = readl(base + RTCROFF(WAKEUP_CTRL));
    uint32_t var2 = readl(base + RTCROFF(VIOLATION_INT));

    var1 |= (1ul << RTCBS(WAKEUP_CTRL,DIS_VIO_CLR));

    RTC_LOCK(base);

    writel(var1, base + RTCROFF(WAKEUP_CTRL));
    rtc_clk_sync(base,CLK_SYNC);

    var1 &= ~(1ul << RTCBS(WAKEUP_CTRL,DIS_VIO_CLR));
    var2 &= ~(1ul << RTCBS(VIOLATION_INT,DIS_VIO_STATUS));

    writel(var2, base + RTCROFF(VIOLATION_INT));
    writel(var1, base + RTCROFF(WAKEUP_CTRL));
    rtc_clk_sync(base,CLK_SYNC);

    RTC_UNLOCK(base);
}

void rtc_clr_vio_ovf_status(addr_t base)
{
    uint32_t var1 = readl(base + RTCROFF(WAKEUP_CTRL));
    uint32_t var2 = readl(base + RTCROFF(VIOLATION_INT));

    var1 |= (1ul << RTCBS(WAKEUP_CTRL,OVF_VIO_CLR));

    RTC_LOCK(base);

    writel(var1, base + RTCROFF(WAKEUP_CTRL));
    rtc_clk_sync(base,CLK_SYNC);

    var1 &= ~(1ul << RTCBS(WAKEUP_CTRL,OVF_VIO_CLR));
    var2 &= ~(1ul << RTCBS(VIOLATION_INT,OVF_VIO_STATUS));

    writel(var2, base + RTCROFF(VIOLATION_INT));
    writel(var1, base + RTCROFF(WAKEUP_CTRL));
    rtc_clk_sync(base,CLK_SYNC);

    RTC_UNLOCK(base);
}

//*****************************************************************************
//
// Mark the end of the C bindings section for C++ compilers.
//
//*****************************************************************************
#ifdef __cplusplus
}
#endif

