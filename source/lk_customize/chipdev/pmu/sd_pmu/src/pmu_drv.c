/*
 * Copyright (c) 2018 Semidrive Semiconductor.
 * All rights reserved.
 */

#include <sys/types.h>
#include <kernel/mutex.h>
#include <trace.h>
#include <reg.h>
#include <assert.h>
#include "pmu_drv.h"


int32_t pmu_init(pmudev_t *dev)
{
	if (dev->name == NULL || dev->base_paddr == NULL
	        || dev->base_vaddr == NULL) {
		pmu_err("init failure\n");
		return -1;
	}

	dev->init_flag = 1;
	pmu_dbg("%s init okay\n", dev->name);
	return 0;
}

int32_t pmu_exit(pmudev_t *dev)
{
	if (dev->init_flag != 1) {
		pmu_err("exit failure\n");
		return -1;
	}

	dev->init_flag = 0;
	pmu_dbg("%s exit okay\n", dev->name);
	return 0;
}

static int32_t pmu_check(pmudev_t *dev)
{
	if (dev->name == NULL || dev->base_paddr == NULL
	        || dev->base_vaddr == NULL || dev->init_flag == 0)
		return 1;
	return 0;
}

static inline void pmu_modify_reg32(volatile uint8_t *addr, int startbit, int width, uint32_t val)
{
	RMWREG32(addr, startbit, width, val);
}

/**************************************************
 * pmu_get_status
 *
 * status value 0x01 - INIT
 *              0x02 - PWRUP
 *              0x04 - ON
 *              0x08 - PWRDOWN
 *              0x10 - OFF
 *              < 0  - ERROR
 ************************************************/
int32_t pmu_get_status(pmudev_t *dev, int *status)
{
	uint32_t v = 0;

	if (pmu_check(dev) || status == NULL) {
		pmu_err("check failure\n");
		return -1;
	}

	v = readl(dev->base_vaddr + PMU_RST_CTRL);
	v = (v & (0x1f << 24)) >> 24;
	if (v != 0x01 && v != 0x02 && v != 0x04 && v != 0x08 && v != 0x10) {
		pmu_err("get status fail v = %02x\n", v);
		return -1;
	}
	*status = v;
	return 0;
}

int32_t pmu_get_event_source(pmudev_t *dev, uint32_t *event)
{
	uint32_t v;

	if (pmu_check(dev) || event == NULL) {
		pmu_err("check failure\n");
		return -1;
	}

	v = readl(dev->base_vaddr + PMU_INTR);
	*event = v & 0xff;
	return 0;
}

int32_t pmu_clean_single_event_source(pmudev_t *dev, int event_id)
{
	if (pmu_check(dev) || event_id < 0 || event_id > 7) {
		pmu_err("check failure\n");
		return -1;
	}

	writel(1 << event_id, dev->base_vaddr + PMU_INTR);
	return 0;
}

int32_t pmu_clean_all_event_source(pmudev_t *dev)
{
	if (pmu_check(dev)) {
		pmu_err("check failure\n");
		return -1;
	}

	writel(0xff, dev->base_vaddr + PMU_INTR);
	return 0;
}

