#include <platform/interrupts.h>
#include <sys/types.h>
#include <platform/debug.h>
#include <reg.h>
#include <trace.h>
#include <string.h>
#include <assert.h>
#include <platform.h>
#include <lib/console.h>
#include "__regs_base.h"
#include "pmu_hal_ip_test.h"
#include "chip_res.h"

int pmu_set_iomux(int argc, const cmd_args *argv)
{
	uint32_t data;
	volatile uint8_t *pmu_iomux_phys = (volatile uint8_t *)APB_IOMUXC_RTC_BASE;
	volatile uint8_t *pmu_iomux_virt = (volatile uint8_t *)phys_to_virt(pmu_iomux_phys);

	if (argc < 2) {
		printf("eg: %s select\n", argv[0].str);
		printf("    select 0: wakeup and ctrl\n");
		printf("    select 1: pmu reset\n");
		return -1;
	}

	if (argv[1].u == 0)
		data = 1;
	else
		data = 4;
	writel(data, pmu_iomux_virt + 0x218);
	writel(data, pmu_iomux_virt + 0x228);
	return 0;
}

int pmu_status_test(int argc, const cmd_args *argv)
{
	void *handle;
	int ret = 0;
	int status = 0;
	uint32_t event = 0;

	ret = hal_pmu_creat_handle(&handle, RES_PMU_PMU);
	if (ret != 0) {
		printf("get handle fail\n");
		return ret;
	}

	ret = hal_pmu_init(handle);
	if (ret != 0) {
		printf("pmu init fail\n");
		return ret;
	}

	ret = hal_pmu_get_status(handle, &status);
	if (ret != 0) {
		printf("pmu get status fail\n");
		return ret;
	}
	printf("pmu status is %x\n", status);


	ret = hal_pmu_get_event_source(handle, &event);
	if (ret != 0) {
		printf("pmu exit fail\n");
		return ret;
	}
	printf("pmu event is %x\n", event);

	ret = hal_pmu_exit(handle);
	if (ret != 0) {
		printf("pmu exit fail\n");
		return ret;
	}

	ret = hal_pmu_release_handle(handle);
	if (ret != 0) {
		printf("release handle fail\n");
		return ret;
	}
	printf("test ok\n");
	return 0;
}

/* test for clean event */
int pmu_clean_event_test(int argc, const cmd_args *argv)
{
	void *handle;
	int ret = 0;
	int event_id = 0xff;
	uint32_t event = 0;

	if (argc < 2) {
		printf("eg: %s event_id\n", argv[0].str);
		return -1;
	}

	event_id = argv[1].u;
	printf("clean event_id %d status\n", event_id);

	ret = hal_pmu_creat_handle(&handle, RES_PMU_PMU);
	if (ret != 0) {
		printf("get handle fail\n");
		return ret;
	}

	ret = hal_pmu_init(handle);
	if (ret != 0) {
		printf("pmu init fail\n");
		return ret;
	}

	ret = hal_pmu_get_event_source(handle, &event);
	if (ret != 0) {
		printf("pmu exit fail\n");
		return ret;
	}
	printf("pmu event is %x before clean\n", event);

	if (event_id == 0xff)
		ret = hal_pmu_clean_all_event_source(handle);
	else
		ret = hal_pmu_clean_single_event_source(handle, event_id);
	if (ret != 0) {
		printf("pmu clean event status fail\n");
		return ret;
	}

	ret = hal_pmu_get_event_source(handle, &event);
	if (ret != 0) {
		printf("pmu exit fail\n");
		return ret;
	}
	printf("pmu event is %x after clean\n", event);

	ret = hal_pmu_exit(handle);
	if (ret != 0) {
		printf("pmu exit fail\n");
		return ret;
	}

	ret = hal_pmu_release_handle(handle);
	if (ret != 0) {
		printf("release handle fail\n");
		return ret;
	}
	printf("test ok\n");
	return 0;
}


