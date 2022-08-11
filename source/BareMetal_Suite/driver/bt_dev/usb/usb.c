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

#include <soc_usb.h>
#include <list.h>
#include <stdlib.h>
#include <string.h>
#include <dev/usbc.h>
#include <dev/usb.h>

#if !defined(USB_MAX_STRINGS)
#define USB_MAX_STRINGS 5
#endif

static struct {
    bool active;
    uint8_t active_config;
    usb_config *config;
    struct list_node cb_list;
    usb_string strings[USB_MAX_STRINGS];
} usb __IN_BSS2__;

typedef struct {
    struct list_node node;
    usb_callback_t cb;
    void *cookie;
} usb_callback_container_t;

static usb_callback_container_t g_usb_callback_c[2] __IN_BSS2__;

static void usb_do_callbacks(usb_callback_op_t op,
                             const union usb_callback_args *args);

static uint8_t usb_get_current_iface_num(const usb_descriptor *desc)
{
    DEBUG_ASSERT(desc);

    return ((uint8_t *)desc->desc)[4];
}

uint8_t usb_get_current_iface_num_highspeed(void)
{
    return usb_get_current_iface_num(&usb.config->highspeed.config);
}

uint8_t usb_get_current_iface_num_lowspeed(void)
{
    return usb_get_current_iface_num(&usb.config->lowspeed.config);
}

status_t usb_add_string(const char *string, uint8_t id)
{
    for (int i = 0; i < USB_MAX_STRINGS; i++) {
        if (usb.strings[i].id == 0) {
            usb.strings[i].string.desc = (void*)string;
            usb.strings[i].string.len = string[0];
            usb.strings[i].id = id;
            return NO_ERROR;
        }
    }
    return ERR_NO_MEMORY;
}

static void usb_set_active_config(uint8_t config)
{
    if (config != usb.active_config) {
        usb.active_config = config;
        if (usb.active_config != 0) {
            USBDBG_L2("%s: usb online\n", __FUNCTION__);
            usb_do_callbacks(USB_CB_ONLINE, NULL);
        } else {
            USBDBG_L2("%s: usb offline\n", __FUNCTION__);
            usb_do_callbacks(USB_CB_OFFLINE, NULL);
        }
    }
}

void set_usb_id(uint16_t vendor, uint16_t product)
{
    ((uint16_t *)usb.config->lowspeed.device.desc)[4] = vendor;
    ((uint16_t *)usb.config->lowspeed.device.desc)[5] = product;

    ((uint16_t *)usb.config->highspeed.device.desc)[4] = vendor;
    ((uint16_t *)usb.config->highspeed.device.desc)[5] = product;
}

status_t usb_register_callback(usb_callback_t cb, void *cookie)
{
    DEBUG_ASSERT(cb);
#if !defined(CFG_USB_STATIC_BUFs)
    usb_callback_container_t *c = malloc(sizeof(usb_callback_container_t));
    if (!c)
        return ERR_NO_MEMORY;
#else
    static U32 created = 0;
    if (created >= sizeof(g_usb_callback_c)/sizeof(g_usb_callback_c[0])) {
        return ERR_NO_MEMORY;
    }
    usb_callback_container_t *c = &g_usb_callback_c[created++];
#endif
    c->cb = cb;
    c->cookie = cookie;
    list_add_tail(&usb.cb_list, &c->node);

    return NO_ERROR;
}

static void usb_do_callbacks(usb_callback_op_t op,
                             const union usb_callback_args *args)
{
    usb_callback_container_t *c;
    list_for_every_entry(&usb.cb_list, c, usb_callback_container_t, node) {
        c->cb(c->cookie, op, args);
    }
}

