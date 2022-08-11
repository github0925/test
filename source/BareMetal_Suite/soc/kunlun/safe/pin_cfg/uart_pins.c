/********************************************************
 *          Copyright(c) 2018   Semidrive               *
 ********************************************************/

#include <common_hdr.h>
#include <srv_pin/srv_pin.h>
#include <soc.h>
#include <iomux/iomux_def.h>

/* Pull-up, default drive strength */
#define DAT_PAD_SETTING    (FV_IO_PAD_CONFIG_DS(1)\
                            | BM_IO_PAD_CONFIG_PS \
                            | BM_IO_PAD_CONFIG_PE   \
                            )
#define PAD_SETTING_I2C     (FV_IO_PAD_CONFIG_DS(1)\
                                | BM_IO_PAD_CONFIG_PS \
                                | BM_IO_PAD_CONFIG_PE)

#if defined(CFG_DRV_UART)
const pad_ctrl_t uart1_pins[] = {
    PIN_DCLR(IOMUXC_SAFE, GPIO_A4, GPIO_A4, SEL_NONE,        /* TX */
             FV_PIN_MUX_CONFIG_MUX(1), DAT_PAD_SETTING, 0),
    PIN_DCLR(IOMUXC_SAFE, GPIO_A5, GPIO_A5, SEL_NONE,
             FV_PIN_MUX_CONFIG_MUX(1), DAT_PAD_SETTING, 0),  /* RX */

    PIN_DCLR_NONE,
};

const pad_ctrl_t uart3_pins[] = {
    PIN_DCLR(IOMUXC_SAFE, GPIO_A8, GPIO_A8, SEL_NONE,
             FV_PIN_MUX_CONFIG_MUX(2), DAT_PAD_SETTING, 0),
    PIN_DCLR(IOMUXC_SAFE, GPIO_A9, GPIO_A9, SEL_NONE,
             FV_PIN_MUX_CONFIG_MUX(2), DAT_PAD_SETTING, 0),

    PIN_DCLR_NONE,
};

const pad_ctrl_t uart4_pins[] = {
    PIN_DCLR(IOMUXC_SAFE, GPIO_A10, GPIO_A10, SEL_NONE,             /* UART4 TX for debug */
             FV_PIN_MUX_CONFIG_MUX(2), DAT_PAD_SETTING, 0),
    PIN_DCLR(IOMUXC_SAFE, GPIO_A11, GPIO_A11, SEL_NONE,
             FV_PIN_MUX_CONFIG_MUX(2), DAT_PAD_SETTING, 0),    /* UART4 RX for debug*/

    PIN_DCLR_NONE,
};

void soc_uart1_pin_cfg(void *para)
{
    srv_pin_cfg(&uart1_pins[0]);
}

void soc_uart3_pin_cfg(void *para)
{
    srv_pin_cfg(&uart3_pins[0]);
}

void soc_uart4_pin_cfg(void *para)
{
    srv_pin_cfg(&uart4_pins[0]);
}
#endif  /* CFG_DRV_UART */
