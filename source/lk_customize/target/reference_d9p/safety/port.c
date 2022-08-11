#include "target_port.h"
#include "hal_port.h"
#include "hal_dio.h"
#include "res.h"
#include "chip_res.h"

const Port_PinType UART_TX_PIN = PortConf_PIN_GPIO_A8;
const Port_PinModeType UART_TX_PIN_MODE = {
    ((uint32_t)PORT_PAD_POE__DISABLE | PORT_PAD_IS__OUT | PORT_PAD_SR__FAST | PORT_PAD_DS__MID1 | PORT_PIN_IN_NO_PULL),
    ((uint32_t)PORT_PIN_MUX_FV__MIN | PORT_PIN_MUX_FIN__MIN  | PORT_PIN_OUT_PUSHPULL | PORT_PIN_MODE_ALT2),
};

const Port_PinType UART_RX_PIN = PortConf_PIN_GPIO_A9;
const Port_PinModeType UART_RX_PIN_MODE = {
    ((uint32_t)PORT_PAD_POE__DISABLE | PORT_PAD_IS__OUT | PORT_PAD_SR__FAST | PORT_PAD_DS__MID1 | PORT_PIN_IN_NO_PULL),
    ((uint32_t)PORT_PIN_MUX_FV__MIN | PORT_PIN_MUX_FIN__MIN  | PORT_PIN_OUT_PUSHPULL | PORT_PIN_MODE_ALT2),
};



void uart_port_init(void)
{
    void *port_init_handle = NULL;
    hal_port_creat_handle(&port_init_handle,
                          RES_PAD_CONTROL_SAF_JTAG_TMS);//safety iomuxc res

    if (port_init_handle) {
        hal_port_set_pin_mode(port_init_handle, UART_TX_PIN, UART_TX_PIN_MODE);
        hal_port_set_pin_mode(port_init_handle, UART_RX_PIN, UART_RX_PIN_MODE);
        hal_port_release_handle(port_init_handle);
    }
    else {
        dprintf(ALWAYS, "port get handle failed!\n");
    }
}
