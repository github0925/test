/*
 * Copyright (c) 2008-2015 Travis Geiselbrecht
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
#include <debug.h>
#include <trace.h>
#include <err.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <list.h>
#include <lk/init.h>
#include "res.h"
#include "hal_usb.h"
#include "usbc.h"
#include "lib/reg.h"
#include "irq.h"
#include <arch/defines.h>

#define LOCAL_TRACE 0

typedef struct {
    struct list_node node;
    usb_callback_t cb;
    void *cookie;
} usb_callback_container_t;

static void hal_usb_do_callbacks(hal_usb_callback_op_t op,
                                 const struct hal_usb_callback_args *args);

uint32_t usb_irq[] = {
    0,
    USB1_O_USB1_USB3_INTERRUPT_NUM,
    USB2_O_USB2_USB3_INTERRUPT_NUM
};

static void hal_append_desc_data(hal_usb_descriptor *desc, const void *dat,
                                 size_t len)
{
    uint8_t *ptr = memalign(CACHE_LINE, ROUNDUP(desc->len + len, CACHE_LINE));

    if (ptr == NULL) {
        desc->desc = NULL;
        return;
    }

    memcpy(ptr, desc->desc, desc->len);
    memcpy(ptr + desc->len, dat, len);

    /* free the old buffer if it wasn't marked static */
    if ((desc->flags & USB_DESC_FLAG_STATIC) == 0) {
        free(desc->desc);
    }

    desc->flags &= ~USB_DESC_FLAG_STATIC;
    desc->desc = ptr;
    desc->len += len;
}

static uint8_t hal_usb_get_current_iface_num(const hal_usb_descriptor
        *desc)
{
    DEBUG_ASSERT(desc);
    return ((uint8_t *)desc->desc)[4];
}

uint8_t hal_usb_get_current_iface_num_highspeed(usb_t *usb)
{
    return hal_usb_get_current_iface_num(&usb->config->highspeed.config);
}

uint8_t hal_usb_get_current_iface_num_lowspeed(usb_t *usb)
{
    return hal_usb_get_current_iface_num(&usb->config->lowspeed.config);
}

/* returns the interface number assigned */
static int hal_usb_append_interface(hal_usb_descriptor *desc,
                                    const uint8_t *int_descr, size_t len)
{
    uint8_t *ptr = memalign(CACHE_LINE, ROUNDUP(len, CACHE_LINE));
    int interface_num;
    // create a temporary copy of the interface
    memcpy(ptr, int_descr, len);
    // find the last interface used
    interface_num = hal_usb_get_current_iface_num(desc);  // current interface
    // patch our interface descriptor with the new id
    ptr[2] = interface_num;
    // append it to our config descriptor
    hal_append_desc_data(desc, ptr, len);
    free(ptr);
    // patch the total length of the config descriptor and set the number of interfaces
    ((uint16_t *)desc->desc)[1] += len;
    interface_num++;
    ((uint8_t *)desc->desc)[4] = interface_num;
    return interface_num - 1;
}

int hal_usb_append_interface_highspeed(usb_t *usb,
                                       const uint8_t *int_descr, size_t len)
{
    return hal_usb_append_interface(&usb->config->highspeed.config, int_descr,
                                    len);
}

int hal_usb_append_interface_lowspeed(usb_t *usb, const uint8_t *int_descr,
                                      size_t len)
{
    return hal_usb_append_interface(&usb->config->lowspeed.config, int_descr,
                                    len);
}

void hal_usb_set_string_descriptor(hal_usb_descriptor *desc,
                                   const char *string)
{
    int len = strlen(string);
    ushort *data;
    int datalen = len * 2 + 2;
    data = memalign(CACHE_LINE, ROUNDUP(datalen, CACHE_LINE));

    if (data == NULL) {
        desc->desc = NULL;
        return;
    }

    /* write length field */
    data[0] = 0x0300 + datalen;
    /* copy the string into the uint16_t based usb string */
    int i;

    for (i = 0; i < len; i++) {
        data[i + 1] = string[i];
    }

    desc->desc = (void *)data;
    desc->len = datalen;
}

void hal_set_usb_id(usb_t *usb, uint16_t vendor, uint16_t product,
                    uint16_t version)
{
    // patch the current configuration to with the vendor/product id
    ((uint16_t *)usb->config->lowspeed.device.desc)[4] = vendor;
    ((uint16_t *)usb->config->lowspeed.device.desc)[5] = product;
    ((uint16_t *)usb->config->lowspeed.device.desc)[6] = version;
    ((uint16_t *)usb->config->highspeed.device.desc)[4] = vendor;
    ((uint16_t *)usb->config->highspeed.device.desc)[5] = product;
    ((uint16_t *)usb->config->highspeed.device.desc)[6] = version;
}

