
/* Own header file, this includes own configuration file also */
#include "Port.h"

#include "__regs_base.h"

/* Semidrive IOMUXC register header */
#include "__regs_iomux.h"

#include "scr_hal.h"
/*******************************************************************************
**                      Private Object Like Macro Definitions                 **
*******************************************************************************/

//#define DEBUG_PORT

#ifdef DEBUG_PORT
#define DBG(...) dprintf(ALWAYS, __VA_ARGS__)
#else
#define DBG(...)
#endif

/*
  Offset definitions for data in Port configuration
*/

/* Status to indicate that PORT is initialized */
#define PORT_INITIALIZED  ((uint8_t)1)

/* general constants */
#define PORT_CONSTANT_0x01     (0x01U)

/* Maximum port pin number */
//#define PORT_PIN_MAX_NUMBER    (0x1FU)

/*PortPin available */
#define PIN_AVAILABLE 32

/* Pin 0~47 are configured by IOMUXC_SAF. Pins above 47
 * are configured by IOMUXC_SEC.
 */
#define IOMUXC_SAF_PIN_NUM    (48)

/*  pin operation register */
// n: 0 ~155
#define GPIO_CTRL_PIN_0     0x00
#define GPIO_CTRL_PIN_1     0x10
#define GPIO_CTRL_SIZE      (GPIO_CTRL_PIN_1 - GPIO_CTRL_PIN_0)

// for each pin, 0 ~ 155
#define GPIO_CTRL_PIN_X(n)    (((n)<=47) ? ((n)>=24 ? ((n)-24)*GPIO_CTRL_SIZE : ((n)+24)*GPIO_CTRL_SIZE) : ((n)*GPIO_CTRL_SIZE))
#define GPIO_SET_PIN_X(n)   (GPIO_CTRL_PIN_X(n) +  0x4)
#define GPIO_CLEAR_PIN_X(n) (GPIO_CTRL_PIN_X(n) +  0x8)
#define GPIO_TOGGLE_PIN_X(n)    (GPIO_CTRL_PIN_X(n) +  0xc)

#define PORT_RUN_IN_SAFETY      (0)
#define PORT_RUN_IN_SECURE      (1)
#define PORT_RUN_IN_AP          (2)
/*******************************************************************************
**                   Function like macro definitions                          **
*******************************************************************************/

/*******************************************************************************
**                      Private Type Definitions                              **
*******************************************************************************/

/*******************************************************************************
**                      Global Constant Definitions                           **
*******************************************************************************/


/*******************************************************************************
**                      Global Variable Definitions                           **
*******************************************************************************/

static struct port_handle g_port_handle;

static int run_in_domain = -1; // 0 - safety, 1 - secue, 2 - AP

/*******************************************************************************
**                      Private Constant Definitions                          **
*******************************************************************************/

/*******************************************************************************
**                      Private Variable Definitions                          **
*******************************************************************************/

/* To store the Port driver configuration pointer */
static  const Port_ConfigType  *Port_kConfigPtr;

static const uint32_t PIN_MAX_NUMBER[5] = {31, 31, 31, 31, 27};

/*******************************************************************************
**                      Private Function Declarations                         **
*******************************************************************************/

/*INLINE function to initialize Port module*/
LOCAL_INLINE void Port_lIOInit(void);

LOCAL_INLINE void iomux_write(uint32_t value, uint32_t paddr);

LOCAL_INLINE uint32_t iomux_read(uint32_t paddr);

LOCAL_INLINE vaddr_t getIOPadAddrByPin(uint32_t pin);

LOCAL_INLINE vaddr_t getPinMuxAddrByPin(uint32_t pin);

LOCAL_INLINE void setupInputSourceSelRegByPin(uint32_t pin, uint32_t alt);

/*INLINE function to check if the port is
  available or not for the microcontroller    */
LOCAL_INLINE uint32_t Port_lIsPortAvailable(uint32_t Port);

static void Port_ConfigMMC2CANMuxSCR(void);

LOCAL_INLINE vaddr_t getGPIOBaseAddr(void);

LOCAL_INLINE vaddr_t getGPIOAddr(uint32_t gpio_index);

static void Port_SetSafetyGPIOSCR(uint32_t gpio_id, uint32_t pin_num);

static void Port_SetSecureGPIOSCR(uint32_t gpio_id, uint32_t pin_num);

static void Port_SetGPIOSCR(uint32_t gpio_id, uint32_t pin_num);

static void Port_getSafetyGPIOSCR(uint32_t pin_num, uint32_t * gpio_id);

static void Port_getSecureGPIOSCR(uint32_t pin_num, uint32_t * gpio_id);

static void Port_getGPIOSCR(uint32_t pin_num, uint32_t * gpio_id);

/*******************************************************************************
**                      Global Function Definitions                           **
*******************************************************************************/

void Port_Init ( const Port_ConfigType *const ConfigPtr )
{
    DBG("+++Port_Init \n");
    Port_kConfigPtr = ConfigPtr;

    /* Initialise General Purpose I/O Ports */
    Port_ConfigMMC2CANMuxSCR();

    Port_lIOInit();

    DBG("---Port_Init finished!\n");
}/* Port_Init */


/* Enable / Disable the use of the function */
#if (PORT_SET_PIN_DIRECTION_API == STD_ON)
void Port_SetPinDirection(
    const Port_PinType Pin,
    const Port_PinDirectionType Direction
)
{
    vaddr_t             IOPadAddr;
    uint32_t            IOPadVal;

    DBG("+++Port_SetPinDirection, pin[%d], Dir[%d] \n", Pin, Direction);
    /* setup IO_PAD_CONFIG */
    IOPadAddr = getIOPadAddrByPin(Pin);
    IOPadVal = ((uint32_t)iomux_read(IOPadAddr) & 0xFFFFEFFFU) | (Direction << 12);
    iomux_write(IOPadVal, IOPadAddr);

    /* changed the GPIO Direction too, no need to check wether in ATL0 */
    /* Non-EMMC pins GPIO Setup */
    /* GPIO/DIO mode, do DIO Init */
    if (Direction == ((uint32_t)PORT_PIN_IN)) {
        /* GPIO Input mode */
        iomux_write(0x00, getGPIOBaseAddr() + GPIO_CTRL_PIN_X(Pin));
        DBG("setup GPIO[%d] to input, address: 0x%lx, value: %d\n",
            (Pin), getGPIOBaseAddr() + GPIO_CTRL_PIN_X(Pin), PORT_PIN_IN);
    }
    else {
        /* GPIO Output mode*/
        iomux_write(0x01, getGPIOBaseAddr() + GPIO_CTRL_PIN_X(Pin));
        DBG("setup GPIO[%d] to output, address: 0x%lx, value: %d\n",
            (Pin), getGPIOBaseAddr() + GPIO_CTRL_PIN_X(Pin), PORT_PIN_OUT);
    }

} /* Port_SetPinDirection */
#endif /* Direction changes allowed / Port_SetPinDirection API is ON */

