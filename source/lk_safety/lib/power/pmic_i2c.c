/********************************************************
 *          Copyright(c) 2020   Semidrive               *
 ********************************************************/

#include "pmic.h"
#include "i2c_hal.h"

bool pmic_i2c_init(struct pmic_dev_t *dev)
{
    if (dev->handle)
        return true;

    hal_i2c_creat_handle(&dev->handle, dev->i2c_bus_res);

    if (dev->handle == NULL)
        return false;

    i2c_app_config_t i2c_conf = hal_i2c_get_busconfig(dev->handle);
    i2c_conf.poll = 1;
    hal_i2c_set_busconfig(dev->handle, &i2c_conf);
    return true;
}

bool pmic_i2c_write_reg(struct pmic_dev_t *dev, uint8_t reg,
                        uint8_t data)
{
    hal_i2c_write_reg_data(dev->handle, dev->slaveaddr, (void *)&reg, 1,
                           (void *)&data, 1);
    return true;
}

bool pmic_i2c_read_reg(struct pmic_dev_t *dev, uint8_t reg,
                       uint8_t *data)
{
    hal_i2c_read_reg_data(dev->handle, dev->slaveaddr, (void *)&reg, 1,
                          (void *)data, 1);
    return true;
}

bool pmic_i2c_deinit(struct pmic_dev_t *dev)
{
    if (!dev || !dev->handle) {
        return false;
    }
    else {
        hal_i2c_release_handle(dev->handle);
        dev->handle = NULL;
        return true;
    }
}
