/*
 * target_init.c
 *
 * Copyright (c) 2019 Semidrive Semiconductor.
 * All rights reserved.
 */
#include <debug.h>
#include <reg.h>
#include <target.h>
#include <stdio.h>
#include "hal_port.h"
#ifdef WITH_HAL_MODULE_HELPER_HAL
#include <module_helper_hal.h>
#endif
#ifdef SSYSTEM_DISABLE_UART_IRQ
#include <uart_hal.h>
#include <target_res.h>
#endif
#if defined(ENABLE_WIFI) || defined(ENABLE_BT)
#include "tca9539.h"
#endif
extern const domain_res_t g_iomuxc_res;

static const Port_PinModeType uart10_tx = {
    ((uint32_t)PORT_PAD_POE__DISABLE | PORT_PAD_IS__OUT | PORT_PAD_SR__FAST
     | PORT_PAD_DS__MID1 | PORT_PIN_IN_PULL_DOWN),
    ((uint32_t)PORT_PIN_MUX_FV__MIN | PORT_PIN_MUX_FIN__MIN | PORT_PIN_OUT_PUSHPULL
     | PORT_PIN_MODE_ALT1),
};

static const Port_PinModeType uart10_rx = {
    ((uint32_t)PORT_PAD_POE__DISABLE | PORT_PAD_IS__OUT | PORT_PAD_SR__FAST
     | PORT_PAD_DS__MID1 | PORT_PIN_IN_PULL_DOWN),
    ((uint32_t)PORT_PIN_MUX_FV__MIN | PORT_PIN_MUX_FIN__MIN | PORT_PIN_OUT_PUSHPULL
     | PORT_PIN_MODE_ALT1),
};

static void uart_port_config(void)
{
    void *g_handle;
    bool ret;
    ret = hal_port_creat_handle(&g_handle, g_iomuxc_res.res_id[0]);

    if (!ret) {
        printf("create prot hal failed.\n");
        return;
    }

    hal_port_set_pin_mode(g_handle, PortConf_PIN_GPIO_C6, uart10_tx);
    hal_port_set_pin_mode(g_handle, PortConf_PIN_GPIO_C7, uart10_rx);
    hal_port_release_handle(&g_handle);
}
#ifdef NEED_CHANGE_VOLTAGE
#include "i2c_hal.h"
#include <boardinfo_hwid_usr.h>
#ifndef ARRAYSIZE
#define ARRAYSIZE(A) ((int)(sizeof(A)/sizeof((A)[0])))
#endif

const Port_PinModeType MODE_GPIO_D2_M1_I2C10_SCL = {
    ((uint32_t)PORT_PAD_POE__DISABLE | PORT_PAD_IS__OUT | PORT_PAD_SR__FAST | PORT_PAD_DS__MID1 | PORT_PIN_IN_NO_PULL),
    ((uint32_t)PORT_PIN_MUX_FV__MIN | PORT_PIN_MUX_FIN__MIN  | PORT_PIN_OUT_PUSHPULL | PORT_PIN_MODE_ALT1),
};

const Port_PinModeType MODE_GPIO_D3_M1_I2C10_SDA = {
    ((uint32_t)PORT_PAD_POE__DISABLE | PORT_PAD_IS__OUT | PORT_PAD_SR__FAST | PORT_PAD_DS__MID1 | PORT_PIN_IN_NO_PULL),
    ((uint32_t)PORT_PIN_MUX_FV__MIN | PORT_PIN_MUX_FIN__MIN  | PORT_PIN_OUT_PUSHPULL | PORT_PIN_MODE_ALT1),
};

void i2c10_port_config(void)
{
    static void *i2c10_port_handle;
    bool ioret;
    // Port setup
    ioret = hal_port_creat_handle(&i2c10_port_handle, g_iomuxc_res.res_id[0]);

    if (!ioret) {
        return;
    }

    ioret = hal_port_set_pin_mode(i2c10_port_handle, PortConf_PIN_GPIO_D2,
                                  MODE_GPIO_D2_M1_I2C10_SCL);
    ioret = hal_port_set_pin_mode(i2c10_port_handle, PortConf_PIN_GPIO_D3,
                                  MODE_GPIO_D3_M1_I2C10_SDA);
    hal_port_release_handle(&i2c10_port_handle);
}

