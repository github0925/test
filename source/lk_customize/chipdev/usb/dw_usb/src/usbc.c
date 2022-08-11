/*
* dw_usb.c
*
* Copyright (c) 2018 Semidrive Semiconductor.
* All rights reserved.
*
* Description: implement synopsys usb driver
*
* Revision History:
* -----------------
* 011, 03/08/2019 chenqing create this file
*/

#include <reg.h>
#include <err.h>
#include <assert.h>
#include <malloc.h>
#include <arch/ops.h>
#include <shared/lk/trace.h>
#include <shared/lk/macros.h>
#include <string.h>
#include <kernel/spinlock.h>
#include <arch/defines.h>
#include <kernel/vm.h>
#include <res.h>

#include "usbc.h"
#include "dw_usb_ll.h"

#define LOCAL_TRACE 0

#if WITH_KERNEL_VM
#define v2p(va)    (paddr_t)(vaddr_to_paddr(va))
#define p2v(pa)    (vaddr_t)(paddr_to_kvaddr(pa))
#else
#define v2p(va)    (paddr_t)(va)
#define p2v(pa)    (vaddr_t)(pa)
#endif


#if SD_USB_FOR_ROM
#define MAX_USB_NUM 1
#define NUM_EP 2
#define EVT_BUFF_SIZE (10*4)
#define SETUP_PKT_BUF_LEN 64
#else
#define MAX_USB_NUM 2
#define NUM_EP 8
#define EVT_BUFF_SIZE (1024*4)
#define SETUP_PKT_BUF_LEN 1024
#endif

#define CACHELINE_SIZE CACHE_LINE
#define TRB_ALIGN MAX(16,CACHE_LINE)

typedef enum  {
    DW_EP0_UNKNOWN = 0,
    DW_EP0_COMPLETE,
    DW_EP0_NRDY_DATA,
    DW_EP0_NRDY_STATUS,
} dw_ep0_next;

struct ep_status {
    bool ack_ep0_in;
    usbc_transfer_t *transfer;
};

typedef union {
    struct {
        uint32_t bufsize: 24;
        uint32_t pcm1: 2;
        uint32_t spr: 1;
        uint32_t rsvd: 1;
        uint32_t trbsts: 4;
    };
    uint32_t v;
} trb2_t;

typedef union {
    struct {
        uint32_t hwo: 1;
        uint32_t lst: 1;
        uint32_t chn: 1;
        uint32_t csp: 1;
        uint32_t trbctl: 6;
        uint32_t isp_imi: 1;
        uint32_t ioc: 1;
        uint32_t rsvd: 2;
        uint32_t sid: 16;
        uint32_t rsvd2: 2;
    };
    uint32_t v;
} trb3_t;

typedef struct {
    uint32_t bp_l;
    uint32_t bp_h;
    trb2_t trb2;
    trb3_t trb3;
} trb_t;

typedef struct {
    trb_t *trb;
    paddr_t paddr;
    uint32_t orig_size;
} trbd_t;

typedef struct {
    void *evtb;
    paddr_t paddr;
    int lpos;
    uint16_t size;
} evtbd_t;

typedef struct {
    uint32_t evttype: 1;
    uint32_t devspec_evt: 7;
    uint32_t evtnum: 5;
    uint32_t rsvd1: 3;
    uint32_t evtinfo: 9;
    uint32_t rsvd2: 7;
} devt_t;

typedef struct {
    uint32_t evttype: 1;
    uint32_t phy_epnum: 5;
    uint32_t evt: 4;
    uint32_t rsvd: 2;
    uint32_t evtstatus: 4;
    uint32_t evtparam: 16;
} depevt_t;

typedef union {
    devt_t devt;
    depevt_t depevt;
    uint32_t v;
} evt_t;

typedef enum {
    EP0_UNCONNECTED,
    EP0_SETUP_PHASE,
    EP0_DATA_PHASE,
    EP0_STATUS_PHASE,
} EP0_STAGE;

typedef enum {
    TRBCTL_NORMAL = 1,
    TRBCTL_CTL_SETUP,
    TRBCTL_CTL_STATUS_2,
    TRBCTL_CTL_STATUS_3,
    TRBCTL_CTL_DATA,
    TRBCTL_ISO_FIRST,
    TRBCTL_ISO,
    TRBCTL_LINK,
    TRBCTL_NORMAL_ZLP,
} TRBCTL;

typedef struct {
    uint8_t   num;
    uint8_t   is_in;
    uint8_t   is_stall;
    uint8_t   type;
    void      *trbd;
    void      *evtbd;
    uint32_t  maxpacket;
    uint32_t  bsz;
    uint8_t   *xfer_buff;
    uint32_t  xfer_len;
    uint32_t  xfer_count;
    bool      is_enabled;
    bool      is_busy;
    int32_t   res_id;
} DW_USB_EP;

typedef struct {
    vaddr_t     io_base;        /* Register base address */
    vaddr_t     phy_base;
    uint8_t     USB_Address;    /* USB Address */
    DW_USB_EP   IN_ep[NUM_EP];  /* IN endpoint parameters */
    DW_USB_EP   OUT_ep[NUM_EP]; /* OUT endpoint parameters */
    spin_lock_t lock;           /* PCD peripheral status */
    EP0_STAGE   ep0stage;
    bool        ep0_expect_in;
    bool        three_stage_setup;
    dw_ep0_next ep0_next_event;
    trbd_t     *ep0_trbd;
    trbd_t     *ep1_trbd;
    int        speed;
    char       *Setup;          /* Setup packet buffer */
    void       *pData;          /* Pointer to upper stack Handler */
    enum dw_link_state  link_state;
    bool do_resched;
    struct ep_status    ep_in[NUM_EP];
    struct ep_status    ep_out[NUM_EP];
} DW_USB_Context;

static char setup_buff[MAX_USB_NUM][ROUNDUP(SETUP_PKT_BUF_LEN,
                                    CACHE_LINE)] __ALIGNED(CACHE_LINE);
static char event_buff[MAX_USB_NUM][ROUNDUP(EVT_BUFF_SIZE,
                                    CACHE_LINE)] __ALIGNED(CACHE_LINE);

