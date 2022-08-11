/*
 * func_dio.c
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: ii4 Test App.
 *
 * Revision History:
 * -----------------
 */
#include <stdio.h>
#include <string.h>
#include "hal_port.h"
#include "hal_dio.h"
#include "board_start.h"
#include "board_cfg.h"
#include "func_dio.h"
#include "func_can.h"
#include "func_i2c.h"
#include "remote_test.h"
#include "Port.h"

#define get_gpio_legal_value_(x) (x)?(x=1):(x=0)

extern const domain_res_t g_iomuxc_res;
extern const domain_res_t g_gpio_res;

static void *g_handle;
static void *dio_handle;
#if 0
//pwm_3 as the output pin  gpio_c12 and gpio_c13
const Port_PinModeType mode_GPIO_OUT = {
    ((uint32_t)PORT_PAD_POE__DISABLE | PORT_PAD_IS__OUT | PORT_PAD_SR__FAST | PORT_PAD_DS__MID1 | PORT_PIN_IN_PULL_DOWN ),
    ((uint32_t)PORT_PIN_MUX_FV__MIN | PORT_PIN_MUX_FIN__MIN | PORT_PIN_OUT_PUSHPULL | PORT_PIN_MODE_GPIO),
};

//cpt_3 as the input pin gpio_c2 and gpio_c3
const Port_PinModeType mode_GPIO_IN_FL = {
    ((uint32_t)PORT_PAD_POE__DISABLE | PORT_PAD_IS__IN | PORT_PAD_SR__FAST | PORT_PAD_DS__MID1 | PORT_PIN_IN_NO_PULL ),
    ((uint32_t)PORT_PIN_MUX_FV__MIN | PORT_PIN_MUX_FIN__MIN | PORT_PIN_OUT_OPENDRAIN | PORT_PIN_MODE_GPIO),
};

const Port_PinModeType mode_GPIO_IN_DN = {
    ((uint32_t)PORT_PAD_POE__DISABLE | PORT_PAD_IS__IN | PORT_PAD_SR__FAST | PORT_PAD_DS__MID1 | PORT_PIN_IN_PULL_DOWN ),
    ((uint32_t)PORT_PIN_MUX_FV__MIN | PORT_PIN_MUX_FIN__MIN | PORT_PIN_OUT_OPENDRAIN | PORT_PIN_MODE_GPIO),
};

const Port_PinModeType mode_EMMC_GPIO_IN = {
    ((uint32_t)PORT_PAD_MMC_SP__MIN | PORT_PAD_MMC_SN__MIN | PORT_PAD_MMC_RXSEL__IN | PORT_PAD_MMC_TXPREP__MIN | PORT_PAD_MMC_TXPREN__MIN | PORT_PAD_MMC_PULL__OFF ),
    ((uint32_t)PORT_PIN_MUX_FV__MIN | PORT_PIN_MUX_FIN__MIN | PORT_PIN_OUT_OPENDRAIN | PORT_PIN_MODE_GPIO),
};

const Port_PinModeType mode_EMMC_GPIO_OUT = {
    ((uint32_t)PORT_PAD_MMC_SP__MIN | PORT_PAD_MMC_SN__MIN | PORT_PAD_MMC_RXSEL__MIN | PORT_PAD_MMC_TXPREP__MIN | PORT_PAD_MMC_TXPREN__MIN | PORT_PAD_MMC_PULL__OFF ),
    ((uint32_t)PORT_PIN_MUX_FV__MIN | PORT_PIN_MUX_FIN__MIN | PORT_PIN_OUT_PUSHPULL | PORT_PIN_MODE_GPIO),
};
#endif
//special pin_160 for satety domain
const Port_PinModeType mode_SAFETY_SEPCIAL_GPIO_IN = {
    ((uint32_t)PORT_PAD_POE__DISABLE | PORT_PAD_IS__IN | PORT_PAD_SR__FAST | PORT_PAD_DS__MID1 | PORT_PIN_IN_NO_PULL),
    ((uint32_t)PORT_PIN_MUX_FV__MIN  | PORT_PIN_MUX_FIN__MIN | PORT_PIN_OUT_OPENDRAIN | PORT_PIN_MODE_ALT1),
};

const gpio_value_table_t eth_gpio_int_value_table[] = {

    {0x0, GPIO_C11, AP_DOMAIN, COM_GPIO, DEFAULT},//2122_INT_1
    {0x1, GPIO_C9,  AP_DOMAIN, COM_GPIO, DEFAULT},//2122_INT_2
    {0x2, GPIO_C14, AP_DOMAIN, COM_GPIO, DEFAULT},//2122_INT_3
    {0x3, GPIO_C15, AP_DOMAIN, COM_GPIO, DEFAULT},//2122_INT_4
    {0x4, GPIO_D15, AP_DOMAIN, COM_GPIO, DEFAULT},//"5072_INTn",
    {0x5, GPIO_C4,  AP_DOMAIN, COM_GPIO, DEFAULT} //"5050_INTN",
};

