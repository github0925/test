/*
*
* Copyright (c) 2018 Semidrive Semiconductor.
* All rights reserved.
*
* Description: implement fastboot class
*
* Revision History:
* -----------------
* 011, 8/14/2019 lujianyong create this file
*/
#include <debug.h>
#include <err.h>
#include <list.h>
#include <platform/interrupts.h>
#include <stdlib.h>
#include <string.h>
#include <compiler.h>
#include <arch/ops.h>
#include "class_fastboot.h"

#define LOCAL_TRACE         0
#define INTERFACE_NUM       1
#define ENDPOINT_IN_ADDR    1
#define ENDPOINT_OUT_ADDR   1
#define W(w)                (w & 0xff), (w >> 8)
#define W3(w)               (w & 0xff), ((w >> 8) & 0xff), ((w >> 16) & 0xff)

#define MAX_USBFS_BULK_SIZE (32 * 1024)

uint8_t fb_cmd_buf[ROUNDUP(512, CACHE_LINE)] __ALIGNED(CACHE_LINE);

/* top level device descriptor */
static uint8_t __SECTION(".nocache") dev_descr[] = {
    0x12,           /* descriptor length */
    DEVICE,         /* Device Descriptor type */
    W(0x0200),      /* USB Version */
    0xff,           /* class */
    0xff,           /* subclass */
    0xff,           /* protocol */
    64,             /* max packet size, ept0 */
    W(0x18d1),      /* vendor */
    W(0x4d00),      /* product */
    W(0x0100),      /* release */
    0x1,            /* manufacturer string */
    0x2,            /* product string */
    0x3,            /* serialno string */
    0x1,            /* num configs */
};

/* high/low speed device qualifier */
static const uint8_t devqual_descr[] = {
    0x0a,           /* len */
    DEVICE_QUALIFIER,/* Device Qualifier type */
    W(0x0200),      /* USB version */
    0x00,           /* class */
    0x00,           /* subclass */
    0x00,           /* protocol */
    64,             /* max packet size, ept0 */
    0x01,           /* num configs */
    0x00            /* reserved */
};

static const uint8_t  cfg_descr[] = {
    0x09,           /* Length of Cfg Descr */
    CONFIGURATION,  /* Type of Cfg Descr */
    W(0x09),        /* Total Length (incl ifc, ept) */
    0x00,           /* # Interfaces */
    0x01,           /* Cfg Value */
    0x04,           /* Cfg String */
    0xc0,           /* Attributes -- self powered */
    250,            /* Power Consumption - 500mA */
};

static const uchar langid[] = { 0x04, 0x03, 0x09, 0x04 };
static hal_usb_config config = {
    .lowspeed = {
        .device = USB_DESC_STATIC(dev_descr),
        .device_qual = USB_DESC_STATIC(devqual_descr),
        .config = USB_DESC_STATIC(cfg_descr),
    },
    .highspeed = {
        .device = USB_DESC_STATIC(dev_descr),
        .device_qual = USB_DESC_STATIC(devqual_descr),
        .config = USB_DESC_STATIC(cfg_descr),
    },

    .langid = USB_DESC_STATIC(langid),
};

/* build a descriptor for it */
static uint8_t __SECTION(".nocache") fs_if_descriptor[]  = {
    0x09,               /* length */
    INTERFACE,          /* type */
    INTERFACE_NUM,      /* interface num */
    0x00,               /* alternates */
    0x02,               /* endpoint count */
    0xff,               /* interface class */
    0x42,               /* interface subclass */
    0x03,               /* interface protocol */
    0x00,               /* string index */

    /* endpoint 1 IN */
    0x07,               /* length */
    ENDPOINT,           /* type */
    ENDPOINT_IN_ADDR | 0x80, /* address: 1 IN */
    0x02,               /* type: bulk */
    W(64),              /* max packet size: 64 */
    0x00,               /* interval */

    /* endpoint 1 OUT */
    0x07,               /* length */
    ENDPOINT,           /* type */
    ENDPOINT_OUT_ADDR,  /* address: 1 OUT */
    0x02,               /* type: bulk */
    W(64),              /* max packet size: 64 */
    0x00,               /* interval */
};

/* build a descriptor for it */
static uint8_t __SECTION(".nocache") hs_if_descriptor[] = {
    0x09,               /* length */
    INTERFACE,          /* type */
    INTERFACE_NUM,      /* interface num */
    0x00,               /* alternates */
    0x02,               /* endpoint count */
    0xff,               /* interface class */
    0x42,               /* interface subclass */
    0x03,               /* interface protocol */
    0x00,               /* string index */

    /* endpoint 1 IN */
    0x07,               /* length */
    ENDPOINT,           /* type */
    ENDPOINT_IN_ADDR | 0x80, /* address: 1 IN */
    0x02,               /* type: bulk */
    W(512),             /* max packet size: 512 */
    0x00,               /* interval */

    /* endpoint 1 OUT */
    0x07,               /* length */
    ENDPOINT,           /* type */
    ENDPOINT_OUT_ADDR,  /* address: 1 OUT */
    0x02,               /* type: bulk */
    W(512),             /* max packet size: 512 */
    0x00,               /* interval */
};

