/*
* dw_usb.c
*
* Copyright (c) 2019 Semidrive Semiconductor.
* All rights reserved.
*
* Description: implement synopsys usb driver
*
* Revision History:
* -----------------
* 011, 03/08/2019 chenqing create this file
*      03/20/2019 JerryFan Ported into ROM code.
*/

#include <soc_usb.h>
#include <string.h>
#include <dw_usb.h>
#include <dev/usb.h>
#include "dw_usb_ll.h"

#define MAX_USB_NUM         1
#define NUM_EP              2
#define EVT_BUFF_SIZE       (1024*4)
#define SETUP_PKT_BUF_LEN   64
#define CACHELINE_SIZE      CACHE_LINE
#define TRB_ALIGN           16

#define DEPEVT_STATUS_CONTROL_DATA  1
#define DEPEVT_STATUS_CONTROL_STATUS    2

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
        uint32_t bufsize:24;
        uint32_t pcm1:2;
        uint32_t spr:1;
        uint32_t rsvd:1;
        uint32_t trbsts:4;
    } bits;
    uint32_t v;
} trb2_t;

typedef union {
    struct {
        uint32_t hwo:1;
        uint32_t lst:1;
        uint32_t chn:1;
        uint32_t csp:1;
        uint32_t trbctl:6;
        uint32_t isp_imi:1;
        uint32_t ioc:1;
        uint32_t rsvd:2;
        uint32_t sid:16;
        uint32_t rsvd2:2;
    } bits;
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
    uint32_t evttype:1;     //0, 0,ep special event;1,none ep special event.
    uint32_t devspec_evt:7; //[]1-7], 0, dev special event
    uint32_t evtnum:5;      //8-12,
    uint32_t rsvd1:3;       //13-15
    uint32_t evtinfo:9;     //16-24
    uint32_t rsvd2:7 ;      //25-31
} devt_t;

typedef struct {
    uint32_t evttype:1;     //0, 0,ep special event;1,none ep special event.
    uint32_t phy_epnum:5;   //1-5
    uint32_t evt:4;         /*6-9,  7,epcmdcmplt;6,stream event;3,xfernotready;
                                    2,xferinprogress;1,xfercomplete; */
    uint32_t rsvd:2;        //10-11
    uint32_t evtstatus:4;   //12-15
    uint32_t evtparam:16 ;  //16-31
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
    TRBCTL_NORMAL =1,
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
    vaddr_t             io_base;      /*!< Register base address         */
    vaddr_t             phy_base;
    DW_USB_INIT         Init;           /*!< PCD required parameters     */
    uint8_t             USB_Address;    /*!< USB Address                 */
    DW_USB_EP           IN_ep[NUM_EP];       /*!< IN endpoint parameters */
    DW_USB_EP           OUT_ep[NUM_EP];      /*!< OUT endpoint parameters*/
    /*spin_lock_t       lock;*/           /*!< PCD peripheral status          */
    EP0_STAGE ep0stage;
    bool ep0_expect_in;
    bool three_stage_setup;
    dw_ep0_next ep0_next_event;
    USB_State   State;          /*!< PCD communication state            */
    trbd_t     *ep0_trbd;
    trbd_t     *ep1_trbd;
    int speed;
    addr_t *Setup;      /*!< Setup packet buffer               */
    addr_t *txbuf;
    void                    *pData;       /*!< Pointer to upper stack Handler*/
    enum dw_link_state    link_state;

    bool do_resched;

    struct ep_status ep_in[NUM_EP];
    struct ep_status ep_out[NUM_EP];

} DW_USB_Context;
void arch_clean_cache_range(const void *start, size_t len);
static DW_USB_Context g_usbc[MAX_USB_NUM] __IN_BSS2__;

#if defined(CFG_USB_STATIC_BUFs)
static trbd_t g_ep_trbd[NUM_EP*2] __IN_BSS2__;
static trb_t g_ep_trb[NUM_EP*2] __ALIGNED(TRB_ALIGN) __IN_BSS2__;
static U32 trbd_created = 0, trb_created = 0;;
static evtbd_t g_ep_evtbd[NUM_EP*2] __IN_BSS2__ __ALIGNED(CACHELINE_SIZE);
static U8 g_ep_evtb[NUM_EP*2][EVT_BUFF_SIZE] __IN_BSS2__;
static U32 evtb_created = 0;
#endif

#if !defined(CFG_USB_STATIC_BUFs)
static void *usbmalloc(size_t size)
{
    return malloc(size);
}

static void *usbmemalign(size_t boundary, size_t size)
{
    return memalign(boundary,  size);
}

static void usbfree(void *p)
{
    free(p);
}
#endif

static uint32_t get_buffer_transferred(int orig_size, int remain_size)
{
    return orig_size - remain_size;
}
static int epnumtopnum(int epnum, int is_in)
{
    /*first OUT, then IN*/
    return (epnum*2 + is_in);
}

static evtbd_t *create_evtb(int instance, DW_USB_EP *ep)
{
#if !defined(CFG_USB_STATIC_BUFs)
    evtbd_t *evtbd=(evtbd_t *)usbmalloc(sizeof(evtbd_t));
    evtbd->evtb = (void *)usbmemalign(CACHELINE_SIZE, EVT_BUFF_SIZE) ;
#else
    if (evtb_created >= (sizeof(g_ep_evtbd)/sizeof(g_ep_evtbd[0]))) {
        evtb_created = 0;
    }
    evtbd_t *evtbd = &g_ep_evtbd[evtb_created];
    evtbd->evtb = (void *)&g_ep_evtb[evtb_created];
    evtb_created++;
#endif
    evtbd->lpos =0;
    evtbd->size = EVT_BUFF_SIZE;
    memset(evtbd->evtb,0,evtbd->size);
    evtbd->paddr = vaddr_to_paddr(evtbd->evtb);

    USBDBG_L1("%s: evt buffer of ep_%d_%s be created at %p\n", __FUNCTION__,
              ep->num, ep->is_in ? "IN" : "OUT", evtbd->evtb);

    return evtbd;
}

