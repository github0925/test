/*
* dw_usb_ll.c
*
* Copyright (c) 2019 Semidrive Semiconductor.
* All rights reserved.
*
* Description: implement synopsys usb low level api
*
* Revision History:
* -----------------
* 011, 03/08/2019 chenqing create this file
*      03/20/2019 JerryFan Ported into ROM code.
*/

#include <soc_usb.h>
#include <dw_usb.h>
#include "dw_usb_ll.h"
#include <__regs_ap_u3drd_dwc_usb3.h>
#include <__regs_ap_u3drd_ncr.h>
#include <__regs_ap_u3phy_ncr.h>

#define RESET_TIMEOUT   1000
#define CMDACT_TIMEOUT  5000
/*0,hs,1,ls*/
#define GUSB2PHYCFG_PHYSEL 0
/*ulpi auto resume:0, disable;1, enable*/
#define GUSB2PHYCFG_ULPIAUTORES 0
/*UTMI PHYIF bits:0,8bits;1,16bits*/
#define GUSB2PHYCFG_PHYIF 1
/*usb turnaround time:5, utmi 16 bits; 9, utmi 8 bits or ulpi*/
#define GUSB2PHYCFG_USBTRDTIM 5
/*mode: 1,host; 2, device*/
#define GCTL_PRTCAPDIR 2
/*speed: 0, hs; 1,fs; 4, ss*/
#define DCFG_DEVSPD     1

static int reg_poll_value(vaddr_t reg, int start,
                          int width,uint32_t value,int retrycount)
{
    uint32_t v;
    do {
        v= readl(reg);
        if (((v>>start) & ((1<<width)-1)) == value)
            return 0;
        spin(1);
    } while (--retrycount);

    return -1;
}