struct fastboot_cmd {
    struct fastboot_cmd *next;
    const char *prefix;
    unsigned prefix_len;
    void (*handle)(fastboot_t *fb, const char *arg, void *data, unsigned sz);
};

struct fastboot_var {
    struct fastboot_var *next;
    const char *name;
    const char *value;
};

extern void hal_winusb_init(usb_t *usb, uint16_t vid, uint16_t pid,
                            uint16_t version);
static struct fastboot_cmd *cmdlist;
static struct fastboot_var *varlist;

void fastboot_register_cmd(const char *prefix,
                           void (*handle)(fastboot_t *fb, const char *arg, void *data, unsigned sz))
{
    struct fastboot_cmd *cmd;
    cmd = malloc(sizeof(*cmd));

    if (cmd) {
        cmd->prefix = prefix;
        cmd->prefix_len = strlen(prefix);
        cmd->handle = handle;
        cmd->next = cmdlist;
        cmdlist = cmd;
    }
}

void fastboot_register_var(const char *name, const char *value)
{
    struct fastboot_var *var;
    var = malloc(sizeof(*var));

    if (var) {
        var->name = name;
        var->value = value;
        var->next = varlist;
        varlist = var;
    }
}

static void fastboot_command_loop(fastboot_t *fb)
{
    struct fastboot_cmd *cmd;
    int r;
    dprintf(ALWAYS, "fastboot: processing commands\n");
again:

    while (fb->state != STATE_ERROR) {
        memset(fb_cmd_buf, 0, MAX_RSP_SIZE);
        arch_clean_invalidate_cache_range((addr_t)fb_cmd_buf, MAX_RSP_SIZE);
        r = fb->usb_read((void *)fb, fb_cmd_buf, MAX_RSP_SIZE);

        if (r < 0) {
            break;
        }

        fb_cmd_buf[r] = 0;
        dprintf(INFO, "fastboot cmd: %s\n", fb_cmd_buf);
        fb->state = STATE_COMMAND;

        for (cmd = cmdlist; cmd; cmd = cmd->next) {
            if (memcmp(fb_cmd_buf, cmd->prefix, cmd->prefix_len)) {
                continue;
            }

            cmd->handle(fb, (const char *)fb_cmd_buf + cmd->prefix_len,
                        (void *)fb->priv.download_base, fb->priv.download_size);

            if (fb->state == STATE_COMMAND) {
                fastboot_fail(fb, "unknown reason");
            }

            goto again;
        }

        fastboot_fail(fb, "unknown command");
    }

    fb->state = STATE_OFFLINE;
    dprintf(ALWAYS, "fastboot: oops!\n");
}

int fastboot_handler(void *args)
{
    fastboot_t *fb = (fastboot_t *)args;

    for (;;) {
        event_wait(&fb->priv.usb_online);
        fastboot_command_loop(fb);
    }

    return 0;
}


__WEAK void fastboot_ack(fastboot_t *fb, const char *code,
                         const char *reason)
{
    STACKBUF_DMA_ALIGN(response, MAX_RSP_SIZE);

    if (fb->state != STATE_COMMAND) {
        return;
    }

    if (reason == 0) {
        reason = "";
    }

    snprintf((char *)response, MAX_RSP_SIZE, "%s%s", code, reason);
    fb->state = STATE_COMPLETE;
    fb->usb_write((void *)fb, response, strlen((const char *)response));
}

__WEAK void fastboot_info(fastboot_t *fb, const char *reason)
{
    STACKBUF_DMA_ALIGN(response, MAX_RSP_SIZE);

    if (fb->state != STATE_COMMAND || reason == NULL) {
        return;
    }

    snprintf((char *)response, MAX_RSP_SIZE, "INFO%s", reason);
    fb->usb_write((void *)fb, response, strlen((const char *)response));
}

__WEAK void fastboot_fail(fastboot_t *fb, const char *reason)
{
    fastboot_ack(fb, "FAIL", reason);
}

__WEAK void fastboot_okay(fastboot_t *fb, const char *info)
{
    fastboot_ack(fb, "OKAY", info);
}

