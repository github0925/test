

/*******************************************************************************
**                      Include Section                                       **
*******************************************************************************/
#include <debug.h>

/* Own header file, this includes own configuration file also */
#include "Dio.h"

/*******************************************************************************
**                      Private Object Like Macro Definitions                 **
*******************************************************************************/


//#define SEC_APB_GPIO2_BASE (0xf0400000u)
//#define GPIO_BASE_ADDR SEC_APB_GPIO2_BASE
#define GPIO_BASE_ADDR (g_dio_handle.phy_addr)

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

// register each bit define
#define GPIO_CTRL_DIR_BIT               0
#define GPIO_CTRL_DATA_IN_BIT           1
#define GPIO_CTRL_DATA_OUT_BIT          2
#define GPIO_CTRL_INT_EN_BIT            3
#define GPIO_CTRL_INT_MASK_BIT          4
#define GPIO_CTRL_INT_TYPE_BIT          5
#define GPIO_CTRL_INT_POL_BIT           6
#define GPIO_CTRL_INT_BOTH_EDGE_BIT     7
#define GPIO_CTRL_INT_STATUS_BIT        8
#define GPIO_CTRL_INT_STATUS_UNMASK_BIT 9
#define GPIO_CTRL_INT_EDGE_CLR_BIT      10
#define GPIO_CTRL_INT_LEV_SYNC_BIT      11
#define GPIO_CTRL_INT_DEB_EN_BIT        12
#define GPIO_CTRL_INT_CLK_SEL_BIT       13
#define GPIO_CTRL_PCLK_ACTIVE_BIT       14

/* port operation registers */
// n: 0 ~4
#define GPIO_DIR_PORT_1     0x2000
#define GPIO_DIR_PORT_2     0x2010
#define GPIO_DIR_PORT_SIZE \
    (GPIO_DIR_PORT_2 - GPIO_DIR_PORT_1)
#define GPIO_DIR_PORT_X(n) \
    (GPIO_DIR_PORT_1 + ((n) * GPIO_DIR_PORT_SIZE))

#define GPIO_DATA_IN_PORT_1     0x2200
#define GPIO_DATA_IN_PORT_2     0x2210
#define GPIO_DATA_IN_PORT_SIZE \
    (GPIO_DATA_IN_PORT_2 - GPIO_DATA_IN_PORT_1)
#define GPIO_DATA_IN_PORT_X(n) \
    (GPIO_DATA_IN_PORT_1 + ((n) * GPIO_DATA_IN_PORT_SIZE))

#define GPIO_DATA_OUT_PORT_1        0x2400
#define GPIO_DATA_OUT_PORT_2        0x2410
#define GPIO_DATA_OUT_PORT_SIZE \
    (GPIO_DATA_OUT_PORT_2 - GPIO_DATA_OUT_PORT_1)
#define GPIO_DATA_OUT_PORT_X(n) \
    (GPIO_DATA_OUT_PORT_1 + ((n) * GPIO_DATA_OUT_PORT_SIZE))

#define GPIO_INT_EN_PORT_1      0x2600
#define GPIO_INT_EN_PORT_2      0x2610
#define GPIO_INT_EN_PORT_SIZE \
    (GPIO_INT_EN_PORT_2 - GPIO_INT_EN_PORT_1)
#define GPIO_INT_EN_PORT_X(n) \
    (GPIO_INT_EN_PORT_1 + ((n) * GPIO_INT_EN_PORT_SIZE))

#define GPIO_INT_MASK_PORT_1        0x2800
#define GPIO_INT_MASK_PORT_2        0x2810
#define GPIO_INT_MASK_PORT_SIZE \
    (GPIO_INT_MASK_PORT_2 - GPIO_INT_MASK_PORT_1)
#define GPIO_INT_MASK_PORT_X(n) \
    (GPIO_INT_MASK_PORT_1 + ((n) * GPIO_INT_MASK_PORT_SIZE))

#define GPIO_INT_TYPE_PORT_1        0x2a00
#define GPIO_INT_TYPE_PORT_2        0x2a10
#define GPIO_INT_TYPE_PORT_SIZE \
    (GPIO_INT_TYPE_PORT_2 - GPIO_INT_TYPE_PORT_1)
#define GPIO_INT_TYPE_PORT_X(n) \
    (GPIO_INT_TYPE_PORT_1 + ((n) * GPIO_INT_TYPE_PORT_SIZE))