static void dw_usbc_ep_open(int instance, DW_USB_EP *ep)
{
    DW_USB_Context *uc=&g_usbc[instance];
    int p_num = epnumtopnum(ep->num, ep->is_in);

    dw_usb_ll_ep_open(uc->io_base, p_num);
    ep->is_enabled = true;
}

static void dw_usbc_ep_close(int instance, DW_USB_EP *ep)
{
    DW_USB_Context *uc=&g_usbc[instance];
    int p_num = epnumtopnum(ep->num,ep->is_in);

    dw_usb_ll_ep_close(uc->io_base,p_num);
    ep->is_enabled = false;
}

static void dw_usbc_set_ep_event_buffer(int instance, DW_USB_EP *ep)
{
    DW_USB_Context *uc=&g_usbc[instance];
    uint32_t adr_l=0, adr_h=0;
    uint16_t size=0;
    evtbd_t *evtbd;
    int p_num = epnumtopnum(ep->num,ep->is_in);
    ep->evtbd = create_evtb(instance, ep);
    evtbd = ep->evtbd;
    clean_cache_range((const void*)(addr_t)evtbd->evtb, evtbd->size);
    adr_l=(uint32_t)(evtbd->paddr & 0xffffffff);
#if defined(ARCH_PTR_64BIT)
    adr_h=(uint32_t)((((u64)evtbd->paddr) >> 32) & 0xffffffff);
#endif
    size = evtbd->size;
    dw_usb_ll_set_ep_event_buffer(uc->io_base,p_num, adr_l,adr_h,size);
}

uint16_t dw_usbc_get_ep_event_count(int instance, DW_USB_EP *ep)
{
    DW_USB_Context *uc=&g_usbc[instance];
    int p_num = epnumtopnum(ep->num,ep->is_in);

    return dw_usb_ll_get_ep_event_count(uc->io_base, p_num);
}

bool dw_usbc_clear_ep_event(int instance, DW_USB_EP *ep, uint16_t size)
{
    DW_USB_Context *uc=&g_usbc[instance];
    int p_num = epnumtopnum(ep->num,ep->is_in);
    return dw_usb_ll_clear_ep_event( uc->io_base, p_num, size);
}

static int gettrbctl(int instance,DW_USB_EP *ep)
{
    DW_USB_Context *uc=&g_usbc[instance];
    if (ep->num == 0) {
        if (uc->ep0stage == EP0_SETUP_PHASE) {
            return TRBCTL_CTL_SETUP;
        } else if (uc->ep0stage == EP0_DATA_PHASE) {
            return TRBCTL_CTL_DATA;
        } else if (uc->ep0stage == EP0_STATUS_PHASE) {
            if (uc->three_stage_setup)
                return TRBCTL_CTL_STATUS_3;
            else
                return TRBCTL_CTL_STATUS_2;
        }
    }

    if (((ep_type_t)ep->type==USB_ISOC)) {
        return TRBCTL_ISO_FIRST;
    } else {
        return TRBCTL_NORMAL;
    }
    return 0;
}

#if !defined(CFG_USB_STATIC_BUFs)
static void free_trb(DW_USB_EP *ep)
{
    trbd_t *trbd = ep->trbd;
    if (trbd) {
        free(trbd->trb);
        free(trbd);
        ep->trbd=NULL;
    }
}
#endif

static trbd_t *create_trb(int instance, DW_USB_EP *ep, int num)
{
#if !defined(CFG_USB_STATIC_BUFs)
    trbd_t *trbd=(trbd_t *)usbmalloc(sizeof(trbd_t));
    trbd->trb = (trb_t *)usbmemalign(TRB_ALIGN, sizeof(trb_t) *num) ;
#else
    if (trbd_created >= (sizeof(g_ep_trbd)/sizeof(g_ep_trbd[0]))) {
        trbd_created = 0;
    }
    if (trb_created >= (sizeof(g_ep_trb)/sizeof(g_ep_trb[0]))) {
        trb_created = 0;
    }
    trbd_t * trbd = &g_ep_trbd[trbd_created++];
    trbd->trb = &g_ep_trb[trb_created];
    trb_created += num;
#endif
    memset(trbd->trb, 0, sizeof(trb_t)*num);
    trbd->orig_size = 0;
    trbd->paddr = vaddr_to_paddr(trbd->trb);
    USBDBG_L1("%s: trb of ep_%d_%s be created at %p\n", __FUNCTION__,
              ep->num, ep->is_in ? "IN" : "OUT",trbd->trb);
    return trbd;
}

static trbd_t *update_trb(int instance, DW_USB_EP *ep, uint8_t *buf, int size)
{
    USBDBG_L1("%s: buf = %p, size = %d\n", __FUNCTION__, buf, size);

    uint64_t paddr;
    trbd_t *trbd = ep->trbd;
    memset(trbd->trb, 0, sizeof(trb_t));
    trbd->orig_size = size;
    if (buf == 0) {
        paddr = 0;
    } else {
        if (ep->is_in) {
            clean_cache_range((const void*)(addr_t)buf, size);
        }
        paddr = vaddr_to_paddr(buf);
    }

    trbd->trb->bp_l = (uint32_t)(paddr & 0xffffffff);
    trbd->trb->bp_h = (uint32_t)((paddr >> 32) & 0xffffffff);
    trbd->trb->trb2.bits.bufsize = size;
    trbd->trb->trb3.bits.lst = 1;
    trbd->trb->trb3.bits.chn = 0;
    trbd->trb->trb3.bits.csp = 0;
    trbd->trb->trb3.bits.trbctl = gettrbctl(instance, ep);
    trbd->trb->trb3.bits.isp_imi = 1;
    trbd->trb->trb3.bits.ioc = 1;
    trbd->trb->trb3.bits.sid = 0;
    trbd->trb->trb3.bits.hwo = 1;
    clean_cache_range((const void*)(addr_t)trbd->trb, sizeof(trb_t));

    return trbd;
}