static DW_USB_Context g_usbc[MAX_USB_NUM];

static void *usbmalloc(size_t size)
{
    return malloc(size);
}
static void *usbmemalign(size_t boundary, size_t size)
{
    size_t tmp = ROUNDUP(size, CACHE_LINE);
    void *p = memalign(boundary,  tmp);
    return p;
}

static int get_buffer_transferred(int orig_size, int remain_size)
{
    return orig_size - remain_size;
}

static int epnumtopnum(int epnum, int is_in)
{
    /*first OUT, then IN*/
    return epnum * 2 + is_in;
}

static evtbd_t *create_evtb(int instance, DW_USB_EP *ep)
{
    evtbd_t *evtbd = (evtbd_t *)usbmalloc(sizeof(evtbd_t));

    if (!evtbd) {
        printf("no free memory %s\n", __func__);
        return NULL;
    }

    evtbd->evtb = (void *)&event_buff[instance][0];
    evtbd->lpos = 0;
    evtbd->size = EVT_BUFF_SIZE;
    memset(evtbd->evtb, 0, evtbd->size);
    evtbd->paddr = p2ap(v2p(evtbd->evtb));
    return evtbd;
}

static void dw_usbc_ep_open(int instance, DW_USB_EP *ep)
{
    DW_USB_Context *uc = &g_usbc[instance];
    int p_num = epnumtopnum(ep->num, ep->is_in);
    dw_usb_ll_ep_open(uc->io_base, p_num);
    ep->is_enabled = true;
}

static void dw_usbc_ep_close(int instance, DW_USB_EP *ep)
{
    DW_USB_Context *uc = &g_usbc[instance];
    int p_num = epnumtopnum(ep->num, ep->is_in);
    dw_usb_ll_ep_close(uc->io_base, p_num);
    ep->is_enabled = false;
}

static void dw_usbc_set_ep_event_buffer(int instance, DW_USB_EP *ep)
{
    DW_USB_Context *uc = &g_usbc[instance];
    uint32_t adr_l = 0, adr_h = 0;
    uint16_t size = 0;
    evtbd_t *evtbd;
    int p_num = epnumtopnum(ep->num, ep->is_in);
    ep->evtbd = create_evtb(instance, ep);

    if (!ep->evtbd) {
        dprintf(CRITICAL, "create evtb fail\n");
        return;
    }

    evtbd = ep->evtbd;
    arch_clean_cache_range((addr_t)evtbd->evtb, evtbd->size);
    adr_l = (uint32_t)(evtbd->paddr & 0xffffffff);
#if IS_64BIT
    adr_h = (uint32_t)((evtbd->paddr >> 32) & 0xffffffff);
#endif
    size = evtbd->size;
    dw_usb_ll_set_ep_event_buffer(uc->io_base, p_num, adr_l, adr_h, size);
}

uint16_t dw_usbc_get_ep_event_count(int instance, DW_USB_EP *ep)
{
    DW_USB_Context *uc = &g_usbc[instance];
    int p_num = epnumtopnum(ep->num, ep->is_in);
    return dw_usb_ll_get_ep_event_count(uc->io_base, p_num);
}

bool dw_usbc_clear_ep_event(int instance, DW_USB_EP *ep, uint16_t size)
{
    DW_USB_Context *uc = &g_usbc[instance];
    int p_num = epnumtopnum(ep->num, ep->is_in);
    return  dw_usb_ll_clear_ep_event(uc->io_base,  p_num, size);
}

static int gettrbctl(int instance, DW_USB_EP *ep)
{
    DW_USB_Context *uc = &g_usbc[instance];

    if (ep->num == 0) {
        if (uc->ep0stage == EP0_SETUP_PHASE) {
            return TRBCTL_CTL_SETUP;
        }
        else if (uc->ep0stage == EP0_DATA_PHASE) {
            return TRBCTL_CTL_DATA;
        }
        else if (uc->ep0stage == EP0_STATUS_PHASE) {
            if (uc->three_stage_setup) {
                return TRBCTL_CTL_STATUS_3;
            }
            else {
                return TRBCTL_CTL_STATUS_2;
            }
        }
    }

    if (((ep_type_t)ep->type == USB_ISOC)) {
        return TRBCTL_ISO_FIRST;
    }
    else {
        return TRBCTL_NORMAL;
    }

    return 0;
}

static void free_trb(DW_USB_EP *ep)
{
    trbd_t *trbd = ep->trbd;

    if (trbd) {
        free(trbd->trb);
        free(trbd);
        ep->trbd = NULL;
    }
}

static trbd_t *create_trb(int instance, DW_USB_EP *ep, int num)
{
    trbd_t *trbd = (trbd_t *)usbmalloc(sizeof(trbd_t));

    if (!trbd) {
        return NULL;
    }

    trbd->trb = (trb_t *)usbmemalign(TRB_ALIGN, sizeof(trb_t) * num) ;

    if (!trbd->trb) {
        free(trbd);
        return NULL;
    }

    memset(trbd->trb, 0, sizeof(trb_t)*num);
    trbd->orig_size = 0;
    trbd->paddr = p2ap(v2p(trbd->trb));
    return trbd;
}

static trbd_t *update_trb(int instance, DW_USB_EP *ep, uint8_t *buf,
                          int size)
{
    uint64_t paddr;
    trbd_t *trbd = ep->trbd;
    memset(trbd->trb, 0, sizeof(trb_t));
    trbd->orig_size = size;

    if (buf == 0) {
        paddr = p2ap(0);
    }
    else {
        paddr = p2ap(v2p(buf));
        arch_clean_cache_range((addr_t)buf, ROUNDUP(size, CACHE_LINE));
    }

    trbd->trb->bp_l = (uint32_t)(paddr & 0xffffffff);
    trbd->trb->bp_h = (uint32_t)((paddr >> 32) & 0xffffffff);
    trbd->trb->trb2.bufsize = size;
    trbd->trb->trb3.lst = 1;
    trbd->trb->trb3.chn = 0;
    trbd->trb->trb3.csp = 0;
    trbd->trb->trb3.trbctl = gettrbctl(instance, ep);
    trbd->trb->trb3.isp_imi = 1;
    trbd->trb->trb3.ioc = 1;
    trbd->trb->trb3.sid = 0;
    trbd->trb->trb3.hwo = 1;
    arch_clean_cache_range((addr_t)trbd->trb, ROUNDUP(sizeof(trb_t),
                           CACHE_LINE));
    return trbd;
}

