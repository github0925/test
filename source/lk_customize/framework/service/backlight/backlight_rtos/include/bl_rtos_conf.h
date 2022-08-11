#ifndef _BACKLIGHT_RTOS_CFG_H_
#define _BACKLIGHT_RTOS_CFG_H_

#define BL_RTOS_DEBUG_LEVEL 1

#define BL_RTOS_PWM_FRQ 25000  //25k

#define BL_RTOS_PWM_MAX_DUTY 100

//api
int backlight_rtos_duty_set(int value);

#endif //_BACKLIGHT_RTOS_CFG_H_