status_t usbc_callback(usb_callback_op_t op,
                       const union usb_callback_args *args)
{
    if (op == USB_CB_SETUP_MSG) {
        bool setup_handled = false;
        const struct usb_setup *setup = args->setup;
        DEBUG_ASSERT(setup);

        USBDBG_L1("%s: SETUP: req_type=%#x req=%#x value=%#x index=%#x len=%#x\n",
                  __FUNCTION__, setup->request_type, setup->request, setup->value,
                  setup->index, setup->length);

        if ((setup->request_type & TYPE_MASK) == TYPE_STANDARD) {
            switch (setup->request) {
            case SET_ADDRESS:
                usbc_ep0_ack();
                usbc_set_address(setup->value);
                setup_handled = true;
                USBDBG_L2("%s:SET_ADDRESS 0x%x\n", __FUNCTION__, setup->value);
                break;
            case SET_FEATURE:
            case CLEAR_FEATURE:
                usbc_ep0_ack();
                setup_handled = true;
                USBDBG_L2("%s:SET/CLEAR_FEATURE, feature 0x%x\n", __FUNCTION__, setup->value);
                break;
            case SET_DESCRIPTOR:
                usbc_ep0_stall();
                setup_handled = true;
                USBDBG_L2("%s:SET_DESCRIPTOR\n", __FUNCTION__);
                break;
            case GET_DESCRIPTOR: {
                if ((setup->request_type & RECIP_MASK) == RECIP_DEVICE) {
                    const struct usb_descriptor_speed *speed = NULL;
                    if (usbc_is_highspeed()) {
                        speed = &usb.config->highspeed;
                    } else {
                        speed = &usb.config->lowspeed;
                    }

                    switch (setup->value) {
                    case 0x100: /* device */
                        usbc_ep0_send(speed->device.desc, speed->device.len,
                                      setup->length);
                        USBDBG_L2("%s:GET_DESCRIPTOR, device\n", __FUNCTION__);
                        break;
                    case 0x200:    /* CONFIGURATION */
                        usbc_ep0_send(speed->config.desc, speed->config.len,
                                      setup->length);
                        USBDBG_L2("%s:GET_DESCRIPTOR, config\n", __FUNCTION__);
                        break;
                    case 0x300:    /* Language ID */
                        usbc_ep0_send(usb.config->langid.desc,
                                      usb.config->langid.len, setup->length);
                        USBDBG_L2("%s:GET_DESCRIPTOR, language id\n", __FUNCTION__);
                        break;
                    case (0x301)...(0x3ff): {
                        bool found = false;
                        uint8_t id = setup->value & 0xff;
                        for (uint i = 0; i < USB_MAX_STRINGS; i++) {
                            if (usb.strings[i].id == id) {
                                usbc_ep0_send(usb.strings[i].string.desc,
                                              usb.strings[i].string.len,
                                              setup->length);
                                found = true;
                                break;
                            }
                        }
                        if (!found) {
                            USBDBG_L3("%s: Opps, invalid string id %d\n",
                                      __FUNCTION__, id);
                            usbc_ep0_stall();
                        }
                        USBDBG_L2("%s:GET_DESCRIPTOR, string %d\n", __FUNCTION__, id);
                        break;
                    }
                    case 0x600:    /* DEVICE QUALIFIER */
                        usbc_ep0_send(speed->device_qual.desc,
                                      speed->device_qual.len, setup->length);
                        USBDBG_L2("%s:GET_DESCRIPTOR, device qualifier\n", __FUNCTION__);
                        break;
                    case 0xa00:
                        usbc_ep0_stall();
                        USBDBG_L2("%s:GET_DESCRIPTOR, debug descriptor\n", __FUNCTION__);
                        break;
                    default:
                        usbc_ep0_stall();
                        USBDBG_L3("%s: unsupoorted descriptor %#x\n", __FUNCTION__, setup->value);
                        break;
                    }
                    setup_handled = true;
                }
                break;
            }

            case SET_CONFIGURATION:
                usbc_ep0_ack();
                usb_set_active_config(setup->value);
                USBDBG_L2("%s:SET_CONFIGURATION %d\n", __FUNCTION__, setup->value);
                break;

            case GET_CONFIGURATION:
                usbc_ep0_send(&usb.active_config, 1, setup->length);
                USBDBG_L2("%s:GET_CONFIGURATION\n", __FUNCTION__);
                break;

            case SET_INTERFACE:
                usbc_ep0_ack();
                USBDBG_L2("%s:SET_INTERFACE %d\n", __FUNCTION__, setup->value);
                break;

            case GET_INTERFACE: {
                static uint8_t v = 1;
                usbc_ep0_send(&v, 1, setup->length);
                USBDBG_L2("%s:GET_INTERFACE\n", __FUNCTION__);
                break;
            }

            case GET_STATUS: {
                static uint16_t v = 1; // self powered
                usbc_ep0_send(&v, 2, setup->length);
                USBDBG_L2("%s:GET_STATUS\n", __FUNCTION__);
                break;
            }
            default:
                USBDBG_L2("%s: unhandled standard request 0x%x\n",
                          __FUNCTION__, setup->request);
            }
        }
        if (!setup_handled) {
            usb_do_callbacks(op, args);
        }
    } else if (op == USB_CB_RESET) {
        usb_do_callbacks(op, args);
        usb.active_config = 0;
        usb_do_callbacks(USB_CB_OFFLINE, args);
    } else {
        usb_do_callbacks(op, args);
    }

    return NO_ERROR;
}

status_t usb_setup(usb_config *config)
{
    DEBUG_ASSERT(config);
    DEBUG_ASSERT(usb.active == false);

    usb.config = config;

    return NO_ERROR;
}

status_t usb_start(void)
{
    USBDBG_L2("%s being called.\n", __FUNCTION__);
    DEBUG_ASSERT(usb.config);
    DEBUG_ASSERT(usb.active == false);

    usbc_set_active(true);
    usb.active = true;

    return NO_ERROR;
}

status_t usb_stop(void)
{
    DEBUG_ASSERT(usb.active == true);

    usb.active = false;
    usbc_set_active(false);

    return NO_ERROR;
}

void usb_init(uint level)
{
    list_initialize(&usb.cb_list);
}
