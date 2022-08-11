/*
* dw_usb_ll.c
*
* Copyright (c) 2018 Semidrive Semiconductor.
* All rights reserved.
*
* Description: implement synopsys usb low level api
*
* Revision History:
* -----------------
* 011, 03/08/2019 chenqing create this file
*/
//#include <dw_usb.h>
#include "dw_usb_ll.h"
#include <reg.h>
#include <errno.h>
#include <debug.h>

#define BIT_(x)  (1 << x)

#define RESET_TIMEOUT 500
#define CMDACT_TIMEOUT 500
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
#define DCFG_DEVSPD 0

//regs_ap_u3drd_ncr.h
#define U3DRD_NCR_BASE_ADDR 0xD000
#define U3DRD_NCR_INTEN (U3DRD_NCR_BASE_ADDR + 0x0)
#define U3DRD_NCR_INTR (U3DRD_NCR_BASE_ADDR + 0x80)

//regs_ap_u3phy_ncr.h
#define U3PHY_NCR_BASE_ADDR 0x10000
#define U3PHY_NCR_CTRL_0 (U3PHY_NCR_BASE_ADDR + 0x0)
#define U3PHY_NCR_CTRL_3 (U3PHY_NCR_BASE_ADDR + 0xc)
#define BIT_U3PHY_NCR_CTRL_0_REF_USE_PAD    (BIT_(18))
#define BIT_U3PHY_NCR_CTRL_0_PHY_RESET      (BIT_(0))
#define BIT_U3PHY_NCR_CTRL_3_IDPULLUP0      (BIT_(8))

//regs_ap_u3drd_dwc_usb3.h
#define DWC_USB3_BLOCK_DEV_BASE_ADDR 0xc700
#define U3DRD_DWC_USB3_DCTL (DWC_USB3_BLOCK_DEV_BASE_ADDR + (0x4<<0))
#define DCTL_RUN_STOP_FIELD_OFFSET 31
#define DCTL_RUN_STOP_FIELD_SIZE 1
#define DCTL_CSFTRST_FIELD_OFFSET 30
#define DCTL_CSFTRST_FIELD_SIZE 1
#define BIT_U3DRD_DWC_USB3_DCTL_INITU1ENA    (BIT_(10))
#define BIT_U3DRD_DWC_USB3_DCTL_INITU2ENA    (BIT_(12))

#define DWC_USB3_BLOCK_GBL_BASE_ADDR 0xc100
#define U3DRD_DWC_USB3_GRXTHRCFG (DWC_USB3_BLOCK_GBL_BASE_ADDR + (0xc<<0))
#define GRXTHRCFG_USBRXPKTCNTSEL_FIELD_OFFSET 29

#define U3DRD_DWC_USB3_GUSB2PHYCFG (DWC_USB3_BLOCK_GBL_BASE_ADDR + (0x100<<0))
#define GUSB2PHYCFG_PHYIF_FIELD_OFFSET 3
#define GUSB2PHYCFG_PHYIF_FIELD_SIZE 1
#define GUSB2PHYCFG_SUSPENDUSB20_FIELD_OFFSET 6
#define GUSB2PHYCFG_SUSPENDUSB20_FIELD_SIZE 1
#define GUSB2PHYCFG_PHYSEL_FIELD_OFFSET 7
#define GUSB2PHYCFG_PHYSEL_FIELD_SIZE 1
#define GUSB2PHYCFG_ENBLSLPM_FIELD_OFFSET 8
#define GUSB2PHYCFG_ENBLSLPM_FIELD_SIZE 1
#define GUSB2PHYCFG_USBTRDTIM_FIELD_OFFSET 10
#define GUSB2PHYCFG_USBTRDTIM_FIELD_SIZE 4
#define GUSB2PHYCFG_ULPIAUTORES_FIELD_OFFSET 15
#define GUSB2PHYCFG_ULPIAUTORES_FIELD_SIZE 1

