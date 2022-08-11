#ifndef __RTC_REGS_DEF_H__
#define __RTC_REGS_DEF_H__

/*------------ registers offset definition. ------------*/

#define RTCROFF(_name) RTCROFF_##_name##_OFFSET

#define RTCROFF_RTC_CTRL_OFFSET        (0x00)
#define RTCROFF_RTC_H_OFFSET           (0x04)
#define RTCROFF_RTC_L_OFFSET           (0x08)
#define RTCROFF_AUTO_ADJUST_OFFSET     (0x0C)
#define RTCROFF_TIMER_H_OFFSET         (0x10)
#define RTCROFF_TIMER_L_OFFSET         (0x14)
#define RTCROFF_WAKEUP_CTRL_OFFSET     (0x18)
#define RTCROFF_PERIODICAL_CTRL_OFFSET (0x1C)
#define RTCROFF_VIOLATION_INT_OFFSET   (0x20)



/*------------ bit shift definition. ------------*/
#define RTCBS(_reg_name,_bit_name) RTCBS_##_reg_name##_##_bit_name##_SHIFT
/* RTC_CTRL */
#define RTCBS_RTC_CTRL_LOCK_SHIFT        (31)
#define RTCBS_RTC_CTRL_PRV_EN_SHIFT      (2)
#define RTCBS_RTC_CTRL_SEC_EN_SHIFT      (1)
#define RTCBS_RTC_CTRL_EN_SHIFT          (0)

/* RTC_H */
#define RTCBS_RTC_H_V_SHIFT              (0) //[15:0]

/* RTC_L */
#define RTCBS_RTC_L_V_SHIFT              (0)//[31:0]

/* AUTO_ADJUST */
#define RTCBS_AUTO_ADJUST_CYCLE_SHIFT    (8) //[15:8]
#define RTCBS_AUTO_ADJUST_DIR_SHIFT      (1) //0-increase; 1-decrease
#define RTCBS_AUTO_ADJUST_EN_SHIFT       (0)

/* TIMER_H */
#define RTCBS_TIMER_H_V_SHIFT            (0) //[15:0]

/* TIMER_L */
#define RTCBS_TIMER_L_V_SHIFT            (0)//[31:0]

/* WAKEUP_CTRL */
#define RTCBS_WAKEUP_CTRL_DIS_VIO_CLR_SHIFT  (6)
#define RTCBS_WAKEUP_CTRL_OVF_VIO_CLR_SHIFT  (5)
#define RTCBS_WAKEUP_CTRL_CLEAR_SHIFT        (4) //write 1 to clear IRQ status
#define RTCBS_WAKEUP_CTRL_STATUS_SHIFT       (3)
#define RTCBS_WAKEUP_CTRL_EN_SHIFT           (2)
#define RTCBS_WAKEUP_CTRL_IRQ_EN_SHIFT       (1)
#define RTCBS_WAKEUP_CTRL_REQ_EN_SHIFT       (0)



/* PERIODICAL_CTRL */
#define RTCBS_PERIODICAL_CTRL_FREQ_SHIFT     (3) // periodical irq freq = 32768/(2^(FREQ+1)) [6:3]
#define RTCBS_PERIODICAL_CTRL_IRQ_EN_SHIFT   (0)

/* VIOLATION_INT */
#define RTCBS_VIOLATION_INT_DIS_VIO_STATUS_SHIFT (3)
#define RTCBS_VIOLATION_INT_OVF_VIO_STATUS_SHIFT (2)
#define RTCBS_VIOLATION_INT_DIS_VIO_MASK_SHIFT   (1)
#define RTCBS_VIOLATION_INT_OVF_VIO_MASK_SHIFT   (0)


#endif