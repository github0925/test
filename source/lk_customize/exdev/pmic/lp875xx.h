/********************************************************
 *          Copyright(c) 2020   Semidrive               *
 ********************************************************/

#ifndef __LP_875XX_H
#define __LP_875XX_H
#include "pmic.h"

#define LP875XX_BUCK0_IDX   0
#define LP875XX_BUCK1_IDX   1
#define LP875XX_BUCK2_IDX   2
#define LP875XX_BUCK3_IDX   3

#define LP875XX_PIN_EN1 0
#define LP875XX_PIN_EN2 1
#define LP875XX_PIN_EN3 2

bool pmic_dev_add_lp875xx(struct pmic_dev_t *dev, uint32_t i2c_bus,
                          int slaveaddr);
bool lp875xx_set_power_ctrl_by_pin(struct pmic_dev_t *dev, int index,
                                   int pin_idx, bool isctrl);
#endif