void dw_usbc_wakeup(int instance)
{
    int     needs_wakeup;
    int ret = 0;
    DW_USB_Context *uc = &g_usbc[instance];
    needs_wakeup = (uc->link_state == DW_LINK_STATE_U1 ||
                    uc->link_state == DW_LINK_STATE_U2 ||
                    uc->link_state == DW_LINK_STATE_U3);

    if (needs_wakeup) {
        ret = dw_usb_ll_wakeup(uc->io_base);

        if (ret) {
            printf("wakeup failed --> %d\n", ret);
        }
    }
}

/*command 1*/
static void dw_usbc_set_ep_config(int instance, DW_USB_EP *ep, bool modify,
                                  bool restore)
{
    DW_USB_Context *uc = &g_usbc[instance];
    uint32_t param0 = 0, param1 = 0, param2 = 0;
    int p_num = epnumtopnum(ep->num, ep->is_in);
    /*construct param0*/
    /*type 1 to 2*/
    RMWREG32(&param0, 1, 2, ep->type);
    /*mps 3 to 13*/
    RMWREG32(&param0, 3, 11, ep->maxpacket);
    /*burst size 22 to 25*/

    if (modify) {
        RMWREG32(&param0, 30, 2, 2);
    }
    else if (restore) {
        RMWREG32(&param0, 30, 2, 1);
        RMWREG32(&param2, 30, 2, 1);
    }
    else {
        RMWREG32(&param0, 30, 2, 0);
    }

    if (ep->is_in) {
        RMWREG32(&param0, 17, 5, ep->num);
    }

    //complete
    RMWREG32(&param1, 8, 1, 1);

    //not ready
    if (ep->num == 0 || ep->type == USB_ISOC) {
        RMWREG32(&param1, 10, 1, 1);
    }

    if (ep->type != USB_CTRL) {
        RMWREG32(&param1, 9, 1, 1);
    }

    RMWREG32(&param1, 25, 5, ((ep->num << 1) | ep->is_in));
    RMWREG32(&param1, 16, 8, 0);
    dw_usb_ll_set_ep_config(uc->io_base, p_num, param0, param1, param2);
}

/*command 2*/
static void dw_usbc_set_ep_xfer_config(int instance, DW_USB_EP *ep)
{
    DW_USB_Context *uc = &g_usbc[instance];
    uint32_t param0 = 1;
    int p_num = epnumtopnum(ep->num, ep->is_in);
    dw_usb_ll_set_ep_xfer_config(uc->io_base, p_num, param0);
}

/*command 4*/
static void dw_usbc_set_ep_stall(int instance, DW_USB_EP *ep)
{
    DW_USB_Context *uc = &g_usbc[instance];
    int p_num = epnumtopnum(ep->num, ep->is_in);
    ep->is_stall = 1;
    dw_usb_ll_set_ep_stall(uc->io_base, p_num);
}

/*command 5*/
static void dw_usbc_clear_ep_stall(int instance, DW_USB_EP *ep)
{
    DW_USB_Context *uc = &g_usbc[instance];
    uint32_t param0 = 0;
    int p_num = epnumtopnum(ep->num, ep->is_in);
    ep->is_stall = 0;
    dw_usb_ll_clear_ep_stall(uc->io_base, p_num, param0);
}

/*command 6*/
static void dw_usbc_ep_start_transfer(int instance, DW_USB_EP *ep)
{
    DW_USB_Context *uc = &g_usbc[instance];
    trbd_t *trbd = ep->trbd;
    uint32_t param0 = 0, param1 = 0;
    int p_num = epnumtopnum(ep->num, ep->is_in);
    param1 = (uint32_t)(trbd->paddr & 0xffffffff);
#if IS_64BIT
    param0 = (uint32_t)((trbd->paddr >> 32) & 0xffffffff);
#endif
    dw_usbc_wakeup(instance);
    dw_usb_ll_ep_start_transfer(uc->io_base, p_num, param0, param1);
    ep->res_id = dw_usb_ll_get_ep_transfer_index(uc->io_base, p_num);
}

/*command 7*/
static void dw_usbc_ep_update_transfer(int instance, DW_USB_EP *ep)
{
    DW_USB_Context *uc = &g_usbc[instance];
    int p_num = epnumtopnum(ep->num, ep->is_in);
    dw_usb_ll_ep_update_transfer(uc->io_base, p_num, ep->res_id);
}

/*command 8*/
static void dw_usbc_ep_end_transfer(int instance, DW_USB_EP *ep)
{
    DW_USB_Context *uc = &g_usbc[instance];
    int p_num = epnumtopnum(ep->num, ep->is_in);
    dw_usb_ll_ep_end_transfer(uc->io_base, p_num, ep->res_id);
    ep->res_id = 0;
}

/*command 9*/
static void dw_usbc_set_ep_new_config(int instance, DW_USB_EP *ep)
{
    DW_USB_Context *uc = &g_usbc[instance];
    int p_num = epnumtopnum(ep->num, ep->is_in);
    dw_usb_ll_set_ep_new_config(uc->io_base, p_num);
}

static status_t dw_usbc_ep_trans(int instance, DW_USB_EP *ep,
                                 uint8_t *pBuf, uint32_t len)
{
    /*setup and start the Xfer */
    ep->xfer_buff = pBuf;
    ep->xfer_len = len;

    /* Multi packet transfer*/
    if (ep->xfer_len > ep->maxpacket) {
        len = ep->maxpacket;
        ep->xfer_len -= len;
    }
    else {
        len = ep->xfer_len;
        ep->xfer_len = 0;

        if (ep->is_in == 0) { //out packet need align with mps
            len = ROUNDUP(len, ep->maxpacket);
        }
    }

    update_trb(instance, ep, pBuf, len);

    if (!ep->is_busy) {
        dw_usbc_ep_start_transfer(instance, ep);
    }
    else {
        dw_usbc_ep_update_transfer(instance, ep);
    }

    return NO_ERROR;
}