#define GPIO_INT_POL_PORT_1     0x2c00
#define GPIO_INT_POL_PORT_2     0x2c10
#define GPIO_INT_POL_PORT_SIZE \
    (GPIO_INT_POL_PORT_2 - GPIO_INT_POL_PORT_1)
#define GPIO_INT_POL_PORT_X(n) \
    (GPIO_INT_POL_PORT_1 + ((n) * GPIO_INT_POL_PORT_SIZE))

#define GPIO_INT_BOTH_EDGE_PORT_1       0x2e00
#define GPIO_INT_BOTH_EDGE_PORT_2       0x2e10
#define GPIO_INT_BOTH_EDGE_PORT_SIZE \
    (GPIO_INT_BOTH_EDGE_PORT_2 - GPIO_INT_BOTH_EDGE_PORT_1)
#define GPIO_INT_BOTH_EDGE_PORT_X(n) \
    (GPIO_INT_BOTH_EDGE_PORT_1 + ((n) * GPIO_INT_BOTH_EDGE_PORT_SIZE))

#define GPIO_INT_STATUS_PORT_1      0x3000
#define GPIO_INT_STATUS_PORT_2      0x3010
#define GPIO_INT_STATUS_PORT_SIZE \
    (GPIO_INT_STATUS_PORT_2 - GPIO_INT_STATUS_PORT_1)
#define GPIO_INT_STATUS_PORT_X(n) \
    (GPIO_INT_STATUS_PORT_1 + ((n) * GPIO_INT_STATUS_PORT_SIZE))

#define GPIO_INT_STATUS_UNMASK_PORT_1       0x3200
#define GPIO_INT_STATUS_UNMASK_PORT_2       0x3210
#define GPIO_INT_STATUS_UNMASK_PORT_SIZE \
    (GPIO_INT_STATUS_UNMASK_PORT_2 - GPIO_INT_STATUS_UNMASK_PORT_1)
#define GPIO_INT_STATUS_UNMASK_PORT_X(n) \
    (GPIO_INT_STATUS_UNMASK_PORT_1 + \
    ((n) * GPIO_INT_STATUS_UNMASK_PORT_SIZE))

#define GPIO_INT_EDGE_CLR_PORT_1        0x3400
#define GPIO_INT_EDGE_CLR_PORT_2        0x3410
#define GPIO_INT_EDGE_CLR_PORT_SIZE \
    (GPIO_INT_EDGE_CLR_PORT_2 - GPIO_INT_EDGE_CLR_PORT_1)
#define GPIO_INT_EDGE_CLR_PORT_X(n) \
    (GPIO_INT_EDGE_CLR_PORT_1 + ((n) * GPIO_INT_EDGE_CLR_PORT_SIZE))

#define GPIO_INT_LEV_SYNC_PORT_1        0x3600
#define GPIO_INT_LEV_SYNC_PORT_2        0x3610
#define GPIO_INT_LEV_SYNC_PORT_SIZE \
    (GPIO_INT_LEV_SYNC_PORT_2 - GPIO_INT_LEV_SYNC_PORT_1)
#define GPIO_INT_LEV_SYNC_PORT_X(n) \
    (GPIO_INT_LEV_SYNC_PORT_1 + ((n) * GPIO_INT_LEV_SYNC_PORT_SIZE))

#define GPIO_INT_DEB_EN_PORT_1      0x3800
#define GPIO_INT_DEB_EN_PORT_2      0x3810
#define GPIO_INT_DEB_EN_PORT_SIZE \
    (GPIO_INT_DEB_EN_PORT_2 - GPIO_INT_DEB_EN_PORT_1)
#define GPIO_INT_DEB_EN_PORT_X(n) \
    (GPIO_INT_DEB_EN_PORT_1 + ((n) * GPIO_INT_DEB_EN_PORT_SIZE))

#define GPIO_INT_CLK_SEL_PORT_1     0x3a00
#define GPIO_INT_CLK_SEL_PORT_2     0x3a10
#define GPIO_INT_CLK_SEL_PORT_SIZE \
    (GPIO_INT_CLK_SEL_PORT_2 - GPIO_INT_CLK_SEL_PORT_1)
#define GPIO_INT_CLK_SEL_PORT_X(n) \
    (GPIO_INT_CLK_SEL_PORT_1 + ((n) * GPIO_INT_CLK_SEL_PORT_SIZE))

#define GPIO_INT_PCLK_ACTIVE_PORT_1     0x3c00
#define GPIO_INT_PCLK_ACTIVE_PORT_2     0x3c10
#define GPIO_INT_PCLK_ACTIVE_PORT_SIZE \
    (GPIO_INT_PCLK_ACTIVE_PORT_2 - GPIO_INT_PCLK_ACTIVE_PORT_1)