void Port_RefreshPortDirection(void)
{
    uint32_t                LoopCtr;
    /* Each Port Number for the hardware unit  */
    uint32_t                PortNumber;
    uint32_t                ConfigIndex;
    const uint32_t          *DataPtr;
    vaddr_t                 IOPadAddr;
    uint32_t                RegVal;
    uint32_t                gpio_index;

    DBG("+++Port_RefreshPortDirection \n");
    /* Loop from Port 0 till last Port */
    PortNumber = 0U;
    ConfigIndex = 0U;

    do {
        DBG("PortNumber[%d]\n", PortNumber);

        /*Check if the port is available*/
        if (Port_lIsPortAvailable(PortNumber) != (uint32_t)0U) {
            DBG("ConfigIndex[%d]\n", ConfigIndex);
            /* Pointer to the data for the port */
            DataPtr = (const uint32_t *)(const void *)
                      ((Port_kConfigPtr->PortConfigSetPtr) + ConfigIndex);

            /* Start from the first pin */
            LoopCtr = 0U;

            do {
                DBG("Pin[%d]\n", (PortNumber * 32 + LoopCtr));

                if (((run_in_domain == PORT_RUN_IN_SECURE) || (run_in_domain == PORT_RUN_IN_AP)) && (PortNumber * 32 + LoopCtr) <= 47)
                    continue;

                /* setup IO_PAD_CONFIG for default Direction */
                IOPadAddr = getIOPadAddrByPin(PortNumber * 32 + LoopCtr);
                iomux_write((uint32_t)(*(DataPtr + LoopCtr)), IOPadAddr);
                /* set default GPIO registers */
                /* get MUX_MODE */
                RegVal = (uint32_t)(*(DataPtr + 32 + LoopCtr)) & 0x7;

                if (RegVal == PORT_PIN_MODE_GPIO) {
                    /* GPIO/DIO mode, do DIO Init */
                    /* Non-EMMC pins GPIO Setup */
                    /* GPIO/DIO mode, do DIO Init */

                    gpio_index = (*(DataPtr + 64 + LoopCtr)) & 0xffff;
                    if (((uint32_t)(*(DataPtr + 64 + LoopCtr)) >> 16) == ((uint32_t)PORT_PIN_IN)) {
                        /* GPIO Input mode */
                        iomux_write(0x00, getGPIOAddr(gpio_index) + GPIO_CTRL_PIN_X(
                                        PortNumber * 32 + LoopCtr));
                        DBG("setup GPIO[%d] to input\n", (PortNumber * 32 + LoopCtr));
                    }
                    else {
                        /* GPIO Output mode*/
                        iomux_write(0x01, getGPIOAddr(gpio_index) + GPIO_CTRL_PIN_X(
                                        PortNumber * 32 + LoopCtr));
                        DBG("setup GPIO[%d] to output\n", (PortNumber * 32 + LoopCtr));
                    }

                    /* Config GPIO SCR based on GPIO Controller id */
                    Port_SetGPIOSCR((uint32_t)(*(DataPtr + 64 + LoopCtr)) & 0xffff,
                                    (PortNumber * 32 + LoopCtr));
                }

                /* Loop for each pin in the port */
                LoopCtr++;
            }
            while (LoopCtr <= (uint32_t)PIN_MAX_NUMBER[PortNumber] );

            ConfigIndex++;
        }

        PortNumber++;
    }
    while (PortNumber <= (uint32_t)
            PORT_MAX_NUMBER);   /* Loop for all the ports */

}/* Port_RefreshPortDirection */

/* Enable / Disable the use of the function */
#if (PORT_SET_PIN_MODE_API == STD_ON)
//AUTOSAR used, zhuming
void Port_SetPinMode(const Port_PinType Pin, const Port_PinModeType Mode)
{
    vaddr_t                 IOPadAddr;
    vaddr_t                 PinMuxAddr;
    uint32_t                AltNum;
    //uint32_t                RegVal;

    DBG("+++Port_SetPinMode \n");
    /* setup IO_PAD_CONFIG */
    IOPadAddr = getIOPadAddrByPin(Pin);

    if (IOPadAddr)
        iomux_write(Mode.io_pad_config, IOPadAddr);

    /* setup PIN_MUX_CONFIG, and INPUT_SOURCE_SELECT if needed */
    PinMuxAddr  = getPinMuxAddrByPin(Pin);

    if (PinMuxAddr)
        iomux_write(Mode.pin_mux_config, PinMuxAddr);

#if 0
    /* Setup GPIO registers if ATL0,
     * get MUX_MODE
     */
    RegVal = (uint32_t)(Mode.pin_mux_config) & 0x7;
    if(PortConf_PIN_GPIO_SAF == Pin)
        RegVal = PORT_PIN_MODE_GPIO;

    //just set pin mode and not set GPIO controller config
    if (RegVal == PORT_PIN_MODE_GPIO) {
        /* GPIO/DIO mode, do DIO Init */
        if ( Pin < 132 || PortConf_PIN_GPIO_SAF == Pin) {
            /* Non-EMMC pins GPIO Setup */
            /* GPIO/DIO mode, do DIO Init */
            RegVal = ((uint32_t)(Mode.io_pad_config) >> 12) & (uint32_t)0x01;

            if (RegVal == ((uint32_t)PORT_PAD_IS__IN >> 12)) {
                /* GPIO Input mode */
                iomux_write(0x00, getGPIOBaseAddr() + GPIO_CTRL_PIN_X(Pin));
                DBG("setup GPIO[%d] to input\n", (Pin));
            }
            else {
                /* GPIO Output mode*/
                iomux_write(0x01, getGPIOBaseAddr() + GPIO_CTRL_PIN_X(Pin));
                DBG("setup GPIO[%d] to output\n", (Pin));
            }
        }
        else {
            /* EMMC pins GPIO Setup */
            RegVal = ((uint32_t)(Mode.io_pad_config) >> 12) & (uint32_t)0x07;

            if (RegVal == ((uint32_t)PORT_PAD_MMC_RXSEL__IN >> 12)) {
                /* EMMC GPIO Input mode */
                iomux_write(0x00, getGPIOBaseAddr() + GPIO_CTRL_PIN_X(Pin));
                DBG("setup EMMC GPIO[%d] to input\n", (Pin));
            }
            else {
                /* EMMC GPIO Output mode*/
                iomux_write(0x01, getGPIOBaseAddr() + GPIO_CTRL_PIN_X(Pin));
                DBG("setup EMMC GPIO[%d] to output\n", (Pin));
            }
        }
    }

    if (RegVal == PORT_PIN_MODE_GPIO) {
        Port_SetGPIOSCR(g_port_handle.dio_real_idx, Pin);
    }
#endif

    AltNum = (uint32_t)Mode.pin_mux_config & 0x00000007;
    setupInputSourceSelRegByPin(Pin, AltNum);
}/* Port_SetPinMode */
#endif /* (PORT_SET_PIN_MODE_API == STD_ON) */


