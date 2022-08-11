//*****************************************************************************
//
// hal_port.h - Prototypes for the Port hal
//
// Copyright (c) 2019 Semidrive Semiconductor.  All rights reserved.
// Software License Agreement
//
//
//*****************************************************************************

#ifndef __PORT_HAL_H__
#define __PORT_HAL_H__
//*****************************************************************************
//
// If building with a C++ compiler, make all of the definitions in this header
// have a C binding.
//
//*****************************************************************************
#ifdef __cplusplus
extern "C" {
#endif

#include <kernel/mutex.h>
#include <platform/interrupts.h>
#include "chip_res.h"
#include "res.h"
#include "system_cfg.h"
#include "Port.h"

typedef enum gpio_ctrl {
    GPIO_CTRL_1 = 1,
    GPIO_CTRL_2 = 2,
    GPIO_CTRL_3 = 3,
    GPIO_CTRL_4 = 4,
    GPIO_CTRL_5 = 5
}gpio_ctrl_t;

typedef enum board_type {
    CORE_BOARD = 0,
    BASE_BOARD = 1
} board_type_t;

typedef struct delta_config_head {
    uint32_t version;	    // struct version
    unsigned short hw_id;   // hardware id
    unsigned short hw_type; // board type, 0: core board, 1: base board
    uint32_t size;          // config size
    uint32_t config_offset; // config offset, it base on delta config partition
} delta_config_head_t;

/*
 * Function: port creat handle api
 * Arg     : port device handle, port id
 * Return  : true on Success, false on failed
 */
bool hal_port_creat_handle(void **handle, uint32_t port_res_glb_idx);

/*
 * Function: port release handle api
 * Arg     : port device handle
 * Return  : true on Success, false on failed
 */
bool hal_port_release_handle(void **handle);

/*
 * Function: search all ports resource api
 * Arg     : port device handle
 * Arg     : port resource global index
 * Return  : true on Success, false on failed
 */
bool hal_port_check_res(void *handle, uint32_t port_res_glb_idx);

/*
 * Function: port init api, alloc device struct memory
 * Arg     : port device handle
 * Return  : 0 on Success, non zero on failure
 */
int hal_port_init(void *handle);

/*
 * Function: port init api with delta configuration
 * Arg     : port device handle
 * Arg     : hardware id of board
 * Arg     : board type, core board or base board
 * Return  : 0 on Success, non zero on failure
 */
int hal_port_init_delta(void *handle, board_type_t board_type, uint32_t hwid);

/*
 * Function: port set pin direction
 * Arg     : port device handle, pin number, direction
 * Return  : 0 on Success, non zero on failure
 */
int hal_port_set_pin_direction(void *handle, const Port_PinType pin,
                               const Port_PinDirectionType direction);

/*
 * Function: port refresh port direction
 * Arg     : void
 * Return  : 0 on Success, non zero on failure
 */
int hal_port_refresh_port_direction(void *handle);

/*
 * Function: port set pin mode
 * Arg     : port device handle, pin number, pin mode
 * Return  : 0 on Success, non zero on failure
 */
int hal_port_set_pin_mode(void *handle, const Port_PinType pin,
                          const Port_PinModeType mode);

/*
 * Function: port set pin to gpio controller
 * Arg     : port device handle, gpio controller, pin number
 * Return  : 0 on Success, non zero on failure
 */
int hal_port_set_to_gpioctrl(void *handle, const gpio_ctrl_t gpio_ctrl,
                          const Port_PinType pin);


int hal_port_set_pin_data(Port_PinModeType *pin_mode, const Port_PinType pin_num,
                int32_t data);

/*
 * Function : getting pin info
 * pin_num  : pin index
 * pin_mode : pin pad/iomux configuration
 * gpio_ctrl: gpio controller index, valid for GPIO
 * direction: gpio input/output configuration, valid for GPIO
 * level    : default level, valid for GPIO
*/
int hal_port_get_pin_info(void *handle, const Port_PinType pin_num,
    Port_PinModeType *pin_mode, uint32_t * input_select,
    uint32_t * gpio_ctrl, int32_t * gpio_config);

/*
 * Function : init level2 mux config
 * mux      :  mux value, PORT_MUX_DISP - mux to disp, or mux to canfd
*/
void hal_port_init_disp_canfd_mux(void *handle, port_disp_canfd_mux_t mux);

//*****************************************************************************
//
// Mark the end of the C bindings section for C++ compilers.
//
//*****************************************************************************
#ifdef __cplusplus
}
#endif
#endif // __PORT_HAL_H__
