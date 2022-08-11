/*
 * Copyright (c) 2008 Travis Geiselbrecht
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#ifndef __DEV_USBC_H
#define __DEV_USBC_H

#include <compiler.h>
#include <stdbool.h>
#include <stdio.h>
#include <sys/types.h>
#include "hal_usb.h"

__BEGIN_CDECLS

enum {
	USB_TRANSFER_RESULT_OK = 0,
	USB_TRANSFER_RESULT_ERR = -1,
	USB_TRANSFER_RESULT_CANCELLED = -2,
};

void usbc_init(usb_t *usb);
enum handler_return usbc_irq(void *args);

status_t usbc_setup_endpoint(usb_t *usb, ep_t ep, ep_dir_t dir, uint width, ep_type_t type);
status_t usbc_queue_rx(usb_t *usb, ep_t ep, usbc_transfer_t *transfer);
status_t usbc_queue_tx(usb_t *usb, ep_t ep, usbc_transfer_t *transfer);
status_t usbc_flush_ep(usb_t *usb, ep_t ep);

status_t usbc_set_active(usb_t *usb, bool active);
void usbc_set_address(usb_t *usb, uint8_t address);

/* called back from within a callback to handle setup responses */
void usbc_ep0_ack(usb_t *usb);
void usbc_ep0_stall(usb_t *usb);
void usbc_ep0_send(usb_t *usb, const void *buf, size_t len, size_t maxlen);
void usbc_ep0_recv(usb_t *usb, void *buf, size_t len, ep_callback);

bool usbc_is_highspeed(usb_t *usb);

static inline void usbc_dump_transfer(const usbc_transfer_t *t)
{
	printf("usb transfer %p: cb %p buf %p, buflen %zd, bufpos %u, result %d\n", t, t->callback, t->buf, t->buflen, t->bufpos, t->result);
}

__END_CDECLS

#endif