/* Enable / Disable the use of the function */
#if (PORT_VERSION_INFO_API == STD_ON)
//AUTOSAR used, zhuming
void Port_GetVersionInfo(Std_VersionInfoType *const versioninfo)
{
    /* Vendor ID information */
    ((Std_VersionInfoType *)(versioninfo))->vendorID = 0; // TODO
    /* Port module ID information */
    ((Std_VersionInfoType *)(versioninfo))->moduleID = 0; // TODO
    /* Port module Software major version information */
    ((Std_VersionInfoType *)(versioninfo))->sw_major_version =
        (uint8_t)PORT_SW_MAJOR_VERSION;
    /* Port module Software minor version information */
    ((Std_VersionInfoType *)(versioninfo))->sw_minor_version =
        (uint8_t)PORT_SW_MINOR_VERSION;
    /* Port module Software patch version information */
    ((Std_VersionInfoType *)(versioninfo))->sw_patch_version =
        (uint8_t)PORT_SW_PATCH_VERSION;

}
#endif /*(PORT_VERSION_INFO_API == STD_ON)*/

void Port_SetHandle(void *handle)
{
    struct port_handle *p_handle;

    if (handle != NULL) {
        p_handle = (struct port_handle *)handle;

        g_port_handle.phy_addr = p_handle->phy_addr;
        g_port_handle.real_idx = p_handle->real_idx;
        g_port_handle.dio_phy_addr = p_handle->dio_phy_addr;
        g_port_handle.dio_real_idx = p_handle->dio_real_idx;
        DBG("Port_SetHandle: phy_addr[0x%lx], idx[%d]\n",
            g_port_handle.phy_addr, g_port_handle.real_idx);
        DBG("Port_SetHandle: dio_phy_addr[0x%lx], dio_idx[%d]\n",
            g_port_handle.dio_phy_addr, g_port_handle.dio_real_idx);
#if SAF_SYSTEM_CFG
        run_in_domain = PORT_RUN_IN_SAFETY;
        DBG("Port_SetHandle: Run in Safety!\n");
#elif SEC_SYSTEM_CFG
        run_in_domain = PORT_RUN_IN_SECURE;
        DBG("Port_SetHandle: Run in Secure!\n");
#else
        run_in_domain = PORT_RUN_IN_AP;
        DBG("Port_SetHandle: Run in AP!\n");
#endif
    }
}

void Port_SetToGPIOCtrl(const uint32_t gpio_id, const uint32_t pin_num)
{
    Port_SetGPIOSCR(gpio_id, pin_num);
}

/*******************************************************************************
**                      Private Function Definitions                          **
*******************************************************************************/

LOCAL_INLINE void iomux_write(uint32_t value, uint32_t paddr)
{
#ifdef WITH_KERNEL_VM
    vaddr_t        vaddr = (vaddr_t)paddr_to_kvaddr(paddr);
#else
    vaddr_t        vaddr = (vaddr_t)paddr;
#endif
    DBG("iomux_write:  value[0x%x], vaddr[0x%lx]\n", value, vaddr);
    writel(value, vaddr);
}

LOCAL_INLINE uint32_t iomux_read(uint32_t paddr)
{
    uint32_t value;
#ifdef WITH_KERNEL_VM
    vaddr_t        vaddr = (vaddr_t)paddr_to_kvaddr(paddr);
#else
    vaddr_t        vaddr = (vaddr_t)paddr;
#endif
    value = readl(vaddr);
    DBG("iomux_read:  value[0x%x], vaddr[0x%lx]\n", value, vaddr);
    return value;
}

LOCAL_INLINE vaddr_t getIOPadAddrByPin(uint32_t pin)
{
    if (PortConf_PIN_GPIO_SAF == pin)
        return (APB_IOMUXC_SAF_BASE
                + ((GPIO_SAF_PAD_OFF) << 10));

    if (pin >= PORT_PIN_CNT)
        return 0;

    DBG("io_pad_config_reg_offset[%d][0x%x]\n", pin,
        (x9_pins[pin].io_pad_config_reg_offset));

    if (pin < IOMUXC_SAF_PIN_NUM)
        return (APB_IOMUXC_SAF_BASE
                + ((x9_pins[pin].io_pad_config_reg_offset) << 10));
    else {
        return (APB_IOMUXC_SEC_BASE
                + ((x9_pins[pin].io_pad_config_reg_offset) << 10));
    }
}

LOCAL_INLINE vaddr_t getPinMuxAddrByPin(uint32_t pin)
{
    if (pin >= PORT_PIN_CNT)
        return 0;

    DBG("pin_mux_config_reg_offset[%d][0x%x]\n", pin,
        (x9_pins[pin].pin_mux_config_reg_offset));

    if (pin < IOMUXC_SAF_PIN_NUM)
        return (APB_IOMUXC_SAF_BASE
                + ((x9_pins[pin].pin_mux_config_reg_offset) << 10));
    else {
        return (APB_IOMUXC_SEC_BASE
                + ((x9_pins[pin].pin_mux_config_reg_offset) << 10));
    }
}