#define U3DRD_DWC_USB3_GUSB3PIPECTL (DWC_USB3_BLOCK_GBL_BASE_ADDR + (0x1c0<<0))
#define GUSB3PIPECTL_SUSPENDENABLE_FIELD_OFFSET 17

#define U3DRD_DWC_USB3_GCTL (DWC_USB3_BLOCK_GBL_BASE_ADDR + (0x10<<0))
#define GCTL_SCALEDOWN_FIELD_OFFSET 4
#define GCTL_SCALEDOWN_FIELD_SIZE 2
#define GCTL_DSBLCLKGTNG_FIELD_OFFSET 0
#define GCTL_DSBLCLKGTNG_FIELD_SIZE 1
#define GCTL_DISSCRAMBLE_FIELD_OFFSET 3
#define GCTL_DISSCRAMBLE_FIELD_SIZE 1
#define GCTL_PRTCAPDIR_FIELD_OFFSET 12
#define GCTL_PRTCAPDIR_FIELD_SIZE 2

#define U3DRD_DWC_USB3_DCFG (DWC_USB3_BLOCK_DEV_BASE_ADDR + (0x0<<0))
#define DCFG_DEVSPD_FIELD_OFFSET 0
#define DCFG_DEVSPD_FIELD_SIZE 3
#define DCFG_DEVADDR_FIELD_OFFSET 3
#define DCFG_DEVADDR_FIELD_SIZE 7

#define U3DRD_DWC_USB3_GUCTL2 (DWC_USB3_BLOCK_GBL_BASE_ADDR + (0x9c<<0))
#define GUCTL2_RST_ACTBITLATER_FIELD_OFFSET 14

#define U3DRD_DWC_USB3_DEVTEN (DWC_USB3_BLOCK_DEV_BASE_ADDR + (0x8<<0))
#define DEVTEN_ULSTCNGEN_FIELD_OFFSET 3
#define DEVTEN_ULSTCNGEN_FIELD_SIZE 1
#define DEVTEN_CONNECTDONEEVTEN_FIELD_OFFSET 2
#define DEVTEN_CONNECTDONEEVTEN_FIELD_SIZE 1
#define DEVTEN_USBRSTEVTEN_FIELD_OFFSET 1
#define DEVTEN_USBRSTEVTEN_FIELD_SIZE 1
#define DEVTEN_DISSCONNEVTEN_FIELD_OFFSET 0
#define DEVTEN_DISSCONNEVTEN_FIELD_SIZE 1

#define U3DRD_DWC_USB3_GEVNTSIZ (DWC_USB3_BLOCK_GBL_BASE_ADDR + (0x308<<0))
#define GEVNTSIZ_EVNTINTRPTMASK_FIELD_OFFSET 31
#define GEVNTSIZ_EVNTINTRPTMASK_FIELD_SIZE 1
#define GEVNTSIZ_EVENTSIZ_FIELD_OFFSET 0
#define GEVNTSIZ_EVENTSIZ_FIELD_SIZE 16

#define U3DRD_DWC_USB3_DSTS (DWC_USB3_BLOCK_DEV_BASE_ADDR + (0xc<<0))
#define DSTS_DEVCTRLHLT_FIELD_OFFSET 22
#define DSTS_DEVCTRLHLT_FIELD_SIZE 1

#define U3DRD_DWC_USB3_DEPCMDPAR0 (DWC_USB3_BLOCK_DEV_BASE_ADDR + (0x108<<0))
#define U3DRD_DWC_USB3_DEPCMDPAR1 (DWC_USB3_BLOCK_DEV_BASE_ADDR + (0x104<<0))
#define U3DRD_DWC_USB3_DEPCMDPAR2 (DWC_USB3_BLOCK_DEV_BASE_ADDR + (0x100<<0))