status_t dw_usbc_setup_endpoint(int instance, ep_t ept, ep_dir_t dir,
                                uint width, ep_type_t type, bool modify, bool restore)
{
    DW_USB_EP *ep = NULL;
    DW_USB_Context *uc = &g_usbc[instance];
    DEBUG_ASSERT(ept <= NUM_EP);

    if (dir == USB_IN) {
        ep = &uc->IN_ep[ept];
    }
    else {
        ep = &uc->OUT_ep[ept];
    }

    ep->is_in = (dir == USB_IN);
    ep->maxpacket = width;
    ep->type = type;

    if (ep->num == 0 && ep->is_in == 0
            && !ep->is_enabled) { //only ep0out do this
        dw_usbc_set_ep_new_config(instance, ep);
        int i = 0;
        DW_USB_EP *eptemp;

        for (i = 0; i < NUM_EP; i++) {
            eptemp = &uc->OUT_ep[i];
            dw_usbc_set_ep_xfer_config(instance, eptemp);
            eptemp = &uc->IN_ep[i];
            dw_usbc_set_ep_xfer_config(instance, eptemp);
        }
    }

    dw_usbc_set_ep_config(instance, ep, modify,    restore);

    if (ep->num == 0) {
        ep->trbd = uc->ep0_trbd;
    }
    else {
        ep->trbd = create_trb(instance, ep, 1);

        if (!ep->trbd) {
            dprintf(CRITICAL, "malloc ep%d trbd fail\n", ep->num);
            return ERR_NO_MEMORY;
        }
    }

    if (!ep->is_enabled) {
        dw_usbc_ep_open(instance,  ep);
    }

    return NO_ERROR;
}

static void dw_usbc_disconnectcb(int instance)
{
    DW_USB_Context *uc = &g_usbc[instance];
    dw_usb_ll_disconnect_evt(uc->io_base);
}

static void dw_usbc_conndonecb(int instance)
{
    DW_USB_Context *uc = &g_usbc[instance];
    int width = 64;
    int speed = dw_usb_ll_get_cur_speed(uc->io_base);
    RMWREG32(uc->io_base + 0xc704, 24, 5, 0);
    uc->speed = speed;

    if (speed == 0) { //hs
        width = 64;
    }
    else { //fs
        width = 64;
    }

    dw_usbc_setup_endpoint(instance, 0, USB_OUT, width, USB_CTRL, true, false);
    dw_usbc_setup_endpoint(instance, 0, USB_IN, width, USB_CTRL, true, false);
    dump_registers(uc->io_base, "connectdone");
    uc->do_resched = true;
}

static void dw_usbc_resetcb(usb_t *usb)
{
    DW_USB_Context *uc = &g_usbc[usb->priv.instance_id];
    int instance = usb->priv.instance_id;
    RMWREG32(uc->io_base + 0xc704, 1, 4, 0);

    /* fail all the outstanding transactions */
    for (uint i = 0; i < NUM_EP; i++) {
        if (uc->ep_in[i].transfer) {
            usbc_transfer_t *t = uc->ep_in[i].transfer;
            uc->ep_in[i].transfer = NULL;

            if (t) {
                t->result = ERR_CANCELLED;

                if (t->callback) {
                    t->callback(i, t);
                }
            }
        }

        if (uc->ep_out[i].transfer) {
            usbc_transfer_t *t = uc->ep_out[i].transfer;
            uc->ep_out[i].transfer = NULL;

            if (t) {
                t->result = ERR_CANCELLED;

                if (t->callback) {
                    t->callback(i, t);
                }
            }
        }

        DW_USB_EP *ep = &uc->OUT_ep[i];

        if (ep->res_id != 0 && i >= 1) {
            dw_usbc_ep_end_transfer(instance, ep);
        }

        ep = &uc->IN_ep[i];

        if (ep->res_id != 0 && i >= 1) {
            dw_usbc_ep_end_transfer(instance, ep);
        }

        if (i == 0) {
            dw_usbc_clear_ep_stall(instance, &uc->IN_ep[i]);
        }
        else if (i >= 1) {
            dw_usbc_clear_ep_stall(instance, &uc->OUT_ep[i]);
            dw_usbc_clear_ep_stall(instance, &uc->IN_ep[i]);
        }
    }

    //disable ep
    usbc_set_address(usb,  0);
    dump_registers(uc->io_base, "reset");
    uc->do_resched = true;
}

static void dw_usbc_linksts_changecb(int instance, unsigned int evtinfo)
{
    DW_USB_Context *uc = &g_usbc[instance];
    enum dw_link_state next = evtinfo & DW_LINK_STATE_MASK;
    uc->link_state = next;
    printf("U%d\n", next);
}

void dw_usbc_ep0_send(int instance, const void *buf, size_t len,
                      size_t maxlen)
{
    DW_USB_Context *uc = &g_usbc[instance];
    struct ep_status *ep = &uc->ep_in[0];
    int size;
    ep->ack_ep0_in = true;
    DW_USB_EP *ept = &uc->IN_ep[0];
    ept->xfer_count = 0;

    if (uc->three_stage_setup) {
        bool        direction;
        direction = uc->ep0_expect_in;

        if (direction != ept->is_in) {
            dprintf(CRITICAL, "direction error, expect %d, get %d\n", direction,
                    ept->is_in);
            return;
        }
    }

    if (maxlen != 0) {
        size = MIN(len, maxlen);
    }
    else {
        size = len;
    }

    dw_usbc_ep_trans(instance, ept, (uint8_t *)buf, size);

    if (ept->num == 0) {
        uc->ep0_next_event = DW_EP0_COMPLETE;
    }
}

void dw_usbc_ep0_recv(int instance, void *buf, size_t len, ep_callback cb)
{
    DW_USB_Context *uc = &g_usbc[instance];
    struct ep_status *ep = &uc->ep_out[0];
    ep->ack_ep0_in = true;
    DW_USB_EP *ept = &uc->OUT_ep[0];
    bool direction;
    ept->xfer_count = 0;

    if (uc->three_stage_setup) {
        direction = uc->ep0_expect_in;

        if (direction != ept->is_in) {
            dprintf(CRITICAL, "direction error, expect %d, get %d\n", direction,
                    ept->is_in);
            return;
        }
    }

    dw_usbc_ep_trans(instance, ept, (void *)buf, len);

    if (ept->num == 0) {
        uc->ep0_next_event = DW_EP0_COMPLETE;
    }
}

