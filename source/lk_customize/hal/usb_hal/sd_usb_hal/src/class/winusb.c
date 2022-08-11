
#include <assert.h>
#include <err.h>
#include <lk/list.h>
#include <stdbool.h>
#include "hal_usb.h"
#include "usbc.h"

#define EXTENDED_COMPATIBILITY_ID   4
#define EXTENDED_PROPERTIES_ID      5

#define IS_VALID_USB_STD_REQUEST(r) \
    (((r) == GET_STATUS) || ((r) == CLEAR_FEATURE) || ((r) == SET_FEATURE) \
     || ((r) >= SET_ADDRESS && (r) <= SET_INTERFACE))
#define USB_STR_OS_LEN 18

typedef struct {
    uint8_t inf_num;
    uint8_t rsvd1;
    char id[8];   /* String describing the compatible id */
    char sub_id[8];
    uint8_t rsvd2[6];
} __PACKED ext_comp_inf_desc_t;

typedef struct {
    uint32_t len;
    uint16_t bcdVer;
    uint16_t index; /* 04 for extended compatibility id */
    uint8_t count;  /* Number of interfaces for which an extended compatibility
                       feature desriptor is defined */
    uint8_t rsvd[7];
    ext_comp_inf_desc_t inf_desc;
} __PACKED ext_comp_desc_t;

typedef struct {
    uint32_t len;
    uint32_t type;
    uint16_t name_len;
    uint16_t name[6];
    uint32_t data_len;
    uint16_t data[16];
} __PACKED prop_section0_t;

typedef struct {
    uint32_t len;
    uint16_t ver;
    uint16_t index;
    uint16_t cnt;
    /* Only one properties supported here */
    prop_section0_t section0;
} __PACKED ext_prop_desc_t;

static ext_comp_desc_t __SECTION(".nocache") ext_comp_desc = {
    sizeof(ext_comp_desc_t),
    0x0100,
    EXTENDED_COMPATIBILITY_ID,
    1,
    {0, 0, 0, 0, 0, 0, 0},
    {
        0,
        1, /* Reserved for system use. Set this value to 0x01 */
        "WINUSB\0",
        {0},
    }
};

static ext_prop_desc_t __SECTION(".nocache") ext_prop_desc = {
    sizeof(ext_prop_desc),
    0x0100,
    EXTENDED_PROPERTIES_ID,
    1,
    {
        sizeof(prop_section0_t),
        1,  /* Data Type: REG_SZ */
        6 * 2,
        {'L', 'a', 'b', 'e', 'l', 0},
        16 * 2,
        {'F', 'a', 's', 't', 'B', 'o', 'o', 't', ' ', 'D', 'e', 'v', 'i', 'c', 'e', 0},
    },
};

static unsigned char usb_str_os[] = {
    USB_STR_OS_LEN,
    0x30,   /* String Descriptor */
    'M', 0, 'S', 0, 'F', 0, 'T', 0, '1', 0, '0', 0, '0', 0,
    1,      /* bMS_VerdorCode */
    0,      /* bPad */
};

static status_t winusb_dev_event_cb(void *cookie, hal_usb_callback_op_t op,
                                    const struct hal_usb_callback_args *args)
{
    usb_t *usb = args->usb;
    dprintf(INFO, "%s op:%d\n", __FUNCTION__, op);

    if (op == USB_CB_SETUP_MSG) {
        const struct usb_setup *setup = args->setup;
        DEBUG_ASSERT(setup);

        if (((setup->request_type & TYPE_MASK) == TYPE_VENDOR)
                && ((setup->request_type & RECIP_MASK) == RECIP_DEVICE)
                && ((setup->request_type & DIR_MASK) == DIR_IN)
                && (setup->request == 0x01)
                && (setup->index == EXTENDED_COMPATIBILITY_ID)) {
            usbc_ep0_send(usb, (void *)&ext_comp_desc,
                          (size_t)sizeof(ext_comp_desc), setup->length);
            dprintf(INFO, "%s: Get windows OS feature descriptor.\n",
                    __FUNCTION__);
        }
        else if (((setup->request_type & TYPE_MASK) == TYPE_VENDOR)
                 && ((setup->request_type & RECIP_MASK) == RECIP_INTERFACE)
                 && ((setup->request_type & DIR_MASK) == DIR_IN)
                 && (setup->request == 0x01)
                 && ((setup->index == EXTENDED_PROPERTIES_ID)
                     || setup->index == 0)) {
            usbc_ep0_send(usb, (void *)&ext_prop_desc,
                          (size_t)sizeof(ext_prop_desc), setup->length);
            dprintf(INFO, "%s: Get windows extended properties descriptor.\n",
                    __FUNCTION__);
        }
        else if (((setup->request_type & TYPE_MASK) != TYPE_STANDARD)
                 || ((setup->request_type & TYPE_MASK) == TYPE_STANDARD
                     && !IS_VALID_USB_STD_REQUEST(setup->request))) {
            usbc_ep0_stall(usb);
            dprintf(CRITICAL,
                    "%s: unsupported SETUP: req_type=%#x req=%#x value=%#x index=%#x len=%#x\n",
                    __FUNCTION__, setup->request_type, setup->request, setup->value,
                    setup->index, setup->length);
        }
    }

    return NO_ERROR;
}

void hal_winusb_init(usb_t *usb, uint16_t vid, uint16_t pid, uint16_t ver)
{
    hal_set_usb_id(usb, vid, pid, ver);
    hal_usb_add_string(usb, (const char *)usb_str_os, 238);
    hal_usb_register_callback(usb, winusb_dev_event_cb, NULL);
}