#define U3DRD_DWC_USB3_DEPCMD (DWC_USB3_BLOCK_DEV_BASE_ADDR + (0x10c<<0))
#define DEPCMD_CMDTYP_FIELD_OFFSET 0
#define DEPCMD_CMDTYP_FIELD_SIZE 4
#define DEPCMD_CMDACT_FIELD_OFFSET 10
#define DEPCMD_CMDACT_FIELD_SIZE 1
#define DEPCMD_COMMANDPARAM_FIELD_OFFSET 16
#define DEPCMD_HIPRI_FORCERM_FIELD_OFFSET 11
#define DEPCMD_CMDIOC_FIELD_OFFSET 8

#define U3DRD_DWC_USB3_GEVNTADRLO (DWC_USB3_BLOCK_GBL_BASE_ADDR + (0x300<<0))
#define U3DRD_DWC_USB3_GEVNTADRHI (DWC_USB3_BLOCK_GBL_BASE_ADDR + (0x304<<0))
#define U3DRD_DWC_USB3_GEVNTCOUNT (DWC_USB3_BLOCK_GBL_BASE_ADDR + (0x30c<<0))
#define U3DRD_DWC_USB3_DALEPENA   (DWC_USB3_BLOCK_DEV_BASE_ADDR + (0x20<<0))


#if 0
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

static int32_t usbregs[]= {
	0xc704,/*dtcl*/
	0xc100,/*GRXTHRCFG*/
	0xc108,/*GTXTHRCFG*/
	0xc200,/*GUSB2PHYCFG*/
	0xc110,/*gctl*/
	0xc700,/*dcfg*/
	0xc300,/*U3DRD_DWC_USB3_GTXFIFOSIZ0*/
	0xc304,/*U3DRD_DWC_USB3_GTXFIFOSIZ1*/
	0xc2c0,/*GUSB3PIPECTL*/
};
void dump_registers(vaddr_t iobase,const char *pre)
{
	unsigned i=0;
	printf("\n%s:\n",pre);
	for (i=0; i<ARRAY_SIZE(usbregs); i++)
		printf("0x%x:0x%08x\n",usbregs[i],readl(iobase+usbregs[i]));
}
#else
void dump_registers(vaddr_t iobase,const char *pre)
{
}
#endif

static int reg_poll_value(vaddr_t reg, int start, int width,uint32_t value,int retrycount)
{
	uint32_t v;
	do {
		v= readl(reg);
		if (((v>>start) & ((1<<width)-1)) == value)
			return 0;
		spin(1);
	} while (--retrycount);
	return -ETIMEDOUT;
}

