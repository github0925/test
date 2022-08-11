#ifndef __PMU_HAL_H
#define __PMU_HAL_H
#include <sys/types.h>
#include <compiler.h>
#include "pmu_drv.h"

__BEGIN_CDECLS;

#if WITH_KERNEL_VM
#include <kernel/vm.h>
# define phys_to_virt(pa)    (paddr_to_kvaddr(pa))
#else
# define phys_to_virt(pa)    (pa)
#endif

typedef struct _pmu_global_ops {
	int32_t (*init)(struct pmu_device *dev);
	int32_t (*exit)(struct pmu_device *dev);
	int32_t (*get_status)(struct pmu_device *dev, int *status);
	int32_t (*get_event_source)(struct pmu_device *dev, uint32_t *event);
	int32_t (*clean_single_event)(struct pmu_device *dev, int event_id);
	int32_t (*clean_all_event)(struct pmu_device *dev);
} pmu_global_ops_t;

typedef struct _pmu_ctrl_ops {
	int32_t (*auto_powerdown)(struct pmu_device *dev, int enable);
	int32_t (*force_powerdown)(struct pmu_device *dev, int enable);
	int32_t (*set_pd_delay)(struct pmu_device *dev, int ctrl_id, int delay);
	int32_t (*set_pu_delay)(struct pmu_device *dev, int ctrl_id, int delay);
	int32_t (*set_io_mode)(struct pmu_device *dev, int ctrl_id, int mode);
	int32_t (*set_out_mode)(struct pmu_device *dev, int ctrl_id, int mode);
	int32_t (*set_out_ctrl)(struct pmu_device *dev, int ctrl_id, int out);
	int32_t (*get_input_status)(struct pmu_device *dev, int ctrl_id, int *status);
} pmu_ctrl_ops_t;

typedef struct _pmu_ops {
	int32_t (*set_enable)(struct pmu_device *dev, int id, int enable);
	int32_t (*set_polarity)(struct pmu_device *dev, int id, int polarity);
	int32_t (*set_debounce_enable)(struct pmu_device *dev, int id, int enable);
	int32_t (*set_debounce_delay)(struct pmu_device *dev, int id, int delay);
	int32_t (*get_status)(struct pmu_device *dev, int id, int *status);
} pmu_ops_t;

typedef struct _pmu_drv_ops {
	pmu_global_ops_t global;
	pmu_ctrl_ops_t pwr_on;
	pmu_ctrl_ops_t pwr_ctrl;
	pmu_ctrl_ops_t por;
	pmu_ops_t internal_powerdown;
	pmu_ops_t external_reset;
	pmu_ops_t internal_wakeup;
	pmu_ops_t external_wakeup;
	pmu_ops_t glitch_filter;
} pmu_drv_ops_t;

int32_t hal_pmu_creat_handle(void **handle, uint32_t pmu_res_glb_idx);
int32_t hal_pmu_release_handle(void *handle);
int32_t hal_pmu_init(void *handle);
int32_t hal_pmu_exit(void *handle);

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
int32_t hal_pmu_get_status(void *handle, int *status);


int32_t hal_pmu_get_event_source(void *handle, uint32_t *event);

int32_t hal_pmu_clean_single_event_source(void *handle, int event_id);

int32_t hal_pmu_clean_all_event_source(void *handle);

int32_t hal_pmu_powerdown(void *handle);

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
int32_t hal_pmu_set_powerdown_delay(void *handle, int delay);

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
int32_t hal_pmu_set_powerup_delay(void *handle, int delay);


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
int32_t hal_pmu_set_powerctrl_powerdown_delay(void *handle, int ctrl_id, int delay);


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
int32_t hal_pmu_set_powerctrl_powerup_delay(void *handle, int ctrl_id, int delay);
int32_t hal_pmu_set_powerctrl_io_mode(void *handle, int ctrl_id, int mode);

int32_t hal_pmu_set_powerctrl_out_mode(void *handle, int ctrl_id, int mode);

int32_t hal_pmu_set_powerctrl_out_ctrl(void *handle, int ctrl_id, int out);

int32_t hal_pmu_get_powerctrl_input_status(void *handle, int ctrl_id, int *status);

int32_t hal_pmu_set_internal_powerdown_enable(void *handle, int id, int enable);

int32_t hal_pmu_set_internal_powerdown_polarity(void *handle, int id, int polarity);

int32_t hal_pmu_get_internal_powerdown_status(void *handle, int id, int *status);

int32_t hal_pmu_set_external_reset_enable(void *handle, int id, int enable);

int32_t hal_pmu_set_external_reset_polarity(void *handle, int id, int polarity);

int32_t hal_pmu_set_external_reset_debounce_enable(void *handle, int id, int enable);

int32_t hal_pmu_set_external_reset_debounce_delay(void *handle, int id, int delay);

int32_t hal_pmu_get_external_reset_status(void *handle, int id, int *status);

int32_t hal_pmu_set_internal_wakeup_enable(void *handle, int id, int enable);

int32_t hal_pmu_set_external_wakeup_enable(void *handle, int id, int enable);

int32_t hal_pmu_set_external_wakeup_polarity(void *handle, int id, int polarity);

int32_t hal_pmu_set_external_wakeup_debounce_enable(void *handle, int id, int enable);

int32_t hal_pmu_set_external_wakeup_debounce_delay(void *handle, int id, int delay);

int32_t hal_pmu_get_external_wakeup_status(void *handle, int id, int *status);

int32_t hal_pmu_set_glitch_filter_delay(void *handle, int delay);

int32_t hal_pmu_set_glitch_filter_enable(void *handle, int id, int enable);

int32_t hal_pmu_set_por_powerdown_delay(void *handle, int delay);

int32_t hal_pmu_set_por_auto_powerdown(void *handle, int enable);

int32_t hal_pmu_set_por_force_powerdown(void *handle, int enable);

__END_CDECLS;
#endif