bool dw_usb_dev_ll_por_srst(vaddr_t iobase, vaddr_t phybase)
{
    uint32_t v;
    int ret=0;

    /* By default, refclk sourced internally. Per design, this should be OK for
     * boot. Post-Boot SW can switch to external refclk(if exists on board) for
     * better USB complaince if desired */
    v=readl(phybase + REG_AP_APB_U3PHY_NCR_CTRL_0);
    if (FUSE_USB_REF_CLK_FRM_PAD()) {
        v |= BIT_AP_APB_U3PHY_NCR_CTRL_0_REF_USE_PAD;
    } else {
        v &= ~BIT_AP_APB_U3PHY_NCR_CTRL_0_REF_USE_PAD;
    }
    writel(v,phybase+REG_AP_APB_U3PHY_NCR_CTRL_0);
    if (FUSE_USB_REF_CLK_FRM_PAD()) {
        v = readl(phybase + REG_AP_APB_U3PHY_NCR_CTRL_1);
        v &= ~FM_AP_APB_U3PHY_NCR_CTRL_1_FSEL;
        /* Ref Clk in 100MHz */
        v |= (0x27 << FS_AP_APB_U3PHY_NCR_CTRL_1_FSEL);
        writel(v, phybase + REG_AP_APB_U3PHY_NCR_CTRL_1);
    }
    /* Analog ID Input Sample Enable */
    v = readl(phybase + REG_AP_APB_U3PHY_NCR_CTRL_3);
    v |= BIT_AP_APB_U3PHY_NCR_CTRL_3_IDPULLUP0;
    writel(v, phybase + REG_AP_APB_U3PHY_NCR_CTRL_3);

    /*controller NCR*/
    writel(0x1f, iobase+REG_AP_APB_U3DRD_NCR_INTEN);

    // writel( 123,iobase+REG_AP_APB_U3DRD_DWC_USB3_GUID);
    //do soft reset
    writel(1<<DCTL_CSFTRST_FIELD_OFFSET,iobase+REG_AP_APB_U3DRD_DWC_USB3_DCTL);
    ret=reg_poll_value(iobase+REG_AP_APB_U3DRD_DWC_USB3_DCTL,
                       DCTL_CSFTRST_FIELD_OFFSET,DCTL_CSFTRST_FIELD_SIZE,0,RESET_TIMEOUT);
    if (ret){
        DBG("soft reset fail.\n");
        return false;
    }
    /*gsbuscfg0,1*/
    /*gtxthrcfg,grxthrcfg*/
    v=readl(iobase+REG_AP_APB_U3DRD_DWC_USB3_GRXTHRCFG);
    v &= ~(1 << GRXTHRCFG_USBRXPKTCNTSEL_FIELD_OFFSET);
    writel(v, iobase+REG_AP_APB_U3DRD_DWC_USB3_GRXTHRCFG);
    /*gsnpsid*/
    //v=readl(iobase+REG_AP_APB_U3DRD_DWC_USB3_GSNPSID);
    //printf("usb controller version 0x%x\n",v);
    /*guid. optional*/
    /*gusb2phycfg*/
    v=readl(iobase+REG_AP_APB_U3DRD_DWC_USB3_GUSB2PHYCFG);

    RMWREG32(&v,GUSB2PHYCFG_PHYSEL_FIELD_OFFSET,GUSB2PHYCFG_PHYSEL_FIELD_SIZE,GUSB2PHYCFG_PHYSEL);//hs

    RMWREG32(&v,GUSB2PHYCFG_ULPIAUTORES_FIELD_OFFSET,GUSB2PHYCFG_ULPIAUTORES_FIELD_SIZE,GUSB2PHYCFG_ULPIAUTORES); //disable ulpiautoresume
    RMWREG32(&v,GUSB2PHYCFG_PHYIF_FIELD_OFFSET,GUSB2PHYCFG_PHYIF_FIELD_SIZE,GUSB2PHYCFG_PHYIF); //PHYIF 16bit
    RMWREG32(&v,GUSB2PHYCFG_USBTRDTIM_FIELD_OFFSET,GUSB2PHYCFG_USBTRDTIM_FIELD_SIZE,GUSB2PHYCFG_USBTRDTIM); //UTMI 16bit
    writel(v, iobase+REG_AP_APB_U3DRD_DWC_USB3_GUSB2PHYCFG);
    /*gusb3pipectl*/
    v=readl(iobase + REG_AP_APB_U3DRD_DWC_USB3_GUSB3PIPECTL);
    v |= 1<<GUSB3PIPECTL_SUSPENDENABLE_FIELD_OFFSET;
    writel(v, iobase+REG_AP_APB_U3DRD_DWC_USB3_GUSB3PIPECTL);
    /*gtxfifosizn*/
    /*grxfifosiz0*/
    /*gevntadrn*/
    /*gevntsizn*/
    /*gevntcountn*/
    /*gctl*/
    v=readl(iobase+REG_AP_APB_U3DRD_DWC_USB3_GCTL);
#if (defined(DEBUG_ENABLE) || defined(VTEST_USB)) && !defined(TC_z1) && !defined(TC_zebu) && !defined(TC_fpga)
    /* Scale-Down mode for faster simulation */
    RMWREG32(&v, GCTL_U1U2TIMERSCALE_FIELD_OFFSET, GCTL_U1U2TIMERSCALE_FIELD_SIZE, 1);
    RMWREG32(&v, GCTL_SCALEDOWN_FIELD_OFFSET, GCTL_SCALEDOWN_FIELD_SIZE, 3);
#else
    RMWREG32(&v, GCTL_U1U2TIMERSCALE_FIELD_OFFSET, GCTL_U1U2TIMERSCALE_FIELD_SIZE, 0);
    RMWREG32(&v, GCTL_SCALEDOWN_FIELD_OFFSET, GCTL_SCALEDOWN_FIELD_SIZE, 0);    //remove scale down
#endif
    RMWREG32(&v, GCTL_DSBLCLKGTNG_FIELD_OFFSET, GCTL_DSBLCLKGTNG_FIELD_SIZE, 0);//remove clk gating
    RMWREG32(&v, GCTL_DISSCRAMBLE_FIELD_OFFSET, GCTL_DISSCRAMBLE_FIELD_SIZE, 0);//DW_GCTL_DISSCRAMBLE;
    RMWREG32(&v, GCTL_PRTCAPDIR_FIELD_OFFSET,GCTL_PRTCAPDIR_FIELD_SIZE,GCTL_PRTCAPDIR);//dev MODE
    writel(v, iobase+REG_AP_APB_U3DRD_DWC_USB3_GCTL);
    /*dcfg*/
    v=readl(iobase+REG_AP_APB_U3DRD_DWC_USB3_DCFG);
    RMWREG32(&v,DCFG_DEVSPD_FIELD_OFFSET,DCFG_DEVSPD_FIELD_SIZE,DCFG_DEVSPD);//despd,hs
    //RMWREG32(&v,17,5,9);//nump
    writel(v, iobase+REG_AP_APB_U3DRD_DWC_USB3_DCFG);
    /*guctl2*/
    v = readl(iobase+REG_AP_APB_U3DRD_DWC_USB3_GUCTL2);
    v |= (1 << GUCTL2_RST_ACTBITLATER_FIELD_OFFSET);
    writel(v,iobase+REG_AP_APB_U3DRD_DWC_USB3_GUCTL2);
    /*dctl,runstop*/

    /* ATB-35: usb phy reset has to be toggled after phy setting configured */
    v = readl(phybase + REG_AP_APB_U3PHY_NCR_CTRL_0);
    v |= BIT_AP_APB_U3PHY_NCR_CTRL_0_PHY_RESET;
    writel(v, phybase + REG_AP_APB_U3PHY_NCR_CTRL_0);
    /* longer than enough. Still use this number to align with the romfix */
    udelay(1280);
    v &= ~BIT_AP_APB_U3PHY_NCR_CTRL_0_PHY_RESET;
    writel(v, phybase + REG_AP_APB_U3PHY_NCR_CTRL_0);

    return true;
}

