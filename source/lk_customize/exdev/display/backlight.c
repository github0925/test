#include <hal_port.h>
#include <hal_dio.h>

const Port_PinType PIN_GPIO_D8_M0_GPIO_D8 = PortConf_PIN_GPIO_D8;
const Port_PinModeType MODE_GPIO_D8_M0_GPIO_D8 = {
    ((uint32_t)PORT_PAD_POE__DISABLE | PORT_PAD_IS__OUT | PORT_PAD_SR__FAST | PORT_PAD_DS__MID1 | PORT_PIN_IN_NO_PULL),
    ((uint32_t)PORT_PIN_MUX_FV__MIN | PORT_PIN_MUX_FIN__MIN  | PORT_PIN_OUT_PUSHPULL | PORT_PIN_MODE_GPIO),
};

const Port_PinType PIN_GPIO_D9_M0_GPIO_D9 = PortConf_PIN_GPIO_D9;
const Port_PinModeType MODE_GPIO_D9_M0_GPIO_D9 = {
    ((uint32_t)PORT_PAD_POE__DISABLE | PORT_PAD_IS__OUT | PORT_PAD_SR__FAST | PORT_PAD_DS__MID1 | PORT_PIN_IN_NO_PULL),
    ((uint32_t)PORT_PIN_MUX_FV__MIN | PORT_PIN_MUX_FIN__MIN  | PORT_PIN_OUT_PUSHPULL | PORT_PIN_MODE_GPIO),
};

const Port_PinModeType MODE_GPIO_C14_M0_GPIO = {
    ((uint32_t)PORT_PAD_POE__ENABLE | PORT_PAD_IS__OUT | PORT_PAD_SR__FAST | PORT_PAD_DS__MID1 | PORT_PIN_IN_PULL_UP),
    ((uint32_t)PORT_PIN_MUX_FV__MIN | PORT_PIN_MUX_FIN__MIN  | PORT_PIN_OUT_PUSHPULL | PORT_PIN_MODE_GPIO),
};

extern const domain_res_t g_iomuxc_res;
extern const domain_res_t g_gpio_res;

void backlight_enable(void)
{
    static void *some_port_handle;
    static void *some_dio_handle;
    bool ret;
    /* Port setup */
    ret = hal_port_creat_handle(&some_port_handle, g_iomuxc_res.res_id[0]);
    printf("%s: create handle: %d\n", __func__, ret);
    ret = hal_port_set_pin_mode(some_port_handle, PIN_GPIO_D8_M0_GPIO_D8,
                                MODE_GPIO_D8_M0_GPIO_D8);
    ret = hal_port_set_pin_mode(some_port_handle, PIN_GPIO_D9_M0_GPIO_D9,
                                MODE_GPIO_D9_M0_GPIO_D9);
    hal_port_release_handle(&some_port_handle);

    /* Dio setup */
    ret = hal_dio_creat_handle(&some_dio_handle, g_gpio_res.res_id[0]);
    hal_dio_set_channel_direction(some_dio_handle, PIN_GPIO_D8_M0_GPIO_D8, DIO_CHANNEL_OUT);
    hal_dio_set_channel_direction(some_dio_handle, PIN_GPIO_D9_M0_GPIO_D9, DIO_CHANNEL_OUT);
    hal_dio_write_channel(some_dio_handle, PIN_GPIO_D8_M0_GPIO_D8, 1);
    hal_dio_write_channel(some_dio_handle, PIN_GPIO_D9_M0_GPIO_D9, 1);
    hal_dio_release_handle(&some_dio_handle);
}