/* test for powerctrl input mode*/
int pmu_powerctrl_input_test(int argc, const cmd_args *argv)
{
	void *handle;
	int ret = 0;
	int status =0;
	int id = 2;

	if (argc < 2) {
		printf("eg: %s id\n", argv[0].str);
		return -1;
	}

	id = argv[1].u;
	printf("get ctrl %d input status\n", id);

	ret = hal_pmu_creat_handle(&handle, RES_PMU_PMU);
	if (ret != 0) {
		printf("get handle fail\n");
		return ret;
	}

	ret = hal_pmu_init(handle);
	if (ret != 0) {
		printf("pmu init fail\n");
		return ret;
	}

	/* set ctrl_id as input*/
	ret = hal_pmu_set_powerctrl_io_mode(handle, id, 1);
	if (ret != 0) {
		printf("pmu set powerctrl io mode fail\n");
		return ret;
	}

	ret = hal_pmu_get_powerctrl_input_status(handle, id, &status);
	if (ret != 0) {
		printf("pmu set powerctrl input status fail\n");
		return ret;
	}
	printf("status = %0x\n", status);

	ret = hal_pmu_exit(handle);
	if (ret != 0) {
		printf("pmu exit fail\n");
		return ret;
	}

	ret = hal_pmu_release_handle(handle);
	if (ret != 0) {
		printf("release handle fail\n");
		return ret;
	}
	printf("test ok\n");
	return 0;
}

/**************************************************************
 *  test for powerctrl output mode
 *
 *  id            {0, 1, 2, 3}
 *  manual:       0  auto
 *                1  manual
 *  level         0  low
 *                1  high
 ************************************************************/
int pmu_powerctrl_output_test(int argc, const cmd_args *argv)
{
	void *handle;
	int ret = 0;
	int id = 2;
	int manual = 0;
	int level = 0xff;
	int pd_delay = 0xff;
	int pu_delay = 0xff;

	if (argc < 3) {
		printf("eg: %s id manual [level] [powerdown delay] [powerup delay]\n", argv[0].str);
		return -1;
	}

	id = argv[1].u;
	manual = argv[2].u;
	if (argc > 3)
		level = argv[3].u;
	if (argc > 4)
		pu_delay = argv[4].u;
	if (argc > 5)
		pd_delay = argv[5].u;

	printf("test ctrl %d output, manual %d out_level %d pu_delay %d pd_delay %d\n",
	       id, manual, level, pu_delay, pd_delay);

	ret = hal_pmu_creat_handle(&handle, RES_PMU_PMU);
	if (ret != 0) {
		printf("get handle fail\n");
		return ret;
	}

	ret = hal_pmu_init(handle);
	if (ret != 0) {
		printf("pmu init fail\n");
		return ret;
	}

	/* set ctrl_id as output mode*/
	ret = hal_pmu_set_powerctrl_io_mode(handle, id, 0);
	if (ret != 0) {
		printf("pmu set powerctrl io mode fail\n");
		return ret;
	}

	/* set output as auto or manual*/
	ret = hal_pmu_set_powerctrl_out_mode(handle, id, manual);
	if (ret != 0) {
		printf("pmu set powerctrl out mode fail\n");
		return ret;
	}

	/* set powerctrl powerup delay*/
	if (pu_delay !=0xff) {
		ret = hal_pmu_set_powerctrl_powerup_delay(handle, id, pu_delay);
		if (ret != 0) {
			printf("pmu set powerctrl powerup delay fail\n");
			return ret;
		}
	}

	/* set powerctrl powerdown delay*/
	if (pd_delay != 0xff) {
		ret = hal_pmu_set_powerctrl_powerdown_delay(handle, id, pd_delay);
		if (ret != 0) {
			printf("pmu set powerctrl powerdown delay fail\n");
			return ret;
		}
	}

	/* set powerctrl out high or low*/
	if (level != 0xff) {
		ret = hal_pmu_set_powerctrl_out_ctrl(handle, id, level);
		if (ret != 0) {
			printf("pmu set powerctrl out level fail\n");
			return ret;
		}
	}

	ret = hal_pmu_exit(handle);
	if (ret != 0) {
		printf("pmu exit fail\n");
		return ret;
	}

	ret = hal_pmu_release_handle(handle);
	if (ret != 0) {
		printf("release handle fail\n");
		return ret;
	}
	printf("test ok\n");
	return 0;
}

/* test for pmu powerdown*/
int pmu_powerdown_test(int argc, const cmd_args *argv)
{
	void *handle;
	int ret = 0;
	int pu_delay = 0xff;
	int pd_delay = 0xff;

	if (argc < 3) {
		printf("eg: %s powerup_delay powerdown_delay\n", argv[0].str);
		return -1;
	}

	pu_delay = argv[1].u;
	pd_delay = argv[2].u;
	printf("set pu_delay %x pd_delay %x\n", pu_delay, pd_delay);

	ret = hal_pmu_creat_handle(&handle, RES_PMU_PMU);
	if (ret != 0) {
		printf("get handle fail\n");
		return ret;
	}

	ret = hal_pmu_init(handle);
	if (ret != 0) {
		printf("pmu init fail\n");
		return ret;
	}

	if (pu_delay != 0xff) {
		ret = hal_pmu_set_powerup_delay(handle, pu_delay);
		if (ret != 0) {
			printf("pmu set pu delay fail\n");
			return ret;
		}
	}

	if (pd_delay != 0xff) {
		ret = hal_pmu_set_powerdown_delay(handle, pd_delay);
		if (ret != 0) {
			printf("pmu set pd delay fail\n");
			return ret;
		}
	}

	ret = hal_pmu_powerdown(handle);
	if (ret != 0) {
		printf("pmu set pd fail\n");
		return ret;
	}

	ret = hal_pmu_exit(handle);
	if (ret != 0) {
		printf("pmu exit fail\n");
		return ret;
	}

	ret = hal_pmu_release_handle(handle);
	if (ret != 0) {
		printf("release handle fail\n");
		return ret;
	}
	printf("test ok\n");
	return 0;
}

