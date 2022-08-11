/*
* Copyright (c) 2019 Semidrive Semiconductor Inc.
* All rights reserved.
*
* Description: domain communication configure for secure
*
*/
#include <reg.h>
#include <stdio.h>
#include <trace.h>
#include <platform/debug.h>
#include "dcf.h"

#if (CONFIG_IPCC_RPMSG == 1)
struct ipcc_dev_config ipcc_instances[] = {
    {.devname = "sec<->saf",  .rproc = IPCC_RRPOC_SAF,  .cfg = { .mbox_src = DCF_LOCAL_MBA_SEC_SAF, .mbox_dst = DCF_LOCAL_MBA_SEC_SAF} },
    {.devname = "sec<->ap1",  .rproc = IPCC_RRPOC_AP1,  .cfg = { .mbox_src = DCF_LOCAL_MBA_SEC_AP1, .mbox_dst = DCF_LOCAL_MBA_SEC_AP1} },
#if CONFIG_DCF_HAS_AP2
    {.devname = "sec<->ap2",  .rproc = IPCC_RRPOC_AP2,  .cfg = { .mbox_src = DCF_LOCAL_MBA_SEC_AP2, .mbox_dst = DCF_LOCAL_MBA_SEC_AP2} },
#endif
    { },
};
#endif

int dcf_get_this_proc(void)
{
    return DP_CR5_SEC;
}

struct sys_property_value dom_properties[] = {
    {.val = 0, .id = DMP_ID_DDR_STATUS,      .flags = SYS_PROP_F_RO, .owner = DP_CR5_SAF},
    {.val = 0, .id = DMP_ID_PORT_STATUS,     .flags = SYS_PROP_F_RO, .owner = DP_CR5_SAF},
    {.val = 0, .id = DMP_ID_PLL_CLK_STATUS,  .flags = SYS_PROP_F_RO, .owner = DP_CR5_SAF},
    {.val = 0, .id = DMP_ID_HANDOVER_STATUS, .flags = SYS_PROP_F_WR, .owner = DP_CR5_SAF},
    {.val = 0, .id = DMP_ID_DC_STATUS,       .flags = SYS_PROP_F_WR, .owner = DP_CR5_SAF},
    {.val = 0, .id = DMP_ID_VDSP_STATUS,     .flags = SYS_PROP_F_WR, .owner = DP_CR5_SAF},
    {.val = 0, .id = DMP_ID_VPU_STATUS,      .flags = SYS_PROP_F_WR, .owner = DP_CR5_SAF},
    {.val = 0, .id = DMP_ID_I2C_STATUS,      .flags = SYS_PROP_F_WR, .owner = DP_CR5_SAF},
    {.val = 0, .id = DMP_ID_REBOOT_STATUS,   .flags = SYS_PROP_F_WR, .owner = DP_CR5_SAF},
};
unsigned int dom_properties_num =  ARRAY_SIZE(dom_properties);

#if CONFIG_SUPPORT_POSIX
/* TODO: replace these static files once VFS is ready
 * static or builtin file description
 */
struct dcf_file dcf_files[DCF_MAX_FILES] = {
    [0] = {DEV_SSYSTEM,     "rpmsg-ssystem",  SSYSTEM_EPT,     DP_CR5_SAF},
};
#endif
