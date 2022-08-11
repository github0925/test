/*
 * Copyright (c) 2016, Xilinx Inc. and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * @file	lk/time.c
 * @brief	lk libmetal time handling.
 */

#include <platform.h>
#include <metal/time.h>

unsigned long long metal_get_timestamp(void)
{
	return (unsigned long long)current_time_hires();
}

