/*
 * dw_pcie.c
 *
 * Copyright (c) 2019 Semidrive Semiconductor.
 * All rights reserved.
 *
 * pcie driver c file
 * TBD
 *
 * Revision History:
 * -----------------
 *
 */

#include <sys/types.h>
#include <platform/debug.h>
#include <trace.h>
#include <reg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <platform.h>
#include <kernel/thread.h>
#include "__regs_base.h"
#include "target_res.h"
#include "dw_pcie.h"
#include "clkgen_hal.h"
#include "rstgen_hal.h"

#include "hal_port.h"
#include "hal_dio.h"
#include "res.h"
#include "chip_res.h"

#define lower_32_bits(n) ((u32)(n))
#define upper_32_bits(n) ((u32)(((n) >> 16) >> 16))

#define wr(a,v) writel(v,a)
#define rd(a,p) *p=readl(a)

#define PCIE_ATU_BUS(x)         (((x) & 0xff) << 24)
#define PCIE_ATU_DEV(x)         (((x) & 0x1f) << 19)
#define PCIE_ATU_FUNC(x)        (((x) & 0x7) << 16)

#define PCIE_GET_OB_ATU_REG(region) ((region) << 9)
#define PCIE_GET_IB_ATU_REG(region) (((region) << 9) | 1 << 8)
#define PCIE_IBATU_SET_BAR_NUMBER(index) (index << 8)

#if 0
/*PCIE2*/
const Port_PinType PIN_GPIO_D13_M5_PCIE2_CLKREQ_N = PortConf_PIN_GPIO_D13;
const Port_PinModeType MODE_GPIO_D13_M5_PCIE2_CLKREQ_N = {
    ((uint32_t)PORT_PAD_POE__DISABLE | PORT_PAD_IS__OUT | PORT_PAD_SR__FAST | PORT_PAD_DS__MID1 | PORT_PIN_IN_NO_PULL ),
    ((uint32_t)PORT_PIN_MUX_FV__MIN | PORT_PIN_MUX_FIN__MIN  | PORT_PIN_OUT_OPENDRAIN | PORT_PIN_MODE_ALT5),
};

const Port_PinType PIN_EMMC2_M5_PCIE2_WAKE_N = PortConf_PIN_EMMC2_DATA1;
const Port_PinModeType MODE_EMMC2_M5_PCIE2_WAKE_N = {
    ((uint32_t)PORT_PAD_MMC_SP__MIN | PORT_PAD_MMC_SN__MIN | PORT_PAD_MMC_RXSEL__MIN | PORT_PAD_MMC_TXPREP__MIN | PORT_PAD_MMC_TXPREN__MIN | PORT_PIN_IN_NO_PULL ),
    ((uint32_t)PORT_PIN_MUX_FV__MIN | PORT_PIN_MUX_FIN__MIN  | PORT_PIN_OUT_PUSHPULL | PORT_PIN_MODE_ALT5),
};

const Port_PinType PIN_EMMC2_M5_PCIE2_PERST_N = PortConf_PIN_EMMC2_DATA0;
const Port_PinModeType MODE_EMMC2_M5_PCIE2_PERST_N = {
    ((uint32_t)PORT_PAD_MMC_SP__MIN | PORT_PAD_MMC_SN__MIN | PORT_PAD_MMC_RXSEL__MIN | PORT_PAD_MMC_TXPREP__MIN | PORT_PAD_MMC_TXPREN__MIN | PORT_PIN_IN_NO_PULL ),
    ((uint32_t)PORT_PIN_MUX_FV__MIN | PORT_PIN_MUX_FIN__MIN  | PORT_PIN_OUT_PUSHPULL | PORT_PIN_MODE_ALT5),
};

/*PCIE1*/
const Port_PinType PIN_GPIO_D12_M5_PCIE1_CLKREQ_N = PortConf_PIN_GPIO_D12;
const Port_PinModeType MODE_GPIO_D12_M5_PCIE1_CLKREQ_N = {
    ((uint32_t)PORT_PAD_POE__DISABLE | PORT_PAD_IS__OUT | PORT_PAD_SR__FAST | PORT_PAD_DS__MID1 | PORT_PIN_IN_NO_PULL ),
    ((uint32_t)PORT_PIN_MUX_FV__MIN | PORT_PIN_MUX_FIN__MIN  | PORT_PIN_OUT_OPENDRAIN | PORT_PIN_MODE_ALT5),
};

const Port_PinType PIN_EMMC2_M5_PCIE1_WAKE_N = PortConf_PIN_EMMC2_DATA3;
const Port_PinModeType MODE_EMMC2_M5_PCIE1_WAKE_N = {
    ((uint32_t)PORT_PAD_POE__DISABLE | PORT_PAD_IS__OUT | PORT_PAD_SR__FAST | PORT_PAD_DS__MID1 | PORT_PIN_IN_NO_PULL ),
    ((uint32_t)PORT_PIN_MUX_FV__MIN | PORT_PIN_MUX_FIN__MIN  | PORT_PIN_OUT_PUSHPULL | PORT_PIN_MODE_ALT5),
};

const Port_PinType PIN_EMMC2_M5_PCIE1_PERST_N = PortConf_PIN_EMMC2_DATA2;
const Port_PinModeType MODE_EMMC2_M5_PCIE1_PERST_N = {
    ((uint32_t)PORT_PAD_POE__DISABLE | PORT_PAD_IS__OUT | PORT_PAD_SR__FAST | PORT_PAD_DS__MID1 | PORT_PIN_IN_NO_PULL ),
    ((uint32_t)PORT_PIN_MUX_FV__MIN | PORT_PIN_MUX_FIN__MIN  | PORT_PIN_OUT_PUSHPULL | PORT_PIN_MODE_ALT5),
};

extern const domain_res_t g_iomuxc_res;

void pcie2_port_setup(void)
{
    static void *pcie2_port_handle;
    bool ioret;

    ioret = hal_port_creat_handle(&pcie2_port_handle, g_iomuxc_res.res_id[0]);
    printf("%s: create handle: %d\n", __func__, ioret);

    ioret = hal_port_set_pin_mode(pcie2_port_handle, PIN_GPIO_D13_M5_PCIE2_CLKREQ_N,
                                  MODE_GPIO_D13_M5_PCIE2_CLKREQ_N);

    ioret = hal_port_set_pin_mode(pcie2_port_handle, PIN_EMMC2_M5_PCIE2_WAKE_N,
                                  MODE_EMMC2_M5_PCIE2_WAKE_N);

    ioret = hal_port_set_pin_mode(pcie2_port_handle, PIN_EMMC2_M5_PCIE2_PERST_N,
                                  MODE_EMMC2_M5_PCIE2_PERST_N);

    hal_port_release_handle(&pcie2_port_handle);
}

void pcie1_port_setup(void)
{
    static void *pcie1_port_handle;
    bool ioret;

    ioret = hal_port_creat_handle(&pcie1_port_handle, g_iomuxc_res.res_id[0]);
    printf("%s: create handle: %d\n", __func__, ioret);

    ioret = hal_port_set_pin_mode(pcie1_port_handle, PIN_GPIO_D12_M5_PCIE1_CLKREQ_N,
                                  MODE_GPIO_D12_M5_PCIE1_CLKREQ_N);

    ioret = hal_port_set_pin_mode(pcie1_port_handle, PIN_EMMC2_M5_PCIE1_WAKE_N,
                                  MODE_EMMC2_M5_PCIE1_WAKE_N);

    ioret = hal_port_set_pin_mode(pcie1_port_handle, PIN_EMMC2_M5_PCIE1_PERST_N,
                                  MODE_EMMC2_M5_PCIE1_PERST_N);

    hal_port_release_handle(&pcie1_port_handle);
}
#endif

inline void kunlun_phy_ncr_writel(struct kunlun_pcie *kunlun_pcie,
                                  u32 val, u32 reg)
{
    writel(val, p2v(kunlun_pcie->phy_ncr_base + reg));
}

static inline void kunlun_phy_ncr_writel_noprint(struct kunlun_pcie
        *kunlun_pcie,
        u32 val, u32 reg)
{
    writel(val, p2v(kunlun_pcie->phy_ncr_base + reg));
}

inline u32 kunlun_phy_ncr_readl(struct kunlun_pcie *kunlun_pcie, u32 reg)
{
    return readl(p2v(kunlun_pcie->phy_ncr_base + reg));
}

inline void kunlun_ctrl_ncr_writel(struct kunlun_pcie *kunlun_pcie,
                                   u32 val, u32 reg)
{
    writel(val, p2v(kunlun_pcie->ctrl_ncr_base + reg));
}

inline u32 kunlun_ctrl_ncr_readl(struct kunlun_pcie *kunlun_pcie, u32 reg)
{
    return readl(p2v(kunlun_pcie->ctrl_ncr_base + reg));
}

static inline void kunlun_dbi_writel(struct kunlun_pcie *kunlun_pcie,
                                     u32 val, u32 reg)
{
    writel(val, p2v(kunlun_pcie->dbi + reg));
}

static inline u32 kunlun_dbi_readl(struct kunlun_pcie *kunlun_pcie, u32 reg)
{
    return readl(p2v(kunlun_pcie->dbi + reg));
}

static inline void kunlun_dbi2_writel(struct kunlun_pcie *kunlun_pcie,
                                      u32 val, u32 reg)
{
    writel(val, p2v(kunlun_pcie->dbi + PCIE_EP_TYPE0_HDR_DBI2 + reg));
}

void kunlun_pcie_phy_refclk_sel(struct kunlun_pcie *kunlun_pcie)
{
    u32 reg_val;

    u32 flag = kunlun_pcie->phy_refclk_sel & 0x3;
    bool diffbuf_out_en =  !!(kunlun_pcie->phy_refclk_sel >> 31);

    if (flag == PHY_REFCLK_USE_INTERNAL) {
        printf("using internel ref clk\n");
        reg_val = kunlun_phy_ncr_readl(kunlun_pcie, PCIE_PHY_NCR_CTRL1);
        reg_val &= ~PHY_REF_USE_PAD_BIT;
        reg_val &= ~PHY_REF_ALT_CLK_SEL_BIT;
        kunlun_phy_ncr_writel(kunlun_pcie, reg_val, PCIE_PHY_NCR_CTRL1);
    }
    else if (flag == PHY_REFCLK_USE_EXTERNAL) {
        printf("using externel ref clk\n");
        reg_val = kunlun_phy_ncr_readl(kunlun_pcie, PCIE_PHY_NCR_CTRL1);
        reg_val |= PHY_REF_USE_PAD_BIT;
        kunlun_phy_ncr_writel(kunlun_pcie, reg_val, PCIE_PHY_NCR_CTRL1);
    }
    else {
        printf("error phy_refclk_sel\n");
        ASSERT(0);
    }

    if (diffbuf_out_en) {
        printf("diffbuf out enable\n");
        reg_val = kunlun_phy_ncr_readl(kunlun_pcie, PCIE_PHY_NCR_CTRL15);
        reg_val |= 0x1;
        kunlun_phy_ncr_writel(kunlun_pcie, reg_val, PCIE_PHY_NCR_CTRL15);
    }
}