void dw_usbc_wakeup(int instance)
{
    int     needs_wakeup;
    int ret=0;
    DW_USB_Context *uc=&g_usbc[instance];

    needs_wakeup = (uc->link_state == DW_LINK_STATE_U1 ||
                    uc->link_state == DW_LINK_STATE_U2 ||
                    uc->link_state == DW_LINK_STATE_U3);

    if (needs_wakeup) {
        ret = dw_usb_ll_wakeup(uc->io_base);
        if (ret)
            DBG("%s: wakeup failed --> %d\n", __FUNCTION__, ret);
    }
}

static void dw_usbc_set_ep_config(int instance, DW_USB_EP *ep,bool modify, bool restore)
{
    DW_USB_Context *uc=&g_usbc[instance];
    uint32_t param0=0,param1=0,param2=0;
    int p_num = epnumtopnum(ep->num,ep->is_in);

    /*construct param0*/
    /*type 1 to 2*/
    RMWREG32(&param0, 1, 2,ep->type);
    /*mps 3 to 13*/
    RMWREG32(&param0, 3, 11, ep->maxpacket);
    /*burst size 22 to 25*/
    //RMWREG32(&param0, 22, 4, ep->bsz-1);
    /* iso have special config*/

    if (modify) {
        RMWREG32(&param0, 30, 2, 2);
    } else if (restore) {
        RMWREG32(&param0, 30, 2, 1);
        //params.param2 |= dep->saved_state;
        RMWREG32(&param2, 30, 2, 1);
    } else {
        RMWREG32(&param0, 30, 2, 0);
    }
    if (ep->is_in)
        RMWREG32(&param0, 17, 5, ep->num);

    RMWREG32(&param1, 8, 1, 1);

    if (ep->num==0 || ep->type == USB_ISOC)
        //not ready
        RMWREG32(&param1, 10, 1, 1);

    if (ep->type != USB_CTRL){
        //in progress
        RMWREG32(&param1, 9, 1, 1);
    }
    RMWREG32(&param1, 25, 5, ((ep->num << 1)|ep->is_in));

    //interval
    RMWREG32(&param1, 16, 8, 0);

    dw_usb_ll_set_ep_config(uc->io_base, p_num, param0, param1, param2);
}

static void dw_usbc_set_ep_xfer_config(int instance, DW_USB_EP *ep)
{
    DW_USB_Context *uc=&g_usbc[instance];
    uint32_t param0=1;
    int p_num = epnumtopnum(ep->num,ep->is_in);

    dw_usb_ll_set_ep_xfer_config(uc->io_base,p_num, param0);
}

static void dw_usbc_set_ep_stall(int instance, DW_USB_EP *ep)
{
    DW_USB_Context *uc=&g_usbc[instance];
    int p_num = epnumtopnum(ep->num,ep->is_in);

    ep->is_stall = 1;

    dw_usb_ll_set_ep_stall(uc->io_base,p_num);
}

static void dw_usbc_clear_ep_stall(int instance, DW_USB_EP *ep)
{
    DW_USB_Context *uc=&g_usbc[instance];
    int p_num = epnumtopnum(ep->num,ep->is_in);

    ep->is_stall = 0;
    dw_usb_ll_clear_ep_stall(uc->io_base,p_num, 0);
}

static void dw_usbc_ep_start_transfer(int instance, DW_USB_EP *ep)
{
    USBDBG_L1("%s being called.\n", __FUNCTION__);

    
    DW_USB_Context *uc=&g_usbc[instance];
    trbd_t *trbd=ep->trbd;
    uint32_t param0=0, param1 =0;
    int p_num = epnumtopnum(ep->num,ep->is_in);
    if (ep->is_busy) {
        USBDBG_L1("ep%d is busy.\n", p_num);
        return ;
    }
    param1 = (uint32_t)(trbd->paddr & 0xffffffff);
#if defined(ARCH_PTR_64BIT)
    param0 = (uint32_t)((((u64)trbd->paddr) >> 32) & 0xffffffff);
#endif

    dw_usbc_wakeup(instance);
    dw_usb_ll_ep_start_transfer(uc->io_base, p_num, param0, param1);
    ep->is_busy = true;
    ep->res_id = dw_usb_ll_get_ep_transfer_index(uc->io_base, p_num);
}

static void dw_usbc_ep_update_transfer(int instance, DW_USB_EP *ep)
{
    USBDBG_L1("%s being called.\n", __FUNCTION__);

    DW_USB_Context *uc=&g_usbc[instance];
    int p_num = epnumtopnum(ep->num,ep->is_in);

    dw_usb_ll_ep_update_transfer(uc->io_base,p_num,ep->res_id);
}

static void dw_usbc_ep_end_transfer(int instance, DW_USB_EP *ep)
{
    DW_USB_Context *uc=&g_usbc[instance];
    int p_num = epnumtopnum(ep->num,ep->is_in);

    dw_usb_ll_ep_end_transfer(uc->io_base,p_num,ep->res_id);
    ep->res_id =0;
}

static void dw_usbc_set_ep_new_config(int instance, DW_USB_EP *ep)
{
    DW_USB_Context *uc=&g_usbc[instance];
    int p_num = epnumtopnum(ep->num,ep->is_in);

    dw_usb_ll_set_ep_new_config(uc->io_base,p_num);
}

static status_t dw_usbc_ep_trans(int instance, DW_USB_EP *ep, uint8_t *pBuf, uint32_t len)
{
    /*setup and start the xfer */
    ep->xfer_buff = pBuf;
    ep->xfer_len = len;

    /* Multi packet transfer*/
    if (ep->xfer_len > ep->maxpacket) {
        len = ep->maxpacket;
    } else {
        len = ep->xfer_len;
        if (ep->is_in == 0) {//out packet need align with mps
            len = ep->maxpacket;
        }
    }

    update_trb(instance, ep, pBuf, len);

    if (!ep->is_busy) {
        dw_usbc_ep_start_transfer(instance, ep);
    } else {
        dw_usbc_ep_update_transfer(instance, ep);
    }

    return NO_ERROR;
}

