#include <app.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <debug.h>
#include <platform.h>
#include <lib/console.h>
#include "pmu_hal_ip_test.h"

char pmu_test1_help[]= {"test for get pmu status"};
char pmu_test2_help[]= {"test pmu powerctrl input mode"};
char pmu_test3_help[]= {"test pmu powerctrl output mode"};
char pmu_test4_help[]= {"test for pmu powerdown"};
char pmu_test5_help[]= {"test for pmu clean event"};
char pmu_test6_help[]= {"test for pmu internal powerdown"};
char pmu_test7_help[]= {"test for get pmu internal powerdown status"};
char pmu_test8_help[]= {"test for pmu external reset"};
char pmu_test9_help[]= {"test for get pmu external reset status"};
char pmu_test10_help[]= {"test for pmu internal wakeup"};
char pmu_test11_help[]= {"test for pmu external wakeup"};
char pmu_test12_help[]= {"test for pmu set iomux"};
char pmu_test13_help[]= {"test for pmu set glitch filter"};
char pmu_test14_help[]= {"test for pmu set por powerdown"};

STATIC_COMMAND_START
STATIC_COMMAND("pmu_test1", pmu_test1_help, (console_cmd)&pmu_status_test)
STATIC_COMMAND("pmu_test2", pmu_test2_help, (console_cmd)&pmu_powerctrl_input_test)
STATIC_COMMAND("pmu_test3", pmu_test3_help, (console_cmd)&pmu_powerctrl_output_test)
STATIC_COMMAND("pmu_test4", pmu_test4_help, (console_cmd)&pmu_powerdown_test)
STATIC_COMMAND("pmu_test5", pmu_test5_help, (console_cmd)&pmu_clean_event_test)
STATIC_COMMAND("pmu_test6", pmu_test6_help, (console_cmd)&pmu_internal_powerdown_test)
STATIC_COMMAND("pmu_test7", pmu_test7_help, (console_cmd)&pmu_internal_powerdown_status_test)
STATIC_COMMAND("pmu_test8", pmu_test8_help, (console_cmd)&pmu_external_reset_test)
STATIC_COMMAND("pmu_test9", pmu_test9_help, (console_cmd)&pmu_external_reset_status_test)
STATIC_COMMAND("pmu_test10", pmu_test10_help, (console_cmd)&pmu_internal_wakeup_test)
STATIC_COMMAND("pmu_test11", pmu_test11_help, (console_cmd)&pmu_external_wakeup_test)
STATIC_COMMAND("pmu_test12", pmu_test12_help, (console_cmd)&pmu_set_iomux)
STATIC_COMMAND("pmu_test13", pmu_test13_help, (console_cmd)&pmu_glitch_filter_test)
STATIC_COMMAND("pmu_test14", pmu_test14_help, (console_cmd)&pmu_por_powerdown_test)
STATIC_COMMAND_END(pmutest);