const gpio_value_table_t gpio_write_value_table[] = {

    {0x01, GPIOB_4,               SAFETY_DOMAIN, COM_GPIO,  DEFAULT},// "BOOT_MODE_AND_LIN_CS",
    //{0x02, GPIOB_5,             SAFETY_DOMAIN, COM_GPIO,  DEFAULT},//"SAFETY_RESET_AP_IO",
    {0x04, GPIOE_1,               SAFETY_DOMAIN, COM_GPIO,  DEFAULT},//"PMIC_POFF_IN_SAFETY",
    {0x05, GPIOE_3,               SAFETY_DOMAIN, COM_GPIO,  DEFAULT},//"PMIC_WDI_SAFETY",
    {0x06, GPIO_D14,              AP_DOMAIN,     COM_GPIO,  DEFAULT},//"5072_RESET",
    {0x07, GPIO_D13,              AP_DOMAIN,     COM_GPIO,  DEFAULT},//"5050_RESET",
    {0x08, GPIO_C7,               AP_DOMAIN,     COM_GPIO,  DEFAULT},//"5072_2122_RESET",
    {0x09, GPIO_C5,               AP_DOMAIN,     COM_GPIO,  DEFAULT},//"5050_2112_RESET",
    {0x0a, GPIO_D8,               AP_DOMAIN,     COM_GPIO,  DEFAULT},//"5072_WAKE",
    {0x0b, GPIO_D7,               AP_DOMAIN,     COM_GPIO,  DEFAULT},//"5050_WAKE",
    {0x0c, GPIO_D6,               AP_DOMAIN,     COM_GPIO,  DEFAULT},//PCA9539PW
    {0x0d, GPIO_E0,               SAFETY_DOMAIN, COM_GPIO,  DEFAULT},//PCA9539PW
    {0x0e, CANFD1_EN_SAFETY,      SAFETY_DOMAIN, COM_GPIO,  DEFAULT},//"CANFD1_EN_AP",
    {0x0f, CANFD1_STDBY_SAFETY,   SAFETY_DOMAIN, COM_GPIO,  DEFAULT},//"CANFD1_STDBY_AP",
    {0x10, CANFD2_EN_SAFETY,      SAFETY_DOMAIN, COM_GPIO,  DEFAULT},//"CANFD2_EN_AP",
    {0x11, CANFD2_STDBY_SAFETY,   SAFETY_DOMAIN, COM_GPIO,  DEFAULT},//"CANFD2_STDBY_AP",
    {0x12, CANFD3_EN_SAFETY,      SAFETY_DOMAIN, COM_GPIO,  DEFAULT},//"CANFD3_EN_AP",
    {0x13, CANFD3_STDBY_SAFETY,   SAFETY_DOMAIN, COM_GPIO,  DEFAULT},//"CANFD3_STDBY_AP",
    {0x14, CANFD4_EN_SAFETY,      SAFETY_DOMAIN, COM_GPIO,  DEFAULT},//"CANFD4_EN_AP",
    {0x15, CANFD4_STDBY_SAFETY,   SAFETY_DOMAIN, COM_GPIO,  DEFAULT},//"CANFD4_STDBY_AP",
    {0x16, CANFD9_EN_AP,          SAFETY_DOMAIN, COM_GPIO,  DEFAULT},//"CANFD9_EN_AP",
    {0x17, CANFD9_STDBY_AP,       SAFETY_DOMAIN, COM_GPIO,  DEFAULT},//"CANFD9_STDBY_AP",
    {0x18, CANFD10_EN_AP,         SAFETY_DOMAIN, COM_GPIO,  DEFAULT},//"CANFD10_EN_AP",
    {0x19, CANFD10_STDBY_AP,      SAFETY_DOMAIN, COM_GPIO,  DEFAULT},//"CANFD10_STDBY_AP",
    {0x1a, CANFD11_EN_AP,         SAFETY_DOMAIN, COM_GPIO,  DEFAULT},//"CANFD11_EN_AP",
    {0x1b, CANFD11_STDBY_AP,      SAFETY_DOMAIN, COM_GPIO,  DEFAULT},//"CANFD11_STDBY_AP",
    {0x1c, CANFD12_EN_AP,         SAFETY_DOMAIN, COM_GPIO,  DEFAULT},//"CANFD12_EN_AP",
    {0x1d, CANFD12_STDBY_AP,      SAFETY_DOMAIN, COM_GPIO,  DEFAULT},//"CANFD12_STDBY_AP",
    {0x1e, CANFD16_EN_AP,         SAFETY_DOMAIN, COM_GPIO,  DEFAULT},//"CANFD16_EN_AP",
    {0x1f, CANFD16_STDBY_AP,      SAFETY_DOMAIN, COM_GPIO,  DEFAULT},//"CANFD16_STDBY_AP",
    {0x20, CANFD17_EN_AP,         SAFETY_DOMAIN, COM_GPIO,  DEFAULT},//CANFD17_EN_AP
    {0x21, CANFD17_STDBY_AP,      SAFETY_DOMAIN, COM_GPIO,  DEFAULT},//CANFD17_STDBY_AP
    {0x22, CANFD18_EN_AP,         SAFETY_DOMAIN, COM_GPIO,  DEFAULT},//"CANFD18_EN_AP",
    {0x23, CANFD18_STDBY_AP,      SAFETY_DOMAIN, COM_GPIO,  DEFAULT},//"CANFD18_STDBY_AP",
    {0x24, CANFD19_EN_AP,         SAFETY_DOMAIN, COM_GPIO,  DEFAULT},//"CANFD19_EN_AP",
    {0x25, CANFD19_STDBY_AP,      SAFETY_DOMAIN, COM_GPIO,  DEFAULT},//"CANFD19_STDBY_AP",
    {0x26, IOEX_CANFD13_EN_AP,    SAFETY_DOMAIN, IO_EXTEND, U3103},//"CANFD13_EN_AP",
    {0x27, IOEX_CANFD13_STDBY_AP, SAFETY_DOMAIN, IO_EXTEND, U3103},//"CANFD13_STDBY_AP",
    {0x28, IOEX_CANFD14_EN_AP,    SAFETY_DOMAIN, IO_EXTEND, U3103},//"CANFD14_EN_AP",
    {0x29, IOEX_CANFD14_STDBY_AP, SAFETY_DOMAIN, IO_EXTEND, U3103},//"CANFD14_STDBY_AP",
    {0x2a, IOEX_CANFD15_EN_AP,    SAFETY_DOMAIN, IO_EXTEND, U3103},//"CANFD15_EN_AP",
    {0x2b, IOEX_CANFD15_STDBY_AP, SAFETY_DOMAIN, IO_EXTEND, U3103},//"CANFD15_STDBY_AP",
    {0x2c, IOEX_CANFD20_EN_AP,    SAFETY_DOMAIN, IO_EXTEND, U3103},//"CANFD20_EN_AP",
    {0x2d, IOEX_CANFD20_STDBY_AP, SAFETY_DOMAIN, IO_EXTEND, U3103},//"CANFD20_STDBY_AP",
    {0x2e, IOEX_2122WAKE_1,       SAFETY_DOMAIN, IO_EXTEND, U3001},//"2122wake1",
    {0x2f, IOEX_2122WAKE_2,       SAFETY_DOMAIN, IO_EXTEND, U3001},//"2122wake2",
    {0x30, IOEX_2122WAKE_3,       SAFETY_DOMAIN, IO_EXTEND, U3001},//"2122wake3",
    {0x31, IOEX_2122WAKE_4,       SAFETY_DOMAIN, IO_EXTEND, U3001},//"2122wake4",
    {0x32, G9_WDT1_REST_AP,       SAFETY_DOMAIN, COM_GPIO, DEFAULT},//"G9_WDT1_REST_AP",
    {0x33, G9_WDT3_REST_AP,       SAFETY_DOMAIN, COM_GPIO, DEFAULT},//"G9_WDT3_REST_AP",
    {0x34, G9_SAFETY_SEST_AP,     SAFETY_DOMAIN, COM_GPIO, DEFAULT},//"G9_SAFETY_SEST_AP",
};
/*
*GPIO_pin_read_parament config
*/
const gpio_value_table_t gpio_read_value_table[] = {

    //{0x01, GPIO_C2,             AP_DOMAIN,     COM_GPIO,  DEFAULT},//"STAND_BY",
    //{0x02, GPIO_D11,            AP_DOMAIN,     COM_GPIO,  DEFAULT},//"STAND_BY",  select slave/master mode
    {0x05, PGOOD_3_3V,            SAFETY_DOMAIN, COM_GPIO,  DEFAULT},//DCDC BD9S400 3.3V
    {0x06, GPIO_E9,               SAFETY_DOMAIN, COM_GPIO,  DEFAULT},//KL15_WAKE_UP
    //{0x07, GPIO_E8,               SAFETY_DOMAIN, COM_GPIO,  DEFAULT},//DCDC MAX20098 +5V PGOOD
    {0x08, PGOOD_0_8V,            SAFETY_DOMAIN, COM_GPIO,  DEFAULT},//DCDC LTC7150 0.8V
    {0x09, OSPI1_SS1,             SAFETY_DOMAIN, COM_GPIO,  DEFAULT},//AP_to_SAF
    {0x0a, PGOOD_PMIC,            SAFETY_DOMAIN, COM_GPIO,  DEFAULT},//PMIC PF8200_PGOOD
    {0x0b, PGOOD_1_8V,            SAFETY_DOMAIN, COM_GPIO,  DEFAULT},//PS7A1601AQ +1V8_PGOOD
    {0x0c, CANFD16_INH_AP,        SAFETY_DOMAIN, COM_GPIO,  DEFAULT},//CANFD16_INH_AP
    {0x0d, CANFD16_ERR_AP,        SAFETY_DOMAIN, COM_GPIO,  DEFAULT},//CANFD16_ERR_AP
    {0x0e, CANFD18_INH_AP,        SAFETY_DOMAIN, COM_GPIO,  DEFAULT},//CANFD18_INH_AP
    {0x0f, CANFD18_ERR_AP,        SAFETY_DOMAIN, COM_GPIO,  DEFAULT},//CANFD18_ERR_AP
    {0x10, GPIO_C11,              AP_DOMAIN,     COM_GPIO,  DEFAULT},//2122_INT_1
    {0x11, GPIO_C9,               AP_DOMAIN,     COM_GPIO,  DEFAULT},//2122_INT_2
    {0x12, GPIO_C14,              AP_DOMAIN,     COM_GPIO,  DEFAULT},//2122_INT_3
    {0x13, GPIO_C15,              AP_DOMAIN,     COM_GPIO,  DEFAULT},//2122_INT_4
    {0x14, GPIO_C4,               AP_DOMAIN,     COM_GPIO,  DEFAULT},//read 5072 INTn
    {0x15, GPIO_D15,              AP_DOMAIN,     COM_GPIO,  DEFAULT},//read 5050 INTn
    {0x16, G9_5072_INH,           SAFETY_DOMAIN, COM_GPIO,  DEFAULT},//Wake source identification for 5072
    {0x17, G9_5050_INH,           SAFETY_DOMAIN, COM_GPIO,  DEFAULT},//Wake source identification for 5050
    {0x18, G9_2122_P10_INH_AP,    AP_DOMAIN,     COM_GPIO,  DEFAULT},//G9_2122_P10_INH_AP
    {0x19, G9_ACTIVE_LINE_AP,     AP_DOMAIN,     COM_GPIO,  DEFAULT},//G9_ACTIVE_LINE_AP
    {0x1a, G9X_GPIO_RES1_AP,      AP_DOMAIN,     COM_GPIO,  DEFAULT},//DTU/DTU'
    {0x1b, G9_2122_P7_INH_SAFETY, SAFETY_DOMAIN, COM_GPIO,  DEFAULT},//G9_2122_P7_INH_SAFETY
    {0x1c, G9_2122_P9_INH_SAFETY, SAFETY_DOMAIN, COM_GPIO,  DEFAULT},//G9_2122_P9_INH_SAFETY
    {0x1d, CANFD1_INH_SAFETY,     SAFETY_DOMAIN, COM_GPIO,  DEFAULT},//CANFD1_INH_AP
    {0x1e, CANFD1_ERR_SAFETY,     SAFETY_DOMAIN, COM_GPIO,  DEFAULT},//CANFD1_ERR_AP
    {0x1f, CANFD2_INH_SAFETY,     SAFETY_DOMAIN, COM_GPIO,  DEFAULT},//CANFD2_INH_AP
    {0x20, CANFD2_ERR_SAFETY,     SAFETY_DOMAIN, COM_GPIO,  DEFAULT},//CANFD2_ERR_AP
    {0x21, CANFD4_INH_SAFETY,     SAFETY_DOMAIN, COM_GPIO,  DEFAULT},//CANFD4_INH_AP
    {0x22, CANFD4_ERR_SAFETY,     SAFETY_DOMAIN, COM_GPIO,  DEFAULT},//CANFD4_ERR_AP
    {0x23, CANFD9_INH_AP,         SAFETY_DOMAIN, COM_GPIO,  DEFAULT},//CANFD9_INH_AP
    {0x24, CANFD9_ERR_AP,         SAFETY_DOMAIN, COM_GPIO,  DEFAULT},//CANFD9_ERR_AP
    {0x25, IOEX_CANFD3_INH_SAFETY, SAFETY_DOMAIN, IO_EXTEND, U3001}, //CANFD3_INH_AP
    {0x26, IOEX_CANFD3_ERR_SAFETY, SAFETY_DOMAIN, IO_EXTEND, U3001}, //CANFD3_ERR_AP
    {0x27, IOEX_CANFD10_INH_AP,   SAFETY_DOMAIN, IO_EXTEND, U3001},//CANFD10_INH_AP
    {0x28, IOEX_CANFD10_ERR_AP,   SAFETY_DOMAIN, IO_EXTEND, U3001},//CANFD10_ERR_AP
    {0x29, IOEX_CANFD11_INH_AP,   SAFETY_DOMAIN, IO_EXTEND, U3001},//CANFD11_INH_AP
    {0x2a, IOEX_CANFD11_ERR_AP,   SAFETY_DOMAIN, IO_EXTEND, U3001},//CANFD11_ERR_AP
    {0x2b, IOEX_CANFD12_INH_AP,   SAFETY_DOMAIN, IO_EXTEND, U3001},//CANFD12_INH_AP
    {0x2c, IOEX_CANFD12_ERR_AP,   SAFETY_DOMAIN, IO_EXTEND, U3001},//CANFD12_ERR_AP
    {0x2d, IOEX_CANFD13_INH_AP,   SAFETY_DOMAIN, IO_EXTEND, U3103},//CANFD13_INH_AP
    {0x2e, IOEX_CANFD13_ERR_AP,   SAFETY_DOMAIN, IO_EXTEND, U3103},//CANFD13_ERR_AP
    {0x2f, IOEX_CANFD14_INH_AP,   SAFETY_DOMAIN, IO_EXTEND, U3103},//CANFD14_INH_AP
    {0x30, IOEX_CANFD14_ERR_AP,   SAFETY_DOMAIN, IO_EXTEND, U3103},//CANFD14_ERR_AP
    {0x31, IOEX_CANFD15_INH_AP,   SAFETY_DOMAIN, IO_EXTEND, U3103},//CANFD15_INH_AP
    {0x32, IOEX_CANFD15_ERR_AP,   SAFETY_DOMAIN, IO_EXTEND, U3103},//CANFD15_ERR_AP
    {0x33, IOEX_CANFD17_INH_AP,   SAFETY_DOMAIN, IO_EXTEND, U3001},//CANFD17_INH_AP
    {0x34, IOEX_CANFD17_ERR_AP,   SAFETY_DOMAIN, IO_EXTEND, U3001},//CANFD17_ERR_AP
    {0x35, IOEX_CANFD19_INH_AP,   SAFETY_DOMAIN, IO_EXTEND, U3001},//CANFD19_INH_AP
    {0x36, IOEX_CANFD19_ERR_AP,   SAFETY_DOMAIN, IO_EXTEND, U3001},//CANFD19_ERR_AP
    {0x37, IOEX_CANFD20_INH_AP,   SAFETY_DOMAIN, IO_EXTEND, U3103},//CANFD20_INH_AP
    {0x38, IOEX_CANFD20_ERR_AP,   SAFETY_DOMAIN, IO_EXTEND, U3103},//CANFD20_ERR_AP
};
/*get kl15 gpio state*/
uint8_t get_kl15_gpio_state(void)
{
    return hal_dio_read_channel(dio_handle, KL15_PORT);
}
/*monitor DTU or DTU' mode*/
void monitor_master_slave_mode(uint8_t timers)
{
    static uint8_t cnt = 0;
    BaseType_t xHigherPriorityTaskWoken = pdTRUE;

    if (cnt < timers) {
        cnt++;
    }
    else {
        set_para_value(cnt, 0);
        xEventGroupSetBitsFromISR(canx_app.dtu.xEventGroupHandle, MONITOR_DUT_MODE,
                                  &xHigherPriorityTaskWoken);
    }
}
/*gpio initialization*/
static void dio_init_port(void)
{
    struct can_transceiver_config {
        Port_PinType    standby;
        Port_PinType    enable;
    } can_transceiver_configs[] = {
        {
            /* CANFD1 */
            .standby = CANFD1_STDBY_SAFETY,
            .enable = CANFD1_EN_SAFETY,
        },
        {
            /* CANFD2 */
            .standby = CANFD2_STDBY_SAFETY,
            .enable = CANFD2_EN_SAFETY,
        },
        {
            /* CANFD3 */
            .standby = CANFD3_STDBY_SAFETY,
            .enable = CANFD3_EN_SAFETY,
        },
        {
            /* CANFD4 */
            .standby = CANFD4_STDBY_SAFETY,
            .enable = CANFD4_EN_SAFETY,
        },
        {
            /* CANFD9 */
            .standby = CANFD9_STDBY_AP,
            .enable = CANFD9_EN_AP,
        },
        {
            /* CANFD10 */
            .standby = CANFD10_STDBY_AP,
            .enable = CANFD10_EN_AP,
        },
        {
            /* CANFD11 */
            .standby = CANFD11_STDBY_AP,
            .enable = CANFD11_EN_AP,
        },
        {
            /* CANFD12 */
            .standby = CANFD12_STDBY_AP,
            .enable = CANFD12_EN_AP,
        },
        {
            /* CANFD16 */
            .standby = CANFD16_STDBY_AP,
            .enable = CANFD16_EN_AP,
        },
        {
            /* CANFD17 */
            .standby = CANFD17_STDBY_AP,
            .enable = CANFD17_EN_AP,
        },
        {
            /* CANFD18 */
            .standby = CANFD18_STDBY_AP,
            .enable = CANFD18_EN_AP,
        },
        {
            /* CANFD19 */
            .standby = CANFD19_STDBY_AP,
            .enable = CANFD19_EN_AP,
        }
    };

    hal_dio_creat_handle(&dio_handle, g_gpio_res.res_id[0]);
    hal_port_creat_handle(&g_handle, g_iomuxc_res.res_id[0]);

    assert(dio_handle);
    assert(g_handle);
#if 0
    hal_port_set_pin_mode(g_handle, CANFD10_EN_AP,
                          mode_EMMC_GPIO_OUT);//CANFD10_EN_AP
    hal_port_set_pin_mode(g_handle, CANFD10_STDBY_AP,
                          mode_EMMC_GPIO_OUT);//CANFD10_STDBY_AP

    hal_port_set_pin_mode(g_handle, CANFD11_EN_AP,
                          mode_EMMC_GPIO_OUT);//CANFD11_EN_AP
    hal_port_set_pin_mode(g_handle, CANFD11_STDBY_AP,
                          mode_EMMC_GPIO_OUT);//CANFD11_STDBY_AP

    hal_port_set_pin_mode(g_handle, CANFD12_EN_AP,
                          mode_EMMC_GPIO_OUT);//CANFD12_EN_AP
    hal_port_set_pin_mode(g_handle, CANFD12_STDBY_AP,
                          mode_EMMC_GPIO_OUT);//CANFD12_STDBY_AP

    hal_port_set_pin_mode(g_handle, CANFD16_STDBY_AP,
                          mode_EMMC_GPIO_OUT);//CANFD16_STDBY_AP

    hal_port_set_pin_mode(g_handle, CANFD17_EN_AP,
                          mode_EMMC_GPIO_OUT);//CANFD17_EN_AP
    hal_port_set_pin_mode(g_handle, CANFD17_STDBY_AP,
                          mode_EMMC_GPIO_OUT);//CANFD17_STDBY_AP

    hal_port_set_pin_mode(g_handle, CANFD19_EN_AP,
                          mode_EMMC_GPIO_OUT);//CANFD19_EN_AP
    hal_port_set_pin_mode(g_handle, CANFD19_STDBY_AP,
                          mode_EMMC_GPIO_OUT);//CANFD19_STDBY_AP
#endif

    for (size_t i = 0;
            i < ARRAY_SIZE(can_transceiver_configs);
            i++) {
        struct can_transceiver_config *config = &can_transceiver_configs[i];

        hal_port_set_pin_direction(g_handle, config->standby, PORT_PIN_OUT);
        hal_port_set_pin_direction(g_handle, config->enable, PORT_PIN_OUT);
        hal_dio_write_channel(dio_handle, config->standby, 1);
        hal_dio_write_channel(dio_handle, config->enable, 1);
    }

    hal_port_set_pin_direction(g_handle, GPIO_E0, PORT_PIN_OUT);
    hal_dio_write_channel(dio_handle, GPIO_E0, 1);
    hal_port_set_pin_direction(g_handle, GPIOB_4, PORT_PIN_OUT);//B4
    hal_dio_write_channel(dio_handle, GPIOB_4, 1);
    hal_port_set_pin_direction(g_handle, GPIOE_1, PORT_PIN_OUT);//E1
    hal_dio_write_channel(dio_handle, GPIOE_1, 1);//default high level
    hal_port_set_pin_direction(g_handle, GPIOE_3,
                               PORT_PIN_OUT);//E3   can not to be used, reset
    hal_dio_write_channel(dio_handle, GPIOE_3, 1); //set high level, reset,
    hal_port_set_pin_direction(g_handle, GPIO_D13,
                               PORT_PIN_OUT);//D14   5072 reset // can not to be used,die
    hal_port_set_pin_direction(g_handle, GPIO_D14,
                               PORT_PIN_OUT);//D13   5050 reset
    hal_port_set_pin_direction(g_handle, OSPI1_SS1, PORT_PIN_IN);//OSPI1_SS1
    hal_port_set_pin_direction(g_handle, GPIO_E9, PORT_PIN_IN);//GPIO_E9
    hal_port_set_pin_direction(g_handle, GPIO_E8, PORT_PIN_IN);//GPIO_E8
    hal_port_set_pin_direction(g_handle, PGOOD_0_8V, PORT_PIN_IN);//pGOOD_0_8V
    hal_port_set_pin_direction(g_handle, PGOOD_3_3V, PORT_PIN_IN);//PGOOD_3_3V
    hal_port_set_pin_direction(g_handle, PGOOD_PMIC, PORT_PIN_IN);//PGOOD_PMIC
    hal_port_set_pin_direction(g_handle, PGOOD_1_8V, PORT_PIN_IN);//PGOOD_1_8V
    hal_port_set_pin_direction(g_handle, CANFD1_INH_SAFETY,
                               PORT_PIN_IN);//CANFD1_INH_SAFETY
    //hal_port_set_pin_direction(g_handle, CANFD1_ERR_SAFETY,
    //                           PORT_PIN_IN);//CANFD1_ERR_SAFETY
    hal_port_set_pin_direction(g_handle, CANFD2_INH_SAFETY,
                               PORT_PIN_IN);//CANFD2_INH_SAFETY
    //hal_port_set_pin_direction(g_handle, CANFD2_ERR_SAFETY,
    //                          PORT_PIN_IN);//CANFD2_ERR_SAFETY
    hal_port_set_pin_direction(g_handle, CANFD4_INH_SAFETY,
                               PORT_PIN_IN);//CANFD4_INH_SAFETY
    hal_port_set_pin_direction(g_handle, CANFD4_ERR_SAFETY,
                               PORT_PIN_IN);//CANFD4_ERR_SAFETY
    hal_port_set_pin_direction(g_handle, CANFD9_INH_AP,
                               PORT_PIN_IN);//CANFD9_INH_AP
    hal_port_set_pin_direction(g_handle, CANFD9_ERR_AP,
                               PORT_PIN_IN);//CANFD9_ERR_AP
    hal_port_set_pin_direction(g_handle, CANFD16_INH_AP,
                               PORT_PIN_IN);//CANFD16_INH_AP
    //hal_port_set_pin_mode(g_handle, CANFD16_ERR_AP,
    //                      mode_EMMC_GPIO_IN);//CANFD16_ERR_AP
    hal_port_set_pin_direction(g_handle, CANFD16_ERR_AP,
                               PORT_PIN_IN);//CANFD16_ERR_AP
    hal_port_set_pin_direction(g_handle, CANFD18_INH_AP,
                               PORT_PIN_IN);//CANFD18_INH_AP
    hal_port_set_pin_direction(g_handle, CANFD18_ERR_AP,
                               PORT_PIN_IN);//CANFD18_ERR_AP
    hal_port_set_pin_direction(g_handle, G9_5072_INH,
                               PORT_PIN_IN);//Wake source identification for 5072
    hal_port_set_pin_direction(g_handle, G9_5050_INH,
                               PORT_PIN_IN);//Wake source identification for 5050
    hal_port_set_pin_mode(g_handle, G9_2122_P7_INH_SAFETY,
                          mode_SAFETY_SEPCIAL_GPIO_IN);//safety special pin for p_160
}
/*if power down, set can standby pin into low level. this is i2c extend gpio*/
void i2cx_gpio_set_system_power_mgr(uint8_t STATE)
{
    if ((STATE != LOW) && (STATE != HIGH)) {
        return;
    }

    /*can_fd_13*/
    i2c_channel_to_write(0x1, 3, HIGH);//CANFD13_EN_SAFETY
    i2c_channel_to_write(0x1, 4, STATE);//CANFD13_STDBY_SAFETY
    /*can_fd_14*/
    i2c_channel_to_write(0x1, 7, HIGH);//CANFD14_EN_SAFETY
    i2c_channel_to_write(0x1, 8, STATE);//CANFD14_STDBY_SAFETY
    /*can_fd_15*/
    i2c_channel_to_write(0x1, 11, HIGH);//CANFD15_EN_SAFETY
    i2c_channel_to_write(0x1, 12, STATE);//CANFD15_STDBY_SAFETY
    /*can_fd_20*/
    i2c_channel_to_write(0x1, 15, HIGH);//CANFD20_EN_SAFETY
    i2c_channel_to_write(0x1, 16, STATE);//CANFD20_STDBY_SAFETY
#if 0
    /*CANFD13_INH_SAFETY*/
    i2c_channel_to_write(0x1, 1, LOW);//CANFD13_INH_SAFETY
    /*CANFD14_INH_SAFETY*/
    i2c_channel_to_write(0x1, 5, LOW);//CANFD14_INH_SAFETY
    /*CANFD15_INH_SAFETY*/
    i2c_channel_to_write(0x1, 9, LOW);//CANFD15_INH_SAFETY
    /*CANFD20_INH_SAFETY*/
    i2c_channel_to_write(0x1, 13, LOW);//CANFD20_INH_SAFETY
    /*CANFD19_INH_AP*/
    i2c_channel_to_write(0x0, 1, LOW);//CANFD19_INH_AP
    /*CANFD12_INH_AP*/
    i2c_channel_to_write(0x0, 3, LOW);//CANFD12_INH_AP
    /*CANFD11_INH_AP*/
    i2c_channel_to_write(0x0, 5, LOW);//CANFD11_INH_AP
    /*CANFD10_INH_AP*/
    i2c_channel_to_write(0x0, 8, LOW);//CANFD10_INH_AP
    /*CANFD17_INH_AP*/
    i2c_channel_to_write(0x0, 14, LOW);//CANFD17_INH
    /*CANFD3_INH_AP*/
    i2c_channel_to_write(0x0, 15, LOW);//CANFD3_INH
#endif
}

