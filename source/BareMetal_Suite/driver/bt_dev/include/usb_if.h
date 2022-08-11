#ifndef  USB_IF_H
#define  USB_IF_H

#include <common_hdr.h>
#include <soc.h>
#include <arch.h>
#include "soc_usb.h"

void usb_dev_init(void);
void usb_dev_deinit(void);
U32 usb_recv(U8 *to, U32 sz);
U32 usb_send(U8 *from, U32 sz);
U32 usb_send_zero(void);

#endif

