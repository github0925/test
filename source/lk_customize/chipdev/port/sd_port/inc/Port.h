

#ifndef PORT_H
#define PORT_H

/*******************************************************************************
**                      Include Section                                       **
*******************************************************************************/

#include <stdint.h>
#include <reg.h>
#include <stdio.h>
#include <debug.h>
#include <kernel/vm.h>

/* Inclusion of Platform_Types.h and Compiler.h */
#include "Std_Types.h"

/* Pre-compile/static configuration parameters for PORT  */
#include "port_cfg_def.h"

/*******************************************************************************
**                      Global Macro Definitions                              **
*******************************************************************************/

typedef uintptr_t addr_t;
typedef uintptr_t vaddr_t;
typedef uintptr_t paddr_t;


#define LOCAL_INLINE    static inline

/*
  Macros required for configuring the PORT driver
*/


/*
  Macros to define pin mode i.e. pin functionality
  These are used internally to define other macros
*/
#define  PORT_PIN_MODE_GPIO             (0x00U)
#define  PORT_PIN_MODE_ALT1             (0x01U)
#define  PORT_PIN_MODE_ALT2             (0x02U)
#define  PORT_PIN_MODE_ALT3             (0x03U)
#define  PORT_PIN_MODE_ALT4             (0x04U)
#define  PORT_PIN_MODE_ALT5             (0x05U)
#define  PORT_PIN_MODE_ALT6             (0x06U)
#define  PORT_PIN_MODE_ALT7             (0x07U)

/* Macros to define whether pin direction can be changed
   during runtime or not */
#define  PORT_PIN_DIRECTION_NOT_CHANGEABLE    (0x00U)
#define  PORT_PIN_DIRECTION_CHANGEABLE        (0x01U)

/* Macros to define whether pin mode can be changed
   during runtime or not */
#define  PORT_PIN_MODE_NOT_CHANGEABLE   (0x00U)
#define  PORT_PIN_MODE_CHANGEABLE       (0x01U)

/* Mode of operation */
#define PORT_MCAL_SUPERVISOR                 (0U)
#define PORT_MCAL_USER1                      (1U)

/* IO_PAD_CONFIG POE */
#define PORT_PAD_POE__ENABLE       (0x1U << 16)
#define PORT_PAD_POE__DISABLE      (0x0U << 16)

/* IO_PAD_CONFIG IS */
#define PORT_PAD_IS__IN             (0x0U << 12)
#define PORT_PAD_IS__OUT            (0x1U << 12)

/* IO_PAD_CONFIG SR */
#define PORT_PAD_SR__SLOW          (0x1U << 8)
#define PORT_PAD_SR__FAST          (0x0U << 8)

/* IO_PAD_CONFIG DS */
#define PORT_PAD_DS__LOW             (0x0U << 4)
#define PORT_PAD_DS__MID1            (0x1U << 4)
#define PORT_PAD_DS__MID2            (0x2U << 4)
#define PORT_PAD_DS__HIGH            (0x3U << 4)

/* IO_PAD_CONFIG PS/PE */
#define PORT_PAD_PS__PULL_UP        (0x3U)
#define PORT_PAD_PS__PULL_DOWN      (0x1U)
#define PORT_PAD_PE__DISABLE        (0x0U)

/* IO_PAD_CONFIG EMMC SP */
#define PORT_PAD_MMC_SP__MIN        (0x0U << 20)
#define PORT_PAD_MMC_SP__MAX        (0xFU << 20)

/* IO_PAD_CONFIG EMMC SN */
#define PORT_PAD_MMC_SN__MIN        (0x0U << 16)
#define PORT_PAD_MMC_SN__MAX        (0xFU << 16)

/* IO_PAD_CONFIG EMMC RXSEL */
#define PORT_PAD_MMC_RXSEL__MIN        (0x0U << 12)
#define PORT_PAD_MMC_RXSEL__IN         (0x2U << 12)
#define PORT_PAD_MMC_RXSEL__MAX        (0x7U << 12)

/* IO_PAD_CONFIG EMMC TXPREP */
#define PORT_PAD_MMC_TXPREP__MIN        (0x0U << 8)
#define PORT_PAD_MMC_TXPREP__MAX        (0xFU << 8)

/* IO_PAD_CONFIG EMMC TXPREN */
#define PORT_PAD_MMC_TXPREN__MIN        (0x0U << 4)
#define PORT_PAD_MMC_TXPREN__MAX        (0xFU << 4)