static void dw_usbc_setupstagecb(usb_t *usb, DW_USB_EP *ep)
{
    DW_USB_Context *uc = &g_usbc[usb->priv.instance_id];
    struct hal_usb_callback_args args;
    arch_invalidate_cache_range((addr_t)uc->Setup, ROUNDUP(8, CACHE_LINE));
    args.setup = (struct usb_setup *)uc->Setup;
    args.usb = usb;

    if (!args.setup->length) {
        uc->three_stage_setup = false;
        uc->ep0_expect_in = false;
        uc->ep0_next_event = DW_EP0_NRDY_STATUS;
    }
    else {
        uc->three_stage_setup = true;
        uc->ep0_expect_in = !!(args.setup->request_type & DIR_IN);
        uc->ep0stage = EP0_DATA_PHASE;
        uc->ep0_next_event = DW_EP0_NRDY_DATA;
    }

    hal_usbc_callback(USB_CB_SETUP_MSG, &args);
    uc->do_resched = true;
}

static void dw_usbc_datainstagecb(int instance, DW_USB_EP *ep)
{
    DW_USB_Context *uc = &g_usbc[instance];
    usbc_transfer_t *t = 0;

    if (uc->ep_in[ep->num].transfer) {
        // completing a transfer
        t = uc->ep_in[ep->num].transfer;
        uc->ep_in[ep->num].transfer = 0;
        t->bufpos = ep->xfer_count;
        t->result = 0;
        dprintf(INFO, "completing transfer len %zd, finished %d %p\n", t->buflen,
                t->bufpos, t->callback);

        if (t->callback) {
            t->callback(ep->num, t);
        }
    }

    uc->do_resched = true;
}

static void dw_usbc_dataoutstagecb(int instance, DW_USB_EP *ep)
{
    DW_USB_Context *uc = &g_usbc[instance];
    usbc_transfer_t *t = 0;

    if (uc->ep_out[ep->num].transfer) {
        // completing a transfer
        t = uc->ep_out[ep->num].transfer;
        arch_invalidate_cache_range((addr_t)(t->buf), ROUNDUP(ep->xfer_count,
                                    CACHE_LINE));
        uc->ep_out[ep->num].transfer = 0;
        LTRACEF("completing transfer %p\n", t);
        t->bufpos = ep->xfer_count;
        t->result = 0;

        if (t->callback) {
            t->callback(ep->num | 0x80, t);
        }
    }

    uc->do_resched = true;
}

static void dump_ep_special_evt(int instance, DW_USB_EP *ep, depevt_t evt)
{
    switch (evt.evt) {
        case 7:
            //printf("get epcmd complete on ep:%d, in:%d\n",ep->num,ep->is_in);
            break;

        case 6:
            printf("get stream event on ep:%d, in:%d\n", ep->num, ep->is_in);
            break;

        case 3:
            printf("xfernotready on ep:%d, in:%d\n", ep->num, ep->is_in);
            break;

        case 2:
            printf("get xferinprogress on ep:%d, in:%d\n", ep->num, ep->is_in);
            break;

        case 1:
            printf("xfercomplete on ep:%d, in:%d\n", ep->num, ep->is_in);
            break;

        default:
            //printf("get unknown or reserved epevt %d on ep:%d, in:%d\n",evt.evt,ep->num,ep->is_in);
            break;
    }
}

static void dw_usbc_ep0_xfernotready(usb_t *usb, DW_USB_EP *ep,
                                     depevt_t evt)
{
    DW_USB_Context *uc = &g_usbc[usb->priv.instance_id];
    int instance = usb->priv.instance_id;

    //printf("s %d\n",evt.evtstatus);
    switch (evt.evtstatus) {
        /*Control data*/
        case 1:

            //dprintf(CRITICAL, "Control Data\n");
            if (uc->ep0_expect_in != ep->is_in) {
                ep = uc->ep0_expect_in ? &uc->IN_ep[0] : &uc->OUT_ep[0];
                dprintf(CRITICAL, "Wrong direction for Data phase\n");
                dw_usbc_ep_end_transfer(instance, ep);
                usbc_ep0_stall(usb);

                //dwc3_ep0_stall_and_restart(dwc);
                if (uc->OUT_ep[0].is_stall) { //stall
                    uc->OUT_ep[0].is_stall = 0;
                    uc->ep0stage = EP0_SETUP_PHASE;
                    uc->three_stage_setup = false;
                    dw_usbc_ep0_recv(instance, uc->Setup, 8, NULL);
                }

                return;
            }

            break;

        /*Control Status*/
        case 2:
            if (uc->ep0_next_event != DW_EP0_NRDY_STATUS) {
                return;
            }

            //dprintf(CRITICAL, "Control Status\n");
            uc->ep0stage = EP0_STATUS_PHASE;
            uc->ep0_expect_in = !uc->ep0_expect_in;

            if (uc->ep0_expect_in) {
                dw_usbc_ep0_send(instance, uc->Setup, 0, 0);
            }
            else {
                dw_usbc_ep0_recv(instance, uc->Setup, 0, 0);
            }

            uc->ep0_next_event = DW_EP0_COMPLETE;
            break;
    }
}