/*gpio initialization*/
void comx_gpio_init(void)
{
    dio_init_port();
}

/*gpio read self level*/
Dio_LevelType dio_read_channel_saf(uint8_t ChannelId)
{
    Dio_LevelType channelLevel;

    channelLevel = hal_dio_read_channel(dio_handle, ChannelId);

    return channelLevel;
}

/*if power down, set can standby pin into low level*/
void com_gpio_set_system_power_mgr(uint8_t STATE)
{
    hal_dio_write_channel(dio_handle, CANFD1_EN_SAFETY, HIGH);// CANFD1_EN_SAFETY
    hal_dio_write_channel(dio_handle, CANFD1_STDBY_SAFETY,
                          STATE); // CANFD1_STDBY_SAFETY

    hal_dio_write_channel(dio_handle, CANFD2_EN_SAFETY, HIGH);// CANFD2_EN_SAFETY
    hal_dio_write_channel(dio_handle, CANFD2_STDBY_SAFETY,
                          STATE); // CANFD2_STDBY_SAFETY

    hal_dio_write_channel(dio_handle, CANFD3_EN_SAFETY, HIGH);// CANFD3_EN_SAFETY
    hal_dio_write_channel(dio_handle, CANFD3_STDBY_SAFETY,
                          STATE); // CANFD3_STDBY_SAFETY

    hal_dio_write_channel(dio_handle, CANFD4_EN_SAFETY, HIGH);// CANFD4_EN_SAFETY
    hal_dio_write_channel(dio_handle, CANFD4_STDBY_SAFETY,
                          STATE); // CANFD4_STDBY_SAFETY

    hal_dio_write_channel(dio_handle, CANFD9_EN_AP, HIGH);//CANFD9_EN_AP
    hal_dio_write_channel(dio_handle, CANFD9_STDBY_AP, STATE);//CANFD9_STDBY_AP

    hal_dio_write_channel(dio_handle, CANFD10_EN_AP, HIGH);// CANFD10_EN_AP
    hal_dio_write_channel(dio_handle, CANFD10_STDBY_AP, STATE); // CANFD10_STDBY_AP

    hal_dio_write_channel(dio_handle, CANFD11_EN_AP, HIGH);// CANFD11_EN_AP
    hal_dio_write_channel(dio_handle, CANFD11_STDBY_AP, STATE); // CANFD11_STDBY_AP

    hal_dio_write_channel(dio_handle, CANFD12_EN_AP, HIGH);// CANFD12_EN_AP
    hal_dio_write_channel(dio_handle, CANFD12_STDBY_AP, STATE); // CANFD12_STDBY_AP

    hal_dio_write_channel(dio_handle, CANFD16_EN_AP, HIGH);// CANFD16_EN_AP
    hal_dio_write_channel(dio_handle, CANFD16_STDBY_AP, STATE); // CANFD16_STDBY_AP

    hal_dio_write_channel(dio_handle, CANFD17_EN_AP, HIGH);// CANFD17_EN_AP
    hal_dio_write_channel(dio_handle, CANFD17_STDBY_AP, STATE); // CANFD17_STDBY_AP

    hal_dio_write_channel(dio_handle, CANFD18_EN_AP, HIGH);// CANFD18_EN_AP
    hal_dio_write_channel(dio_handle, CANFD18_STDBY_AP, STATE); // CANFD18_STDBY_AP

    hal_dio_write_channel(dio_handle, CANFD19_EN_AP, HIGH);// CANFD19_EN_AP
    hal_dio_write_channel(dio_handle, CANFD19_STDBY_AP, STATE); // CANFD19_STDBY_AP
#if 0
    hal_port_set_pin_direction(g_handle, CANFD1_INH_SAFETY,
                               PORT_PIN_OUT);//CANFD1_INH_SAFETY
    hal_port_set_pin_direction(g_handle, CANFD2_INH_SAFETY,
                               PORT_PIN_OUT);//CANFD2_INH_SAFETY
    hal_port_set_pin_direction(g_handle, CANFD4_INH_SAFETY,
                               PORT_PIN_OUT);//CANFD4_INH_SAFETY
    hal_port_set_pin_direction(g_handle, CANFD9_INH_AP,
                               PORT_PIN_OUT);//CANFD9_INH_AP
    hal_port_set_pin_direction(g_handle, CANFD16_INH_AP,
                               PORT_PIN_OUT);//CANFD16_INH_AP
    hal_port_set_pin_direction(g_handle, CANFD18_INH_AP,
                               PORT_PIN_OUT);//CANFD18_INH_AP

    hal_dio_write_channel(dio_handle, CANFD1_INH_SAFETY, STATE);
    hal_dio_write_channel(dio_handle, CANFD2_INH_SAFETY, STATE);
    hal_dio_write_channel(dio_handle, CANFD4_INH_SAFETY, STATE);
    hal_dio_write_channel(dio_handle, CANFD9_INH_AP, STATE);
    hal_dio_write_channel(dio_handle, CANFD16_INH_AP, STATE);
    hal_dio_write_channel(dio_handle, CANFD18_INH_AP, STATE);
#endif
}