/* IO_PAD_CONFIG EMMC Pull-EN */
#define PORT_PAD_MMC_PULL__OFF        (0x0U)
#define PORT_PAD_MMC_PULL__UP         (0x1U)
#define PORT_PAD_MMC_PULL__DOWN       (0x2U)
#define PORT_PAD_MMC_PULL__RESERVED   (0x3U)

/* PIN_MUX_CONFIG FV */
#define PORT_PIN_MUX_FV__MIN        (0x0U << 12)
#define PORT_PIN_MUX_FV__MAX        (0x1U << 12)

/* PIN_MUX_CONFIG FIN */
#define PORT_PIN_MUX_FIN__MIN        (0x0U << 8)
#define PORT_PIN_MUX_FIN__1          (0x1U << 8)
#define PORT_PIN_MUX_FIN__MAX        (0x3U << 8)

/*
Configuration Options: Pin input pull resistor
-NO PULL
-PULL DOWN
-PULL UP
*/

/* IO_PAD_CONFIG PS/PE, except for EMMC pins */
#define PORT_PIN_IN_PULL_UP     (0x3U)

#define PORT_PIN_IN_PULL_DOWN   (0x1U)

#define PORT_PIN_IN_NO_PULL     (0x0U)

/*
Configuration Options: Pin output characteristics
-PUSHPULL
-OPENDRAIN
*/

/* PIN_MUX_CONFIG ODE */
#define PORT_PIN_OUT_PUSHPULL     (0x00U)

#define PORT_PIN_OUT_OPENDRAIN    (0x10U)

/* GPIO Controller id */
#define PORT_GPIO_1             (0x1U)
#define PORT_GPIO_2             (0x2U)
#define PORT_GPIO_3             (0x3U)
#define PORT_GPIO_4             (0x4U)
#define PORT_GPIO_5             (0x5U)

/*******************************************************************************
**                      Global Type Definitions                               **
*******************************************************************************/

/* Type definition for numeric id for port pins */
typedef uint16_t Port_PinType;

/* Type definition for port pin direction */
typedef enum {
    PORT_PIN_IN = 0x0U,
    PORT_PIN_OUT = 0x1U
} Port_PinDirectionType;


/* Type definition for Port pin mode */
typedef struct Port {
    uint32_t io_pad_config;
    uint32_t pin_mux_config;
} Port_PinModeType;


/* structure definition for PORT n PIN_MUX_CONFIG register  */
typedef struct {

    uint32_t PIN_MUX_CONFIG_0;      /* Pin 0  MUX CONFIG */
    uint32_t PIN_MUX_CONFIG_1;      /* Pin 1  MUX CONFIG */
    uint32_t PIN_MUX_CONFIG_2;      /* Pin 2  MUX CONFIG */
    uint32_t PIN_MUX_CONFIG_3;      /* Pin 3  MUX CONFIG */
    uint32_t PIN_MUX_CONFIG_4;      /* Pin 4  MUX CONFIG */
    uint32_t PIN_MUX_CONFIG_5;      /* Pin 5  MUX CONFIG */
    uint32_t PIN_MUX_CONFIG_6;      /* Pin 6  MUX CONFIG */
    uint32_t PIN_MUX_CONFIG_7;      /* Pin 7  MUX CONFIG */
    uint32_t PIN_MUX_CONFIG_8;      /* Pin 8  MUX CONFIG */
    uint32_t PIN_MUX_CONFIG_9;      /* Pin 9  MUX CONFIG */
    uint32_t PIN_MUX_CONFIG_10;      /* Pin 10  MUX CONFIG */
    uint32_t PIN_MUX_CONFIG_11;      /* Pin 11  MUX CONFIG */
    uint32_t PIN_MUX_CONFIG_12;      /* Pin 12  MUX CONFIG */
    uint32_t PIN_MUX_CONFIG_13;      /* Pin 13  MUX CONFIG */
    uint32_t PIN_MUX_CONFIG_14;      /* Pin 14  MUX CONFIG */
    uint32_t PIN_MUX_CONFIG_15;      /* Pin 15  MUX CONFIG */
    uint32_t PIN_MUX_CONFIG_16;      /* Pin 16  MUX CONFIG */
    uint32_t PIN_MUX_CONFIG_17;      /* Pin 17  MUX CONFIG */
    uint32_t PIN_MUX_CONFIG_18;      /* Pin 18  MUX CONFIG */
    uint32_t PIN_MUX_CONFIG_19;      /* Pin 19  MUX CONFIG */
    uint32_t PIN_MUX_CONFIG_20;      /* Pin 20  MUX CONFIG */
    uint32_t PIN_MUX_CONFIG_21;      /* Pin 21  MUX CONFIG */
    uint32_t PIN_MUX_CONFIG_22;      /* Pin 22  MUX CONFIG */
    uint32_t PIN_MUX_CONFIG_23;      /* Pin 23  MUX CONFIG */
    uint32_t PIN_MUX_CONFIG_24;      /* Pin 24  MUX CONFIG */
    uint32_t PIN_MUX_CONFIG_25;      /* Pin 25  MUX CONFIG */
    uint32_t PIN_MUX_CONFIG_26;      /* Pin 26  MUX CONFIG */
    uint32_t PIN_MUX_CONFIG_27;      /* Pin 27  MUX CONFIG */
    uint32_t PIN_MUX_CONFIG_28;      /* Pin 28  MUX CONFIG */
    uint32_t PIN_MUX_CONFIG_29;      /* Pin 29  MUX CONFIG */
    uint32_t PIN_MUX_CONFIG_30;      /* Pin 30  MUX CONFIG */
    uint32_t PIN_MUX_CONFIG_31;      /* Pin 31  MUX CONFIG */
} Port_n_PinMuxConfigType;