void kunlun_pcie_phy_init(struct kunlun_pcie *kunlun_pcie)
{
    u32 reg_val;

    kunlun_pcie_phy_refclk_sel(kunlun_pcie);

    reg_val = kunlun_phy_ncr_readl(kunlun_pcie, PCIE_PHY_NCR_CTRL0);
    reg_val |= CR_CKEN_BIT;
    kunlun_phy_ncr_writel(kunlun_pcie, reg_val, PCIE_PHY_NCR_CTRL0);

    reg_val = kunlun_phy_ncr_readl(kunlun_pcie, PCIE_PHY_NCR_CTRL4);
    reg_val &= ~CR_ADDR_MODE_BIT;
    kunlun_phy_ncr_writel(kunlun_pcie, reg_val, PCIE_PHY_NCR_CTRL4);

    reg_val = kunlun_phy_ncr_readl(kunlun_pcie, PCIE_PHY_NCR_CTRL0);
    reg_val |= BIF_EN_BIT;
    kunlun_phy_ncr_writel(kunlun_pcie, reg_val, PCIE_PHY_NCR_CTRL0);

    reg_val = kunlun_phy_ncr_readl(kunlun_pcie, PCIE_PHY_NCR_CTRL0);
    reg_val &= ~PHY_RESET_BIT;
    kunlun_phy_ncr_writel(kunlun_pcie, reg_val, PCIE_PHY_NCR_CTRL0);

    thread_sleep(1);
}

s32 find_capability(struct kunlun_pcie *kunlun_pcie, u32 cap)
{
    u32 val, id;
    s32 pos;

    val = kunlun_dbi_readl(kunlun_pcie, PCIE_CAP_PTR_OFFSET);
    pos = val & 0xff;

    while (pos) {
        val =  kunlun_dbi_readl(kunlun_pcie, pos);
        id = val & 0xff;

        if (id == cap)
            return pos;

        pos = (val >> 8) & 0xff;
    }

    return 0;
}

static int kunlun_pcie_link_up(struct kunlun_pcie *kunlun_pcie)
{
    u32 reg_val = kunlun_ctrl_ncr_readl(kunlun_pcie, PCIE_CTRL_NCR_INTR0);

    if ((reg_val & INTR_SMLH_LINK_UP) != INTR_SMLH_LINK_UP)
        return 0;

    if ((reg_val & INTR_RDLH_LINK_UP) != INTR_RDLH_LINK_UP)
        return 0;

    return 1;
}

void printf_link_state(struct kunlun_pcie *kunlun_pcie)
{
    u32 reg_val;
    reg_val = kunlun_ctrl_ncr_readl(kunlun_pcie, PCIE_CTRL_NCR_STS0);
    reg_val = (reg_val >> SMLH_LTSSM_STATE_SHIFT) & SMLH_LTSSM_STATE_MASK;
    printf("LTSSM STATE = 0x%x\n", reg_val);

    reg_val = kunlun_dbi_readl(kunlun_pcie,
                               kunlun_pcie->pcie_cap + PCIE_LINK_CONTROL_LINK_STATUS_REG);
    reg_val = (reg_val & PCIE_CAP_LINK_SPEED_MASK) >> PCIE_CAP_LINK_SPEED_SHIFT;
    printf("Link speed = 0x%x\n", reg_val);
}

static int kunlun_pcie_establish_link(struct kunlun_pcie *kunlun_pcie)
{
    u32 reg_val;
    s32 count = 0;
    u32 i = 0;

    if (kunlun_pcie_link_up(kunlun_pcie))
        return 0;

    reg_val = kunlun_ctrl_ncr_readl(kunlun_pcie, PCIE_CTRL_NCR_CTRL0);
    reg_val |= APP_LTSSM_EN_BIT;
    kunlun_ctrl_ncr_writel(kunlun_pcie, reg_val, PCIE_CTRL_NCR_CTRL0);

    while (!kunlun_pcie_link_up(kunlun_pcie)) {
        thread_sleep(1000);
        count++;

        for (i = 0; i < 5; i++)
            printf_link_state(kunlun_pcie);

        if (count == 5) {
            printf("Link Fail\n");
            return -EINVAL;
        }
    }

    printf("Link success\n");

    return 0;
}

int kunlun_pcie_clk_enable(struct kunlun_pcie *kunlun_pcie)
{
    s32 ret = 0;

    static void *ckgen_handle;

    u32 pcie_pclk, pcie_aclk;

    ret = hal_clock_creat_handle(&ckgen_handle);

    if (!ret) {
        printf("hal_clock_creat_handle failed\n");
        ASSERT(0);
        return -1;
    }

    ret = hal_clock_enable(ckgen_handle, RES_GATING_EN_SOC_HIS_BUS_1);

    if (!ret) {
        printf("enable pice phy ref clk failed\n");
        ASSERT(0);
        return -1;
    }

    if (kunlun_pcie->pcie_index == PCIE1) {
        pcie_pclk = RES_GATING_EN_SOC_HIS_BUS_3_PCIE1_PCLK;
        pcie_aclk = RES_GATING_EN_SOC_HIS_BUS_2_PCIE1_MSTR_ACLK;
    }
    else if (kunlun_pcie->pcie_index == PCIE2) {
        pcie_pclk = RES_GATING_EN_SOC_HIS_BUS_3_PCIE2_PLK;
        pcie_aclk = RES_GATING_EN_SOC_HIS_BUS_2_PCIE2_MSTR_ACLK;
    }
    else {
        printf("error pcie index\n");
        ASSERT(0);
        return -1;
    }

    ret = hal_clock_enable(ckgen_handle, pcie_pclk);

    if (!ret) {
        printf("enable pice core pclk failed\n");
        ASSERT(0);
        return -1;
    }

    ret = hal_clock_enable(ckgen_handle, pcie_aclk);

    if (!ret) {
        printf("enable pice core aclk failed\n");
        ASSERT(0);
        return -1;
    }

    ret = hal_clock_enable(ckgen_handle, RES_GATING_EN_SOC_HIS_BUS_3_PCIE_PHY_PCLK);

    if (!ret) {
        printf("enable pice phy pclk failed\n");
        ASSERT(0);
        return -1;
    }

    hal_clock_release_handle(ckgen_handle);
    return 0;
}

static void kunlun_pcie_core_init_rc(struct kunlun_pcie *kunlun_pcie)
{
    u32 reg_val, offset;

    kunlun_ctrl_ncr_writel(kunlun_pcie, 0, PCIE_CTRL_NCR_INTEN0);
    kunlun_ctrl_ncr_writel(kunlun_pcie, 0, PCIE_CTRL_NCR_INTEN1);
    kunlun_ctrl_ncr_writel(kunlun_pcie, 0, PCIE_CTRL_NCR_INTEN2);
    kunlun_ctrl_ncr_writel(kunlun_pcie, 0, PCIE_CTRL_NCR_INTEN3);
    kunlun_ctrl_ncr_writel(kunlun_pcie, 0, PCIE_CTRL_NCR_INTEN4);
    kunlun_ctrl_ncr_writel(kunlun_pcie, 0, PCIE_CTRL_NCR_INTEN5);
    kunlun_ctrl_ncr_writel(kunlun_pcie, 0, PCIE_CTRL_NCR_INTEN6);
    kunlun_ctrl_ncr_writel(kunlun_pcie, 0, PCIE_CTRL_NCR_INTEN7);
    kunlun_ctrl_ncr_writel(kunlun_pcie, 0, PCIE_CTRL_NCR_INTEN8);
    kunlun_ctrl_ncr_writel(kunlun_pcie, 0, PCIE_CTRL_NCR_INTEN9);
    kunlun_ctrl_ncr_writel(kunlun_pcie, 0, PCIE_CTRL_NCR_INTEN10);
    kunlun_ctrl_ncr_writel(kunlun_pcie, 0, PCIE_CTRL_NCR_INTEN11);
    kunlun_ctrl_ncr_writel(kunlun_pcie, 0, PCIE_CTRL_NCR_INTEN12);

    reg_val = kunlun_ctrl_ncr_readl(kunlun_pcie, PCIE_CTRL_NCR_CTRL0);
    reg_val &= ~APP_HOLD_PHY_RST_BIT;
    kunlun_ctrl_ncr_writel(kunlun_pcie, reg_val, PCIE_CTRL_NCR_CTRL0);

    reg_val = kunlun_ctrl_ncr_readl(kunlun_pcie, PCIE_CTRL_NCR_CTRL0);
    reg_val |= DEVICE_TYPE_BIT;
    kunlun_ctrl_ncr_writel(kunlun_pcie, reg_val, PCIE_CTRL_NCR_CTRL0);

    if (kunlun_pcie->pcie_index == PCIE2) {
        reg_val = kunlun_ctrl_ncr_readl(kunlun_pcie, PCIE_CTRL_NCR_CTRL2);
        reg_val &= ~0x3F;
        reg_val |= 0x1c;
        kunlun_ctrl_ncr_writel(kunlun_pcie, reg_val, PCIE_CTRL_NCR_CTRL2);
    }

    kunlun_ctrl_ncr_writel(kunlun_pcie, 0, PCIE_CTRL_NCR_CTRL21);
    kunlun_ctrl_ncr_writel(kunlun_pcie, 0, PCIE_CTRL_NCR_CTRL23);

    reg_val = kunlun_dbi_readl(kunlun_pcie, PCIE_PORT_LINK_CONTROL);
    reg_val &= ~PORT_LINK_MODE_MASK;
    reg_val |= PORT_LINK_MODE_1_LANES;
    kunlun_dbi_writel(kunlun_pcie, reg_val, PCIE_PORT_LINK_CONTROL);

    reg_val = kunlun_dbi_readl(kunlun_pcie, PCIE_LINK_WIDTH_SPEED_CONTROL);
    reg_val &= ~PORT_LOGIC_LINK_WIDTH_MASK;
    reg_val |= PORT_LOGIC_LINK_WIDTH_1_LANES;
    kunlun_dbi_writel(kunlun_pcie, reg_val, PCIE_LINK_WIDTH_SPEED_CONTROL);

    offset = find_capability(kunlun_pcie, PCI_CAP_ID_EXP);

    if (!offset) {
        printf("%s : didn't find link capabilities!\n", __FUNCTION__);
        ASSERT(0);
    }

    kunlun_pcie->pcie_cap = offset;

    reg_val = kunlun_dbi_readl(kunlun_pcie,
                               offset + PCIE_LINK_CONTROL2_LINK_STATUS2_REG);
    reg_val &= ~(PCIE_CAP_TARGET_LINK_SPEED_MASK);
    reg_val |= PCIE_CAP_TARGET_LINK_SPEED_GEN3;
    kunlun_dbi_writel(kunlun_pcie, reg_val,
                      offset + PCIE_LINK_CONTROL2_LINK_STATUS2_REG);

    reg_val = kunlun_dbi_readl(kunlun_pcie, PCIE_LINK_CAPABILITIES_REG + offset);
    reg_val &= ~PCIE_CAP_MAX_LINK_WIDTH_MASK;
    reg_val |= PCIE_CAP_MAX_LINK_WIDTH_1_LANES;
    kunlun_dbi_writel(kunlun_pcie, reg_val, PCIE_LINK_CAPABILITIES_REG + offset);

    reg_val = kunlun_dbi_readl(kunlun_pcie, PCIE_TYPE1_STATUS_COMMAND);
    reg_val &= 0xffff0000;
    reg_val |= PCI_COMMAND_IO | PCI_COMMAND_MEMORY | PCI_COMMAND_MASTER;
    kunlun_dbi_writel(kunlun_pcie, reg_val, PCIE_TYPE1_STATUS_COMMAND);
}

