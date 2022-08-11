//*****************************************************************************
//
// mbox_hal.h - Prototypes for the mailbox hal
//
// Copyright (c) 2019-2029 Semidrive Incorporated.  All rights reserved.
// Software License Agreement
//
//
//*****************************************************************************

#ifndef __MBOX_HAL_H__
#define __MBOX_HAL_H__
//*****************************************************************************
//
// If building with a C++ compiler, make all of the definitions in this header
// have a C binding.
//
//*****************************************************************************
#ifdef __cplusplus
extern "C"
{
#endif

#include <lib/cbuf.h>
#include "__regs_base.h"
#include <chip_res.h>
#include <kernel/mutex.h>
#if defined(ENABLE_SD_MBOX) && (ENABLE_SD_MBOX == 1)
#include <mb_controller.h>
#include "mb_msg.h"
#else
#define MB_MSG_HDR_SZ           (0)
#endif

#define MAKE_VERSION(major, minor, bugfix) (((major) << 16) | ((minor) << 8) | (bugfix))
#define SDV_MBOX_DRIVER_VERSION (MAKE_VERSION(1, 0, 0)) /*!< Version 1.0.0 */

#define MBOX_MAX_NUM            (1)
#define MBOX_NAME_SZ            (16)
/* queue length should be 2^N and less than 256 */
#define MBOX_TX_QUEUE_LEN       (2)
/* MTU should be 2^N and less than 1024 for hardware limitation */
#define MBOX_TX_MTU             (512)
#define MBOX_TX_BUF_SZ          (MBOX_TX_QUEUE_LEN * MBOX_TX_MTU)
#define MBOX_RX_BUF_SZ          (MBOX_TX_QUEUE_LEN * MBOX_TX_MTU)

typedef struct {
    u32 res_glb_idx;
} mbox_capability_cfg_t;

typedef struct {
    u32 version;
    char res_category[20];
    uint8_t res_max_num;
    mbox_capability_cfg_t res_cap[MBOX_MAX_NUM];
} mbox_capability_t;

typedef struct {
    u32 version;
    char res_category[20];
    u32 mbox_phy_addr;//saf mbox addr
} mbox_res_config_t;

/**
 *****************************************************************************
 ** \brief MB HAL overall configuration.
 *****************************************************************************/
typedef struct {
    mbox_capability_t caps;
    mbox_res_config_t res_cfg;
} hal_mb_cfg_t;

struct _hal_mbox_client;

/* mbox channel */
typedef struct _mbox_chan_ {
    char name[MBOX_NAME_SZ];
    u8 remote_proc;
    u8 dest_addr;
    unsigned msg_count, msg_free;
    unsigned char *msg_data[MBOX_TX_QUEUE_LEN];
    spin_lock_t lock; /* Serialise access to the channel */
    struct sd_mbox_chan *hwchan;
} hal_mb_chan_t;

/* mbox client instance */
typedef struct _hal_mbox_client {
    u32 magic;
    hal_mb_chan_t mchan;
    bool is_used;
    bool tx_block;
    u8 this_addr;
    bool low_latency;
    void *user_data;

    void (*rx_new_data)(struct _hal_mbox_client *cl, void *mssg, u16 len);
    void (*tx_complete)(struct _hal_mbox_client *cl, void *mssg, int r);
} *hal_mb_client_t;

typedef void(*hal_mb_rx_cb)(hal_mb_client_t, void *, u16);

typedef u16 hal_mb_buf_id;

typedef enum {
    IPCC_RRPOC_SAF  = 0,
    IPCC_RRPOC_SEC  = 1,
    IPCC_RRPOC_MP   = 2,
    IPCC_RRPOC_AP1  = 3,
    IPCC_RRPOC_AP2  = 4,
    IPCC_RRPOC_VDSP = 5,
    IPCC_RRPOC_MAX  = IPCC_RRPOC_VDSP,
} hal_mb_proc_t;

/******************************************************************************
 ** \brief Create the handle of MB
 **
 ** \param [in]   res_glb_idx      global resource index
 ** \param [out]  handle           pointer of the handle create
 ** \return       bool             result of get instance
 *****************************************************************************/
bool hal_mb_create_handle(void **handle, uint32_t res_glb_idx);

/******************************************************************************
 ** \brief Release the instance of MB
 **
 ** \param [out]  instance         pointer of the instance
 *****************************************************************************/
bool hal_mb_release_handle(void *handle);

/******************************************************************************
 ** \brief HAL Initialize MB instance with configuration
 **
 ** \param [in]   handle      pointer of the handle created
 ** \param [in]   cfg         pointer of the hal configuration
 *****************************************************************************/
void hal_mb_init(void *handle, hal_mb_cfg_t *cfg);

/******************************************************************************
 ** \brief HAL Deinitialize MB instance
 **
 ** \param [in]   handle      pointer of the handle
 *****************************************************************************/
void hal_mb_deinit(void *handle);

/******************************************************************************
 ** \brief to create a user client
 ** with_addr version passing myaddr as client address
 ** if not specified, the default addr is 0xfe
 **
 ** \param [in]   myaddr      a predefined client address
 ** \return hal_mb_client_t   pointer of the client
 *****************************************************************************/
hal_mb_client_t hal_mb_get_client(void);
hal_mb_client_t hal_mb_get_client_with_addr(u8 myaddr);

/******************************************************************************
 ** \brief to destroy a user client
 **
 ** \param [in]   client      pointer of the handle
 *****************************************************************************/
void hal_mb_put_client(hal_mb_client_t cl);

/******************************************************************************
 ** \brief to request a MB communication channel
 **
 ** \param [in]   cl          pointer of a client
 ** \param [in]   low_latency dedicated channel resource, false for safe use
 ** \param [in]   remote      remote processor id
 ** \param [in]   address     destination address in remote processor
 ** \return hal_mb_chan_t     pointer of the channel
 *****************************************************************************/
hal_mb_chan_t *hal_mb_request_channel(hal_mb_client_t cl, bool low_latency,
                                      hal_mb_rx_cb cb, hal_mb_proc_t remote);
hal_mb_chan_t* hal_mb_request_channel_with_addr(hal_mb_client_t cl,
    bool low_latency, hal_mb_rx_cb cb, hal_mb_proc_t remote, int address);

/******************************************************************************
 ** \brief to free a MB communication channel
 **
 ** \param [in]   chan        pointer of the channel
 *****************************************************************************/
void hal_mb_free_channel(hal_mb_chan_t *chan); /* may sleep */

/******************************************************************************
** \brief MB MTU is for current implemetation
**        this value could be ajdusted for later optimization
*****************************************************************************/
#define HAL_MB_MTU          (MBOX_TX_MTU-MB_MSG_HDR_SZ)

/******************************************************************************
** \brief to send data via MB communication channel with detailed arguments
**
** \param [in]   chan        pointer of the channel
** \param [in]   data        pointer of the data to send
** \param [in]   len         data length, call fails if len > MTU
** \param [in]   proto       data protocol defines message head & body layout
** \param [in]   priority    if true, callback in ISR context
** \param [in]   dest        destination address if protocol applied
** \param [in]   timeout     tx done timeout
** \return                   0 for success or errocode
*****************************************************************************/
int hal_mb_send_data_detail(hal_mb_chan_t *chan, u8 *data, u16 len,
                    int proto, int priority, u8 dest, lk_time_t timeout);

/******************************************************************************
 ** \brief to send data via MB communication channel in blocking mode
 **
 ** \param [in]   chan        pointer of the channel
 ** \param [in]   data        pointer of the data to send
 ** \param [in]   len         data length, call fails if len > MTU
 ** \param [in]   timeout     time of blocking call in MS
 ** \return                   0 for success or errocode
 *****************************************************************************/
int hal_mb_send_data(hal_mb_chan_t *chan, u8 *data, u16 len, u32 timeout);

/******************************************************************************
** \brief to send data via MB communication channel in non-blocking mode
**
** \param [in]   chan        pointer of the channel
** \param [in]   data        pointer of the data to send
** \param [in]   len         data length, call fails if len > MTU
** \return                   0 for success or errocode
*****************************************************************************/
int hal_mb_send_data_nb(hal_mb_chan_t *chan, u8 *data, u16 len);

/******************************************************************************
** \brief to send data via MB communication channel with ROM compatible
**
** \param [in]   chan        pointer of the channel
** \param [in]   data        pointer of the data to send
** \param [in]   len         data length, call fails if len > MTU
** \return                   0 for success or errocode
*****************************************************************************/
int hal_mb_send_data_rom(hal_mb_chan_t *chan, u8 *data, u16 len);

/******************************************************************************
** \brief to send data via MB communication channel with DSP compatible
**
** \param [in]   chan        pointer of the channel
** \param [in]   data        pointer of the data to send
** \param [in]   len         data length, call fails if len > MTU
** \return                   0 for success or errocode
*****************************************************************************/
int hal_mb_send_data_dsp(hal_mb_chan_t *chan, u8 *data, u16 len);

/******************************************************************************
** \brief to allocate internal buffer for nocopy send
**
** \param [in]   chan        pointer of the channel
** \param [out]  data        pointer of the data to send
** \param [in]   len         pointer of data length, call fails if len > MTU
** \return hal_mb_buf_id     non-zero for success and zero for failure
*****************************************************************************/
int hal_mb_alloc_txbuf(hal_mb_chan_t *chan, u8 **data, u16 *len);

/******************************************************************************
** \brief to send data via MB communication channel in non-blocking mode
**
** \param [in]   chan        pointer of the channel
** \param [in]   idx         buf id returned by hal_mb_alloc_txbuf()
** \param [in]   timeout     time of blocking call in MS
** \return                   0 for success or errocode
*****************************************************************************/
int hal_mb_send_data_nocopy(hal_mb_chan_t *chan, hal_mb_buf_id idx, u32 timeoutMs);

/******************************************************************************
** \brief to trigger a IPI interrupt only
**
** \param [in]   chan        pointer of the channel
** \return                   0 for success or errocode
*****************************************************************************/
int hal_mb_send_ipi(hal_mb_chan_t *chan);

/******************************************************************************
** \brief to cancel message that is pending in transmission
**
** \param [in]   chan        pointer of the channel
*****************************************************************************/
int hal_mb_cancel_lastsend(hal_mb_chan_t *chan);
int hal_mb_cancel_all(hal_mb_chan_t *chan);

/******************************************************************************
** \brief to save user context
**
** \param [in]   cl           pointer of the client
** \param [in]   data         pointer of the user context
*****************************************************************************/
int hal_mb_set_user(hal_mb_client_t cl, void *data);

/******************************************************************************
** \brief to get user context saved previously
**
** \param [in]   cl           pointer of the client
** \return   data             pointer of the user context
*****************************************************************************/
void *hal_mb_get_user(hal_mb_client_t cl);

//*****************************************************************************
//
// Mark the end of the C bindings section for C++ compilers.
//
//*****************************************************************************
#ifdef __cplusplus
}
#endif
#endif // __MBOX_HAL_H__

