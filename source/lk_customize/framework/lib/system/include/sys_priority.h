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
 *
 * Description: global thread priority deployment
 */

#ifndef __SYS_PRIORITY_H__
#define __SYS_PRIORITY_H__

#include <kernel/thread.h>

/*
 * Thread Priorty list for echo infrastructure and service
 * Priority from 0 lowest to 31 highest
 * 0 is reserved for idle thread and 16 is default, 31 for timer
 * Priority deploy strategy (1 ~ 30)
 *   1 ~ 16 is for normal thread, equal priority is time-slicing scheduled
 *   17 ~ 30 is for latency-sensitive thread e.g.:
 *   input, multimedia, network, irq DSR, do not use equal priority
 */

/* Communication thread */
#define THREAD_PRI_RPMSGECHO    (10)
#define THREAD_PRI_RPMSG_CHN    (DEFAULT_PRIORITY)
#define THREAD_PRI_IPCC_CHN     (DEFAULT_PRIORITY + 2)
#define THREAD_PRI_IPCC_RX      (HIGH_PRIORITY)

/* Service Thread start here */
#define THREAD_PRI_WORKER       (30)
#define THREAD_PRI_SYSDIAG      (DEFAULT_PRIORITY)

/* Service Virtual I2C */
#define THREAD_PRI_VI2C         (DEFAULT_PRIORITY)

/* Service CAN Proxy */
#define THREAD_PRI_CANPROXY     (HIGH_PRIORITY)

/* Service Property */
#define THREAD_PRI_PROPERTY     (8)

/* Service Display */
#define THREAD_PRI_DISP_MAIN    (DEFAULT_PRIORITY)
#define THREAD_PRI_DISP_VSYN    (DEFAULT_PRIORITY + 2)

/* Service Sample */
#define THREAD_PRI_SAMPLE       (DEFAULT_PRIORITY)

/* Application Thread start, usaually less than 16 */


#endif //__SYS_PRIORITY_H__
