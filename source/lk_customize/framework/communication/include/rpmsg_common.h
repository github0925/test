/*
 * Copyright (c) 2019 Semidrive Semiconductor, Inc.
 * All rights reserved.
 */
#ifndef __RPMSG_COMMOM_H__
#define __RPMSG_COMMOM_H__

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <err.h>

/* must include this for thread priority
 * set thread proirity is system policy
 */
#include <sys_priority.h>

#include "ipcc_config.h"
#include "sd_ipcc.h"

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(arr)    (sizeof(arr) / sizeof((arr)[0]))
#endif

/* reserved mbox client address for rpmsg lite */
#define DCF_RPMSG_MBA_SEC_SAF   (IPCC_ADDR_RPMSG + 0)
#define DCF_RPMSG_MBA_SEC_MPC   (IPCC_ADDR_RPMSG + 2)
#define DCF_RPMSG_MBA_SEC_AP1   (IPCC_ADDR_RPMSG + 4)
#define DCF_RPMSG_MBA_SEC_AP2   (IPCC_ADDR_RPMSG + 7)
#define DCF_RPMSG_MBA_SAF_MPC   (IPCC_ADDR_RPMSG + 1)
#define DCF_RPMSG_MBA_SAF_AP1   (IPCC_ADDR_RPMSG + 3)
#define DCF_RPMSG_MBA_SAF_AP2   (IPCC_ADDR_RPMSG + 6)
#define DCF_RPMSG_MBA_MPC_AP1   (IPCC_ADDR_RPMSG + 5)
#define DCF_RPMSG_MBA_MPC_AP2   (IPCC_ADDR_RPMSG + 8)
#define DCF_RPMSG_MBA_AP1_AP2   (IPCC_ADDR_RPMSG + 9)

/* reserved mbox client address for rpmsg mb */
#define DCF_LOCAL_MBA_SEC_SAF   (IPCC_ADDR_DCF_BASE + 0)
#define DCF_LOCAL_MBA_SEC_MPC   (IPCC_ADDR_DCF_BASE + 2)
#define DCF_LOCAL_MBA_SEC_AP1   (IPCC_ADDR_DCF_BASE + 4)
#define DCF_LOCAL_MBA_SEC_AP2   (IPCC_ADDR_DCF_BASE + 7)
#define DCF_LOCAL_MBA_SAF_MPC   (IPCC_ADDR_DCF_BASE + 1)
#define DCF_LOCAL_MBA_SAF_AP1   (IPCC_ADDR_DCF_BASE + 3)
#define DCF_LOCAL_MBA_SAF_AP2   (IPCC_ADDR_DCF_BASE + 6)
#define DCF_LOCAL_MBA_MPC_AP1   (IPCC_ADDR_DCF_BASE + 5)
#define DCF_LOCAL_MBA_MPC_AP2   (IPCC_ADDR_DCF_BASE + 8)
#define DCF_LOCAL_MBA_AP1_AP2   (IPCC_ADDR_DCF_BASE + 9)

/* dcf service endpoint */
#define SVCMGR_IVI_EPT          (10)
#define SVCMGR_CLU_EPT          (11)
#define LOOPBACK_EPT            (12)
#define PROPERTY_EPT            (13)
#define IPCC_RPC_CLIENT_EPT     (14)
#define IPCC_RPC_SAF_MAIN_EPT   (15)
#define IPCC_RPC_MAIN_EPT       (16)
#define IPCC_SYS_EPT            (20)
#define IPCC_ECHO_EPT           (30)
#define IPCC_TTY_EPT            (40)
#define TTY_LINUX_EPT           (40)
#define SRV_SAMPLE_EPT          (48)
#define ERPC_LINUX_EPT          (50)
#define NET_LINUX_EPT           (60)
#define CLUSTER_EPT             (70)
#define IVI_EPT                 (71)
#define SSYSTEM_EPT             (72)
#define SA_VI2C_EPT             (73)
#define EARLYAPP_EPT            (80)
#define VIRCAN_IVI_EPT          (90)
#define VIRCAN_CLU_EPT          (91)
#define SLT_TEST_EPT_SAF        (92)
#define SLT_TEST_EPT_SEC        (93)
#define SLT_TEST_EPT_MP        (94)
#define SLT_TEST_EPT_AP1        (95)
#define SLT_TEST_EPT_AP2        (96)
#define SDSHELL_SERVICE_EPT	(97)

typedef enum {
    DCF_STATE_Unknown,
    DCF_STATE_Initializing,
    DCF_STATE_Initialized,
    DCF_STATE_Closing,
    DCF_STATE_Closed,
    DCF_STATE_Connected,
} dcf_state_t;

#include <dcf_msg.h>

#endif //__RPMSG_COMMOM_H__
