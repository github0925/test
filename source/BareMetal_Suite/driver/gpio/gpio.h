/********************************************************
 *          Copyright(c) 2019   Semidrive               *
 ********************************************************/

#ifndef __GPIO_H__
#define __GPIO_H__

#include <common_hdr.h>
#include <soc.h>

typedef enum {
    GPIO_DIR_IN,
    GPIO_DIR_OUT,
} gpio_dir_e;

int32_t gpio_cfg(module_e m, U32 pin, gpio_dir_e dir);
int32_t gpio_rd(module_e m, U32 pin);
int32_t gpio_wr(module_e m, U32 pin, bool bval);

#endif  /* __GPIO_H__ */