int32_t pmu_powerdown(pmudev_t *dev, int enable)
{
	if (pmu_check(dev)) {
		pmu_err("check failure\n");
		return -1;
	}
	pmu_modify_reg32(dev->base_vaddr + PMU_CTRL_0, 8, 1, 0x0);
	spin(1000);
	pmu_modify_reg32(dev->base_vaddr + PMU_CTRL_0, 8, 1, 0x1);
	return 0;
}

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
int32_t pmu_set_powerdown_delay(pmudev_t *dev, int id, int delay)
{
	if (pmu_check(dev) || delay < 0x0 || delay > 0xf) {
		pmu_err("check failure\n");
		return -1;
	}

	pmu_modify_reg32(dev->base_vaddr + PMU_CTRL_0, 4, 4, delay);
	return 0;
}

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
int32_t pmu_set_powerup_delay(pmudev_t *dev, int id, int delay)
{
	if (pmu_check(dev) || delay < 0x0 || delay > 0xf) {
		pmu_err("check failure\n");
		return -1;
	}

	pmu_modify_reg32(dev->base_vaddr + PMU_CTRL_0, 0, 4, delay);
	return 0;
}

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
int32_t pmu_set_powerctrl_powerdown_delay(pmudev_t *dev, int ctrl_id, int delay)
{
	int reg_offset, reg_start_bit;

	if (pmu_check(dev) || delay < 0x0 || delay > 0xf || ctrl_id < 0 || ctrl_id > 3) {
		pmu_err("check failure\n");
		return -1;
	}

	switch (ctrl_id) {
		case PWR_CTRL_0:
			reg_offset = PMU_CTRL_1;
			reg_start_bit = 4;
			break;
		case PWR_CTRL_1:
			reg_offset = PMU_CTRL_1;
			reg_start_bit = 15;
			break;
		case PWR_CTRL_2:
			reg_offset = PMU_CTRL_2;
			reg_start_bit = 4;
			break;
		case PWR_CTRL_3:
			reg_offset = PMU_CTRL_2;
			reg_start_bit = 15;
			break;
	}

	pmu_modify_reg32(dev->base_vaddr + reg_offset, reg_start_bit, 4, delay);
	return 0;
}

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
int32_t pmu_set_powerctrl_powerup_delay(pmudev_t *dev, int ctrl_id, int delay)
{
	int reg_offset, reg_start_bit;

	if (pmu_check(dev) || delay < 0x0 || delay > 0xf || ctrl_id < 0 || ctrl_id > 3) {
		pmu_err("check failure\n");
		return -1;
	}

	switch (ctrl_id) {
		case PWR_CTRL_0:
			reg_offset = PMU_CTRL_1;
			reg_start_bit = 0;
			break;
		case PWR_CTRL_1:
			reg_offset = PMU_CTRL_1;
			reg_start_bit = 11;
			break;
		case PWR_CTRL_2:
			reg_offset = PMU_CTRL_2;
			reg_start_bit = 0;
			break;
		case PWR_CTRL_3:
			reg_offset = PMU_CTRL_2;
			reg_start_bit = 11;
			break;
	}

	pmu_modify_reg32(dev->base_vaddr + reg_offset, reg_start_bit, 4, delay);
	return 0;
}

int32_t pmu_set_powerctrl_io_mode(pmudev_t *dev, int ctrl_id, int mode)
{
	int reg_offset, reg_start_bit;

	if (pmu_check(dev) || (mode != PWR_CTRL_INPUT && mode != PWR_CTRL_OUTPUT )
	        || ctrl_id < 0 || ctrl_id > 3) {
		pmu_err("check failure\n");
		return -1;
	}

	switch (ctrl_id) {
		case PWR_CTRL_0:
			reg_offset = PMU_CTRL_1;
			reg_start_bit = 8;
			break;
		case PWR_CTRL_1:
			reg_offset = PMU_CTRL_1;
			reg_start_bit = 19;
			break;
		case PWR_CTRL_2:
			reg_offset = PMU_CTRL_2;
			reg_start_bit = 8;
			break;
		case PWR_CTRL_3:
			reg_offset = PMU_CTRL_2;
			reg_start_bit = 19;
			break;
	}

	pmu_modify_reg32(dev->base_vaddr + reg_offset, reg_start_bit, 1, mode);
	return 0;
}

int32_t pmu_set_powerctrl_out_mode(pmudev_t *dev, int ctrl_id, int mode)
{
	int reg_offset, reg_start_bit;

	if (pmu_check(dev) || (mode != PWR_CTRL_OUTPUT_MANUAL && mode != PWR_CTRL_OUTPUT_AUTO )
	        || ctrl_id < 0 || ctrl_id > 3) {
		pmu_err("check failure\n");
		return -1;
	}

	switch (ctrl_id) {
		case PWR_CTRL_0:
			reg_offset = PMU_CTRL_1;
			reg_start_bit = 9;
			break;
		case PWR_CTRL_1:
			reg_offset = PMU_CTRL_1;
			reg_start_bit = 20;
			break;
		case PWR_CTRL_2:
			reg_offset = PMU_CTRL_2;
			reg_start_bit = 9;
			break;
		case PWR_CTRL_3:
			reg_offset = PMU_CTRL_2;
			reg_start_bit = 20;
			break;
	}

	pmu_modify_reg32(dev->base_vaddr + reg_offset, reg_start_bit, 1, mode);
	return 0;
}