bool dw_usb_dev_ll_por_srst(vaddr_t iobase, vaddr_t phybase)
{
	uint32_t v;
	int ret=0;

	/*config usb phy*/
	v=readl(phybase+U3PHY_NCR_CTRL_0);
	v &=~(BIT_U3PHY_NCR_CTRL_0_REF_USE_PAD);
	writel(v,phybase+U3PHY_NCR_CTRL_0);

	v=readl(phybase+U3PHY_NCR_CTRL_3);
	v |=BIT_U3PHY_NCR_CTRL_3_IDPULLUP0;
	writel(v,phybase+U3PHY_NCR_CTRL_3);
	/*controller NCR*/
	writel(0x1f, iobase+U3DRD_NCR_INTEN);

	v=readl(phybase+U3PHY_NCR_CTRL_0);
	v |= (BIT_U3PHY_NCR_CTRL_0_PHY_RESET);
	writel(v,phybase+U3PHY_NCR_CTRL_0);

	spin(100);
	v=readl(phybase+U3PHY_NCR_CTRL_0);
	v &=~(BIT_U3PHY_NCR_CTRL_0_PHY_RESET);
	writel(v,phybase+U3PHY_NCR_CTRL_0);
	/**/
	//do soft reset
	writel(1<<DCTL_CSFTRST_FIELD_OFFSET,iobase+U3DRD_DWC_USB3_DCTL);
	ret=reg_poll_value(iobase+U3DRD_DWC_USB3_DCTL,DCTL_CSFTRST_FIELD_OFFSET,DCTL_CSFTRST_FIELD_SIZE,0,RESET_TIMEOUT);
	if (ret)
		return false;
	/*gsbuscfg0,1*/
	/*gtxthrcfg,grxthrcfg*/
	v=readl(iobase+U3DRD_DWC_USB3_GRXTHRCFG);
	v &= ~(1 << GRXTHRCFG_USBRXPKTCNTSEL_FIELD_OFFSET);
	writel(v, iobase+U3DRD_DWC_USB3_GRXTHRCFG);
	/*gsnpsid*/
	//v=readl(iobase+U3DRD_DWC_USB3_GSNPSID);
	//printf("usb controller version 0x%x\n",v);
	/*guid. optional*/
	/*gusb2phycfg*/
	v=readl(iobase+U3DRD_DWC_USB3_GUSB2PHYCFG);

	RMWREG32(&v,GUSB2PHYCFG_PHYSEL_FIELD_OFFSET,GUSB2PHYCFG_PHYSEL_FIELD_SIZE,GUSB2PHYCFG_PHYSEL);//hs

	RMWREG32(&v,GUSB2PHYCFG_ULPIAUTORES_FIELD_OFFSET,GUSB2PHYCFG_ULPIAUTORES_FIELD_SIZE,GUSB2PHYCFG_ULPIAUTORES); //disable ulpiautoresume
	RMWREG32(&v,GUSB2PHYCFG_PHYIF_FIELD_OFFSET,GUSB2PHYCFG_PHYIF_FIELD_SIZE,GUSB2PHYCFG_PHYIF); //PHYIF 16bit
	RMWREG32(&v,GUSB2PHYCFG_USBTRDTIM_FIELD_OFFSET,GUSB2PHYCFG_USBTRDTIM_FIELD_SIZE,GUSB2PHYCFG_USBTRDTIM); //UTMI 16bit
	writel(v, iobase+U3DRD_DWC_USB3_GUSB2PHYCFG);
	/*gusb3pipectl*/
	v=readl(iobase + U3DRD_DWC_USB3_GUSB3PIPECTL);
	v |= 1<<GUSB3PIPECTL_SUSPENDENABLE_FIELD_OFFSET;
	writel(v, iobase+U3DRD_DWC_USB3_GUSB3PIPECTL);
	/*gtxfifosizn*/
	/*grxfifosiz0*/
	/*gevntadrn*/
	/*gevntsizn*/
	/*gevntcountn*/
	/*gctl*/
	v=readl(iobase+U3DRD_DWC_USB3_GCTL);
	RMWREG32(&v, GCTL_SCALEDOWN_FIELD_OFFSET, GCTL_SCALEDOWN_FIELD_SIZE, 0);//remove scale down
	RMWREG32(&v, GCTL_DSBLCLKGTNG_FIELD_OFFSET, GCTL_DSBLCLKGTNG_FIELD_SIZE, 0);//remove clk gating
	RMWREG32(&v, GCTL_DISSCRAMBLE_FIELD_OFFSET, GCTL_DISSCRAMBLE_FIELD_SIZE, 0);//DW_GCTL_DISSCRAMBLE;
	RMWREG32(&v,GCTL_PRTCAPDIR_FIELD_OFFSET,GCTL_PRTCAPDIR_FIELD_SIZE,GCTL_PRTCAPDIR);//dev MODE
	writel(v, iobase+U3DRD_DWC_USB3_GCTL);
	/*dcfg*/
	v=readl(iobase+U3DRD_DWC_USB3_DCFG);
	RMWREG32(&v,DCFG_DEVSPD_FIELD_OFFSET,DCFG_DEVSPD_FIELD_SIZE,DCFG_DEVSPD);//despd,hs
	//RMWREG32(&v,17,5,9);//nump
	writel(v, iobase+U3DRD_DWC_USB3_DCFG);
	/*guctl2*/
	v = readl(iobase+U3DRD_DWC_USB3_GUCTL2);
	v |=     (1 << GUCTL2_RST_ACTBITLATER_FIELD_OFFSET);
	writel(v,iobase+U3DRD_DWC_USB3_GUCTL2);
	/*dctl,runstop*/

	return true;
}