static void dw_usbc_schedule(void *args)
{
    dw_usb_irqhandler(args);
}

static void dw_usbc_disconnectcb(int instance)
{
    DW_USB_Context *uc = &g_usbc[instance];
    dw_usb_ll_disconnect_evt(uc->io_base);
}

static void dw_usbc_conndonecb(int instance)
{
    DW_USB_Context *uc=&g_usbc[instance];
    USBDBG_L1("%s being called.\n", __FUNCTION__);

    int speed = dw_usbc_get_cur_speed(instance);
    int width = 64;
    // HIRD threshold
    REG_RMWREG32(uc->io_base + 0xc704, 24, 5, 0);
    REG_RMWREG32(uc->io_base + 0xc704, 1, 3, 0);
    if(speed == DCFG_DEVSPD_FS){
        USBDBG_L2("%s: speed FS\n", __FUNCTION__);
    }else{
        USBDBG_L2("%s: speed HS\n", __FUNCTION__);
        width = 512;   
    }
    uc->speed = speed;


    dw_usbc_setup_endpoint(instance, 0, USB_OUT, width, USB_CTRL,true,false);
    dw_usbc_setup_endpoint(instance, 0, USB_IN, width, USB_CTRL,true,false);

    uc->do_resched = true;
}

static void dw_usbc_resetcb(int instance)
{
    DW_USB_Context *uc=&g_usbc[instance];
    // Test control
    REG_RMWREG32(uc->io_base + 0xc704, 1, 4, 0);

    USBDBG_L2("%s being called.\n", __FUNCTION__);
    /* fail all the outstanding transactions */
    for (uint i = 1; i < NUM_EP; i++) {
        if (uc->ep_in[i].transfer) {
            usbc_transfer_t *t = uc->ep_in[i].transfer;
            uc->ep_in[i].transfer = NULL;
            if (t) {
                t->result = ERR_CANCELLED;
                t->callback(i, t);
            }
        }
        if (uc->ep_out[i].transfer) {
            usbc_transfer_t *t = uc->ep_out[i].transfer;
            uc->ep_out[i].transfer = NULL;
            if (t) {
                t->result = ERR_CANCELLED;
                t->callback(i, t);
            }
        }
        DW_USB_EP *ep=&uc->OUT_ep[i];
        if (ep->res_id !=0 ){
            dw_usbc_ep_end_transfer(instance,ep);
        }
        ep=&uc->IN_ep[i];
        if (ep->res_id !=0 ){
            dw_usbc_ep_end_transfer(instance,ep);
        }
    }

    dw_usbc_clear_ep_stall(instance,&uc->IN_ep[0]);
    dw_usbc_clear_ep_stall(instance,&uc->OUT_ep[1]);
    dw_usbc_clear_ep_stall(instance,&uc->IN_ep[1]);
        
    dw_usbc_set_address(instance,  0);

    uc->do_resched = true;
}

static void dw_usbc_linksts_changecb(int instance,
                                     unsigned int evtinfo)
{
    DW_USB_Context *uc=&g_usbc[instance];
    enum dw_link_state    next = evtinfo & DW_LINK_STATE_MASK;

    uc->link_state = next;
    USBDBG_L2("%s: U%d\n",__FUNCTION__, next);
}

status_t dw_usbc_cancel_tansfers(int instance, ep_t ep,bool in)
{
    DW_USB_Context *uc=&g_usbc[instance];
    DEBUG_ASSERT((ep & 0x7f) <= NUM_EP);
    DW_USB_EP *ept;
    struct ep_status *ep_sta;
    if(in){
        ept =  &uc->IN_ep[ep & 0x7f];
        ep_sta = &uc->ep_in[ep & 0x7f];
    }else{
        ept =  &uc->OUT_ep[ep & 0x7f];
        ep_sta = &uc->ep_out[ep & 0x7f];
    }
    if(ep_sta->transfer != NULL){
       return ERR_NO_MEMORY; 
    }
    USBDBG_L1("%s: cancel tansfers %08x\n", __FUNCTION__,ep_sta->transfer);
    ep_sta->transfer = NULL;
    ept->xfer_count = 0;
    dw_usbc_ep_end_transfer(instance, ept);

    return NO_ERROR;
}

U8 setup_buf[SETUP_PKT_BUF_LEN] __attribute__((aligned(CACHELINE_SIZE)));
U8 usbc_tx_buf[SETUP_PKT_BUF_LEN] __attribute__((aligned(CACHELINE_SIZE)));

