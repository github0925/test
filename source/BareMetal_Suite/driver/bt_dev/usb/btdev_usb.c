/********************************************************
 *	    Copyright(c) 2019	Semidrive  Semiconductor    *
 *	    All rights reserved.                            *
 ********************************************************/

#include <soc_usb.h>
#include <dev/usbc.h>
#include <dev/usb.h>
#include <dw_usb.h>
#include <wdog/wdog.h>
#include <ezusb.h>
#include "plat_interrupt.h"
#include "drivers/arm/gicv2.h"
#include "plat_gic.h"
#include <stdarg.h>
#include <arch.h>

extern void usb_early_init(void);
// extern void soc_update_prod_str(uint16_t *str_prod);

#define W(w) (w & 0xff), (w >> 8)
#define W3(w) (w & 0xff), ((w >> 8) & 0xff), ((w >> 16) & 0xff)

/* top level device descriptor */
static uint8_t dev_descr[] = {
    0x12,           /* descriptor length */
    DEVICE,         /* Device Descriptor type */
    W(0x0200),      /* USB Version */
    0xff,           /* class */
    0xff,           /* subclass */
    0xff,           /* protocol */
    64,             /* max packet size, ept0 */
    W(0x3273),      /* vendor */
    W(0x0001),      /* product */
#if defined(TGT_ap)
    W(0x0101),      /* release */
#elif defined(TGT_sec)
    W(0x0102),      /* release */
#endif
    0x1,            /* manufacturer string */
    0x2,            /* product string */
    0x3,            /* serialno string */
    0x1,            /* num configs */
};

/* high/low speed device qualifier */
static uint8_t devqual_descr[] = {
    0x0a,           /* len */
    DEVICE_QUALIFIER, /* Device Qualifier type */
    W(0x0200),      /* USB version */
    0x00,           /* class */
    0x00,           /* subclass */
    0x00,           /* protocol */
    64,             /* max packet size, ept0 */
    0x01,           /* num configs */
    0x00            /* reserved */
};

#include "if_desc.h"

static uint8_t fs_cfg_descr[] = {
    0x09,           /* Length of Cfg Descr */
    CONFIGURATION,  /* Type of Cfg Descr */
#if !defined(CFG_USB_STATIC_BUFs)
    W(0x09),        /* Total Length (incl ifc, ept) */
#else
    W((0x09 + FS_IF_DESC_LEN)),
#endif
#if !defined(CFG_USB_STATIC_BUFs)
    0x00,           /* # Interfaces */
#else
    0x01,
#endif
    0x01,           /* Cfg Value */
    0x00,           /* Cfg String */
    0xc0,           /* Attributes -- self powered */
    250,            /* Power Consumption - 500mA */
#if defined(CFG_USB_STATIC_BUFs)
    FS_IF_DESC
#endif
};

static uint8_t hs_cfg_descr[] = {
    0x09,           /* Length of Cfg Descr */
    CONFIGURATION,  /* Type of Cfg Descr */
#if !defined(CFG_USB_STATIC_BUFs)
    W(0x09),        /* Total Length (incl ifc, ept) */
#else
    W((0x09 + HS_IF_DESC_LEN)),
#endif
#if !defined(CFG_USB_STATIC_BUFs)
    0x00,           /* # Interfaces */
#else
    0x01,
#endif
    0x01,           /* Cfg Value */
    0x00,           /* Cfg String */
    0xc0,           /* Attributes -- self powered */
    250,            /* Power Consumption - 500mA */
#if defined(CFG_USB_STATIC_BUFs)
    HS_IF_DESC
#endif
};

static uchar langid[] = { 0x04, 0x03, 0x09, 0x04 };

usb_config config = {
    .lowspeed = {
        .device = USB_DESC_STATIC(dev_descr),
        .device_qual = USB_DESC_STATIC(devqual_descr),
        .config = USB_DESC_STATIC(fs_cfg_descr),
    },
    .highspeed = {
        .device = USB_DESC_STATIC(dev_descr),
        .device_qual = USB_DESC_STATIC(devqual_descr),
        .config = USB_DESC_STATIC(hs_cfg_descr),
    },

    .langid = USB_DESC_STATIC(langid),
};

#if defined(CFG_USB_STATIC_BUFs)
#include "usb_string.c.h"
#endif

static void ezusb_set_serial_str(uint16_t *usb_str_desc_serial,
                                 uint8_t *sn, U32 sn_sz)
{
    if ((NULL == sn) || (NULL == usb_str_desc_serial)
        || ((sn_sz*2 + 2) > (usb_str_desc_serial[0] & 0xff))) {
        DBG("%s: invalid argument.\n", __FUNCTION__);
        return;
    }
    uint8_t *p = (uint8_t *)&usb_str_desc_serial[1];
    for (int i = 0; i < (sn_sz * 2); i++, p += 2) {
        uint8_t c = (sn[i/2] >> (((i + 1)%2) * 4)) & 0x0fu;
        if ((c < 10)) {
            *p = c + '0';
        } else {
            *p = c - 10 + 'A';
        }
    }
}