void dw_usb_ll_enable_irq(vaddr_t iobase)
{
    uint32_t v = 0;
    /* Reduce unnecessary interruptions to avoid affecting DDR pressure */
    // RMWREG32(&v,DEVTEN_ULSTCNGEN_FIELD_OFFSET,DEVTEN_ULSTCNGEN_FIELD_SIZE,1);//usb link state change
    RMWREG32(&v,DEVTEN_CONNECTDONEEVTEN_FIELD_OFFSET,DEVTEN_CONNECTDONEEVTEN_FIELD_SIZE,1);//connect done
    RMWREG32(&v,DEVTEN_USBRSTEVTEN_FIELD_OFFSET,DEVTEN_USBRSTEVTEN_FIELD_SIZE,1);//usb reset
    RMWREG32(&v,DEVTEN_DISSCONNEVTEN_FIELD_OFFSET,DEVTEN_DISSCONNEVTEN_FIELD_SIZE,1);//usb disconnect
    writel(v, iobase+REG_AP_APB_U3DRD_DWC_USB3_DEVTEN);
    dw_usb_ll_unmask_irq(iobase);
}
void dw_usb_ll_disable_irq(vaddr_t iobase)
{
    writel(0, iobase+REG_AP_APB_U3DRD_DWC_USB3_DEVTEN);
}
void dw_usb_ll_mask_irq(vaddr_t iobase)
{
    REG_RMWREG32(iobase+REG_AP_APB_U3DRD_DWC_USB3_GEVNTSIZ,
                 GEVNTSIZ_EVNTINTRPTMASK_FIELD_OFFSET,
                 GEVNTSIZ_EVNTINTRPTMASK_FIELD_SIZE, 1);
}
void dw_usb_ll_unmask_irq(vaddr_t iobase)
{
    REG_RMWREG32(iobase+REG_AP_APB_U3DRD_DWC_USB3_GEVNTSIZ,
                 GEVNTSIZ_EVNTINTRPTMASK_FIELD_OFFSET,
                 GEVNTSIZ_EVNTINTRPTMASK_FIELD_SIZE, 0);
}

