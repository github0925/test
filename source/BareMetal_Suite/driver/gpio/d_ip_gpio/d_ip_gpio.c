/********************************************************
 *          Copyright(c) 2019   Semidrive               *
 ********************************************************/

#include "../gpio.h"
#include "gpio_reg.h"

int32_t drv_gpio_cfg(U32 b, U32 pin, gpio_dir_e dir)
{
    U32 v = readl(b + GPIO_CTRL_OFF(pin));
    v &= ~BM_GPIO_CTRL_DIR;

    if (GPIO_DIR_OUT == dir) {
        v |= BM_GPIO_CTRL_DIR;
    }

    writel(v, b + GPIO_CTRL_OFF(pin));

    return 0;
}

#if defined(CFG_DRV_GPIO_API_gpio_rd)
int32_t drv_gpio_rd(U32 b, U32 pin)
{
    U32 v = readl(b + GPIO_CTRL_OFF(pin));

    return (v & BM_GPIO_CTRL_DATA_IN) == 0 ? 0 : 1;
}
#endif

int32_t drv_gpio_wr(U32 b, U32 pin, bool bval)
{
    U32 v = readl(b + GPIO_CTRL_OFF(pin));
    v &= ~BM_GPIO_CTRL_DATA_OUT;

    if (bval) {
        v |= BM_GPIO_CTRL_DATA_OUT;
    }

    writel(v, b + GPIO_CTRL_OFF(pin));

    return 0;
}
