/*************************************************************************
    > File Name: include/fastboot_common.h
    > Author:
    > Mail:
    > Created Time: Mon 02 Dec 2019 03:50:25 PM CST
 ************************************************************************/

#ifndef __FASTBOOT_COMMON_H
#define __FASTBOOT_COMMON_H
#include <class_fastboot.h>

fastboot_t *fastboot_common_init(void *, int32_t);
void fastboot_common_okay(fastboot_t *, const char *);
void fastboot_common_fail(fastboot_t *, const char *);
void fastboot_common_info(fastboot_t *fb, const char *);
void fastboot_common_stop(fastboot_t *);
#endif
