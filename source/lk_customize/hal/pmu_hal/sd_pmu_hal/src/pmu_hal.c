#include <platform/interrupts.h>
#include <sys/types.h>
#include <platform/debug.h>
#include <trace.h>
#include <string.h>
#include <assert.h>
#include <platform.h>
#include "res.h"
#include "pmu_hal.h"

pmu_drv_ops_t drv_ops;

pmudev_t dev = {
	.name = "pmu",
};

void hal_pmu_drv_ops_init(void)
{
	memset(&drv_ops, 0, sizeof(pmu_drv_ops_t));

	/* global ops */
	drv_ops.global.init = pmu_init;
	drv_ops.global.exit = pmu_exit;
	drv_ops.global.get_status = pmu_get_status;
	drv_ops.global.get_event_source = pmu_get_event_source;
	drv_ops.global.clean_single_event = pmu_clean_single_event_source;
	drv_ops.global.clean_all_event = pmu_clean_all_event_source;

	/* pwr_on*/
	drv_ops.pwr_on.set_pu_delay = pmu_set_powerup_delay;
	drv_ops.pwr_on.set_pd_delay = pmu_set_powerdown_delay;
	drv_ops.pwr_on.force_powerdown = pmu_powerdown;

	/*pwr_ctrl*/
	drv_ops.pwr_ctrl.set_pu_delay = pmu_set_powerctrl_powerup_delay;
	drv_ops.pwr_ctrl.set_pd_delay = pmu_set_powerctrl_powerdown_delay;
	drv_ops.pwr_ctrl.set_io_mode = pmu_set_powerctrl_io_mode;
	drv_ops.pwr_ctrl.set_out_mode = pmu_set_powerctrl_out_mode;
	drv_ops.pwr_ctrl.set_out_ctrl = pmu_set_powerctrl_out_ctrl;
	drv_ops.pwr_ctrl.get_input_status = pmu_get_powerctrl_input_status;

	/*por*/
	drv_ops.por.set_pd_delay = pmu_set_por_powerdown_delay;
	drv_ops.por.auto_powerdown = pmu_set_por_auto_powerdown;
	drv_ops.por.force_powerdown = pmu_set_por_force_powerdown;

	/*internal powerdown*/
	drv_ops.internal_powerdown.set_enable = pmu_set_internal_powerdown_enable;
	drv_ops.internal_powerdown.set_polarity = pmu_set_internal_powerdown_polarity;
	drv_ops.internal_powerdown.get_status = pmu_get_internal_powerdown_status;

	/*external reset*/
	drv_ops.external_reset.set_enable = pmu_set_external_reset_enable;
	drv_ops.external_reset.set_polarity = pmu_set_external_reset_polarity;
	drv_ops.external_reset.set_debounce_enable = pmu_set_external_reset_debounce_enable;
	drv_ops.external_reset.set_debounce_delay = pmu_set_external_reset_debounce_delay;
	drv_ops.external_reset.get_status = pmu_get_external_reset_status;

	/*internal wakeup*/
	drv_ops.internal_wakeup.set_enable = pmu_set_internal_wakeup_enable;

	/*external wakeup*/
	drv_ops.external_wakeup.set_enable = pmu_set_external_wakeup_enable;
	drv_ops.external_wakeup.set_polarity = pmu_set_external_wakeup_polarity;
	drv_ops.external_wakeup.set_debounce_enable = pmu_set_external_wakeup_debounce_enable;
	drv_ops.external_wakeup.set_debounce_delay = pmu_set_external_wakeup_debounce_delay;
	drv_ops.external_wakeup.get_status = pmu_get_external_wakeup_status;

	/*glitch filter*/
	drv_ops.glitch_filter.set_debounce_enable = pmu_set_glitch_filter_enable;
	drv_ops.glitch_filter.set_debounce_delay = pmu_set_glitch_filter_delay;
}


