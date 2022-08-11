/*
* Copyright (c) 2019 Semidrive Semiconductor Inc.
* All rights reserved.
*
* Description: domain communication high level api for safety
*
*/
#include <reg.h>
#include <stdio.h>
#include <trace.h>
#include <platform/debug.h>
#include <property.h>
#include "dcf.h"

/* DCF remote processor capabilities */
#ifndef CONFIG_DCF_HAS_LO
#define CONFIG_DCF_HAS_LO    (0)
#endif

#ifndef CONFIG_DCF_HAS_SEC
#define CONFIG_DCF_HAS_SEC   (1)
#endif

#ifndef CONFIG_DCF_HAS_AP1
#define CONFIG_DCF_HAS_AP1   (1)
#endif

#ifndef CONFIG_DCF_HAS_MP
#define CONFIG_DCF_HAS_MP    (0)
#endif

#ifndef CONFIG_DCF_HAS_AP2
#define CONFIG_DCF_HAS_AP2   (0)
#endif

#if (CONFIG_IPCC_RPMSG == 1)
struct ipcc_dev_config ipcc_instances[] = {
#if CONFIG_DCF_HAS_LO
    {.devname = "saf<->lo",  .rproc = DP_CR5_SAF,  .cfg = { .mbox_src = IPCC_ADDR_ECHO_TEST,   .mbox_dst = IPCC_ADDR_ECHO_TEST} },
#endif
#if CONFIG_DCF_HAS_SEC
    {.devname = "saf<->sec", .rproc = DP_CR5_SEC,  .cfg = { .mbox_src = DCF_LOCAL_MBA_SEC_SAF, .mbox_dst = DCF_LOCAL_MBA_SEC_SAF} },
#endif
#if CONFIG_DCF_HAS_AP1
    {.devname = "saf<->ap1", .rproc = DP_CA_AP1,   .cfg = { .mbox_src = DCF_LOCAL_MBA_SAF_AP1, .mbox_dst = DCF_LOCAL_MBA_SAF_AP1} },
#endif
#if CONFIG_DCF_HAS_MP
    {.devname = "saf<->mp",  .rproc = DP_CR5_MPC,  .cfg = { .mbox_src = DCF_LOCAL_MBA_SAF_MPC, .mbox_dst = DCF_LOCAL_MBA_SAF_MPC} },
#endif
#if CONFIG_DCF_HAS_AP2
    {.devname = "saf<->ap2", .rproc = DP_CA_AP2,   .cfg = { .mbox_src = DCF_LOCAL_MBA_SAF_AP2, .mbox_dst = DCF_LOCAL_MBA_SAF_AP2} },
#endif
    {.devname = "", .rproc = -1, .cfg = { .mbox_src = -1, .mbox_dst = -1}},
};
#endif

int dcf_get_this_proc(void)
{
    return DP_CR5_SAF;
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
    {.val = 0, .id = DMP_ID_AVM_STATUS,   .flags = SYS_PROP_F_WR, .owner = DP_CR5_SAF},
    {.val = 0, .id = DMP_ID_BA_STATUS,   .flags = SYS_PROP_F_WR, .owner = DP_CR5_SAF},
    {.val = 0, .id = DMP_ID_DMS_STATUS,   .flags = SYS_PROP_F_WR, .owner = DP_CR5_SAF},
    {.val = 0, .id = DMP_ID_CLUSTER_STATUS,   .flags = SYS_PROP_F_WR, .owner = DP_CR5_SAF},
    {.val = 0, .id = DMP_ID_CP_STATUS,   .flags = SYS_PROP_F_WR, .owner = DP_CR5_SAF},
};
unsigned int dom_properties_num =  ARRAY_SIZE(dom_properties);

#if CONFIG_SUPPORT_POSIX

/* TODO: replace these static files once VFS is ready
 * static or builtin file description
 */
struct dcf_file dcf_files[DCF_MAX_FILES] = {
    [0] = {DEV_LOOPBACK,    "rpmsg-loopback", LOOPBACK_EPT,    DP_CA_AP1},
    [1] = {DEV_SM_I,        "rpmsg-sdsm",     SVCMGR_IVI_EPT,  DP_CA_AP1},
    [2] = {DEV_PROPERTY,    "rpmsg-property", PROPERTY_EPT,    DP_CA_AP1},
    [3] = {DEV_VIRCAN_I,    "rpmsg-vircan",   VIRCAN_IVI_EPT,  DP_CA_AP1},
    [4] = {DEV_VIRCAN_C,    "rpmsg-vircan",   VIRCAN_CLU_EPT,  DP_CA_AP2},
    [5] = {DEV_DISP_C ,     "rpmsg-cluster",  CLUSTER_EPT,     DP_CA_AP1},
    [6] = {DEV_DISP_I,      "rpmsg-ivi",      IVI_EPT,         DP_CA_AP2},
    [7] = {DEV_SA_VI2C,     "safety,vi2c",    SA_VI2C_EPT,     DP_CA_AP1},
    [8] = {DEV_SSYSTEM,     "rpmsg-ssystem",  SSYSTEM_EPT,     DP_CR5_SEC},
};
#endif
