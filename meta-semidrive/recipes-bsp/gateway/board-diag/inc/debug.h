/*
 * debug.h
 *
 * Copyright (c) 2020 SemiDrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */
/*
 * debug.h
 *
 * Copyright (c) 2020 SemiDrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */
#ifndef _DEBUG_H
#define _DEBUG_H

#define ERROR(format, args...) fprintf(stderr, "ERROR:%s %d "format, __func__, __LINE__, ##args)
#define DBG(format, args...) //fprintf(stdout, "DEBUG:%s %d "format, __func__, __LINE__, ##args)

void hexdump8_ex(const void *ptr, size_t len, u_int64_t disp_addr);
void hexdump8(const void *ptr, size_t len);

#endif