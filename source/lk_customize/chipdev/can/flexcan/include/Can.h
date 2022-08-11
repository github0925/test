#ifndef CAN_H_
#define CAN_H_

#ifdef SUPPORT_3RD_ERPC
#include "vcan.h"
#endif
#include "../flexcan.h"
#include "../Std_Types.h"

typedef uint16 Can_HwHandleType;

typedef uint16 PduIdType;

typedef uint32 Can_IdType;

typedef enum {
    CAN_UNINIT = 0U,
    CAN_READY
} Can_StatusType;

typedef enum {
    CAN_CS_UNINIT = 0U,
    CAN_CS_STARTED,
    CAN_CS_STOPPED,
    CAN_CS_SLEEP,
    CAN_CS_WAKEUP,
    CAN_CS_INIT
} Can_ControllerStateType;

typedef enum {
    CAN_ERRORSTATE_ACTIVE = 0U,
    CAN_ERRORSTATE_PASSIVE,
    CAN_ERRORSTATE_BUSOFF
} Can_ErrorStateType;

typedef enum {
    INTERRUPT = 0U,
    POLLING,
    MIXED
} Can_EventProcessingType;

typedef enum {
    BASIC = 0U,
    FULL
} Can_HandleType;

#ifndef SUPPORT_3RD_ERPC
// Aliases data types declarations
typedef struct rx_mb_frame_t rx_mb_frame_t;
typedef struct tx_mb_ctrl_t tx_mb_ctrl_t;
typedef struct Can_ControllerConfig Can_ControllerConfig;
typedef struct Can_MBConfig Can_MBConfig;
typedef struct Can_RxFIFOConfig Can_RxFIFOConfig;
typedef struct Can_BaudRateConfig Can_BaudRateConfig;
typedef struct Can_ConfigType Can_ConfigType;
typedef struct Can_PduType Can_PduType;

/******************* CAN configuration data structure *********************/
struct rx_mb_frame_t
{
    uint32 frameId;
    uint32 rxIDFilterMask;
    flexcan_frame_type_t frameType;
    flexcan_frame_format_t frameFormat;
};

struct tx_mb_ctrl_t
{
    uint8 paddingVal;
};

struct Can_ControllerConfig
{
    uint8 controllerId;
    uint32 baseAddr;
    uint32 irq_num;
    uint32  RxProcessType;
    uint32  TxProcessType;
    uint32  BusoffProcessType;
    uint32  WakeupProcessType;
    flexcan_config_t flexcanCfg;
};

struct Can_MBConfig
{
    uint32 hwObjId;
    uint8 controllerId;
    uint8 mbId;
    uint8 polling;
    uint16 CanHwObjectCount;
    uint8 CanHandleType;
    rx_mb_frame_t rx;
    tx_mb_ctrl_t tx;
};

struct Can_RxFIFOConfig
{
    uint32 hwObjId;
    uint8 controllerId;
    uint8 polling;
    flexcan_rx_fifo_config_t flexcanRxFIFOCfg;
};

struct Can_BaudRateConfig
{
    uint8 controllerId;
    flexcan_timing_config_t nomianlBitTimingCfg;
    flexcan_timing_config_t dataBitTimingCfg;
    flexcan_fd_config_t can_fd_cfg;
};

struct Can_ConfigType
{
    uint8 controllerCount;
    uint8 rxFifoCount;
    uint8 baudRateCfgCount;
    uint16 rxCount;
    uint16 txCount;
    Can_ControllerConfig * ctrllerCfg;
    Can_MBConfig * rxMBCfg;
    Can_MBConfig * txMBCfg;
    Can_RxFIFOConfig * rxFIFOCfg;
    Can_BaudRateConfig * baudRateCfg;
};

struct Can_PduType
{
    uint16 swPduHandle;
    uint8 length;
    uint32 id;
    uint8 * sdu;
};

#endif

/* CAN controller error state. */
#define CAN_ERROR_ACTIVE    0U
#define CAN_ERROR_PASSIVE   1U
#define CAN_BUS_OFF         2U

/* Parse ID. */
#define GET_CAN_ID(id)  ((id) & 0x3FFFFFFFU)
#define GET_ID_TYPE(id) (((id) >> 30U) & 3U)
/* ID type. */
#define STANDARD_CAN    0U
#define STANDARD_CAN_FD 1U
#define EXTENDED_CAN    2U
#define EXTENDED_CAN_FD 3U
/* Make ID. */
#define MAKE_STANDARD_CAN_ID(id)    ((id) | (STANDARD_CAN << 30))
#define MAKE_STANDARD_CAN_FD_ID(id) ((id) | (STANDARD_CAN_FD << 30))
#define MAKE_EXTENDED_CAN_ID(id)    ((id) | (EXTENDED_CAN << 30))
#define MAKE_EXTENDED_CAN_FD_ID(id) ((id) | (EXTENDED_CAN_FD << 30))

/* Maxium Rx FIFO depth. */
#define MAX_RX_FIFO_DEPTH   6U

/* CAN busy status. */
#define CAN_BUSY    2U

/***************Function prototypes***************/

extern void Can_Init(const Can_ConfigType* Config);
extern void Can_DeInit(void);
extern Std_ReturnType Can_SetBaudrate(uint8 Controller,
                                      uint16 BaudRateConfigID);
extern Std_ReturnType Can_SetControllerMode(uint8 Controller,
                                                Can_ControllerStateType Transition);
extern void Can_DisableControllerInterrupts(uint8 Controller);
extern void Can_EnableControllerInterrupts(uint8 Controller);
extern void Can_CheckWakeup(uint8 Controller);
extern Std_ReturnType Can_GetControllerErrorState(uint8 ControllerId,
                                                        Can_ErrorStateType* ErrorStatePtr);
extern Std_ReturnType Can_GetControllerMode(uint8 Controller,
                                                Can_ControllerStateType* ControllerModePtr);
extern Std_ReturnType Can_Write(Can_HwHandleType Hth,
                                const Can_PduType* PduInfo);
extern void Can_MainFunction_Read(void);
extern void Can_MainFunction_Write(void);

#endif
