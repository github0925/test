/********************************************************
 *          Copyright(c) 2018   Semidrive               *
 ********************************************************/

#include <common_hdr.h>
#include <srv_pin/srv_pin.h>
#include <soc.h>
#include <iomux/iomux_def.h>

/* Pull-up, default drive strength */
#define PAD_SETTING_UART    (FV_IO_PAD_CONFIG_DS(1)\
                            | BM_IO_PAD_CONFIG_PS \
                            | BM_IO_PAD_CONFIG_PE   \
                            )

#if defined(CFG_DRV_UART)

const pad_ctrl_t uart9_pins[] = {
    PIN_DCLR(IOMUXC_SEC, GPIO_C4, GPIO_C4, SEL_NONE,            /* UART9 TX */
             FV_PIN_MUX_CONFIG_MUX(1), PAD_SETTING_UART, 0),
    PIN_DCLR(IOMUXC_SEC, GPIO_C5, GPIO_C5, SEL_NONE,
             FV_PIN_MUX_CONFIG_MUX(1), PAD_SETTING_UART, 0),    /* UART9 RX */
    PIN_DCLR_NONE,
};

const pad_ctrl_t uart10_pins[] = {
    PIN_DCLR(IOMUXC_SEC, GPIO_C6, GPIO_C6, SEL_NONE,            /* UART10 TX */
             FV_PIN_MUX_CONFIG_MUX(1), PAD_SETTING_UART, 0),
    PIN_DCLR(IOMUXC_SEC, GPIO_C7, GPIO_C7, SEL_NONE,
             FV_PIN_MUX_CONFIG_MUX(1), PAD_SETTING_UART, 0),    /* UART10 RX */
    PIN_DCLR_NONE,
};

const pad_ctrl_t uart13_pins[] = {
    PIN_DCLR(IOMUXC_SEC, GPIO_D4, GPIO_D4, SEL_NONE,            /* UART13 TX */
             FV_PIN_MUX_CONFIG_MUX(1), PAD_SETTING_UART, 0),
    PIN_DCLR(IOMUXC_SEC, GPIO_D5, GPIO_D5, SEL_NONE,
             FV_PIN_MUX_CONFIG_MUX(1), PAD_SETTING_UART, 0),    /* UART13 RX */
    PIN_DCLR_NONE,
};

const pad_ctrl_t uart15_pins[] = {
    PIN_DCLR(IOMUXC_SEC, I2S_SC8_SD, I2S_SC8_SD, UART15_RX,      /* UART15 RX */
             FV_PIN_MUX_CONFIG_MUX(3), PAD_SETTING_UART, 2),
    PIN_DCLR(IOMUXC_SEC, I2S_SC7_SD, I2S_SC7_SD, SEL_NONE,
             FV_PIN_MUX_CONFIG_MUX(3), PAD_SETTING_UART, 0),    /* UART15 TX */
    PIN_DCLR_NONE,
};

void soc_uart_pin_cfg(module_e m, void *para)
{
    switch (m) {
    case UART9:
        srv_pin_cfg(&uart9_pins[0]);
        break;

    case UART10:
        srv_pin_cfg(&uart10_pins[0]);
        break;

    case UART13:
        srv_pin_cfg(&uart13_pins[0]);
        break;

    case UART15:
        srv_pin_cfg(&uart15_pins[0]);
        break;
    default:
        break;
    }
}

#endif  /* CFG_DRV_UART */
