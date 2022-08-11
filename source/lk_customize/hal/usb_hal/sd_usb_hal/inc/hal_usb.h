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
#ifndef __HAL_USB_H
#define __HAL_USB_H

#include <sys/types.h>
#include <hw/usb.h>
#include <compiler.h>

__BEGIN_CDECLS

#define MAX_STRINGS 8

#define USB_SPEED_HS               0
#define USB_SPEED_FS               1
#define USB_SPEED_SS               4
#define USB_SPEED_SSPLUS           5

typedef uint ep_t;

typedef enum {
	USB_IN = 0,
	USB_OUT
} ep_dir_t;

typedef enum {
	USB_CTRL = 0x00,
	USB_ISOC = 0x01,
	USB_BULK = 0x02,
	USB_INTR = 0x03,
} ep_type_t;


/* top level initialization for usb client, abstracts away the interfaces */
typedef struct {
	void *desc;
	size_t len;
	uint flags;
} hal_usb_descriptor __ALIGNED(2);

#define USB_DESC_FLAG_STATIC (0x1)

#define USB_DESC_STATIC(x) { .desc = (void *)(x), .len = sizeof(x), .flags = USB_DESC_FLAG_STATIC }

/* callbacks from usbc and usb layers */
typedef enum {
	USB_CB_RESET,
	USB_CB_SUSPEND,
	USB_CB_RESUME,
	USB_CB_DISCONNECT,
	USB_CB_ONLINE,
	USB_CB_OFFLINE,
	USB_CB_SETUP_MSG,
} hal_usb_callback_op_t;

struct usbc_transfer;
typedef status_t (*ep_callback)(ep_t endpoint, struct usbc_transfer *transfer);

typedef struct usbc_transfer {
	ep_callback callback;
	status_t result;
	void *buf;
	size_t buflen;
	uint bufpos;
	void *extra; // extra pointer to store whatever you want
} usbc_transfer_t;

typedef struct {
	hal_usb_descriptor string;
	uint8_t id;
} hal_usb_string;

/* complete usb config struct, passed in to usb_setup() */
typedef struct {
	struct hal_usb_descriptor_speed {
		hal_usb_descriptor device;
		hal_usb_descriptor device_qual;
		hal_usb_descriptor config;
	} lowspeed, highspeed;
	hal_usb_descriptor langid;
} hal_usb_config;

typedef struct {
	uint32_t irq_num;
	int instance_id;
	int instance_phy_id;
	uint16_t vendor_id;
	uint16_t product_id;
	uint16_t version_id;
	const char *manufacturer;
	const char *product;
	const char *serialno;
	const char *cfg;
	paddr_t usb_phy_paddr;
	paddr_t usb_paddr;
	vaddr_t usb_phy_vaddr;
	vaddr_t usb_vaddr;
	uint32_t max_eps;
	uint32_t ep0_mps;
} usb_priv_t;

typedef struct {
	uint32_t global_base_id;
	uint32_t global_phys_id;
	bool active;
	uint8_t active_config;
	hal_usb_config *config;
	struct list_node cb_list;
	hal_usb_string strings[MAX_STRINGS];
	usb_priv_t priv;
} usb_t;


/* setup arg is valid during CB_SETUP_MSG */
struct hal_usb_callback_args {
	const struct usb_setup *setup;
	usb_t *usb;
};

typedef status_t (*usb_callback_t)(void *cookie, hal_usb_callback_op_t op, const struct hal_usb_callback_args *args);

/* external code needs to set up the usb stack via the following calls */
status_t hal_usb_setup(usb_t *usb, hal_usb_config *config);
void hal_usb_init(usb_t *usb);
enum handler_return hal_usb_irq(void *args);

bool hal_usb_is_highspeed(usb_t *usb);
status_t hal_usb_setup_endpoint(usb_t *usb, ep_t ep, ep_dir_t dir, uint width, ep_type_t type);

/* Returns the Interface Number that will be assigned to the next interface that
   is registered using usb_append_interface_(.*) */
uint8_t hal_usb_get_current_iface_num_highspeed(usb_t *usb);
uint8_t hal_usb_get_current_iface_num_lowspeed(usb_t *usb);

/* apped new interface descriptors to the existing config if desired */
status_t hal_usb_append_interface_highspeed(usb_t *usb, const uint8_t *int_descr, size_t len);
status_t hal_usb_append_interface_lowspeed(usb_t *usb, const uint8_t *int_descr, size_t len);

status_t hal_usb_add_string(usb_t *usb, const char *string, uint8_t id);
void hal_set_usb_id(usb_t *usb, uint16_t vendor, uint16_t product, uint16_t version);

status_t hal_usb_start(usb_t *usb);
status_t hal_usb_stop(usb_t *usb);

/* callback api the usbc driver uses */
status_t hal_usbc_callback(hal_usb_callback_op_t op, const struct hal_usb_callback_args *args);

/* callback api that anyone can register for */
status_t hal_usb_register_callback(usb_t *usb, usb_callback_t, void *cookie);

status_t hal_usb_read(usb_t *usb, ep_t ep, usbc_transfer_t *transfer);
status_t hal_usb_write(usb_t *usb, ep_t ep, usbc_transfer_t *transfer);

int32_t hal_usb_creat_handle(usb_t *usb, uint32_t global_base_id, uint32_t global_phys_id);
int32_t hal_usb_release_handle(usb_t *usb);

__END_CDECLS

#endif