bool dw_usb_ll_runstop(vaddr_t iobase, bool enable)
{
    uint32_t reg, timeout=500;

    REG_RMWREG32(iobase+REG_AP_APB_U3DRD_DWC_USB3_DCTL,
                 DCTL_RUN_STOP_FIELD_OFFSET, DCTL_RUN_STOP_FIELD_SIZE, enable);
    do {
        reg = readl(iobase + REG_AP_APB_U3DRD_DWC_USB3_DSTS);
        reg &= (1 << DSTS_DEVCTRLHLT_FIELD_OFFSET);
    } while (--timeout && !((!enable) ^ (!reg)));

    if (!timeout) {
        DBG("%s: Opps, timeout\n", __FUNCTION__);
        return false;
    }
    return true;
}

bool dw_usb_ll_set_ep_cmd(vaddr_t iobase, int cmdnum, int p_epnum,
                          uint32_t cmdvalue, uint32_t param0,
                          uint32_t param1, uint32_t param2)
{
    uint32_t v,saved_config=0;
    int ret;
    /*set gusb2phycfg 6,8 to 0*/
    v=readl(iobase+REG_AP_APB_U3DRD_DWC_USB3_GUSB2PHYCFG);
    if (v & (1<<GUSB2PHYCFG_SUSPENDUSB20_FIELD_OFFSET)) {
        saved_config |= 1<<GUSB2PHYCFG_SUSPENDUSB20_FIELD_OFFSET;
        v &= ~(1<<GUSB2PHYCFG_SUSPENDUSB20_FIELD_OFFSET);
    }

    if (v & (1<<GUSB2PHYCFG_ENBLSLPM_FIELD_OFFSET)) {
        saved_config |= (1<<GUSB2PHYCFG_ENBLSLPM_FIELD_OFFSET);
        v &= ~(1<<GUSB2PHYCFG_ENBLSLPM_FIELD_OFFSET);
    }

    if (saved_config)
        writel(v, iobase+REG_AP_APB_U3DRD_DWC_USB3_GUSB2PHYCFG);

    writel(param0, iobase+REG_AP_APB_U3DRD_DWC_USB3_DEPCMDPAR0+p_epnum*0x10);
    writel(param1, iobase+REG_AP_APB_U3DRD_DWC_USB3_DEPCMDPAR1+p_epnum*0x10);
    writel(param2, iobase+REG_AP_APB_U3DRD_DWC_USB3_DEPCMDPAR2+p_epnum*0x10);

    v=cmdvalue;
    /* smile */
    if(!(readl(iobase+REG_AP_APB_U3DRD_DWC_USB3_DCTL) & BIT_AP_APB_U3DRD_DWC_USB3_DCTL_RUN_STOP)){
        RMWREG32(&v, DEPCMD_CMDIOC_FIELD_OFFSET, DEPCMD_CMDIOC_FIELD_SIZE, 0);// cmdioc=0
    }else{
        RMWREG32(&v, DEPCMD_CMDIOC_FIELD_OFFSET, DEPCMD_CMDIOC_FIELD_SIZE, 1);// cmdioc=0
    }
    /* smile */

    RMWREG32(&v, DEPCMD_CMDTYP_FIELD_OFFSET, DEPCMD_CMDTYP_FIELD_SIZE, cmdnum);// set cmdtype
    if (cmdnum != 7) //update
        RMWREG32(&v, DEPCMD_CMDACT_FIELD_OFFSET, DEPCMD_CMDACT_FIELD_SIZE, 1);// set cmdact
    writel(v, iobase+REG_AP_APB_U3DRD_DWC_USB3_DEPCMD+p_epnum*0x10);
    ret=reg_poll_value(iobase+REG_AP_APB_U3DRD_DWC_USB3_DEPCMD+p_epnum*0x10,
                       DEPCMD_CMDACT_FIELD_OFFSET,DEPCMD_CMDACT_FIELD_SIZE,0,CMDACT_TIMEOUT);
    if (!ret) {
        v= readl(iobase+REG_AP_APB_U3DRD_DWC_USB3_DEPCMD+p_epnum*0x10);
        ret=DW_GET_CMD_STATUS(v);
    }else{
        DBG("ep%d cmd fail.\n\n",p_epnum);
    }

    /*set gusb2phycfg 6,8 to 1*/
    if (saved_config) {
        v=readl(iobase+REG_AP_APB_U3DRD_DWC_USB3_GUSB2PHYCFG);
        v |= saved_config;
        writel(v, iobase+REG_AP_APB_U3DRD_DWC_USB3_GUSB2PHYCFG);
    }
    if (ret != 0) {
        printf("ep %d cmd %d status %d\n",p_epnum,cmdnum,ret);
        return false;
    }
    return true;
}