int32_t pmu_set_powerctrl_out_ctrl(pmudev_t *dev, int ctrl_id, int out)
{
	int reg_offset, reg_start_bit;

	if (pmu_check(dev) || (out != PWR_CTRL_OUTPUT_LOW && out != PWR_CTRL_OUTPUT_HIGH)
	        || ctrl_id < 0 || ctrl_id > 3) {
		pmu_err("check failure\n");
		return -1;
	}

	switch (ctrl_id) {
		case PWR_CTRL_0:
			reg_offset = PMU_CTRL_1;
			reg_start_bit = 10;
			break;
		case PWR_CTRL_1:
			reg_offset = PMU_CTRL_1;
			reg_start_bit = 21;
			break;
		case PWR_CTRL_2:
			reg_offset = PMU_CTRL_2;
			reg_start_bit = 10;
			break;
		case PWR_CTRL_3:
			reg_offset = PMU_CTRL_2;
			reg_start_bit = 21;
			break;
	}

	pmu_modify_reg32(dev->base_vaddr + reg_offset, reg_start_bit, 1, out);
	return 0;
}

int32_t pmu_get_powerctrl_input_status(pmudev_t *dev, int ctrl_id, int *status)
{
	int reg_offset, reg_start_bit;
	uint32_t v;

	if (pmu_check(dev) || status == NULL || ctrl_id < 0 || ctrl_id > 3) {
		pmu_err("check failure\n");
		return -1;
	}

	switch (ctrl_id) {
		case PWR_CTRL_0:
			reg_offset = PMU_CTRL_1;
			reg_start_bit = 30;
			break;
		case PWR_CTRL_1:
			reg_offset = PMU_CTRL_1;
			reg_start_bit = 31;
			break;
		case PWR_CTRL_2:
			reg_offset = PMU_CTRL_2;
			reg_start_bit = 30;
			break;
		case PWR_CTRL_3:
			reg_offset = PMU_CTRL_2;
			reg_start_bit = 31;
			break;
	}

	v = readl(dev->base_vaddr + reg_offset);
	*status = ((v & (1 << reg_start_bit)) >> reg_start_bit) & 0x1;
	return 0;
}

int32_t pmu_set_internal_powerdown_enable(pmudev_t *dev, int id, int enable)
{
	int reg_start_bit;

	if (pmu_check(dev) || (enable != ENABLE && enable != DISABLE)
	        || id < 0 || id > 1) {
		pmu_err("check failure\n");
		return -1;
	}

	if (id == 0)
		reg_start_bit = 0;
	else
		reg_start_bit = 2;

	pmu_modify_reg32(dev->base_vaddr + PMU_CTRL_3, reg_start_bit, 1, enable);
	return 0;
}

int32_t pmu_set_internal_powerdown_polarity(pmudev_t *dev, int id, int polarity)
{
	int reg_start_bit;

	if (pmu_check(dev) || (polarity != HIGH_LEVEL && polarity != LOW_LEVEL)
	        || id < 0 || id > 1) {
		pmu_err("check failure\n");
		return -1;
	}

	if (id == 0)
		reg_start_bit = 1;
	else
		reg_start_bit = 3;

	pmu_modify_reg32(dev->base_vaddr + PMU_CTRL_3, reg_start_bit, 1, polarity);
	return 0;
}

int32_t pmu_get_internal_powerdown_status(pmudev_t *dev, int id, int *status)
{
	int reg_start_bit;
	uint32_t v;

	if (pmu_check(dev) || status == NULL || id < 0 || id > 1) {
		pmu_err("check failure\n");
		return -1;
	}

	if (id == 0)
		reg_start_bit = 8;
	else
		reg_start_bit = 9;

	v = readl(dev->base_vaddr + PMU_CTRL_3);
	*status = ((v & (1 << reg_start_bit)) >> reg_start_bit) & 0x1;
	return 0;
}

