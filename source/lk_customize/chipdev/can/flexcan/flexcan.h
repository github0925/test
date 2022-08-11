/*
 * Copyright (c)  Semidrive
 */

#ifndef __FLEXCAN_H__
#define __FLEXCAN_H__

#include <stdbool.h>
#include <stdint.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef DBGV
#define DBGV 4
#endif

#ifndef WARN
#define WARN 1
#endif

#ifndef __critical_code__
#define __critical_code__ __CRITICAL_CODE__
#endif

#ifndef __critical_data__
#define __critical_data__ __CRITICAL_DATA__
#endif

/* Number of message buffers */
#define FLEXCAN_HAS_MESSAGE_BUFFER_MAX_NUMBER   64U

/* Doze mode support definition. */
#define FLEXCAN_HAS_DOZE_MODE_SUPPORT   1

/* Abnormal interrupt configuration. */
#define EN_ERR_INT  0
#define EN_WARNING_INT  0
#define EN_WAKE_UP_INT  1

/* Make Rx FIFO ID filter table content. */
/* ID type */
#define STANDARD_DATA_ID   0U
#define STANDARD_REMOTE_ID 2U
#define EXTENDED_DATA_ID   1U
#define EXTENDED_REMOTE_ID 3U
/* Type A */
#define MAKE_TYPE_A_FILTER(id, type) (((type) << 30) | ((id) << (((type) & 1U) ? 1 : 19)))
/* Type B */
#define MAKE_TYPE_B_FILTER(id1, id2, type1, type2)    (((type1) << 30) | ((id1) << (((type1) & 1U) ? 16 : 19)) | \
                                                     ((type2) << 14) | ((id2) << (((type2) & 1U) ? 0 : 3)))
/* Type C */
#define MAKE_TYPE_C_FILTER(id1, id2, id3, id4)  (((id1) & 0xFFU) | ((id2) & 0xFFU) | \
                                                 ((id3) & 0xFFU) | ((id4) & 0xFFU))

#ifndef SUPPORT_3RD_ERPC
// Aliases data types declarations
typedef struct flexcan_fd_config_t flexcan_fd_config_t;
typedef struct flexcan_timing_config_t flexcan_timing_config_t;
typedef struct flexcan_config_t flexcan_config_t;
typedef struct flexcan_rx_fifo_config_t flexcan_rx_fifo_config_t;

/******************* FlexCAN data structure *********************/
typedef enum flexcan_fd_data_size_t
{
    CAN_FD_8BYTES_PER_MB = 0,  /*!< 8 bytes per message buffer. */
    CAN_FD_16BYTES_PER_MB = 1,      /*!< 16 bytes per message buffer. */
    CAN_FD_32BYTES_PER_MB = 2,      /*!< 32 bytes per message buffer. */
    CAN_FD_64BYTES_PER_MB = 3       /*!< 64 bytes per message buffer. */
} flexcan_fd_data_size_t;

/*! @brief FlexCAN clock source. */
typedef enum flexcan_clock_source_t
{
    FLEXCAN_ClkSrcOsc = 0, /*!< FlexCAN Protocol Engine clock from Oscillator. */
    FLEXCAN_ClkSrcPeri = 1 /*!< FlexCAN Protocol Engine clock from Peripheral Clock. */
} flexcan_clock_source_t;

typedef enum flexcan_rx_fifo_filter_type_t
{
    FLEXCAN_RxFifoFilterTypeA = 0, /*!< One full ID (standard and extended) per ID Filter element. */
    FLEXCAN_RxFifoFilterTypeB = 1, /*!< Two full standard IDs or two partial 14-bit ID slices per ID Filter Table element. */
    FLEXCAN_RxFifoFilterTypeC = 2, /*!< Four partial 8-bit Standard or extended ID slices per ID Filter Table element. */
    FLEXCAN_RxFifoFilterTypeD = 3 /*!< All frames rejected. */
} flexcan_rx_fifo_filter_type_t;

typedef enum flexcan_rx_fifo_priority_t
{
    FLEXCAN_RxFifoPrioLow = 0, /*!< Matching process start from Rx Message Buffer first*/
    FLEXCAN_RxFifoPrioHigh = 1 /*!< Matching process start from Rx FIFO first*/
} flexcan_rx_fifo_priority_t;