bool dw_usb_ll_set_ep_config(vaddr_t iobase, int p_epnum,
                             uint32_t param0,uint32_t param1, uint32_t param2)
{
    /*depcmd*/
    return dw_usb_ll_set_ep_cmd(iobase, EPCMD_SET_CONFIG,
                                p_epnum, 0, param0, param1, param2);
}

bool dw_usb_ll_set_ep_xfer_config(vaddr_t iobase, int p_epnum, uint32_t param0)
{
    /*depcmd*/
    return dw_usb_ll_set_ep_cmd(iobase,EPCMD_SET_XFER,
                                p_epnum, 0, param0, 0, 0);
}

uint32_t dw_usb_ll_get_ep_stat(vaddr_t iobase, int p_epnum)
{
    uint32_t v;
    /*depcmd*/
    dw_usb_ll_set_ep_cmd(iobase,EPCMD_GET_STAT,p_epnum,0,0,0,0);
    v= readl(iobase+REG_AP_APB_U3DRD_DWC_USB3_DEPCMDPAR2+p_epnum*0x10);
    return v;
}

u32 dw_usb_ll_get_ep_transfer_index(vaddr_t iobase, int p_epnum)
{
    u32         res_id,v;

    v=readl(iobase + REG_AP_APB_U3DRD_DWC_USB3_DEPCMD + p_epnum*0x10);

    res_id = DW_GET_XFER_RES_ID(v);

    return res_id;
}

int dw_usb_ll_set_link_state(vaddr_t iobase, enum dw_link_state state)
{
    int     retries = 10000;
    u32     reg;

    while (--retries) {
        reg = readl(iobase+REG_AP_APB_U3DRD_DWC_USB3_DSTS);
        if (reg & DW_DSTS_DCNRD)
            spin(5);
        else
            break;
    }

    if (retries <= 0)
        return -1;

    reg = readl(iobase+REG_AP_APB_U3DRD_DWC_USB3_DCTL);
    reg &= ~DW_DCTL_ULSTCHNGREQ_MASK;

    /* set requested state */
    reg |= DW_DCTL_ULSTCHNGREQ(state);
    writel(reg, iobase+REG_AP_APB_U3DRD_DWC_USB3_DCTL);

    return 0;
}

int dw_usb_ll_disconnect_evt(vaddr_t iobase)
{
    u32 reg = readl(iobase + REG_AP_APB_U3DRD_DWC_USB3_DCTL);
    reg &= ~BIT_AP_APB_U3DRD_DWC_USB3_DCTL_INITU1ENA;
    writel(reg, iobase + REG_AP_APB_U3DRD_DWC_USB3_DCTL);
    reg &= ~BIT_AP_APB_U3DRD_DWC_USB3_DCTL_INITU2ENA;
    writel(reg, iobase + REG_AP_APB_U3DRD_DWC_USB3_DCTL);
    return 0;
}