LOCAL_INLINE void setupInputSourceSelRegByPin(uint32_t pin, uint32_t alt)
{
    vaddr_t inputSourceSelAddr;
    uint32_t  val = 0;

    if (pin >= PORT_PIN_CNT)
        return;

    if ((x9_pins[pin].alt_funcs[alt].input_source_sel_reg_offset)  == 0) {
        DBG("No need to setup input_source_sel!\n");
        return;
    }

    DBG("input_source_sel_reg_offset[%d][%d][0x%x]\n", pin, alt,
        (x9_pins[pin].alt_funcs[alt].input_source_sel_reg_offset));
    if (pin < IOMUXC_SAF_PIN_NUM)
    {
        inputSourceSelAddr = APB_IOMUXC_SAF_BASE +
                         ((x9_pins[pin].alt_funcs[alt].input_source_sel_reg_offset) << 10);
    }
    else {
        inputSourceSelAddr = APB_IOMUXC_SEC_BASE +
                         ((x9_pins[pin].alt_funcs[alt].input_source_sel_reg_offset) << 10);
    }

    //val = x9_pins[pin].alt_funcs[alt].sel_val & ((0x01 << x9_pins[pin].alt_funcs[alt].sel_size) - 1);
    val = x9_pins[pin].alt_funcs[alt].sel_val;
    DBG("input_sel Val[%d]\n", val);
    iomux_write(val, inputSourceSelAddr);
}

LOCAL_INLINE void getInputSourceSelRegByPin(uint32_t pin, uint32_t alt, uint32_t * input_select)
{
    vaddr_t inputSourceSelAddr;

    if (pin >= PORT_PIN_CNT)
        return;

    if ((x9_pins[pin].alt_funcs[alt].input_source_sel_reg_offset)  == 0) {
        DBG("No need to setup input_source_sel!\n");
        return;
    }

    DBG("input_source_sel_reg_offset[%d][%d][0x%x]\n", pin, alt,
        (x9_pins[pin].alt_funcs[alt].input_source_sel_reg_offset));
    if (pin < IOMUXC_SAF_PIN_NUM)
    {
        inputSourceSelAddr = APB_IOMUXC_SAF_BASE +
                         ((x9_pins[pin].alt_funcs[alt].input_source_sel_reg_offset) << 10);
    }
    else {
        inputSourceSelAddr = APB_IOMUXC_SEC_BASE +
                         ((x9_pins[pin].alt_funcs[alt].input_source_sel_reg_offset) << 10);
    }

    *input_select = iomux_read(inputSourceSelAddr);
    DBG("input_sel Val[%d]\n", *input_select);
}

/*
 * GPIO MUX SCR setup
 */
struct scr_signal {
    const char      *name;
    scr_signal_t     signal;
    uint32_t        val;
};

#define SCR_SIGNAL(x, y) { \
    .name = #x, \
    .signal = x, \
    .val = y, \
}

#define  BIT_MASK(n)  ((1L<<(n))-1)
#define SIZEOF(a)   (sizeof (a) / sizeof (a[0]))

static const struct scr_signal scr_mshc2_canfd_signals[] = {
    SCR_SIGNAL(SCR_SEC__RW__mshc2_phy_resetb_scr,             0x1),  // enable emmc2 related pad as GPIO
    SCR_SIGNAL(SCR_SEC__L31__iomux_wrap_ap_pio_disp_mux__data1__oe,             0x1), // DSIP/CANFD9 PIN Mux
    SCR_SIGNAL(SCR_SEC__L31__iomux_wrap_ap_pio_disp_mux__data3__oe,             0x1), // DSIP/CANFD10 PIN Mux
    SCR_SIGNAL(SCR_SEC__L31__iomux_wrap_ap_pio_disp_mux__data5__oe,             0x1), // DSIP/CANFD11 PIN Mux
    SCR_SIGNAL(SCR_SEC__L31__iomux_wrap_ap_pio_disp_mux__data7__oe,             0x1), // DSIP/CANFD12 PIN Mux
    SCR_SIGNAL(SCR_SEC__L31__iomux_wrap_ap_pio_disp_mux__data9__oe,             0x1), // DSIP/CANFD13 PIN Mux
    SCR_SIGNAL(SCR_SEC__L31__iomux_wrap_ap_pio_disp_mux__data11__oe,             0x1), // DSIP/CANFD14 PIN Mux
    SCR_SIGNAL(SCR_SEC__L31__iomux_wrap_ap_pio_disp_mux__data13__oe,             0x1), // DSIP/CANFD15 PIN Mux
    SCR_SIGNAL(SCR_SEC__L31__iomux_wrap_ap_pio_disp_mux__data15__oe,             0x1), // DSIP/CANFD16 PIN Mux
    SCR_SIGNAL(SCR_SEC__L31__iomux_wrap_ap_pio_disp_mux__data17__oe,             0x1), // DSIP/CANFD17 PIN Mux
    SCR_SIGNAL(SCR_SEC__L31__iomux_wrap_ap_pio_disp_mux__data19__oe,             0x1), // DSIP/CANFD18 PIN Mux
    SCR_SIGNAL(SCR_SEC__L31__iomux_wrap_ap_pio_disp_mux__data21__oe,             0x1), // DSIP/CANFD19 PIN Mux
    SCR_SIGNAL(SCR_SEC__L31__iomux_wrap_ap_pio_disp_mux__data23__oe,             0x1), // DSIP/CANFD20 PIN Mux

};

static const struct scr_signal scr_safety_mux_signals[] = {
    SCR_SIGNAL(SCR_SAFETY__L16__gpio_mux_saf_gpio_sel_15_0,      0x0), //8
    SCR_SIGNAL(SCR_SAFETY__L16__gpio_mux_saf_gpio_sel_31_16,     0x0), //16
    SCR_SIGNAL(SCR_SAFETY__L16__gpio_mux_saf_gpio_sel_47_32,     0x0), //24
    SCR_SIGNAL(SCR_SAFETY__L16__gpio_mux_saf_gpio_sel_63_48,     0x0), //32
    SCR_SIGNAL(SCR_SAFETY__L16__gpio_mux_saf_gpio_sel_79_64,     0x0), //40
    SCR_SIGNAL(SCR_SAFETY__L16__gpio_mux_saf_gpio_sel_95_80,     0x0), //48
};

static const struct scr_signal scr_safety_mux1_signals[] = {
    SCR_SIGNAL(SCR_SAFETY__L16__gpio_mux1_gpio_sel_15_0,     0x0),
    SCR_SIGNAL(SCR_SAFETY__L16__gpio_mux1_gpio_sel_31_16,     0x0),
    SCR_SIGNAL(SCR_SAFETY__L16__gpio_mux1_gpio_sel_47_32,     0x0),
};