static bool _gpio_single_read(board_test_exec_t *exec)
{
    bool ret = false;
    uint8_t result = 0;
    CMD_STATUS cmdStatus = CMD_PARA_ERR;
    can_cmd_t *gpio_cmd = (can_cmd_t *)exec->cmd;

    dprintf(debug_show_null, "_gpio_single_read\n");

    if (gpio_cmd->dev_id == g_step_case_table[GPIO_SERIAL_R_ID].cmd_id) {

        for (uint8_t num = 0; num < ARRAY_SIZE(gpio_read_value_table); num++) {

            if (gpio_cmd->route_channel_id != gpio_read_value_table[num].channel) {
                continue;
            }

            if (gpio_read_value_table[num].domain == SAFETY_DOMAIN) {

                dprintf(debug_show_null, "route_channel_id:%x,pin_num:%x\n",
                        gpio_cmd->route_channel_id, gpio_read_value_table[num].channel);

                if (gpio_read_value_table[num].gpio_type == COM_GPIO) {
                    result = hal_dio_read_channel(dio_handle, gpio_read_value_table[num].gpio_Pin);
                }
                else if (gpio_read_value_table[num].gpio_type == IO_EXTEND) {
                    result = i2c_channel_to_read(gpio_read_value_table[num].i2c_dev,
                                                 gpio_read_value_table[num].gpio_Pin);
                }
                else {
                    set_resp_err_state(exec->resp[0], cmdStatus);
                    return ret;
                }

                set_para_value(cmdStatus, NORMAL_DEAL);
                set_para_value(ret, true);
                break;
            }
            else {
                remote_test_send_req(gpio_cmd);

                if (remote_test_wait_resp(xTIME_OUT_TICKS, exec) == true) {
                    set_para_value(cmdStatus, NORMAL_DEAL);
                    set_para_value(ret, true);
                    return ret;
                }
            }
        }
    }
    else {
        set_resp_err_state(exec->resp[0], cmdStatus);
        return ret;
    }

    if (ret != true) {
        set_resp_err_state(exec->resp[0], cmdStatus);
        return ret;
    }

    set_para_value(exec->resp[0], cmdStatus);
    set_para_value(exec->resp[1], gpio_cmd->route_channel_id);
    set_para_value(exec->resp[2], result);

    return ret;
}