int dw_usb_ll_wakeup(vaddr_t iobase)
{
    int         retries;

    int         ret;
    u32         reg;

    u8          link_state;
    u8          speed;

    reg = readl(iobase + REG_AP_APB_U3DRD_DWC_USB3_DSTS);

    speed = DW_GET_CONNDONE_SPEED(reg);
    if ((speed == USB_SPEED_SS) ||
        (speed == USB_SPEED_SSPLUS)) {
        return 0;
    }

    link_state = DW_DSTS_USBLNKST(reg);

    switch (link_state) {
    case DW_LINK_STATE_RX_DET:      /* in HS, means Early Suspend */
    case DW_LINK_STATE_U3:          /* in HS, means SUSPEND */
        break;
    default:
        DBG("%s: can't wakeup from %d", __FUNCTION__, link_state);
        return -1;
    }

    ret = dw_usb_ll_set_link_state(iobase, DW_LINK_STATE_RECOV);
    if (ret < 0) {
        DBG("%s: failed to put link in Recovery\n", __FUNCTION__);
        return ret;
    }

    /* poll until Link State changes to ON */
    retries = 20000;

    while (retries--) {
        reg = readl(iobase+REG_AP_APB_U3DRD_DWC_USB3_DSTS);

        /* in HS, means ON */
        if (DW_DSTS_USBLNKST(reg) == DW_LINK_STATE_U0)
            break;
    }

    if (DW_DSTS_USBLNKST(reg) != DW_LINK_STATE_U0) {
        DBG("%s: failed to send remote wakeup\n", __FUNCTION__);
        return -1;
    }

    return 0;
}

bool dw_usb_ll_set_ep_stall(vaddr_t iobase, int p_epnum)
{
    return dw_usb_ll_set_ep_cmd(iobase, EPCMD_SET_STALL,p_epnum, 0, 0, 0, 0);
}

bool dw_usb_ll_clear_ep_stall(vaddr_t iobase, int p_epnum, uint32_t param0)
{
    return dw_usb_ll_set_ep_cmd(iobase, EPCMD_CLEAR_STALL,
                                p_epnum, 0, param0, 0, 0);
}

bool dw_usb_ll_ep_start_transfer(vaddr_t iobase, int p_epnum,
                                 uint32_t param0,uint32_t param1)
{
    return dw_usb_ll_set_ep_cmd(iobase, EPCMD_START_TRANS,
                                p_epnum, 0, param0, param1, 0);
}

bool dw_usb_ll_ep_update_transfer(vaddr_t iobase, int p_epnum, int res_id)
{
    /*can send no response update*/
    return dw_usb_ll_set_ep_cmd(iobase, EPCMD_UPDATE_TRANS,
                                p_epnum, res_id<<16, 0, 0, 0);
}
bool dw_usb_ll_ep_end_transfer(vaddr_t iobase, int p_epnum,int res_id)
{
    uint32_t v = 0;
    /*trb index*/
    v |= res_id <<DEPCMD_COMMANDPARAM_FIELD_OFFSET;
    /*forceRM*/
    v |= 1<<DEPCMD_HIPRI_FORCERM_FIELD_OFFSET;
    /*cmdioc 8*/
    v |= 1<<DEPCMD_CMDIOC_FIELD_OFFSET;

    return dw_usb_ll_set_ep_cmd(iobase, EPCMD_END_TRANS, p_epnum, v, 0, 0, 0);
}

bool dw_usb_ll_set_ep_new_config(vaddr_t iobase, int p_epnum)
{
    return dw_usb_ll_set_ep_cmd(iobase, EPCMD_SET_NEW_CONFIG, 0, 0, 0, 0, 0);
}