/*! @brief FlexCAN frame type. */
typedef enum flexcan_frame_type_t
{
    FLEXCAN_FrameTypeData = 0, /*!< Data frame type attribute. */
    FLEXCAN_FrameTypeRemote = 1 /*!< Remote frame type attribute. */
} flexcan_frame_type_t;

/*! @brief FlexCAN frame format. */
typedef enum flexcan_frame_format_t
{
    FLEXCAN_STANDARD_FRAME = 0, /*!< Standard frame format attribute. */
    FLEXCAN_EXTEND_FRAME = 1           /*!< Extend frame format attribute. */
} flexcan_frame_format_t;

typedef enum flexcan_controller_id_t
{
    CAN1 = 0,
    CAN2 = 1,
    CAN3 = 2,
    CAN4 = 3,
    CAN5 = 4,
    CAN6 = 5,
    CAN7 = 6,
    CAN8 = 7,
    CAN9 = 8,
    CAN10 = 9,
    CAN11 = 10,
    CAN12 = 11,
    CAN13 = 12,
    CAN14 = 13,
    CAN15 = 14,
    CAN16 = 15,
    CAN17 = 16,
    CAN18 = 17,
    CAN19 = 18,
    CAN20 = 19
} flexcan_controller_id_t;

// Structures/unions data types declarations
struct flexcan_fd_config_t
{
    uint8_t enableISOCANFD;
    uint8_t enableBRS;
    uint8_t enableTDC;
    uint8_t TDCOffset;
    flexcan_fd_data_size_t r0_mb_data_size;
    flexcan_fd_data_size_t r1_mb_data_size;
};

/*! NOTICE: The length of the time quantum should be the same in nominal
 *          and data bit timing (i.e. preDivider should be the same in
 *          nominal and data bit timing configuration) in order to minimize
 *          the chance of error frames on the CAN bus, and to optimize the
 *          clock tolerance in networks that use CAN FD frams.
 */
struct flexcan_timing_config_t
{
    uint16_t preDivider;
    uint8_t rJumpwidth;
    uint8_t propSeg;
    uint8_t phaseSeg1;
    uint8_t phaseSeg2;
};

struct flexcan_config_t
{
    flexcan_clock_source_t clkSrc;
    uint8_t maxMbNum;
    bool enableLoopBack;
    bool enableListenOnly;
    bool enableSelfWakeup;
    uint8_t enableIndividMask;
    bool enableDoze;
    bool enableCANFD;
    flexcan_timing_config_t nominalBitTiming;
    flexcan_timing_config_t dataBitTiming;
    flexcan_fd_config_t can_fd_cfg;
};

typedef struct flexcan_rx_fifo_filter_table {
    uint32_t filter_code;
    uint32_t filter_mask;
} flexcan_rx_fifo_filter_table_t;

struct flexcan_rx_fifo_config_t
{
    uint8_t idFilterNum;
    flexcan_rx_fifo_filter_type_t idFilterType;
    flexcan_rx_fifo_priority_t priority;
    flexcan_rx_fifo_filter_table_t *filter_tab;
};
#endif

/*!
 * @brief FlexCAN Receive Message Buffer configuration structure
 *
 * This structure is used as the parameter of FLEXCAN_SetRxMbConfig() function.
 * The FLEXCAN_SetRxMbConfig() function is used to configure FlexCAN Receive
 * Message Buffer. The function abort previous receiving process, clean the
 * Message Buffer and activate the Rx Message Buffer using given Message Buffer
 * setting.
 */
typedef struct _flexcan_rx_mb_config {
    uint32_t id;                   /*!< CAN Message Buffer Frame Identifier. */
    flexcan_frame_format_t format; /*!< CAN Frame Identifier format(Standard of Extend). */
    flexcan_frame_type_t type;     /*!< CAN Frame Type(Data or Remote). */
} flexcan_rx_mb_config_t;

