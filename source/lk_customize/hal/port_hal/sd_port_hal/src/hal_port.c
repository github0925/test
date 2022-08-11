/*
 * hal_port.c
 *
 * Copyright (c) 2019 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: port/iomux driver.
 *
 * Revision History:
 * -----------------
 * 0.1, 9/18/2019 init version
 */

#include <assert.h>
#include <debug.h>
#include <platform.h>
#include <stdint.h>
#include <stdlib.h>
#include <trace.h>
#include <system_configs_parse.h>

#include <port_cfg.h>
#include "hal_port.h"

#if ENABLE_PIN_DELTA_CONFIG
#if NOT_USE_SYS_CFG
#include <assembly/pin_cfg_delta.h>
#endif
#endif

#define LOCAL_TRACE 0

extern const domain_res_t g_gpio_res;

spin_lock_t port_spin_lock = SPIN_LOCK_INITIAL_VALUE;

static struct port_handle s_port_handle;

bool hal_port_creat_handle(void **handle, uint32_t port_res_glb_idx)
{
    struct port_handle *p_handle;
    int8_t ret = 0;
    paddr_t phy_addr = 0;
    int32_t real_idx = 0;
    paddr_t dio_phy_addr = 0;
    int32_t dio_real_idx = 0;
    spin_lock_saved_state_t states;
    LTRACEF("+hal_port_creat_handle \n");

    ret = res_get_info_by_id(port_res_glb_idx, &phy_addr, &real_idx);

    if (ret != -1) {
        LTRACEF("hal_port_creat_handle: phy_addr[0x%lx], real_idx[%d]\n", phy_addr,
               real_idx);
    }
    else {
        printf("hal_port_creat_handle: res_get_info_by_id for port failed! 03\n");
        return false;
    }

    /* To get GPIO base address for Port */
    ret = res_get_info_by_id(g_gpio_res.res_id[0], &dio_phy_addr, &dio_real_idx);

    if (ret != -1) {
        LTRACEF("hal_port_creat_handle: dio_phy_addr[0x%lx], dio_real_idx[%d]\n",
                dio_phy_addr, dio_real_idx);
    }
    else {
        printf("hal_port_creat_handle: res_get_info_by_id for dio failed! 03\n");
        return false;
    }

#if 0
    // FIXME: How to used the domain_res.h to check valid or not ?
    /* get all IOMUXC res phy_addr & real_idx */
    printf("hal_port_creat_handle: get all IOMUX RES...\n");

    for (i = 0; i < IOMUXC_RES_NUM; i++) {
        // to fix g_res_cat unused-variable warning, zhuming, 191118
        cat_id = (g_iomuxc_res.res_id[i] >> 17) & 0x7F;
        res_num = g_res_cat[cat_id]->res_num;
        printf("IOMUXC res_num[%d]\n", res_num);

        ret = res_get_info_by_id(g_iomuxc_res.res_id[i], &phy_addr, &real_idx);

        if (ret == 0) {
            printf("IOMUXC: i[%d], phy_addr[0x%lx], real_idx[%d]\n", i, phy_addr,
                   real_idx);
        }
        else {
            printf("IOMUXC: res_get_info_by_id failed!\n");
            return false;
        }
    }

#endif

    p_handle = &s_port_handle;
    spin_lock_irqsave(&port_spin_lock, states);
    p_handle->phy_addr = phy_addr;
    p_handle->real_idx = real_idx;
    p_handle->dio_phy_addr = dio_phy_addr;
    p_handle->dio_real_idx = dio_real_idx;
    *handle = p_handle;
    Port_SetHandle((void *)p_handle);
    spin_unlock_irqrestore(&port_spin_lock, states);


    LTRACEF("-hal_port_creat_handle finished\n");

    return true;
}

bool hal_port_release_handle(void **handle)
{
    ASSERT(handle);
    struct port_handle *port = *handle;
    spin_lock_saved_state_t states;

    spin_lock_irqsave(&port_spin_lock, states);
    port->phy_addr = 0;
    port->real_idx = -1;
    *handle = NULL;
    spin_unlock_irqrestore(&port_spin_lock, states);

    return true;
}

