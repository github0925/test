#include <app.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <debug.h>
#include <platform.h>
#include <lib/console.h>
#include <arch.h>
#include "timer_hal.h"
#include "hal_port.h"
#include "hal_dio.h"
#include "led.h"
#include <chip_res.h>
#include <platform.h>
#include "uart_hal.h"
#include "gpioirq.h"

#if WITH_KERNEL_VM
#include <kernel/vm.h>
#endif

#define DEBOUNCE_TIME   500
#define RES_TIME_CNT    RES_TIMER_TIMER2
#define NUM_LEDS    2
volatile uint8_t chquit = 0;

static void *_tchnd;

extern const domain_res_t g_iomuxc_res;
extern const domain_res_t g_gpio_res;

extern void uart_update_rx_cbk(hal_uart_int_callback cbk);
volatile bool led_status[NUM_LEDS];
static const Port_PinModeType PIN_GPIO_A16_KEY = {
    ((uint32_t)PORT_PAD_POE__DISABLE | PORT_PAD_IS__OUT | PORT_PAD_SR__FAST | PORT_PAD_DS__MID1 | PORT_PIN_IN_NO_PULL ),
    ((uint32_t)PORT_PIN_MUX_FV__MIN | PORT_PIN_MUX_FIN__MIN | PORT_PIN_OUT_PUSHPULL | PORT_PIN_MODE_ALT2),
};

#define PINKEY PortConf_PIN_GPIO_B4

static hal_timer_glb_cfg_t _glb_cfg = {
    .clk_sel = HAL_TIMER_SEL_LF_CLK,
    .clk_frq = 24000000,
    .clk_div = 24000, //23999?
    .cascade = false,
};

static hal_timer_ovf_cfg_t _ovf_cfg = {
    .periodic = true,
    .cnt_val = 0,
    .ovf_val = 250, //249?
};

static void process_btn_evt(uint8_t bt)
{
    if (bt >= NUM_LEDS)
        return;

    led_status[bt] = !led_status[bt];

    if (bt < NUM_LEDS) {
        if (led_status[bt])
            led_clear(bt);
        else
            led_set(bt);
    }
}

#ifdef PINS_PUSHBUTTONS
static enum handler_return gpio_handler(void *arg)
{
    uint8_t *key = (uint8_t *)arg;
    process_btn_evt(*key);
    printf("enter gpio_handler\r\n");
    return INT_NO_RESCHEDULE;
}
static void configure_buttons(void)
{
    void *key_handle;
    void *key_gpio_handle;
    hal_port_creat_handle(&key_handle, g_iomuxc_res.res_id[0]);

    hal_port_set_pin_mode(key_handle, PINKEY, PIN_GPIO_A16_KEY);
    spin(200);
    hal_port_release_handle(&key_handle);

    hal_dio_creat_handle(&key_gpio_handle, g_gpio_res.res_id[0]);
    hal_dio_set_channel_direction(&key_gpio_handle, PINKEY, DIO_CHANNEL_OUT);
    hal_dio_config_irq(PINKEY,
                       IRQ_TYPE_LEVEL_LOW | DIO_IRQ_TYPE_LEVEL_HIGH | DIO_IRQ_TYPE_EDGE_BOTH);
    register_gpio_int_handler(PINKEY,
                              IRQ_TYPE_LEVEL_LOW | DIO_IRQ_TYPE_LEVEL_HIGH | DIO_IRQ_TYPE_EDGE_BOTH,
                              gpio_handler, NULL);
    hal_dio_enable_irq(PINKEY);
    hal_dio_release_handle(&key_gpio_handle);

}
#endif


enum handler_return _tc_handler(void)
{
    int i = 0;

    for (i = 0; i < NUM_LEDS; ++i) {
        led_toggle(i, led_status[i]);
        led_status[i] = !led_status[i];
        printf("%i ", (unsigned int)i);
    }

    return 0;
}

void _config_timer(void)
{
    hal_timer_creat_handle(&_tchnd, RES_TIME_CNT);

    if (_tchnd == NULL) {
        printf("Failed to create timer handle!\r\n");
        return;
    }

    hal_timer_global_init(_tchnd, &_glb_cfg);
    hal_timer_ovf_init(_tchnd, HAL_TIMER_LOCAL_A, &_ovf_cfg);
    hal_timer_int_src_enable(_tchnd, HAL_TIMER_CNT_LA_OVF_INT_SRC);
    hal_timer_int_cbk_register(_tchnd, HAL_TIMER_CNT_LA_OVF_INT_SRC, _tc_handler);
}
static int console_rx_handler(uint8_t data)
{
    if ((data >= '0') & (data <= '9'))
        process_btn_evt(data - '0');
    else if (data == 's') {
        hal_timer_int_src_disable(_tchnd, HAL_TIMER_CNT_LA_OVF_INT_SRC);
    }
    else if (data == 'b') {
        hal_timer_int_src_enable(_tchnd, HAL_TIMER_CNT_LA_OVF_INT_SRC);
    }
    else if (data == 'q') {
        chquit = 'q';
    }

    return 0;

}
void gettingstart_main(int argc, const cmd_args *argv)
{
    chquit = 0;
    led_configure();
    uart_update_rx_cbk(console_rx_handler);
    _config_timer();

    int i = 0;

    led_status[0] = true;

    for (i = 1; i < NUM_LEDS; i++)
        led_status[i] = led_status[i - 1];

    printf("Getting started demo\r\n");

#ifdef PINS_PUSHBUTTONS
    configure_buttons();
#else
    printf("Use console key 0 to %d. \r\n", NUM_LEDS - 1);
#endif
    printf("Press the number of the led to make it "
           "start or stop blinking.\n\r");
    printf("Press 's' to stop the TC and 'b' to start it\r\n");

    while (1) {
        while (chquit != 'q');

        hal_timer_int_src_disable(_tchnd, HAL_TIMER_CNT_LA_OVF_INT_SRC);
        uart_update_rx_cbk(NULL);
        printf("quit getting_started demo\r\n");
        break;

    }
}

#if defined(WITH_LIB_CONSOLE)
STATIC_COMMAND_START
STATIC_COMMAND("gettingstart", "getting start demo",
               (console_cmd)&gettingstart_main)
STATIC_COMMAND_END(gettingst);
#endif