/*! @brief FlexCAN message frame structure. */
typedef struct _flexcan_frame {
    uint16_t timestamp;         /*!< FlexCAN internal Free-Running Counter Time Stamp. */
    uint32_t id;                /*!< CAN Frame Identifier. */
    struct {
        uint32_t length:7;      /*!< CAN frame payload length in bytes(Range: 0~64). */
        uint32_t type:1;        /*!< CAN Frame Type(DATA or REMOTE). */
        uint32_t format:1;      /*!< CAN Frame Identifier(STD or EXT format). */
        uint32_t isCANFDFrame:1;/*!< CAN FD or classic frame? */
        uint32_t isCANFDBrsEn:1;/*!< CAN FD BRS enabled? */
        uint32_t reserved1:5;   /*!< Reserved for placeholder. */
        uint32_t idHit:9;       /*!< CAN Rx FIFO filter hit id(This value is only used in Rx FIFO receive mode). */
        uint32_t reserved2:7;   /*!< Reserved for placeholder. */
    };
    uint8_t *dataBuffer;        /*!< Frame buffer. NOTE: For transmitting buffer, the data order should be consistent with the cpu endianness. */
} flexcan_frame_t;

/*! @brief FlexCAN Message Buffer transfer. */
typedef struct _flexcan_mb_transfer {
    flexcan_frame_t *pFrame; /*!< The buffer of CAN Message to be transfer. */
    uint8_t mbIdx;          /*!< The index of Message buffer used to transfer Message. */
} flexcan_mb_transfer_t;

/*! @brief FlexCAN Rx FIFO transfer. */
typedef struct _flexcan_fifo_transfer {
    flexcan_frame_t *pFrame; /*!< The buffer of CAN Message to be received from Rx FIFO. */
} flexcan_fifo_transfer_t;

/*! @brief FlexCAN handle structure definition. */
typedef struct _flexcan_handle flexcan_handle_t;

/*! @brief FlexCAN transfer status. */
typedef enum _flexcan_status {
    FLEXCAN_SUCCESS = 0U,     /*!< Operation succeeds. */
    FLEXCAN_FAIL,             /*!< Operation fails. */
    FLEXCAN_TX_BUSY,          /*!< Tx Message Buffer is Busy. */
    FLEXCAN_TX_IDLE,          /*!< Tx Message Buffer is Idle. */
    FLEXCAN_TX_SWITCH_TO_RX,  /*!< Remote Message is send out and Message buffer changed to Receive one. */
    FLEXCAN_RX_BUSY,          /*!< Rx Message Buffer is Busy. */
    FLEXCAN_RX_IDLE,          /*!< Rx Message Buffer is Idle. */
    FLEXCAN_RX_OVERFLOW,      /*!< Rx Message Buffer is Overflowed. */
    FLEXCAN_RX_FIFO_BUSY,     /*!< Rx Message FIFO is Busy. */
    FLEXCAN_RX_FIFO_IDLE,     /*!< Rx Message FIFO is Idle. */
    FLEXCAN_RX_FIFO_OVERFLOW, /*!< Rx Message FIFO is overflowed. */
    FLEXCAN_RX_FIFO_WARNING,  /*!< Rx Message FIFO is almost overflowed. */
    FLEXCAN_ERROR_STATUS,     /*!< FlexCAN Module Error and Status. */
    FLEXCAN_UNHANDLED,        /*!< UnHadled Interrupt asserted. */
} flexcan_status_t;

/*! @brief FlexCAN transfer callback function.
 *
 *  The FlexCAN transfer callback will return value from the underlying layer.
 *  If the status equals to FLEXCAN_ERROR_STATUS, the result parameter will be the Content of
 *  FlexCAN status register which can be used to get the working status(or error status) of FlexCAN module.
 *  If the status equals to other FlexCAN Message Buffer transfer status, the result will be the index of
 *  Message Buffer that generate transfer event.
 *  If the status equals to other FlexCAN Message Buffer transfer status, the result is meaningless and should be
 *  Ignored.
 */
typedef void (*flexcan_transfer_callback_t)(uint8_t ch,
        flexcan_status_t status, uint32_t result, void *userData);

/*! @brief FlexCAN handle structure. */
struct _flexcan_handle {
    /*!< Peripheral base address. */
    void *base_addr;
    /*!< Callback function. */
    flexcan_transfer_callback_t callback;
    /*!< FlexCAN callback function parameter.*/
    void *userData;
    /*!< The buffer for received data from Message Buffers. */
    flexcan_frame_t *volatile pMBFrameBuf[FLEXCAN_HAS_MESSAGE_BUFFER_MAX_NUMBER];
    /*!< The buffer for received data from Rx FIFO. */
    flexcan_frame_t *volatile pRxFifoFrameBuf;
    /*!< Message Buffer transfer state. */
    volatile uint8_t mbState[FLEXCAN_HAS_MESSAGE_BUFFER_MAX_NUMBER];
    /*!< Rx FIFO transfer state. */
    volatile uint8_t rxFifoState;
};