static void dw_usb_handle_ep_special_evt(usb_t *usb, DW_USB_EP *ep,
        depevt_t evt)
{
    DW_USB_Context *uc = &g_usbc[usb->priv.instance_id];
    int instance = usb->priv.instance_id;

    //dump_ep_special_evt(instance,ep,evt);
    switch (evt.evt) {
        case 7:
            //printf("get epcmd complete on ep:%d, in:%d\n",ep->num,ep->is_in);
            break;

        case 6:
            //printf("get stream event on ep:%d, in:%d\n",ep->num,ep->is_in);
            break;

        case 3://xfernotready
            if (ep->num == 0) { //control ep
                dw_usbc_ep0_xfernotready(usb, ep,    evt);
            }

            break;

        case 1://xfercomplete
            //printf("ep %d\n",ep->num);
            ep->res_id = 0;

            if (ep->num == 0) { //control ep
                trbd_t *trbd = ep->trbd;
                arch_invalidate_cache_range((addr_t)trbd->trb, ROUNDUP(sizeof(trb_t),
                                            CACHE_LINE));

                /* Process SETUP Packet*/
                if (uc->ep0stage == EP0_SETUP_PHASE) {
                    dw_usbc_setupstagecb(usb, ep);

                    if (uc->OUT_ep[0].is_stall) { //stall
                        uc->OUT_ep[0].is_stall = 0;
                        uc->ep0stage = EP0_SETUP_PHASE;
                        uc->three_stage_setup = false;
                        dw_usbc_ep0_recv(instance, uc->Setup, 8, NULL);
                    }
                }
                else if (uc->ep0stage == EP0_DATA_PHASE) {
                    if (ep->is_in == 0) { /*data out*/
                        /* Get Control Data OUT Packet*/
                        uint32_t count  = get_buffer_transferred(trbd->orig_size,
                                          trbd->trb->trb2.bufsize);
                        ep->xfer_count += count;
                        ep->xfer_buff += count;

                        if ((ep->xfer_len == 0) || (count < ep->maxpacket)) {
                            /* RX COMPLETE */
                            uc->ep0_next_event = DW_EP0_NRDY_STATUS;
                            uc->ep0stage = EP0_STATUS_PHASE;
                            dw_usbc_dataoutstagecb(instance, ep);
                        }
                        else {
                            dw_usbc_ep_trans(instance, ep, ep->xfer_buff, ep->xfer_len);
                        }
                    }
                    else {
                        int count = get_buffer_transferred(trbd->orig_size,
                                                           trbd->trb->trb2.bufsize);
                        ep->xfer_count += count;
                        ep->xfer_buff += count;

                        /* Zero Length Packet? */
                        if (ep->xfer_len == 0) {
                            /* TX COMPLETE */
                            uc->ep0_next_event = DW_EP0_NRDY_STATUS;
                            uc->ep0stage = EP0_STATUS_PHASE;
                            dw_usbc_datainstagecb(instance, ep);
                        }
                    }
                }
                else if (uc->ep0stage == EP0_STATUS_PHASE) {
                    //printf("setup stage complete\n");
                    uc->ep0stage = EP0_SETUP_PHASE;
                    uc->three_stage_setup = false;
                    dw_usbc_ep0_recv(instance, uc->Setup, 8, NULL);
                }
            }
            else {   //none control ep
                if (ep->is_in == 0) { //out
                    /*multi-packet on the NON control OUT endpoint*/
                    trbd_t *trbd = ep->trbd;
                    //invalidate trb
                    arch_invalidate_cache_range((addr_t)trbd->trb, ROUNDUP(sizeof(trb_t),
                                                CACHE_LINE));
                    uint32_t count = get_buffer_transferred(trbd->orig_size,
                                                            trbd->trb->trb2.bufsize);
                    ep->xfer_count += count;
                    ep->xfer_buff += count;

                    //printf("rx %d,remain %d\n",ep->xfer_count,ep->xfer_len);
                    if ((ep->xfer_len == 0) || (count < ep->maxpacket)) {
                        /* RX COMPLETE */
                        dw_usbc_dataoutstagecb(instance, ep);
                    }
                    else {
                        dw_usbc_ep_trans(instance, ep, ep->xfer_buff, ep->xfer_len);
                    }
                }
                else {   //in
                    /*multi-packet on the NON control IN endpoint*/
                    trbd_t *trbd = ep->trbd;
                    //invalidate trb
                    arch_invalidate_cache_range((addr_t)trbd->trb, ROUNDUP(sizeof(trb_t),
                                                CACHE_LINE));
                    int count = get_buffer_transferred(trbd->orig_size,
                                                       trbd->trb->trb2.bufsize);
                    ep->xfer_count += count;
                    ep->xfer_buff += count;

                    /* Zero Length Packet? */
                    if (ep->xfer_len == 0) {
                        /* TX COMPLETE */
                        dw_usbc_datainstagecb(instance, ep);
                    }
                    else {
                        dw_usbc_ep_trans(instance, ep, ep->xfer_buff, ep->xfer_len);
                    }
                }
            }

            break;

        default:
            break;
    }
}

static void dump_dev_special_evt(int instance, DW_USB_EP *ep, devt_t evt)
{
    switch (evt.evtnum) {
        case 16://ecc error
            printf("get dev ecc error on ep:%d, in:%d\n", ep->num, ep->is_in);
            break;

        case 14://L1 resume or remote wake evt
            printf("get dev L1 resume event on ep:%d, in:%d\n", ep->num, ep->is_in);
            break;

        case 12://vendor dev test lmp received event
            printf("get dev vendor dev test lmp received event on ep:%d, in:%d \n",
                   ep->num, ep->is_in);
            break;

        case 11://event buffer over flow
            printf("get dev evt buffer overflow on ep:%d, in:%d\n", ep->num,
                   ep->is_in);
            break;

        case 10://generic cmd complete
            printf("get dev evt generic cmd complete on ep:%d, in:%d\n", ep->num,
                   ep->is_in);
            break;

        case 9://erratic error
            printf("get dev evt erratic error on ep:%d, in:%d \n", ep->num, ep->is_in);
            break;

        case 8://L1 suspend
            printf("get dev evt L1 suspend on ep:%d, in: %d\n", ep->num, ep->is_in);
            break;

        case 7://sof
            printf("get dev evt sof on ep:%d, in: %d\n", ep->num, ep->is_in);
            break;

        case 6://usb suspend entry
            printf("get dev evt usb suspend entry on ep:%d, in: %d\n", ep->num,
                   ep->is_in);
            break;

        case 5://hibernate request
            printf("get dev evt hibernate request on ep:%d, in: %d\n", ep->num,
                   ep->is_in);
            break;

        case 4://wakeup
            printf("get dev evt wakeup on ep:%d, in: %d\n", ep->num, ep->is_in);
            break;

        case 3://usb/link state change
            printf("linksts change\n");
            break;

        case 2://connection done
            printf("conndone\n");
            break;

        case 1://usb reset
            printf("get dev evt usb reset on ep:%d, in: %d\n", ep->num, ep->is_in);
            break;

        case 0://disconnect detect
            printf("get dev evt disconnect detect on ep:%d, in: %d\n", ep->num,
                   ep->is_in);
            break;

        case 13:
        case 15: //reserved
        default:
            printf("get dev unknown event %d on ep:%d, in:%d \n", evt.evtnum, ep->num,
                   ep->is_in);
            break;
    }
}

