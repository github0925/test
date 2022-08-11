/*
 * Copyright (c) 2018 Semidrive Semiconductor.
 * All rights reserved.
 */

#ifndef __PMU_DRV_H
#define __PMU_DRV_H
#include <sys/types.h>
#include <compiler.h>

__BEGIN_CDECLS;

/* pmu register offset */
#define PMU_EXT_CRTL            (0x00)
#define PMU_RTC_CTRL            (0x04)
#define PMU_GF_CTRL             (0x08)
#define PMU_RST_CTRL            (0x0c)
#define PMU_CTRL_0              (0x10)
#define PMU_CTRL_1              (0x14)
#define PMU_CTRL_2              (0x18)
#define PMU_CTRL_3              (0x1c)
#define PMU_INTR                (0x20)
#define PMU_STS_0               (0x30)
#define PMU_STS_1               (0x34)
#define PMU_STS_2               (0x38)
#define PMU_STS_3               (0x3c)


#define PMU_INIT                0x01
#define PMU_PWRUP               0x02
#define PMU_ON                  0x04
#define PMU_PWRDOWN             0x08
#define PMU_OFF                 0x10

/* power control id */
#define PWR_CTRL_0              0
#define PWR_CTRL_1              1
#define PWR_CTRL_2              2
#define PWR_CTRL_3              3

#define PWR_CTRL_INPUT          1
#define PWR_CTRL_OUTPUT         0
#define PWR_CTRL_OUTPUT_MANUAL  0
#define PWR_CTRL_OUTPUT_AUTO    1
#define PWR_CTRL_OUTPUT_LOW     0
#define PWR_CTRL_OUTPUT_HIGH    1

#define DISABLE                 0
#define ENABLE                  1
#define LOW_LEVEL               0
#define HIGH_LEVEL              1


typedef struct pmu_device {
	const char *name;
	int32_t id;
	uint32_t init_flag;
	volatile uint8_t *base_paddr;
	volatile uint8_t *base_vaddr;
	void *priv_data;
} pmudev_t;