/* structure definition for PORT n IO_PAD_CONFIG register  */
typedef struct {

    uint32_t IO_PAD_CONFIG_0;      /* Pin 0  PAD CONFIG */
    uint32_t IO_PAD_CONFIG_1;      /* Pin 1  PAD CONFIG */
    uint32_t IO_PAD_CONFIG_2;      /* Pin 2  PAD CONFIG */
    uint32_t IO_PAD_CONFIG_3;      /* Pin 3  PAD CONFIG */
    uint32_t IO_PAD_CONFIG_4;      /* Pin 4  PAD CONFIG */
    uint32_t IO_PAD_CONFIG_5;      /* Pin 5  PAD CONFIG */
    uint32_t IO_PAD_CONFIG_6;      /* Pin 6  PAD CONFIG */
    uint32_t IO_PAD_CONFIG_7;      /* Pin 7  PAD CONFIG */
    uint32_t IO_PAD_CONFIG_8;      /* Pin 8  PAD CONFIG */
    uint32_t IO_PAD_CONFIG_9;      /* Pin 9  PAD CONFIG */
    uint32_t IO_PAD_CONFIG_10;      /* Pin 10  PAD CONFIG */
    uint32_t IO_PAD_CONFIG_11;      /* Pin 11  PAD CONFIG */
    uint32_t IO_PAD_CONFIG_12;      /* Pin 12  PAD CONFIG */
    uint32_t IO_PAD_CONFIG_13;      /* Pin 13  PAD CONFIG */
    uint32_t IO_PAD_CONFIG_14;      /* Pin 14  PAD CONFIG */
    uint32_t IO_PAD_CONFIG_15;      /* Pin 15  PAD CONFIG */
    uint32_t IO_PAD_CONFIG_16;      /* Pin 16  PAD CONFIG */
    uint32_t IO_PAD_CONFIG_17;      /* Pin 17  PAD CONFIG */
    uint32_t IO_PAD_CONFIG_18;      /* Pin 18  PAD CONFIG */
    uint32_t IO_PAD_CONFIG_19;      /* Pin 19  PAD CONFIG */
    uint32_t IO_PAD_CONFIG_20;      /* Pin 20  PAD CONFIG */
    uint32_t IO_PAD_CONFIG_21;      /* Pin 21  PAD CONFIG */
    uint32_t IO_PAD_CONFIG_22;      /* Pin 22  PAD CONFIG */
    uint32_t IO_PAD_CONFIG_23;      /* Pin 23  PAD CONFIG */
    uint32_t IO_PAD_CONFIG_24;      /* Pin 24  PAD CONFIG */
    uint32_t IO_PAD_CONFIG_25;      /* Pin 25  PAD CONFIG */
    uint32_t IO_PAD_CONFIG_26;      /* Pin 26  PAD CONFIG */
    uint32_t IO_PAD_CONFIG_27;      /* Pin 27  PAD CONFIG */
    uint32_t IO_PAD_CONFIG_28;      /* Pin 28  PAD CONFIG */
    uint32_t IO_PAD_CONFIG_29;      /* Pin 29  PAD CONFIG */
    uint32_t IO_PAD_CONFIG_30;      /* Pin 30  PAD CONFIG */
    uint32_t IO_PAD_CONFIG_31;      /* Pin 31  PAD CONFIG */

} Port_n_IOPadConfigType;