bool hal_port_check_res(void *handle, uint32_t port_res_glb_idx)
{
    int8_t ret = 0;
    paddr_t phy_addr = 0;
    int32_t real_idx = 0;

    ret = res_get_info_by_id(port_res_glb_idx, &phy_addr, &real_idx);

    if (ret != -1) {
        LTRACEF("IOMUXC: phy_addr[0x%lx], real_idx[0x%x]\n", phy_addr, real_idx);
    }
    else {
        printf("IOMUXC: res_get_info_by_id failed! 02\n");
    }

    return true;
}

int hal_port_init(void *handle)
{
    ASSERT(handle);

    Port_ConfigType port_config;

#if NOT_USE_SYS_CFG
    port_config = (Port_ConfigType){&Port_kConfiguration[0]};
#else
    //check port configs source
    addr_t addr_config = 0;

    uint32_t res = get_config_info(MODULE_PORT_CFG, &addr_config);
    if (0 != res) {
        dprintf(CRITICAL, "get config info fail.\n");
        port_config = (Port_ConfigType){&Port_kConfiguration[0]};
    }
    else {
        LTRACEF("hal_port_init use binary config.\n");
        port_config = (Port_ConfigType){(Port_n_ConfigType*)(void*)addr_config};
    }
#endif

    LTRACEF("hal_port_init \n");
    Port_Init(&port_config);

    return true;
}

int hal_port_init_delta(void *handle, board_type_t board_type, uint32_t hwid)
{
#if ENABLE_PIN_DELTA_CONFIG
    ASSERT(handle);

    //check port configs source
    addr_t addr_config = 0;
    config_header_t *config_header;
    addr_t delta_header_addr;
    delta_config_head_t *delta_header;

#if NOT_USE_SYS_CFG
    addr_config = (addr_t)&pin_cfg_delta[0];
    config_header = (config_header_t*)addr_config;
    delta_header_addr = config_header->config_offset + addr_config;
#else

    uint32_t res = get_config_info(MODULE_PIN_CFG_DELTA, &addr_config);
    if (0 != res) {
        LTRACEF("get config info fail.\n");
        return false;
    }
    else {
        LTRACEF("hal_port_init use binary config.\n");
        config_header = (config_header_t*)addr_config;

        delta_header_addr = config_header->config_offset + addr_config;
    }
#endif

    delta_header = (delta_config_head_t*)delta_header_addr;

    for (uint32_t i = 0; i < config_header->config_count; i++) {
        if ((board_type == delta_header->hw_type) && hwid == delta_header->hw_id) {
            port_init_delta((port_delta_config_t *)((delta_header->config_offset) + addr_config),
                delta_header->size / sizeof(port_delta_config_t));

            return true;
        }

        delta_header++;
    }

    return false;

#endif

    return true;
}

int hal_port_set_pin_direction(void *handle, const Port_PinType pin,
                               const Port_PinDirectionType direction)
{
    ASSERT(handle);
    Port_SetPinDirection(pin, direction);

    return true;
}

int hal_port_refresh_port_direction(void *handle)
{
    ASSERT(handle);
    Port_RefreshPortDirection();

    return true;
}

int hal_port_set_pin_mode(void *handle, const Port_PinType pin,
                          const Port_PinModeType mode)
{
    ASSERT(handle);
    Port_SetPinMode(pin, mode);

    return true;
}

int hal_port_set_to_gpioctrl(void *handle, const gpio_ctrl_t gpio_ctrl,
                          const Port_PinType pin)
{
    ASSERT(handle);
    Port_SetToGPIOCtrl(gpio_ctrl, pin);

    return true;
}

int hal_port_set_pin_data(Port_PinModeType *pin_mode, const Port_PinType pin_num,
                int32_t data)
{
    port_set_pin_data(pin_mode, pin_num, data);
    return true;
}

int hal_port_get_pin_info(void *handle, const Port_PinType pin,
    Port_PinModeType *pin_mode, uint32_t * input_select,
    uint32_t * gpio_ctrl, int32_t * gpio_config)
{
    ASSERT(handle);
    port_get_pin_info(pin, pin_mode, input_select, gpio_ctrl, gpio_config);

    return true;
}

void hal_port_init_disp_canfd_mux(void *handle, port_disp_canfd_mux_t mux)
{
    ASSERT(handle);
    port_init_disp_canfd_mux(mux);
}