static void getvar_all(fastboot_t *fb)
{
    struct fastboot_var *var;
    char getvar_all[256];
    memset((void *) getvar_all, '\0', sizeof(getvar_all));

    for (var = varlist; var; var = var->next) {
        strlcpy((char *) getvar_all, var->name, sizeof(getvar_all));
        strlcat((char *) getvar_all, ":", sizeof(getvar_all));
        strlcat((char *) getvar_all, var->value, sizeof(getvar_all));
        fastboot_info(fb, getvar_all);
    }

    fastboot_okay(fb, "");
}

void cmd_getvar(fastboot_t *fb, const char *arg, void *data, unsigned sz)
{
    struct fastboot_var *var;

    if (!strncmp("all", arg, strlen(arg))) {
        getvar_all(fb);
        return;
    }

    for (var = varlist; var; var = var->next) {
        if (!strcmp(var->name, arg)) {
            fastboot_okay(fb, var->value);
            return;
        }
    }

    fastboot_okay(fb, "");
}

__WEAK void cmd_flash(fastboot_t *fb, const char *arg, void *data,
                      unsigned sz)
{
    fastboot_okay(fb, "");
}

__WEAK void cmd_download(fastboot_t *fb, const char *arg, void *data,
                         unsigned sz)
{
    STACKBUF_DMA_ALIGN(response, MAX_RSP_SIZE);
    int r;
    char *tmp;
    unsigned long len = strtoul(arg, &tmp, 16);
    fb->priv.download_size = 0;

    if (len > fb->priv.download_max) {
        fastboot_fail(fb, "data too large");
        return;
    }

    snprintf((char *)response, MAX_RSP_SIZE, "DATA%08x", len);

    if (fb->usb_write((void *)fb, response,
                      strlen((const char *)response)) < 0) {
        return;
    }

    arch_invalidate_cache_range((addr_t)fb->priv.download_base, ROUNDUP(len,
                                CACHE_LINE));
    r = fb->usb_read((void *)fb, fb->priv.download_base, len);

    if ((r < 0) || ((unsigned) r != len)) {
        fb->state = STATE_ERROR;
        return;
    }

    fb->priv.download_size = len;
    fastboot_okay(fb, "");
}

__WEAK status_t fastboot_online(void *cookie)
{
    dprintf(INFO, "%s E\n", __func__);
    fastboot_t *fb = (fastboot_t *)cookie;
    event_signal(&fb->priv.usb_online, 0);
    return 0;
}

__WEAK status_t fastboot_offline(void *cookie)
{
    dprintf(INFO, "%s E\n", __func__);
    return 0;
}

static status_t usb_read_complete(ep_t ep, usbc_transfer_t *t)
{
    fastboot_t *fb = (fastboot_t *)t->extra;
    fb->priv.usb_read_status = t->result;

    if (t->bufpos > t->buflen || t->bufpos == 0) {
        fb->priv.usb_read_status = ERR_IO;
    }

    fb->priv.actual_read_len = t->bufpos;
    event_signal(&fb->priv.usb_read_done, false);
    return NO_ERROR;
}

int fastboot_usb_read(void *handle, void *_buf, unsigned len)
{
    int r = 0;
    unsigned xfer = 0;
    unsigned char *buf = _buf;
    int count = 0;
    usbc_transfer_t rx;
    fastboot_t *fb = (fastboot_t *)handle;

    if (fb->state == STATE_ERROR) {
        goto oops;
    }

    while (len > 0) {
        xfer = (len > MAX_USBFS_BULK_SIZE) ? MAX_USBFS_BULK_SIZE : len;
        rx.buf      = buf;
        rx.buflen   = xfer;
        rx.callback = usb_read_complete;
        rx.result   = 0;
        rx.bufpos   = 0;
        rx.extra    = (void *)fb;
        r = hal_usb_read(&fb->usb, fb->priv.read_ep, &rx);

        if (r < 0) {
            dprintf(ALWAYS, "usb_read() queue failed\n");
            goto oops;
        }

        event_wait(&fb->priv.usb_read_done);

        if (fb->priv.usb_read_status < 0) {
            dprintf(ALWAYS, "usb_read() transaction failed\n");
            goto oops;
        }

        count += fb->priv.actual_read_len;
        buf   += fb->priv.actual_read_len;
        len   -= fb->priv.actual_read_len;

        /* short transfer? */
        if (fb->priv.actual_read_len != xfer) {
            break;
        }
    }

    arch_invalidate_cache_range((addr_t)_buf, ROUNDUP(count, CACHE_LINE));
    return count;
oops:
    fb->state = STATE_ERROR;
    return -1;
}

static status_t usb_write_complete(ep_t ep, usbc_transfer_t *t)
{
    fastboot_t *fb = (fastboot_t *)t->extra;
    fb->priv.usb_write_status = t->result;
    fb->priv.actual_write_len = t->bufpos;

    if (t->bufpos > t->buflen || t->bufpos == 0) {
        fb->priv.usb_write_status = -1;
    }

    event_signal(&fb->priv.usb_write_done, false);
    return NO_ERROR;
}

