/********************************************************
 *	    Copyright(c) 2019	Semidrive  Semiconductor    *
 *	    All rights reserved.                            *
 ********************************************************/

/*
 * This driver follows the EUB (easy usb boot) protocal.
 *
 * USB configuration shall be as followed:
 *  EP0         default pipe
 *  EP1_IN      BULK_IN         mps(max packet size)  512B for HS and 64 for FS.
 *  EP1_OUT     BULK_OUT        mps(max packet size)  512B for HS and 64 for FS.
 *
 * Boot Process
 *  Host
 *      Sends Image Header (2KB, may be a different mps aligned number on a different SOC) to device.
 *      Sends Image (which size been specified in Image Header) to device.
 *
 *  Device
 *      Receives Image Header.
 *      Validity check on Image Header.
 *      Get image size (Img_SZ) from Image Header, then receive data in Img_Sz bytes.
 *      Validity check on the image.
 */

#include <soc_usb.h>
#include <dev/usbc.h>
#include <dev/usb.h>
#include <dw_usb.h>
#include <wdog/wdog.h>
#include <srv_timer/srv_timer.h>

#define EXTENDED_COMPATIBILITY_ID   4
#define EXTENDED_PROPERTIES_ID      5

#define EZUSB_TX_TMOUT_us   (1000U * 1000U * 60U * 3)

#define EZBOOT_IS_VALID_USB_STD_REQUEST(r) \
    (((r) == GET_STATUS) || ((r) == CLEAR_FEATURE) || ((r) == SET_FEATURE) \
     || ((r) >= SET_ADDRESS && (r) <= SET_INTERFACE))

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
    uint16_t data[14];
} __PACKED prop_section0_t;

typedef struct {
    uint32_t len;
    uint16_t ver;
    uint16_t index;
    uint16_t cnt;
    /* Only one properties supported here */
    prop_section0_t section0;
} __PACKED ext_prop_desc_t;

static usbc_transfer_t rx_xfer __IN_BSS2__;
static usbc_transfer_t tx_xfer __IN_BSS2__;
static U8 g_pkt_buf[512] __IN_BSS2__ __ALIGNED(CACHE_LINE);
static U32 g_pos_in = 0, g_pos_out = 0;
static int mps=64;
static bool ezusb_online = false;

