/********************************************************
 *          Copyright(c) 2020   Semidrive               *
 ********************************************************/
#include "lp875xx.h"
#include "debug.h"
#include <assert.h>
#include "string.h"

const static int vol_reg[] = {0x0a, 0x0c, 0x0e, 0x10};
const static int pin_ctrl_reg[] = {0x02, 0x04, 0x06, 0x08};

static int find_vol_reg_by_index(int index)
{
    return vol_reg[index];
}
static uint32_t convert_val_to_vol(uint8_t val)
{
    if (val >= 0xa && val <= 0x18)
        return (val - 0xa) * 10 + 600;
    else if (val > 0x18 && val <= 0x9e)
        return (val - 0x18) * 5 + 735;
    else if (val > 0x9e && val <= 0xff)
        return (val - 0x9e) * 20 + 1420;
    else
        ASSERT(0);
}
static uint8_t convert_vol_to_val(uint32_t mv)
{
    if (mv >= 600 && mv <= 730) {
        return 0xa + ((mv - 600) + 9) / 10;
    }
    else if (mv > 730 && mv <= 1400) {
        return 0x18 + ((mv - 735) + 4) / 5;
    }
    else if (mv > 1400 && mv <= 3360) {
        return 0x9e + ((mv - 1420) + 19) / 20;
    }
    else {
        ASSERT(0);
    }
}
static int find_pin_ctrl_reg_by_index(int index)
{
    return pin_ctrl_reg[index];
}

static uint8_t update_pin_ctrl_to_val(uint8_t val, int pin_idx,
                                      bool isctrl)
{
    if (!isctrl) {
        val &= ~(1 << 6);
        return val;
    }

    val |= 1 << 6;
    val &= ~(0x3 << 4);
    val |= (pin_idx & 0x3) << 4;
    return val;
}

static bool lp875xx_set_vol(struct pmic_dev_t *dev, int index, int mv)
{
    uint8_t reg = find_vol_reg_by_index(index);
    uint8_t val = convert_vol_to_val(mv);

    if (pmic_i2c_write_reg(dev, reg, val)) {
        uint8_t rdata = 0;
        pmic_i2c_read_reg(dev, reg, &rdata);

        if (rdata == val)
            return true;
        else
            return false;
    }

    return false;
}
static uint32_t lp875xx_get_vol(struct pmic_dev_t *dev, int index)
{
    uint8_t reg = find_vol_reg_by_index(index);
    uint8_t data = 0;

    if (pmic_i2c_read_reg(dev, reg, &data)) {
        return convert_val_to_vol(data);
    }

    return 0;
}

bool lp875xx_set_power_ctrl_by_pin(struct pmic_dev_t *dev, int index,
                                   int pin_idx, bool isctrl)
{
    uint8_t reg = find_pin_ctrl_reg_by_index(index);
    uint8_t val = 0;

    if (!pmic_i2c_init(dev))
        return false;

    pmic_i2c_read_reg(dev, reg, &val);
    val = update_pin_ctrl_to_val(val, pin_idx, isctrl);
    pmic_i2c_write_reg(dev, reg, val);
    pmic_i2c_deinit(dev);
    return true;
}

bool pmic_dev_add_lp875xx(struct pmic_dev_t *dev, uint32_t i2c_bus,
                          int slaveaddr)
{
    if (!dev)
        return false;

    memset(dev, 0, sizeof(struct pmic_dev_t));
    dev->i2c_bus_res = i2c_bus;
    dev->slaveaddr = slaveaddr;
    dev->init = pmic_i2c_init;
    dev->set_vol = lp875xx_set_vol;
    dev->get_vol = lp875xx_get_vol;
    dev->set_power_ctrl_by_pin = lp875xx_set_power_ctrl_by_pin;
    dev->deinit = pmic_i2c_deinit;
    return true;
}