/*!
 * @brief FlexCAN interrupt configuration structure, default settings all disabled.
 *
 * This structure contains the settings for all of the FlexCAN Module interrupt configurations.
 * Note: FlexCAN Message Buffers and Rx FIFO have their own interrupts.
 */
enum _flexcan_interrupt_enable {
    FLEXCAN_BusOffInterruptEnable    = 0x8000U,     /*!< CAN_CTRL1_BOFFMSK_MASK, Bus Off interrupt. */
    FLEXCAN_ErrorInterruptEnable     = 0x4000U,     /*!< CAN_CTRL1_ERRMSK_MASK, Error interrupt. */
    FLEXCAN_RxWarningInterruptEnable = 0x400U,      /*!< CAN_CTRL1_RWRNMSK_MASK, Rx Warning interrupt. */
    FLEXCAN_TxWarningInterruptEnable = 0x800U,      /*!< CAN_CTRL1_TWRNMSK_MASK, Tx Warning interrupt. */
    FLEXCAN_WakeUpInterruptEnable    = 0x4000000U,  /*!< CAN_MCR_WAKMSK_MASK, Wake Up interrupt. */
};


/******************************************************************************
 * API
 *****************************************************************************/

/*!
 * @brief Initializes a FlexCAN instance.
 *
 * This function initializes the FlexCAN module with user-defined settings.
 * This example shows how to set up the flexcan_config_t parameters and how
 * to call the flexcan_init function by passing in these parameters:
 *  @code
 *   flexcan_config_t flexcanConfig;
 *   flexcanConfig.clkSrc            = KFLEXCAN_ClkSrcOsc;
 *   flexcanConfig.maxMbNum          = 16;
 *   flexcanConfig.enableLoopBack    = false;
 *   flexcanConfig.enableListenOnly  = false;
 *   flexcanConfig.enableSelfWakeup  = false;
 *   flexcanConfig.enableIndividMask = false;
 *   flexcanConfig.enableDoze        = false;
 *   flexcanConfig.enableCANFD       = false;
 *   flexcanConfig.nominalBitTiming.preDivider = 0U;
 *   flexcanConfig.nominalBitTiming.rJumpwidth = 1U;
 *   flexcanConfig.nominalBitTiming.propSeg = 3U;
 *   flexcanConfig.nominalBitTiming.phaseSeg1 = 4U;
 *   flexcanConfig.nominalBitTiming.phaseSeg2 = 2U;
 *   flexcan_init(CAN0, &flexcanConfig);
 *   @endcode
 *
 * @param ch FlexCAN channel.
 * @param config Pointer to user-defined configuration structure.
 */
extern void flexcan_init(uint8_t ch, const flexcan_config_t *config);

/*!
 * @brief Enables or disable the FlexCAN module operation.
 *
 * This function enables or disables the FlexCAN module.
 *
 * @param ch FlexCAN channel.
 * @param enable true to enable, false to disable.
 */
extern void flexcan_enable(uint8_t ch, bool enable);

/*!
 * @brief Enter or exit freeze mode.
 *
 * This function makes the FlexCAN work under Fraze Mode
 * if param freeze is true, or work under Normal Mode if
 * param freeze is false.
 *
 * @param ch FlexCAN channel.
 * @param freeze true to enter freeze mode, false to exit
 *        freeze mode
 * @retval FLEXCAN_SUCCESS operate successfully
 * @retval FLEXCAN_FAIL operate timeout
 */
extern flexcan_status_t flexcan_freeze(uint8_t ch, bool freeze);

/*!
 * @brief Reset the FlexCAN Instance.
 *
 * Restores the FlexCAN module to reset state, notice that this function
 * will set all the registers to reset state so the FlexCAN module can not work
 * after calling this API.
 *
 * @param ch FlexCAN channel.
*/
extern void flexcan_reset(uint8_t ch);