static void soc_update_prod_str(uint16_t* str_desc_prod)
{
    if(NULL == str_desc_prod || 
        ((4*2+2)>(str_desc_prod[0]&0xff))){
            DBG("%s invalid argument.\n",__FUNCTION__);
        }
    U8 code[4];
    code[0] = FUSE_PROD_FAMILY_ID();
    code[1] = FUSE_PROD_SERIES_CODE();
    code[2] = FUSE_PROD_FEATURE_CODE();
    code[3] = FUSE_PROD_SPEED_GRADE();
    uint8_t *p = (uint8_t*)&str_desc_prod[1];
    for(int i=0;i<4;i++,p += 2){
        U8 c = code[i];
        if((c < 10)){
            *p = c + '0';
        }else if (c<16){
            *p = c -10 + '0';
        }
    }
}

U32 ezusb_open(bt_dev_id_e dev_id, void *para)
{
    if (FUSE_PROD_MAJOR_ID() | FUSE_PROD_MINOR_ID()) {
        soc_update_prod_str(usb_str_prod);
    }
    uint8_t uuid[8];
    memclr(uuid, sizeof(uuid));
    soc_read_uuid(uuid, 8);
    uint8_t non_zero = 0;
    for (int i = 0; i < sizeof(uuid); i++) {
        non_zero |= uuid[i];
    }
    if (non_zero) {
        ezusb_set_serial_str(usb_str_serial, uuid, 8);
    }

    //module_e m = soc_get_module(dev_id);
    module_e  m = dev_id;
    soc_dis_isolation(m);
    soc_deassert_reset(m);
    soc_config_clk(m, FREQ_DEFAULT);

    usb_init(0);

    usb_early_init();

    usb_setup(&config);

    if (0 != FUSE_USB_VID()) {
        set_usb_id(FUSE_USB_VID(), FUSE_USB_PID());
    }

    usb_add_string((const char*)usb_str_manufacturer, 1);
    usb_add_string((const char*)usb_str_prod, 2);
    usb_add_string((const char*)usb_str_serial, 3);
    if (FUSE_USB_MS_OS_DESC()) {
        usb_add_string((const char*)usb_str_os, 238);
    }

    ezusb_init(1, 1, 1);

    usb_start();

    U64 open_timeout = TIMEOUT_1min * 4;
    if (1 == FUSE_USB_OPEN_TIMEOUT()) {
        open_timeout = TIMEOUT_1min * 5;
    } else if (2 == FUSE_USB_OPEN_TIMEOUT()) {
        open_timeout = TIMEOUT_1min * 6;
    } else if (3 == FUSE_USB_OPEN_TIMEOUT()) {
        open_timeout = TIMEOUT_1min * 3;
    }
    U64 tick = tmr_tick();
    U64 tmot = tick + SOC_us_TO_TICK(open_timeout);
    if (tmr_tick() >= tmot) {
        return ATB_FAIL;
    } else {
        return ATB_SUCCESS;
    }
}

U32 usb_recv(U8 *to, U32 sz)
{
    int rx_bytes = ezusb_rx(to, sz);
    if(sz != rx_bytes) {
        USBDBG_L1("%s: Opps, %d bytes to read but %d got\n", __FUNCTION__, sz, rx_bytes);
    }
    return rx_bytes;
}

U32 usb_send(U8 *from, U32 sz)
{
    int tx_bytes,retry=3;
    while(1){
        tx_bytes = ezusb_tx(from, sz);
        if(tx_bytes == -3){
            if(!retry--){
                break;
            }
        }else if(tx_bytes == -4){
            continue;
        }else if(tx_bytes == sz){
            break;
        }
    }
    if(sz != tx_bytes) {
        mini_printf("%s: Opps, %d bytes to write but %d got\n", __FUNCTION__, sz, tx_bytes);
    }
    return tx_bytes;
}


static void usb_handle(void)
{
    dw_usb_irqhandler(0);
}

void usb_dev_init(void)
{
    soc_assert_reset(USB_CTRL1);
    soc_deassert_reset(GIC);
    udelay(1000*1000*2);
    plat_interrupt_init();
    plat_setup_interrupt(104, 0x80,usb_handle);
    plat_enable_interrupt(104);
    if(PLATFORM_CORE_COUNT>1){
        gicv2_set_spi_routing(104,0);
    } 
    ezusb_open(USB_CTRL1,NULL);   
    el3_fiq_enable();
}

void usb_dev_deinit(void)
{
#if defined (TGT_ap)
    local_fiq_disable();
#else
    arch_disable_interrupt();
#endif
    plat_disable_interrupt(104);  
    
}


