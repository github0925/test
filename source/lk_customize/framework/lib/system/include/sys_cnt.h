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

#ifndef __SYS_COUNTER_H_
#define __SYS_COUNTER_H_

#include <stdint.h>

/* project config can overwrite this */
#ifndef SYSCNT_CNT_PER_SECOND
#define SYSCNT_CNT_PER_SECOND   (3000000)
#endif

#define SYSCNT_CNT_PER_US       (SYSCNT_CNT_PER_SECOND/1000000)

uint32_t syscnt_get_cnt(void);
uint32_t syscnt_get_freq(void);
uint32_t syscnt_time_lapse(uint32_t early, uint32_t late);

#endif //__SYS_COUNTER_H_