static void kunlun_pcie_core_init_ep(struct kunlun_pcie *kunlun_pcie)
{
    u32 reg_val;
    u32 offset;

    kunlun_ctrl_ncr_writel(kunlun_pcie, 0, PCIE_CTRL_NCR_INTEN0);
    kunlun_ctrl_ncr_writel(kunlun_pcie, 0, PCIE_CTRL_NCR_INTEN1);
    kunlun_ctrl_ncr_writel(kunlun_pcie, 0, PCIE_CTRL_NCR_INTEN2);
    kunlun_ctrl_ncr_writel(kunlun_pcie, 0, PCIE_CTRL_NCR_INTEN3);
    kunlun_ctrl_ncr_writel(kunlun_pcie, 0, PCIE_CTRL_NCR_INTEN4);
    kunlun_ctrl_ncr_writel(kunlun_pcie, 0, PCIE_CTRL_NCR_INTEN5);
    kunlun_ctrl_ncr_writel(kunlun_pcie, 0, PCIE_CTRL_NCR_INTEN6);
    kunlun_ctrl_ncr_writel(kunlun_pcie, 0, PCIE_CTRL_NCR_INTEN7);
    kunlun_ctrl_ncr_writel(kunlun_pcie, 0, PCIE_CTRL_NCR_INTEN8);
    kunlun_ctrl_ncr_writel(kunlun_pcie, 0, PCIE_CTRL_NCR_INTEN9);
    kunlun_ctrl_ncr_writel(kunlun_pcie, 0, PCIE_CTRL_NCR_INTEN10);
    kunlun_ctrl_ncr_writel(kunlun_pcie, 0, PCIE_CTRL_NCR_INTEN11);
    kunlun_ctrl_ncr_writel(kunlun_pcie, 0, PCIE_CTRL_NCR_INTEN12);

    reg_val = kunlun_ctrl_ncr_readl(kunlun_pcie, PCIE_CTRL_NCR_CTRL0);
    reg_val &= ~APP_HOLD_PHY_RST_BIT;
    kunlun_ctrl_ncr_writel(kunlun_pcie, reg_val, PCIE_CTRL_NCR_CTRL0);

    reg_val = kunlun_ctrl_ncr_readl(kunlun_pcie, PCIE_CTRL_NCR_CTRL0);
    reg_val &= ~DEVICE_TYPE_BIT;
    kunlun_ctrl_ncr_writel(kunlun_pcie, reg_val, PCIE_CTRL_NCR_CTRL0);

    //clear tlpprex, defalult enable.
    kunlun_ctrl_ncr_writel(kunlun_pcie, 0, PCIE_CTRL_NCR_CTRL21);
    kunlun_ctrl_ncr_writel(kunlun_pcie, 0, PCIE_CTRL_NCR_CTRL23);

    reg_val = kunlun_dbi_readl(kunlun_pcie, PCIE_PORT_LINK_CONTROL);
    reg_val &= ~PORT_LINK_MODE_MASK;
    reg_val |= PORT_LINK_MODE_1_LANES;
    kunlun_dbi_writel(kunlun_pcie, reg_val, PCIE_PORT_LINK_CONTROL);

    reg_val = kunlun_dbi_readl(kunlun_pcie, PCIE_LINK_WIDTH_SPEED_CONTROL);
    reg_val &= ~PORT_LOGIC_LINK_WIDTH_MASK;
    reg_val |= PORT_LOGIC_LINK_WIDTH_2_LANES;
    kunlun_dbi_writel(kunlun_pcie, reg_val, PCIE_LINK_WIDTH_SPEED_CONTROL);

    offset = find_capability(kunlun_pcie, PCI_CAP_ID_EXP);

    if (!offset) {
        printf("%s : didn't find link capabilities!\n", __FUNCTION__);
        ASSERT(0);
    }

    reg_val = kunlun_dbi_readl(kunlun_pcie,
                               offset + PCIE_LINK_CONTROL2_LINK_STATUS2_REG);
    reg_val &= ~(PCIE_CAP_TARGET_LINK_SPEED_MASK);
    reg_val |= PCIE_CAP_TARGET_LINK_SPEED_GEN3;
    kunlun_dbi_writel(kunlun_pcie, reg_val,
                      offset + PCIE_LINK_CONTROL2_LINK_STATUS2_REG);

    reg_val = kunlun_dbi_readl(kunlun_pcie, PCIE_LINK_CAPABILITIES_REG + offset);
    reg_val &= ~PCIE_CAP_MAX_LINK_WIDTH_MASK;
    reg_val |= PCIE_CAP_MAX_LINK_WIDTH_1_LANES;
    kunlun_dbi_writel(kunlun_pcie, reg_val, PCIE_LINK_CAPABILITIES_REG + offset);
}

static inline void kunlun_atu_ob_writel(struct kunlun_pcie *kunlun_pcie,
                                        u32 index,
                                        u32 val, u32 reg)
{
    u32 offset = PCIE_GET_OB_ATU_REG(index);

    writel(val, p2v(kunlun_pcie->dbi + PCIE_ATU_BASE + offset + reg));
}

static inline u32 kunlun_atu_ob_readl(struct kunlun_pcie *kunlun_pcie,
                                      u32 index, u32 reg)
{
    u32 offset = PCIE_GET_OB_ATU_REG(index);

    return readl(p2v(kunlun_pcie->dbi + PCIE_ATU_BASE + offset + reg));
}

static inline void kunlun_atu_ib_writel(struct kunlun_pcie *kunlun_pcie,
                                        u32 index,
                                        u32 val, u32 reg)
{
    u32 offset = PCIE_GET_IB_ATU_REG(index);
    writel(val, p2v(kunlun_pcie->dbi + PCIE_ATU_BASE + offset + reg));
}

static inline u32 kunlun_atu_ib_readl(struct kunlun_pcie *kunlun_pcie,
                                      u32 index, u32 reg)
{
    u32 offset = PCIE_GET_IB_ATU_REG(index);

    return readl(p2v(kunlun_pcie->dbi + PCIE_ATU_BASE + offset + reg ));
}

void kunlun_pcie_config_ob_atu_cfg(struct kunlun_pcie *kunlun_pcie, u32 index,
                                   u32 bus, u32 devnum, u32 func)
{
    u32 target_addr = PCIE_ATU_BUS(bus) | PCIE_ATU_DEV(devnum) | PCIE_ATU_FUNC(
                          func);
    u32 reg_val;

    u64 limit_addr = kunlun_pcie->ecam_base + 0x1000 - 1;

    kunlun_atu_ob_writel(kunlun_pcie, index, lower_32_bits(kunlun_pcie->ecam_base),
                         PCIE_ATU_LWR_BASE_ADDR);
    kunlun_atu_ob_writel(kunlun_pcie, index, upper_32_bits(kunlun_pcie->ecam_base),
                         PCIE_ATU_UPPER_BASE_ADDR);
    kunlun_atu_ob_writel(kunlun_pcie, index, lower_32_bits(limit_addr),
                         PCIE_ATU_LIMIT_ADDR);

    kunlun_atu_ob_writel(kunlun_pcie, index, lower_32_bits(target_addr),
                         PCIE_ATU_LWR_TARGET_ADDR);
    kunlun_atu_ob_writel(kunlun_pcie, index, upper_32_bits(target_addr),
                         PCIE_ATU_UPPER_TARGET_ADDR);

    reg_val = kunlun_atu_ob_readl(kunlun_pcie, index, PCIE_ATU_REGION_CTRL_1);
    reg_val &= ~PCIE_OBATU_TYPE_BIT;
    reg_val |= PCIE_OBATU_TYPE_CFG0;
    kunlun_atu_ob_writel(kunlun_pcie, index, reg_val, PCIE_ATU_REGION_CTRL_1);

    reg_val = kunlun_atu_ob_readl(kunlun_pcie, index, PCIE_ATU_REGION_CTRL_2);
    reg_val |= PCIE_OBATU_ENABLE_BIT;
    kunlun_atu_ob_writel(kunlun_pcie, index, reg_val, PCIE_ATU_REGION_CTRL_2);
}