void dw_usbc_init(int instance,vaddr_t iobase, vaddr_t phybase, DW_USB_INIT init)
{
    uint32_t i=0;
    DW_USB_Context *uc=&g_usbc[instance];
    memset(uc,0,sizeof(g_usbc));
    uc->State = HAL_USB_STATE_BUSY;
    uc->io_base = iobase;
    uc->phy_base = phybase;
    uc->Init = init;
    ASSERT(init.dev_endpoints <= NUM_EP);

    /* power on or rest*/
    dw_usb_dev_ll_por_srst(iobase, phybase);

    /*prepare ep?*/
    for (i=0; i<init.dev_endpoints; i++) {
        uc->IN_ep[i].is_in = 1;
        uc->IN_ep[i].num = i;
        uc->IN_ep[i].type = USB_CTRL;
        uc->IN_ep[i].maxpacket =  512;
        uc->IN_ep[i].bsz = 16;
        uc->IN_ep[i].xfer_buff = 0;
        uc->IN_ep[i].xfer_len = 0;
        uc->IN_ep[i].trbd = 0;
        uc->IN_ep[i].evtbd = 0;
    }

    for (i=0; i<init.dev_endpoints; i++) {
        uc->OUT_ep[i].is_in = 0;
        uc->OUT_ep[i].num = i;
        uc->OUT_ep[i].type = USB_CTRL;
        uc->OUT_ep[i].maxpacket = 512;
        uc->OUT_ep[i].bsz = 16;
        uc->OUT_ep[i].xfer_buff = 0;
        uc->OUT_ep[i].xfer_len = 0;
        uc->OUT_ep[i].trbd = 0;
        uc->OUT_ep[i].evtbd = 0;
    }
    dw_usbc_ep_close(instance,&uc->OUT_ep[0]);
    dw_usbc_ep_close(instance,&uc->IN_ep[0]);

#if defined(CFG_USB_STATIC_BUFs)
    uc->Setup = (addr_t *)setup_buf;
    uc->txbuf = (addr_t *)usbc_tx_buf;
#else
    uc->Setup = usbmemalign(CACHELINE_SIZE, SETUP_PKT_BUF_LEN);
    uc->txbuf = usbmemalign(CACHELINE_SIZE, SETUP_PKT_BUF_LEN);
#endif
    dw_usbc_set_ep_event_buffer(instance, &uc->OUT_ep[0]);
    uc->ep0_trbd = create_trb(instance, &uc->OUT_ep[0], 1);

    //config ep0
    uc->IN_ep[0].maxpacket=uc->Init.ep0_mps;
    uc->OUT_ep[0].maxpacket=uc->Init.ep0_mps;
    //dw_usbc_resetcb(instance);
    dw_usbc_setup_endpoint(instance, 0, USB_OUT,
                           uc->OUT_ep[0].maxpacket, USB_CTRL, false, false);
    dw_usbc_setup_endpoint(instance, 0, USB_IN,
                           uc->IN_ep[0].maxpacket, USB_CTRL, false, false);

    /* setup EP0 to receive SETUP packets */
    uc->ep0stage = EP0_SETUP_PHASE;
    dw_usbc_ep0_recv(instance, uc->Setup, 8, NULL);
    // dw_usbc_set_address(instance,  0);
    dw_usb_ll_enable_irq(uc->io_base);

    
    uc->State = HAL_USB_STATE_RESET;
}

static void dw_usbc_setupstagecb(int instance, DW_USB_EP *ep)
{
    DW_USB_Context *uc=&g_usbc[instance];

    USBDBG_L1("%s be called.\n", __FUNCTION__);

    union usb_callback_args args;
    invalidate_cache_range((const void*)(addr_t)uc->Setup, 8);
    args.setup = (struct usb_setup *)uc->Setup;
    if (!args.setup->length) {
        uc->three_stage_setup = false;
        uc->ep0_expect_in =false;
        uc->ep0_next_event = DW_EP0_NRDY_STATUS;
    } else {
        uc->three_stage_setup = true;
        uc->ep0_expect_in = !!(args.setup->request_type & DIR_IN);
        uc->ep0stage = EP0_DATA_PHASE;
        uc->ep0_next_event = DW_EP0_NRDY_DATA;
    }

    usbc_callback(USB_CB_SETUP_MSG, &args);
    uc->do_resched = true;
}

static void dw_usbc_datainstagecb(int instance, DW_USB_EP *ep)
{
    DW_USB_Context *uc=&g_usbc[instance];

    usbc_transfer_t *t = NULL;

    if (uc->ep_in[ep->num].transfer) {
        t = uc->ep_in[ep->num].transfer;
        uc->ep_in[ep->num].transfer = NULL;
        t->bufpos = ep->xfer_count;
        t->result = NO_ERROR;
        t->tran_sta = USB_TRANSFER_STATUS_FINISH;
        USBDBG_L1("%s: completing transfer: len %d, pos %d, cb %p\n", __FUNCTION__,
                  t->buflen, t->bufpos, t->callback);
        if (t->callback)
            t->callback(ep->num, t);
    }

    uc->do_resched = true;
}

static void dw_usbc_dataoutstagecb(int instance, DW_USB_EP *ep)
{
    USBDBG_L1("%s being called....\n", __FUNCTION__);

    DW_USB_Context *uc=&g_usbc[instance];

    usbc_transfer_t *t = NULL;

    if (uc->ep_out[ep->num].transfer) {
        t = uc->ep_out[ep->num].transfer;
        invalidate_cache_range((const void*)(addr_t)(t->buf),
                               ep->xfer_count);
        uc->ep_out[ep->num].transfer = NULL;
        t->bufpos = ep->xfer_count;
        t->result = NO_ERROR;
        t->tran_sta = USB_TRANSFER_STATUS_FINISH;
        USBDBG_L1("%s: completing transfer: buf %p, len %d, pos %d, cb %p\n", __FUNCTION__,
                  t->buf, t->buflen, t->bufpos, t->callback);
        if (t->callback)
            t->callback(ep->num|0x80, t);
    }

    uc->do_resched = true;
}

static void dw_usbc_ep0_xfernotready(int instance, DW_USB_EP *ep, depevt_t evt)
{
    DW_USB_Context *uc=&g_usbc[instance];
    switch (evt.evtstatus) {
    case DEPEVT_STATUS_CONTROL_DATA:
        if (uc->ep0_expect_in != ep->is_in) {
            ep = uc->ep0_expect_in ? &uc->IN_ep[0] : &uc->OUT_ep[0];
            dprintf(CRITICAL, "Wrong direction for Data phase\n");

            dw_usbc_ep_end_transfer(instance, ep);
            dw_usbc_ep0_stall(instance);
            if (uc->OUT_ep[0].is_stall) {
                uc->OUT_ep[0].is_stall = 0;
                uc->ep0stage = EP0_SETUP_PHASE;
                uc->three_stage_setup = false;
                dw_usbc_ep0_recv(instance, uc->Setup, 8, NULL);
            }
            return;
        }
        break;

    case DEPEVT_STATUS_CONTROL_STATUS:
        if (uc->ep0_next_event != DW_EP0_NRDY_STATUS)
            return;

        uc->ep0stage = EP0_STATUS_PHASE;
        uc->ep0_expect_in = !uc->ep0_expect_in;
        if (uc->ep0_expect_in) {
            dw_usbc_ep0_send(instance, uc->Setup, 0, 0);
        } else {
            dw_usbc_ep0_recv(instance, uc->Setup, 0, 0);
        }
        uc->ep0_next_event = DW_EP0_COMPLETE;
        break;
    default:
        break;
    }
}

