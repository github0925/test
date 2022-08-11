// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2020 Semidrive Semiconductor.
 *
 * SDRV USB HOST xHCI Controller
 *
 * Author: tianbao.zhou
 */

#include <usb_porting_compile.h>

#include <stdio.h>
#include <reg.h>
#include <lib/reg.h>
#include <res.h>
#include <chip_res.h>
#include <errno.h>
#include <usb.h>
#include "dwc3.h"
#include <usb/xhci.h>
#include <clkgen_hal.h>
#include <rstgen_hal.h>

#define USB_PHY_NCR_CTRL0	0x10000

struct sdrv_xhci {
	struct xhci_hccr *hcd;
	struct dwc3 *dwc3_reg;
	paddr_t usb_phy_paddr;
	paddr_t usb_paddr;
	vaddr_t usb_phy_vaddr;
	vaddr_t usb_vaddr;
	int instance_id;
	int instance_phy_id;
};

static struct sdrv_xhci sdrv_xhci;

static int phy_semidrive_usb_init(vaddr_t phy_base)
{
	unsigned int data;

	/* use internal phy clock and reset usb phy, reset high effective */
	data = readl(phy_base + USB_PHY_NCR_CTRL0);
	data &= ~(1<<18);
	data |= (1<<0);
	writel(data, phy_base + USB_PHY_NCR_CTRL0);

	udelay(35);

	data = readl(phy_base + USB_PHY_NCR_CTRL0);
	data &= ~(1<<0);
	writel(data, phy_base + USB_PHY_NCR_CTRL0);

	return 0;
}

static int usb_clkgen_config(void)
{
	int ret = 0;
	void *ckgen_handle;
	uint32_t usb2_gate[] = {
		/* gating 51 for axi, apb, ctrl.apb, phy clock */
		RES_GATING_EN_SOC_HIS_BUS_2_USB2_XM_ACLK,
		/* gating 44 for noc main and per clock */
		RES_GATING_EN_SOC_HIS_BUS_2_NOC_HIS_MAINCLK,
		/* gating 53 for ref alt clk p clock */
		//0x809a0135
	};
	unsigned int i;

	if (!hal_clock_creat_handle(&ckgen_handle)) {
		dprintf(WARN, "%s: clkgen creat handle failed\n", __FUNCTION__);
		return -1;
	}

	/*enable mmc host clock*/
	for (i = 0; i < sizeof(usb2_gate) / sizeof(usb2_gate[0]); i++)
		if (!hal_clock_enable(ckgen_handle, usb2_gate[i])) {
			dprintf(WARN, "usb2 clock %d clkgen enable failed\n", i);
			ret = -1;
		}

	/*release clock handle*/
	hal_clock_release_handle(ckgen_handle);

	return ret;
}

static int usb_module_reset(uint32_t res_id)
{
	int ret;
	void *g_handle;

	ret = hal_rstgen_creat_handle(&g_handle, RES_GLOBAL_RST_SEC_RST_EN);
	if (!ret) {
		return -1;
	}

	/*get handle ok and enable rstgen is true*/
	ret = hal_rstgen_init(g_handle);
	if (ret) {
		ret = hal_rstgen_module_reset(g_handle, res_id);
	}

	hal_rstgen_release_handle(g_handle);

	return ret ? 0 : -1;
}

static int sdrv_xhci_core_init(struct sdrv_xhci *sdrv_xhci)
{
	int ret = 0;
	u32 reg;

	ret = dwc3_core_init(sdrv_xhci->dwc3_reg);
	if (ret) {
		debug("%s:failed to initialize core\n", __func__);
		return ret;
	}

	/* Set dwc3 usb2 phy config */
	reg = readl(&sdrv_xhci->dwc3_reg->g_usb2phycfg[0]);

	/* set phy_type utmi_wide */
	reg |= DWC3_GUSB2PHYCFG_PHYIF;
	reg &= ~DWC3_GUSB2PHYCFG_USBTRDTIM_MASK;
	reg |= DWC3_GUSB2PHYCFG_USBTRDTIM_16BIT;

	writel(reg, &sdrv_xhci->dwc3_reg->g_usb2phycfg[0]);

	/* We are hard-coding DWC3 core to Host Mode */
	dwc3_set_mode(sdrv_xhci->dwc3_reg, DWC3_GCTL_PRTCAP_HOST);

	/* Set GFLADJ_30MHZ as 20h as per XHCI spec default value */
	dwc3_set_fladj(sdrv_xhci->dwc3_reg, GFLADJ_30MHZ_DEFAULT);

	return ret;
}

int board_usb_init(int index, enum usb_init_type init)
{
	int ret;

	ret = usb_clkgen_config();
	if (ret < 0)
		return ret;

	ret = usb_module_reset(RES_MODULE_RST_SEC_USB2);

	return ret;
}

int xhci_hcd_init(int index, struct xhci_hccr **hccr, struct xhci_hcor **hcor)
{
	struct sdrv_xhci *ctx = &sdrv_xhci;
	int ret = 0;

	ret = res_get_info_by_id(RES_USB_USB2, &ctx->usb_paddr,
							&ctx->instance_id);
	if (ret < 0) {
		dprintf(CRITICAL, "get USB2 base ret=%d\n", ret);
		return ret;
	}

	ret = res_get_info_by_id(RES_USBPHY_USBPHY2, &ctx->usb_phy_paddr,
							&ctx->instance_phy_id);
	if (ret < 0) {
		dprintf(CRITICAL, "get USBPHY2 base ret=%d\n", ret);
		return ret;
	}

	ctx->usb_vaddr  = (vaddr_t)_ioaddr(ctx->usb_paddr);
	ctx->usb_phy_vaddr = (vaddr_t)_ioaddr(ctx->usb_phy_paddr);

	ctx->hcd = (struct xhci_hccr *)ctx->usb_vaddr;
	ctx->dwc3_reg = (struct dwc3 *)((void *)(ctx->hcd) + DWC3_REG_OFFSET);

	ret = board_usb_init(index, USB_INIT_HOST);
	if (ret != 0) {
		dprintf(CRITICAL, "Failed to initialize board for USB\n");
		return ret;
	}

	phy_semidrive_usb_init(ctx->usb_phy_vaddr);

	ret = sdrv_xhci_core_init(ctx);
	if (ret < 0) {
		dprintf(CRITICAL, "Failed to initialize xhci\n");
		return ret;
	}

	*hccr = (struct xhci_hccr *)ctx->hcd;
	*hcor = (struct xhci_hcor *)((uintptr_t) *hccr
				+ HC_LENGTH(xhci_readl(&(*hccr)->cr_capbase)));

	debug("sdrv-xhci: init hccr %lx and hcor %lx hc_length %lx\n",
			(uintptr_t)*hccr, (uintptr_t)*hcor,
			(uintptr_t)HC_LENGTH(xhci_readl(&(*hccr)->cr_capbase)));

	return ret;
}

void xhci_hcd_stop(int index)
{
	return;
}

