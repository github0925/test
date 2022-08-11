/*
 * mmc_dwcmshc.c
 *
 * Copyright (c) 2018 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: mmc semidrive dwcmshc code.
 *
 * Revision History:
 * -----------------
 * 0.1, 12/9/2019 init version
 */
#include <assert.h>
#include <debug.h>
#include <kernel/vm.h>
#include <reg.h>

#include "chip_res.h"
#include "clkgen_hal.h"
#include "hal_port.h"
#include "mmc_sdhci.h"
#include "rstgen_hal.h"
#include "scr_hal.h"
#include "sdhci.h"

#define SDHCI_VENDER_EMMC_CTRL_REG (0x2C)

#define SDHCI_IS_EMMC_CARD_MASK BIT(0)

#define DWC_MSHC_PTR_PHY_REGS 0x300
#define DWC_MSHC_PHY_CNFG (DWC_MSHC_PTR_PHY_REGS + 0x0)
#define PAD_SN_LSB 20
#define PAD_SN_MASK 0xF
#define PAD_SN_DEFAULT ((0x8 & PAD_SN_MASK) << PAD_SN_LSB)
#define PAD_SP_LSB 16
#define PAD_SP_MASK 0xF
#define PAD_SP_DEFAULT ((0x9 & PAD_SP_MASK) << PAD_SP_LSB)
#define PHY_PWRGOOD BIT(1)
#define PHY_RSTN BIT(0)

#define DWC_MSHC_CMDPAD_CNFG (DWC_MSHC_PTR_PHY_REGS + 0x4)
#define DWC_MSHC_DATPAD_CNFG (DWC_MSHC_PTR_PHY_REGS + 0x6)
#define DWC_MSHC_CLKPAD_CNFG (DWC_MSHC_PTR_PHY_REGS + 0x8)
#define DWC_MSHC_STBPAD_CNFG (DWC_MSHC_PTR_PHY_REGS + 0xA)
#define DWC_MSHC_RSTNPAD_CNFG (DWC_MSHC_PTR_PHY_REGS + 0xC)
#define TXSLEW_N_LSB 9
#define TXSLEW_N_MASK 0xF
#define TXSLEW_P_LSB 5
#define TXSLEW_P_MASK 0xF
#define WEAKPULL_EN_LSB 3
#define WEAKPULL_EN_MASK 0x3
#define RXSEL_LSB 0
#define RXSEL_MASK 0x3

#define DWC_MSHC_COMMDL_CNFG (DWC_MSHC_PTR_PHY_REGS + 0x1C)
#define DWC_MSHC_SDCLKDL_CNFG (DWC_MSHC_PTR_PHY_REGS + 0x1D)
#define DWC_MSHC_SDCLKDL_DC (DWC_MSHC_PTR_PHY_REGS + 0x1E)
#define DWC_MSHC_SMPLDL_CNFG (DWC_MSHC_PTR_PHY_REGS + 0x20)
#define DWC_MSHC_ATDL_CNFG (DWC_MSHC_PTR_PHY_REGS + 0x21)

#define DWC_MSHC_DLL_CTRL (DWC_MSHC_PTR_PHY_REGS + 0x24)
#define DWC_MSHC_DLL_CNFG1 (DWC_MSHC_PTR_PHY_REGS + 0x25)
#define DWC_MSHC_DLL_CNFG2 (DWC_MSHC_PTR_PHY_REGS + 0x26)
#define DWC_MSHC_DLLDL_CNFG (DWC_MSHC_PTR_PHY_REGS + 0x28)
#define DWC_MSHC_DLL_OFFSET (DWC_MSHC_PTR_PHY_REGS + 0x29)
#define DWC_MSHC_DLLLBT_CNFG (DWC_MSHC_PTR_PHY_REGS + 0x2C)
#define DWC_MSHC_DLL_STATUS (DWC_MSHC_PTR_PHY_REGS + 0x2E)
#define ERROR_STS BIT(1)
#define LOCK_STS BIT(0)

#define DWC_MSHC_PHY_PAD_SD_CLK                                                \
    ((1 << TXSLEW_N_LSB) | (3 << TXSLEW_P_LSB) | (0 << WEAKPULL_EN_LSB) |      \
     (1 << RXSEL_LSB))
#define DWC_MSHC_PHY_PAD_SD_DAT                                                \
    ((1 << TXSLEW_N_LSB) | (3 << TXSLEW_P_LSB) | (1 << WEAKPULL_EN_LSB) |      \
     (1 << RXSEL_LSB))
#define DWC_MSHC_PHY_PAD_SD_STB                                                \
    ((1 << TXSLEW_N_LSB) | (3 << TXSLEW_P_LSB) | (2 << WEAKPULL_EN_LSB) |      \
     (1 << RXSEL_LSB))