void dw_usb_ll_enable_irq(vaddr_t iobase)
{
	uint32_t v;
	/*devten*/
	v=0;
	RMWREG32(&v,DEVTEN_ULSTCNGEN_FIELD_OFFSET,DEVTEN_ULSTCNGEN_FIELD_SIZE,1);//usb link state change
	RMWREG32(&v,DEVTEN_CONNECTDONEEVTEN_FIELD_OFFSET,DEVTEN_CONNECTDONEEVTEN_FIELD_SIZE,1);//connect done
	RMWREG32(&v,DEVTEN_USBRSTEVTEN_FIELD_OFFSET,DEVTEN_USBRSTEVTEN_FIELD_SIZE,1);//usb reset
	RMWREG32(&v,DEVTEN_DISSCONNEVTEN_FIELD_OFFSET,DEVTEN_DISSCONNEVTEN_FIELD_SIZE,1);//usb disconnect
	writel(v, iobase+U3DRD_DWC_USB3_DEVTEN);
}

void dw_usb_ll_disable_irq(vaddr_t iobase)
{
	writel(0, iobase+U3DRD_DWC_USB3_DEVTEN);
}

void dw_usb_ll_mask_irq(vaddr_t iobase)
{
	RMWREG32(iobase+U3DRD_DWC_USB3_GEVNTSIZ, GEVNTSIZ_EVNTINTRPTMASK_FIELD_OFFSET,GEVNTSIZ_EVNTINTRPTMASK_FIELD_SIZE,1);
}

void dw_usb_ll_unmask_irq(vaddr_t iobase)
{
	RMWREG32(iobase+U3DRD_DWC_USB3_GEVNTSIZ, GEVNTSIZ_EVNTINTRPTMASK_FIELD_OFFSET,GEVNTSIZ_EVNTINTRPTMASK_FIELD_SIZE,0);
}

bool dw_usb_ll_runstop(vaddr_t iobase, bool enable)
{
	uint32_t reg,timeout=500;

	RMWREG32(iobase+U3DRD_DWC_USB3_DCTL,DCTL_RUN_STOP_FIELD_OFFSET,DCTL_RUN_STOP_FIELD_SIZE,enable);
	do {
		reg = readl(iobase+U3DRD_DWC_USB3_DSTS);
		reg &= (1<<DSTS_DEVCTRLHLT_FIELD_OFFSET);
	} while (--timeout && !(!enable ^ !reg));

	if (!timeout) {
		printf("start timeout\n");
		return false;
	}
	return true;
}