static void dw_usb_handle_ep_special_evt(int instance,
        DW_USB_EP *ep, depevt_t evt)
{
    DW_USB_Context *uc=&g_usbc[instance];
    usbc_transfer_t * transfer = NULL;
    switch (evt.evt) {
    case 7:
        if(ep->num > 0 ){
            if(ep->is_in){
                transfer = uc->ep_in[ep->num & 0x7f].transfer;
            }else{
                transfer = uc->ep_out[ep->num & 0x7f].transfer;
            }
            if(transfer){
                transfer->tran_sta = USB_TRANSFER_STATUS_TRANNING;
            }
        }
        USBDBG_L1("%s: get epcmd complete on ep:%d, in:%d\n", __FUNCTION__,
                  ep->num,ep->is_in);
        break;
    case 6:
        USBDBG_L1("%s: get stream event on ep:%d, in:%d\n", __FUNCTION__,
                  ep->num,ep->is_in);
        break;
    case 3:     //xfernotready
        USBDBG_L1("%s: get xfernotready on ep:%d, in:%d\n", __FUNCTION__,
                  ep->num,ep->is_in);
        if (ep->num == 0) { //control ep
            dw_usbc_ep0_xfernotready(instance, ep, evt);
        }
        break;
    case 2:     //xferinprogress
         USBDBG_L1("%s: get xferinprogress on ep:%d, in:%d\n", __FUNCTION__,
                  ep->num,ep->is_in);
        break;
    case 1:     //xfercomplete
        USBDBG_L1("%s: get xfercomplete on ep:%d, in:%d\n", __FUNCTION__,
                  ep->num,ep->is_in);
        ep->res_id = 0;
        ep->is_busy =false;
        if (ep->num == 0) { //control ep
            trbd_t *trbd = ep->trbd;
            if (uc->ep0stage == EP0_SETUP_PHASE) {
                dw_usbc_setupstagecb(instance, ep);
                if (ep->is_stall) {
                    ep->is_stall = 0; 
                    uc->ep0stage = EP0_SETUP_PHASE;
                    uc->three_stage_setup = false;
                    dw_usbc_ep0_recv(instance, uc->Setup, 8, NULL);
                }
            } else if (uc->ep0stage == EP0_DATA_PHASE) {
                invalidate_cache_range((const void*)(addr_t)trbd->trb,
                                       sizeof(trb_t));
                if (ep->is_in == 0) {   /* data out */
                    uint32_t count = get_buffer_transferred(trbd->orig_size,
                                                            trbd->trb->trb2.bits.bufsize);
                    ep->xfer_count += count;
                    ep->xfer_buff += count;
                    ep->xfer_len -= count;

                    if ((ep->xfer_len == 0) || (count < ep->maxpacket)) {
                        uc->ep0_next_event = DW_EP0_NRDY_STATUS;
                        uc->ep0stage = EP0_STATUS_PHASE;
                        dw_usbc_dataoutstagecb(instance, ep);
                    } else if (ep->xfer_len > 0){
                        dw_usbc_ep_trans(instance, ep,
                                         ep->xfer_buff, ep->xfer_len);
                    }
                } else {
                    int count = get_buffer_transferred(trbd->orig_size,
                                                       trbd->trb->trb2.bits.bufsize);
                    ep->xfer_count += count;
                    ep->xfer_buff += count;
                    ep->xfer_len -= count;

                    if (ep->xfer_len == 0) {
                        uc->ep0_next_event = DW_EP0_NRDY_STATUS;
                        uc->ep0stage = EP0_STATUS_PHASE;
                        dw_usbc_datainstagecb(instance, ep);
                    }
                }
            } else if (uc->ep0stage == EP0_STATUS_PHASE) {
                USBDBG_L1("%s: setup stage complete\n", __FUNCTION__);
                uc->ep0stage = EP0_SETUP_PHASE;
                uc->three_stage_setup =false;
                dw_usbc_ep0_recv(instance, uc->Setup, 8, NULL);
            }
        } else {
            if (ep->is_in == 0) { //out
                trbd_t *trbd = ep->trbd;
                invalidate_cache_range((const void*)(addr_t)trbd->trb,
                                       sizeof(trb_t));
                uint32_t count = get_buffer_transferred(trbd->orig_size,
                                                        trbd->trb->trb2.bits.bufsize);
                ep->xfer_count += count;
                ep->xfer_buff += count;
                ep->xfer_len -= count;
                USBDBG_L1("%s: rx %d (%d bytes rx-ed totally), remain %d bytes\n", __FUNCTION__,
                          (int)count, (int)(ep->xfer_count), (int)(ep->xfer_len));

                /* !!! NOTE !!!
                 * To make driver less-complicate, the driver here makes some
                 * assumptions to the caller:
                 *  #1. Driver doesn't buffer any packet. It is caller's duty to
                 *      make sure the buffer provisioned is mps aligned
                 *  #2. The caller will take care of the ending packet.
                 */
                if ((ep->xfer_len == 0) || (count < ep->maxpacket)) {
                    dw_usbc_dataoutstagecb(instance, ep);
                } else if (ep->xfer_len > 0) {
                    dw_usbc_ep_trans(instance, ep, ep->xfer_buff, ep->xfer_len);
                }
            } else { // in
                trbd_t *trbd = ep->trbd;
                invalidate_cache_range((const void*)(addr_t)trbd->trb,
                                       sizeof(trb_t));
                uint32_t count = get_buffer_transferred(trbd->orig_size,
                                                        trbd->trb->trb2.bits.bufsize);
                ep->xfer_count += count;
                ep->xfer_buff += count;
                ep->xfer_len -= count;
                USBDBG_L1("%s: tx %d, remain %d bytes\n", __FUNCTION__,
                          (int)(ep->xfer_count), (int)(ep->xfer_len));

                if (ep->xfer_len == 0) {
                    if (count == ep->maxpacket) {
                        dw_usbc_ep_trans(instance, ep, NULL, 0);
                    } else {
                        dw_usbc_datainstagecb(instance, ep);
                    }
                } else if (ep->xfer_len > 0) {
                    dw_usbc_ep_trans(instance, ep, ep->xfer_buff, ep->xfer_len);
                }
            }
        }
        break;
    default:
        break;
    }
}