#define DWC_MSHC_PHY_PAD_EMMC_CLK                                              \
    ((2 << TXSLEW_N_LSB) | (2 << TXSLEW_P_LSB) | (0 << WEAKPULL_EN_LSB) |      \
     (0 << RXSEL_LSB))
#define DWC_MSHC_PHY_PAD_EMMC_DAT                                              \
    ((2 << TXSLEW_N_LSB) | (2 << TXSLEW_P_LSB) | (1 << WEAKPULL_EN_LSB) |      \
     (1 << RXSEL_LSB))
#define DWC_MSHC_PHY_PAD_EMMC_STB                                              \
    ((2 << TXSLEW_N_LSB) | (2 << TXSLEW_P_LSB) | (2 << WEAKPULL_EN_LSB) |      \
     (1 << RXSEL_LSB))

#define SLICE_ID(x) (x)
#define PREDIV(x) (x)
#define POSTDIV(x) (x)
#define SLICE_SRC(x) (x)

#define CLKGEN_PRE_DIV_NUM_MAX 0x7
#define CLKGEN_POST_DIV_NUM_MAX 0x3F

extern struct mmc_priv_res g_mmc_priv_res[];

extern const domain_res_t g_iomuxc_res;

static inline uint32_t abs(int a) { return (a > 0) ? a : -a; }

static inline void mshc_phy_pad_config(struct sdhci_host *host,
                                       uint32_t card_type)
{
    uint16_t clk_ctrl;

     /* Disable the card clock */
    clk_ctrl = REG_READ16(host, SDHCI_CLK_CTRL_REG);
    clk_ctrl &= ~SDHCI_CLK_EN;
    REG_WRITE16(host, clk_ctrl, SDHCI_CLK_CTRL_REG);

    if (card_type >= MMC_TYPE_MMCHC) {
        REG_WRITE16(host, DWC_MSHC_PHY_PAD_EMMC_DAT, DWC_MSHC_CMDPAD_CNFG);
        REG_WRITE16(host, DWC_MSHC_PHY_PAD_EMMC_DAT, DWC_MSHC_DATPAD_CNFG);
        REG_WRITE16(host, DWC_MSHC_PHY_PAD_EMMC_CLK, DWC_MSHC_CLKPAD_CNFG);
        REG_WRITE16(host, DWC_MSHC_PHY_PAD_EMMC_STB, DWC_MSHC_STBPAD_CNFG);
        REG_WRITE16(host, DWC_MSHC_PHY_PAD_EMMC_DAT, DWC_MSHC_RSTNPAD_CNFG);
    }
    else {
        REG_WRITE16(host, DWC_MSHC_PHY_PAD_SD_DAT, DWC_MSHC_CMDPAD_CNFG);
        REG_WRITE16(host, DWC_MSHC_PHY_PAD_SD_DAT, DWC_MSHC_DATPAD_CNFG);
        REG_WRITE16(host, DWC_MSHC_PHY_PAD_SD_CLK, DWC_MSHC_CLKPAD_CNFG);
        REG_WRITE16(host, DWC_MSHC_PHY_PAD_SD_STB, DWC_MSHC_STBPAD_CNFG);
        REG_WRITE16(host, DWC_MSHC_PHY_PAD_SD_DAT, DWC_MSHC_RSTNPAD_CNFG);
    }
    return;
}

static inline void mshc_phy_delay_config(struct sdhci_host *host)
{
    REG_WRITE8(host, 1, DWC_MSHC_COMMDL_CNFG);
    REG_WRITE8(host, 0, DWC_MSHC_SDCLKDL_CNFG);
    REG_WRITE8(host, 8, DWC_MSHC_SMPLDL_CNFG);
    REG_WRITE8(host, 8, DWC_MSHC_ATDL_CNFG);
    return;
}

static inline int mshc_phy_dll_config(struct sdhci_host *host)
{
    uint32_t ret;
    uint16_t clk_ctrl = 0;

    REG_WRITE8(host, 0, DWC_MSHC_DLL_CTRL);

    /* Disable the clock */
    clk_ctrl = REG_READ16(host, SDHCI_CLK_CTRL_REG);
    clk_ctrl &= ~SDHCI_CLK_EN;
    REG_WRITE16(host, clk_ctrl, SDHCI_CLK_CTRL_REG);

    REG_WRITE8(host, 0x20, DWC_MSHC_DLL_CNFG1);
    // TODO: set the dll value by real chip
    REG_WRITE8(host, 0, DWC_MSHC_DLL_CNFG2);
    REG_WRITE8(host, 0x60, DWC_MSHC_DLLDL_CNFG);
    REG_WRITE8(host, 0, DWC_MSHC_DLL_OFFSET);
    REG_WRITE16(host, 0, DWC_MSHC_DLLLBT_CNFG);

    /* Enable the clock */
    clk_ctrl |= SDHCI_CLK_EN;
    REG_WRITE16(host, clk_ctrl, SDHCI_CLK_CTRL_REG);

    REG_WRITE8(host, 0x1, DWC_MSHC_DLL_CTRL);

    ret = sdhci_wait_for_bit(host, DWC_MSHC_DLL_STATUS, LOCK_STS, 0, 150);

    if (!ret) {
        ret = REG_READ8(host, DWC_MSHC_DLL_STATUS) & ERROR_STS;
    }

    return ret;
}