bool dw_usb_ll_set_ep_cmd(vaddr_t iobase, int cmdnum, int p_epnum,uint32_t cmdvalue,uint32_t param0,uint32_t param1, uint32_t param2)
{
	uint32_t v,saved_config=0;
	int ret;
	/*set gusb2phycfg 6,8 to 0*/
	v=readl(iobase+U3DRD_DWC_USB3_GUSB2PHYCFG);
	if (v & (1<<GUSB2PHYCFG_SUSPENDUSB20_FIELD_OFFSET)) {
		saved_config |= 1<<GUSB2PHYCFG_SUSPENDUSB20_FIELD_OFFSET;
		v &= ~(1<<GUSB2PHYCFG_SUSPENDUSB20_FIELD_OFFSET);
	}

	if (v & (1<<GUSB2PHYCFG_ENBLSLPM_FIELD_OFFSET)) {
		saved_config |= (1<<GUSB2PHYCFG_ENBLSLPM_FIELD_OFFSET);
		v &= ~(1<<GUSB2PHYCFG_ENBLSLPM_FIELD_OFFSET);
	}

	if (saved_config)
		writel(v, iobase+U3DRD_DWC_USB3_GUSB2PHYCFG);

	writel(param0, iobase+U3DRD_DWC_USB3_DEPCMDPAR0+p_epnum*0x10);
	writel(param1, iobase+U3DRD_DWC_USB3_DEPCMDPAR1+p_epnum*0x10);
	writel(param2, iobase+U3DRD_DWC_USB3_DEPCMDPAR2+p_epnum*0x10);

	v=cmdvalue;
	RMWREG32(&v, DEPCMD_CMDTYP_FIELD_OFFSET, DEPCMD_CMDTYP_FIELD_SIZE, cmdnum);// set cmdtype
	if (cmdnum != 7) //update
		RMWREG32(&v, DEPCMD_CMDACT_FIELD_OFFSET, DEPCMD_CMDACT_FIELD_SIZE, 1);// set cmdact
	writel(v, iobase+U3DRD_DWC_USB3_DEPCMD+p_epnum*0x10);
	ret=reg_poll_value(iobase+U3DRD_DWC_USB3_DEPCMD+p_epnum*0x10,DEPCMD_CMDACT_FIELD_OFFSET,DEPCMD_CMDACT_FIELD_SIZE,0,CMDACT_TIMEOUT);
	if (!ret) {
		v= readl(iobase+U3DRD_DWC_USB3_DEPCMD+p_epnum*0x10);
		ret=DW_GET_CMD_STATUS(v);
	}

	/*set gusb2phycfg 6,8 to 1*/
	if (saved_config) {
		v=readl(iobase+U3DRD_DWC_USB3_GUSB2PHYCFG);
		v |= saved_config;
		writel(v, iobase+U3DRD_DWC_USB3_GUSB2PHYCFG);
	}
	if (ret != 0) {
		printf("ep %d cmd %d status %d\n",p_epnum,cmdnum,ret);
		return false;
	}
	return true;
}

bool dw_usb_ll_set_ep_config(vaddr_t iobase, int p_epnum, uint32_t param0,uint32_t param1, uint32_t param2)
{
	/*depcmd*/
	return dw_usb_ll_set_ep_cmd(iobase,EPCMD_SET_CONFIG,p_epnum,0,param0,param1,param2);
}

bool dw_usb_ll_set_ep_xfer_config(vaddr_t iobase, int p_epnum, uint32_t param0)
{
	/*depcmd*/
	return dw_usb_ll_set_ep_cmd(iobase,EPCMD_SET_XFER,p_epnum,0,param0,0,0);
}

uint32_t dw_usb_ll_get_ep_stat(vaddr_t iobase, int p_epnum)
{
	uint32_t v;
	/*depcmd*/
	dw_usb_ll_set_ep_cmd(iobase,EPCMD_GET_STAT,p_epnum,0,0,0,0);
	v= readl(iobase+U3DRD_DWC_USB3_DEPCMDPAR2+p_epnum*0x10);
	return v;
}

u32 dw_usb_ll_get_ep_transfer_index(vaddr_t iobase, int p_epnum)
{
	u32         res_id,v;

	v=readl(iobase+U3DRD_DWC_USB3_DEPCMD+p_epnum*0x10);
	res_id =DW_GET_XFER_RES_ID(v);
	return res_id;
}

int dw_usb_ll_set_link_state(vaddr_t iobase, enum dw_link_state state)
{
	int     retries = 10000;
	u32     reg;

	while (--retries) {
		reg = readl(iobase+U3DRD_DWC_USB3_DSTS);
		if (reg & DW_DSTS_DCNRD)
			spin(5);
		else
			break;
	}

	if (retries <= 0)
		return -1;

	reg = readl(iobase+U3DRD_DWC_USB3_DCTL);
	reg &= ~DW_DCTL_ULSTCHNGREQ_MASK;

	/* set requested state */
	reg |= DW_DCTL_ULSTCHNGREQ(state);
	writel(reg, iobase+U3DRD_DWC_USB3_DCTL);
	return 0;
}