static const struct scr_signal scr_sec_mux_signals[] = {
    SCR_SIGNAL(SCR_SEC__L16__gpio_mux_sec_gpio_sel_15_0,      0x0), //8
    SCR_SIGNAL(SCR_SEC__L16__gpio_mux_sec_gpio_sel_31_16,     0x0), //16
    SCR_SIGNAL(SCR_SEC__L16__gpio_mux_sec_gpio_sel_47_32,     0x0), //24
    SCR_SIGNAL(SCR_SEC__L16__gpio_mux_sec_gpio_sel_63_48,     0x0), //32
    SCR_SIGNAL(SCR_SEC__L16__gpio_mux_sec_gpio_sel_79_64,     0x0), //40
    SCR_SIGNAL(SCR_SEC__L16__gpio_mux_sec_gpio_sel_95_80,     0x0), //48
    SCR_SIGNAL(SCR_SEC__L16__gpio_mux_sec_gpio_sel_111_96,    0x0), //56
    SCR_SIGNAL(SCR_SEC__L16__gpio_mux_sec_gpio_sel_127_112,   0x0), //64
    SCR_SIGNAL(SCR_SEC__L16__gpio_mux_sec_gpio_sel_143_128,   0x0), //72
    SCR_SIGNAL(SCR_SEC__L16__gpio_mux_sec_gpio_sel_159_144,   0x0), //80
    SCR_SIGNAL(SCR_SEC__L16__gpio_mux_sec_gpio_sel_175_160,   0x0), //88
    SCR_SIGNAL(SCR_SEC__L16__gpio_mux_sec_gpio_sel_191_176,   0x0), //96
    SCR_SIGNAL(SCR_SEC__L16__gpio_mux_sec_gpio_sel_207_192,   0x0), //104
    SCR_SIGNAL(SCR_SEC__L16__gpio_mux_sec_gpio_sel_215_208,   0x0), //108
};

static const struct scr_signal scr_sec_mux2_signals[] = {
    SCR_SIGNAL(SCR_SEC__L16__gpio_mux2_gpio_sel_15_0,         0xFFFF), //16
    SCR_SIGNAL(SCR_SEC__L16__gpio_mux2_gpio_sel_31_16,        0xFFFF), //32
    SCR_SIGNAL(SCR_SEC__L16__gpio_mux2_gpio_sel_47_32,        0xFFFF), //48
    SCR_SIGNAL(SCR_SEC__L16__gpio_mux2_gpio_sel_63_48,        0xFFFF), //64
    SCR_SIGNAL(SCR_SEC__L16__gpio_mux2_gpio_sel_79_64,        0xFFFF), //80
    SCR_SIGNAL(SCR_SEC__L16__gpio_mux2_gpio_sel_95_80,        0xFFFF), //96
    SCR_SIGNAL(SCR_SEC__L16__gpio_mux2_gpio_sel_107_96,       0xFFFF), //108
};

static bool scr_read_signal(const struct scr_signal *signal, uint32_t *val)
{
    scr_handle_t handle;

    handle = hal_scr_create_handle(signal->signal);

    if (handle) {
        *val = hal_scr_get(handle);
        hal_scr_delete_handle(handle);
        return true;
    }
    else {
        DBG("Can not get handle for %s\n", signal->name);
        return false;
    }
}

static bool scr_write_signal(const struct scr_signal *signal, uint32_t val)
{
    scr_handle_t handle;

    handle = hal_scr_create_handle(signal->signal);

    if (handle) {
        bool ret = hal_scr_set(handle, val);

        if (!ret)
            DBG("Failed to set %s\n", signal->name);

        hal_scr_delete_handle(handle);
        return ret;
    }
    else {
        DBG("Can not get handle for %s\n", signal->name);
        return false;
    }
}

static void Port_SetSafetyGPIOSCR(uint32_t gpio_id, uint32_t pin_num)
{
    uint32_t val1;
    uint32_t val2;
    uint32_t val1_mux1;
    uint32_t val2_mux1;

    uint32_t sigReg_id = pin_num / 8;
    uint32_t sigReg_id_mux1 = pin_num / 16;

    const struct scr_signal *signal_mux1 = &scr_safety_mux1_signals[sigReg_id_mux1];
    scr_read_signal(signal_mux1, &val1_mux1);
    if (1 == gpio_id) {
        val1_mux1 &= (~ (uint32_t)(0x1 << (pin_num % 16))); /* set to 0 */
    }
    else {
        const struct scr_signal *signal = &scr_safety_mux_signals[sigReg_id];
        scr_read_signal(signal, &val1);

        val1 &= (~ (uint32_t)(0x3 << ((pin_num % 8) * 2))); /* set to 0 */
        val1 |= (uint32_t)((gpio_id - 2) << ((pin_num % 8 ) * 2)); /* set mux val */
        val1_mux1 |= (uint32_t)(0x1 << (pin_num % 16)); /* set to 0x1 */
        scr_write_signal(signal, val1);
        scr_read_signal(signal, &val2);
        DBG("%s: 0x%x / 0x%x\n", signal->name, val1, val2);
    }

    scr_write_signal(signal_mux1, val1_mux1);
    scr_read_signal(signal_mux1, &val2_mux1);
    DBG("GPIO id: %d: 0x%x / 0x%x\n", gpio_id, val1_mux1, val2_mux1);
}

static void Port_SetSecureGPIOSCR(uint32_t gpio_id, uint32_t pin_num)
{
    uint32_t val1;
    uint32_t val2;
    uint32_t val1_mux2;
    uint32_t val2_mux2;

    uint32_t sigReg_id;
    uint32_t sigReg_id_mux2;

    uint32_t tmpPinNum = pin_num - 48;

    sigReg_id = tmpPinNum / 8;
    sigReg_id_mux2 = tmpPinNum / 16;

    DBG("Port_SetSecureGPIOSCR: sigReg_id[%d], sigReg_id_mux2[%d]\n", sigReg_id,
        sigReg_id_mux2);

    const struct scr_signal *signal_mux2 = &scr_sec_mux2_signals[sigReg_id_mux2];
    scr_read_signal(signal_mux2, &val1_mux2);

    if (1 == gpio_id) {
        val1_mux2 &= (~(uint32_t)(0x1 << (tmpPinNum % 16))); /* set to 0x0 */
    }
    else {
        const struct scr_signal *signal = &scr_sec_mux_signals[sigReg_id];
        scr_read_signal(signal, &val1);

        val1 &= (~(uint32_t)(0x03 << ((tmpPinNum % 8) * 2))); /* set to 0 */
        val1 |= (uint32_t)((gpio_id - 2) << ((tmpPinNum %8 ) *2)); /* set mux val */
        val1_mux2 |= (uint32_t)(0x1 << (tmpPinNum % 16)); /* set to 0x1 */
        scr_write_signal(signal, val1);
        scr_read_signal(signal, &val2);
        DBG("%s: 0x%x / 0x%x\n", signal->name, val1, val2);
    }

    scr_write_signal(signal_mux2, val1_mux2);
    scr_read_signal(signal_mux2, &val2_mux2);
    DBG("%s: 0x%x / 0x%x\n", signal_mux2->name, val1_mux2, val2_mux2);
}

