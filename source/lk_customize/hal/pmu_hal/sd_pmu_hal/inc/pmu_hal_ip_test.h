#ifndef __PMU_HAL_IP_TEST_H
#define __PMU_HAL_IP_TEST_H
#include <sys/types.h>
#include <compiler.h>
#include "pmu_hal.h"

__BEGIN_CDECLS;

/* test for get pmu status*/
int pmu_status_test(int argc, const cmd_args *argv);

/* test for powerctrl input mode*/
int pmu_powerctrl_input_test(int argc, const cmd_args *argv);

/* test for powerctrl output mode*/
int pmu_powerctrl_output_test(int argc, const cmd_args *argv);

/* test for powerdown*/
int pmu_powerdown_test(int argc, const cmd_args *argv);

/* test for clean event */
int pmu_clean_event_test(int argc, const cmd_args *argv);

/* test for pmu internal powerdown*/
int pmu_internal_powerdown_test(int argc, const cmd_args *argv);

/* test for get internal powerdown status*/
int pmu_internal_powerdown_status_test(int argc, const cmd_args *argv);

/* test for pmu external reset */
int pmu_external_reset_test(int argc, const cmd_args *argv);

/* test for get internal powerdown status*/
int pmu_external_reset_status_test(int argc, const cmd_args *argv);

/* test for pmu internal wakeup*/
int pmu_internal_wakeup_test(int argc, const cmd_args *argv);

/* test for pmu external wakeup */
int pmu_external_wakeup_test(int argc, const cmd_args *argv);

/* test for pmu set iomux */
int pmu_set_iomux(int argc, const cmd_args *argv);

/* test for set glitch filter*/
int pmu_glitch_filter_test(int argc, const cmd_args *argv);

/* test for set por powerdown */
int pmu_por_powerdown_test(int argc, const cmd_args *argv);

__END_CDECLS;
#endif
