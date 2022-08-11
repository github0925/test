#ifndef __GPIOIRQ_H__
#define __GPIOIRQ_H__

#include <platform/interrupts.h>

extern int unmask_gpio_interrupt(uint16_t ChannelId);
extern int mask_gpio_interrupt(uint16_t ChannelId);
extern int register_gpio_int_handler(uint16_t ChannelId, int irqflag,
                                     int_handler handler, void *arg);
#endif

