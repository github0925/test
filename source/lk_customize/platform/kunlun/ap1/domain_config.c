/*
* Copyright (c) 2019 Semidrive Semiconductor Inc.
* All rights reserved.
*
* Description: domain communication high level api for ap1
*
*/
#include <reg.h>
#include <stdio.h>
#include <trace.h>
#include <platform/debug.h>
#include "dcf.h"

#if (CONFIG_IPCC_RPMSG == 1)
struct ipcc_dev_config ipcc_instances[] = {
    {.devname = "ap<->saf", .rproc = IPCC_RRPOC_SAF,  .cfg = { .mbox_src = DCF_LOCAL_MBA_SAF_AP1, .mbox_dst = DCF_LOCAL_MBA_SAF_AP1} },
    {.devname = "ap<->sec", .rproc = IPCC_RRPOC_SEC,  .cfg = { .mbox_src = DCF_LOCAL_MBA_SEC_AP1, .mbox_dst = DCF_LOCAL_MBA_SEC_AP1} },
    {.devname = "ap<->lo",  .rproc = IPCC_RRPOC_AP1,  .cfg = { .mbox_src = IPCC_ADDR_ECHO_TEST,   .mbox_dst = IPCC_ADDR_ECHO_TEST} },
    { },
};
#endif

int dcf_get_this_proc(void)
{
    return DP_CA_AP1;
}

struct sys_property_value dom_properties[] = {
    {.val = 0, .id = DMP_ID_DDR_STATUS,      .flags = SYS_PROP_F_RO, .owner = DP_CR5_SAF},
    {.val = 0, .id = DMP_ID_PORT_STATUS,     .flags = SYS_PROP_F_RO, .owner = DP_CR5_SAF},
    {.val = 0, .id = DMP_ID_PLL_CLK_STATUS,  .flags = SYS_PROP_F_RO, .owner = DP_CR5_SAF},
    {.val = 0, .id = DMP_ID_HANDOVER_STATUS, .flags = SYS_PROP_F_RO, .owner = DP_CR5_SAF},
    {.val = 0, .id = DMP_ID_DC_STATUS,       .flags = SYS_PROP_F_RO, .owner = DP_CR5_SAF},
};
unsigned int dom_properties_num =  ARRAY_SIZE(dom_properties);