void kunlun_pcie_config_ob_atu_mem(struct kunlun_pcie *kunlun_pcie, u32 index,
                                   u64 base_addr, u64 limit_addr, u64 target_addr)
{
    u64 size = limit_addr - base_addr + 1;
    u32 reg_val;

    if (size > 0xffffffff) {
        printf("Error ATU region size!!!\n");
        ASSERT(0);
    }

    kunlun_atu_ob_writel(kunlun_pcie, index, lower_32_bits(base_addr),
                         PCIE_ATU_LWR_BASE_ADDR);
    kunlun_atu_ob_writel(kunlun_pcie, index, upper_32_bits(base_addr),
                         PCIE_ATU_UPPER_BASE_ADDR);

    kunlun_atu_ob_writel(kunlun_pcie, index, lower_32_bits(limit_addr),
                         PCIE_ATU_LIMIT_ADDR);

    kunlun_atu_ob_writel(kunlun_pcie, index, lower_32_bits(target_addr),
                         PCIE_ATU_LWR_TARGET_ADDR);

    kunlun_atu_ob_writel(kunlun_pcie, index, upper_32_bits(target_addr),
                         PCIE_ATU_UPPER_TARGET_ADDR);

    reg_val = kunlun_atu_ob_readl(kunlun_pcie, index, PCIE_ATU_REGION_CTRL_1);
    reg_val &= ~PCIE_OBATU_TYPE_BIT;
    reg_val |= PCIE_OBATU_TYPE_MEM;
    kunlun_atu_ob_writel(kunlun_pcie, index, reg_val, PCIE_ATU_REGION_CTRL_1);

    reg_val = kunlun_atu_ob_readl(kunlun_pcie, index, PCIE_ATU_REGION_CTRL_2);
    reg_val |= PCIE_OBATU_ENABLE_BIT;
    kunlun_atu_ob_writel(kunlun_pcie, index, reg_val, PCIE_ATU_REGION_CTRL_2);

}

void kunlun_pcie_config_ib_atu(struct kunlun_pcie *kunlun_pcie, u32 index,
                               u64 target_addr, u32 bar_index)
{
    u32 reg_val;

    kunlun_atu_ib_writel(kunlun_pcie, index, lower_32_bits(target_addr),
                         PCIE_ATU_LWR_TARGET_ADDR);
    kunlun_atu_ib_writel(kunlun_pcie, index, upper_32_bits(target_addr),
                         PCIE_ATU_UPPER_TARGET_ADDR);

    reg_val = kunlun_atu_ib_readl(kunlun_pcie, index, PCIE_ATU_REGION_CTRL_1);
    reg_val &= ~PCIE_IBATU_TYPE_BIT;
    reg_val |= PCIE_IBATU_TYPE_MEM;
    kunlun_atu_ib_writel(kunlun_pcie, index, reg_val, PCIE_ATU_REGION_CTRL_1);

    reg_val = kunlun_atu_ib_readl(kunlun_pcie, index, PCIE_ATU_REGION_CTRL_2);
    reg_val &= ~PCIE_IBATU_BAR_NUMBER_BIT;
    reg_val |= (PCIE_IBATU_ENABLE_BIT | PCIE_IBATU_MATCH_MODE_BIT |
                PCIE_IBATU_SET_BAR_NUMBER(bar_index));
    kunlun_atu_ib_writel(kunlun_pcie, index, reg_val, PCIE_ATU_REGION_CTRL_2);
}

int reset_pcie_ss(void)
{
    bool ret = true;
    void *handle = NULL;

    ret = hal_rstgen_creat_handle(&handle, RES_GLOBAL_RST_SEC_RST_EN);
    ASSERT(ret);

    hal_rstgen_init(handle);

    ret = hal_rstgen_iso_disable(handle, RES_ISO_EN_SEC_PCIE);
    ASSERT(ret);

    ret = hal_rstgen_module_reset(handle, RES_MODULE_RST_SEC_PCIEPHY);
    ASSERT(ret);

    ret = hal_rstgen_module_reset(handle, RES_MODULE_RST_SEC_PCIE1);
    ASSERT(ret);

    ret = hal_rstgen_module_reset(handle, RES_MODULE_RST_SEC_PCIE2);
    ASSERT(ret);

    hal_rstgen_release_handle(handle);
    return 0;
}

void kunlun_pcie_init(struct kunlun_pcie *kunlun_pcie)
{
    kunlun_pcie_clk_enable(kunlun_pcie);

    reset_pcie_ss();
}



void kunlun_pcie2_rc_mode_init(struct kunlun_pcie *kunlun_pcie)
{
    u32 ret;

    kunlun_pcie->ctrl_ncr_base = PCIE2_CTRL_NCR_BASE;
    kunlun_pcie->dbi = PCIE2_DBI_BASE;
    kunlun_pcie->phy_base = PCIE_PHY_BASE;
    kunlun_pcie->phy_ncr_base = PCIE_PHY_NCR_BASE;
    kunlun_pcie->ecam_base = PCIE2_ECAM_BASE;
    kunlun_pcie->pcie_index = PCIE2;
    kunlun_pcie->phy_refclk_sel = PHY_REFCLK_USE_INTERNAL;

    kunlun_pcie_init(kunlun_pcie);

    kunlun_pcie_phy_init(kunlun_pcie);

    kunlun_pcie_core_init_rc(kunlun_pcie);

    ret = kunlun_pcie_establish_link(kunlun_pcie);

    if (ret)
        return;

    kunlun_pcie_config_ob_atu_cfg(kunlun_pcie, 0, 0, 0, 0);
    u32 val = readl(p2v(kunlun_pcie->ecam_base +
                        PCIE_TYPE0_DEVICE_ID_VENDOR_ID_REG));
    printf("B=0/D=0/F=0 vender id = 0x%x, device id = 0x%x\n", val & 0xffff,
           val >> 16);
}

void kunlun_pcie1_rc_mode_init(struct kunlun_pcie *kunlun_pcie)
{
    int ret = 0;

    kunlun_pcie->ctrl_ncr_base = PCIE1_CTRL_NCR_BASE;
    kunlun_pcie->dbi = PCIE1_DBI_BASE;
    kunlun_pcie->phy_base = PCIE_PHY_BASE;
    kunlun_pcie->phy_ncr_base = PCIE_PHY_NCR_BASE;
    kunlun_pcie->ecam_base = PCIE1_ECAM_BASE;
    kunlun_pcie->pcie_index = PCIE1;
    kunlun_pcie->phy_refclk_sel = PHY_REFCLK_USE_EXTERNAL | DIFFBUF_OUT_EN;

    kunlun_pcie_init(kunlun_pcie);

    kunlun_pcie_phy_init(kunlun_pcie);

    kunlun_pcie_core_init_rc(kunlun_pcie);

    ret = kunlun_pcie_establish_link(kunlun_pcie);

    if (ret)
        return;

    kunlun_pcie_config_ob_atu_cfg(kunlun_pcie, 0, 0, 0, 0);

    u32 val = readl(p2v(kunlun_pcie->ecam_base +
                        PCIE_TYPE0_DEVICE_ID_VENDOR_ID_REG));
    printf("B=0/D=0/F=0 vender id = 0x%x, device id = 0x%x\n", val & 0xffff,
           val >> 16);
}

void kunlun_pcie1_ep_mode_init(struct kunlun_pcie *kunlun_pcie)
{
    u32 reg_val;

    if (!kunlun_pcie) {
        printf("[Error] kunlun_pcie should not be NULL\n");
        return;
    }

    kunlun_pcie->ctrl_ncr_base = PCIE1_CTRL_NCR_BASE;
    kunlun_pcie->dbi = PCIE1_DBI_BASE;
    kunlun_pcie->phy_base = PCIE_PHY_BASE;
    kunlun_pcie->phy_ncr_base = PCIE_PHY_NCR_BASE;
    kunlun_pcie->pcie_index = PCIE1;
    kunlun_pcie->phy_refclk_sel = PHY_REFCLK_USE_INTERNAL;

    kunlun_pcie_init(kunlun_pcie);

    kunlun_pcie_phy_init(kunlun_pcie);

    kunlun_pcie_core_init_ep(kunlun_pcie);

    reg_val = kunlun_ctrl_ncr_readl(kunlun_pcie, PCIE_CTRL_NCR_CTRL0);
    reg_val |= APP_LTSSM_EN_BIT;
    kunlun_ctrl_ncr_writel(kunlun_pcie, reg_val, PCIE_CTRL_NCR_CTRL0);

    kunlun_dbi_writel(kunlun_pcie, 0x00011e8c, PCIE_TYPE0_DEVICE_ID_VENDOR_ID_REG);
    reg_val = kunlun_dbi_readl(kunlun_pcie, PCIE_TYPE0_DEVICE_ID_VENDOR_ID_REG);
    printf("set device id = 0x%x, vender id = 0x%x\n", reg_val >> 16,
           reg_val & 0xffff);

    reg_val = kunlun_dbi_readl(kunlun_pcie, PCIE_TYPE0_CLASS_CODE_REVISION_ID_REG);
    reg_val &= ~(0xffffff << 8);
    reg_val |= 0x058000 << 8;
    kunlun_dbi_writel(kunlun_pcie, reg_val, PCIE_TYPE0_CLASS_CODE_REVISION_ID_REG);
    printf("set class id = 0x0580\n");

}

void kunlun_pcie1_ep_v9_cfg(struct kunlun_pcie *kunlun_pcie)
{
    u32 reg_val;

    if (!kunlun_pcie) {
        printf("[Error] kunlun_pcie should not be NULL\n");
        return;
    }

    reg_val = kunlun_dbi_readl(kunlun_pcie, PCIE_MSI_CTRL);
    reg_val &= ~PCIE_MSI_VECTOR_MASK;
    reg_val |= PCIE_MSI_VECTOR_32;
    kunlun_dbi_writel(kunlun_pcie, reg_val,  PCIE_MSI_CTRL);

    kunlun_pcie_config_ib_atu(kunlun_pcie, 0, V9TS_IP_REG_BASE, 2);
    kunlun_dbi2_writel(kunlun_pcie, V9TS_IP_REG_SIZE - 1, PCIE_EP_BAR2_MASK);
    kunlun_pcie_config_ib_atu(kunlun_pcie, 1, V9TS_MEM_BASE, 4);
    kunlun_dbi2_writel(kunlun_pcie, V9TS_MEM_SIZE - 1, PCIE_EP_BAR4_MASK);

    kunlun_dbi2_writel(kunlun_pcie, 0x0, PCIE_EP_BAR0_MASK + 0x20000);
    kunlun_dbi2_writel(kunlun_pcie, 0x0, PCIE_EP_BAR2_MASK + 0x20000);
    kunlun_dbi2_writel(kunlun_pcie, 0x0, PCIE_EP_BAR4_MASK + 0x20000);

    u64 addr = 0;
    u32 tmp_addr = 0;

    do  {
        tmp_addr = kunlun_dbi_readl(kunlun_pcie, PCIE_MSI_UPPER_ADDR);
        addr = tmp_addr;
        tmp_addr = kunlun_dbi_readl(kunlun_pcie, PCIE_MSI_ADDR);
        addr = addr << 32;
        addr |= tmp_addr;

    }
    while (!addr);

    kunlun_pcie_config_ob_atu_mem(kunlun_pcie, 0, PCIE1_AP_PCIE_IO1_BASE,
                                  (PCIE1_AP_PCIE_IO1_BASE + 0x10000 - 1), addr);

}