static uint8_t calculate_voltage(int ap_volt_mv)
{
    if (ap_volt_mv >= 600 && ap_volt_mv <= 730) {
        return 0xa + ((ap_volt_mv - 600) + 9) / 10;
    }
    else if (ap_volt_mv > 730 && ap_volt_mv <= 1400) {
        return 0x18 + ((ap_volt_mv - 735) + 4) / 5;
    }
    else if (ap_volt_mv > 1400 && ap_volt_mv <= 3360) {
        return 0x9e + ((ap_volt_mv - 1420) + 19) / 20;
    }
    else {
        ASSERT(0);
    }

    return 0;
}

/*
 * @brief   change DDR voltages
 * @para    vdd_id
 *                  1 - VDD2(1.1v by default), SW_B1
 *                  2 - VDDQ(0.6v by default), SW_B2
 *                  3 - VDD1(1.8v by default), SW_B3
 * @para    mv  voltage value to be set, in mv.
 */
#if CFG_DDR_VOLTAGE_ADJUST
int setup_ddr_voltage(uint32_t vdd_id, uint32_t mv)
{
    int ret = 1;
    void *i2c_handle = NULL;
    uint8_t pmic_addr[2] = {0x60, 0x61};
    uint8_t pmic1_reg_data[][2] = {
        {0x0c, 97},   //  SW_B1, 1.1v by default.
        {0x0e, 10},   //  SW_B2, 0.6v by default.
        {0x10, 177},  //  SW_B3, 1.8v by default.
    };
    uint8_t read_reg_data = 0;

    if (vdd_id >= 3) {
        return -1;
    }

    vdd_id -= 1;
    pmic1_reg_data[vdd_id][1] = calculate_voltage(mv);
    hal_i2c_creat_handle(&i2c_handle, RES_I2C_I2C10);

    if (i2c_handle != NULL) {
        uint8_t pgood_ctl1 = 0, reg_addr = 0x28;
        hal_i2c_read_reg_data(i2c_handle, pmic_addr[1], &reg_addr, 1, &pgood_ctl1,
                              1); /* PGOOD_CTRL1 */
        /* Disable PGOOD monitor when change the BULK's voltage, otherwise PGOOD
         * de-asserted and may reset the board */
        uint8_t v = pgood_ctl1 & (~(0x03u << ((vdd_id + 1) * 2)));
        hal_i2c_write_reg_data(i2c_handle, pmic_addr[1], &reg_addr, 1, &v, 1);
        hal_i2c_write_reg_data(i2c_handle, pmic_addr[1],
                               (void *)&pmic1_reg_data[vdd_id][0], 1, (void *)&pmic1_reg_data[vdd_id][1],
                               1);
        /* Confirm data been written correctly */
        hal_i2c_read_reg_data(i2c_handle, pmic_addr[1],
                              (void *)&pmic1_reg_data[vdd_id][0], 1, (void *)&read_reg_data, 1);

        if (read_reg_data != pmic1_reg_data[vdd_id][1]) {
            printf("%s: Opps, 0x%x written but read back as 0x%x\n",
                   __FUNCTION__, pmic1_reg_data[vdd_id][1], read_reg_data);
            ret = -2;
        }
        else {
            ret = 0;
        }

        spin(1000 * 20);    // Wait a while for voltage ramp
        /* restore pgood ctrl1 reg */
        reg_addr = 0x28;
        hal_i2c_write_reg_data(i2c_handle, pmic_addr[1], &reg_addr, 1, &pgood_ctl1,
                               1);
        hal_i2c_release_handle(i2c_handle);
    }
    else {
        //printf("no i2c 10 on this domain");
    }

    return ret;
}
#endif  /* CFG_DDR_VOLTAGE_ADJUST */

