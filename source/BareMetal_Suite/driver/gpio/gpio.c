/********************************************************
 *          Copyright(c) 2019   Semidrive               *
 ********************************************************/

#include <soc.h>
#include "gpio.h"

extern int32_t drv_gpio_cfg(U32 b, U32 pin, gpio_dir_e dir);
extern int32_t drv_gpio_rd(U32 b, U32 pin);
extern int32_t drv_gpio_wr(U32 b, U32 pin, bool bval);

int32_t gpio_cfg(module_e m, U32 pin, gpio_dir_e dir)
{
    U32 b = soc_get_module_base(m);
    return drv_gpio_cfg(b, pin, dir);
}

#if defined(CFG_DRV_GPIO_API_gpio_rd)
int32_t gpio_rd(module_e m, U32 pin)
{
    U32 b = soc_get_module_base(m);
    return drv_gpio_rd(b, pin);
}
#endif

int32_t gpio_wr(module_e m, U32 pin, bool bval)
{
    U32 b = soc_get_module_base(m);
    return drv_gpio_wr(b, pin, bval);
}
