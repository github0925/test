/*
 * partition_mem.h
 *
 * Copyright (c) 2020 SemiDrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */

#ifndef _PARTITION_MEM_H
#define _PARTITION_MEM_H

#define PTDEV_CALLOC(c, s)     calloc(c, s)
#define PTDEV_MEMALIGN(b, s)   malloc(s)
#define PTDEV_FREE(p)          free(p)

#endif
