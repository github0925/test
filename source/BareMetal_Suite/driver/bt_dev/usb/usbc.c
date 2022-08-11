/*
* usbc.c
*
* Copyright (c) 2018 Semidrive Semiconductor.
* All rights reserved.
*
* Description: implement usbc api for lk usb framework
*
* Revision History:
* -----------------
* 011, 3/8/2019 chenqing create this file
*/

#include <soc_usb.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <dev/usb.h>
#include <dev/usbc.h>
#include <dw_usb.h>

#if !defined(USB_INTS)
#define USB_INTS 0
#endif

status_t usbc_set_active(bool active)
{
    LTRACEF("active %u\n", active);

    return dw_usbc_set_active(USB_INTS, active);
}

void usbc_set_address(uint8_t address)
{
    LTRACEF("%s: address %u\n", __FUNCTION__, address);
    dw_usbc_set_address(USB_INTS, address);
}

void usbc_ep0_ack(void)
{
    USBDBG_L1("%s being called.\n", __FUNCTION__);
    dw_usbc_ep0_ack(USB_INTS);
}

void usbc_ep0_stall(void)
{
    DBG("%s being called.\n", __FUNCTION__);
    dw_usbc_ep0_stall(USB_INTS);
}

void usbc_ep0_send(const void *buf, size_t len, size_t maxlen)
{
    USBDBG_L1("%s: buf %p, len %d, maxlen %d\n", __FUNCTION__, buf, len, maxlen);
    dw_usbc_ep0_send(USB_INTS, buf, len, maxlen);
}

void usbc_ep0_recv(void *buf, size_t len, ep_callback cb)
{
    PANIC_UNIMPLEMENTED;
}

status_t usbc_setup_endpoint(ep_t ep, ep_dir_t dir, uint width, ep_type_t type)
{
    LTRACEF("%s: ep %u dir %u width %u\n", __FUNCTION__, ep, dir, width);

    return dw_usbc_setup_endpoint(USB_INTS, ep, dir, width, type, false, false);
}

bool usbc_is_highspeed(void)
{
    return dw_usbc_is_highspeed(USB_INTS);
}

status_t usbc_queue_rx(ep_t ep, usbc_transfer_t *transfer)
{
    USBDBG_L1("%s: ep %u, transfer %p (buf %p, buflen %d)\n", __FUNCTION__, ep,
              transfer, transfer->buf, transfer->buflen);

    return dw_usbc_queue_rx(USB_INTS, ep, transfer);
}

status_t usbc_queue_tx(ep_t ep, usbc_transfer_t *transfer)
{
    USBDBG_L1("%s: ep %u, transfer %p (buf %p, buflen %d)\n", __FUNCTION__, ep,
              transfer, transfer->buf, transfer->buflen);

    return dw_usbc_queue_tx(USB_INTS, ep, transfer);
}

void usb_early_init(void)
{
    vaddr_t iobase = (vaddr_t)APB_USB1_BASE;
    vaddr_t phybase = (vaddr_t)APB_USBPHY1_BASE;
    DW_USB_INIT Init;

    memset(&Init, 0, sizeof(Init));
    Init.dev_endpoints = 2;
    Init.ep0_mps = 512;
    Init.phy_itface = 0;    //PCD_PHY_EMBEDDED;
    Init.speed = USB_SPEED_FS;
    Init.low_power_enable = 0;
    Init.lpm_enable = 0;
    Init.battery_charging_enable = 0;

// #if !defined(VTEST_USB)
//     /* to tigger usb uvm sequence, for RTL test only */
//     writel(1, USBSS_TB_CTRL_BASE_ADDR);
// #endif

    dw_usbc_init(USB_INTS, iobase, phybase, Init);
}
