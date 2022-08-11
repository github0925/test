

#ifndef DIO_H
#define DIO_H
/*******************************************************************************
**                      Include Section                                       **
*******************************************************************************/

#include <stdint.h>
#include <reg.h>
#include <stdio.h>
#include <kernel/vm.h>

/* Inclusion of Platform_Types.h and Compiler.h */
#include "Std_Types.h"

/* Pre-compile/static configuration parameters for Dio  */
#include "Dio_Cfg.h"


/*******************************************************************************
**                      Global Macro Definitions                              **
*******************************************************************************/

typedef uintptr_t addr_t;
typedef uintptr_t vaddr_t;
typedef uintptr_t paddr_t;


#define LOCAL_INLINE    static inline

/* DIO MODULE INSTANCE ID */
#define DIO_INSTANCE_ID              ((uint8_t)0U)

/*Macros to inform the Dio port is configured or not*/
#define DIO_PORT_NOT_CONFIGURED      ((uint8_t)0U)
#define DIO_PORT_CONFIGURED          ((uint8_t)1U)
/*******************************************************************************
**                      Global Type Definitions                               **
*******************************************************************************/

/*Type definition for numeric id of Dio Channels*/
typedef uint16_t Dio_ChannelType;

/*Type definition for possible levels that a Dio Channel can have*/
typedef uint8_t Dio_LevelType;

/*Type definition for numeric id of Dio Port*/
typedef uint8_t Dio_PortType;

/*Type definition for level of a Dio Port */
typedef uint32_t Dio_PortLevelType;

/*Type definition of a Dio Channel Group*/
typedef struct {
    /*Defines the Position of Channel Group*/
    Dio_PortLevelType       mask;

    /*Defines the position of group from LSB*/
    uint8_t        offset;

    /*Port Number on which Channel Group is defined*/
    Dio_PortType port;
} Dio_ChannelGroupType;

struct dio_handle {
    paddr_t phy_addr;
    int32_t real_idx;
};

typedef enum {
    DIO_CHANNEL_IN = 0x0U,
    DIO_CHANNEL_OUT = 0x1U
} Dio_ChannelDirectionType;

/*******************************************************************************
**                      Global Constant Declarations                          **
*******************************************************************************/
/*******************************************************************************
**                      Global Variable Declarations                          **
*******************************************************************************/

/*******************************************************************************
**                      Global Function Declarations                          **
*******************************************************************************/

#define DIO_IRQ_TYPE_NONE           0
#define DIO_IRQ_TYPE_EDGE_RISING    1
#define DIO_IRQ_TYPE_EDGE_FALLING   2
#define DIO_IRQ_TYPE_EDGE_BOTH      (DIO_IRQ_TYPE_EDGE_FALLING | DIO_IRQ_TYPE_EDGE_RISING)
#define DIO_IRQ_TYPE_LEVEL_HIGH     4
#define DIO_IRQ_TYPE_LEVEL_LOW      8

extern void Dio_enable_irq(const Dio_ChannelType ChannelId);
extern void Dio_disable_irq(const Dio_ChannelType ChannelId, int irqflag);
extern void Dio_config_irq(const Dio_ChannelType ChannelId, int irqflag);
extern bool Dio_get_irq_status(const Dio_ChannelType ChannelId);

extern Dio_LevelType Dio_ReadChannel
(
    const Dio_ChannelType ChannelId
);

extern void Dio_WriteChannel
(
    const Dio_ChannelType ChannelId,
    const Dio_LevelType Level
);

extern Dio_PortLevelType Dio_ReadPort
(
    const Dio_PortType PortId
);

extern void Dio_WritePort
(
    const Dio_PortType PortId,
    const Dio_PortLevelType Level
);

extern Dio_PortLevelType Dio_ReadChannelGroup
(
    const Dio_ChannelGroupType *const ChannelGroupIdPtr
);

extern void Dio_WriteChannelGroup
(
    const Dio_ChannelGroupType *const ChannelGroupIdPtr,
    const Dio_PortLevelType Level
);

extern Dio_LevelType Dio_FlipChannel
(
    const Dio_ChannelType ChannelId
);

extern void Dio_GetVersionInfo
(
    Std_VersionInfoType *const VersionInfo
);

extern void Dio_MaskedWritePort
(
    Dio_PortType PortId,
    Dio_PortLevelType Level,
    Dio_PortLevelType Mask
);

void Dio_SetChannelDirection
(
    Dio_ChannelType ChannelId,
    Dio_ChannelDirectionType direction
);

extern void Dio_SetHandle(
    void *handle
);
/*******************************************************************************
**                      Global Inline Function Definitions                    **
*******************************************************************************/


#endif   /*  DIO_H  */
