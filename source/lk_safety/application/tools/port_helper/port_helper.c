/*
 * hal_port.c
 *
 * Copyright (c) 2019 Semidrive Semiconductor.
 * All rights reserved.
 */

#include <assert.h>
#include <lib/console.h>
#include <debug.h>
#include <platform.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <hal_port.h>

extern const domain_res_t g_iomuxc_res;


void get_pin_info_per(uint32_t pin_index)
{
    static void *some_port_handle;
    int ret;
    Port_PinModeType pin_mode;
    uint32_t input_select;
    uint32_t gpio_ctrl;
    int32_t gpio_config;

    ret = hal_port_creat_handle(&some_port_handle, g_iomuxc_res.res_id[0]);
    if (!ret) {
        printf("%s: create handle: %d\n", __func__, ret);
        return;
    }

    hal_port_get_pin_info(some_port_handle, pin_index, &pin_mode, &input_select, &gpio_ctrl, &gpio_config);

    hal_port_release_handle(&some_port_handle);

    if (0 == ((pin_mode.pin_mux_config) & 0x7)) {
        dprintf(ALWAYS, "\npin[%d] is set to GPIO, info as following:\n", pin_index);
        dprintf(ALWAYS, "   GPIO controller: GPIO%d\n", gpio_ctrl);
        dprintf(ALWAYS, "   Direction: %s\n", (gpio_config & 0x1) ? "output" : "input");
        if (gpio_config & 0x1) {
            dprintf(ALWAYS, "   Level: %s\n", (gpio_config & 0x4) ? "High" : "Low");
        }
    }
    else {
        dprintf(ALWAYS, "\npin is set to functoin%d, info as following:\n", (pin_mode.pin_mux_config) & 0x7);
        if (pin_index > 131) {
            dprintf(ALWAYS, "   Driver strength for P(SP):");
            switch(((pin_mode.io_pad_config) >> 20) & 0xf) {
                case 0xf:
                    dprintf(ALWAYS, "33 ohm\n");
                    break;
                case 0xc:
                    dprintf(ALWAYS, "40 ohm\n");
                    break;
                case 0x0:
                    dprintf(ALWAYS, "100 ohm\n");
                    break;
                default:
                    dprintf(ALWAYS, "50/66 ohm\n");
            }

            dprintf(ALWAYS, "   Driver strength for N(SN):");
            switch(((pin_mode.io_pad_config) >> 16) & 0xf) {
                case 0xf:
                    dprintf(ALWAYS, "33 ohm\n");
                    break;
                case 0xc:
                    dprintf(ALWAYS, "40 ohm\n");
                    break;
                case 0x0:
                    dprintf(ALWAYS, "100 ohm\n");
                    break;
                default:
                    dprintf(ALWAYS, "50/66 ohm\n");
            }

            dprintf(ALWAYS, "   Pad input select(RXSEL):");
            switch(((pin_mode.io_pad_config) >> 12) & 0x7) {
                case 0x0:
                    dprintf(ALWAYS, "Receiver disabled\n");
                    break;
                case 0x2:
                    dprintf(ALWAYS, "3.3V Schmitt Trigger\n");
                    break;
                case 0x7:
                    dprintf(ALWAYS, "Loop_back mode\n");
                    break;
                default:
                    dprintf(ALWAYS, "Reserved\n");
            }

            dprintf(ALWAYS, "   PAD Slew-rate control for P(TXPREP): %s\n",
                    ((pin_mode.io_pad_config) >> 8) & 0xf ? "Fastest": "Slowest");

            dprintf(ALWAYS, "   PAD Slew-rate control for N(TXPREN): %s\n",
                    ((pin_mode.io_pad_config) >> 4) & 0xf ? "Fastest": "Slowest");
        }
        else {
            dprintf(ALWAYS, "   PAD input select(Input Mode): %s\n",
                    ((pin_mode.io_pad_config) >> 12) & 0x1 ? "Schmitt": "CMOS");

            dprintf(ALWAYS, "   PAD Slew-rate(Slew Rate): %s\n",
                    ((pin_mode.io_pad_config) >> 8) & 0x1 ? "Slow": "Fast");

            dprintf(ALWAYS, "   PAD Driver select(Driver Strength):");
            switch(((pin_mode.io_pad_config) >> 4) & 0x3) {
                case 0x0:
                    dprintf(ALWAYS, "2mA\n");
                    break;
                case 0x1:
                    dprintf(ALWAYS, "4mA\n");
                    break;
                case 0x2:
                    dprintf(ALWAYS, "8mA\n");
                    break;
                case 0x3:
                    dprintf(ALWAYS, "12mA\n");
                    break;
                default:
                    dprintf(ALWAYS, "Error\n");
            }
        }
    }

    dprintf(ALWAYS, "   Pull Down/Up: ");
    switch ((pin_mode.io_pad_config) & 0x3)  {
        case 0:
            dprintf(ALWAYS, "No Pull\n");
            break;
        case 1:
            if (pin_index > 131) {
                dprintf(ALWAYS, "Pull Up\n");
            }
            else {
                dprintf(ALWAYS, "Pull Down\n");
            }
            break;
        case 2:
            if (pin_index > 131) {
                dprintf(ALWAYS, "Pull Down\n");
            }
            else {
                dprintf(ALWAYS, "No Pull\n");
            }
            break;
        case 3:
            if (pin_index > 131) {
                dprintf(ALWAYS, "Reserved\n");
            }
            else {
                dprintf(ALWAYS, "Pull Up\n");
            }
            break;
        default:
            dprintf(ALWAYS, "Reserved\n");
    }

    dprintf(ALWAYS, "   PAD Mode: %s\n", (pin_mode.pin_mux_config) & 0x10 ? "Open-Drain": "Push-Pull");

    dprintf(ALWAYS, "\nRegisters values as following:\n");
    dprintf(ALWAYS, "   PAD config value 0x%x\n", pin_mode.io_pad_config);
    dprintf(ALWAYS, "   Mux config value 0x%x\n", pin_mode.pin_mux_config);
    if (0 == ((pin_mode.pin_mux_config) & 0x7)) {
        dprintf(ALWAYS, "   GPIO%d controller config value 0x%x\n", gpio_ctrl, gpio_config);
    }
}

void get_pin_info(int argc, const cmd_args* argv)
{
    static size_t len;

    if (strcmp(argv[0].str, "get_pin_info") != 0) {
        dprintf(ALWAYS, "command is wrong.\n");
        return;
    }

    if (argc < 2 && len == 0) {
        dprintf(ALWAYS, "traversal all pins.\n");
        for (uint32_t i = 0; i < 156; i++) {
            get_pin_info_per(i);
            dprintf(ALWAYS, " \n\n");
        }
    }
    else {
        uint32_t pin_index = argv[1].u;
        get_pin_info_per(pin_index);
    }
}

#if defined(WITH_LIB_CONSOLE)
STATIC_COMMAND_START
STATIC_COMMAND("get_pin_info", "get pin info", (console_cmd)&get_pin_info)
//STATIC_COMMAND("j2mp", "j2mp", (console_cmd)&j2mp)
STATIC_COMMAND_END(port_helper);
#endif
