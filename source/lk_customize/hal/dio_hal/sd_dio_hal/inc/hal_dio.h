//*****************************************************************************
//
// hal_dio.h - Prototypes for the dio hal
//
// Copyright (c) 2019 Semidrive Semiconductor.  All rights reserved.
// Software License Agreement
//
//
//*****************************************************************************

#ifndef __DIO_HAL_H__
#define __DIO_HAL_H__
//*****************************************************************************
//
// If building with a C++ compiler, make all of the definitions in this header
// have a C binding.
//
//*****************************************************************************
#ifdef __cplusplus
extern "C" {
#endif

#include <kernel/mutex.h>
#include <platform/interrupts.h>
#include "chip_res.h"
#include "res.h"
#include "system_cfg.h"
#include "Dio.h"

#define IRQ_TYPE_NONE         DIO_IRQ_TYPE_NONE
#define IRQ_TYPE_EDGE_RISING  DIO_IRQ_TYPE_EDGE_RISING
#define IRQ_TYPE_EDGE_FALLING DIO_IRQ_TYPE_EDGE_FALLING
#define IRQ_TYPE_EDGE_BOTH    DIO_IRQ_TYPE_EDGE_BOTH
#define IRQ_TYPE_LEVEL_HIGH   DIO_IRQ_TYPE_LEVEL_HIGH
#define IRQ_TYPE_LEVEL_LOW    DIO_IRQ_TYPE_LEVEL_LOW

void hal_dio_enable_irq(const Dio_ChannelType ChannelId);
void hal_dio_disable_irq(const Dio_ChannelType ChannelId, int irqflag);
bool hal_dio_config_irq(const Dio_ChannelType ChannelId, int irqflag);
bool hal_dio_get_irq_status(const Dio_ChannelType ChannelId);

/*
 * Function: dio creat handle api
 * Arg     : dio device handle, dio id
 * Return  : true on Success, false on failed
 */
bool hal_dio_creat_handle(void **handle, uint32_t dio_res_glb_idx);

/*
 * Function: dio release handle api
 * Arg     : dio device handle
 * Return  : true on Success, false on failed
 */
bool hal_dio_release_handle(void **handle);


/*
 * Function: dio read one channel
 * Arg     : dio device handle, channel id
 * Return  : channel's level
 */
Dio_LevelType hal_dio_read_channel(void *handle,
                                   const Dio_ChannelType ChannelId);

/*
 * Function: dio write one channel
 * Arg     : dio device handle, channel id, level be written
 * Return  : none
 */
void hal_dio_write_channel(void *handle, const Dio_ChannelType ChannelId,
                           const Dio_LevelType Level);

/*
 * Function: dio read one port
 * Arg     : dio device handle, port id
 * Return  : port level
 */
Dio_PortLevelType hal_dio_read_port(void *handle,
                                    const Dio_PortType PortId);

/*
 * Function: dio write one port
 * Arg     : dio device handle, port id, port's level to be written
 * Return  : none
 */
void hal_dio_write_port(void *handle, const Dio_PortType PortId,
                        const Dio_PortLevelType Level);

/*
 * Function: dio read channel group
 * Arg     : dio device handle, channel group id pointer
 * Return  : channel group level
 */
Dio_PortLevelType hal_dio_read_channel_group(void *handle,
        const Dio_ChannelGroupType *const ChannelGroupIdPtr);

/*
 * Function: dio write channel group
 * Arg     : dio device handle, channel group id pointer, level
 * Return  : none
 */
void hal_dio_write_channel_group(void *handle,
                                 const Dio_ChannelGroupType  *const ChannelGroupIdPtr,
                                 const Dio_PortLevelType Level);

/*
 * Function: dio flip channel
 * Arg     : dio device handle, channel id
 * Return  : flipped channel's level
 */
Dio_LevelType hal_dio_flip_channel(void *handle,
                                   const Dio_ChannelType ChannelId);

/*
 * Function: dio set pin direction
 * Arg     : dio device handle, pin number, direction
 * Return  : 0 on Success, non zero on failure
 */
void hal_dio_masked_write_port(void *handle, Dio_PortType PortId,
                               Dio_PortLevelType Level, Dio_PortLevelType Mask);

/*
 * Function: dio set channel direction
 * Arg     : dio device handle, channel number, direction
 * Return  : 0 on Success, non zero on failure
 */
int hal_dio_set_channel_direction(void *handle, const Dio_ChannelType ChannelId,
                               const Dio_ChannelDirectionType direction);
//*****************************************************************************
//
// Mark the end of the C bindings section for C++ compilers.
//
//*****************************************************************************
#ifdef __cplusplus
}
#endif
#endif // __DIO_HAL_H__
