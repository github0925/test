/*
 * Copyright (c) 2019 Semidrive Semiconductor Inc.
 * All rights reserved.
 *
 * Description: domain communication domain-independent define
 *
 */

#ifndef __DCF_H__
#define __DCF_H__

#include <reg.h>
#include <stdio.h>
#include <trace.h>
#include <dcf_common.h>

/*
 * CONFIG_RPMSG_SERVICE==1 means rpmsg rtos is enabled,
 * set SUPPORT_3RD_RPMSG_LITE = true to enable it in project.
 */
#if defined(CONFIG_RPMSG_SERVICE) && (CONFIG_RPMSG_SERVICE == 1)
#include <rpmsg_rtos.h>

/* add shared memory region for IPC */
int init_shm_domain_area(int region_base);
struct rpmsg_dev_config *init_rpmsg_dev_config(void);

#else

/* add shared memory region for IPC */
inline static int init_shm_domain_area(int region_base)
{
    return region_base;
}

inline static struct rpmsg_dev_config *init_rpmsg_dev_config(void)
{
    return NULL;
}

#endif

#endif //__DCF_H__