#define GPIO_INT_PCLK_ACTIVE_PORT_X(n) \
    (GPIO_INT_PCLK_ACTIVE_PORT_1 + ((n) * GPIO_INT_PCLK_ACTIVE_PORT_SIZE))

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

static struct dio_handle g_dio_handle;

/*******************************************************************************
**                      Private Constant Definitions                          **
*******************************************************************************/

/*******************************************************************************
**                      Private Variable Definitions                          **
*******************************************************************************/


/*******************************************************************************
**                      Private Function Declarations                         **
*******************************************************************************/

LOCAL_INLINE void gpio_write(uint32_t value, uint32_t paddr);

LOCAL_INLINE uint32_t gpio_read(uint32_t paddr);

LOCAL_INLINE void set_pin(int pinnum, int val);
/*******************************************************************************
**                      Global Function Definitions                           **
*******************************************************************************/

void Dio_enable_irq(const Dio_ChannelType ChannelId)
{
    gpio_write((0x1 << GPIO_CTRL_INT_EN_BIT),
               GPIO_BASE_ADDR + GPIO_SET_PIN_X(ChannelId));
}

void Dio_disable_irq(const Dio_ChannelType ChannelId, int irqflag)
{
    if (irqflag == DIO_IRQ_TYPE_EDGE_RISING ||
            irqflag == DIO_IRQ_TYPE_EDGE_FALLING ||
            irqflag == DIO_IRQ_TYPE_EDGE_BOTH)
        gpio_write((0x1 << GPIO_CTRL_INT_EDGE_CLR_BIT),
                   GPIO_BASE_ADDR + GPIO_SET_PIN_X(ChannelId));

    gpio_write((0x1 << GPIO_CTRL_INT_EN_BIT),
               GPIO_BASE_ADDR + GPIO_CLEAR_PIN_X(ChannelId));
}

void Dio_config_irq(const Dio_ChannelType ChannelId, int irqflag)
{
    if (irqflag == DIO_IRQ_TYPE_EDGE_RISING) {
        gpio_write((0x1 << GPIO_CTRL_INT_TYPE_BIT),
                   GPIO_BASE_ADDR + GPIO_SET_PIN_X(ChannelId));
        gpio_write((0x1 << GPIO_CTRL_INT_POL_BIT),
                   GPIO_BASE_ADDR + GPIO_SET_PIN_X(ChannelId));
    }
    else if (irqflag == DIO_IRQ_TYPE_EDGE_FALLING) {
        gpio_write((0x1 << GPIO_CTRL_INT_TYPE_BIT),
                   GPIO_BASE_ADDR + GPIO_SET_PIN_X(ChannelId));
    }
    else if (irqflag == DIO_IRQ_TYPE_EDGE_BOTH) {
        gpio_write((0x1 << GPIO_CTRL_INT_TYPE_BIT),
                   GPIO_BASE_ADDR + GPIO_SET_PIN_X(ChannelId));
        gpio_write((0x1 << GPIO_CTRL_INT_BOTH_EDGE_BIT),
                   GPIO_BASE_ADDR + GPIO_SET_PIN_X(ChannelId));
    }
    else if (irqflag == DIO_IRQ_TYPE_LEVEL_HIGH) {
        gpio_write((0x1 << GPIO_CTRL_INT_POL_BIT),
                   GPIO_BASE_ADDR + GPIO_SET_PIN_X(ChannelId));
    }
    else if (irqflag == DIO_IRQ_TYPE_LEVEL_LOW) {
        //default level low trigger
    }
}

bool Dio_get_irq_status(const Dio_ChannelType ChannelId)
{
    uint16_t regval = 0;
    regval = gpio_read(GPIO_BASE_ADDR + GPIO_CTRL_PIN_X(ChannelId));
    return regval & (0x1 << GPIO_CTRL_INT_STATUS_BIT) ? true : false;
}

Dio_LevelType Dio_ReadChannel(const Dio_ChannelType ChannelId)
{
    uint32_t         RegisterVal;
    Dio_LevelType  RetVal;

    /* Return value should be zero for errors*/
    RetVal = (Dio_LevelType)STD_LOW;

    // Dio setup in Port_init()
    // TODO: check the ChannelId valid or not for read/write ?

    /* read data in */
    RegisterVal = gpio_read(GPIO_BASE_ADDR + GPIO_CTRL_PIN_X(ChannelId));

    /* Read the Channel level and decide the return value */
    if (RegisterVal & (uint32_t)(0x01 << GPIO_CTRL_DATA_IN_BIT)) {
        RetVal = (Dio_LevelType)STD_HIGH;
    }

    return RetVal;
}/* Dio_ReadChannel */