static int change_voltage(int ap_volt_mv, int gpu_volt_mv, int cpu_volt_mv)
{
    int ret = 1;
    int i = 0;
    void *i2c_handle = NULL;
    uint8_t pmic_addr[2] = {0x60, 0x61};
    uint8_t pmic1_reg_data[][2] = {
        {0x0a, 0x1b}, //ap
        {0x0e, 0x2f}, //gpu
        {0x10, 0x2f}, //cpu
        {0x06, 0x96}
    }; //gpu ctrl
    uint8_t read_reg_data = 0;
    pmic1_reg_data[0][1] = calculate_voltage(ap_volt_mv);
    pmic1_reg_data[1][1] = calculate_voltage(gpu_volt_mv);
    pmic1_reg_data[2][1] = calculate_voltage(cpu_volt_mv);
    hal_i2c_creat_handle(&i2c_handle, RES_I2C_I2C10);

    if (i2c_handle != NULL) {
        i2c_app_config_t i2c_conf = hal_i2c_get_busconfig(i2c_handle);
        i2c_conf.poll = 1;
        hal_i2c_set_busconfig(i2c_handle, &i2c_conf);
        for (i = 0; i < ARRAYSIZE(pmic1_reg_data); i++) {
            hal_i2c_write_reg_data(i2c_handle, pmic_addr[0],
                                   (void *)&pmic1_reg_data[i][0], 1, (void *)&pmic1_reg_data[i][1], 1);
            hal_i2c_read_reg_data(i2c_handle, pmic_addr[0],
                                  (void *)&pmic1_reg_data[i][0], 1, (void *)&read_reg_data, 1);
            //printf("change_vdd_ap_voltage read_reg_data device_addr=0x%x reg=0x%x, data = 0x%x\n",pmic_addr[0],pmic1_reg_data[i][0],read_reg_data);

            if (read_reg_data != pmic1_reg_data[i][1]) {
                ret = 0;
                break;
            }
        }

        hal_i2c_release_handle(i2c_handle);
    }
    else {
        //printf("no i2c 10 on this domain");
    }

    return ret;
}

#endif
void target_early_init(void)
{
#ifdef WITH_HAL_MODULE_HELPER_HAL
    module_helper_init();
#endif
    uart_port_config();
#ifdef NEED_CHANGE_VOLTAGE
    i2c10_port_config();

    if (get_part_id(PART_REV) == 1)
        change_voltage(800, 850, 850);
    else
        change_voltage(720, 850, 850);

#endif
}

void target_init(void)
{
}
#ifdef ENABLE_BT
static void config_bt_pin(int level)
{
    struct tca9539_device *pd;
    printf("\n%s: \n", __func__);
    pd = tca9539_init(12, 0x76);

    if (pd == NULL) {
        printf("init tca9359 error!\n");
        return ;
    }
    tca9539_enable_i2cpoll(pd);

    pd->ops.output_enable(pd, 5);
    pd->ops.output_val(pd, 5, level);
    tca9539_deinit(pd);
    printf("\n%s: end\n", __func__);
}
#endif
#ifdef ENABLE_WIFI
static void config_wifi_pin(int level)
{
    struct tca9539_device *pd;
    printf("\n%s: \n", __func__);
    pd = tca9539_init(12, 0x76);

    if (pd == NULL) {
        printf("init tca9359 error!\n");
        return ;
    }
    tca9539_enable_i2cpoll(pd);

    pd->ops.output_enable(pd, 6);
    pd->ops.output_val(pd, 6, level);
    tca9539_deinit(pd);
    printf("\n%s: end\n", __func__);
}
#endif

#ifdef SSYSTEM_DISABLE_UART_IRQ
static void uart_release(void)
{
    void *handle;
    bool ret = hal_uart_creat_handle(&handle, DEBUG_COM);

    if (!ret) {
        dprintf(CRITICAL, "create uart handle fail.\n");
        return;
    }

    hal_uart_irq_mask(handle);
    hal_uart_release_handle(handle);
}
#endif
extern void uart_exit(void);
void target_quiesce(void)
{
#ifdef SSYSTEM_DISABLE_UART_IRQ
    uart_release();
    uart_exit();
#endif
#ifdef ENABLE_BT
    config_bt_pin(1);
#endif
#ifdef ENABLE_WIFI
    config_wifi_pin(1);
#endif
}