int32_t pmu_set_external_reset_enable(pmudev_t *dev, int id, int enable)
{
	int reg_start_bit;

	if (pmu_check(dev) || (enable != ENABLE && enable != DISABLE)
	        || id < 0 || id > 1) {
		pmu_err("check failure\n");
		return -1;
	}

	if (id == 0)
		reg_start_bit = 0;
	else
		reg_start_bit = 7;

	pmu_modify_reg32(dev->base_vaddr + PMU_EXT_CRTL, reg_start_bit, 1, enable);
	return 0;
}

int32_t pmu_set_external_reset_polarity(pmudev_t *dev, int id, int polarity)
{
	int reg_start_bit;

	if (pmu_check(dev) || (polarity != HIGH_LEVEL && polarity != LOW_LEVEL)
	        || id < 0 || id > 1) {
		pmu_err("check failure\n");
		return -1;
	}

	if (id == 0)
		reg_start_bit = 1;
	else
		reg_start_bit = 8;

	pmu_modify_reg32(dev->base_vaddr + PMU_EXT_CRTL, reg_start_bit, 1, polarity);
	return 0;
}

int32_t pmu_set_external_reset_debounce_enable(pmudev_t *dev, int id, int enable)
{
	int reg_start_bit;

	if (pmu_check(dev) || (enable != ENABLE && enable != DISABLE)
	        || id < 0 || id > 1) {
		pmu_err("check failure\n");
		return -1;
	}

	if (id == 0)
		reg_start_bit = 2;
	else
		reg_start_bit = 9;

	pmu_modify_reg32(dev->base_vaddr + PMU_EXT_CRTL, reg_start_bit, 1, enable);
	return 0;
}

int32_t pmu_set_external_reset_debounce_delay(pmudev_t *dev, int id, int delay)
{
	int reg_start_bit;

	if (pmu_check(dev) || delay < 0 || delay > 0xf || id < 0 || id > 1) {
		pmu_err("check failure\n");
		return -1;
	}

	if (id == 0)
		reg_start_bit = 3;
	else
		reg_start_bit = 10;

	pmu_modify_reg32(dev->base_vaddr + PMU_EXT_CRTL, reg_start_bit, 4, delay);
	return 0;
}

int32_t pmu_get_external_reset_status(pmudev_t *dev, int id, int *status)
{
	int reg_start_bit;
	uint32_t v;

	if (pmu_check(dev) || status == NULL || id < 0 || id > 1) {
		pmu_err("check failure\n");
		return -1;
	}

	if (id == 0)
		reg_start_bit = 28;
	else
		reg_start_bit = 29;

	v = readl(dev->base_vaddr + PMU_EXT_CRTL);
	*status = ((v & (1 << reg_start_bit)) >> reg_start_bit) & 0x1;
	return 0;
}

int32_t pmu_set_internal_wakeup_enable(pmudev_t *dev, int id, int enable)
{
	if (pmu_check(dev) || (enable != ENABLE && enable != DISABLE)
	        || id < 0 || id > 1) {
		pmu_err("check failure\n");
		return -1;
	}

	pmu_modify_reg32(dev->base_vaddr + PMU_RTC_CTRL, id, 1, enable);
	return 0;
}

int32_t pmu_set_external_wakeup_enable(pmudev_t *dev, int id, int enable)
{
	int reg_start_bit;

	if (pmu_check(dev) || (enable != ENABLE && enable != DISABLE)
	        || id < 0 || id > 1) {
		pmu_err("check failure\n");
		return -1;
	}

	if (id == 0)
		reg_start_bit = 14;
	else
		reg_start_bit = 21;

	pmu_modify_reg32(dev->base_vaddr + PMU_EXT_CRTL, reg_start_bit, 1, enable);
	return 0;
}