void Dio_WriteChannel(const Dio_ChannelType ChannelId,
                      const Dio_LevelType Level)
{

    if (Level & 0x01) {
        // output 1
        gpio_write((0x01 << GPIO_CTRL_DATA_OUT_BIT),
                   GPIO_BASE_ADDR + GPIO_SET_PIN_X(ChannelId));
    }
    else {
        // outout 0
        gpio_write((0x01 << GPIO_CTRL_DATA_OUT_BIT),
                   GPIO_BASE_ADDR + GPIO_CLEAR_PIN_X(ChannelId));
    }

}/* Dio_WriteChannel */

Dio_PortLevelType Dio_ReadPort(const Dio_PortType PortId)
{

    Dio_PortLevelType  RetVal;

    /* read the Port data in */
    RetVal = (Dio_PortLevelType)gpio_read(GPIO_BASE_ADDR + GPIO_DATA_IN_PORT_X(
            PortId));

    return (RetVal);
}/* Dio_ReadPort */

void Dio_WritePort (const Dio_PortType PortId,
                    const  Dio_PortLevelType Level)
{
    uint32_t         RegisterVal = 0;

    /* write the data out */
    gpio_write(Level, GPIO_BASE_ADDR + GPIO_DATA_OUT_PORT_X(PortId));

    RegisterVal = gpio_read(GPIO_BASE_ADDR + GPIO_DATA_OUT_PORT_X(PortId));
    dprintf(INFO, "+++Dio_WritePort Readback Port[%d],DATA_OUT[0x%x]\n", PortId,
           RegisterVal);
}/* Dio_WritePort */

Dio_PortLevelType Dio_ReadChannelGroup
( const Dio_ChannelGroupType *const ChannelGroupIdPtr)
{
    Dio_PortLevelType  RetVal;
    uint32_t         RegisterVal = 0;

    /* read ChannelGroup data in */
    RegisterVal = gpio_read(GPIO_BASE_ADDR + GPIO_DATA_IN_PORT_X(
                                ChannelGroupIdPtr->port));
    RetVal = (Dio_PortLevelType)((uint32_t)RegisterVal &
                                 (uint32_t)ChannelGroupIdPtr->mask);

    RetVal = (RetVal >> ChannelGroupIdPtr->offset);

    return (RetVal);
} /* Dio_ReadChannelGroup */

void Dio_WriteChannelGroup
( const Dio_ChannelGroupType *const ChannelGroupIdPtr,
  const Dio_PortLevelType Level)
{
    uint32_t          PortVal;
    uint32_t         RegisterVal = 0;

  PortVal = (uint32_t)(((uint32_t)Level << (uint32_t)ChannelGroupIdPtr->offset) & \
                       (uint32_t) ChannelGroupIdPtr->mask);


    dprintf(INFO, "+++Dio_WriteChannelGroup offset[0x%x], mask[0x%x], port[%d]\n",
           (uint32_t)ChannelGroupIdPtr->offset, (uint32_t) ChannelGroupIdPtr->mask,
           ChannelGroupIdPtr->port);

    /* read the DATA out first */
    RegisterVal = gpio_read(GPIO_BASE_ADDR + GPIO_DATA_OUT_PORT_X(
                                ChannelGroupIdPtr->port));  // Data out used
    dprintf(INFO, "+++Dio_WriteChannelGroup Level[0x%x],PortVal[0x%x],DATAOUT[0x%x]\n",
           Level, PortVal, RegisterVal);

    RegisterVal &= (~((uint32_t) ChannelGroupIdPtr->mask));
    dprintf(INFO, "+++Dio_WriteChannelGroup RegisterVal_Masked[0x%x]\n", RegisterVal);

    /* write the ChannelGroup data out */
    gpio_write((RegisterVal | PortVal),
               GPIO_BASE_ADDR + GPIO_DATA_OUT_PORT_X(ChannelGroupIdPtr->port));

} /* Dio_WriteChannelGroup */