static void dw_usb_handle_dev_special_evt(int instance, DW_USB_EP *ep, devt_t evt)
{
    switch (evt.evtnum) {
    case 3: //usb/link state change
        dw_usbc_linksts_changecb(instance, evt.evtinfo);
        break;
    case 2: //connection done
        dw_usbc_conndonecb(instance);
        break;
    case 1: //usb reset
        dw_usbc_resetcb(instance);
        usbc_callback(USB_CB_RESET, NULL);
        break;
    case 0: //disconnect detect
        USBDBG_L2("%s: disconnect detect\n", __FUNCTION__);
        dw_usbc_disconnectcb(instance); 
        usbc_callback(USB_CB_DISCONNECT, NULL);
        break;
    default:
        USBDBG_L3("%s: get unknown event %d on ep:%d, in:%d \n", __FUNCTION__,
                  evt.evtnum, ep->num, ep->is_in);
        break;
    }
}

static void dw_usb_handle_ncr_evt(int instance)
{
    DW_USB_Context *uc=&g_usbc[instance];
    U32 evt = dw_usb_ll_get_ncr_evt(uc->io_base);
    if (evt & 0x7Fu) {
        // USBDBG_L1("%s: ncr event: 0x%x\n", __FUNCTION__, evt);
        dw_usb_ll_clr_ncr_evt(uc->io_base);
    }
}

static void dw_usb_handle_evt(int instance)
{
    DW_USB_Context *uc=&g_usbc[instance];
    DW_USB_EP *ep = &uc->OUT_ep[0];
    evtbd_t *evtbd = ep->evtbd;
    int eventcount;
    evt_t *evt;
    eventcount = dw_usbc_get_ep_event_count(instance, ep);
    if (eventcount ==0)
        return;
    dw_usb_ll_mask_irq(uc->io_base);

    while (eventcount) {
        invalidate_cache_range(
            (const void*)((addr_t)evtbd->evtb + evtbd->lpos), 4);
        evt=(evt_t *)((addr_t)evtbd->evtb + evtbd->lpos);
        // USBDBG_L1("%s: event type: 0x%x\n", __FUNCTION__, evt->devt.evttype);
        if (evt->devt.evttype == 0) { //ep special event
            DW_USB_EP *ept = ((evt->depevt.phy_epnum %2)==0 ?
                              &(uc->OUT_ep[evt->depevt.phy_epnum/2]):
                              &(uc->IN_ep[evt->depevt.phy_epnum/2]));
            dw_usb_handle_ep_special_evt(instance, ept, evt->depevt);
        } else if (evt->devt.devspec_evt == 0) { //dev special event
            dw_usb_handle_dev_special_evt(instance, ep, evt->devt);
        }
        eventcount -= 4;
        evtbd->lpos = (evtbd->lpos+4) % EVT_BUFF_SIZE;
        dw_usbc_clear_ep_event(instance, ep, 4);
    }
    dw_usb_ll_unmask_irq(uc->io_base);

    return;
}

enum handler_return dw_usb_irqhandler(void *args)
{
    int instance = (int)(uintptr_t)args;
    DW_USB_Context *uc=&g_usbc[instance];
    
    uc->do_resched = false;
    dw_usb_handle_evt(instance);
    dw_usb_handle_ncr_evt(instance);

    return INT_RESCHEDULE;
}

status_t dw_usbc_setup_endpoint(int instance, ep_t ept, ep_dir_t dir,
                                uint width, ep_type_t type,bool modify, bool restore)
{
    DW_USB_EP *ep= NULL;
    DW_USB_Context *uc=&g_usbc[instance];

    DEBUG_ASSERT(ept <= NUM_EP);

    if (dir == USB_IN)
        ep = &uc->IN_ep[ept];
    else
        ep = &uc->OUT_ep[ept];

    ep->is_in = (dir == USB_IN);
    ep->maxpacket = width;
    ep->type = type;

    if (!ep->is_enabled) { //only ep0out do this
        dw_usbc_set_ep_new_config(instance, ep);
        int i=0;
        DW_USB_EP *eptemp;
        for (i=0; i<NUM_EP; i++) {
            eptemp=&uc->OUT_ep[i];
            dw_usbc_set_ep_xfer_config(instance, eptemp);
            eptemp=&uc->IN_ep[i];
            dw_usbc_set_ep_xfer_config(instance, eptemp);
        }
    }
    dw_usbc_set_ep_config(instance, ep, modify, restore);

    if (ep->num ==0) {
        ep->trbd = uc->ep0_trbd;
    } else {
        ep->trbd = create_trb(instance, ep, 1);
    }

    if (!ep->is_enabled) {
        dw_usbc_ep_open(instance,  ep);
    }

    return NO_ERROR;
}