int dw_usb_ll_disconnect_evt(vaddr_t iobase)
{
	u32 reg = 0;
	reg = readl(iobase + U3DRD_DWC_USB3_DCTL);
	reg &= ~BIT_U3DRD_DWC_USB3_DCTL_INITU1ENA;
	writel(reg, iobase + U3DRD_DWC_USB3_DCTL);

	reg &= ~BIT_U3DRD_DWC_USB3_DCTL_INITU2ENA;
	writel(reg, iobase + U3DRD_DWC_USB3_DCTL);
	return 0;
}

int dw_usb_ll_wakeup(vaddr_t iobase)
{
	int         retries;
	int         ret;
	u32         reg;
	u8          link_state;
	u8          speed;

	reg = readl(iobase + U3DRD_DWC_USB3_DSTS);

	speed = DW_GET_CONNDONE_SPEED(reg);
	if ((speed == USB_SPEED_SS) ||
	        (speed == USB_SPEED_SSPLUS)) {
		return 0;
	}

	link_state = DW_DSTS_USBLNKST(reg);

	switch (link_state) {
		case DW_LINK_STATE_RX_DET:    /* in HS, means Early Suspend */
		case DW_LINK_STATE_U3:    /* in HS, means SUSPEND */
			break;
		default:
			printf(
			    "can't wakeup from %d",
			    link_state);
			return -1;
	}

	ret = dw_usb_ll_set_link_state(iobase, DW_LINK_STATE_RECOV);
	if (ret < 0) {
		printf("failed to put link in Recovery\n");
		return ret;
	}

	/* poll until Link State changes to ON */
	retries = 20000;

	while (retries--) {
		reg = readl(iobase+U3DRD_DWC_USB3_DSTS);

		/* in HS, means ON */
		if (DW_DSTS_USBLNKST(reg) == DW_LINK_STATE_U0)
			break;
	}

	if (DW_DSTS_USBLNKST(reg) != DW_LINK_STATE_U0) {
		printf("failed to send remote wakeup\n");
		return -1;
	}
	return 0;
}

bool dw_usb_ll_set_ep_stall(vaddr_t iobase, int p_epnum)
{
	/*depcmd*/
	return dw_usb_ll_set_ep_cmd(iobase,EPCMD_SET_STALL,p_epnum,0,0,0,0);
}

bool dw_usb_ll_clear_ep_stall(vaddr_t iobase, int p_epnum, uint32_t param0)
{
	/*depcmd*/
	return dw_usb_ll_set_ep_cmd(iobase,EPCMD_CLEAR_STALL,p_epnum,0,param0,0,0);
}

bool dw_usb_ll_ep_start_transfer(vaddr_t iobase, int p_epnum, uint32_t param0,uint32_t param1)
{
	return dw_usb_ll_set_ep_cmd(iobase,EPCMD_START_TRANS,p_epnum,0,param0,param1,0);
}
bool dw_usb_ll_ep_update_transfer(vaddr_t iobase, int p_epnum, int res_id)
{
	/*can send no response update*/
	return dw_usb_ll_set_ep_cmd(iobase,EPCMD_UPDATE_TRANS,p_epnum,res_id<<16,0,0,0);
}
bool dw_usb_ll_ep_end_transfer(vaddr_t iobase, int p_epnum,int res_id)
{
	uint32_t v;
	v=0;
	/*trb index*/
	v |= res_id <<DEPCMD_COMMANDPARAM_FIELD_OFFSET;
	/*forceRM*/
	v |= 1<<DEPCMD_HIPRI_FORCERM_FIELD_OFFSET;
	/*cmdioc 8*/
	v |= 1<<DEPCMD_CMDIOC_FIELD_OFFSET;
	return dw_usb_ll_set_ep_cmd(iobase,EPCMD_END_TRANS,p_epnum,v,0,0,0);
}