/* set GPIO SCR based on gpio_id & pin_num(0~155) */
static void Port_SetGPIOSCR(uint32_t gpio_id, uint32_t pin_num)
{

    uint32_t pin_num_internal = 0;
    DBG("Port_SetGPIOSCR: gpio_id[%d], pin_num[%d]\n", gpio_id, pin_num);

    /* safety GPIO CTRL base address index NOT follow PinList */
    pin_num_internal =  (((pin_num) <= 47) ? ((pin_num) >= 24 ? ((
                             pin_num) - 24) : ((pin_num) + 24)) : (pin_num));

#if 0
    if (gpio_id == 1) {
        /* GPIO1 for Safety */
        Port_SetSafetyGPIOSCR(pin_num_internal);
    }
    else if (gpio_id == 2) {
        /* GPIO2 for secure */
        Port_SetSecureGPIOSCR(pin_num_internal, 0x00);
    }
    else if (gpio_id == 3) {
        /* GPIO3 */
        Port_SetSecureGPIOSCR(pin_num_internal, 0x01);
    }
    else if (gpio_id == 4) {
        /* GPIO4 */
        Port_SetSecureGPIOSCR(pin_num_internal, 0x02);
    }
    else if (gpio_id == 5) {
        /* GPIO5 */
        Port_SetSecureGPIOSCR(pin_num_internal, 0x03);
    }
#endif

    if (pin_num_internal > 47) {
        Port_SetSecureGPIOSCR(gpio_id, pin_num_internal);
    }
    else {
        Port_SetSafetyGPIOSCR(gpio_id, pin_num_internal);
    }
}

static void Port_getSafetyGPIOSCR(uint32_t pin_num, uint32_t * gpio_id)
{
    uint32_t val1;
    uint32_t val1_mux1;

    uint32_t sigReg_id = pin_num / 8;
    uint32_t sigReg_id_mux1 = pin_num / 16;

    const struct scr_signal *signal_mux1 = &scr_safety_mux1_signals[sigReg_id_mux1];
    scr_read_signal(signal_mux1, &val1_mux1);

    if (0 == ((val1_mux1 >> (pin_num % 16)) & 0x1)) {
        *gpio_id = PORT_GPIO_1;
    }
    else {
        const struct scr_signal *signal = &scr_safety_mux_signals[sigReg_id];
        scr_read_signal(signal, &val1);

        *gpio_id = ((val1 >> ((pin_num % 8) *2)) & 0x3) + 2;
    }

    DBG("GPIO id: %d: 0x%x\n", *gpio_id, val1_mux1);
}

static void Port_getSecureGPIOSCR(uint32_t pin_num, uint32_t * gpio_id)
{
    uint32_t val1;
    uint32_t val1_mux2;

    uint32_t sigReg_id;
    uint32_t sigReg_id_mux2;

    uint32_t tmpPinNum = pin_num - 48;

    sigReg_id = tmpPinNum / 8;
    sigReg_id_mux2 = tmpPinNum / 16;

    DBG("Port_SetSecureGPIOSCR: sigReg_id[%d], sigReg_id_mux2[%d]\n", sigReg_id,
        sigReg_id_mux2);

    const struct scr_signal *signal_mux2 = &scr_sec_mux2_signals[sigReg_id_mux2];
    scr_read_signal(signal_mux2, &val1_mux2);

    if (0 == ((val1_mux2 >> (tmpPinNum % 16)) & 0x1)) {
        *gpio_id = PORT_GPIO_1;
    }
    else {
        const struct scr_signal *signal = &scr_sec_mux_signals[sigReg_id];
        scr_read_signal(signal, &val1);

        *gpio_id = ((val1 >> ((tmpPinNum %8 ) *2)) & 0x3) + 2;
    }

    DBG("%s: 0x%x\n", signal_mux2->name, val1_mux2);
}

/* set GPIO SCR based on gpio_id & pin_num(0~155) */
static void Port_getGPIOSCR(uint32_t pin_num, uint32_t * gpio_id)
{

    uint32_t pin_num_internal = 0;
    DBG("Port_SetGPIOSCR: pin_num[%d]\n", pin_num);

    /* safety GPIO CTRL base address index NOT follow PinList */
    pin_num_internal =  (((pin_num) <= 47) ? ((pin_num) >= 24 ? ((
                             pin_num) - 24) : ((pin_num) + 24)) : (pin_num));

    if (pin_num_internal > 47) {
        Port_getSecureGPIOSCR(pin_num_internal, gpio_id);
    }
    else {
        Port_getSafetyGPIOSCR(pin_num_internal, gpio_id);
    }
}

static void Port_ConfigMMC2CANMuxSCR(void)
{
    uint32_t val1;
    uint32_t val2;
    uint32_t i;

    /* enable emmc2/disp pins for gpio/canfd  */
    for (i = 0; i < SIZEOF(scr_mshc2_canfd_signals); i++) {
        const struct scr_signal *signal = &scr_mshc2_canfd_signals[i];

        if (scr_read_signal(signal, &val1)) {
            scr_write_signal(signal,
                             signal->val & BIT_MASK(_scr_width(signal->signal)));
            scr_read_signal(signal, &val2);
            DBG("%s: 0x%x / 0x%x\n", signal->name, val1, val2);
        }
        else {
            DBG("Failed to read signal %s\n", signal->name);
        }
    }
}

