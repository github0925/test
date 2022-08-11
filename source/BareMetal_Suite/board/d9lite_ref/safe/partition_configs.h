/*
 * partition_configs.h
 *
 * Copyright (c) 2020 SemiDrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */

#ifndef _PARTITION_CONFIGS_H_
#define _PARTITION_CONFIGS_H_

#define PT_CONFIGS                             \
        PT_LOAD_CONFIG_ITEM(0, 0, 0, ssystem)        \
        PT_LOAD_CONFIG_ITEM(0, 0, 0, hsm_fw)         \
        PT_LOAD_CONFIG_ITEM(0, 0, 0, preloader)      \
        PT_LOAD_CONFIG_ITEM(SYS_CFG_MEMBASE, SYS_CFG_MEMSIZE, PT_LD_DECD, system_config)  \
        PT_LOAD_CONFIG_ITEM(SAFETY_MEMBASE, SAFETY_MEMSIZE, PT_LD_DECD, safety_os)         \
        PT_LOAD_CONFIG_ITEM(DIL2_MEMBASE, DIL2_MEMSIZE, PT_LD_DECD, dil2)         \
        PT_LOAD_CONFIG_ITEM(VBMETA_MEMBASE, VBMETA_MEMSIZE, PT_LD_DECD, vbmeta)         \

#endif
