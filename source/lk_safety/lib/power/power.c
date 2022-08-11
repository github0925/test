/********************************************************
 *          Copyright(c) 2020   Semidrive               *
 ********************************************************/

#include "power.h"
#include "i2c_hal.h"

static struct power_item_t *g_power_configs = NULL;
static int g_power_item_count = 0;

static int change_voltage_internal(struct pmic_dev_t *dev, int index,
                                   uint32_t mv)
{
    uint32_t read_data = 0;

    if (!dev->init(dev)) {
        dprintf(0, "%s: Opps, failed to init pmic\n", __FUNCTION__);
        return -1;
    }

    if (!dev->set_vol(dev, index, mv)) {
        dprintf(0, "%s: Opps, failed to set vol %u %u\n", __FUNCTION__, index, mv);
        dev->deinit(dev);
        return -2;
    }

    read_data = dev->get_vol(dev, index);
    dev->deinit(dev);
    dprintf(0, "change voltage sucessfully, val:%u\n", read_data);
    return 0;
}

static int set_power_ctrl_by_pin_internal(struct pmic_dev_t *dev,
        int index, int pin_idx, bool isctrl)
{
    if (!dev->init(dev)) {
        dprintf(0, "%s: Opps, failed to init pmic\n", __FUNCTION__);
        return -1;
    }

    if (dev->set_power_ctrl_by_pin
            && !dev->set_power_ctrl_by_pin(dev, index, pin_idx, isctrl)) {
        dprintf(0, "%s: Opps, failed to set power ctrl %u %u %d\n", __FUNCTION__,
                index, pin_idx, isctrl);
        dev->deinit(dev);
        return -2;
    }

    dev->deinit(dev);
    return 0;
}

bool change_voltage(enum vdd_id_e id, uint32_t mv)
{
    int i = 0;
    bool found = false;
    struct power_item_t *item;

    if (g_power_configs == NULL || g_power_item_count == 0)
        return false;

    for (i = 0; i < g_power_item_count; i++) {
        item = &g_power_configs[i];

        if (item->vdd_id == id) {
            found = true;

            if (change_voltage_internal(item->dev, item->index, mv) != 0)
                return false;
        }
    }

    return found;
}
bool set_power_ctrl_by_pin(enum vdd_id_e id, int pin_index, bool isctrl)
{
    int i = 0;
    bool found = false;
    struct power_item_t *item;

    if (g_power_configs == NULL || g_power_item_count == 0)
        return false;

    for (i = 0; i < g_power_item_count; i++) {
        item = &g_power_configs[i];

        if (item->vdd_id == id) {
            found = true;

            if (set_power_ctrl_by_pin_internal(item->dev, item->index, pin_index,
                                               isctrl) != 0)
                return false;
        }
    }

    return found;
}
bool add_power_config(struct power_item_t *power_config, int count)
{
    g_power_configs = power_config;
    g_power_item_count = count;
    return true;
}
