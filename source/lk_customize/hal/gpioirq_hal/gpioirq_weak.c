#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <debug.h>
#include "gpioirq.h"

int register_gpio_int_handler(uint16_t ChannelId, int irqflag,
                              int_handler handler, void *arg)
{
    dprintf(ALWAYS, "%s, not implement\n", __func__);
    return -1;
}

int unmask_gpio_interrupt(uint16_t ChannelId)
{
    dprintf(ALWAYS, "%s, not implement\n", __func__);
    return -1;
}

int mask_gpio_interrupt(uint16_t ChannelId)
{
    dprintf(ALWAYS, "%s, not implement\n", __func__);
    return -1;
}