/*!
 * @brief Setting FlexCAN classic protocol timing characteristic.
 *
 * This function give user fine settings to CAN bus timing characteristic.
 * The function is for user who is really versed in CAN protocol, for these
 * users who just what to establish CAN communication among MCUs, just call
 * flexcan_init() and fill the baud rate field with desired one.
 * Doing this, default timing characteristic will provide to the module.
 *
 * Note: Calling flexcan_classic_set_timing_config() will override the baud rate setted
 * in flexcan_init().
 *
 * @param ch FlexCAN channel.
 * @param config Pointer to the timing configuration structure.
 */
extern void flexcan_classic_set_timing_config(uint8_t ch,
        const flexcan_timing_config_t *config);

/*!
 * @brief Setting FlexCAN FD protocol timing characteristic.
 *
 * This function give user fine settings to CAN bus timing characteristic.
 * The function is for user who is really versed in CAN protocol, for these
 * users who just what to establish CAN communication among MCUs, just call
 * flexcan_init() and fill the baud rate field with desired one.
 * Doing this, default timing characteristic will provide to the module.
 *
 * Note: Calling flexcan_fd_set_timing_config() will override the baud rate setted
 * in flexcan_init().
 *
 * @param ch FlexCAN channel.
 * @param arbitrPhaseConfig Pointer to the arbitration phase timing configuration structure.
 * @param dataPhaseConfig Pointer to the data phase timing configuration structure.
 */
extern void flexcan_fd_set_timing_config(uint8_t ch,
        const flexcan_timing_config_t *arbitrPhaseConfig,
        const flexcan_timing_config_t *dataPhaseConfig);

/*!
 * @brief De-initializes a FlexCAN instance.
 *
 * This function disable the FlexCAN module clock and set all register value
 * to reset value.
 *
 * @param ch FlexCAN channel.
 */
extern void flexcan_deinit(uint8_t ch);

/*!
 * @brief Activate/De-activate CAN FD.
 *
 * This function Set the FlexCAN to CAN FD active mode or normal CAN mode.
 *
 * @param ch FlexCAN channel.
 * @param enable Enable/disable CAN FD.
 */
extern void flexcan_activate_can_fd(uint8_t ch, bool enable);

/*!
 * @brief Set the FlexCAN Receive Message Buffer Global Mask.
 *
 * This function Set the global mask for FlexCAN Message Buffer in matching process.
 * The configuration is only effective when Rx Individual Mask is disabled in flexcan_init().
 *
 * @param ch FlexCAN channel.
 * @param mask Rx Message Buffer Global Mask value.
 */
extern void flexcan_set_rx_mb_global_mask(uint8_t ch, uint32_t mask);

/*!
 * @brief Set the FlexCAN Receive FIFO Global Mask.
 *
 * This function Set the global mask for FlexCAN FIFO in matching process.
 *
 * @param ch FlexCAN channel.
 * @param mask Rx Fifo Global Mask value.
 */
extern void flexcan_set_rx_fifo_global_mask(uint8_t ch, uint32_t mask);

/*!
 * @brief Set the FlexCAN Receive Individual Mask.
 *
 * This function Set the Individual mask for FlexCAN matching process.
 * The configuration is only effective when Rx Individual Mask is enabled in FLEXCAN_Init().
 * If Rx FIFO is disabled, the Individual Mask is applied to corresponding Message Buffer.
 * If Rx FIFO is enabled, the Individual Mask for Rx FIFO occupied Message Buffer will be applied to
 * Rx Filter with same index. What calls for special attention is that only the first 32
 * Individual Mask can be used as Rx FIFO Filter Mask.
 *
 * @param ch FlexCAN channel.
 * @param maskIdx The Index of individual Mask.
 * @param mask Rx Individual Mask value.
 */
extern void flexcan_set_rx_individual_mask(uint8_t ch, uint8_t maskIdx,
        uint32_t mask);

/*!
 * @brief Configure a FlexCAN Transmit Message Buffer.
 *
 * This function abort privious transmission, clean the Message Buffer and
 * configure it as a Transmit Message Buffer.
 *
 * @param ch FlexCAN channel.
 * @param mbIdx The Message Buffer index.
 * @param tx_by_interrupt Tx by interrupt (true) or polling (false).
 */
extern void flexcan_set_tx_mb_config(uint8_t ch, uint8_t mbIdx, bool tx_by_interrupt);

/*!
 * @brief Configure a FlexCAN Receive Message Buffer.
 *
 * This function clean a FlexCAN build-in Message Buffer and configure it
 * as a Receive Message Buffer.
 *
 * @param ch FlexCAN channel.
 * @param mbIdx The Message Buffer index.
 * @param config Pointer to FlexCAN Message Buffer configuration structure.
 */