int fastboot_usb_write(void *handle, void *_buf, unsigned len)
{
    int r = 0;
    uint32_t xfer = 0;
    unsigned char *buf = _buf;
    int count = 0;
    usbc_transfer_t tx;
    fastboot_t *fb = (fastboot_t *)handle;

    if (fb->state == STATE_ERROR) {
        goto oops;
    }

    while (len > 0) {
        xfer = (len > MAX_USBFS_BULK_SIZE) ? MAX_USBFS_BULK_SIZE : len;
        tx.buf      = buf;
        tx.buflen   = xfer;
        tx.callback = usb_write_complete;
        tx.result   = 0;
        tx.bufpos   = 0;
        tx.extra    = (void *)fb;
        r = hal_usb_write(&fb->usb, fb->priv.write_ep, &tx);

        if (r < 0) {
            dprintf(ALWAYS, "usb_write() queue failed\n");
            goto oops;
        }

        event_wait(&fb->priv.usb_write_done);

        if (fb->priv.usb_write_status < 0) {
            dprintf(ALWAYS, "usb_write() transaction failed\n");
            goto oops;
        }

        count += fb->priv.actual_write_len;
        buf   += fb->priv.actual_write_len;
        len   -= fb->priv.actual_write_len;

        /* short transfer? */
        if (fb->priv.actual_write_len != xfer) { break; }
    }

    return count;
oops:
    fb->state = STATE_ERROR;
    return -1;
}


static status_t fastboot_usb_cb(void *cookie, hal_usb_callback_op_t op,
                                const struct hal_usb_callback_args *args)
{
    int mps = 64;
    fastboot_t *fb = (fastboot_t *)cookie;
    dprintf(INFO, "%s cookie %p, op %u, args %p\n", __func__, cookie, op,
            args);

    if (op == USB_CB_ONLINE) {
        if (hal_usb_is_highspeed(&fb->usb)) {
            mps = 512;
        }

        hal_usb_setup_endpoint(&fb->usb, fb->priv.write_ep, USB_IN, mps, USB_BULK);
        hal_usb_setup_endpoint(&fb->usb, fb->priv.read_ep, USB_OUT, mps, USB_BULK);

        if (fb->cb.online) {
            fb->cb.online(cookie);
        }
    }
    else if (op == USB_CB_DISCONNECT) {
        if (fb->cb.offline) {
            fb->cb.offline(cookie);
        }
    }

    return NO_ERROR;
}


void fastboot_init(void *args)
{
    int ret;
    fastboot_t *fb = (fastboot_t *)args;
    usb_t *usb = &fb->usb;
    fb->state = STATE_OFFLINE;
    fb->priv.read_ep = ENDPOINT_OUT_ADDR;
    fb->priv.write_ep = ENDPOINT_IN_ADDR;
    usb->priv.max_eps = 8;
    usb->priv.ep0_mps = 512;
    event_init(&fb->priv.usb_online, 0, EVENT_FLAG_AUTOUNSIGNAL);
    event_init(&fb->priv.usb_write_done, 0, EVENT_FLAG_AUTOUNSIGNAL);
    event_init(&fb->priv.usb_read_done, 0, EVENT_FLAG_AUTOUNSIGNAL);
    list_initialize(&usb->cb_list);
    hal_usb_creat_handle(usb, usb->global_base_id, usb->global_phys_id);
    hal_usb_init(usb);
    hal_usb_setup(usb, &config);
    hal_usb_add_string(usb, usb->priv.manufacturer, 1);
    hal_usb_add_string(usb, usb->priv.product, 2);
    hal_usb_add_string(usb, usb->priv.serialno, 3);
    hal_usb_add_string(usb, usb->priv.cfg, 4);
    hal_usb_append_interface_lowspeed(usb, fs_if_descriptor,
                                      sizeof(fs_if_descriptor));
    hal_usb_append_interface_highspeed(usb, hs_if_descriptor,
                                       sizeof(hs_if_descriptor));
    hal_usb_register_callback(usb, fastboot_usb_cb, (void *)fb);
    hal_winusb_init(usb, usb->priv.vendor_id, usb->priv.product_id,
                    usb->priv.version_id);
    register_int_handler(usb->priv.irq_num, &hal_usb_irq, (void *)usb);
    unmask_interrupt(usb->priv.irq_num);
    hal_usb_start(usb);
}

void fastboot_stop(void *args)
{
    fastboot_t *fb = (fastboot_t *)args;
    mask_interrupt(fb->usb.priv.irq_num);
    hal_usb_stop(&fb->usb);
    hal_usb_release_handle(&fb->usb);
}

