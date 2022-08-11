/*
 * bipc.c
 *
 * Copyright (c) 2019 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: The BIPC performs Bus Integrity Protocol Convertion
 * in between modules using different BIP protocols. BIPC basically
 * doesn't need SW control, except for BIPC SEM reports, which are
 * managed in this driver.
 *
 * Revision History:
 * -----------------
 */
#include <assert.h>
#include <compiler.h>
#include <reg.h>

#include "lib/reg.h"
#include "__regs_base.h"
#include "bipc.h"


static const uint32_t g_bipc_addr[BIPC_MAX] = {
    [BIPC_VP6]  = BIPC_VSN_BASE,
    [BIPC_DDR]  = APB_BIPC_DDR_BASE,
    [BIPC_ENET] = APB_BIPC_ENET1_BASE,
};

/* Register offset of AXI buses. */
static const uint32_t g_bipc_axi_bus_regbase[] = {
    0x10,   /* CH_1 */
    0x50,   /* CH_2 */
    0x80    /* CH_3 */
};

/*
 * BIPC register offset.
 */
#define BIPC_CTL            (0x0)   /* vp6 only */
#define BIPC_AXI_CHNL(chnl_idx, axichnl) \
    (g_bipc_axi_bus_regbase[chnl_idx] + (0x8 * axichnl))
#define BIPC_AXI_CHNL_MASK(chnl_idx, axichnl) \
    (BIPC_AXI_CHNL(chnl_idx, axichnl) + 0x4)

/*
 * Number of AXI buses the BIPC module is monitoring.
 */
static const int g_bipc_axi_bus_nr[BIPC_MAX] = {
    /* FAB_VSN m0 (VP6 master)
     * FAB_VSN m1 (VP6 iDMA master)
     * FAB_VSN s0 (VP6 slave)
     */
    [BIPC_VP6]  = 3,

    /* FAB_HPIa s_0_sbp */
    [BIPC_DDR]  = 1,

    /* FAB_SAF m_4 */
    [BIPC_ENET] = 1,
};

static inline void bipc_write_reg(enum bipc bipc, uint32_t reg,
                                  uint32_t val)
{
    writel(val, _ioaddr(g_bipc_addr[bipc] + reg));
}

static inline uint32_t bipc_read_reg(enum bipc bipc, uint32_t reg)
{
    return readl(_ioaddr(g_bipc_addr[bipc] + reg));
}

static inline void bipc_modify_reg(enum bipc bipc, uint32_t reg,
                                   uint32_t mask, uint32_t val)
{
    uint32_t _val = bipc_read_reg(bipc, reg);
    _val &= ~mask;
    _val |= val;
    bipc_write_reg(bipc, reg, _val);
}

/*
 * Enable or disable BIPC monitoring on AXI signal.
 *
 * @bipc        BIPC module
 * @bus         AXI bus index. 0 for enet1/ddr, 0~2 for vp6.
 * @axichnl     AXI channel (AR, AW, W, R, B, COR)
 * @signal      Signal bit in the channel. See __regs_ap_bipc_xxxx.h for bit definition.
 * @enable      Enable or disable monitoring on the signal.
 */
void bipc_monitor_signal_err(enum bipc bipc, int bus,
                             enum bipc_axichnl axichnl,
                             int signal, bool enable)
{
    ASSERT(bipc >= BIPC_VP6 && bipc < BIPC_MAX);
    ASSERT(bus >= 0 && bus < g_bipc_axi_bus_nr[bipc]);

    bipc_modify_reg(bipc, BIPC_AXI_CHNL_MASK(bus, axichnl),
                    1ul << signal, (uint32_t)enable << signal);
}

/*
 * Check if the signal is in error condition.
 *
 * @bipc        BIPC module
 * @bus         AXI bus index. 0 for enet1/ddr, 0~2 for vp6.
 * @axichnl     AXI channel (ar, aw, w, r, b, cor)
 * @signal      Signal bit within @axichnl. See __regs_ap_bipc_xxxx.h for bit definition.
 * @return      True if signal status is invalid.
 */
bool bipc_is_signal_err(enum bipc bipc, int bus,
                        enum bipc_axichnl axichnl, int signal)
{
    ASSERT(bipc >= BIPC_VP6 && bipc < BIPC_MAX);
    ASSERT(bus >= 0 && bus < g_bipc_axi_bus_nr[bipc]);

    return !!(bipc_read_reg(bipc, BIPC_AXI_CHNL(bus, axichnl))
                & (1ul << signal));
}

/*
 * Clear BIPC error status.
 *
 * @bipc        BIPC module
 * @bus         AXI bus index. 0 for enet1/ddr, 0~2 for vp6.
 * @axichnl     AXI channel (ar, aw, w, r, b, cor)
 * @signal      Signal bit within @axichnl. See __regs_ap_bipc_xxxx.h for bit definition.
 */
void bipc_clear_signal_err(enum bipc bipc, int bus,
                           enum bipc_axichnl axichnl, int signal)
{
    ASSERT(bipc >= BIPC_VP6 && bipc < BIPC_MAX);
    ASSERT(bus >= 0 && bus < g_bipc_axi_bus_nr[bipc]);

    bipc_write_reg(bipc, BIPC_AXI_CHNL(bus, axichnl), 1ul << signal);
}

void bipc_init(enum bipc bipc)
{
    if (bipc == BIPC_VP6) {
        bipc_write_reg(bipc, BIPC_CTL, 1);  /* parity select: odd */
    }
}