/*set board into DTU or DTU' by con1_19 gpio state*/
bool remote_require_for_gpio_value(board_test_exec_t *exec)
{
    return _gpio_single_read(exec);
}

static bool _gpio_period_read(board_test_exec_t *exec)
{
    bool ret = false;
    uint8_t val = 0;
    uint64_t result = 0;
    CMD_STATUS cmdStatus = NORMAL_DEAL;
    uint32_t respCanID = PERIODIC_RESP_GPIO;
    can_cmd_t *gpio_cmd = (can_cmd_t *)exec->cmd;

    for (uint8_t num = 0; num < ARRAY_SIZE(gpio_read_value_table); num++) {

        if (gpio_read_value_table[num].domain == SAFETY_DOMAIN) {

            val = hal_dio_read_channel(dio_handle, gpio_read_value_table[num].gpio_Pin);
        }
        else {
            set_para_value(gpio_cmd->dev_id, SUBCMD_GPIO_R);
            set_para_value(gpio_cmd->route_channel_id, gpio_read_value_table[num].channel);
            remote_test_send_req(gpio_cmd);

            if (remote_test_wait_resp(xTIME_OUT_TICKS, exec) == true) {
                val = exec->resp[2];
            }
        }

        val ? \
        (result |=  (1 << num)) : \
        (result &= ~(1 << num));
    }

    set_para_value(exec->resp[0], cmdStatus);
    set_para_value(exec->resp[1], result & 0xff);
    set_para_value(exec->resp[2], (result >> 8) & 0xff);
    set_para_value(exec->resp[3], (result >> 16) & 0xff);
    set_para_value(exec->resp[4], (result >> 24) & 0xff);
    set_para_value(exec->resp[5], (result >> 32) & 0xff);
    set_para_value(exec->resp[6], (result >> 40) & 0xff);
    set_para_value(exec->resp[7], (result >> 48) & 0xff);
    set_para_value(exec->peridic_resp_id, respCanID);

    return ret;
}