static ext_comp_desc_t ext_comp_desc = {
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

static ext_prop_desc_t ext_prop_desc = {
    sizeof(ext_prop_desc),
    0x0100,
    EXTENDED_PROPERTIES_ID,
    1,
    {
        sizeof(prop_section0_t),
        1,  /* Data Type: REG_SZ */
        6 * 2,
        {'L', 'a', 'b', 'e', 'l', 0},
        14 * 2,
        {'E', 'z', 'B', 'o', 'o', 't', ' ', 'D', 'D', 'R', 'T', 'S', 'T', 0},
    },
};

void cancel_tansfers(ep_t ep,bool dir)
{
    dw_usbc_cancel_tansfers(0,ep,dir);
}

static status_t queue_rx(usbc_transfer_t* xfer)
{
    xfer->result = ERR_GENERIC;
    xfer->bufpos = 0;
    xfer->extra = 0;

    return usbc_queue_rx(1, xfer);
}

static status_t queue_tx(usbc_transfer_t* xfer)
{
    xfer->result = ERR_GENERIC;
    xfer->bufpos = 0;
    xfer->extra = 0;

    return usbc_queue_tx(1, xfer);
}

bool ezusb_is_online(void)
{
    return (ezusb_online == true);
}


int ezusb_tx(char *buffer, unsigned int size)
{
    U64 tk_tmot;
    if(!size){
        return 0 ;
    }else if(!ezusb_is_online()) {
        USBDBG_L1("%s: Opps, USB not online\n", __FUNCTION__);
        return -1;
    }

    tx_xfer.buf = buffer;
    tx_xfer.buflen = size;
    tx_xfer.bufpos = 0;
    tx_xfer.tran_sta = USB_TRANSFER_STATUS_START;

    if(!queue_tx(&tx_xfer)){
        return -4;
    }
    tk_tmot = tmr_tick() + SOC_us_TO_TICK(EZUSB_TX_TMOUT_us);
    while(1){
        if (!ezusb_is_online()) {
            USBDBG_L1("%s: Opps, USB not online\n", __FUNCTION__);
            return -1;
        } else if (ERR_CANCELLED == tx_xfer.result) {
            USBDBG_L1("%s: Opps, xfer error.\n", __FUNCTION__);
            return -2;
        } else if (tx_xfer.tran_sta == USB_TRANSFER_STATUS_FINISH) {
            // mini_printf("%s: %d bytes send \n\n\n", __FUNCTION__,tx_xfer.bufpos);
            return tx_xfer.bufpos;
        }else {
            if(tmr_tick() >= tk_tmot){
                cancel_tansfers(1,true);
                return -3;
            }
        }
    }
    return -5;
}

static int ezusb_tx_callback(ep_t ep, usbc_transfer_t *t)
{
    USBDBG_L1("%s being called...\n", __FUNCTION__);

    return NO_ERROR;
}

static uint64_t rx_overall = 0;

int ezusb_rx(U8 *to, size_t sz)
{
    USBDBG_L1("%s:entry: %d bytes to receive...\n", __FUNCTION__, (int)sz);
    size_t bytes_rx = 0;

    if(!sz){
        return 0 ;
    }else if(!ezusb_is_online()) {
        USBDBG_L1("%s: Opps, USB not online\n", __FUNCTION__);
        return -1;
    }
        
    rx_xfer.buf = g_pkt_buf;
    rx_xfer.buflen = sizeof(g_pkt_buf);
    rx_xfer.tran_sta = USB_TRANSFER_STATUS_START;
    if(!queue_rx(&rx_xfer)){
        return -4;
    }
    U64 tk_tmot = tmr_tick() + SOC_us_TO_TICK(EZUSB_TX_TMOUT_us);
    while(ezusb_is_online()){
        if (!ezusb_is_online()) {
            USBDBG_L1("%s: Opps, USB not online\n", __FUNCTION__);
            return -5;
        } else if (ERR_CANCELLED == rx_xfer.result) {
            USBDBG_L1("%s: Opps, xfer error.\n", __FUNCTION__);
            return -6;
        } else if (NO_ERROR == rx_xfer.result) {
            U32 n = rx_xfer.bufpos;
            rx_overall += rx_xfer.bufpos;
            USBDBG_L1("%s: %d bytes rx-ed.\n", __FUNCTION__, n);
            g_pos_in = n;
            g_pos_out = 0;
            USBDBG_L1("%s:pkt_buf: in=%u, out=%u\n", __FUNCTION__, g_pos_in, g_pos_out);
            if ((0 != n) && (sz > 0)) {
                U32 cp_sz = MIN(sz, n);
                mini_memcpy_s(to, g_pkt_buf, cp_sz);
                USBDBG_L1("%s: cp %d bytes from pkt buffer\n", __FUNCTION__, (int)cp_sz);
                bytes_rx += cp_sz;
                g_pos_out += cp_sz;
                sz -= cp_sz;
            }
            if(rx_xfer.tran_sta == USB_TRANSFER_STATUS_FINISH)
            {
                USBDBG_L1("%s: %d bytes recved\n\n\n", __FUNCTION__,bytes_rx);
                return bytes_rx;
            }
        }else{
             if(tmr_tick() >= tk_tmot){
                cancel_tansfers(1,true);
                return -3;
            }
        }
    }
    return -5;
}

static status_t ezusb_rx_callback(ep_t ep, usbc_transfer_t *t)
{
    USBDBG_L1("%s being called...\n", __FUNCTION__);

    return NO_ERROR;
}

static status_t ezusb_dev_event_cb(void *cookie, usb_callback_op_t op,
                                   const union usb_callback_args *args)
{
    USBDBG_L1("%s: cookie %p, op %u, args %p\n", __FUNCTION__, cookie, op, args);

    if (op == USB_CB_ONLINE) {
        rx_xfer.buf = NULL;
        rx_xfer.callback = &ezusb_rx_callback;
        rx_xfer.buflen = 0;

        tx_xfer.buf = NULL;
        tx_xfer.callback = &ezusb_tx_callback;
        tx_xfer.buflen = 0;
        if (usbc_is_highspeed()) {
            mps = 512;
        }

        usbc_setup_endpoint(1, USB_IN, mps, USB_BULK);
        usbc_setup_endpoint(1, USB_OUT, mps, USB_BULK);

        ezusb_online = true;
    } else if (op == USB_CB_OFFLINE) {
        ezusb_online = false;
        rx_xfer.buf = NULL;
        rx_xfer.callback = &ezusb_rx_callback;
        rx_xfer.buflen = 0;

        tx_xfer.buf = NULL;
        tx_xfer.callback = &ezusb_tx_callback;
        tx_xfer.buflen = 0;
    } else if (op == USB_CB_DISCONNECT) {
        ezusb_online = false;
    } else if (op == USB_CB_SETUP_MSG) {
        const struct usb_setup *setup = args->setup;
        DEBUG_ASSERT(setup);
        if (((setup->request_type & TYPE_MASK) == TYPE_VENDOR) 
                && ((setup->request_type & RECIP_MASK) == RECIP_DEVICE)
                && ((setup->request_type & DIR_MASK) == DIR_IN)
                && (setup->request == 0x01)
                && (setup->index == EXTENDED_COMPATIBILITY_ID)){
                usbc_ep0_send((void*)&ext_comp_desc, 
                        sizeof(ext_comp_desc), setup->length);
                USBDBG_L2("%s: Get windows OS feature descriptor.\n",
                        __FUNCTION__);
        } else if (((setup->request_type & TYPE_MASK) == TYPE_VENDOR)
                && ((setup->request_type & RECIP_MASK) == RECIP_INTERFACE) 
                && ((setup->request_type & DIR_MASK) == DIR_IN)
                && (setup->request == 0x01)
                && ((setup->index == EXTENDED_PROPERTIES_ID)
                    || setup->index == 0)) {
                usbc_ep0_send((void*)&ext_prop_desc, 
                        sizeof(ext_prop_desc), setup->length);
                USBDBG_L2("%s: Get windows extended properties descriptor.\n",
                        __FUNCTION__);
        } else if (((setup->request_type & TYPE_MASK) != TYPE_STANDARD)
                    || (!EZBOOT_IS_VALID_USB_STD_REQUEST(setup->request))){
            usbc_ep0_stall();
            USBDBG_L2("%s: unsupported SETUP: req_type=%#x req=%#x value=%#x index=%#x len=%#x\n",
                  __FUNCTION__, setup->request_type, setup->request, setup->value,
                  setup->index, setup->length);
        }
    }

    return NO_ERROR;
}

int ezusb_init(U32 interface_num, ep_t epin, ep_t epout)
{
    LTRACEF("%s: epin %u, epout %u\n", __FUNCTION__, epin, epout);

    usb_register_callback(&ezusb_dev_event_cb, NULL);

    return 0;
}