static void dw_usb_handle_dev_special_evt(usb_t *usb, DW_USB_EP *ep,
        devt_t evt)
{
    int instance = usb->priv.instance_id;
    struct hal_usb_callback_args args;
    args.usb = usb;
    dump_dev_special_evt(instance, ep, evt);

    switch (evt.evtnum) {
        case 3://usb/link state change
            dw_usbc_linksts_changecb(instance, evt.evtinfo);
            break;

        case 2://connection done
            dw_usbc_conndonecb(instance);
            break;

        case 1://usb reset
            dw_usbc_resetcb(usb);
            hal_usbc_callback(USB_CB_RESET, &args);
            break;

        case 0://disconnect detect
            dw_usbc_disconnectcb(instance);
            hal_usbc_callback(USB_CB_DISCONNECT, &args);
            break;

        default:
            printf("get unknown event %d on ep:%d, in:%d \n", evt.evtnum, ep->num,
                   ep->is_in);
            break;
    }
}

static void dw_usb_handle_ncr_evt(int instance)
{
    DW_USB_Context *uc = &g_usbc[instance];
    uint32_t evt = dw_usb_ll_get_ncr_evt(uc->io_base);

    if (evt & 0x7Fu) {
        //printf("%s: ncr event: 0x%x\n", __FUNCTION__, evt);
        dw_usb_ll_clr_ncr_evt(uc->io_base);
    }
}

static void dw_usb_handle_evt(usb_t *usb)
{
    DW_USB_Context *uc = &g_usbc[usb->priv.instance_id];
    DW_USB_EP *ep = &uc->OUT_ep[0];
    evtbd_t *evtbd = ep->evtbd;
    int eventcount;
    evt_t *evt;
    int instance = usb->priv.instance_id;
    eventcount = dw_usbc_get_ep_event_count(instance, ep);

    if (eventcount == 0) {
        return;
    }

    dw_usb_ll_mask_irq(uc->io_base);
    addr_t start = ROUNDDOWN((addr_t)evtbd->evtb + evtbd->lpos, CACHE_LINE);
    addr_t end;
    int len;

    if ((evtbd->lpos + eventcount) > EVT_BUFF_SIZE) {
        end = (addr_t)(evtbd->evtb + EVT_BUFF_SIZE);
        len = end - start;
        arch_invalidate_cache_range(start, len);
        start = (addr_t)evtbd->evtb;
        len = ROUNDUP(evtbd->lpos + eventcount - EVT_BUFF_SIZE, CACHE_LINE);
        arch_invalidate_cache_range(start, len);
    }
    else {
        end = (addr_t)evtbd->evtb + evtbd->lpos + eventcount;
        len = ROUNDUP(end - start, CACHE_LINE);
        arch_invalidate_cache_range(start, len);
    }

    while (eventcount) {
        //arch_invalidate_cache_range((addr_t)evtbd->evtb+evtbd->lpos, 4);
        evt = (evt_t *)(evtbd->evtb + evtbd->lpos);

        if (evt->devt.evttype == 0) { //ep special event
            DW_USB_EP *ept = ((evt->depevt.phy_epnum % 2) == 0 ?
                              & (uc->OUT_ep[evt->depevt.phy_epnum / 2]) :
                              & (uc->IN_ep[evt->depevt.phy_epnum / 2]));
            dw_usb_handle_ep_special_evt(usb, ept, evt->depevt);
        }
        else if (evt->devt.devspec_evt == 0) {   //dev special event
            dw_usb_handle_dev_special_evt(usb, ep, evt->devt);
        }

        eventcount -= 4;
        evtbd->lpos = (evtbd->lpos + 4) % EVT_BUFF_SIZE;
        dw_usbc_clear_ep_event(instance, ep, 4);
    }

    dw_usb_ll_unmask_irq(uc->io_base);
    return;
}

void usbc_init(usb_t *usb)
{
    uint32_t i = 0;
    DW_USB_Context *uc = &g_usbc[usb->priv.instance_id];
    int instance = usb->priv.instance_id;
    uc->io_base = (vaddr_t)usb->priv.usb_vaddr;
    uc->phy_base = (vaddr_t)usb->priv.usb_phy_vaddr;
    ASSERT(usb->priv.max_eps <= NUM_EP);
    /* power on or rest*/
    dw_usb_dev_ll_por_srst(uc->io_base, uc->phy_base);

    /*prepare ep?*/
    for (i = 0; i < usb->priv.max_eps; i++) {
        uc->IN_ep[i].is_in = 1;
        uc->IN_ep[i].num = i;
        uc->IN_ep[i].type = USB_CTRL;
        uc->IN_ep[i].maxpacket =  1024;
        uc->IN_ep[i].bsz = 16;
        uc->IN_ep[i].xfer_buff = 0;
        uc->IN_ep[i].xfer_len = 0;
        uc->IN_ep[i].trbd = 0;
        uc->IN_ep[i].evtbd = 0;
    }

    for (i = 0; i < usb->priv.max_eps; i++) {
        uc->OUT_ep[i].is_in = 0;
        uc->OUT_ep[i].num = i;
        uc->OUT_ep[i].type = USB_CTRL;
        uc->OUT_ep[i].maxpacket = 1024;
        uc->OUT_ep[i].bsz = 16;
        uc->OUT_ep[i].xfer_buff = 0;
        uc->OUT_ep[i].xfer_len = 0;
        uc->OUT_ep[i].trbd = 0;
        uc->OUT_ep[i].evtbd = 0;
    }

    dw_usbc_ep_close(instance, &uc->OUT_ep[0]);
    dw_usbc_ep_close(instance, &uc->IN_ep[0]);
    uc->Setup = &setup_buff[instance][0];
    dw_usbc_set_ep_event_buffer(instance, &uc->OUT_ep[0]);
    uc->ep0_trbd = create_trb(instance, &uc->OUT_ep[0], 1);

    if (!uc->ep0_trbd) {
        dprintf(CRITICAL, "malloc ep0_trbd fail\n");
        return;
    }

    //config ep0
    uc->IN_ep[0].maxpacket = usb->priv.ep0_mps;
    uc->OUT_ep[0].maxpacket = usb->priv.ep0_mps;
    //dw_usbc_resetcb(instance);
    dw_usbc_setup_endpoint(instance, 0, USB_OUT, uc->OUT_ep[0].maxpacket,
                           USB_CTRL, false, false);
    dw_usbc_setup_endpoint(instance, 0, USB_IN, uc->IN_ep[0].maxpacket,
                           USB_CTRL, false, false);
    /* setup EP0 to receive SETUP packets */
    uc->ep0stage = EP0_SETUP_PHASE;
    dw_usbc_ep0_recv(instance, uc->Setup, 8, NULL);
    dw_usb_ll_enable_irq(uc->io_base);
    usbc_set_address(usb,  0);
    dump_registers(uc->io_base, "init");
}