status_t hal_usb_add_string(usb_t *usb, const char *string, uint8_t id)
{
    uint i;
    size_t len = strlen(string);
    uint16_t *strbuf = memalign(CACHE_LINE, ROUNDUP(len * 2 + 2, CACHE_LINE));

    if (!strbuf) {
        return ERR_NO_MEMORY;
    }

    /* build the usb string descriptor */
    strbuf[0] = 0x300 | (len * 2 + 2);

    for (i = 0; i < len; i++) {
        strbuf[i + 1] = (uint16_t)string[i];
    }

    /* find a slot to put it */
    for (i = 0; i < MAX_STRINGS; i++) {
        if (usb->strings[i].id == 0) {
            usb->strings[i].string.desc = strbuf;
            usb->strings[i].string.len = len * 2 + 2;
            usb->strings[i].id = id;
            return NO_ERROR;
        }
    }

    /* couldn't find a spot */
    free(strbuf);
    return ERR_NO_MEMORY;
}

static void hal_usb_set_active_config(usb_t *usb, uint8_t config)
{
    struct hal_usb_callback_args args;
    args.usb = usb;

    if (config != usb->active_config) {
        usb->active_config = config;

        if (usb->active_config != 0) {
            printf("usb online\n");
            hal_usb_do_callbacks(USB_CB_ONLINE, &args);
        }
        else {
            printf("usb offline\n");
            hal_usb_do_callbacks(USB_CB_OFFLINE, &args);
        }
    }
}

status_t hal_usb_register_callback(usb_t *usb, usb_callback_t cb,
                                   void *cookie)
{
    DEBUG_ASSERT(cb);
    usb_callback_container_t *c = malloc(sizeof(usb_callback_container_t));

    if (!c) {
        return ERR_NO_MEMORY;
    }

    c->cb = cb;
    c->cookie = cookie;
    list_add_tail(&usb->cb_list, &c->node);
    return NO_ERROR;
}

static void hal_usb_do_callbacks(hal_usb_callback_op_t op,
                                 const struct hal_usb_callback_args *args)
{
    usb_t *usb = args->usb;
    usb_callback_container_t *c;
    list_for_every_entry(&usb->cb_list, c, usb_callback_container_t, node) {
        c->cb(c->cookie, op, args);
    }
}

status_t hal_usbc_callback(hal_usb_callback_op_t op,
                           const struct hal_usb_callback_args *args)
{
    LTRACEF("op %d, args %p\n", op, args);
    usb_t *usb = args->usb;

    /* start looking for specific things to handle */
    if (op == USB_CB_SETUP_MSG) {
        bool setup_handled = false;
        const struct usb_setup *setup = args->setup;
        DEBUG_ASSERT(setup);
        LTRACEF("SETUP: req_type=%#x req=%#x value=%#x index=%#x len=%#x\n",
                setup->request_type, setup->request, setup->value, setup->index,
                setup->length);

        if ((setup->request_type & TYPE_MASK) == TYPE_STANDARD) {
            switch (setup->request) {
                case SET_ADDRESS:
                    LTRACEF("SET_ADDRESS 0x%x\n", setup->value);
                    usbc_ep0_ack(usb);
                    usbc_set_address(usb, setup->value);
                    setup_handled = true;
                    break;

                case SET_FEATURE:
                case CLEAR_FEATURE:
                    LTRACEF("SET/CLEAR_FEATURE, feature 0x%x\n", setup->value);
                    usbc_ep0_ack(usb);
                    setup_handled = true;
                    break;

                case SET_DESCRIPTOR:
                    LTRACEF("SET_DESCRIPTOR\n");
                    usbc_ep0_stall(usb);
                    setup_handled = true;
                    break;

                case GET_DESCRIPTOR: {
                    if ((setup->request_type & RECIP_MASK) == RECIP_DEVICE) {
                        /* handle device descriptor fetches */
                        /* Get the right descriptors based on current speed */
                        const struct hal_usb_descriptor_speed *speed;

                        if (usbc_is_highspeed(usb)) {
                            speed = &usb->config->highspeed;
                        }
                        else {
                            speed = &usb->config->lowspeed;
                        }

                        switch (setup->value) {
                            case 0x100: /* device */
                                LTRACEF("got GET_DESCRIPTOR, device descriptor\n");
                                usbc_ep0_send(usb, speed->device.desc,
                                              speed->device.len, setup->length);
                                break;

                            case 0x200:    /* CONFIGURATION */
                                LTRACEF("got GET_DESCRIPTOR, config descriptor\n");
                                usbc_ep0_send(usb, speed->config.desc,
                                              speed->config.len, setup->length);
                                break;

                            case 0x300:    /* Language ID */
                                LTRACEF("got GET_DESCRIPTOR, language id\n");
                                usbc_ep0_send(usb, usb->config->langid.desc,
                                              usb->config->langid.len, setup->length);
                                break;

                            case (0x301)...(0x3ff): {
                                /* string descriptor, search our list for a match */
                                uint i;
                                bool found = false;
                                uint8_t id = setup->value & 0xff;

                                for (i = 0; i < MAX_STRINGS; i++) {
                                    if (usb->strings[i].id == id) {
                                        usbc_ep0_send(usb, usb->strings[i].string.desc,
                                                      usb->strings[i].string.len,
                                                      setup->length);
                                        found = true;
                                        break;
                                    }
                                }

                                if (!found) {
                                    /* couldn't find one, stall */
                                    usbc_ep0_stall(usb);
                                }

                                break;
                            }

                            case 0x600:    /* DEVICE QUALIFIER */
                                LTRACEF("got GET_DESCRIPTOR, device qualifier\n");
                                usbc_ep0_send(usb, speed->device_qual.desc,
                                              speed->device_qual.len, setup->length);
                                break;

                            case 0xa00:
                                /* we aint got one of these */
                                LTRACEF("got GET_DESCRIPTOR, debug descriptor\n");
                                usbc_ep0_stall(usb);
                                break;

                            default:
                                LTRACEF("unhandled descriptor %#x\n", setup->value);
                                // stall
                                break;
                        }

                        setup_handled = true;
                    }

                    break;
                }

                case SET_CONFIGURATION:
                    LTRACEF("SET_CONFIGURATION %d\n", setup->value);
                    usbc_ep0_ack(usb);
                    hal_usb_set_active_config(usb, setup->value);
                    break;

                case GET_CONFIGURATION:
                    LTRACEF("GET_CONFIGURATION\n");
                    usbc_ep0_send(usb, &usb->active_config, 1, setup->length);
                    break;

                case SET_INTERFACE:
                    LTRACEF("SET_INTERFACE %d\n", setup->value);
                    usbc_ep0_ack(usb);
                    break;

                case GET_INTERFACE: {
                    static uint8_t __SECTION(".nocache") i = 1;
                    LTRACEF("GET_INTERFACE\n");
                    usbc_ep0_send(usb, &i, 1, setup->length);
                    break;
                }

                case GET_STATUS: {
                    static uint16_t __SECTION(".nocache") i = 1; // self powered
                    LTRACEF("GET_STATUS\n");
                    usbc_ep0_send(usb, &i, 2, setup->length);
                    break;
                }

                default:
                    LTRACEF("unhandled standard request 0x%x\n", setup->request);
            }
        }

        if (!setup_handled) {
            hal_usb_do_callbacks(op, args);
        }
    }
    else if (op == USB_CB_RESET) {
        LTRACEF("USB_CB_RESET\n");
        hal_usb_do_callbacks(op, args);
        usb->active_config = 0;
        hal_usb_do_callbacks(USB_CB_OFFLINE, args);
    }
    else {
        // other non setup messages, pass them down to anyone else
        hal_usb_do_callbacks(op, args);
    }

    return NO_ERROR;
}