#define pmu_dbg(str, x...) \
    do { printf("%s:%d: " str, __func__, __LINE__, ## x); } while (0)
#define pmu_err(str, x...) \
    do { printf("%s:%d: " str, __func__, __LINE__, ## x); } while (0)

int32_t pmu_init(pmudev_t *dev);
int32_t pmu_exit(pmudev_t *dev);

/**************************************************
 * pmu_get_status
 *
 * status value 0x01 - INIT Mode
 *              0x02 - PWRUP Mode
 *              0x04 - ON Mode
 *              0x08 - PWRDOWN Mode
 *              0x10 - OFF Mode
 *              < 0  - error
 ************************************************/
int32_t pmu_get_status(pmudev_t *dev, int *status);

int32_t pmu_get_event_source(pmudev_t *dev, uint32_t *event);

int32_t pmu_clean_single_event_source(pmudev_t *dev, int event_id);

int32_t pmu_clean_all_event_source(pmudev_t *dev);

int32_t pmu_powerdown(pmudev_t *dev, int enable);

/******************************************************************************
 * pmu_set_powerdown_delay
 *
 * input delay:  0x0 - 0us
 *               0x1 - 30us
 *               0x2 - 60us
 *               0x3 - 120us
 *               0x4 - 0.25ms
 *               0x5 - 0.5ms
 *               0x6 - 0.75ms
 *               0x7 - 1ms
 *               0x8 - 2ms
 *               0x9 - 3ms
 *               0xa - 4ms
 *               0xb - 5ms
 *               0xc - 6ms
 *               0xd - 7ms
 *               0xe - 8ms
 *               0xf - 16ms
 *
 *****************************************************************************/
int32_t pmu_set_powerdown_delay(pmudev_t *dev, int id, int delay);


/******************************************************************************
 * pmu_set_powerup_delay
 *
 * input delay:  0x0 - 0us
 *               0x1 - 30us
 *               0x2 - 60us
 *               0x3 - 120us
 *               0x4 - 0.25ms
 *               0x5 - 0.5ms
 *               0x6 - 0.75ms
 *               0x7 - 1ms
 *               0x8 - 2ms
 *               0x9 - 3ms
 *               0xa - 4ms
 *               0xb - 5ms
 *               0xc - 6ms
 *               0xd - 7ms
 *               0xe - 8ms
 *               0xf - 16ms
 *
 *****************************************************************************/
int32_t pmu_set_powerup_delay(pmudev_t *dev, int id, int delay);

/******************************************************************************
 * pmu_set_powerctrl_powerdown_delay
 *
 * input delay:  0x0 - 0us
 *               0x1 - 30us
 *               0x2 - 60us
 *               0x3 - 120us
 *               0x4 - 0.25ms
 *               0x5 - 0.5ms
 *               0x6 - 0.75ms
 *               0x7 - 1ms
 *               0x8 - 2ms
 *               0x9 - 3ms
 *               0xa - 4ms
 *               0xb - 5ms
 *               0xc - 6ms
 *               0xd - 7ms
 *               0xe - 8ms
 *               0xf - 16ms
 *
 *****************************************************************************/
int32_t pmu_set_powerctrl_powerdown_delay(pmudev_t *dev, int ctrl_id, int delay);

/******************************************************************************
 * pmu_set_powerctrl_powerup_delay
 *
 * input delay:  0x0 - 0us
 *               0x1 - 30us
 *               0x2 - 60us
 *               0x3 - 120us
 *               0x4 - 0.25ms
 *               0x5 - 0.5ms
 *               0x6 - 0.75ms
 *               0x7 - 1ms
 *               0x8 - 2ms
 *               0x9 - 3ms
 *               0xa - 4ms
 *               0xb - 5ms
 *               0xc - 6ms
 *               0xd - 7ms
 *               0xe - 8ms
 *               0xf - 16ms
 *
 *****************************************************************************/
int32_t pmu_set_powerctrl_powerup_delay(pmudev_t *dev, int ctrl_id, int delay);

int32_t pmu_set_powerctrl_io_mode(pmudev_t *dev, int ctrl_id, int mode);

int32_t pmu_set_powerctrl_out_mode(pmudev_t *dev, int ctrl_id, int mode);

int32_t pmu_set_powerctrl_out_ctrl(pmudev_t *dev, int ctrl_id, int out);

int32_t pmu_get_powerctrl_input_status(pmudev_t *dev, int ctrl_id, int *status);

int32_t pmu_set_internal_powerdown_enable(pmudev_t *dev, int id, int enable);

int32_t pmu_set_internal_powerdown_polarity(pmudev_t *dev, int id, int polarity);

int32_t pmu_get_internal_powerdown_status(pmudev_t *dev, int id, int *status);

int32_t pmu_set_external_reset_enable(pmudev_t *dev, int id, int enable);

int32_t pmu_set_external_reset_polarity(pmudev_t *dev, int id, int polarity);

int32_t pmu_set_external_reset_debounce_enable(pmudev_t *dev, int id, int enable);

int32_t pmu_set_external_reset_debounce_delay(pmudev_t *dev, int id, int delay);

int32_t pmu_get_external_reset_status(pmudev_t *dev, int id, int *status);

int32_t pmu_set_internal_wakeup_enable(pmudev_t *dev, int id, int enable);

int32_t pmu_set_external_wakeup_enable(pmudev_t *dev, int id, int enable);

int32_t pmu_set_external_wakeup_polarity(pmudev_t *dev, int id, int polarity);

int32_t pmu_set_external_wakeup_debounce_enable(pmudev_t *dev, int id, int enable);

int32_t pmu_set_external_wakeup_debounce_delay(pmudev_t *dev, int id, int delay);

int32_t pmu_get_external_wakeup_status(pmudev_t *dev, int id, int *status);

int32_t pmu_set_glitch_filter_delay(pmudev_t *dev, int id, int delay);

int32_t pmu_set_glitch_filter_enable(pmudev_t *dev, int id, int enable);

int32_t pmu_set_por_powerdown_delay(pmudev_t *dev, int id, int delay);

int32_t pmu_set_por_auto_powerdown(pmudev_t *dev, int enable);

int32_t pmu_set_por_force_powerdown(pmudev_t *dev, int enable);

__END_CDECLS;
#endif
