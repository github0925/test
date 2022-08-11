#include "hal_port.h"
#include "hal_dio.h"
#include <chip_res.h>
#include <platform.h>
#include <lib/console.h>
#include <stdio.h>
#include <string.h>

extern const domain_res_t g_gpio_res;
extern const domain_res_t g_iomuxc_res;

static const Port_PinModeType PIN_GPIO_B4_CANFD2_STB = {
    ((uint32_t)PORT_PAD_POE__DISABLE | PORT_PAD_IS__OUT | PORT_PAD_SR__FAST | PORT_PAD_DS__MID1 | PORT_PIN_IN_NO_PULL ),
    ((uint32_t)PORT_PIN_MUX_FV__MIN | PORT_PIN_MUX_FIN__MIN | PORT_PIN_OUT_PUSHPULL | PORT_PIN_MODE_GPIO),
};
static const Port_PinModeType PIN_GPIO_A0_CANFD2_EN = {
    ((uint32_t)PORT_PAD_POE__DISABLE | PORT_PAD_IS__OUT | PORT_PAD_SR__FAST | PORT_PAD_DS__MID1 | PORT_PIN_IN_NO_PULL ),
    ((uint32_t)PORT_PIN_MUX_FV__MIN | PORT_PIN_MUX_FIN__MIN | PORT_PIN_OUT_PUSHPULL | PORT_PIN_MODE_GPIO),
};

void *canfd2_stb_dio_handle;

void can2_trcv_init(void)
{
    void *canfd2_stb_port_handle;
    /* port set up */
    hal_port_creat_handle(&canfd2_stb_port_handle, g_iomuxc_res.res_id[0]);

    hal_port_set_pin_mode(canfd2_stb_port_handle, PortConf_PIN_GPIO_B4, PIN_GPIO_B4_CANFD2_STB);
    hal_port_set_pin_mode(canfd2_stb_port_handle, PortConf_PIN_GPIO_A0, PIN_GPIO_A0_CANFD2_EN);
    spin(200);

    hal_port_release_handle(&canfd2_stb_port_handle);

    hal_dio_creat_handle(&canfd2_stb_dio_handle, g_gpio_res.res_id[0]);

    hal_dio_set_channel_direction(&canfd2_stb_dio_handle, PortConf_PIN_GPIO_B4, DIO_CHANNEL_OUT);
    hal_dio_set_channel_direction(&canfd2_stb_dio_handle, PortConf_PIN_GPIO_A0, DIO_CHANNEL_OUT);

    hal_dio_write_channel(&canfd2_stb_dio_handle, PortConf_PIN_GPIO_B4, 1);
    hal_dio_write_channel(&canfd2_stb_dio_handle, PortConf_PIN_GPIO_A0, 1);

    hal_dio_release_handle(&canfd2_stb_dio_handle);
}

void can2_trcv_sleep(void)
{
    hal_dio_write_channel(&canfd2_stb_dio_handle, PortConf_PIN_GPIO_B4, 0);
    /* Typ. hold time in Go-to-Sleep command: 25us */
    spin(25);
}

void can2_trcv_wakeup(void)
{
    hal_dio_write_channel(&canfd2_stb_dio_handle, PortConf_PIN_GPIO_B4, 1);
    hal_dio_write_channel(&canfd2_stb_dio_handle, PortConf_PIN_GPIO_A0, 1);
}

static int can_str(int argc, const cmd_args *argv)
{
    if (!strcmp(argv[1].str, "help")) {
        dprintf(0, "can_str config/sleep/exit \n");
        return 0;
    } else if (!strcmp(argv[1].str, "config")) {
        dprintf(0, "can_str config\n");
        can2_trcv_init();
        return 0;
    } else if (!strcmp(argv[1].str, "sleep")) {
        dprintf(0, "can_str sleep\n");
        can2_trcv_sleep();
        return 0;
    } else if (!strcmp(argv[1].str, "exit")) {
        dprintf(0, "can_str exit sleep\n");
        can2_trcv_wakeup();
    }
        return 0;
}
#if defined(WITH_LIB_CONSOLE)
STATIC_COMMAND_START
STATIC_COMMAND("can_str", "can_str config/sleep/exit", (console_cmd)&can_str)
STATIC_COMMAND_END(can_str);
#endif