/*
 * pin.c
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

#define OP_NONE 0
#define OP_GET  1
#define OP_SET  2
#define OP_CTRL 3

#define NO_PULL   0x00
#define PULL_UP   0x03
#define PULL_DOWN 0x01

static void get_pin_info_per(uint32_t pin_index)
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
static void help(void)
{
    dprintf(ALWAYS, "pin -c pin_idx contrlerX   set pin to contrlerX\n"); 
    dprintf(ALWAYS, "pin -g pin_idx        get pin's status.\n");
    dprintf(ALWAYS, "pin -s pin_idx        func dir pull pad-mode	set pin's cfg.\n");
    dprintf(ALWAYS, "pin -s 104 0  1  0  0  pin=104 func=0~GPIO dir=1~out data=0~low pull=0~no-pull\n");
    dprintf(ALWAYS, "     |  |  |  |  |  |                                                    \n");
    dprintf(ALWAYS, "     |  |  |  |  |  \\--- pull(0~2): 0~no pull ; 1~pull up ; 2~pull down \n");
    dprintf(ALWAYS, "     |  |  |  |  \\------ data(0~1):  0~low ; 1~high                     \n");
    dprintf(ALWAYS, "     |  |  |  \\--------- dir(0~1): 0~input ; 1~output                   \n");
    dprintf(ALWAYS, "     |  |  \\------------ func(0):  0~GPIO                               \n");
    dprintf(ALWAYS, "     |  \\--------------- pin index(0~155) : pin index                   \n");
    dprintf(ALWAYS, "     \\------------------ operation: -s set ; -g get; -c ctrl(set controler)\n");
    dprintf(ALWAYS, "\n\n");
    dprintf(ALWAYS, "pad addr's bit0&bit1 is pull cfg:00~no 01~up 10~down\n");
    dprintf(ALWAYS, "mux addr's bit0~2 is func sel:000~GPIO 001~func1...\n");
    dprintf(ALWAYS, "pin addr's bit2 is output data sel:0~low 1~high\n");
    
}

static int set_pin_func(int argc, const cmd_args* argv)
{
    uint32_t pin_index = -1;
    uint32_t func  = 0;
    uint32_t dir   = 1;
    uint32_t data  = 1;
    uint32_t pull  = 0;

    switch(argc){
        case 3:
            pin_index = argv[2].u;
            break;
        case 4:
            pin_index = argv[2].u;
            func = argv[3].u;
            break;
        case 5:
            pin_index = argv[2].u;
            func = argv[3].u;
            dir = argv[4].u;
            break;
        case 6:
            pin_index = argv[2].u;
            func = argv[3].u;
            dir = argv[4].u;
            data = argv[5].u;
            break;
        case 7:
            pin_index = argv[2].u;
            func = argv[3].u;
            dir = argv[4].u;
            data = argv[5].u;
            pull = argv[6].u;
            break;
        default:
            break;
    }

    dprintf(ALWAYS, "func=%d\n", func);
    dprintf(ALWAYS, "dir=%s\n", (dir == 1)? "output":"input");
    dprintf(ALWAYS, "data=%s\n", (data == 1)? "high":"low");
    dprintf(ALWAYS, "pull=%d\n", pull);

    if( (pin_index > 156) || (func > 4) || (dir > 2) ||  (data > 2) ||  (pull > 2) ){
       dprintf(ALWAYS, "input param error.\n");
       return -1;
    }

    static void *some_port_handle;
    int ret;
    Port_PinModeType pin_mode;
    uint32_t input_select;
    uint32_t gpio_ctrl;
    int32_t gpio_config;

    ret = hal_port_creat_handle(&some_port_handle, g_iomuxc_res.res_id[0]);
    if (!ret) {
        printf("%s: create handle: %d\n", __func__, ret);
        return -1;
    }

    hal_port_get_pin_info(some_port_handle, pin_index, &pin_mode, &input_select, &gpio_ctrl, &gpio_config);

    pin_mode.pin_mux_config &= ~0x07;
    pin_mode.pin_mux_config |= (func & 0x07 );

    if (pin_index < 131) {
        pin_mode.io_pad_config &= ~0x03;
        uint32_t pull_reg = NO_PULL;
        switch(pull){
            case 0:
                pull_reg = NO_PULL;
                break;
            case 1:
                pull_reg = PULL_UP;
                break;
            case 2:
                pull_reg = PULL_DOWN;
                break;
            default:
                dprintf(ALWAYS, "error, pull param err\n");
        }
        pin_mode.io_pad_config |= (pull_reg & 0x03 );
    }else{
        dprintf(ALWAYS, "EMMC Pin. use other command to change this\n");
    }

    hal_port_set_pin_direction(&some_port_handle, (Port_PinType)pin_index, dir);
    hal_port_set_pin_mode(&some_port_handle, (Port_PinType)pin_index, pin_mode);
    hal_port_release_handle(&some_port_handle);

    hal_port_set_pin_data(&pin_mode, pin_index, data);  
    return 0;
}

//set pin_idx to X controler. X is 1~4.
static int set_gpioctrl(int argc, const cmd_args* argv)
{
    uint32_t pin_index = argv[2].u;
    uint32_t ctrl_num = argv[3].u;

    if( (pin_index > 156) || (ctrl_num == 0) || (ctrl_num > 4) ){
       dprintf(ALWAYS, "set_gpioctrl input param error.\n");
       return -1;
    }

    static void *some_port_handle;
    int ret;
    Port_PinModeType pin_mode;
    uint32_t input_select;
    uint32_t gpio_ctrl;
    int32_t gpio_config;

    ret = hal_port_creat_handle(&some_port_handle, g_iomuxc_res.res_id[0]);
    if (!ret) {
        printf("%s: create handle: %d\n", __func__, ret);
        return -1;
    }

    hal_port_get_pin_info(some_port_handle, pin_index, &pin_mode, &input_select, &gpio_ctrl, &gpio_config);

    hal_port_set_to_gpioctrl(some_port_handle, ctrl_num, pin_index);

    hal_port_get_pin_info(some_port_handle, pin_index, &pin_mode, &input_select, &ctrl_num, &gpio_config);

    hal_port_release_handle(&some_port_handle);
    return 0;
}

void gpio_info(int argc, const cmd_args* argv)
{
    uint32_t op=OP_NONE;

    if ( (strcmp(argv[0].str, "pin") != 0) || (argc < 2) ) {
        dprintf(ALWAYS, "command is error.\n");
        help();
        return;
    }

    if (strcmp(argv[1].str, "-g") == 0) { // -g get
        op = OP_GET;
        if (argc > 3) {
            help();
            return;
        }
    }else if (strcmp(argv[1].str, "-s") == 0) { // -s set
        op = OP_SET;
    }else if (strcmp(argv[1].str, "-c") == 0) {  // -c ctrl  set pin to controlerX
        op = OP_CTRL;
    }else{
        help();
        return;
    }

    uint32_t pin_index = argv[2].u;
    if(pin_index > 156){
        dprintf(ALWAYS, "command is wrong.\n");
        help();
        return;
    }

    dprintf(ALWAYS, "use get_pin_info %d to get more info \n",pin_index);

    if( op == OP_GET ){ 
        get_pin_info_per(pin_index);
    }else if( op == OP_SET ){
        set_pin_func(argc, argv);
    }else if( op == OP_CTRL ){
        set_gpioctrl(argc, argv);
    }

    dprintf(ALWAYS, "pad addr's bit0&bit1 is pull cfg:00~no 01~up 10~down\n");
    dprintf(ALWAYS, "mux addr's bit0~2 is func sel:000~GPIO 001~func1...\n");
    dprintf(ALWAYS, "gpio addr's bit2 is output data sel:0~low 1~high\n");
}

#if defined(WITH_LIB_CONSOLE)
STATIC_COMMAND_START
STATIC_COMMAND("pin", "get/set pin's config", (console_cmd)&gpio_info)
STATIC_COMMAND_END(pin);
#endif