/*which common gpio into writing*/
static bool _gpio_single_write(board_test_exec_t *exec)
{
    bool ret = false;
    CMD_STATUS cmdStatus = CMD_PARA_ERR;
    can_cmd_t *gpio_cmd = (can_cmd_t *)exec->cmd;

    dprintf(debug_show_dg, "_gpio_single_write\n");
    memset(exec->resp, 0, sizeof(exec->resp));

    if (gpio_cmd->dev_id == g_step_case_table[GPIO_SERIAL_W_ID].cmd_id) {

        for (uint8_t num = 0; num < ARRAY_SIZE(gpio_write_value_table); num++) {

            if (gpio_cmd->route_channel_id != gpio_write_value_table[num].channel) {
                continue;
            }
            else {
                if (gpio_write_value_table[num].domain == SAFETY_DOMAIN) {

                    if (gpio_write_value_table[num].gpio_type == COM_GPIO) {
                        hal_dio_write_channel(dio_handle, gpio_write_value_table[num].gpio_Pin,
                                              gpio_cmd->recv_data);
                    }
                    else if (gpio_write_value_table[num].gpio_type == IO_EXTEND) {
                        i2c_channel_to_write(gpio_write_value_table[num].i2c_dev,
                                             gpio_write_value_table[num].gpio_Pin, gpio_cmd->recv_data);
                    }
                    else {
                        set_resp_err_state(exec->resp[0], cmdStatus);
                        return ret;
                    }

                    set_para_value(cmdStatus, NORMAL_DEAL);
                    set_para_value(ret, true);
                    break;
                }
                else {
                    remote_test_send_req(gpio_cmd);

                    if ((ret = remote_test_wait_resp(xTIME_OUT_TICKS, exec)) == true) {
                        set_para_value(cmdStatus, NORMAL_DEAL);
                        break;
                    }
                }
            }
        }
    }
    else {
        set_resp_err_state(exec->resp[0], cmdStatus);
        return ret;
    }

    if (ret != true) {
        set_resp_err_state(exec->resp[0], cmdStatus);
        return ret;
    }

    set_para_value(exec->resp[0], cmdStatus);
    set_para_value(exec->resp[1], gpio_cmd->route_channel_id);
    set_para_value(exec->resp[2], gpio_cmd->recv_data);

    return ret;
}