enum handler_return usbc_irq(void *args)
{
    usb_t *usb = (usb_t *)args;
    int instance = usb->priv.instance_id;
    DW_USB_Context *uc = &g_usbc[instance];
    spin_lock_saved_state_t flags;

    do {
        uc->do_resched = false;
        spin_lock_irqsave(&uc->lock, flags);
        dw_usb_handle_evt(usb);
        dw_usb_handle_ncr_evt(instance);
        spin_unlock_irqrestore(&uc->lock, flags);
    } while (0);

    return INT_RESCHEDULE;
}

status_t usbc_queue_rx(usb_t *usb, ep_t ep, usbc_transfer_t *transfer)
{
    DW_USB_Context *uc = &g_usbc[usb->priv.instance_id];
    DEBUG_ASSERT((ep & 0x7f) <= NUM_EP);
    DEBUG_ASSERT(uc->ep_out[ep & 0x7f].transfer == NULL);
    uc->ep_out[ep & 0x7f].transfer = transfer;
    DW_USB_EP *ept = &uc->OUT_ep[ep & 0x7f];
    ept->xfer_count = 0;
    dw_usbc_ep_trans(usb->priv.instance_id, ept, transfer->buf,
                     transfer->buflen);
    return NO_ERROR;
}

status_t usbc_queue_tx(usb_t *usb, ep_t ep, usbc_transfer_t *transfer)
{
    DW_USB_Context *uc = &g_usbc[usb->priv.instance_id];
    DEBUG_ASSERT((ep & 0x7f) <= NUM_EP);
    DEBUG_ASSERT(uc->ep_in[ep & 0x7f].transfer == NULL);
    uc->ep_in[ep & 0x7f].transfer = transfer;
    DW_USB_EP *ept = &uc->IN_ep[ep & 0x7f];
    ept->xfer_count = 0;
    dw_usbc_ep_trans(usb->priv.instance_id, ept, transfer->buf,
                     transfer->buflen);
    return NO_ERROR;
}

status_t usbc_flush_ep(usb_t *usb, ep_t ep)
{
    ASSERT((ep & 0x7F) <= NUM_EP);
    DW_USB_Context *uc = &g_usbc[usb->priv.instance_id];
    int p_num = 0;

    if ((ep & 0x80) == 0x80) {
        p_num = epnumtopnum((ep & 0x7F), 0);
    }
    else {
        p_num = epnumtopnum((ep & 0x7F), 1);
    }

    // Flush the FIFOs for the endpoint.
    if (dw_usb_ll_flush_ep(uc->io_base, p_num) != true) {
        return ERR_GENERIC;
    }

    // Clear any transfers that we may have been waiting on.
    if (ep & 0x80) {
        uc->ep_in[ep & 0x7F].transfer = NULL;
    }
    else {
        uc->ep_out[ep].transfer = NULL;
    }

    return NO_ERROR;
}

status_t usbc_set_active(usb_t *usb, bool active)
{
    DW_USB_Context *uc = &g_usbc[usb->priv.instance_id];
    //write dctrl runstop bit to 1/0
    dw_usb_ll_runstop(uc->io_base, active);
    /*z1 workaround to tell the edk we are ready*/
    RMWREG32(uc->phy_base + 0x1001c, 0, 1, 1);
    dump_registers(uc->io_base, "run");
    return NO_ERROR;
}

void usbc_set_address(usb_t *usb, uint8_t address)
{
    DW_USB_Context *uc = &g_usbc[usb->priv.instance_id];
    dw_usb_ll_set_address(uc->io_base, address);
}

/* called back from within a callback to handle setup responses */
void usbc_ep0_ack(usb_t *usb)
{
#if 0
    DW_USB_Context *uc = &g_usbc[instance];
    struct ep_status *ep = &uc->ep_in[0];
    ep->ack_ep0_in = false;
    DW_USB_EP *ept = &uc->IN_ep[0];
    ept->xfer_count = 0;
    dw_usbc_ep_trans(instance, ept, 0, 0);
#endif
}

void usbc_ep0_stall(usb_t *usb)
{
    DW_USB_Context *uc = &g_usbc[usb->priv.instance_id];
    dw_usbc_set_ep_stall(usb->priv.instance_id, &uc->OUT_ep[0]);
}

bool usbc_is_highspeed(usb_t *usb)
{
    DW_USB_Context *uc = &g_usbc[usb->priv.instance_id];

    if (uc->speed == 0) {
        return true;
    }
    else {
        return false;
    }
}

status_t usbc_setup_endpoint(usb_t *usb, ep_t ep, ep_dir_t dir, uint width,
                             ep_type_t type)
{
    return dw_usbc_setup_endpoint(usb->priv.instance_id,  ep,  dir,  width,
                                  type, false, false);
}

void usbc_ep0_send(usb_t *usb, const void *buf, size_t len, size_t maxlen)
{
    dw_usbc_ep0_send(usb->priv.instance_id, buf, len, maxlen);
}

