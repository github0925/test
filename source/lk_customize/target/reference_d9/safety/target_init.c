/*
 * target_init.c
 *
 * Copyright (c) 2019 Semidrive Semiconductor.
 * All rights reserved.
 */
#include <debug.h>
#include <reg.h>
#include <target.h>
#include <dcf.h>

void sdm_display_init(void);

void target_early_init(void)
{

}

void target_init(void)
{
#if ENABLE_SD_DISP
#if CONFIG_USE_SYS_PROPERTY
    system_property_wait_condition(DMP_ID_PLL_CLK_STATUS, 1);
#endif

    sdm_display_init();

#if ENABLE_SERDES && CONFIG_USE_SYS_PROPERTY
    system_property_set(DMP_ID_I2C_STATUS, 3);
#endif
#endif
}

