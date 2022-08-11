/*
* dw_usb.h
*
* Copyright (c) 2018 Semidrive Semiconductor.
* All rights reserved.
*
* Description: implement synopsys usb driver
*
* Revision History:
* -----------------
* 011, 03/08/2019 chenqing create this file
*/
#ifndef DW_USB_H
#define DW_USB_H

#include <dev/usb.h>
#include <dev/usbc.h>
#include <hw/usb.h>
//#include <sys/types.h>

#define USB_SPEED_FS               1
#define USB_SPEED_HS               0
#define USB_SPEED_SS               4
#define USB_SPEED_SSPLUS           5

typedef struct {
    uint8_t   num;            /*!< Endpoint number
                                This parameter must be a number between Min_Data = 1 and Max_Data = 15    */

    uint8_t   is_in;          /*!< Endpoint direction
                                This parameter must be a number between Min_Data = 0 and Max_Data = 1     */

    uint8_t   is_stall;       /*!< Endpoint stall condition
                                This parameter must be a number between Min_Data = 0 and Max_Data = 1     */

    uint8_t   type;           /*!< Endpoint type
                                 This parameter can be any value of @ref ep_type_t                      */

    void     *trbd;           /*    */
    void     *evtbd;

    uint32_t  maxpacket;      /*!< Endpoint Max packet size
                                 This parameter must be a number between Min_Data = 0 and Max_Data = 64KB */
    uint32_t  bsz;             /* busrt size*/

    uint8_t   *xfer_buff;     /*!< Pointer to transfer buffer                                               */

    int32_t  xfer_len;       /*!< Current transfer length                                                  */

    uint32_t  xfer_count;     /*!< Partial transfer length in case of multi packet transfer                 */
    bool is_enabled;
    bool is_busy;
    int res_id;
} DW_USB_EP;

typedef enum {
    HAL_USB_STATE_RESET   = 0x00,
    HAL_USB_STATE_WAIT_SETUP   = 0x00,
    HAL_USB_STATE_READY   = 0x01,
    HAL_USB_STATE_ERROR   = 0x02,
    HAL_USB_STATE_BUSY    = 0x03,
    HAL_USB_STATE_TIMEOUT = 0x04
} USB_State;

typedef struct {
    uint32_t dev_endpoints;               /*!< Device Endpoints number.
                                             This parameter depends on the used USB core.
                                             This parameter must be a number between Min_Data = 1 and Max_Data = 8 */

    uint32_t speed;                       /*!< USB Core speed. */

    uint32_t ep0_mps;                     /*!< Set the Endpoint 0 Max Packet size.  */

    uint32_t phy_itface;                  /*!< Select the used PHY interface.*/

    uint32_t Sof_enable;                  /*!< Enable or disable the output of the SOF signal.
                                             This parameter can be set to ENABLE or DISABLE                         */

    uint32_t low_power_enable;            /*!< Enable or disable Low Power mode
                                             This parameter can be set to ENABLE or DISABLE                         */

    uint32_t lpm_enable;                  /*!< Enable or disable the Link Power Management .
                                             This parameter can be set to ENABLE or DISABLE                         */

    uint32_t battery_charging_enable;     /*!< Enable or disable Battery charging.
                                             This parameter can be set to ENABLE or DISABLE                         */
    int irq;

} DW_USB_INIT;

enum handler_return dw_usb_irqhandler(void *args);


void dw_usbc_init(int instance,vaddr_t iobase, vaddr_t phybase,DW_USB_INIT init);


status_t dw_usbc_setup_endpoint(int instance, ep_t ep, ep_dir_t dir, uint width, ep_type_t type,bool modify, bool restore);
status_t dw_usbc_queue_rx(int instance, ep_t ep, usbc_transfer_t *transfer);
status_t dw_usbc_queue_tx(int instance, ep_t ep, usbc_transfer_t *transfer);
status_t dw_usbc_flush_ep(int instance, ep_t ep);

status_t dw_usbc_set_active(int instance, bool active);
status_t dw_usbc_cancel_tansfers(int instance, ep_t ep,bool in);
void dw_usbc_set_address(int instance, uint8_t address);

/* called back from within a callback to handle setup responses */
void dw_usbc_ep0_ack(int instance);
void dw_usbc_ep0_stall(int instance);
void dw_usbc_ep0_send(int instance, const void *buf, size_t len, size_t maxlen);
void dw_usbc_ep0_recv(int instance, void *buf, size_t len, ep_callback);

bool dw_usbc_is_highspeed(int instance);
int dw_usbc_get_cur_speed(int instance);
#endif