/* test for pmu internal powerdown*/
int pmu_internal_powerdown_test(int argc, const cmd_args *argv)
{
	void *handle;
	int ret = 0;
	int id  = 0;
	int enable  = 0;
	int polarity = 0;
	int status = 0;

	if (argc < 4) {
		printf("eg: %s id polarity enable\n", argv[0].str);
		return -1;
	}

	id = argv[1].u;
	polarity = argv[2].u;
	enable = argv[3].u;
	printf("set id %x polarity %x, enable %x\n", id, polarity, enable);

	ret = hal_pmu_creat_handle(&handle, RES_PMU_PMU);
	if (ret != 0) {
		printf("get handle fail\n");
		return ret;
	}

	ret = hal_pmu_init(handle);
	if (ret != 0) {
		printf("pmu init fail\n");
		return ret;
	}

	ret = hal_pmu_set_internal_powerdown_polarity(handle, id, polarity);
	if (ret != 0) {
		printf("pmu set pd fail\n");
		return ret;
	}

	ret = hal_pmu_get_internal_powerdown_status(handle, id, &status);
	if (ret != 0) {
		printf("pmu set pd fail\n");
		return ret;
	}
	printf("get internal powerdown id %d status %0x\n", id, status);

	ret = hal_pmu_set_internal_powerdown_enable(handle, id, enable);
	if (ret != 0) {
		printf("pmu set pd fail\n");
		return ret;
	}

	ret = hal_pmu_exit(handle);
	if (ret != 0) {
		printf("pmu exit fail\n");
		return ret;
	}

	ret = hal_pmu_release_handle(handle);
	if (ret != 0) {
		printf("release handle fail\n");
		return ret;
	}
	printf("test ok\n");
	return 0;
}

/* test for get internal powerdown status*/
int pmu_internal_powerdown_status_test(int argc, const cmd_args *argv)
{
	void *handle;
	int ret = 0;
	int status = 0;
	int id = 0;

	if (argc < 2) {
		printf("eg: %s id\n", argv[0].str);
		return -1;
	}

	id = argv[1].u;
	ret = hal_pmu_creat_handle(&handle, RES_PMU_PMU);
	if (ret != 0) {
		printf("get handle fail\n");
		return ret;
	}

	ret = hal_pmu_init(handle);
	if (ret != 0) {
		printf("pmu init fail\n");
		return ret;
	}

	ret = hal_pmu_get_internal_powerdown_status(handle, id, &status);
	if (ret != 0) {
		printf("pmu get status fail\n");
		return ret;
	}
	printf("pmu internal powerdown %d status is %x\n", id, status);

	ret = hal_pmu_exit(handle);
	if (ret != 0) {
		printf("pmu exit fail\n");
		return ret;
	}

	ret = hal_pmu_release_handle(handle);
	if (ret != 0) {
		printf("release handle fail\n");
		return ret;
	}
	printf("test ok\n");
	return 0;
}

