/********************************************************
 *          Copyright(c) 2020   Semidrive               *
 ********************************************************/

#ifndef __POWER_H
#define __POWER_H
#include <stdint.h>
#include <reg.h>
#include <bits.h>
#include "pmic.h"
enum vdd_id_e {
    VDD_AP,
    VDD_GPU,
    VDD_CPU,
    VDDQ_1V1,
    VDDQLP_0V6,
};
struct power_item_t {
    enum vdd_id_e vdd_id;
    struct pmic_dev_t *dev;
    int index;
};

/* api for user */
bool change_voltage(enum vdd_id_e id, uint32_t mv);
/* api for board config */
bool add_power_config(struct power_item_t *power_config, int count);
/* api for pmic en pin control*/
bool set_power_ctrl_by_pin(enum vdd_id_e id, int pin_index, bool isctrl);

#endif
