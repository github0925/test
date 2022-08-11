/********************************************************
 *          Copyright(c) 2018   Semidrive               *
 ********************************************************/

#include <common_hdr.h>
#include <srv_pin/srv_pin.h>
#include <soc.h>
#include <iomux/iomux_def.h>

/* Pull-up, default drive strength */
#define PAD_SETTING_I2C     (FV_IO_PAD_CONFIG_DS(1)\
                                | BM_IO_PAD_CONFIG_PS \
                                | BM_IO_PAD_CONFIG_PE)

const pad_ctrl_t i2c10_pins[] = {
    PIN_DCLR(IOMUXC_SEC, GPIO_D2, GPIO_D2, SEL_NONE,            /* I2C10.SCL */
             FV_PIN_MUX_CONFIG_MUX(1), PAD_SETTING_I2C, 0),
    PIN_DCLR(IOMUXC_SEC, GPIO_D3, GPIO_D3, SEL_NONE,            /* I2C10.SDA */
             FV_PIN_MUX_CONFIG_MUX(1), PAD_SETTING_I2C, 0),
    PIN_DCLR_NONE,
};

const pad_ctrl_t i2c4_pins[] = {
    PIN_DCLR(IOMUXC_SAFE, RGMII1_RXD3, RGMII1_RXD3, SEL_NONE,            /* I2C4.SCL */
             FV_PIN_MUX_CONFIG_MUX(3), PAD_SETTING_I2C, 0),
    PIN_DCLR(IOMUXC_SAFE, RGMII1_RX_CTL, RGMII1_RX_CTL, SEL_NONE,            /* I2C4.SDA */
             FV_PIN_MUX_CONFIG_MUX(3), PAD_SETTING_I2C, 0),
    PIN_DCLR_NONE,
};

void soc_i2c_pin_cfg(module_e m, void *para)
{
    switch (m) {
    case I2C10:
        srv_pin_cfg(&i2c10_pins[0]);
        break;

    case I2C4:
        srv_pin_cfg(&i2c4_pins[0]);
        break;

    default:
        break;
    }
}