Dio_LevelType Dio_FlipChannel(const Dio_ChannelType ChannelId)
{
    Dio_LevelType    RetVal;
    uint32_t         RegisterVal = 0;

    /* flip the Channel */
    gpio_write((0x01 << GPIO_CTRL_DATA_OUT_BIT),
               GPIO_BASE_ADDR + GPIO_TOGGLE_PIN_X(ChannelId));

    /* read the data in flipped */
    RegisterVal = gpio_read(GPIO_BASE_ADDR +
		GPIO_CTRL_PIN_X(ChannelId)) & (0x01 << GPIO_CTRL_DATA_OUT_BIT);
    RetVal = (Dio_LevelType)(RegisterVal >> GPIO_CTRL_DATA_OUT_BIT);

    return RetVal;
}/* Dio_FlipChannel */

void Dio_GetVersionInfo(Std_VersionInfoType *const VersionInfo)
{
    {
        /* Vendor ID information */
        ((Std_VersionInfoType *)(VersionInfo))->vendorID = 0;
        /*DIO module ID information */
        ((Std_VersionInfoType *)(VersionInfo))->moduleID = 0;
        /*DIO Instance ID information */
        /* DIO module Software major version information */
        ((Std_VersionInfoType *)(VersionInfo))->sw_major_version =
            (uint8_t)DIO_SW_MAJOR_VERSION;
        /* DIO module Software minor version information */
        ((Std_VersionInfoType *)(VersionInfo))->sw_minor_version =
            (uint8_t)DIO_SW_MINOR_VERSION;
        /* DIO module Software patch version information */
        ((Std_VersionInfoType *)(VersionInfo))->sw_patch_version =
            (uint8_t)DIO_SW_PATCH_VERSION;
    }
}


void Dio_MaskedWritePort(Dio_PortType PortId, Dio_PortLevelType Level,
                         Dio_PortLevelType Mask)
{
    uint32_t         RegisterVal = 0;

    /* read the DATA out first, and clear Masked bits */
    RegisterVal = gpio_read(GPIO_BASE_ADDR + GPIO_DATA_OUT_PORT_X(PortId));
    dprintf(INFO, "+++Dio_MaskedWritePort port[%d],Level[0x%x],Mask[0x%x],DATAOUT[0x%x]\n",
           PortId, Level, Mask, RegisterVal);

    RegisterVal &= (~Mask);
    dprintf(INFO, "+++Dio_MaskedWritePort RegisterVal_Masked[0x%x]\n", RegisterVal);

    /* write the ChannelGroup data out */
    gpio_write((RegisterVal | (Level & Mask)),
               GPIO_BASE_ADDR + GPIO_DATA_OUT_PORT_X(PortId));
}

void Dio_SetChannelDirection(Dio_ChannelType ChannelId, Dio_ChannelDirectionType direction)
{
    uint32_t val = gpio_read(GPIO_BASE_ADDR + GPIO_CTRL_PIN_X(ChannelId));
    gpio_write((val & 0xfffffffe) | direction, GPIO_BASE_ADDR + GPIO_SET_PIN_X(ChannelId));
}

void Dio_SetHandle(void *handle)
{
    struct dio_handle *p_handle;

    if (handle != NULL) {
        p_handle = (struct dio_handle *)handle;

        g_dio_handle.phy_addr = p_handle->phy_addr;
        g_dio_handle.real_idx = p_handle->real_idx;
        dprintf(INFO, "Dio_SetHandle: phy_addr[0x%lx], idx[%d]\n", g_dio_handle.phy_addr,
               g_dio_handle.real_idx);
    }
}
/*******************************************************************************
**                      Private Function Definitions                          **
*******************************************************************************/

LOCAL_INLINE void gpio_write(uint32_t value, uint32_t paddr)
{
#ifdef WITH_KERNEL_VM
    vaddr_t        vaddr = (vaddr_t)paddr_to_kvaddr(paddr);
#else
    vaddr_t        vaddr = (vaddr_t)paddr;
#endif
    dprintf(INFO, "gpio_write:  value[0x%x], vaddr[0x%lx]\n", value, vaddr);
    writel(value, vaddr);
}

LOCAL_INLINE uint32_t gpio_read(uint32_t paddr)
{
    uint32_t value;
#ifdef WITH_KERNEL_VM
    vaddr_t        vaddr = (vaddr_t)paddr_to_kvaddr(paddr);
#else
    vaddr_t        vaddr = (vaddr_t)paddr;
#endif
    value = readl(vaddr);
    dprintf(INFO, "gpio_read:  value[0x%x], vaddr[0x%lx]\n", value, vaddr);
    return value;
}

/***************** End of Dio driver module **********************************/