static int mshc_phy_init(struct sdhci_host *host)
{
    int ret;
    uint32_t reg = 0;

    REG_WRITE32(host, 0, DWC_MSHC_PHY_CNFG);

    /* Disable the clock */
    REG_WRITE16(host, 0, SDHCI_CLK_CTRL_REG);

    mshc_phy_pad_config(host, MMC_TYPE_MMCHC);
    mshc_phy_delay_config(host);

    ret = sdhci_wait_for_bit(host, DWC_MSHC_PHY_CNFG, PHY_PWRGOOD, 0, 150);
    if (!ret) {
        reg = PAD_SN_DEFAULT | PAD_SP_DEFAULT;
        REG_WRITE32(host, reg, DWC_MSHC_PHY_CNFG);
        /* de-assert the phy */
        reg |= PHY_RSTN;
        REG_WRITE32(host, reg, DWC_MSHC_PHY_CNFG);
    }
    return ret;
}

static int mshc_clkgen_config(struct sdhci_host *host, unsigned int freq)
{
    int ret = 0;
    void *mmc_ckgen_handle;
    clkgen_app_ip_cfg_t ip_cfg;
    ip_cfg.clk_src_select_num = 4;
    // TODO: hardcode mshc clock source 400Mhz
    const uint32_t mshc_base_freq = 400000000;
    uint32_t clock_div;
    uint32_t pre_div, post_div;
    uint32_t clock_div_succ = 0;
    uint32_t freq_bias = mshc_base_freq;
    uint32_t curr_freq_bias;

    if (!hal_clock_creat_handle(&mmc_ckgen_handle)) {
        printf("%s: clkgen creat handle failed\n", __FUNCTION__);
        return -1;
    }

    clock_div = DIV_ROUND_UP(mshc_base_freq, freq);

    for (int i = 0; i <= CLKGEN_PRE_DIV_NUM_MAX; i++) {
        pre_div = i;
        post_div = DIV_ROUND_UP(clock_div, i + 1) - 1;

        if (post_div <= CLKGEN_POST_DIV_NUM_MAX) {
            if (0 == clock_div % (pre_div + 1)) {
                ip_cfg.pre_div = pre_div;
                ip_cfg.post_div = post_div;
                clock_div_succ = 1;
                break;
            }

            curr_freq_bias = abs((pre_div + 1) * (post_div + 1) - clock_div);
            if (curr_freq_bias < freq_bias) {
                freq_bias = curr_freq_bias;
                ip_cfg.pre_div = pre_div;
                ip_cfg.post_div = post_div;
                clock_div_succ = 1;
            }
        }
    }

    if (0 == clock_div_succ) {
        dprintf(CRITICAL, "%s: calculate the clock div failed!\n",
                __FUNCTION__);
        ret = -1;
        goto clkgen_release_handle;
    }

    if (!hal_clock_ipclk_set(mmc_ckgen_handle,
                             g_mmc_priv_res[host->slot].clk_ip_idx, &ip_cfg)) {
        dprintf(CRITICAL, "%s: clkgen set ip clock failed\n", __FUNCTION__);
        ret = -1;
        goto clkgen_release_handle;
    }

    /*enable mmc host clock*/
    if (!hal_clock_enable(mmc_ckgen_handle,
                          g_mmc_priv_res[host->slot].clk_ip_gate_idx)) {
        dprintf(CRITICAL, "%s: clkgen enable ip clock failed\n", __FUNCTION__);
        ret = -1;
        goto clkgen_release_handle;
    }

clkgen_release_handle:
    /*release clock handle*/
    hal_clock_release_handle(mmc_ckgen_handle);

    return ret;
}