status_t dw_usbc_queue_rx(int instance, ep_t ep, usbc_transfer_t *transfer)
{
    DW_USB_Context *uc=&g_usbc[instance];
    DEBUG_ASSERT((ep & 0x7f) <= NUM_EP);
    if(uc->ep_out[ep & 0x7f].transfer != NULL){
        return ERR_NO_MEMORY;
    }

    USBDBG_L1("%s: queue rx %zd\n", __FUNCTION__, transfer->buflen);

    uc->ep_out[ep & 0x7f].transfer = transfer;
    DW_USB_EP *ept = &uc->OUT_ep[ep & 0x7f];
    ept->xfer_count = 0;
    dw_usbc_ep_trans(instance, ept, transfer->buf, transfer->buflen);

    return NO_ERROR;
}

status_t dw_usbc_queue_tx(int instance, ep_t ep, usbc_transfer_t *transfer)
{
    DW_USB_Context *uc=&g_usbc[instance];
    DEBUG_ASSERT((ep & 0x7f) <= NUM_EP);
    if(uc->ep_in[ep & 0x7f].transfer != NULL){
       return ERR_NO_MEMORY; 
    }
    USBDBG_L1("%s: queue tx %d\n", __FUNCTION__, transfer->buflen);

    uc->ep_in[ep & 0x7f].transfer = transfer;
    DW_USB_EP *ept=&uc->IN_ep[ep & 0x7f];
    ept->xfer_count = 0;
    dw_usbc_ep_trans(instance, ept, transfer->buf, transfer->buflen);

    return NO_ERROR;
}

status_t dw_usbc_flush_ep(int instance, ep_t ep)
{
    // Make sure The endpoint is in range.
    ASSERT((ep & 0x7F) <= NUM_EP);
    DW_USB_Context *uc=&g_usbc[instance];
    int p_num = 0;
    if ((ep & 0x80) == 0x80) {
        p_num = epnumtopnum((ep & 0x7F), 0);
    } else {
        p_num = epnumtopnum((ep & 0x7F), 1);
    }

    // Flush the FIFOs for the endpoint.
    if (dw_usb_ll_flush_ep(uc->io_base, p_num) != true) {
        return ERR_GENERIC;
    }

    // Clear any transfers that we may have been waiting on.
    if (ep & 0x80) {
        uc->ep_in[ep & 0x7F].transfer = NULL;
    } else {
        uc->ep_out[ep].transfer = NULL;
    }

    return NO_ERROR;
}

status_t dw_usbc_set_active(int instance, bool active)
{
    DW_USB_Context *uc=&g_usbc[instance];

    //write dctrl runstop bit to 1/0
    dw_usb_ll_runstop(uc->io_base, active);
    /* Note: 
     * z1 workaround to tell the edk we are ready.
     * Connected to phy ctrl_7_rsvd0 */
    /* Synced with design, this bit is RW but NC */
    REG_RMWREG32(uc->phy_base + 0x1001c, 0, 1, 1);

    return NO_ERROR;
}
void dw_usbc_set_address(int instance, uint8_t address)
{
    DW_USB_Context *uc=&g_usbc[instance];
    dw_usb_ll_set_address(uc->io_base, address);
}

void dw_usbc_ep0_ack(int instance)
{
#if 0
    DW_USB_Context *uc=&g_usbc[instance];
    struct ep_status *ep = &uc->ep_in[0];
    ep->ack_ep0_in = false;
    DW_USB_EP *ept=&uc->IN_ep[0];
    ept->xfer_count = 0;
    dw_usbc_ep_trans(instance, ept, 0, 0);
#endif
}

void dw_usbc_ep0_stall(int instance)
{
    DW_USB_Context *uc=&g_usbc[instance];
    dw_usbc_set_ep_stall(instance, &uc->OUT_ep[0]);
}

void dw_usbc_ep0_send(int instance, const void *buf, size_t len, size_t maxlen)
{
    DW_USB_Context *uc=&g_usbc[instance];
    USBDBG_L1("%s: tx len %d\n", __FUNCTION__, len);

    struct ep_status *ep = &uc->ep_in[0];
    int size;
    ep->ack_ep0_in = true;
    DW_USB_EP *ept=&uc->IN_ep[0];
    ept->xfer_count = 0;
    if (uc->three_stage_setup) {
        bool        direction;

        direction = uc->ep0_expect_in;
        if (direction != ept->is_in) {
            dprintf(CRITICAL,"direction error, expect %d, get %d\n",direction,ept->is_in);
            return;
        }
    }
    if (maxlen != 0)
        size = MIN(len, maxlen);
    else
        size = len;

    dw_usbc_ep_trans(instance, ept, (uint8_t *)buf, size);
    if (ept->num == 0)
        uc->ep0_next_event = DW_EP0_COMPLETE;
}

void dw_usbc_ep0_recv(int instance, void *buf, size_t len, ep_callback cb)
{
    DW_USB_Context *uc=&g_usbc[instance];
    struct ep_status *ep = &uc->ep_out[0];
    ep->ack_ep0_in = true;
    DW_USB_EP *ept=&uc->OUT_ep[0];
    ept->xfer_count = 0;
    if (uc->three_stage_setup) {
        bool direction;
        direction = uc->ep0_expect_in;
        if (direction != ept->is_in) {
            DBG("direction error, expect %d, get %d\n",direction,ept->is_in);
            return;
        }
        //uc->ep0stage = EP0_DATA_PHASE;
    }
    dw_usbc_ep_trans(instance, ept, (void *)buf, len);
    if (ept->num == 0)
        uc->ep0_next_event = DW_EP0_COMPLETE;
}

bool dw_usbc_is_highspeed(int instance)
{
    DW_USB_Context *uc=&g_usbc[instance];
    if (uc->speed == 0)
        return true;
    else
        return false;
}

int dw_usbc_get_cur_speed(int instance)
{
    DW_USB_Context *uc=&g_usbc[instance];
    return dw_usb_ll_get_cur_speed(uc->io_base);
}

bool if_usb_connected(void)
{
    DW_USB_Context *uc=&g_usbc[0];
    do {
        dw_usbc_schedule(0);
    } while (uc->do_resched);
    if (uc->State == HAL_USB_STATE_READY)   //connected
        return true;
    else
        return false;
}
