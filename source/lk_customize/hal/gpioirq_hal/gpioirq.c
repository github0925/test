#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "gpioirq.h"
#include "hal_dio.h"

#define GPIO_PORT_SIZE 32

struct gpio_int_handler {
    uint16_t ChannelId;
    int irqflag;
    int_handler handler;
    void *arg;
};

extern const domain_res_t g_gpio_res;
static struct gpio_int_handler *int_handler_table[160];

struct gpiochip_irq_info {
    int gpiochip_irq_num;
    spin_lock_t gpiochip_lock;
    bool enable_flag;
    uint32_t enable_bitmap;
};

static struct gpiochip_irq_info ginfo[5] = {
    {IRQ_GIC1_GPIO1_GPIO_INT0_NUM, SPIN_LOCK_INITIAL_VALUE, false, 0},
    {IRQ_GIC1_GPIO1_GPIO_INT1_NUM, SPIN_LOCK_INITIAL_VALUE, false, 0},
    {IRQ_GIC1_GPIO1_GPIO_INT2_NUM, SPIN_LOCK_INITIAL_VALUE, false, 0},
    {IRQ_GIC1_GPIO1_GPIO_INT3_NUM, SPIN_LOCK_INITIAL_VALUE, false, 0},
    {IRQ_GIC1_GPIO1_GPIO_INT4_NUM, SPIN_LOCK_INITIAL_VALUE, false, 0},
};

mutex_t gpio_int_mutex;
static bool mutex_flag = false;

static enum handler_return gpiochip_int_handler(void *data)
{
    int port = (int)data;

    for (int id = port * GPIO_PORT_SIZE; id < (port + 1) * GPIO_PORT_SIZE; id++) {
        if (int_handler_table[id] && hal_dio_get_irq_status(id)) {
            int_handler_table[id]->handler(int_handler_table[id]->arg);
        }
    }

    return INT_RESCHEDULE;
}

static void gpiochip_int_init(int port)
{
    if (ginfo[port].enable_flag)
        return;

    register_int_handler(ginfo[port].gpiochip_irq_num, gpiochip_int_handler, (void *)port);
    ginfo[port].enable_flag = true;
}

int register_gpio_int_handler(uint16_t ChannelId, int irqflag,
                              int_handler handler, void *arg)
{
    void *gpio_handler = NULL;

    if (!handler) {
        dprintf(ALWAYS, "%s, handler invalid %d\n", __func__);
        return -1;
    }

    if (!mutex_flag) {
        mutex_init(&gpio_int_mutex);
        mutex_flag = true;
    }

    mutex_acquire(&gpio_int_mutex);

    if (int_handler_table[ChannelId]) {
        dprintf(ALWAYS, "%s, gpio irq %u already registered\n", __func__, ChannelId);
        goto done;
    }

    int_handler_table[ChannelId] = malloc(sizeof(struct gpio_int_handler));

    if (!int_handler_table[ChannelId]) {
        dprintf(ALWAYS, "%s, alloc mem for gpio irq %u fail\n", __func__, ChannelId);
        goto done;
    }

    memset(int_handler_table[ChannelId], 0, sizeof(struct gpio_int_handler));
    int_handler_table[ChannelId]->ChannelId = ChannelId;
    int_handler_table[ChannelId]->irqflag = irqflag;
    int_handler_table[ChannelId]->handler = handler;
    int_handler_table[ChannelId]->arg = arg;

    if(!hal_dio_creat_handle(&gpio_handler, g_gpio_res.res_id[0]))
        goto done;

    if (!hal_dio_config_irq(ChannelId, irqflag)) {
        dprintf(ALWAYS, "%s, config gpio irq %u fail,irqflag=%d\n", __func__,
                ChannelId, irqflag);
        free(int_handler_table[ChannelId]);
        int_handler_table[ChannelId] = NULL;
        goto done1;
    }

    hal_dio_release_handle(&gpio_handler);

    gpiochip_int_init(ChannelId / GPIO_PORT_SIZE);
    mutex_release(&gpio_int_mutex);
    return 0;

done1:
    hal_dio_release_handle(&gpio_handler);
done:
    mutex_release(&gpio_int_mutex);
    return -1;
}

int unmask_gpio_interrupt(uint16_t ChannelId)
{
    int port = ChannelId / GPIO_PORT_SIZE;
    int bitoffset = ChannelId % GPIO_PORT_SIZE;
    spin_lock_saved_state_t state;

    if (int_handler_table[ChannelId]
            && int_handler_table[ChannelId]->ChannelId == ChannelId) {
        hal_dio_enable_irq(ChannelId);

        spin_lock_irqsave(&ginfo[port].gpiochip_lock, state);

        if (!ginfo[port].enable_bitmap) {
            ginfo[port].enable_bitmap |= 0x1 << bitoffset;
            unmask_interrupt(ginfo[port].gpiochip_irq_num);
        }
        else
            ginfo[port].enable_bitmap |= 0x1 << bitoffset;

        spin_unlock_irqrestore(&ginfo[port].gpiochip_lock, state);

        return 0;
    }

    dprintf(ALWAYS, "unmask gpio irq %u fail\n", ChannelId);
    return -1;
}

int mask_gpio_interrupt(uint16_t ChannelId)
{
    int port = ChannelId / GPIO_PORT_SIZE;
    int bitoffset = ChannelId % GPIO_PORT_SIZE;
    spin_lock_saved_state_t state;

    if (int_handler_table[ChannelId]
            && int_handler_table[ChannelId]->ChannelId == ChannelId) {
        hal_dio_disable_irq(ChannelId, int_handler_table[ChannelId]->irqflag);

        spin_lock_irqsave(&ginfo[port].gpiochip_lock, state);

        ginfo[port].enable_bitmap &= ~(0x1 << bitoffset);

        if (!ginfo[port].enable_bitmap)
            mask_interrupt(ginfo[port].gpiochip_irq_num);

        spin_unlock_irqrestore(&ginfo[port].gpiochip_lock, state);

        return 0;
    }

    dprintf(ALWAYS, "mask gpio irq %u fail\n", ChannelId);
    return -1;
}