static uint32_t mshc_set_clk(struct sdhci_host *host, unsigned int clock)
{
    uint32_t ret = 0;
    uint16_t clk_ctrl = 0;

    /* Disable the clock */
    REG_WRITE16(host, clk_ctrl, SDHCI_CLK_CTRL_REG);

    /*
     * Beacuse the clock will be 2 dvider by mshc model,
     * so we need twice base frequency.
     */
    if (mshc_clkgen_config(host, MIN(clock, host->max_clk_rate) * 2))
        return 1;

    clk_ctrl |= SDHCI_INT_CLK_EN;
    REG_WRITE16(host, clk_ctrl, SDHCI_CLK_CTRL_REG);

    /* Check for clock stable, timeout 150ms */
    ret =
        sdhci_wait_for_bit(host, SDHCI_CLK_CTRL_REG, SDHCI_CLK_STABLE, 0, 150);
    if (ret) {
        dprintf(CRITICAL, "Error: sdhci clock wait stable timeout!");
        return ret;
    }

    clk_ctrl |= SDHCI_CLK_PLL_EN;
    REG_WRITE16(host, clk_ctrl, SDHCI_CLK_CTRL_REG);

    /* Check for clock stable, timeout 150ms */
    ret =
        sdhci_wait_for_bit(host, SDHCI_CLK_CTRL_REG, SDHCI_CLK_STABLE, 0, 150);
    if (ret) {
        dprintf(CRITICAL, "Error: sdhci clock wait stable timeout!");
        return ret;
    }

    /* Now clock is stable, enable it */
    clk_ctrl = REG_READ16(host, SDHCI_CLK_CTRL_REG);
    clk_ctrl |= SDHCI_CLK_EN;
    REG_WRITE16(host, clk_ctrl, SDHCI_CLK_CTRL_REG);
    return 0;
}

static bool scr_write_signal(const scr_signal_t signal, uint32_t val)
{
    bool ret;
    scr_handle_t handle;

    handle = hal_scr_create_handle(signal);

    if (handle) {
        ret = hal_scr_set(handle, val);
        if (!ret)
            dprintf(CRITICAL, "mshc: failed to set scr!\n");
        hal_scr_delete_handle(handle);
        return ret;
    }
    else {
        dprintf(CRITICAL, "mshc: failed to create scr handle!\n");
        return false;
    }
}

static void mshc_init(struct sdhci_host *host)
{
    uint16_t reg;
    uint32_t card_is_emmc;
    uint16_t vender_base;

    /* dwcmshc need set card type, emmc or other */
    card_is_emmc = !host->card_type;

    /* read verder base register address */
    vender_base = REG_READ16(host, SDHCI_VENDOR_BASE_REG) & 0xFFF;

    reg = REG_READ16(host, vender_base + SDHCI_VENDER_EMMC_CTRL_REG);
    reg &= ~SDHCI_IS_EMMC_CARD_MASK;
    reg |= card_is_emmc;
    REG_WRITE16(host, reg, vender_base + SDHCI_VENDER_EMMC_CTRL_REG);

    if (mshc_phy_init(host))
        dprintf(CRITICAL, "mshc: phy init failed!\n");
}

static void mshc_set_uhs_mode(struct sdhci_host *host, uint32_t mode)
{
    sdhci_set_uhs_mode(host, mode);

    /* if ddr mode, need set ddr_mode enable in scr */
    u32 ddr_mode = 0;

    /* it may be high speed DDR or HS400 */
    if (mode > SDHCI_EMMC_HS200_MODE)
        ddr_mode = 1;

    /* set mshc ddr mode in scr */

    scr_write_signal(g_mmc_priv_res[host->slot].ddr_mode_scr_signal, ddr_mode);

    if (mode == SDHCI_EMMC_HS400_MODE) {
        if (mshc_phy_dll_config(host))
            dprintf(CRITICAL, "mshc phy dll config failed!\n");
    }

    return;
}

struct sdhci_ops sdhci_dwcmshc_ops = {
    .priv_init = mshc_init,
    .set_clock = mshc_set_clk,
    .set_uhs_mode = mshc_set_uhs_mode,
};

static bool model_reset(uint32_t res_id)
{
    bool ret = true;
    static void *g_handle;
    ret = hal_rstgen_creat_handle(&g_handle, RES_GLOBAL_RST_SEC_RST_EN);

    if (!ret) {
        return -1;
    }

    /*get handle ok and enable rstgen is true*/
    ret = hal_rstgen_init(g_handle);

    if (ret) {
        ret = hal_rstgen_module_reset(g_handle, res_id);
    }

    ret &= hal_rstgen_release_handle(g_handle);

    return ret;
}

int mmc_platform_init(struct mmc_device *dev)
{
    struct sdhci_host *host;

    host = &dev->host;
    host->ops = &sdhci_dwcmshc_ops;

    if (!model_reset(g_mmc_priv_res[dev->config.slot].rst_idx)) {
        dprintf(CRITICAL, "reset mshc failed!\n");
        return -1;
    }

    /* Init the ref clock: 400Khz */
    if (mshc_clkgen_config(host, 400000 * 2))
        return -1;

    if (mmc_sdhci_init(dev)) {
        dprintf(CRITICAL, "mmc init failed!\n");
        return -1;
    }

    return 0;
}