/* test for pmu external reset */
int pmu_external_reset_test(int argc, const cmd_args *argv)
{
	void *handle;
	int ret = 0;
	int id = 0xff;
	int enable = 0xff;
	int polarity = 0xff;
	int debounce_enable = 0xff;
	int delay = 0xff;

	if (argc < 4) {
		printf("eg: %s id polarity enable [debounce_enable] [debounce_delay]\n", argv[0].str);
		return -1;
	}

	id = argv[1].u;
	polarity = argv[2].u;
	enable = argv[3].u;

	if (argc > 4)
		debounce_enable = argv[4].u;
	if (argc > 5)
		delay = argv[5].u;
	printf("reset test id %d, enable %d, polarity %d, enable %x delay %x\n",
	       id, enable, polarity, enable, delay);

	ret = hal_pmu_creat_handle(&handle, RES_PMU_PMU);
	if (ret != 0) {
		printf("get handle fail\n");
		return ret;
	}

	ret = hal_pmu_init(handle);
	if (ret != 0) {
		printf("pmu init fail\n");
		return ret;
	}

	if (polarity != 0xff) {
		ret = hal_pmu_set_external_reset_polarity(handle, id, polarity);
		if (ret != 0) {
			printf("pmu set reset polarity fail\n");
			return ret;
		}
	}

	if (delay != 0xff) {
		ret = hal_pmu_set_external_reset_debounce_delay(handle, id, delay);
		if (ret != 0) {
			printf("pmu set reset debounce delay fail\n");
			return ret;
		}
	}

	if (debounce_enable != 0xff) {
		ret = hal_pmu_set_external_reset_debounce_enable(handle, id, debounce_enable);
		if (ret != 0) {
			printf("pmu set reset debounce enable fail\n");
			return ret;
		}
	}

	ret = hal_pmu_set_external_reset_enable(handle, id, enable);
	if (ret != 0) {
		printf("pmu set pd fail\n");
		return ret;
	}

	ret = hal_pmu_exit(handle);
	if (ret != 0) {
		printf("pmu exit fail\n");
		return ret;
	}

	ret = hal_pmu_release_handle(handle);
	if (ret != 0) {
		printf("release handle fail\n");
		return ret;
	}
	printf("test ok\n");
	return 0;
}

/* test for get internal powerdown status*/
int pmu_external_reset_status_test(int argc, const cmd_args *argv)
{
	void *handle;
	int ret = 0;
	int status = 0;
	int id = 0;

	if (argc < 2) {
		printf("eg: %s id\n", argv[0].str);
		return -1;
	}
	id = argv[1].u;

	ret = hal_pmu_creat_handle(&handle, RES_PMU_PMU);
	if (ret != 0) {
		printf("get handle fail\n");
		return ret;
	}

	ret = hal_pmu_init(handle);
	if (ret != 0) {
		printf("pmu init fail\n");
		return ret;
	}

	ret = hal_pmu_get_external_reset_status(handle, id, &status);
	if (ret != 0) {
		printf("pmu get status fail\n");
		return ret;
	}
	printf("pmu reset %d status is %x\n", id, status);

	ret = hal_pmu_exit(handle);
	if (ret != 0) {
		printf("pmu exit fail\n");
		return ret;
	}

	ret = hal_pmu_release_handle(handle);
	if (ret != 0) {
		printf("release handle fail\n");
		return ret;
	}
	printf("test ok\n");
	return 0;
}

/* test for pmu internal wakeup*/
int pmu_internal_wakeup_test(int argc, const cmd_args *argv)
{
	void *handle;
	int ret = 0;
	int id = 0xff;
	int enable = 0xff;

	if (argc < 3) {
		printf("eg: %s id enable\n", argv[0].str);
		return -1;
	}

	id = argv[1].u;
	enable = argv[2].u;
	printf("set internal wakeup id %x enable %x\n", id, enable);

	ret = hal_pmu_creat_handle(&handle, RES_PMU_PMU);
	if (ret != 0) {
		printf("get handle fail\n");
		return ret;
	}

	ret = hal_pmu_init(handle);
	if (ret != 0) {
		printf("pmu init fail\n");
		return ret;
	}

	ret = hal_pmu_set_internal_wakeup_enable(handle, id, enable);
	if (ret != 0) {
		printf("pmu set internal wakeup enable fail\n");
		return ret;
	}

	ret = hal_pmu_exit(handle);
	if (ret != 0) {
		printf("pmu exit fail\n");
		return ret;
	}

	ret = hal_pmu_release_handle(handle);
	if (ret != 0) {
		printf("release handle fail\n");
		return ret;
	}
	printf("test ok\n");
	return 0;
}