LOCAL_INLINE void Port_lIOInit(void)
{
    const uint32_t            *DataPtr;
    const Port_n_ConfigType *ConfigDataPtr;
    /* Each Port Number for the hardware unit */
    uint32_t                   PortNumber;
    /* Each Pin Number for one port */
    uint32_t                   PinNumber;
    /* Index to identify the port configuration information
    from the configuration array  */
    uint32_t                   ConfigIndex;
    vaddr_t                    IOPadAddr;
    vaddr_t                    PinMuxAddr;
    uint32_t                   AltNum;
    uint32_t                   RegVal;
    uint32_t                   gpio_index;

    DBG("Port_lIOInit \n");
    /* Function call to initialize IOMUX registers */
    ConfigIndex = 0U;

    /* writing IO_PAD_CONFIG, PIN_MUX_CONFIG and INPUT_SELECT_SOURCE registers */
    for (PortNumber = (uint32_t)0U; PortNumber <= (uint32_t)PORT_MAX_NUMBER;
            PortNumber++) {

        DBG("Port[%d]\n", PortNumber);

        if (Port_lIsPortAvailable(PortNumber) != (uint32_t)0U) {
            ConfigDataPtr = (Port_kConfigPtr->PortConfigSetPtr) + ConfigIndex ;

            /* Address of each port configuration */
            DataPtr = (const uint32_t *)(const void *)(ConfigDataPtr);

            /* for each pin in one port */
            for (PinNumber = (uint32_t)0U;
                    PinNumber <= (uint32_t)PIN_MAX_NUMBER[PortNumber]; PinNumber++) {
                DBG("Pin[%d]\n", (PortNumber * 32 + PinNumber));

                //to skip safety's pin config (0~47), zhuming, 191111
                // TODO: check wether in Safety or Secure based on BASEADDR, 191127
                if (((run_in_domain == PORT_RUN_IN_SECURE) || (run_in_domain == PORT_RUN_IN_AP)) && (PortNumber * 32 + PinNumber) <= 47)
                    continue;

                /* setup IO_PAD_CONFIG */
                IOPadAddr = getIOPadAddrByPin(PortNumber * 32 + PinNumber);
                iomux_write((uint32_t)(*(DataPtr + PinNumber)), IOPadAddr);

                /* setup PIN_MUX_CONFIG, and INPUT_SOURCE_SELECT if needed */
                PinMuxAddr  = getPinMuxAddrByPin(PortNumber * 32 + PinNumber);
                iomux_write((uint32_t)(*(DataPtr + 32 + PinNumber)), PinMuxAddr);

                /* get MUX_MODE */
                RegVal = (uint32_t)(*(DataPtr + 32 + PinNumber)) & 0x7;

                if (RegVal == PORT_PIN_MODE_GPIO) {
                    /* Non-EMMC pins GPIO Setup */
                    /* GPIO/DIO mode, do DIO Init */

                    gpio_index = (*(DataPtr + 64 + PinNumber)) & 0xffff;
                    if (((uint32_t)(*(DataPtr + 64 + PinNumber)) >> 16) == ((uint32_t)PORT_PIN_IN)) {
                        /* GPIO Input mode */
                        iomux_write(0x00, getGPIOAddr(gpio_index) + GPIO_CTRL_PIN_X(
                                        PortNumber * 32 + PinNumber));
                        DBG("setup GPIO[%d] to input\n", (PortNumber * 32 + PinNumber));
                    }
                    else {
                        if ((uint32_t)(*(DataPtr + 96)) & (0x01 << PinNumber)) {
                            /* GPIO Output High*/
                            iomux_write(0x05, getGPIOAddr(gpio_index) + GPIO_CTRL_PIN_X(
                                           PortNumber * 32 + PinNumber));
                            DBG("setup GPIO[%d] to output high, level 0x%x\n",
                                (PortNumber * 32 + PinNumber), (uint32_t)(*(DataPtr + 96)));
                        }
                        else {
                            /* GPIO Output Low*/
                            iomux_write(0x01, getGPIOAddr(gpio_index) + GPIO_CTRL_PIN_X(
                                       PortNumber * 32 + PinNumber));
                            DBG("setup GPIO[%d] to output low, level 0x%x\n", (PortNumber * 32 + PinNumber),
                                (uint32_t)(*(DataPtr + 96)));
                        }
                    }

                    /* Config GPIO SCR based on GPIO Controller id */
                    Port_SetGPIOSCR((uint32_t)(*(DataPtr + 64 + PinNumber)) & 0xffff,
                                    (PortNumber * 32 + PinNumber));
                 }

                AltNum = (uint32_t)(*(DataPtr + 32 + PinNumber)) & 0x00000007;
                setupInputSourceSelRegByPin(PortNumber * 32 + PinNumber, AltNum);
            }
        }

        ConfigIndex++;
   } /* For loop */
}

void port_init_delta(port_delta_config_t *delta_config, uint32_t config_count)
{
    const port_delta_config_t * dconfig;
    uint32_t pin_number = 0;
    uint32_t pin_level = 0;
    vaddr_t  pad_addr;
    vaddr_t  mux_addr;
    uint32_t alt_num;
    uint32_t reg_val;
    uint32_t gpio_index;

    DBG("port_init_delta \n");

    /* for each pin in one port */
    for (uint32_t i = 0; i < config_count; i++) {
        dconfig = delta_config + i;

        pin_number = (dconfig->misc_config) >> 16;
        pin_level = ((dconfig->misc_config) >> 8) & 0xff;

        DBG("Pin[%d]\n", pin_number);
        if ((run_in_domain == PORT_RUN_IN_SECURE || run_in_domain == PORT_RUN_IN_AP) && (pin_number <= 47))
                continue;

        /* setup IO_PAD_CONFIG */
        pad_addr = getIOPadAddrByPin(pin_number);
        iomux_write(dconfig->pad_config, pad_addr);

        /* setup PIN_MUX_CONFIG, and INPUT_SOURCE_SELECT if needed */
        mux_addr  = getPinMuxAddrByPin(pin_number);
        iomux_write(dconfig->mux_config, mux_addr);

        /* get MUX_MODE */
        reg_val = (dconfig->mux_config) & 0x7;
        gpio_index = (dconfig->gpio_ctrl_config) & 0xffff;
        if (reg_val == PORT_PIN_MODE_GPIO) {
                /* Non-EMMC pins GPIO Setup, GPIO/DIO mode, do DIO Init */
            if (((dconfig->gpio_ctrl_config) >> 16) == ((uint32_t)PORT_PIN_IN)) {
                /* GPIO Input mode */
                iomux_write(0x00, getGPIOAddr(gpio_index) + GPIO_CTRL_PIN_X(pin_number));
                DBG("setup GPIO[%d] to input\n", (pin_number));
            }
            else {
                if (pin_level & 0x01) {
                    /* GPIO Output High*/
                    iomux_write(0x05, getGPIOAddr(gpio_index) + GPIO_CTRL_PIN_X(pin_number));

                    DBG("setup GPIO[%d] to output high, level 0x%x\n", pin_number, pin_level);
                }
                else {
                    /* GPIO Output Low*/
                    iomux_write(0x01, getGPIOAddr(gpio_index) + GPIO_CTRL_PIN_X(pin_number));

                    DBG("setup GPIO[%d] to output low, level 0x%x\n", pin_number, pin_number);
                }
            }
            /* Config GPIO SCR based on GPIO Controller id */
            Port_SetGPIOSCR((dconfig->gpio_ctrl_config) & 0xffff, pin_number);
        }

        alt_num = (dconfig->mux_config) & 0x7;
        setupInputSourceSelRegByPin(pin_number, alt_num);
    }
}

