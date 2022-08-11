#include "led.h"
#include "hal_port.h"
#include "hal_dio.h"
#include <chip_res.h>
#include <platform.h>

static uint32_t led_count = 2;
extern const domain_res_t g_iomuxc_res;
extern const domain_res_t g_gpio_res;
void *led_gpio_handle;

static const Port_PinModeType PIN_GPIO_A10_GREEN = {
    ((uint32_t)PORT_PAD_POE__DISABLE | PORT_PAD_IS__OUT | PORT_PAD_SR__FAST | PORT_PAD_DS__MID1 | PORT_PIN_IN_NO_PULL ),
    ((uint32_t)PORT_PIN_MUX_FV__MIN | PORT_PIN_MUX_FIN__MIN | PORT_PIN_OUT_PUSHPULL | PORT_PIN_MODE_GPIO),
};
static const Port_PinModeType PIN_GPIO_A11_RED = {
    ((uint32_t)PORT_PAD_POE__DISABLE | PORT_PAD_IS__OUT | PORT_PAD_SR__FAST | PORT_PAD_DS__MID1 | PORT_PIN_IN_NO_PULL ),
    ((uint32_t)PORT_PIN_MUX_FV__MIN | PORT_PIN_MUX_FIN__MIN | PORT_PIN_OUT_PUSHPULL | PORT_PIN_MODE_GPIO),
};

#define GREEN_LED PortConf_PIN_GPIO_A10
#define RED_LED   PortConf_PIN_GPIO_A11

static Port_PinType pins_leds[] = {PortConf_PIN_GPIO_A10,
                                   PortConf_PIN_GPIO_A11
                                  };

void led_configure(void)
{
    //cfg pin
    void *led_handle;
    hal_port_creat_handle(&led_handle, g_iomuxc_res.res_id[0]);

    hal_port_set_pin_mode(led_handle, GREEN_LED, PIN_GPIO_A10_GREEN);
    hal_port_set_pin_mode(led_handle, RED_LED, PIN_GPIO_A11_RED);

    spin(200);
    hal_port_release_handle(&led_handle);

    hal_dio_creat_handle(&led_gpio_handle, g_gpio_res.res_id[0]);
    hal_dio_set_channel_direction(&led_gpio_handle, GREEN_LED, DIO_CHANNEL_OUT);
    hal_dio_set_channel_direction(&led_gpio_handle, RED_LED, DIO_CHANNEL_OUT);
    hal_dio_write_channel(&led_gpio_handle, GREEN_LED, 1);
    hal_dio_write_channel(&led_gpio_handle, RED_LED, 1);
    hal_dio_release_handle(&led_gpio_handle);

}

uint32_t led_set(uint32_t led)
{
    if (led >= led_count)
        return 0;

    hal_dio_write_channel(&led_gpio_handle, pins_leds[led], 1);
    return 1;
}

uint32_t led_clear(uint32_t led)
{
    if (led >= led_count)
        return 0;

    hal_dio_write_channel(&led_gpio_handle, pins_leds[led], 0);

    return 1;

}

uint32_t led_toggle(uint32_t led, uint32_t value)
{
    if ( led >= led_count)
        return 0;

    hal_dio_write_channel(&led_gpio_handle, pins_leds[led], value);
    return 1;
}
