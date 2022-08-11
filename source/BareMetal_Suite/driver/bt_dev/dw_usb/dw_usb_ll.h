/*
* dw_usb_ll.h
*
* Copyright (c) 2018 Semidrive Semiconductor.
* All rights reserved.
*
* Description: implement synopsys usb low level api
*
* Revision History:
* -----------------
* 011, 03/08/2019 chenqing create this file
*/

#ifndef DW_USB_LL_H
#define DW_USB_LL_H

#include <types_def.h>
#include <clib_headers.h>

#define DW_USB_EP0MPS_64                          0
#define DW_USB_EP0MPS_32                          1
#define DW_USB_EP0MPS_16                          2
#define DW_USB_EP0MPS_08                          3

/* Device Status Register */
#define DW_DSTS_DCNRD         (1 << 29)

#define DW_DCTL_ULSTCHNGREQ_MASK  (0x0f << 5)
#define DW_DCTL_ULSTCHNGREQ(n) (((n) << 5) & DW_DCTL_ULSTCHNGREQ_MASK)

enum dw_link_state {
    /* In SuperSpeed */
    DW_LINK_STATE_U0      = 0x00, /* in HS, means ON */
    DW_LINK_STATE_U1      = 0x01,
    DW_LINK_STATE_U2      = 0x02, /* in HS, means SLEEP */
    DW_LINK_STATE_U3      = 0x03, /* in HS, means SUSPEND */
    DW_LINK_STATE_SS_DIS      = 0x04,
    DW_LINK_STATE_RX_DET      = 0x05, /* in HS, means Early Suspend */
    DW_LINK_STATE_SS_INACT    = 0x06,
    DW_LINK_STATE_POLL        = 0x07,
    DW_LINK_STATE_RECOV       = 0x08,
    DW_LINK_STATE_HRESET      = 0x09,
    DW_LINK_STATE_CMPLY       = 0x0a,
    DW_LINK_STATE_LPBK        = 0x0b,
    DW_LINK_STATE_RESET       = 0x0e,
    DW_LINK_STATE_RESUME      = 0x0f,
    DW_LINK_STATE_MASK        = 0x0f,
};

#define EPCMD_SET_CONFIG 1
#define EPCMD_SET_XFER 2
#define EPCMD_GET_STAT 3
#define EPCMD_SET_STALL 4
#define EPCMD_CLEAR_STALL 5
#define EPCMD_START_TRANS 6
#define EPCMD_UPDATE_TRANS 7
#define EPCMD_END_TRANS 8
#define EPCMD_SET_NEW_CONFIG 9

#define DW_GET_XFER_RES_ID(v) (((v) >> 16) & 0x7f)
#define DW_GET_CMD_STATUS(v) ((v)>>12 & 0xf)
#define DW_GET_CONNDONE_SPEED(v) ((v) & 0x7)

#define DW_DSTS_USBLNKST_MASK     (0x0f << 18)
#define DW_DSTS_USBLNKST(n)       (((n) & DW_DSTS_USBLNKST_MASK) >> 18)

bool dw_usb_dev_ll_init(vaddr_t iobase);

bool dw_usb_dev_ll_por_srst(vaddr_t iobase, vaddr_t phybase);
bool dw_usb_ll_runstop(vaddr_t iobase, bool enable);
/*EP */
bool dw_usb_ll_set_ep_config(vaddr_t iobase, int p_epnum, 
        uint32_t param0,uint32_t param1, uint32_t param2);
bool dw_usb_ll_set_ep_xfer_config(vaddr_t iobase, int p_epnum, uint32_t param0);
uint32_t dw_usb_ll_get_ep_stat(vaddr_t iobase, int p_epnum);
bool dw_usb_ll_set_ep_stall(vaddr_t iobase, int p_epnum);
bool dw_usb_ll_clear_ep_stall(vaddr_t iobase, int p_epnum, uint32_t param0);
bool dw_usb_ll_ep_start_transfer(vaddr_t iobase, int p_epnum,
        uint32_t param0,uint32_t param1);
bool dw_usb_ll_ep_update_transfer(vaddr_t iobase, int p_epnum,int res_id);
bool dw_usb_ll_ep_end_transfer(vaddr_t iobase, int p_epnum,int res_id);
bool dw_usb_ll_set_ep_new_config(vaddr_t iobase, int p_epnum);
bool dw_usb_ll_flush_ep(vaddr_t iobase, int p_epnum);
bool dw_usb_ll_ep_open(vaddr_t iobase, int p_epnum);
bool dw_usb_ll_ep_close(vaddr_t iobase, int p_epnum);

/*event buffer*/
bool dw_usb_ll_set_ep_event_buffer(vaddr_t iobase, int p_epnum, 
        uint32_t adr_l,uint32_t adr_h,uint16_t size);
uint16_t dw_usb_ll_get_ep_event_count(vaddr_t iobase, int p_epnum);
bool dw_usb_ll_clear_ep_event(vaddr_t iobase, int p_epnum,uint16_t size);

bool dw_usb_ll_set_address(vaddr_t iobase, uint8_t addr);
int dw_usb_ll_get_cur_speed(vaddr_t iobase);
u32 dw_usb_ll_get_ep_transfer_index(vaddr_t iobase, int p_epnum);
int dw_usb_ll_disconnect_evt(vaddr_t iobase);
int dw_usb_ll_wakeup(vaddr_t iobase);
void dw_usb_ll_enable_irq(vaddr_t iobase);
void dw_usb_ll_disable_irq(vaddr_t iobase);
void dw_usb_ll_mask_irq(vaddr_t iobase);
void dw_usb_ll_unmask_irq(vaddr_t iobase);
void dump_registers(vaddr_t iobase,const char *pre);
uint32_t dw_usb_ll_get_ncr_evt(vaddr_t iobase);
void dw_usb_ll_clr_ncr_evt(vaddr_t iobase);

#endif