extern void flexcan_set_rx_mb_config(uint8_t ch, uint8_t mbIdx,
                                     const flexcan_rx_mb_config_t *config);

/*!
 * @brief Configure the FlexCAN Rx FIFO.
 *
 * This function Configure the Rx FIFO with given Rx FIFO configuration.
 *
 * @param ch FlexCAN channel.
 * @param config Pointer to FlexCAN Rx FIFO configuration structure.
 */
extern void flexcan_set_rx_fifo_config(uint8_t ch,
                                       const flexcan_rx_fifo_config_t *config);

/*!
 * @brief Initialize the FlexCAN handle.
 *
 * This function initializes the FlexCAN handle which can be used for other FlexCAN
 * APIs. Usually, for a specified FlexCAN instance, user only need
 * to call this API once to get the initialized handle.
 * NOTE: This function should be called before all the other FlexCAN APIs.
 *
 * @param ch FlexCAN channel.
 * @param reg_base FlexCAN peripheral base address.
 * @param handle FlexCAN handle pointer.
 * @param callback The callback function.
 * @param userData The parameter of the callback function.
 */
extern void flexcan_create_handle(uint8_t ch, void *reg_base,
                                  flexcan_handle_t *handle, flexcan_transfer_callback_t callback,
                                  void *userData);

/*!
 * @brief Enable FlexCAN interrupts according to provided mask.
 *
 * This function enables the FlexCAN interrupts according to provided mask. The mask
 * is a logical OR of enumeration members, see @ref _flexcan_interrupt_enable.
 *
 * @param ch FlexCAN channel.
 * @param mask The interrupts to enable. Logical OR of @ref _flexcan_interrupt_enable.
 */
extern void flexcan_enable_interrupts(uint8_t ch, uint32_t mask);

/*!
 * @brief Disable FlexCAN interrupts according to provided mask.
 *
 * This function disables the FlexCAN interrupts according to provided mask. The mask
 * is a logical OR of enumeration members, see @ref _flexcan_interrupt_enable.
 *
 * @param ch FlexCAN channel.
 * @param mask The interrupts to disable. Logical OR of @ref _flexcan_interrupt_enable.
 */
extern void flexcan_disable_interrupts(uint8_t ch, uint32_t mask);

/*!
 * @brief send message using IRQ
 *
 * This function send message using IRQ, this is non-blocking function, will return
 * right away, when message have been sent out, the send callback function will be called.
 *
 * @param ch FlexCAN channel.
 * @param xfer FlexCAN Message Buffer transfer structure, refer to #flexcan_mb_transfer_t.
 * @param padding_val value used to pad unspecified data in CAN FD frames > 8bytes.
 * @retval FLEXCAN_SUCCESS        Start Tx Message Buffer sending process successfully.
 * @retval FLEXCAN_FAIL           Write Tx Message Buffer failed.
 * @retval FLEXCAN_TX_BUSY        Tx Message Buffer is in use.
 */
extern flexcan_status_t __critical_code__ flexcan_send_nonblocking(uint8_t ch,
        flexcan_mb_transfer_t *xfer,
        uint8_t padding_val);

/*!
 * @brief Receive message using IRQ
 *
 * This function receive message using IRQ, this is non-blocking function, will return
 * right away, when message have been received, the receive callback function will be called.
 *
 * @param ch FlexCAN channel.
 * @param xfer FlexCAN Message Buffer transfer structure, refer to #flexcan_mb_transfer_t.
 * @retval FLEXCAN_SUCCESS        Start Rx Message Buffer receiving process successfully.
 * @retval FLEXCAN_RX_BUSY        Rx Message Buffer is in use.
 */
extern flexcan_status_t flexcan_receive_nonblocking(uint8_t ch,
        flexcan_mb_transfer_t *xfer);

/*!
 * @brief Receive message from Rx FIFO using IRQ
 *
 * This function receive message using IRQ, this is non-blocking function, will return
 * right away, when all messages have been received, the receive callback function will be called.
 *
 * @param ch FlexCAN channel.
 * @param xfer FlexCAN Rx FIFO transfer structure, refer to #flexcan_fifo_transfer_t.
 * @retval FLEXCAN_SUCCESS            - Start Rx FIFO receiving process successfully.
 * @retval FLEXCAN_RX_FIFO_BUSY       - Rx FIFO is currently in use.
 */