int32_t pmu_set_external_wakeup_polarity(pmudev_t *dev, int id, int polarity)
{
	int reg_start_bit;

	if (pmu_check(dev) || (polarity != HIGH_LEVEL && polarity != LOW_LEVEL)
	        || id < 0 || id > 1) {
		pmu_err("check failure\n");
		return -1;
	}

	if (id == 0)
		reg_start_bit = 15;
	else
		reg_start_bit = 22;

	pmu_modify_reg32(dev->base_vaddr + PMU_EXT_CRTL, reg_start_bit, 1, polarity);
	return 0;
}

int32_t pmu_set_external_wakeup_debounce_enable(pmudev_t *dev, int id, int enable)
{
	int reg_start_bit;

	if (pmu_check(dev) || (enable != ENABLE && enable != DISABLE)
	        || id < 0 || id > 1) {
		pmu_err("check failure\n");
		return -1;
	}

	if (id == 0)
		reg_start_bit = 16;
	else
		reg_start_bit = 23;

	pmu_modify_reg32(dev->base_vaddr + PMU_EXT_CRTL, reg_start_bit, 1, enable);
	return 0;
}

int32_t pmu_set_external_wakeup_debounce_delay(pmudev_t *dev, int id, int delay)
{
	int reg_start_bit;

	if (pmu_check(dev) || delay < 0 || delay > 0xf || id < 0 || id > 1) {
		pmu_err("check failure\n");
		return -1;
	}

	if (id == 0)
		reg_start_bit = 17;
	else
		reg_start_bit = 24;

	pmu_modify_reg32(dev->base_vaddr + PMU_EXT_CRTL, reg_start_bit, 4, delay);
	return 0;
}

int32_t pmu_get_external_wakeup_status(pmudev_t *dev, int id, int *status)
{
	int reg_start_bit;
	uint32_t v;

	if (pmu_check(dev) || status == NULL || id < 0 || id > 1) {
		pmu_err("check failure\n");
		return -1;
	}

	if (id == 0)
		reg_start_bit = 30;
	else
		reg_start_bit = 31;

	v = readl(dev->base_vaddr + PMU_EXT_CRTL);
	*status = ((v & (1 << reg_start_bit)) >> reg_start_bit) & 0x1;
	return 0;
}

int32_t pmu_set_glitch_filter_delay(pmudev_t *dev, int id, int delay)
{
	if (pmu_check(dev) || delay < 0 || delay > 0x7) {
		pmu_err("check failure\n");
		return -1;
	}

	pmu_modify_reg32(dev->base_vaddr + PMU_GF_CTRL, 11, 3, delay);
	return 0;
}

int32_t pmu_set_glitch_filter_enable(pmudev_t *dev, int id, int enable)
{
	if (pmu_check(dev) || (enable != ENABLE && enable != DISABLE)
	        || id < 0 || id > 10) {
		pmu_err("check failure\n");
		return -1;
	}

	pmu_modify_reg32(dev->base_vaddr + PMU_GF_CTRL, id, 1, enable);
	return 0;
}

int32_t pmu_set_por_powerdown_delay(pmudev_t *dev, int id, int delay)
{
	if (pmu_check(dev) || delay < 0 || delay > 0x7) {
		pmu_err("check failure\n");
		return -1;
	}

	pmu_modify_reg32(dev->base_vaddr + PMU_RST_CTRL, 11, 3, delay);
	return 0;
}

int32_t pmu_set_por_auto_powerdown(pmudev_t *dev, int enable)
{
	if (pmu_check(dev) || (enable != ENABLE && enable != DISABLE)) {
		pmu_err("check failure\n");
		return -1;
	}

	pmu_modify_reg32(dev->base_vaddr + PMU_RST_CTRL, 10, 1, enable);
	return 0;
}

int32_t pmu_set_por_force_powerdown(pmudev_t *dev, int enable)
{
	if (pmu_check(dev) || (enable != ENABLE && enable != DISABLE)) {
		pmu_err("check failure\n");
		return -1;
	}

	pmu_modify_reg32(dev->base_vaddr + PMU_RST_CTRL, 9, 1, enable);
	return 0;
}
