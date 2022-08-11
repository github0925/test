/*
 * Copyright (c) 2019, SemiDrive, Inc. All rights reserved
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <lib/console.h>
#include <i2c_hal.h>
#include <slt_config.h>

//LP87563 pmic set

#define ARRAYSIZE(A) ((int)(sizeof(A)/sizeof((A)[0])))

static uint8_t slt_config_calculate_voltage(int ap_volt_mv)
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
int slt_config_setup_ddr_voltage(uint32_t vdd_id, uint32_t mv)
{
    int ret = 1;
    void* i2c_handle = NULL;
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
    pmic1_reg_data[vdd_id][1] = slt_config_calculate_voltage(mv);
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
                               (void*)&pmic1_reg_data[vdd_id][0], 1, (void*)&pmic1_reg_data[vdd_id][1],
                               1);
        /* Confirm data been written correctly */
        hal_i2c_read_reg_data(i2c_handle, pmic_addr[1],
                              (void*)&pmic1_reg_data[vdd_id][0], 1, (void*)&read_reg_data, 1);

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

int slt_config_change_vdd_ap_voltage(int ap_volt_mv)
{
    int ret = 1;
    int i = 0;
    void* i2c_handle = NULL;
    uint8_t pmic_addr[2] = {0x60, 0x61};
    uint8_t pmic1_reg_data[][2] = {
        {0x0a, 0x1b}, //ap
        {0x0e, 0x2f}, //gpu
        {0x06, 0x96}
    }; //gpu ctrl
    uint8_t read_reg_data = 0;
    pmic1_reg_data[0][1] = slt_config_calculate_voltage(ap_volt_mv);
    hal_i2c_creat_handle(&i2c_handle, RES_I2C_I2C10);

    if (i2c_handle != NULL) {
        for (i = 0; i < ARRAYSIZE(pmic1_reg_data); i++) {
            hal_i2c_write_reg_data(i2c_handle, pmic_addr[0],
                                   (void*)&pmic1_reg_data[i][0], 1, (void*)&pmic1_reg_data[i][1], 1);
            hal_i2c_read_reg_data(i2c_handle, pmic_addr[0],
                                  (void*)&pmic1_reg_data[i][0], 1, (void*)&read_reg_data, 1);
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

uint32_t slt_config_get_chipid(void)
{
    return 0x12345678;
}

uint32_t slt_config_init_from_pc(slt_app_context_t* pcontext)
{
    return 1;
}