/* test for pmu external wakeup */
int pmu_external_wakeup_test(int argc, const cmd_args *argv)
{
	void *handle;
	int ret = 0;
	int id = 0xff;
	int enable = 0xff;
	int polarity = 0xff;
	int debounce_enable = 0xff;
	int delay = 0xff;

	if (argc < 4) {
		printf("eg: %s id polarity enable [debounce_enable] [debounce_delay]\n", argv[0].str);
		return -1;
	}
	id = argv[1].u;
	polarity = argv[2].u;
	enable = argv[3].u;

	if (argc > 4)
		debounce_enable = argv[4].u;
	if (argc > 5)
		delay = argv[5].u;
	printf("reset test id %d, enable %d, polarity %d, enable %x delay %x\n",
	       id, enable, polarity, debounce_enable, delay);

	ret = hal_pmu_creat_handle(&handle, RES_PMU_PMU);
	if (ret != 0) {
		printf("get handle fail\n");
		return ret;
	}

	ret = hal_pmu_init(handle);
	if (ret != 0) {
		printf("pmu init fail\n");
		return ret;
	}

	ret = hal_pmu_set_external_wakeup_polarity(handle, id, polarity);
	if (ret != 0) {
		printf("pmu set wakeup polarity fail\n");
		return ret;
	}

	if (delay != 0xff) {
		ret = hal_pmu_set_external_wakeup_debounce_delay(handle, id, delay);
		if (ret != 0) {
			printf("pmu set wakeup debounce delay fail\n");
			return ret;
		}
	}

	if (debounce_enable != 0xff) {
		ret = hal_pmu_set_external_wakeup_debounce_enable(handle, id, debounce_enable);
		if (ret != 0) {
			printf("pmu set wakeup debounce enable fail\n");
			return ret;
		}
	}

	ret = hal_pmu_set_external_wakeup_enable(handle, id, enable);
	if (ret != 0) {
		printf("pmu set external wakeup fail\n");
		return ret;
	}

	ret = hal_pmu_exit(handle);
	if (ret != 0) {
		printf("pmu exit fail\n");
		return ret;
	}

	ret = hal_pmu_release_handle(handle);
	if (ret != 0) {
		printf("release handle fail\n");
		return ret;
	}
	printf("test ok\n");
	return 0;
}

/* test for set glitch filter*/
int pmu_glitch_filter_test(int argc, const cmd_args *argv)
{
	void *handle;
	int ret = 0;
	int id = 0;
	int delay = 0xff;
	int enable = 0;

	if (argc < 3) {
		printf("eg: %s id enable [delay]\n", argv[0].str);
		return -1;
	}

	id = argv[1].u;
	enable = argv[2].u;

	if (argc > 3)
		delay = argv[3].u;

	printf("glitch test id %d, enable %d, delay %x\n", id, enable, delay);
	ret = hal_pmu_creat_handle(&handle, RES_PMU_PMU);
	if (ret != 0) {
		printf("get handle fail\n");
		return ret;
	}

	ret = hal_pmu_init(handle);
	if (ret != 0) {
		printf("pmu init fail\n");
		return ret;
	}

	if (delay != 0xff) {
		ret = hal_pmu_set_glitch_filter_delay(handle, delay);
		if (ret != 0) {
			printf("pmu set glitch filter fail\n");
			return ret;
		}
	}

	ret = hal_pmu_set_glitch_filter_enable(handle, id, enable);
	if (ret != 0) {
		printf("pmu set glitch enable fail\n");
		return ret;
	}

	ret = hal_pmu_exit(handle);
	if (ret != 0) {
		printf("pmu exit fail\n");
		return ret;
	}

	ret = hal_pmu_release_handle(handle);
	if (ret != 0) {
		printf("release handle fail\n");
		return ret;
	}
	printf("test ok\n");
	return 0;
}

/* test for set por powerdown */
int pmu_por_powerdown_test(int argc, const cmd_args *argv)
{
	void *handle;
	int ret = 0;
	int manual;
	int delay = 0xff;
	int enable;

	if (argc < 3) {
		printf("eg: %s enable manual [delay]\n", argv[0].str);
		return -1;
	}

	enable = argv[1].u;
	manual = argv[2].u;

	if (argc > 3)
		delay = argv[3].u;
	printf("glitch test enable %d, manual %d, delay %x\n", enable, manual, delay);

	ret = hal_pmu_creat_handle(&handle, RES_PMU_PMU);
	if (ret != 0) {
		printf("get handle fail\n");
		return ret;
	}

	ret = hal_pmu_init(handle);
	if (ret != 0) {
		printf("pmu init fail\n");
		return ret;
	}

	if (delay != 0xff) {
		ret = hal_pmu_set_por_powerdown_delay(handle, delay);
		if (ret != 0) {
			printf("pmu set glitch filter fail\n");
			return ret;
		}
	}

	if (manual == 1)
		ret = hal_pmu_set_por_force_powerdown(handle, enable);
	else
		ret = hal_pmu_set_por_auto_powerdown(handle, enable);
	if (ret != 0) {
		printf("pmu set por powerdown enable fail\n");
		return ret;
	}

	ret = hal_pmu_exit(handle);
	if (ret != 0) {
		printf("pmu exit fail\n");
		return ret;
	}

	ret = hal_pmu_release_handle(handle);
	if (ret != 0) {
		printf("release handle fail\n");
		return ret;
	}
	printf("test ok\n");
	return 0;
}