int32_t hal_pmu_creat_handle(void **handle, uint32_t pmu_res_glb_idx)
{
	static int flag = 0;
	int32_t ret;
	addr_t paddr;

	if (handle == NULL) {
		pmu_err("parameters is wrong\n");
		return -1;
	}

	ret = res_get_info_by_id(pmu_res_glb_idx, &paddr, &dev.id);
	if (ret < 0) {
		pmu_err("res_get_info_by_id fail\n");
		return ret;
	}

	if (flag == 0) {
		hal_pmu_drv_ops_init();
		dev.priv_data = (void *)&drv_ops;
		dev.base_paddr = (volatile uint8_t *)paddr;
		dev.base_vaddr = (volatile uint8_t *)phys_to_virt(paddr);
		flag = 1;
	}

	*handle = (void *)&dev;
	return 0;
}


int32_t hal_pmu_release_handle(void *handle)
{
	return 0;
}

int32_t hal_pmu_init(void *handle)
{
	pmudev_t *dev = (pmudev_t *)handle;
	pmu_drv_ops_t *ops;

	if (dev == NULL || dev->priv_data == NULL) {
		pmu_err("parameters is wrong\n");
		return -1;
	}

	ops = (pmu_drv_ops_t *)dev->priv_data;

	if (ops->global.init != NULL)
		return ops->global.init(dev);

	pmu_err("%s is not support\n", __func__);
	return -1;
}

int32_t hal_pmu_exit(void *handle)
{
	pmudev_t *dev = (pmudev_t *)handle;
	pmu_drv_ops_t *ops;

	if (dev == NULL || dev->priv_data == NULL) {
		pmu_err("parameters is wrong\n");
		return -1;
	}

	ops = (pmu_drv_ops_t *)dev->priv_data;

	if (ops->global.exit != NULL)
		return ops->global.exit(dev);

	pmu_err("%s is not support\n", __func__);
	return -1;
}

/**************************************************
 * hal_pmu_get_status
 *
 * status value 0x01 - INIT Mode
 *              0x02 - PWRUP Mode
 *              0x04 - ON Mode
 *              0x08 - PWRDOWN Mode
 *              0x10 - OFF Mode
 *              < 0  - error
 ************************************************/
int32_t hal_pmu_get_status(void *handle, int *status)
{
	pmudev_t *dev = (pmudev_t *)handle;
	pmu_drv_ops_t *ops;

	if (dev == NULL || dev->priv_data == NULL || status == NULL) {
		pmu_err("parameters is wrong\n");
		return -1;
	}

	ops = (pmu_drv_ops_t *)dev->priv_data;

	if (ops->global.get_status != NULL)
		return ops->global.get_status(dev, status);

	pmu_err("%s is not support\n", __func__);
	return -1;
}

int32_t hal_pmu_get_event_source(void *handle, uint32_t *event)
{
	pmudev_t *dev = (pmudev_t *)handle;
	pmu_drv_ops_t *ops;

	if (dev == NULL || dev->priv_data == NULL || event == NULL) {
		pmu_err("parameters is wrong\n");
		return -1;
	}

	ops = (pmu_drv_ops_t *)dev->priv_data;

	if (ops->global.get_event_source != NULL)
		return ops->global.get_event_source(dev, event);

	pmu_err("%s is not support\n", __func__);
	return -1;
}

int32_t hal_pmu_clean_single_event_source(void *handle, int event_id)
{
	pmudev_t *dev = (pmudev_t *)handle;
	pmu_drv_ops_t *ops;

	if (dev == NULL || dev->priv_data == NULL) {
		pmu_err("parameters is wrong\n");
		return -1;
	}

	ops = (pmu_drv_ops_t *)dev->priv_data;

	if (ops->global.clean_single_event != NULL)
		return ops->global.clean_single_event(dev, event_id);

	pmu_err("%s is not support\n", __func__);
	return -1;
}

int32_t hal_pmu_clean_all_event_source(void *handle)
{
	pmudev_t *dev = (pmudev_t *)handle;
	pmu_drv_ops_t *ops;

	if (dev == NULL || dev->priv_data == NULL) {
		pmu_err("parameters is wrong\n");
		return -1;
	}

	ops = (pmu_drv_ops_t *)dev->priv_data;

	if (ops->global.clean_all_event != NULL)
		return ops->global.clean_all_event(dev);

	pmu_err("%s is not support\n", __func__);
	return -1;
}