/*which ethernet gpio into writing*/
bool _set_eth_gpio_single_write(board_test_exec_t *exec)
{
    bool ret = false;
    CMD_STATUS cmdStatus = CMD_PARA_ERR;
    can_cmd_t *gpio_cmd = (can_cmd_t *)exec->cmd;

    memset(exec->resp, 0, sizeof(exec->resp));

    if (gpio_cmd->dev_id == g_step_case_table[ETH_PHY_INT_ID].cmd_id) {

        for (uint8_t num = 0; num < ARRAY_SIZE(eth_gpio_int_value_table); num++) {

            if (gpio_cmd->route_channel_id != eth_gpio_int_value_table[num].channel) {
                continue;
            }
            else {
                if (eth_gpio_int_value_table[num].domain == SAFETY_DOMAIN) {

                    hal_dio_write_channel(dio_handle, eth_gpio_int_value_table[num].gpio_Pin,
                                          gpio_cmd->recv_data);
                    set_para_value(cmdStatus, NORMAL_DEAL);
                    set_para_value(ret, true);
                    break;
                }
                else {
                    remote_test_send_req(gpio_cmd);//ap to safety real time

                    if (remote_test_wait_resp(xTIME_OUT_TICKS, exec) == true) {
                        set_para_value(cmdStatus, NORMAL_DEAL);
                    }
                }
            }
        }
    }
    else {
        set_resp_err_state(exec->resp[0], cmdStatus);
        return ret;
    }

    if (ret != true) {
        set_resp_err_state(exec->resp[0], cmdStatus);
        return ret;
    }

    set_para_value(exec->resp[0], cmdStatus);
    set_para_value(exec->resp[1], gpio_cmd->route_channel_id);
    set_para_value(exec->resp[2], gpio_cmd->recv_data);

    return ret;
}

/*
*gpio read
*/
bool board_gpio_read_reply_deal(board_test_exec_t *exec,
                                board_test_state_e state)
{
    bool ret = false;

    if (state == STATE_SINGLE) {
        ret = _gpio_single_read(exec);
    }
    else if (state == STATE_PERIODIC) {
        ret = _gpio_period_read(exec);
    }

    set_para_value(exec->board_response, can_common_response);
    return ret;
}

/*gpio read process function start*/
bool board_gpio_write_reply_deal(board_test_exec_t *exec,
                                 board_test_state_e state)
{
    bool ret = false;
    printf("com_gpio_write_reply_deal\n");

    if (state == STATE_SINGLE) {
        ret = _gpio_single_write(exec);
        set_para_value(exec->board_response, can_common_response);
    }

    return ret;
}

/*gpio write process function start*/
bool write_eth_phy_gpio_deal(board_test_exec_t *exec, board_test_state_e state)
{
    bool ret = false;
    printf("eth_gpio_write_reply_deal\n");

    if (state == STATE_SINGLE) {
        ret = _set_eth_gpio_single_write(exec);
        set_para_value(exec->board_response, can_common_response);
    }

    return ret;
}