unsigned int pciephy_cr16_read(struct kunlun_pcie *kunlun_pcie,
                               unsigned int addr)
{
    unsigned int addr_15_14, addr_13_0, new_addr, rdata, phycr_base;
    u32 reg_val;
    addr_15_14 = addr & 0x0000C000;
    addr_15_14 = addr_15_14 >> 14;
    addr_13_0 = addr & 0x00003FFF;
    addr_13_0 = addr_13_0 * 4;
    phycr_base = kunlun_pcie->phy_base;
    new_addr = phycr_base + addr_13_0;

    // sys_tb_ctrl_set_reg_field(REG_PCIE3PHY_NCR_APB_AB0_CTRL_4, 30, 2, addr_15_14);

    reg_val = kunlun_phy_ncr_readl(kunlun_pcie, PCIE_PHY_NCR_CTRL4);
    reg_val &= ~(0x3 << 30);
    reg_val |= (addr_15_14 << 30);
    kunlun_phy_ncr_writel_noprint(kunlun_pcie, reg_val, PCIE_PHY_NCR_CTRL4);

    rd(p2v(new_addr), &rdata);
    rdata = rdata & 0x0000FFFF;
    rd(p2v(new_addr), &rdata);
    rdata = rdata & 0x0000FFFF;

    printf("PHYCR Reading Register Addr %08X, Data %04X\n", addr, rdata);

    //printf("PHYCR Reading Register Addr 0x%x\n", addr);
    //printf("PHYCR Reading Register Data 0x%x\n", rdata);

    return rdata;
}

int pciephy_cr16_read_tst(struct kunlun_pcie *kunlun_pcie, unsigned int addr,
                          unsigned int expval /*Expect val*/, unsigned int expmsk/*Mask*/)
{
    unsigned int rdata, rdata_mask, expval_mask;

    rdata = pciephy_cr16_read(kunlun_pcie, addr);
    rdata_mask = rdata & expmsk;
    expval_mask = expval & expmsk;

    if (rdata_mask == expval_mask) {
        printf("PHYCR Reading Register Test Pass. Addr %08X, Expect Data %04X\n", addr,
               expval);
        //send_message_hex("PHYCR Reading Register, Expect data", expval, INFO);
        //send_message_hex("PHYCR Reading Register, Expect data mask", expmsk, INFO);
        //send_message_hex("PHYCR Reading Register, Actual data", rdata, INFO);
        return 1;
    }
    else {
        printf("PHYCR Reading Register Test Error. Addr %08X, Expect Data %04X, Actual Data %04X, Mask %04X\n",
               addr, expval, rdata, expmsk);
        //send_message_hex("PHYCR Reading Register, Expect data", expval, INFO);
        //send_message_hex("PHYCR Reading Register, Expect data mask", expmsk, INFO);
        //send_message_hex("PHYCR Reading Register, Actual data", rdata, INFO);
        return 0;
    }
}

void pciephy_cr16_write(struct kunlun_pcie *kunlun_pcie, unsigned int addr,
                        unsigned int wdata)
{
    printf("PCIe--%s\n", __FUNCTION__);


    unsigned int addr_15_14, addr_13_0, new_addr, phycr_base;
    u32 reg_val;

    addr_15_14 = addr & 0x0000C000;
    addr_15_14 = addr_15_14 >> 14;
    addr_13_0 = addr & 0x00003FFF;
    addr_13_0 = addr_13_0 * 4;
    phycr_base = kunlun_pcie->phy_base;
    new_addr = phycr_base + addr_13_0;

    //sys_tb_ctrl_set_reg_field(REG_PCIE3PHY_NCR_APB_AB0_CTRL_4, 30, 2, addr_15_14);

    reg_val = kunlun_phy_ncr_readl(kunlun_pcie, PCIE_PHY_NCR_CTRL4);
    reg_val &= ~(0x3 << 30);
    reg_val |= (addr_15_14 << 30);
    kunlun_phy_ncr_writel(kunlun_pcie, reg_val, PCIE_PHY_NCR_CTRL4);


    wr(p2v(new_addr), wdata);

    printf("PHYCR Writing Register Addr %08X, Data %04X\n", addr, wdata);
    //send_message_hex("PHYCR Writing Register Addr ", addr, INFO);
    //send_message_hex("PHYCR Writing Register Data ", wdata, INFO);

    pciephy_cr16_read_tst(kunlun_pcie, addr, wdata /*Expect val*/, 0xFFFF /*Mask*/);

}