extern flexcan_status_t flexcan_receive_fifo_nonblocking(uint8_t ch,
        flexcan_fifo_transfer_t *xfer);

/*!
 * @brief Write FlexCAN Message to Transmit Message Buffer.
 *
 * This function write a CAN Message to the specified Transmit Message Buffer.
 * and change the Message Buffer state to start CAN Message transmit, after
 * that the function will return immediately.
 *
 * @param ch FlexCAN channel.
 * @param mbIdx The FlexCAN Message Buffer index.
 * @param txFrame Pointer to CAN message frame to be sent.
 * @param padding_val value used to pad unspecified data in CAN FD frames > 8bytes.
 * @return FLEXCAN_SUCCESS - Write Tx Message Buffer Successfully.
           FLEXCAN_FAIL    - Tx Message Buffer is currently in use.
 */
extern flexcan_status_t __critical_code__ flexcan_write_tx_mb(uint8_t ch, uint8_t mbIdx,
        const flexcan_frame_t *txFrame, uint8_t padding_val);

/*!
 * @brief Read a FlexCAN Message from Receive Message Buffer.
 *
 * This function read a CAN message from a specified Receive Message Buffer.
 * The function will fill receive CAN message frame structure with
 * just received data and activate the Message Buffer again.
 * The function will return immediately.
 *
 * @param ch FlexCAN channel.
 * @param mbIdx The FlexCAN Message Buffer index.
 * @param rxFrame Pointer to CAN message frame structure.
 * @retval FLEXCAN_SUCCESS            - Rx Message Buffer is full and has been read successfully.
 * @retval FLEXCAN_RX_OVERFLOW        - Rx Message Buffer is already overflowed and has been read successfully.
 * @retval FLEXCAN_FAIL               - Rx Message Buffer is empty.
 */
extern flexcan_status_t __critical_code__ flexcan_read_rx_mb(uint8_t ch, uint8_t mbIdx,
        flexcan_frame_t *rxFrame);

/*!
 * @brief Read a FlexCAN Message from Rx FIFO.
 *
 * This function Read a CAN message from the FlexCAN build-in Rx FIFO.
 *
 * @param ch FlexCAN channel.
 * @param rxFrame Pointer to CAN message frame structure.
 * @retval FLEXCAN_SUCCESS - Read Message from Rx FIFO successfully.
 * @retval FLEXCAN_FAIL    - Rx FIFO is not enabled.
 */
extern flexcan_status_t __critical_code__ flexcan_read_rx_fifo(uint8_t ch,
        flexcan_frame_t *rxFrame);

/*!
 * @brief Abort interrupt driven message send process.
 *
 * This function aborts interrupt driven message send process.
 *
 * @param ch FlexCAN channel.
 * @param mbIdx The FlexCAN Message Buffer index.
 */
extern void flexcan_abort_mb_send(uint8_t ch, uint8_t mbIdx);

/*!
 * @brief Abort interrupt driven message receive process.
 *
 * This function abort interrupt driven message receive process.
 *
 * @param ch FlexCAN channel.
 * @param mbIdx The FlexCAN Message Buffer index.
 */
extern void flexcan_abort_mb_receive(uint8_t ch, uint8_t mbIdx);

/*!
 * @brief Abort interrupt driven message receive from Rx FIFO process.
 *
 * This function abort interrupt driven message receive from Rx FIFO process.
 *
 * @param ch FlexCAN channel.
 */
extern void flexcan_abort_receive_fifo(uint8_t ch);

/*!
 * @brief FlexCAN IRQ handle function
 *
 * This function handles the FlexCAN Error, Message Buffer and Rx FIFO IRQ request.
 *
 * @param arg FlexCAN channel.
 */
extern enum handler_return __critical_code__ flexcan_irq_handler(void *arg);

/*!
 * @brief Read MB interrupt status.
 *
 * @param ch FlexCAN channel.
 * @param mbIdx The FlexCAN Message Buffer index.
 * @return uint32_t If MB interupt triggered.
 */
extern uint32_t flexcan_read_mb_int_status(uint8_t ch, uint8_t mbIdx);

#ifdef __cplusplus
}
#endif
#endif
