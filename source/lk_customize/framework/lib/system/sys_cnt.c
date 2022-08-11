/*
 * SEMIDRIVE Copyright Statement
 * Copyright (c) SEMIDRIVE. All rights reserved
 *
 * This software and all rights therein are owned by SEMIDRIVE, and are
 * protected by copyright law and other relevant laws, regulations and
 * protection. Without SEMIDRIVE's prior written consent and/or related rights,
 * please do not use this software or any potion thereof in any form or by any
 * means. You may not reproduce, modify or distribute this software except in
 * compliance with the License. Unless required by applicable law or agreed to
 * in writing, software distributed under the License is distributed on
 * an "AS IS" basis, WITHOUT WARRANTIES OF ANY KIND, either express or implied.
 *
 * You should have received a copy of the License along with this program.
 * If not, see <http://www.semidrive.com/licenses/>.
 */

#include <debug.h>
#include <string.h>
#include <__regs_base.h>
#include <reg.h>
#include "sys_cnt.h"

uint32_t syscnt_get_cnt(void)
{
    return readl(APB_SYS_CNT_RO_BASE);
}

uint32_t syscnt_get_freq(void)
{
    return SYSCNT_CNT_PER_SECOND;
}

uint32_t syscnt_time_lapse(uint32_t early, uint32_t late)
{
    /* not support uint32 overflow occur every about 23 min in 3MHz */
    return (late - early) / SYSCNT_CNT_PER_US;
}