void kunlun_pcie_phy_internal_loopback_test(struct kunlun_pcie *kunlun_pcie,
        unsigned int refclksel, unsigned int speed, unsigned int lbertmode,
        unsigned int lbertpat0)
{
    unsigned int i;

    // Asserting per lane PCS-RAW RX reset
    pciephy_cr16_write(kunlun_pcie, 0xa006, 0x0007);
    pciephy_cr16_write(kunlun_pcie, 0xa006, 0x0007);
    // Asserting per lane PCS-RAW TX reset
    pciephy_cr16_write(kunlun_pcie, 0xa001, 0x0f07);
    pciephy_cr16_write(kunlun_pcie, 0xa001, 0x0f07);
    // Asserting per lane PMA RX reset
    pciephy_cr16_write(kunlun_pcie, 0x900c, 0x0001);
    pciephy_cr16_write(kunlun_pcie, 0x900c, 0x0003);
    // Asserting per lane PMA TX reset
    pciephy_cr16_write(kunlun_pcie, 0x9005, 0x0001);
    pciephy_cr16_write(kunlun_pcie, 0x9005, 0x0003);
    printf("PCIE3PHY internal loopback test: speed GEN%d, refclk: %d, lbertmode: %d, lbertpat0: %d.\n",
           speed, refclksel, lbertmode, lbertpat0);

    // --------------------------------------------------------------------------------
    //  Changed By Shaohu
    //    ref_use_pad_en bit 5 = 0x1
    //    ref_use_pad_val bit 4 = 0x0
    // --------------------------------------------------------------------------------
    if (refclksel == 0x0) {
        // Overriding ref_range_r; ref_repeat_clk_en_r; ref_use_pad_r; ref_clk_div2_en_r; ref_clk_en_r
        pciephy_cr16_write(kunlun_pcie, 0x0002, 0x1301);
        // Overriding respective override enables
        pciephy_cr16_write(kunlun_pcie, 0x0002, 0x3bab);
    }
    else {
        // Overriding ref_range_r; ref_repeat_clk_en_r; ref_use_pad_r; ref_clk_div2_en_r; ref_clk_en_r
        pciephy_cr16_write(kunlun_pcie, 0x0002, 0x1311);
        // Overriding respective override enables
        pciephy_cr16_write(kunlun_pcie, 0x0002, 0x3bbb);
    }

    // Waiting from          5 microseconds for expected events to take place
    spin(10 * 5);
    // Overriding mplla/b_force_en; ref_clk_en
    pciephy_cr16_write(kunlun_pcie, 0x203b, 0x003a);
    //  Waiting from          5 microseconds for expected events to take place
    spin(10 * 5);
    // rxX_term_ctrl
    pciephy_cr16_write(kunlun_pcie, 0xa01a, 0x0002);
    pciephy_cr16_write(kunlun_pcie, 0xa01a, 0x000a);
    // txX_term_ctrl
    pciephy_cr16_write(kunlun_pcie, 0xa01a, 0x002a);
    pciephy_cr16_write(kunlun_pcie, 0xa01a, 0x00aa);
    // Overwriting mpll_skipcal_r which otherwsie is maintained to de-asserted value
    pciephy_cr16_write(kunlun_pcie, 0x0020, 0x0008);
    // Overwriting mpll_skipcal_r which otherwsie is maintained to de-asserted value
    pciephy_cr16_write(kunlun_pcie, 0x002c, 0x0008);
    // Overriding tx_vboost_lvl and rx_vref_ctrl
    pciephy_cr16_write(kunlun_pcie, 0x000f, 0x00d1);
    // Overriding tx_vboost_lvl_ovrd and rx_vref_ctrl_ovrd
    pciephy_cr16_write(kunlun_pcie, 0x000f, 0x02f1);

    if (speed == 0x3) { // GEN3
        // Overriding mplla_bandwidth
        pciephy_cr16_write(kunlun_pcie, 0x0008, 0x0000);
        // Overriding mpllb_bandwidth
        pciephy_cr16_write(kunlun_pcie, 0x000c, 0x0078);
        // De-asserting MPLLA enable and overwriting ref_clk_mplla_div2_en; mplla_div8_clk_en; mplla_div10_clk_en; mplla_div16p5_clk_en and ate_mplla_multiplier
        pciephy_cr16_write(kunlun_pcie, 0x0005, 0x0000);
        // mplla_ovrd_in_en
        pciephy_cr16_write(kunlun_pcie, 0x0005, 0x8000);
        // De-asserting MPLLB enable and overwriting ref_clk_mpllb_div2_en; mpllb_div8_clk_en; mpllb_div10_clk_en; mpllb_div16p5_clk_en and ate_mpllb_multiplier
        pciephy_cr16_write(kunlun_pcie, 0x0009, 0x150a);
        // mpllb_ovrd_in_en
        pciephy_cr16_write(kunlun_pcie, 0x0009, 0x950a);
    }
    else {   // GEN2/GEN1
        // Overriding mplla_bandwidth
        pciephy_cr16_write(kunlun_pcie, 0x0008, 0x00c5);
        // Overriding mpllb_bandwidth
        pciephy_cr16_write(kunlun_pcie, 0x000c, 0x0000);
        // De-asserting MPLLA enable and overwriting ref_clk_mplla_div2_en; mplla_div8_clk_en; mplla_div10_clk_en; mplla_div16p5_clk_en and ate_mplla_multiplier
        pciephy_cr16_write(kunlun_pcie, 0x0005, 0x1198);
        // mplla_ovrd_in_en
        pciephy_cr16_write(kunlun_pcie, 0x0005, 0x9198);
        // De-asserting MPLLB enable and overwriting ref_clk_mpllb_div2_en; mpllb_div8_clk_en; mpllb_div10_clk_en; mpllb_div16p5_clk_en and ate_mpllb_multiplier
        pciephy_cr16_write(kunlun_pcie, 0x0009, 0x0000);
        // mpllb_ovrd_in_en
        pciephy_cr16_write(kunlun_pcie, 0x0009, 0x8000);
    }

    // Overriding mplla_fracn_ctrl
    pciephy_cr16_write(kunlun_pcie, 0x0007, 0x0000);
    // Overriding mplla_ssc_freq_cnt_init
    pciephy_cr16_write(kunlun_pcie, 0x002a, 0x1000);
    // Overriding mplla_ssc_freq_cnt_peak
    pciephy_cr16_write(kunlun_pcie, 0x002b, 0x0100);
    // Overriding mplla_ssc_range; mplla_ssc_ref_clk_sel; mplla_ssc_gen_clk_sel; mplla_fracn_ctrl
    pciephy_cr16_write(kunlun_pcie, 0x0006, 0x0000);
    // Overriding mplla_ssc_en
    pciephy_cr16_write(kunlun_pcie, 0x0006, 0x0000);
    // Overriding enable for mplla_ssc_range ate_mplla_fracn_ctrl ate_mplla_ssc_clk_sel mplla_ssc_en; mplla_word_div2_en
    pciephy_cr16_write(kunlun_pcie, 0x0006, 0x0080);
    //
    pciephy_cr16_write(kunlun_pcie, 0x000b, 0x0000);
    // Overriding mpllb_ssc_freq_cnt_init
    pciephy_cr16_write(kunlun_pcie, 0x0036, 0x1000);
    // Overriding mpllb_ssc_freq_cnt_peak
    pciephy_cr16_write(kunlun_pcie, 0x0037, 0x0100);
    // Overriding mpllb_ssc_range; mpllb_ssc_ref_clk_sel; mpllb_ssc_gen_clk_sel; mpllb_fracn_ctrl
    pciephy_cr16_write(kunlun_pcie, 0x000a, 0x0000);
    // Overriding mpllb_ssc_en
    pciephy_cr16_write(kunlun_pcie, 0x000a, 0x0000);
    // Overriding enable for mpllb_ssc_range ate_mpllb_fracn_ctrl ate_mpllb_ssc_clk_sel mpllb_ssc_en
    pciephy_cr16_write(kunlun_pcie, 0x000a, 0x0080);
    // Overriding mpllb_word_div2_en
    pciephy_cr16_write(kunlun_pcie, 0x000a, 0x0080);
    // Overriding mplla_div_multiplier; mplla_div_clk_en
    pciephy_cr16_write(kunlun_pcie, 0x0003, 0x0200);


    if (speed == 0x3) { // GEN3
        // Overriding mpllb_div_multiplier; mpllb_div_clk_en
        pciephy_cr16_write(kunlun_pcie, 0x0004, 0x0211);
        // Hooking up tx0_clk to appropriate mpll word/dword/qword clock  ...Overriding tx_clk_sel
        pciephy_cr16_write(kunlun_pcie, 0xa0c1, 0x000f);
        // Overriding tx_clk_en_r
        pciephy_cr16_write(kunlun_pcie, 0xa0c1, 0x000f);
        // Overriding lane_rx2tx_par_lb_en and lane_tx2rx_ser_lb_en
        pciephy_cr16_write(kunlun_pcie, 0x9000, 0x0005);
        // Overriding tx_eq_main;tx_iboost_lvl_r; tx_beacon_en; tx_disable
        pciephy_cr16_write(kunlun_pcie, 0x9002, 0x30f8);
        // Overriding tx_eq_main_ovrd_in_en and tx_ovrd_in_1_en
        pciephy_cr16_write(kunlun_pcie, 0x9002, 0xb1f8);
        // Overriding tx_eq_pre and tx_eq_post
        pciephy_cr16_write(kunlun_pcie, 0x9003, 0x0000);
        // Overriding tx_eq_pre_en and tx_eq_post_en
        pciephy_cr16_write(kunlun_pcie, 0x9003, 0x2040);
        // tx_detrx_req; tx_mpllb_sel; tx_width[1:0]; tx_rate[2:0]; tx_pstate[1:0]; tx_lpd; tx_req; tx_data_en; tx_invert; tx_reset; tx_clk_rdy
        pciephy_cr16_write(kunlun_pcie, 0x9004, 0x0001);
        pciephy_cr16_write(kunlun_pcie, 0x9004, 0x00ab);
        pciephy_cr16_write(kunlun_pcie, 0x9001, 0x1600);
        pciephy_cr16_write(kunlun_pcie, 0x9001, 0xbf12);
        // rxX_eq_delta_iq
        pciephy_cr16_write(kunlun_pcie, 0xa019, 0x0003);
        pciephy_cr16_write(kunlun_pcie, 0xa019, 0x0013);
        // Overriding rx_eq_ctle_boost; rx_eq_vga2_gain; rx_eq_vga1_gain; rx_eq_att_lvl
        pciephy_cr16_write(kunlun_pcie, 0x900d, 0x3800);
        // Overriding rx_eq_dfe_tap1; rx_eq_ctle_pole
        pciephy_cr16_write(kunlun_pcie, 0x900e, 0x0000);
        // Overriding rx_ovrd_eq_in_en
        pciephy_cr16_write(kunlun_pcie, 0x900e, 0x8000);
        // Setting rx_ref_ld_val_r and rx_cdr_vco_lowfreq_r
        pciephy_cr16_write(kunlun_pcie, 0x9008, 0x0022);
        // Setting rx_ovrd_in_1_en
        pciephy_cr16_write(kunlun_pcie, 0x9008, 0x00a2);
        // Setting  rx_vco_ld_val
        pciephy_cr16_write(kunlun_pcie, 0x9009, 0x0550);
        // Setting rx_ovrd_in_2_en
        pciephy_cr16_write(kunlun_pcie, 0x9009, 0x2550);
        // Overriding rx_term_en; rx_los_lfps_en; rx_los_threshold; rx_disable; rx_clk_shift; rx_align_en; rx_cdr_ssc_en; rx_cdr_track_en
        pciephy_cr16_write(kunlun_pcie, 0x900a, 0x4aa8);
        pciephy_cr16_write(kunlun_pcie, 0x900b, 0x0180);
        pciephy_cr16_write(kunlun_pcie, 0x900b, 0x0380);
        // Waiting from          5 microseconds for expected events to take place
        spin(10 * 5);
        // Perform the resistor tuning
        pciephy_cr16_write(kunlun_pcie, 0x000d, 0x0018);
        pciephy_cr16_write(kunlun_pcie, 0x000d, 0x001b);
        // Waiting from        200 microseconds for expected events to take place
        spin(10 * 200);
        // Reading RTUNE_ACK
        // Check RTURN_ACK=0x1
        pciephy_cr16_read_tst(kunlun_pcie, 0x0019 /*Addr*/, 0x0100 /*Expect val*/,
                              0x0100/*Mask*/);
        // Checking RTUNE ACK to be asserted
        // Expecting RTUNE ACK to be asserted :: 1
        spin(10 * 100);
        // Done with resistor tuning and de-asserting RTUNE_REQ
        pciephy_cr16_write(kunlun_pcie, 0x000d, 0x001a);
        spin(10 * 100);
        // Check RTUNE_ACK=0x0
        pciephy_cr16_read_tst(kunlun_pcie, 0x0019 /*Addr*/, 0x0000 /*Expect val*/,
                              0x0100/*Mask*/);
        // Waiting from         20 microseconds for expected events to take place
        spin(10 * 20);
        // Checking RTUNE ACK to be de-asserted
        // Expecting RTUNE ACK to be de-asserted :: 0
        // Asserting MPLLB/A enable
        pciephy_cr16_write(kunlun_pcie, 0x0009, 0x950b);
        // Asserting MPLLA/B enable
        pciephy_cr16_write(kunlun_pcie, 0x0005, 0x8000);
        // Waiting from        150 microseconds for expected events to take place
        spin(10 * 150);
        // Reading MPLLA STATE
        // Bit 13 MPLLA_STATE, Bit 14 MPLLB_STATE
        pciephy_cr16_read_tst(kunlun_pcie, 0x0019 /*Addr*/, 0x4000 /*Expect val*/,
                              0x6000/*Mask*/);
        // Checking MPLLA STATE to be asserted
        // Expecting MPLLA STATE to be asserted :: 1
    }
    else {   // GEN2/GEN1
        // Overriding mpllb_div_multiplier; mpllb_div_clk_en
        pciephy_cr16_write(kunlun_pcie, 0x0004, 0x0200);

        if (speed == 0x2) {
            // Hooking up tx0_clk to appropriate mpll word/dword/qword clock  ...Overriding tx_clk_sel
            pciephy_cr16_write(kunlun_pcie, 0xa0c1, 0x0005);
            // Overriding tx_clk_en_r
            pciephy_cr16_write(kunlun_pcie, 0xa0c1, 0x0005);
        }
        else {   // GEN1
            // Hooking up tx0_clk to appropriate mpll word/dword/qword clock  ...Overriding tx_clk_sel
            pciephy_cr16_write(kunlun_pcie, 0xa0c1, 0x0007);
            // Overriding tx_clk_en_r
            pciephy_cr16_write(kunlun_pcie, 0xa0c1, 0x0007);
        }

        // Overriding lane_rx2tx_par_lb_en and lane_tx2rx_ser_lb_en
        pciephy_cr16_write(kunlun_pcie, 0x9000, 0x0005);
        // Overriding tx_eq_main;tx_iboost_lvl_r; tx_beacon_en; tx_disable
        pciephy_cr16_write(kunlun_pcie, 0x9002, 0x28f8);
        // Overriding tx_eq_main_ovrd_in_en and tx_ovrd_in_1_en
        pciephy_cr16_write(kunlun_pcie, 0x9002, 0xa9f8);
        // Overriding tx_eq_pre and tx_eq_post
        pciephy_cr16_write(kunlun_pcie, 0x9003, 0x0800);
        // Overriding tx_eq_pre_en and tx_eq_post_en
        pciephy_cr16_write(kunlun_pcie, 0x9003, 0x2840);
        // tx_detrx_req; tx_mpllb_sel; tx_width[1:0]; tx_rate[2:0]; tx_pstate[1:0]; tx_lpd; tx_req; tx_data_en; tx_invert; tx_reset; tx_clk_rdy
        pciephy_cr16_write(kunlun_pcie, 0x9004, 0x0001);
        pciephy_cr16_write(kunlun_pcie, 0x9004, 0x00ab);

        if (speed == 0x2) {
            pciephy_cr16_write(kunlun_pcie, 0x9001, 0x0600);
            pciephy_cr16_write(kunlun_pcie, 0x9001, 0xaf12);
        }
        else {
            pciephy_cr16_write(kunlun_pcie, 0x9001, 0x0620);
            pciephy_cr16_write(kunlun_pcie, 0x9001, 0xaf32);
        }

        // rxX_eq_delta_iq
        pciephy_cr16_write(kunlun_pcie, 0xa019, 0x0000);
        pciephy_cr16_write(kunlun_pcie, 0xa019, 0x0000);
        // Overriding rx_eq_ctle_boost; rx_eq_vga2_gain; rx_eq_vga1_gain; rx_eq_att_lvl
        pciephy_cr16_write(kunlun_pcie, 0x900d, 0x3800);
        // Overriding rx_eq_dfe_tap1; rx_eq_ctle_pole
        pciephy_cr16_write(kunlun_pcie, 0x900e, 0x0000);
        // Overriding rx_ovrd_eq_in_en
        pciephy_cr16_write(kunlun_pcie, 0x900e, 0x8000);
        // Setting rx_ref_ld_val_r and rx_cdr_vco_lowfreq_r
        pciephy_cr16_write(kunlun_pcie, 0x9008, 0x001b);
        // Setting rx_ovrd_in_1_en
        pciephy_cr16_write(kunlun_pcie, 0x9008, 0x009b);
        // Setting  rx_vco_ld_val
        pciephy_cr16_write(kunlun_pcie, 0x9009, 0x0546);
        // Setting rx_ovrd_in_2_en
        pciephy_cr16_write(kunlun_pcie, 0x9009, 0x2546);
        // Overriding rx_term_en; rx_los_lfps_en; rx_los_threshold; rx_disable; rx_clk_shift; rx_align_en; rx_cdr_ssc_en; rx_cdr_track_en
        pciephy_cr16_write(kunlun_pcie, 0x900a, 0x4aa8);
        pciephy_cr16_write(kunlun_pcie, 0x900b, 0x0180);
        pciephy_cr16_write(kunlun_pcie, 0x900b, 0x0380);
        // Waiting from          5 microseconds for expected events to take place
        spin(10 * 5);
        // Perform the resistor tuning
        pciephy_cr16_write(kunlun_pcie, 0x000d, 0x0018);
        pciephy_cr16_write(kunlun_pcie, 0x000d, 0x001b);
        // Waiting from        200 microseconds for expected events to take place
        spin(10 * 200);
        // Reading RTUNE_ACK
        // Check RTURN_ACK=0x1
        pciephy_cr16_read_tst(kunlun_pcie, 0x0019 /*Addr*/, 0x0100 /*Expect val*/,
                              0x0100/*Mask*/);
        // Checking RTUNE ACK to be asserted
        // Expecting RTUNE ACK to be asserted :: 1
        spin(10 * 100);
        // Done with resistor tuning and de-asserting RTUNE_REQ
        pciephy_cr16_write(kunlun_pcie, 0x000d, 0x001a);
        spin(10 * 100);
        // Check RTUNE_ACK=0x0
        pciephy_cr16_read_tst(kunlun_pcie, 0x0019 /*Addr*/, 0x0000 /*Expect val*/,
                              0x0100/*Mask*/);
        // Waiting from         20 microseconds for expected events to take place
        spin(10 * 20);
        // Checking RTUNE ACK to be de-asserted
        // Expecting RTUNE ACK to be de-asserted :: 0
        // Asserting MPLLB/A enable
        pciephy_cr16_write(kunlun_pcie, 0x0009, 0x8000);
        // Asserting MPLLA/B enable
        pciephy_cr16_write(kunlun_pcie, 0x0005, 0x9199);
        // Waiting from        150 microseconds for expected events to take place
        spin(10 * 150);
        // Reading MPLLA STATE
        // Bit 13 MPLLA_STATE, Bit 14 MPLLB_STATE
        pciephy_cr16_read_tst(kunlun_pcie, 0x0019 /*Addr*/, 0x2000 /*Expect val*/,
                              0x6000/*Mask*/);
        // Checking MPLLA STATE to be asserted
        // Expecting MPLLA STATE to be asserted :: 1
    }

    // De-asserting per lane PCS-RAW TX reset
    pciephy_cr16_write(kunlun_pcie, 0xa001, 0x0f06);
    pciephy_cr16_write(kunlun_pcie, 0xa001, 0x0f06);
    // De-asserting per lane PMA TX reset
    pciephy_cr16_write(kunlun_pcie, 0x9005, 0x0002);
    pciephy_cr16_write(kunlun_pcie, 0x9005, 0x0002);
    // Reading TX_ACK=0x1
    // pciephy_cr16_read_tst(0x1014 /*Addr*/, 0x0001 /*Expect val*/, 0x0001/*Mask*/);
    // Waiting from        100 microseconds for expected events to take place
    spin(10 * 100);
    // Reading TX ACK
    // Checking TX ACK of Channel0 to be de-asserted
    // Reading TX_ACK=0x0
    // Bit 0 TX_ACK
    pciephy_cr16_read_tst(kunlun_pcie, 0x1014 /*Addr*/, 0x0000 /*Expect val*/,
                          0x0001/*Mask*/);
    // Expecting TX ACK of Channel0 to be de-asserted :: 0
    // Checking TX ACK of Channel1 to be de-asserted
    // Expecting TX ACK of Channel1 to be de-asserted :: 0
    // Bit 0 TX_ACK
    pciephy_cr16_read_tst(kunlun_pcie, 0x1114 /*Addr*/, 0x0000 /*Expect val*/,
                          0x0001/*Mask*/);

    if (speed == 0x3) { // GEN3
        // Overriding rx_adapt_dfe_en; rx_adapt_afe_en; rx_div16p5_clk_en; rx_mpllb_sel; rx_width; rx_rate; rx_pstate; rx_lpd; rx_req; rx_data_en; rx_invert; rx_reset
        pciephy_cr16_write(kunlun_pcie, 0x900b, 0x03f0);
        pciephy_cr16_write(kunlun_pcie, 0x900b, 0x03fa);
        pciephy_cr16_write(kunlun_pcie, 0x9007, 0x1e4e);
    }
    else if (speed == 0x2) { // GEN2
        // Overriding rx_adapt_dfe_en; rx_adapt_afe_en; rx_div16p5_clk_en; rx_mpllb_sel; rx_width; rx_rate; rx_pstate; rx_lpd; rx_req; rx_data_en; rx_invert; rx_reset
        pciephy_cr16_write(kunlun_pcie, 0x900b, 0x03c0);
        pciephy_cr16_write(kunlun_pcie, 0x900b, 0x03ca);
        pciephy_cr16_write(kunlun_pcie, 0x9007, 0x1ece);
    }
    else {   // GEN1
        // Overriding rx_adapt_dfe_en; rx_adapt_afe_en; rx_div16p5_clk_en; rx_mpllb_sel; rx_width; rx_rate; rx_pstate; rx_lpd; rx_req; rx_data_en; rx_invert; rx_reset
        pciephy_cr16_write(kunlun_pcie, 0x900b, 0x03c0);
        pciephy_cr16_write(kunlun_pcie, 0x900b, 0x03ca);
        pciephy_cr16_write(kunlun_pcie, 0x9007, 0x1f4e);
    }

    // De-asserting per lane PCS-RAW RX reset
    pciephy_cr16_write(kunlun_pcie, 0xa006, 0x0006);
    pciephy_cr16_write(kunlun_pcie, 0xa006, 0x0006);
    // De-asserting per lane PMA RX reset
    pciephy_cr16_write(kunlun_pcie, 0x900c, 0x0002);
    pciephy_cr16_write(kunlun_pcie, 0x900c, 0x0002);

    // Waiting from        200 microseconds for expected events to take place
    spin(10 * 200);
    // Reading RX ACK and RX VALID
    // Checking RX ACK to be de-asserted and RX VALID to be asserted on Channel0
    // Expecting RX ACK to be de-asserted and RX VALID to be asserted on Channel0 :: {0,1}
    // Bit 0 RX_ACK, Bit 2 RXVALID
    pciephy_cr16_read_tst(kunlun_pcie, 0x101B /*Addr*/, 0x0004 /*Expect val*/,
                          0x0005/*Mask*/);
    // Checking RX ACK to be de-asserted and RX VALID to be asserted on Channel1
    // Expecting RX ACK to be de-asserted and RX VALID to be asserted on Channel1 :: {0,1}
    // Bit 0 RX_ACK, Bit 2 RXVALID
    pciephy_cr16_read_tst(kunlun_pcie, 0x111B /*Addr*/, 0x0004 /*Expect val*/,
                          0x0005/*Mask*/);

    // Enabling TX LBERT in LFSR31
    pciephy_cr16_write(kunlun_pcie, 0x902a, 0x0000 | lbertmode | (lbertpat0 << 5));
    // Waiting from          5 microseconds for expected events to take place
    spin(10 * 5);

    // Enabling RX LBERT in LFSR31 and setting RX LBERT SYNC to 1
    //pciephy_cr16_write(0x9051, 0x0000 | lbertmode);
    // Set RX LBERT SYNC to 0
    pciephy_cr16_write(kunlun_pcie, 0x9051, 0x0000 | lbertmode);
    // Set RX LBERT SYNC to 1
    pciephy_cr16_write(kunlun_pcie, 0x9051, 0x0010 | lbertmode);
    // Set RX LBERT SYNC to 0
    pciephy_cr16_write(kunlun_pcie, 0x9051, 0x0000 | lbertmode);
    // Enabling RX LBERT in LFSR31 and setting RX LBERT SYNC to 1
    //pciephy_cr16_write(0x9051, 0x0000 | lbertmode);
    // Set RX LBERT SYNC to 0
    //pciephy_cr16_write(0x9051, 0x0000 | lbertmode);
    // Set RX LBERT SYNC to 1
    //pciephy_cr16_write(0x9051, 0x0010 | lbertmode);
    // Set RX LBERT SYNC to 0
    //pciephy_cr16_write(0x9051, 0x0000 | lbertmode);
    // + ATE_INTERNAL_LOOPBACK_TEST
    // +*************************************************************************
    // +Functional test to verify that the device is operational and
    // +the BERTS are functioning. The device is set-up in internal loopback.
    // +The berts are tested that they can match lfsr31 and that they can count
    // +errors
    // +*************************************************************************
    // Waiting from         10 microseconds for expected events to take place
    // Waiting from          1 microseconds for expected events to take place
    // LANE0_DIG_RX_LBERT_ERR
    spin(10 * 15);
    pciephy_cr16_read_tst(kunlun_pcie, 0x1052 /*Addr*/, 0x0000 /*Expect val*/,
                          0xFFFF/*Mask*/);
    // Bert errors on channel0 expect 0 error
    // Waiting from          1 microseconds for expected events to take place
    // LANE1_DIG_RX_LBERT_ERR
    pciephy_cr16_read_tst(kunlun_pcie, 0x1152 /*Addr*/, 0x0000 /*Expect val*/,
                          0xFFFF/*Mask*/);

    // Bert errors on channel1 expect 0 error
    for (i = 0; i < 10; i++) {
        unsigned int exp_errcnt;
        // Introducing error
        pciephy_cr16_write(kunlun_pcie, 0x902a, 0x0010 | lbertmode | (lbertpat0 << 5));
        pciephy_cr16_write(kunlun_pcie, 0x902a, 0x0000 | lbertmode | (lbertpat0 << 5));
        // Waiting from         10 microseconds for expected events to take place
        // Waiting from          1 microseconds for expected events to take place
        // LANE0_DIG_RX_LBERT_ERR

        spin(10 * 15);
        exp_errcnt = i + 1;

        if (lbertmode == 0x1) {
            exp_errcnt = i + 1;
            //} else {
        }
        else if (lbertmode == 0x9) {
            exp_errcnt = (i + 1) * 2;
        }

        pciephy_cr16_read_tst(kunlun_pcie, 0x1052 /*Addr*/, exp_errcnt /*Expect val*/,
                              0xFFFF/*Mask*/);
        // Bert errors on channel0 expect 1 error
        // Waiting from          1 microseconds for expected events to take place
        // LANE1_DIG_RX_LBERT_ERR
        pciephy_cr16_read_tst(kunlun_pcie, 0x1152 /*Addr*/, exp_errcnt /*Expect val*/,
                              0xFFFF/*Mask*/);
        // Bert errors on channel1 expect 1 error
    }
}