static void write_reg(uint32_t *addr , uint32_t val)
{
    *addr = val;
}

int port_set_pin_data(Port_PinModeType *pin_mode, const Port_PinType pin_num, int32_t data)
{
    if (pin_num >= PORT_PIN_CNT) {
        dprintf(CRITICAL, "port_set_pin_data pin_num out of range\n");
        return -1;
    }

    uint32_t gpio_ctrl = 0;

    if (PORT_PIN_MODE_GPIO == ((pin_mode->pin_mux_config) & 0x7)) {
        Port_getGPIOSCR(pin_num, &gpio_ctrl);

        vaddr_t gpio_addr = getGPIOAddr(gpio_ctrl) + GPIO_CTRL_PIN_X(pin_num);

        uint32_t data_reg = iomux_read(gpio_addr);
        dprintf(ALWAYS, "port_set_pin_data get data_reg=0x%x\n", data_reg);

        data_reg &= ~0x04;
        data_reg |= ((data & 0x01) << 2);
	
        write_reg((uint32_t *)gpio_addr, data_reg);

        dprintf(ALWAYS, "port_set_pin_data addr: 0x%lx, value: 0x%x\n", gpio_addr, data_reg);
    }

    return 0;
}


/*
 * function for getting pin info
 * pin_num - pin index
 * pin_mode - pin pad/iomux configuration
 * gpio_ctrl - gpio controller index, valid for GPIO
 * direction - gpio input/output configuration, valid for GPIO
 * level - default level, valid for GPIO
*/
int port_get_pin_info(const Port_PinType pin_num, Port_PinModeType *pin_mode, uint32_t * input_select, uint32_t * gpio_ctrl, int32_t * gpio_config)
{
    vaddr_t pad_addr;
    vaddr_t  mux_addr;

    if (pin_num >= PORT_PIN_CNT) {
        dprintf(CRITICAL, "pin_num out of range\n");
        return -1;
    }

    *gpio_ctrl = 0;
    *gpio_config = -1;

    if (((run_in_domain == PORT_RUN_IN_SECURE) || (run_in_domain == PORT_RUN_IN_AP)) && (pin_num <= 47)) {
        DBG("can't get pin info in secure or ap domain.\n");
        return -1;
    }

    pad_addr = getIOPadAddrByPin(pin_num);
    pin_mode->io_pad_config = iomux_read(pad_addr);
	dprintf(ALWAYS, "pin[%d], pad addr: 0x%lx, value: 0x%x\n", pin_num, pad_addr, pin_mode->io_pad_config);

    mux_addr  = getPinMuxAddrByPin(pin_num);
    pin_mode->pin_mux_config = iomux_read(mux_addr);
    dprintf(ALWAYS, "mux addr: 0x%lx, value: 0x%x\n", mux_addr, pin_mode->pin_mux_config);

    getInputSourceSelRegByPin(pin_num, ((pin_mode->pin_mux_config) & 0x7), input_select);

    if (PORT_PIN_MODE_GPIO == ((pin_mode->pin_mux_config) & 0x7)) {
        //get gpio controller info
        Port_getGPIOSCR(pin_num, gpio_ctrl);

        if (((run_in_domain == PORT_RUN_IN_SECURE) || (run_in_domain == PORT_RUN_IN_AP))
            && (PORT_GPIO_1 == *gpio_ctrl)) {
            return 0;
        }

        vaddr_t gpio_addr = getGPIOAddr(*gpio_ctrl) + GPIO_CTRL_PIN_X(pin_num);
        *gpio_config = iomux_read(gpio_addr);

        dprintf(ALWAYS, "gpio addr: 0x%lx, value: 0x%x\n", gpio_addr, *gpio_config);
    }

    return 0;
}

LOCAL_INLINE uint32_t Port_lIsPortAvailable(const uint32_t Port)
{
    uint32_t RetVal;

    RetVal = ( ((uint32_t)(PORT_CONSTANT_0x01) << (Port)) &
               ((uint32_t)PORTS_AVAILABLE_00_04)
             );
    return (RetVal);
}

LOCAL_INLINE vaddr_t getGPIOBaseAddr(void)
{
    return g_port_handle.dio_phy_addr;
}

#define GPIO1_GPIO2_ADDR_GAP 0x400000
#define GPIO_ADDR_GAP 0x10000

LOCAL_INLINE vaddr_t getGPIOAddr(uint32_t gpio_index)
{
    vaddr_t gpio_addr;

    if (gpio_index == (uint32_t)g_port_handle.dio_real_idx) {
        return getGPIOBaseAddr();
    }

    if (PORT_GPIO_1 == gpio_index) {
        gpio_addr = getGPIOBaseAddr() - GPIO1_GPIO2_ADDR_GAP
                - (g_port_handle.dio_real_idx - PORT_GPIO_2) * GPIO_ADDR_GAP;
    }
    else if (PORT_GPIO_1 == g_port_handle.dio_real_idx) {
        gpio_addr = getGPIOBaseAddr() + GPIO1_GPIO2_ADDR_GAP
                + (gpio_index - PORT_GPIO_2) * GPIO_ADDR_GAP;
    }
    else {
        gpio_addr = getGPIOBaseAddr() + (gpio_index - g_port_handle.dio_real_idx) * GPIO_ADDR_GAP;
    }

    return gpio_addr;
}

void port_init_disp_canfd_mux(const port_disp_canfd_mux_t mux)
{
    uint32_t val = 1;
    uint32_t val1;
    uint32_t val2;
    uint32_t i;

    if (PORT_MUX_DISP == mux) {
        val = 0;
    }

    /* enable pin108-131 for disp/canfd, 0 for disp, 1 for cnafd */
    for (i = 1; i < SIZEOF(scr_mshc2_canfd_signals); i++) {
        const struct scr_signal *signal = &scr_mshc2_canfd_signals[i];

        if (scr_read_signal(signal, &val1)) {
            scr_write_signal(signal,
                             val & BIT_MASK(_scr_width(signal->signal)));
            scr_read_signal(signal, &val2);
            DBG("%s: 0x%x / 0x%x\n", signal->name, val1, val2);
        }
        else {
            DBG("Failed to read signal %s\n", signal->name);
        }
    }
}
