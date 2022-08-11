/********************************************************
 *          Copyright(c) 2020   Semidrive               *
 ********************************************************/

#ifndef __PMIC_H
#define __PMIC_H
#include <stdint.h>
#include <reg.h>
#include <bits.h>

struct pmic_dev_t {
    void *handle;
    uint32_t i2c_bus_res;
    int slaveaddr;
    bool (*init)(struct pmic_dev_t *dev);
    bool (*set_vol)(struct pmic_dev_t *dev, int index, int mv);
    uint32_t (*get_vol)(struct pmic_dev_t *dev, int index);
    bool (*set_power_ctrl_by_pin)(struct pmic_dev_t *dev, int index,
                                  int pin_idx, bool isctrl);
    bool (*deinit)(struct pmic_dev_t *dev);
};
bool pmic_i2c_init(struct pmic_dev_t *dev);
bool pmic_i2c_write_reg(struct pmic_dev_t *dev, uint8_t reg,
                        uint8_t data);
bool pmic_i2c_read_reg(struct pmic_dev_t *dev, uint8_t reg,
                       uint8_t *data);
bool pmic_i2c_deinit(struct pmic_dev_t *dev);
#endif