void kunlun_pcie2_phy_loopback_test(struct kunlun_pcie *kunlun_pcie)
{
    u32 reg_val;

    kunlun_pcie->ctrl_ncr_base = PCIE2_CTRL_NCR_BASE;
    kunlun_pcie->dbi = PCIE2_DBI_BASE;
    kunlun_pcie->phy_base = PCIE_PHY_BASE;
    kunlun_pcie->phy_ncr_base = PCIE_PHY_NCR_BASE;
    kunlun_pcie->ecam_base = PCIE2_ECAM_BASE;
    kunlun_pcie->pcie_index = PCIE2;
    kunlun_pcie->phy_refclk_sel = PHY_REFCLK_USE_INTERNAL;

    printf("PCIe%d--ctrl_ncr_base = 0x%x\n", kunlun_pcie->pcie_index,
           kunlun_pcie->ctrl_ncr_base);
    printf("PCIe%d--dbi = 0x%x\n", kunlun_pcie->pcie_index, kunlun_pcie->dbi);
    printf("PCIe%d--phy_base = 0x%x\n", kunlun_pcie->pcie_index,
           kunlun_pcie->phy_base);
    printf("PCIe%d--phy_ncr_base = 0x%x\n", kunlun_pcie->pcie_index,
           kunlun_pcie->phy_ncr_base);
    printf("PCIe%d--ecam_base = 0x%llx\n", kunlun_pcie->pcie_index,
           kunlun_pcie->ecam_base);

    /* clock setting*/
    kunlun_pcie_clk_enable(kunlun_pcie);

    reset_pcie_ss();


    reg_val = kunlun_phy_ncr_readl(kunlun_pcie, PCIE_PHY_NCR_CTRL0);
    reg_val &= ~BIF_EN_BIT;
    //reg_val |= BIF_EN_BIT;
    kunlun_phy_ncr_writel(kunlun_pcie, reg_val, PCIE_PHY_NCR_CTRL0);

    reg_val = kunlun_phy_ncr_readl(kunlun_pcie, PCIE_PHY_NCR_CTRL4);
    reg_val &= ~CR_ADDR_MODE_BIT;
    kunlun_phy_ncr_writel(kunlun_pcie, reg_val, PCIE_PHY_NCR_CTRL4);

    reg_val = kunlun_phy_ncr_readl(kunlun_pcie, PCIE_PHY_NCR_CTRL0);
    reg_val |= CR_CKEN_BIT;
    kunlun_phy_ncr_writel(kunlun_pcie, reg_val, PCIE_PHY_NCR_CTRL0);

    // PHYNCR.CTRL_1.PHY_REF_ALT_CLK_SEL=0x0: On-Chip PLL output 100MHz
    reg_val = kunlun_phy_ncr_readl(kunlun_pcie, PCIE_PHY_NCR_CTRL1);
    reg_val &= ~PHY_REF_ALT_CLK_SEL_BIT;
    kunlun_phy_ncr_writel(kunlun_pcie, reg_val, PCIE_PHY_NCR_CTRL1);

    if (kunlun_pcie->phy_refclk_sel == PHY_REFCLK_USE_INTERNAL) {
        reg_val = kunlun_phy_ncr_readl(kunlun_pcie, PCIE_PHY_NCR_CTRL1);
        reg_val &= ~PHY_REF_USE_PAD_BIT;
        kunlun_phy_ncr_writel(kunlun_pcie, reg_val, PCIE_PHY_NCR_CTRL1);
        printf("sel internal ref clk\n");
    }
    else if (kunlun_pcie->phy_refclk_sel == PHY_REFCLK_USE_EXTERNAL) {
        reg_val = kunlun_phy_ncr_readl(kunlun_pcie, PCIE_PHY_NCR_CTRL1);
        reg_val |= PHY_REF_USE_PAD_BIT;
        kunlun_phy_ncr_writel(kunlun_pcie, reg_val, PCIE_PHY_NCR_CTRL1);
        printf("sel external ref clk\n");
    }

    // set device type
    // 0: EP
    // 1: RC
    reg_val = kunlun_ctrl_ncr_readl(kunlun_pcie, PCIE_CTRL_NCR_CTRL0);
    reg_val |= DEVICE_TYPE_BIT;
    kunlun_ctrl_ncr_writel(kunlun_pcie, reg_val, PCIE_CTRL_NCR_CTRL0);

    reg_val = kunlun_ctrl_ncr_readl(kunlun_pcie, PCIE_CTRL_NCR_CTRL2);
    reg_val &= ~0x80;
    kunlun_ctrl_ncr_writel(kunlun_pcie, reg_val, PCIE_CTRL_NCR_CTRL2);
    thread_sleep(1);

    reg_val = kunlun_ctrl_ncr_readl(kunlun_pcie, PCIE_CTRL_NCR_CTRL2);
    reg_val |= 0x80;
    kunlun_ctrl_ncr_writel(kunlun_pcie, reg_val, PCIE_CTRL_NCR_CTRL2);
    thread_sleep(1);

    // set app_hold_phy_rst = 0
    reg_val = kunlun_ctrl_ncr_readl(kunlun_pcie, PCIE_CTRL_NCR_CTRL0);
    reg_val &= ~APP_HOLD_PHY_RST_BIT;
    kunlun_ctrl_ncr_writel(kunlun_pcie, reg_val, PCIE_CTRL_NCR_CTRL0);
    thread_sleep(1);

    //release phy reset
    reg_val = kunlun_phy_ncr_readl(kunlun_pcie, PCIE_PHY_NCR_CTRL0);
    reg_val &= ~PHY_RESET_BIT;
    kunlun_phy_ncr_writel(kunlun_pcie, reg_val, PCIE_PHY_NCR_CTRL0);
    thread_sleep(50);

    if (kunlun_pcie->phy_refclk_sel == PHY_REFCLK_USE_INTERNAL) {
        kunlun_pcie_phy_internal_loopback_test(kunlun_pcie, 0, 3, 0x1, 0x0);
    }
    else if (kunlun_pcie->phy_refclk_sel == PHY_REFCLK_USE_EXTERNAL) {
        kunlun_pcie_phy_internal_loopback_test(kunlun_pcie, 1, 3, 0x1, 0x0);
    }


}