bool dw_usb_ll_set_ep_new_config(vaddr_t iobase, int p_epnum)
{
	return dw_usb_ll_set_ep_cmd(iobase,EPCMD_SET_NEW_CONFIG,0,0,0,0,0);
}

bool dw_usb_ll_flush_ep(vaddr_t iobase, int p_epnum)
{
	return true;
}

bool dw_usb_ll_set_ep_event_buffer(vaddr_t iobase, int p_epnum, uint32_t adr_l,uint32_t adr_h,uint16_t size)
{
	//gevntadrlo
	writel(adr_l, iobase+U3DRD_DWC_USB3_GEVNTADRLO+p_epnum*0x10);
	//gevntadrhi
	writel(adr_h, iobase+U3DRD_DWC_USB3_GEVNTADRHI+p_epnum*0x10);
	//gevntsiz
	RMWREG32(iobase+U3DRD_DWC_USB3_GEVNTSIZ+p_epnum*0x10, GEVNTSIZ_EVENTSIZ_FIELD_OFFSET,GEVNTSIZ_EVENTSIZ_FIELD_SIZE,size);
	//gevntcount
	writel(0, iobase+U3DRD_DWC_USB3_GEVNTCOUNT+p_epnum*0x10);
	return true;
}

uint16_t dw_usb_ll_get_ep_event_count(vaddr_t iobase, int p_epnum)
{
	uint16_t v=0;
	v=(readl(iobase+U3DRD_DWC_USB3_GEVNTCOUNT+p_epnum*0x10) & 0xfffc);
	return v;
}

bool dw_usb_ll_clear_ep_event(vaddr_t iobase, int p_epnum,uint16_t size)
{
	//gevntcount, clear size bytes
	writel(size, iobase+U3DRD_DWC_USB3_GEVNTCOUNT+p_epnum*0x10);
	return true;
}

bool dw_usb_ll_ep_open(vaddr_t iobase, int p_epnum)
{
	uint32_t v;
	/*dalepena*/
	v=readl(iobase+U3DRD_DWC_USB3_DALEPENA);
	v |= 1 << p_epnum;
	writel(v, iobase+U3DRD_DWC_USB3_DALEPENA);
	return true;
}

bool dw_usb_ll_ep_close(vaddr_t iobase, int p_epnum)
{
	uint32_t v;
	/*dalepena*/
	v=readl(iobase+U3DRD_DWC_USB3_DALEPENA);
	v &= ~(1 << p_epnum);
	writel(v, iobase+U3DRD_DWC_USB3_DALEPENA);
	return true;
}

bool dw_usb_ll_set_address(vaddr_t iobase, uint8_t addr)
{
	uint32_t v;

	v=readl(iobase + U3DRD_DWC_USB3_DCFG);
	RMWREG32(&v, DCFG_DEVADDR_FIELD_OFFSET, DCFG_DEVADDR_FIELD_SIZE, addr);
	writel(v, iobase+U3DRD_DWC_USB3_DCFG);
	return true;
}

int dw_usb_ll_get_cur_speed(vaddr_t iobase)
{
	int speed;
	uint32_t v=readl(iobase+U3DRD_DWC_USB3_DSTS);
	speed = DW_GET_CONNDONE_SPEED(v);
	return speed;
}

uint32_t dw_usb_ll_get_ncr_evt(vaddr_t iobase)
{
	return readl(iobase + U3DRD_NCR_INTR);
}

void dw_usb_ll_clr_ncr_evt(vaddr_t iobase)
{
	uint32_t v = readl(iobase + U3DRD_NCR_INTR);
	if (v)
		writel(v & 0x7Fu, iobase + U3DRD_NCR_INTR);
}