/* structure definition for GPIO controller id and direction info of each pin
 * low 16 bits for GPIO ID
 * high 16 bits for GPIO direction
*/
typedef struct {
    uint32_t IO_GPIO_ID_CONFIG_0;      /* Pin 0 */
    uint32_t IO_GPIO_ID_CONFIG_1;      /* Pin 1 */
    uint32_t IO_GPIO_ID_CONFIG_2;      /* Pin 2 */
    uint32_t IO_GPIO_ID_CONFIG_3;      /* Pin 3 */
    uint32_t IO_GPIO_ID_CONFIG_4;      /* Pin 4 */
    uint32_t IO_GPIO_ID_CONFIG_5;      /* Pin 5 */
    uint32_t IO_GPIO_ID_CONFIG_6;      /* Pin 6 */
    uint32_t IO_GPIO_ID_CONFIG_7;      /* Pin 7 */
    uint32_t IO_GPIO_ID_CONFIG_8;      /* Pin 8 */
    uint32_t IO_GPIO_ID_CONFIG_9;      /* Pin 9 */
    uint32_t IO_GPIO_ID_CONFIG_10;      /* Pin 10 */
    uint32_t IO_GPIO_ID_CONFIG_11;      /* Pin 11 */
    uint32_t IO_GPIO_ID_CONFIG_12;      /* Pin 12 */
    uint32_t IO_GPIO_ID_CONFIG_13;      /* Pin 13 */
    uint32_t IO_GPIO_ID_CONFIG_14;      /* Pin 14 */
    uint32_t IO_GPIO_ID_CONFIG_15;      /* Pin 15 */
    uint32_t IO_GPIO_ID_CONFIG_16;      /* Pin 16 */
    uint32_t IO_GPIO_ID_CONFIG_17;      /* Pin 17 */
    uint32_t IO_GPIO_ID_CONFIG_18;      /* Pin 18 */
    uint32_t IO_GPIO_ID_CONFIG_19;      /* Pin 19 */
    uint32_t IO_GPIO_ID_CONFIG_20;      /* Pin 20 */
    uint32_t IO_GPIO_ID_CONFIG_21;      /* Pin 21 */
    uint32_t IO_GPIO_ID_CONFIG_22;      /* Pin 22 */
    uint32_t IO_GPIO_ID_CONFIG_23;      /* Pin 23 */
    uint32_t IO_GPIO_ID_CONFIG_24;      /* Pin 24 */
    uint32_t IO_GPIO_ID_CONFIG_25;      /* Pin 25 */
    uint32_t IO_GPIO_ID_CONFIG_26;      /* Pin 26 */
    uint32_t IO_GPIO_ID_CONFIG_27;      /* Pin 27 */
    uint32_t IO_GPIO_ID_CONFIG_28;      /* Pin 28 */
    uint32_t IO_GPIO_ID_CONFIG_29;      /* Pin 29 */
    uint32_t IO_GPIO_ID_CONFIG_30;      /* Pin 30 */
    uint32_t IO_GPIO_ID_CONFIG_31;      /* Pin 31 */
} Port_n_GPIOIdConfigType;

typedef struct {

    unsigned int P0        : 1;    /* Pin 0  */
    unsigned int P1        : 1;    /* Pin 1  */
    unsigned int P2        : 1;    /* Pin 2  */
    unsigned int P3        : 1;    /* Pin 3  */
    unsigned int P4        : 1;    /* Pin 4  */
    unsigned int P5        : 1;    /* Pin 5  */
    unsigned int P6        : 1;    /* Pin 6  */
    unsigned int P7        : 1;    /* Pin 7  */
    unsigned int P8        : 1;    /* Pin 8  */
    unsigned int P9        : 1;    /* Pin 9  */
    unsigned int P10        : 1;    /* Pin 10  */
    unsigned int P11        : 1;    /* Pin 11  */
    unsigned int P12        : 1;    /* Pin 12  */
    unsigned int P13        : 1;    /* Pin 13  */
    unsigned int P14        : 1;    /* Pin 14  */
    unsigned int P15        : 1;    /* Pin 15  */
    unsigned int P16        : 1;    /* Pin 16  */
    unsigned int P17        : 1;    /* Pin 17  */
    unsigned int P18        : 1;    /* Pin 18  */
    unsigned int P19        : 1;    /* Pin 19  */
    unsigned int P20        : 1;    /* Pin 20  */
    unsigned int P21        : 1;    /* Pin 21  */
    unsigned int P22        : 1;    /* Pin 22  */
    unsigned int P23        : 1;    /* Pin 23  */
    unsigned int P24        : 1;    /* Pin 24  */
    unsigned int P25        : 1;    /* Pin 25  */
    unsigned int P26        : 1;    /* Pin 26  */
    unsigned int P27        : 1;    /* Pin 27  */
    unsigned int P28        : 1;    /* Pin 28  */
    unsigned int P29        : 1;    /* Pin 29  */
    unsigned int P30        : 1;    /* Pin 30  */
    unsigned int P31        : 1;    /* Pin 31  */

} Port_n_PinType;