bool dw_usb_ll_flush_ep(vaddr_t iobase, int p_epnum)
{
    return true;
}

bool dw_usb_ll_set_ep_event_buffer(vaddr_t iobase, int p_epnum,
                                   uint32_t adr_l,uint32_t adr_h,uint16_t size)
{
    //gevntadrlo
    writel(adr_l, iobase+REG_AP_APB_U3DRD_DWC_USB3_GEVNTADRLO+p_epnum*0x10);
    //gevntadrhi
    writel(adr_h, iobase+REG_AP_APB_U3DRD_DWC_USB3_GEVNTADRHI+p_epnum*0x10);
    //gevntsiz
    REG_RMWREG32(iobase+REG_AP_APB_U3DRD_DWC_USB3_GEVNTSIZ+p_epnum*0x10, GEVNTSIZ_EVENTSIZ_FIELD_OFFSET,GEVNTSIZ_EVENTSIZ_FIELD_SIZE,size);
    //gevntcount
    writel(0, iobase+REG_AP_APB_U3DRD_DWC_USB3_GEVNTCOUNT+p_epnum*0x10);
    return true;
}

uint16_t dw_usb_ll_get_ep_event_count(vaddr_t iobase, int p_epnum)
{
    uint16_t v=0;
    v=(readl(iobase+REG_AP_APB_U3DRD_DWC_USB3_GEVNTCOUNT+p_epnum*0x10) & 0xfffc);
    return v;
}

bool dw_usb_ll_clear_ep_event(vaddr_t iobase, int p_epnum,uint16_t size)
{
    //gevntcount, clear size bytes
    writel(size, iobase + REG_AP_APB_U3DRD_DWC_USB3_GEVNTCOUNT + p_epnum*0x10);
    return true;
}

bool dw_usb_ll_ep_open(vaddr_t iobase, int p_epnum)
{
    uint32_t v;
    /*dalepena*/
    v=readl(iobase+REG_AP_APB_U3DRD_DWC_USB3_DALEPENA);
    v |= 1 << p_epnum;
    writel(v, iobase+REG_AP_APB_U3DRD_DWC_USB3_DALEPENA);
    return true;
}

bool dw_usb_ll_ep_close(vaddr_t iobase, int p_epnum)
{
    uint32_t v;
    /*dalepena*/
    v=readl(iobase+REG_AP_APB_U3DRD_DWC_USB3_DALEPENA);
    v &= ~(1 << p_epnum);
    writel(v, iobase+REG_AP_APB_U3DRD_DWC_USB3_DALEPENA);
    return true;
}

bool dw_usb_ll_set_address(vaddr_t iobase, uint8_t addr)
{
    uint32_t v;
    /*dcfg*/
    v=readl(iobase+REG_AP_APB_U3DRD_DWC_USB3_DCFG);
    RMWREG32(&v,DCFG_DEVADDR_FIELD_OFFSET,DCFG_DEVADDR_FIELD_SIZE,addr);//devaddr
    writel(v, iobase+REG_AP_APB_U3DRD_DWC_USB3_DCFG);

    return true;
}

int dw_usb_ll_get_cur_speed(vaddr_t iobase)
{
    int speed;
    uint32_t v=readl(iobase + REG_AP_APB_U3DRD_DWC_USB3_DSTS);
    speed = DW_GET_CONNDONE_SPEED(v);
    return speed;
}

uint32_t dw_usb_ll_get_ncr_evt(vaddr_t iobase)
{
    U32 v = readl(iobase + REG_AP_APB_U3DRD_NCR_INTR);
    return v;
}

void dw_usb_ll_clr_ncr_evt(vaddr_t iobase)
{
    U32 v = readl(iobase + REG_AP_APB_U3DRD_NCR_INTR);
    if (v) {
        writel(v & 0x7Fu, iobase + REG_AP_APB_U3DRD_NCR_INTR);
    }
}