int kunlun_pcie_phy_internal_loopback_check(struct kunlun_pcie *kunlun_pcie,
        unsigned int lbertmode, unsigned int lbertpat0)
{
    u32 i;
    int ret  = 0;

    unsigned int exp_errcnt0 = pciephy_cr16_read(kunlun_pcie, 0x1052) & 0xffff;
    unsigned int exp_errcnt1 = pciephy_cr16_read(kunlun_pcie, 0x1152) & 0xffff;

    for (i = 1; i <= 10; i++) {
        // Introducing error
        pciephy_cr16_write(kunlun_pcie, 0x902a, 0x0010 | lbertmode | (lbertpat0 << 5));
        pciephy_cr16_write(kunlun_pcie, 0x902a, 0x0000 | lbertmode | (lbertpat0 << 5));
        // Waiting from     10 microseconds for expected events to take place
        // Waiting from      1 microseconds for expected events to take place
        // LANE0_DIG_RX_LBERT_ERR

        spin(10 * 15);

        if (lbertmode == 0x1) {
            exp_errcnt0++;
            exp_errcnt1++;
        }
        else if (lbertmode == 0x9) {
            exp_errcnt0 += 2;
            exp_errcnt1 += 2;
        }

        printf("check lane0 error cnt\n");
        ret = pciephy_cr16_read_tst(kunlun_pcie, 0x1052 /*Addr*/,
                                    exp_errcnt0 /*Expect val*/, 0xFFFF/*Mask*/);
        // Bert errors on channel0 expect 1 error
        // Waiting from      1 microseconds for expected events to take place
        // LANE1_DIG_RX_LBERT_ERR
        printf("check lane1 error cnt\n");
        ret = pciephy_cr16_read_tst(kunlun_pcie, 0x1152 /*Addr*/,
                                    exp_errcnt1 /*Expect val*/, 0xFFFF/*Mask*/);
        // Bert errors on channel1 expect 1 error

    }

    return ret;
}

int kunlun_pcie_phy_loopback_state_check(struct kunlun_pcie *kunlun_pcie)
{
    int ret = 0;

    ret = kunlun_pcie_phy_internal_loopback_check(kunlun_pcie, 0x1, 0x0);

    return ret;
}