/*
  Structure definition for PORT n configuration
*/
typedef struct {
    /* Port Parametric output, input select, slew rate, driver select, pull enable configuration */
    Port_n_IOPadConfigType  IOPadConfig;            // OFFSET: 0
    /* Port pins mode, open drain configuration */
    Port_n_PinMuxConfigType  PinMuxConfig;          // OFFSET: 32
    /* GPIO Controller id for each pin */
    Port_n_GPIOIdConfigType  PinGPIOIdConfig;        // OFFSET: 64
    /* Port pins initial level configuration */
    Port_n_PinType      PinLevel;                   // OFFSET: 96
    /* If Mode changeable is enabled */
#if (PORT_SET_PIN_MODE_API == STD_ON)
    /* Port pin run time mode changeable or not configuration */
    Port_n_PinType      ModeChangeControl;
#endif /* PORT_SET_PIN_MODE_API */

} Port_n_ConfigType;


/* to store the PORT init data */
typedef struct {
    /* Pointer to PORT SET configuration */
    const Port_n_ConfigType *PortConfigSetPtr;

} Port_ConfigType;

struct port_handle {
    paddr_t phy_addr;
    int32_t real_idx;

    paddr_t dio_phy_addr;
    int32_t dio_real_idx;
};

typedef struct port_delta_config {
    uint32_t misc_config;   // high(16): pin Index [0, 155], mid(8): pin level config, low(8): changeable config
    uint32_t pad_config;
    uint32_t mux_config;
    uint32_t gpio_ctrl_config;
} port_delta_config_t;

typedef enum port_disp_canfd_mux{
    PORT_MUX_DISP = 0x0U,
    PORT_MUX_CANFD = 0x1U
} port_disp_canfd_mux_t;
/*******************************************************************************
**                      Global Constant Declarations                          **
*******************************************************************************/



/*******************************************************************************
**                      Global Variable Declarations                          **
*******************************************************************************/

/*******************************************************************************
**                      Global Function Declarations                          **
*******************************************************************************/

extern void Port_Init
(
    const Port_ConfigType *ConfigPtr
);

extern void Port_SetPinDirection
(
    const Port_PinType Pin,
    const Port_PinDirectionType Direction
);

extern void Port_RefreshPortDirection(void);

extern void Port_GetVersionInfo
(
    Std_VersionInfoType *const versioninfo
);

extern void Port_SetPinMode
(
    const Port_PinType Pin,
    const Port_PinModeType Mode
);

extern void Port_SetHandle(
    void *handle
);

extern void Port_SetToGPIOCtrl(
    const uint32_t gpio_id,
    const uint32_t pin_num
);

extern void port_init_delta(
    port_delta_config_t *const delta_config,
    const uint32_t config_count
);

int port_set_pin_data(
    Port_PinModeType *pin_mode,
    const Port_PinType pin_num,
    int32_t data
);

/*
 * function for getting pin info
 * pin_num - pin index
 * pin_mode - pin pad/iomux configuration
 * gpio_ctrl - gpio controller index, valid for GPIO
 * direction - gpio input/output configuration, valid for GPIO
 * level - default level, valid for GPIO
*/
int port_get_pin_info(
    const Port_PinType pin,
    Port_PinModeType *pin_mode,
    uint32_t * input_select,
    uint32_t * gpio_ctrl,
    int32_t * gpio_config
);

/*
 * Function : init level2 mux config
 * mux - mux value, PORT_MUX_DISP - mux to disp, or mux to canfd
*/
void port_init_disp_canfd_mux(
	const port_disp_canfd_mux_t mux
);

/*******************************************************************************
**                      Global Inline Function Definitions                    **
*******************************************************************************/



#include "Port_PBcfg.h"

#endif   /*  PORT_H  */