enum handler_return hal_usb_irq(void *args)
{
    return usbc_irq(args);
}

status_t hal_usb_setup(usb_t *usb, hal_usb_config *config)
{
    DEBUG_ASSERT(config);
    DEBUG_ASSERT(usb->active == false);
    usb->config = config;
    return NO_ERROR;
}

status_t hal_usb_start(usb_t *usb)
{
    DEBUG_ASSERT(usb->config);
    DEBUG_ASSERT(usb->active == false);
    // go online
    usbc_set_active(usb, true);
    usb->active = true;
    return NO_ERROR;
}

status_t hal_usb_stop(usb_t *usb)
{
    DEBUG_ASSERT(usb->active == true);
    usb->active = false;
    usbc_set_active(usb, false);
    return NO_ERROR;
}

void hal_usb_init(usb_t *usb)
{
    usbc_init(usb);
}

status_t hal_usb_read(usb_t *usb, ep_t ep, usbc_transfer_t *transfer)
{
    return usbc_queue_rx(usb, ep, transfer);
}


status_t hal_usb_write(usb_t *usb, ep_t ep, usbc_transfer_t *transfer)
{
    return usbc_queue_tx(usb, ep, transfer);
}

int32_t hal_usb_creat_handle(usb_t *usb, uint32_t global_base_id,
                             uint32_t global_phy_id)
{
    int32_t ret;
    ret = res_get_info_by_id(global_base_id, &usb->priv.usb_paddr,
                             &usb->priv.instance_id);

    if (ret < 0) {
        return ret;
    }

    ret = res_get_info_by_id(global_phy_id, &usb->priv.usb_phy_paddr,
                             &usb->priv.instance_phy_id);

    if (ret < 0) {
        return ret;
    }

    usb->priv.usb_vaddr  = (vaddr_t)_ioaddr(usb->priv.usb_paddr);
    usb->priv.usb_phy_vaddr = (vaddr_t)_ioaddr(usb->priv.usb_phy_paddr);
    usb->priv.irq_num = usb_irq[usb->priv.instance_id];
    return 0;
}

int32_t hal_usb_release_handle(usb_t *usb)
{
    usb->priv.usb_vaddr = 0;
    usb->priv.usb_phy_vaddr = 0;
    usb->priv.irq_num = 0;
    return 0;
}

bool hal_usb_is_highspeed(usb_t *usb)
{
    return usbc_is_highspeed(usb);
}

status_t hal_usb_setup_endpoint(usb_t *usb, ep_t ep, ep_dir_t dir,
                                uint width, ep_type_t type)
{
    return usbc_setup_endpoint(usb, ep, dir, width, type);
}