int32_t hal_pmu_powerdown(void *handle)
{
	pmudev_t *dev = (pmudev_t *)handle;
	pmu_drv_ops_t *ops;

	if (dev == NULL || dev->priv_data == NULL) {
		pmu_err("parameters is wrong\n");
		return -1;
	}

	ops = (pmu_drv_ops_t *)dev->priv_data;

	if (ops->pwr_on.force_powerdown != NULL)
		return ops->pwr_on.force_powerdown(dev, 1);

	pmu_err("%s is not support\n", __func__);
	return -1;
}

/******************************************************************************
 * hal_pmu_set_powerdown_delay
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
int32_t hal_pmu_set_powerdown_delay(void *handle, int delay)
{
	pmudev_t *dev = (pmudev_t *)handle;
	pmu_drv_ops_t *ops;

	if (dev == NULL || dev->priv_data == NULL) {
		pmu_err("parameters is wrong\n");
		return -1;
	}

	ops = (pmu_drv_ops_t *)dev->priv_data;

	if (ops->pwr_on.set_pd_delay != NULL)
		return ops->pwr_on.set_pd_delay(dev, 0, delay);

	pmu_err("%s is not support\n", __func__);
	return -1;
}


/******************************************************************************
 * hal_pmu_set_powerup_delay
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
int32_t hal_pmu_set_powerup_delay(void *handle, int delay)
{
	pmudev_t *dev = (pmudev_t *)handle;
	pmu_drv_ops_t *ops;

	if (dev == NULL || dev->priv_data == NULL) {
		pmu_err("parameters is wrong\n");
		return -1;
	}

	ops = (pmu_drv_ops_t *)dev->priv_data;

	if (ops->pwr_on.set_pu_delay != NULL)
		return ops->pwr_on.set_pu_delay(dev, 0, delay);

	pmu_err("%s is not support\n", __func__);
	return -1;
}

/******************************************************************************
 * hal_pmu_set_powerctrl_powerdown_delay
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
int32_t hal_pmu_set_powerctrl_powerdown_delay(void *handle, int ctrl_id, int delay)
{
	pmudev_t *dev = (pmudev_t *)handle;
	pmu_drv_ops_t *ops;

	if (dev == NULL || dev->priv_data == NULL) {
		pmu_err("parameters is wrong\n");
		return -1;
	}

	ops = (pmu_drv_ops_t *)dev->priv_data;

	if (ops->pwr_ctrl.set_pd_delay != NULL)
		return ops->pwr_ctrl.set_pd_delay(dev, ctrl_id, delay);

	pmu_err("%s is not support\n", __func__);
	return -1;
}

/******************************************************************************
 * hal_pmu_set_powerctrl_powerup_delay
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
int32_t hal_pmu_set_powerctrl_powerup_delay(void *handle, int ctrl_id, int delay)
{
	pmudev_t *dev = (pmudev_t *)handle;
	pmu_drv_ops_t *ops;

	if (dev == NULL || dev->priv_data == NULL) {
		pmu_err("parameters is wrong\n");
		return -1;
	}

	ops = (pmu_drv_ops_t *)dev->priv_data;

	if (ops->pwr_ctrl.set_pu_delay != NULL)
		return ops->pwr_ctrl.set_pu_delay(dev, ctrl_id, delay);

	pmu_err("%s is not support\n", __func__);
	return -1;
}

int32_t hal_pmu_set_powerctrl_io_mode(void *handle, int ctrl_id, int mode)
{
	pmudev_t *dev = (pmudev_t *)handle;
	pmu_drv_ops_t *ops;

	if (dev == NULL || dev->priv_data == NULL) {
		pmu_err("parameters is wrong\n");
		return -1;
	}

	ops = (pmu_drv_ops_t *)dev->priv_data;

	if (ops->pwr_ctrl.set_io_mode != NULL)
		return ops->pwr_ctrl.set_io_mode(dev, ctrl_id, mode);

	pmu_err("%s is not support\n", __func__);
	return -1;
}

int32_t hal_pmu_set_powerctrl_out_mode(void *handle, int ctrl_id, int mode)
{
	pmudev_t *dev = (pmudev_t *)handle;
	pmu_drv_ops_t *ops;

	if (dev == NULL || dev->priv_data == NULL) {
		pmu_err("parameters is wrong\n");
		return -1;
	}

	ops = (pmu_drv_ops_t *)dev->priv_data;

	if (ops->pwr_ctrl.set_out_mode != NULL)
		return ops->pwr_ctrl.set_out_mode(dev, ctrl_id, mode);

	pmu_err("%s is not support\n", __func__);
	return -1;
}

int32_t hal_pmu_set_powerctrl_out_ctrl(void *handle, int ctrl_id, int out)
{
	pmudev_t *dev = (pmudev_t *)handle;
	pmu_drv_ops_t *ops;

	if (dev == NULL || dev->priv_data == NULL) {
		pmu_err("parameters is wrong\n");
		return -1;
	}

	ops = (pmu_drv_ops_t *)dev->priv_data;

	if (ops->pwr_ctrl.set_out_ctrl != NULL)
		return ops->pwr_ctrl.set_out_ctrl(dev, ctrl_id, out);

	pmu_err("%s is not support\n", __func__);
	return -1;
}

int32_t hal_pmu_get_powerctrl_input_status(void *handle, int ctrl_id, int *status)
{
	pmudev_t *dev = (pmudev_t *)handle;
	pmu_drv_ops_t *ops;

	if (dev == NULL || dev->priv_data == NULL || status == NULL) {
		pmu_err("parameters is wrong\n");
		return -1;
	}

	ops = (pmu_drv_ops_t *)dev->priv_data;

	if (ops->pwr_ctrl.get_input_status != NULL)
		return ops->pwr_ctrl.get_input_status(dev, ctrl_id, status);

	pmu_err("%s is not support\n", __func__);
	return -1;
}

int32_t hal_pmu_set_internal_powerdown_enable(void *handle, int id, int enable)
{
	pmudev_t *dev = (pmudev_t *)handle;
	pmu_drv_ops_t *ops;

	if (dev == NULL || dev->priv_data == NULL) {
		pmu_err("parameters is wrong\n");
		return -1;
	}

	ops = (pmu_drv_ops_t *)dev->priv_data;

	if (ops->internal_powerdown.set_enable != NULL)
		return ops->internal_powerdown.set_enable(dev, id, enable);

	pmu_err("%s is not support\n", __func__);
	return -1;
}

int32_t hal_pmu_set_internal_powerdown_polarity(void *handle, int id, int polarity)
{
	pmudev_t *dev = (pmudev_t *)handle;
	pmu_drv_ops_t *ops;

	if (dev == NULL || dev->priv_data == NULL) {
		pmu_err("parameters is wrong\n");
		return -1;
	}

	ops = (pmu_drv_ops_t *)dev->priv_data;

	if (ops->internal_powerdown.set_polarity != NULL)
		return ops->internal_powerdown.set_polarity(dev, id, polarity);

	pmu_err("%s is not support\n", __func__);
	return -1;
}

int32_t hal_pmu_get_internal_powerdown_status(void *handle, int id, int *status)
{
	pmudev_t *dev = (pmudev_t *)handle;
	pmu_drv_ops_t *ops;

	if (dev == NULL || dev->priv_data == NULL || status == NULL) {
		pmu_err("parameters is wrong\n");
		return -1;
	}

	ops = (pmu_drv_ops_t *)dev->priv_data;

	if (ops->internal_powerdown.get_status != NULL)
		return ops->internal_powerdown.get_status(dev, id, status);

	pmu_err("%s is not support\n", __func__);
	return -1;
}

int32_t hal_pmu_set_external_reset_enable(void *handle, int id, int enable)
{
	pmudev_t *dev = (pmudev_t *)handle;
	pmu_drv_ops_t *ops;

	if (dev == NULL || dev->priv_data == NULL) {
		pmu_err("parameters is wrong\n");
		return -1;
	}

	ops = (pmu_drv_ops_t *)dev->priv_data;

	if (ops->external_reset.set_enable != NULL)
		return ops->external_reset.set_enable(dev, id, enable);

	pmu_err("%s is not support\n", __func__);
	return -1;
}

int32_t hal_pmu_set_external_reset_polarity(void *handle, int id, int polarity)
{
	pmudev_t *dev = (pmudev_t *)handle;
	pmu_drv_ops_t *ops;

	if (dev == NULL || dev->priv_data == NULL) {
		pmu_err("parameters is wrong\n");
		return -1;
	}

	ops = (pmu_drv_ops_t *)dev->priv_data;

	if (ops->external_reset.set_polarity != NULL)
		return ops->external_reset.set_polarity(dev, id, polarity);

	pmu_err("%s is not support\n", __func__);
	return -1;
}

int32_t hal_pmu_set_external_reset_debounce_enable(void *handle, int id, int enable)
{
	pmudev_t *dev = (pmudev_t *)handle;
	pmu_drv_ops_t *ops;

	if (dev == NULL || dev->priv_data == NULL) {
		pmu_err("parameters is wrong\n");
		return -1;
	}

	ops = (pmu_drv_ops_t *)dev->priv_data;

	if (ops->external_reset.set_debounce_enable != NULL)
		return ops->external_reset.set_debounce_enable(dev, id, enable);

	pmu_err("%s is not support\n", __func__);
	return -1;
}

int32_t hal_pmu_set_external_reset_debounce_delay(void *handle, int id, int delay)
{
	pmudev_t *dev = (pmudev_t *)handle;
	pmu_drv_ops_t *ops;

	if (dev == NULL || dev->priv_data == NULL) {
		pmu_err("parameters is wrong\n");
		return -1;
	}

	ops = (pmu_drv_ops_t *)dev->priv_data;

	if (ops->external_reset.set_debounce_delay != NULL)
		return ops->external_reset.set_debounce_delay(dev, id, delay);

	pmu_err("%s is not support\n", __func__);
	return -1;
}

int32_t hal_pmu_get_external_reset_status(void *handle, int id, int *status)
{
	pmudev_t *dev = (pmudev_t *)handle;
	pmu_drv_ops_t *ops;

	if (dev == NULL || dev->priv_data == NULL || status == NULL) {
		pmu_err("parameters is wrong\n");
		return -1;
	}

	ops = (pmu_drv_ops_t *)dev->priv_data;

	if (ops->external_reset.get_status != NULL)
		return ops->external_reset.get_status(dev, id, status);

	pmu_err("%s is not support\n", __func__);
	return -1;
}

int32_t hal_pmu_set_internal_wakeup_enable(void *handle, int id, int enable)
{
	pmudev_t *dev = (pmudev_t *)handle;
	pmu_drv_ops_t *ops;

	if (dev == NULL || dev->priv_data == NULL) {
		pmu_err("parameters is wrong\n");
		return -1;
	}

	ops = (pmu_drv_ops_t *)dev->priv_data;

	if (ops->internal_wakeup.set_enable != NULL)
		return ops->internal_wakeup.set_enable(dev, id, enable);

	pmu_err("%s is not support\n", __func__);
	return -1;
}

int32_t hal_pmu_set_external_wakeup_enable(void *handle, int id, int enable)
{
	pmudev_t *dev = (pmudev_t *)handle;
	pmu_drv_ops_t *ops;

	if (dev == NULL || dev->priv_data == NULL) {
		pmu_err("parameters is wrong\n");
		return -1;
	}

	ops = (pmu_drv_ops_t *)dev->priv_data;

	if (ops->external_wakeup.set_enable != NULL)
		return ops->external_wakeup.set_enable(dev, id, enable);

	pmu_err("%s is not support\n", __func__);
	return -1;
}

int32_t hal_pmu_set_external_wakeup_polarity(void *handle, int id, int polarity)
{
	pmudev_t *dev = (pmudev_t *)handle;
	pmu_drv_ops_t *ops;

	if (dev == NULL || dev->priv_data == NULL) {
		pmu_err("parameters is wrong\n");
		return -1;
	}

	ops = (pmu_drv_ops_t *)dev->priv_data;

	if (ops->external_wakeup.set_polarity != NULL)
		return ops->external_wakeup.set_polarity(dev, id, polarity);

	pmu_err("%s is not support\n", __func__);
	return -1;
}

int32_t hal_pmu_set_external_wakeup_debounce_enable(void *handle, int id, int enable)
{
	pmudev_t *dev = (pmudev_t *)handle;
	pmu_drv_ops_t *ops;

	if (dev == NULL || dev->priv_data == NULL) {
		pmu_err("parameters is wrong\n");
		return -1;
	}

	ops = (pmu_drv_ops_t *)dev->priv_data;

	if (ops->external_wakeup.set_debounce_enable != NULL)
		return ops->external_wakeup.set_debounce_enable(dev, id, enable);

	pmu_err("%s is not support\n", __func__);
	return -1;
}

int32_t hal_pmu_set_external_wakeup_debounce_delay(void *handle, int id, int delay)
{
	pmudev_t *dev = (pmudev_t *)handle;
	pmu_drv_ops_t *ops;

	if (dev == NULL || dev->priv_data == NULL) {
		pmu_err("parameters is wrong\n");
		return -1;
	}

	ops = (pmu_drv_ops_t *)dev->priv_data;

	if (ops->external_wakeup.set_debounce_delay != NULL)
		return ops->external_wakeup.set_debounce_delay(dev, id, delay);

	pmu_err("%s is not support\n", __func__);
	return -1;
}

int32_t hal_pmu_get_external_wakeup_status(void *handle, int id, int *status)
{
	pmudev_t *dev = (pmudev_t *)handle;
	pmu_drv_ops_t *ops;

	if (dev == NULL || dev->priv_data == NULL || status == NULL) {
		pmu_err("parameters is wrong\n");
		return -1;
	}

	ops = (pmu_drv_ops_t *)dev->priv_data;

	if (ops->external_wakeup.get_status != NULL)
		return ops->external_wakeup.get_status(dev, id, status);

	pmu_err("%s is not support\n", __func__);
	return -1;
}

int32_t hal_pmu_set_glitch_filter_delay(void *handle, int delay)
{
	pmudev_t *dev = (pmudev_t *)handle;
	pmu_drv_ops_t *ops;

	if (dev == NULL || dev->priv_data == NULL) {
		pmu_err("parameters is wrong\n");
		return -1;
	}

	ops = (pmu_drv_ops_t *)dev->priv_data;

	if (ops->glitch_filter.set_debounce_delay != NULL)
		return ops->glitch_filter.set_debounce_delay(dev, 0, delay);

	pmu_err("%s is not support\n", __func__);
	return -1;
}

int32_t hal_pmu_set_glitch_filter_enable(void *handle, int id, int enable)
{
	pmudev_t *dev = (pmudev_t *)handle;
	pmu_drv_ops_t *ops;

	if (dev == NULL || dev->priv_data == NULL) {
		pmu_err("parameters is wrong\n");
		return -1;
	}

	ops = (pmu_drv_ops_t *)dev->priv_data;

	if (ops->glitch_filter.set_debounce_enable != NULL)
		return ops->glitch_filter.set_debounce_enable(dev, id, enable);

	pmu_err("%s is not support\n", __func__);
	return -1;
}

int32_t hal_pmu_set_por_powerdown_delay(void *handle, int delay)
{
	pmudev_t *dev = (pmudev_t *)handle;
	pmu_drv_ops_t *ops;

	if (dev == NULL || dev->priv_data == NULL) {
		pmu_err("parameters is wrong\n");
		return -1;
	}

	ops = (pmu_drv_ops_t *)dev->priv_data;

	if (ops->por.set_pd_delay != NULL)
		return ops->por.set_pd_delay(dev, 0, delay);

	pmu_err("%s is not support\n", __func__);
	return -1;
}

int32_t hal_pmu_set_por_auto_powerdown(void *handle, int enable)
{
	pmudev_t *dev = (pmudev_t *)handle;
	pmu_drv_ops_t *ops;

	if (dev == NULL || dev->priv_data == NULL) {
		pmu_err("parameters is wrong\n");
		return -1;
	}

	ops = (pmu_drv_ops_t *)dev->priv_data;

	if (ops->por.auto_powerdown != NULL)
		return ops->por.auto_powerdown(dev, enable);

	pmu_err("%s is not support\n", __func__);
	return -1;
}

int32_t hal_pmu_set_por_force_powerdown(void *handle, int enable)
{
	pmudev_t *dev = (pmudev_t *)handle;
	pmu_drv_ops_t *ops;

	if (dev == NULL || dev->priv_data == NULL) {
		pmu_err("parameters is wrong\n");
		return -1;
	}

	ops = (pmu_drv_ops_t *)dev->priv_data;

	if (ops->por.force_powerdown != NULL)
		return ops->por.force_powerdown(dev, enable);

	pmu_err("%s is not support\n", __func__);
	return -1;
}

