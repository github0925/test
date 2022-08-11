/*
 * cadence_ospi.c
 *
 * Copyright (c) 2018 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: ospi controller.
 *
 * Revision History:
 * -----------------
 * 0.1, 4/30/2019 init version
 * 0.2, 5/13/2019 add indirect access with dma
 * 0.3, 7/30/2019 add phy mode and stig membank
 * 0.4, 11/20/2019 supprot D-cache
 * 0.5, 11/25/2019 add support for asynchronous call in dma mode
 */

#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include <debug.h>
#include <reg.h>
#include <pow2.h>
//#include <arch/arm/mpu.h>
#include <kernel/thread.h>
#include <kernel/event.h>
#if WITH_KERNEL_VM
#include <kernel/vm.h>
#endif
#include <lk/init.h>
#include <platform.h>
#include <platform/interrupts.h>
#include <lib/reg.h>

#include <dma_hal.h>

#ifdef ENABLE_SD_DMA
#define COSPI_DMA_ENABLE 1
#else
#define COSPI_DMA_ENABLE 0
#endif

#include "spi_nor.h"

enum cospi_flash_phy_mode {
    COSPI_PHY_NONE = 0,
    COSPI_PHY_BYPASS_DLL,
    COSPI_PHY_MASTER_DLL,
};

struct cospi_pdata {
    u8 id;
    addr_t apb_base;
    addr_t ahb_base;
    unsigned int irq;
    mutex_t bus_mutex;
    event_t dma_event;
    event_t complete_event;
    event_t xfer_start_event;
    event_t xfer_done_event;

    struct dma_chan *dma_chan;
    struct dma_desc *dma_desc;

    bool cs_decoded;
    u8 current_cs;

    u8 block_power_index;
    u32 page_size;

    unsigned long ref_clk_hz;
    unsigned int sclk;

    bool dqs_en;
    bool rclk_loopback;
    u8 master_delay;
    u8 capture_delay;

    u8 addr_bytes;
    u8 sram_size_nbit;
    u8 fifo_depth;
    u8 fifo_width;
    u32 trigger_address;
    u32 trigger_range_size;
    enum cospi_flash_phy_mode phy_mode;
    bool phy_training_succ;
    void *priv;
};

#undef BIT
#define BIT(nr) (1U << (nr))

#define udelay(x) spin(x)

#define OSPI_NS_2_TICKS(hz, ns) (((hz) / 1000 * (ns) + 1000000) / 1000000)

/* Operation timeout value */
#define COSPI_TIMEOUT_MS 1000
#define COSPI_READ_TIMEOUT_MS 10

/* Instruction type */
#define COSPI_INST_TYPE_SINGLE 0
#define COSPI_INST_TYPE_DUAL 1
#define COSPI_INST_TYPE_QUAD 2
#define COSPI_INST_TYPE_OCTAL 3

#define COSPI_DUMMY_CLKS_PER_BYTE 8
#define COSPI_DUMMY_BYTES_MAX 4
#define COSPI_DUMMY_CLKS_MAX 31

#define COSPI_STIG_DATA_LEN_MAX 512

/* Register map */
#define COSPI_REG_CONFIG 0x00
#define COSPI_REG_CONFIG_ENABLE_MASK BIT(0)
#define COSPI_REG_CONFIG_ENABLE_PHY_MASK BIT(3)
#define COSPI_REG_CONFIG_RESET_PIN_MASK BIT(5)
#define COSPI_REG_CONFIG_RESET_SELECT_MASK BIT(6)
#define COSPI_REG_CONFIG_ENB_DIR_ACC_CTRL BIT(7)
#define COSPI_REG_CONFIG_DECODE_MASK BIT(9)
#define COSPI_REG_CONFIG_CHIPSELECT_LSB 10
#define COSPI_REG_CONFIG_DMA_MASK BIT(15)
#define COSPI_REG_CONFIG_DMA_LSB 15
#define COSPI_REG_CONFIG_ENTER_XIP BIT(17)
#define COSPI_REG_CONFIG_BAUD_LSB 19
#define COSPI_REG_CONFIG_DTR_ENABLE_MASK BIT(24)
#define COSPI_REG_CONFIG_DTR_ENABLE_LSB 24
#define COSPI_REG_CONFIG_IDLE_MASK BIT(31)
#define COSPI_REG_CONFIG_CHIPSELECT_MASK 0xF
#define COSPI_REG_CONFIG_BAUD_MASK 0xF

#define COSPI_REG_RD_INSTR 0x04
#define COSPI_REG_RD_INSTR_OPCODE_LSB 0
#define COSPI_REG_RD_INSTR_TYPE_INST_LSB 8
#define COSPI_REG_RD_INSTR_DDR_EN 10
#define COSPI_REG_RD_INSTR_TYPE_ADDR_LSB 12
#define COSPI_REG_RD_INSTR_TYPE_DATA_LSB 16
#define COSPI_REG_RD_INSTR_MODE_EN_LSB 20
#define COSPI_REG_RD_INSTR_DUMMY_LSB 24
#define COSPI_REG_RD_INSTR_TYPE_INST_MASK 0x3
#define COSPI_REG_RD_INSTR_TYPE_ADDR_MASK 0x3
#define COSPI_REG_RD_INSTR_TYPE_DATA_MASK 0x3
#define COSPI_REG_RD_INSTR_DUMMY_MASK 0x1F

#define COSPI_REG_WR_INSTR 0x08
#define COSPI_REG_WR_INSTR_OPCODE_LSB 0
#define COSPI_REG_WR_DIS_WEL_MASK BIT(8)
#define COSPI_REG_WR_INSTR_TYPE_ADDR_LSB 12
#define COSPI_REG_WR_INSTR_TYPE_DATA_LSB 16

#define COSPI_REG_DELAY 0x0C
#define COSPI_REG_DELAY_CSSOT_LSB 0
#define COSPI_REG_DELAY_CSEOT_LSB 8
#define COSPI_REG_DELAY_CSDADS_LSB 16
#define COSPI_REG_DELAY_CSDA_LSB 24
#define COSPI_REG_DELAY_CSSOT_MASK 0xFF
#define COSPI_REG_DELAY_CSEOT_MASK 0xFF
#define COSPI_REG_DELAY_CSDADS_MASK 0xFF
#define COSPI_REG_DELAY_CSDA_MASK 0xFF

#define COSPI_REG_READCAPTURE 0x10
#define COSPI_REG_READCAPTURE_BYPASS_LSB 0
#define COSPI_REG_READCAPTURE_DQSEN_LSB 8
#define COSPI_REG_READCAPTURE_DELAY_LSB 1
#define COSPI_REG_READCAPTURE_DELAY_MASK 0xF
#define COSPI_REG_READCAPTURE_MASTER_DELAY_LSB 16
#define COSPI_REG_READCAPTURE_MASTER_DELAY_MASK 0xF
#define COSPI_READCAPTURE_DELAY_DEFAULT 1

#define COSPI_REG_SIZE 0x14
#define COSPI_REG_SIZE_ADDRESS_LSB 0
#define COSPI_REG_SIZE_PAGE_LSB 4
#define COSPI_REG_SIZE_BLOCK_LSB 16
#define COSPI_REG_SIZE_ADDRESS_MASK 0xF
#define COSPI_REG_SIZE_PAGE_MASK 0xFFF
#define COSPI_REG_SIZE_BLOCK_MASK 0x3F
#define COSPI_REG_SIZE_SIZE_LSB 0x21

#define COSPI_REG_SRAMPARTITION 0x18
#define COSPI_REG_INDIRECTTRIGGER 0x1C

#define COSPI_REG_DMA 0x20
#define COSPI_REG_DMA_SINGLE_LSB 0
#define COSPI_REG_DMA_BURST_LSB 8
#define COSPI_REG_DMA_SINGLE_MASK 0xFF
#define COSPI_REG_DMA_BURST_MASK 0xFF

#define COSPI_REG_REMAP 0x24
#define COSPI_REG_MODE_BIT 0x28

#define COSPI_REG_SDRAMLEVEL 0x2C
#define COSPI_REG_SDRAMLEVEL_RD_LSB 0
#define COSPI_REG_SDRAMLEVEL_WR_LSB 16
#define COSPI_REG_SDRAMLEVEL_RD_MASK 0xFFFF
#define COSPI_REG_SDRAMLEVEL_WR_MASK 0xFFFF

#define COSPI_REG_IRQSTATUS 0x40
#define COSPI_REG_IRQSTATUS_INDIRECT_DONE_MASK BIT(2)
#define COSPI_REG_IRQMASK 0x44

#define COSPI_REG_INDIRECTRD 0x60
#define COSPI_REG_INDIRECTRD_START_MASK BIT(0)
#define COSPI_REG_INDIRECTRD_CANCEL_MASK BIT(1)
#define COSPI_REG_INDIRECTRD_SRAM_FULL_MASK BIT(3)
#define COSPI_REG_INDIRECTRD_DONE_MASK BIT(5)
#define COSPI_REG_INDIRECTRD_Q_DONE_MASK BIT(7)

#define COSPI_REG_INDIRECTRDWATERMARK 0x64
#define COSPI_REG_INDIRECTRDSTARTADDR 0x68
#define COSPI_REG_INDIRECTRDBYTES 0x6C

#define COSPI_REG_CMDCTRL_MEM 0x8C
#define COSPI_REG_CMDCTRL_MEM_EXECUTE_MASK BIT(0)
#define COSPI_REG_CMDCTRL_MEM_INPROGRESS_MASK BIT(1)
#define COSPI_REG_CTDCTRL_MEM_LEN_LSB 16
#define COSPI_REG_CTDCTRL_MEM_LEN_MASK 0x7
#define COSPI_REG_CTDCTRL_MEM_ADDR_LSB 20
#define COSPI_REG_CTDCTRL_MEM_ADDR_MASK 0x1F

#define COSPI_REG_CTDCTRL_MEM_DATA 0x8D

#define COSPI_REG_CMDCTRL 0x90
#define COSPI_REG_CMDCTRL_EXECUTE_MASK BIT(0)
#define COSPI_REG_CMDCTRL_INPROGRESS_MASK BIT(1)
#define COSPI_REG_CMDCTRL_MEMBANK_EN_MASK BIT(2)
#define COSPI_REG_CMDCTRL_WR_DUMMY_LSB 7
#define COSPI_REG_CMDCTRL_WR_DUMMY_MASK 0xF
#define COSPI_REG_CMDCTRL_WR_BYTES_LSB 12
#define COSPI_REG_CMDCTRL_WR_EN_LSB 15
#define COSPI_REG_CMDCTRL_ADD_BYTES_LSB 16
#define COSPI_REG_CMDCTRL_ADDR_EN_LSB 19
#define COSPI_REG_CMDCTRL_RD_BYTES_LSB 20
#define COSPI_REG_CMDCTRL_RD_EN_LSB 23
#define COSPI_REG_CMDCTRL_OPCODE_LSB 24
#define COSPI_REG_CMDCTRL_WR_BYTES_MASK 0x7
#define COSPI_REG_CMDCTRL_ADD_BYTES_MASK 0x3
#define COSPI_REG_CMDCTRL_RD_BYTES_MASK 0x7

#define COSPI_REG_INDIRECTWR 0x70
#define COSPI_REG_INDIRECTWR_START_MASK BIT(0)
#define COSPI_REG_INDIRECTWR_CANCEL_MASK BIT(1)
#define COSPI_REG_INDIRECTWR_IN_MASK BIT(2)
#define COSPI_REG_INDIRECTWR_DONE_MASK BIT(5)

#define COSPI_REG_INDIRECTTRIGGER_RANGE 0x80

#define COSPI_REG_INDIRECTWRWATERMARK 0x74
#define COSPI_REG_INDIRECTWRSTARTADDR 0x78
#define COSPI_REG_INDIRECTWRBYTES 0x7C

#define COSPI_REG_CMDADDRESS 0x94
#define COSPI_REG_CMDREADDATALOWER 0xA0
#define COSPI_REG_CMDREADDATAUPPER 0xA4
#define COSPI_REG_CMDWRITEDATALOWER 0xA8
#define COSPI_REG_CMDWRITEDATAUPPER 0xAC

/* Interrupt status bits */
#define COSPI_REG_IRQ_MODE_ERR BIT(0)
#define COSPI_REG_IRQ_UNDERFLOW BIT(1)
#define COSPI_REG_IRQ_IND_COMP BIT(2)
#define COSPI_REG_IRQ_IND_RD_REJECT BIT(3)
#define COSPI_REG_IRQ_WR_PROTECTED_ERR BIT(4)
#define COSPI_REG_IRQ_ILLEGAL_AHB_ERR BIT(5)
#define COSPI_REG_IRQ_WATERMARK BIT(6)
#define COSPI_REG_IRQ_IND_SRAM_FULL BIT(12)
#define COSPI_REG_IRQ_ECC_ERR BIT(19)

/* auto polling status bits */
#define COSPI_REG_FLASH_STATUS 0xB0
#define COSPI_REG_FLASH_STATUS_STATUS_LSB 0
#define COSPI_REG_FLASH_STATUS_STATUS_MASK 0xFF
#define COSPI_REG_FLASH_STATUS_STATUS_LSB 0
#define COSPI_REG_FLASH_STATUS_VALID BIT(8)
#define COSPI_REG_FLASH_STATUS_DUMMY_LSB 16
#define COSPI_REG_FLASH_STATUS_DUMMY_MASK 0xF

/* Internal phy register */
#define COSPI_REG_PHYCONFIG 0xB4
#define COSPI_REG_PHYCONFIG_RX_DELAY_LSB 0
#define COSPI_REG_PHYCONFIG_TX_DELAY_LSB 16
#define COSPI_REG_PHYCONFIG_RX_BYPASS BIT(29)
#define COSPI_REG_PHYCONFIG_RX_DELAY_MASK 0x7F
#define CCOSPI_REG_PHYCONFIG_TX_DELAY_MASK 0x7F
#define COSPI_REG_PHYCONFIG_RST BIT(30)
#define COSPI_REG_PHYCONFIG_RESYNC BIT(31)

#define COSPI_REG_PHYMASTERCTL 0xB8
#define COSPI_REG_PHYMASTERCTL_INIT_DELAY_LSB 0
#define COSPI_REG_PHYMASTERCTL_INIT_DELAY_MASK 0x7F
#define COSPI_REG_PHYMASTERCTL_BYPASS BIT(23)

#define COSPI_REG_PHYDLLOBS 0xBC
#define COSPI_REG_PHYDLLOBS_LOCK BIT(0)
#define COSPI_REG_PHYDLLOBS_LOOPBACK_LOCK BIT(15)

#define COSPI_IRQ_MASK_RD                                                      \
    (COSPI_REG_IRQ_WATERMARK | COSPI_REG_IRQ_IND_SRAM_FULL |                   \
     COSPI_REG_IRQ_IND_COMP)

#define COSPI_IRQ_MASK_WR                                                      \
    (COSPI_REG_IRQ_IND_COMP | COSPI_REG_IRQ_WATERMARK | COSPI_REG_IRQ_UNDERFLOW)

#define COSPI_IRQ_STATUS_MASK (0x1FFFF | COSPI_REG_IRQ_ECC_ERR)

#define INDIRECT_READ_FLAG 0
#define INDIRECT_WRITE_FLAG 1

#define COSPI_IO_PAD_PULL_ENBALE BIT(0)
#define COSPI_IO_PAD_PULL_UP_SEL BIT(1)
#define COSPI_IO_PAD_DRIVER_SEL_LSB (4)
#define COSPI_IO_PAD_DRIVER_SEL_MASK (0x3)
#define COSPI_IO_PAD_SLEW_RATE BIT(8)
#define COSPI_IO_PAD_INPUT_SEL BIT(12)
#define COSPI_IO_PAD_PULL_POE BIT(16)

#define COSPI_IO_PAD_PULL_UP                                                   \
    ((0x1 & COSPI_IO_PAD_DRIVER_SEL_MASK) << COSPI_IO_PAD_DRIVER_SEL_LSB) |    \
        COSPI_IO_PAD_PULL_ENBALE | COSPI_IO_PAD_PULL_UP_SEL |                  \
        COSPI_IO_PAD_INPUT_SEL

#define COSPI_IO_PAD_PULL_DOWN                                                 \
    ((0x1 & COSPI_IO_PAD_DRIVER_SEL_MASK) << COSPI_IO_PAD_DRIVER_SEL_LSB) |    \
        COSPI_IO_PAD_PULL_ENBALE | COSPI_IO_PAD_INPUT_SEL

#define COSPI_IO_PAD_PULL_DISABLE                                              \
    ((0x1 & COSPI_IO_PAD_DRIVER_SEL_MASK) << COSPI_IO_PAD_DRIVER_SEL_LSB) |    \
        COSPI_IO_PAD_INPUT_SEL

uint32_t g_dummy = 0;

static struct cospi_pdata s_cospi;

static inline void readsl(uint32_t *addr, uint32_t *data, int len)
{
    while (len--)
        *data++ = readl(addr++);
}

static inline void readsb(uint8_t *addr, uint8_t *data, int len)
{
    while (len--)
        *data++ = readb(addr++);
}

static inline void writesl(uint32_t *addr, uint32_t *data, int len)
{
    while (len--)
        writel(*data++, addr++);
}

static void cospi_enable(struct cospi_pdata *cospi, bool enable)
{
    u32 reg;
    reg = ospi_readl(cospi->apb_base + COSPI_REG_CONFIG);

    if (enable)
        reg |= COSPI_REG_CONFIG_ENABLE_MASK;
    else
        reg &= ~COSPI_REG_CONFIG_ENABLE_MASK;

    ospi_writel(reg, cospi->apb_base + COSPI_REG_CONFIG);
}

#define mdelay(x) spin(x * 1000)

static u32 cospi_idle_status(struct cospi_pdata *cospi)
{
    u32 reg;
    reg = readl(cospi->apb_base + COSPI_REG_CONFIG);
    return reg & COSPI_REG_CONFIG_IDLE_MASK;
}

static int cospi_wait_for_bit(addr_t reg, const u32 mask, bool clear, u32 time)
{
    u32 val;
    lk_time_t timeout;

    timeout = current_time() + time;

    while (1) {
        val = readl(reg);
        if (clear)
            val = ~val;
        val &= mask;

        if (val == mask)
            return 0;

        if (current_time() > timeout)
            return -ETIMEDOUT;
        udelay(1);
    }
    return -1;
}

static int cospi_wait_for_bit_times(addr_t reg, const u32 mask, bool clear, u32 times)
{
    u32 val;
    u32 count = 0;

    while (count < times) {
        val = readl(reg);
        if (clear)
            val = ~val;
        val &= mask;

        if (val == mask) {
            return 0;
        }
        count++;
    }
    return -1;
}

static u32 cospi_get_rd_sram_level(struct cospi_pdata *cospi)
{
    u32 reg = ospi_readl(cospi->apb_base + COSPI_REG_SDRAMLEVEL);
    reg >>= COSPI_REG_SDRAMLEVEL_RD_LSB;
    return reg & COSPI_REG_SDRAMLEVEL_RD_MASK;
}

static int cospi_check_rd_data(struct cospi_pdata *cospi)
{
    u32 val;
    lk_time_t timeout;

    timeout = current_time() + COSPI_READ_TIMEOUT_MS;

    while (1) {
        val = cospi_get_rd_sram_level(cospi);
        if (val)
            return val;
        if (current_time() > timeout)
            return -ETIMEDOUT;
        udelay(1);
    }
}

static int cospi_wait_idle(struct cospi_pdata *cospi)
{
    unsigned int idle_latency_cycles = 4;
    unsigned int count = 0;
    lk_time_t timeout;

    timeout = current_time() + COSPI_TIMEOUT_MS;
    while (1) {
        if (cospi_idle_status(cospi))
            count++;
        else
            count = 0;

        /* When IDLE bit asserted, need wait 4 cycyles of ref_clk. */
        if (count >= idle_latency_cycles)
            return 0;

        if (current_time() > timeout) {
            dprintf(CRITICAL, "Wait ospi idle time out!\n");
            return -1;
        }
        udelay(1);
    }
}

static void cospi_size_set(struct spi_nor *nor)
{
    u32 reg = 0;
    struct cospi_pdata *cospi = nor->priv_data;
    cospi->block_power_index = log2_uint(nor->block_size);
    cospi->page_size = nor->page_size;

    // TODO: for support direct access ahb decode
#if 0
    reg = ospi_readl(cospi->apb_base + COSPI_REG_SIZE);

    /* Config spi flash device size */
    reg &= ~(0x3 << (COSPI_REG_SIZE_SIZE_LSB + 2 * nor->cs));
    reg |= ((u64)nor->size << (COSPI_REG_SIZE_SIZE_LSB + 2 * nor->cs));
#endif
    reg = (cospi->block_power_index << COSPI_REG_SIZE_BLOCK_LSB);
    reg |= (cospi->page_size << COSPI_REG_SIZE_PAGE_LSB);

    ospi_writel(reg, cospi->apb_base + COSPI_REG_SIZE);

    /* The size set opt will clear the addr bytes config data */
    cospi->addr_bytes = 0;
}

static void cospi_cs_set(struct spi_nor *nor)
{
    u32 reg;
    u8 cs = nor->cs;
    struct cospi_pdata *cospi = nor->priv_data;

    cospi_wait_idle(cospi);

    reg = ospi_readl(cospi->apb_base + COSPI_REG_CONFIG);

    if (cospi->cs_decoded) {
        reg |= COSPI_REG_CONFIG_DECODE_MASK;
    }
    else {
        reg &= ~COSPI_REG_CONFIG_DECODE_MASK;
        cs = ~(1 << cs);
    }

    /* Set DTR protocol enable or not */
    reg &= ~COSPI_REG_CONFIG_DTR_ENABLE_MASK;
    reg |= (nor->dtr_en << COSPI_REG_CONFIG_DTR_ENABLE_LSB);

    reg &=
        ~(COSPI_REG_CONFIG_CHIPSELECT_MASK << COSPI_REG_CONFIG_CHIPSELECT_LSB);
    reg |= (cs & COSPI_REG_CONFIG_CHIPSELECT_MASK)
           << COSPI_REG_CONFIG_CHIPSELECT_LSB;

    ospi_writel(reg, cospi->apb_base + COSPI_REG_CONFIG);
}

void cospi_baudrate_set(struct spi_nor *nor)
{
    u32 reg, div;
    struct cospi_pdata *cospi = nor->priv_data;

    /* Recalculate the baudrate divisor based on OSPI specification. */
    div = DIV_ROUND_UP(cospi->ref_clk_hz, 2 * cospi->sclk) - 1;

    reg = ospi_readl(cospi->apb_base + COSPI_REG_CONFIG);
    reg &= ~(COSPI_REG_CONFIG_BAUD_MASK << COSPI_REG_CONFIG_BAUD_LSB);
    reg |= (div & COSPI_REG_CONFIG_BAUD_MASK) << COSPI_REG_CONFIG_BAUD_LSB;
    cospi_wait_idle(cospi);
    ospi_writel(reg, cospi->apb_base + COSPI_REG_CONFIG);
}

static void cospi_delay_set(struct spi_nor *nor)
{
    u32 reg, cssot, cseot, csdads, csda;
    struct cospi_pdata *cospi = nor->priv_data;

    cssot = OSPI_NS_2_TICKS(cospi->ref_clk_hz, nor->cssot_ns);
    cseot = OSPI_NS_2_TICKS(cospi->ref_clk_hz, nor->cseot_ns);
    csdads = OSPI_NS_2_TICKS(cospi->ref_clk_hz, nor->csdads_ns);
    csda = OSPI_NS_2_TICKS(cospi->ref_clk_hz, nor->csda_ns);

    reg = (cssot & COSPI_REG_DELAY_CSSOT_MASK) << COSPI_REG_DELAY_CSSOT_LSB;
    reg |= (cseot & COSPI_REG_DELAY_CSEOT_MASK) << COSPI_REG_DELAY_CSEOT_LSB;
    reg |= (csdads & COSPI_REG_DELAY_CSDADS_MASK) << COSPI_REG_DELAY_CSDADS_LSB;
    reg |= (csda & COSPI_REG_DELAY_CSDA_MASK) << COSPI_REG_DELAY_CSDA_LSB;
    ospi_writel(reg, cospi->apb_base + COSPI_REG_DELAY);
}

static void cospi_capture_set(struct spi_nor *nor)
{
    u32 reg;
    struct cospi_pdata *cospi = nor->priv_data;

    if (nor->dtr_en) {
        cospi->master_delay = 2;
    }
    else {
        cospi->master_delay = 0;
    }
    cospi->capture_delay = cospi->master_delay +
                            COSPI_READCAPTURE_DELAY_DEFAULT;

    if (nor->phy_en) {
        cospi->master_delay = 0;
        cospi->capture_delay = 0;
    }

    ospi_writel(((nor->status_dummy + g_dummy) & COSPI_REG_FLASH_STATUS_DUMMY_MASK) <<
                  COSPI_REG_FLASH_STATUS_DUMMY_LSB,
                cospi->apb_base + COSPI_REG_FLASH_STATUS);

    reg = ospi_readl(cospi->apb_base + COSPI_REG_READCAPTURE);

    if (cospi->rclk_loopback)
        reg &= ~(1 << COSPI_REG_READCAPTURE_BYPASS_LSB);
    else
        reg |= (1 << COSPI_REG_READCAPTURE_BYPASS_LSB);

    if (cospi->dqs_en)
        reg |= (1 << COSPI_REG_READCAPTURE_DQSEN_LSB);
    else
        reg &= ~(1 << COSPI_REG_READCAPTURE_DQSEN_LSB);

    reg &=
        ~(COSPI_REG_READCAPTURE_DELAY_MASK << COSPI_REG_READCAPTURE_DELAY_LSB);

    reg |= ((cospi->capture_delay & COSPI_REG_READCAPTURE_DELAY_MASK)
            << COSPI_REG_READCAPTURE_DELAY_LSB);

    reg &=
        ~(COSPI_REG_READCAPTURE_MASTER_DELAY_MASK << COSPI_REG_READCAPTURE_MASTER_DELAY_LSB);

    reg |= ((cospi->master_delay & COSPI_REG_READCAPTURE_MASTER_DELAY_MASK)
            << COSPI_REG_READCAPTURE_MASTER_DELAY_LSB);

    ospi_writel(reg, cospi->apb_base + COSPI_REG_READCAPTURE);
}

static void cospi_addr_bytes_set(struct spi_nor *nor)
{
    u32 reg;
    struct cospi_pdata *cospi = nor->priv_data;

    reg = ospi_readl(cospi->apb_base + COSPI_REG_SIZE);
    /* Config spi flash device addr bytes */
    reg &= ~(COSPI_REG_SIZE_ADDRESS_MASK << COSPI_REG_SIZE_ADDRESS_LSB);
    if (cospi->addr_bytes > 1)
        reg |= ((cospi->addr_bytes - 1) << COSPI_REG_SIZE_ADDRESS_LSB);
    else
        reg |= 0 << COSPI_REG_SIZE_ADDRESS_LSB;

    ospi_writel(reg, cospi->apb_base + COSPI_REG_SIZE);
}

/* all read ops, write ops, stig ops use the read inst reg's inst type */
static inline void cospi_inst_width_set(struct cospi_pdata *cospi,
                                        uint8_t inst_width)
{
    uint32_t reg;

    reg = ospi_readl(cospi->apb_base + COSPI_REG_RD_INSTR);
    reg &= ~(COSPI_REG_RD_INSTR_TYPE_INST_MASK
             << COSPI_REG_RD_INSTR_TYPE_INST_LSB);
    reg |= (uint32_t)inst_width << COSPI_REG_RD_INSTR_TYPE_INST_LSB;
    ospi_writel(reg, cospi->apb_base + COSPI_REG_RD_INSTR);
}

static void cospi_read_setup(struct spi_nor *nor, struct spi_nor_cmd *cmd)
{
    u32 reg;
    struct cospi_pdata *cospi = nor->priv_data;

    if (cmd == NULL)
        return;

    /* The addr_bytes may be changed by other flash operation */
    if (cospi->addr_bytes != cmd->addr_bytes) {
        cospi->addr_bytes = cmd->addr_bytes;
        cospi_addr_bytes_set(nor);
    }

    if (nor->dtr_en) {
        cmd->inst_width = SPI_NOR_OCTAL_LANS;
        cmd->addr_width = SPI_NOR_OCTAL_LANS;
        cmd->data_width = SPI_NOR_OCTAL_LANS;
    }

    reg = cmd->opcode << COSPI_REG_RD_INSTR_OPCODE_LSB;
    reg |= cmd->inst_width << COSPI_REG_RD_INSTR_TYPE_INST_LSB;
    reg |= cmd->addr_width << COSPI_REG_RD_INSTR_TYPE_ADDR_LSB;
    reg |= cmd->data_width << COSPI_REG_RD_INSTR_TYPE_DATA_LSB;
    reg |= cmd->ddr_en << COSPI_REG_RD_INSTR_DDR_EN;

    if (nor->global_read_dummy)
        reg |= ((nor->global_read_dummy + g_dummy) & COSPI_REG_RD_INSTR_DUMMY_MASK)
               << COSPI_REG_RD_INSTR_DUMMY_LSB;
    else
        reg |= ((cmd->dummy + g_dummy) & COSPI_REG_RD_INSTR_DUMMY_MASK)
            << COSPI_REG_RD_INSTR_DUMMY_LSB;

    ospi_writel(reg, cospi->apb_base + COSPI_REG_RD_INSTR);
}

static void cospi_write_setup(struct spi_nor *nor, struct spi_nor_cmd *cmd)
{
    u32 reg;
    struct cospi_pdata *cospi = nor->priv_data;

    if (cmd == NULL)
        return;

    if (cospi->addr_bytes != cmd->addr_bytes) {
        cospi->addr_bytes = cmd->addr_bytes;
        cospi_addr_bytes_set(nor);
    }

    cospi_inst_width_set(cospi, cmd->inst_width);

    if (nor->dtr_en) {
        cmd->addr_width = SPI_NOR_OCTAL_LANS;
        cmd->data_width = SPI_NOR_OCTAL_LANS;
    }

    reg = cmd->opcode << COSPI_REG_WR_INSTR_OPCODE_LSB;
    reg |= (cmd->addr_width << COSPI_REG_WR_INSTR_TYPE_ADDR_LSB);
    reg |= (cmd->data_width << COSPI_REG_WR_INSTR_TYPE_DATA_LSB);
    ospi_writel(reg, cospi->apb_base + COSPI_REG_WR_INSTR);
}

static int cospi_exec_flash_cmd(struct cospi_pdata *cospi, unsigned int reg)
{
    dprintf(INFO, "cospi_exec_flash_cmd in.\n");
    int ret;

    /* Write the CMDCTRL without start execution. */
    ospi_writel(reg, cospi->apb_base + COSPI_REG_CMDCTRL);
    /* Start execute */
    reg |= COSPI_REG_CMDCTRL_EXECUTE_MASK;
    ospi_writel(reg, cospi->apb_base + COSPI_REG_CMDCTRL);

    /* Polling for completion. */
    ret = cospi_wait_for_bit(cospi->apb_base + COSPI_REG_CMDCTRL,
                             COSPI_REG_CMDCTRL_INPROGRESS_MASK, 1, 100);
    if (ret) {
        dprintf(CRITICAL, "Flash command execution timed out.\n");
        return ret;
    }

    dprintf(INFO, "cospi_exec_flash_cmd out.\n");
    /* Polling OSPI idle status. */
    return cospi_wait_idle(cospi);
}

static int cospi_exec_flash_cmd_ext(struct cospi_pdata *cospi, unsigned int reg,
                                    unsigned int ext_reg, u8 *buf, u32 len)
{
    u32 i;
    u32 tmp_reg;

    dprintf(INFO, "cospi_exec_flash_cmd_ext in.\n");
    int ret;

    /* Write the CMDCTRL without start execution. */
    ospi_writel(reg, cospi->apb_base + COSPI_REG_CMDCTRL);

    /* Write the CMDCTRL_MEM without start request. */
    ospi_writel(ext_reg, cospi->apb_base + COSPI_REG_CMDCTRL_MEM);

    /* Start execute */
    reg |= COSPI_REG_CMDCTRL_MEM_EXECUTE_MASK;
    ospi_writel(reg, cospi->apb_base + COSPI_REG_CMDCTRL);

    /* Polling for completion. */
    ret = cospi_wait_for_bit(cospi->apb_base + COSPI_REG_CMDCTRL,
                             COSPI_REG_CMDCTRL_INPROGRESS_MASK, 1, 100);

    if (ret) {
        dprintf(CRITICAL, "Flash command execution timed out.\n");
        return ret;
    }

    for (i = 0; i < len; i++) {
        tmp_reg = (i << COSPI_REG_CTDCTRL_MEM_ADDR_LSB);
        ospi_writel(tmp_reg, cospi->apb_base + COSPI_REG_CMDCTRL_MEM);

        tmp_reg |= COSPI_REG_CMDCTRL_MEM_EXECUTE_MASK;
        ospi_writel(tmp_reg, cospi->apb_base + COSPI_REG_CMDCTRL_MEM);
        /* Polling for completion. */
        ret = cospi_wait_for_bit(cospi->apb_base + COSPI_REG_CMDCTRL_MEM,
                                 COSPI_REG_CMDCTRL_MEM_INPROGRESS_MASK, 1, 10);
        if (ret) {
            dprintf(CRITICAL, "Membank request timed out.\n");
            return ret;
        }

        *(buf + i) = readb(cospi->apb_base + COSPI_REG_CTDCTRL_MEM_DATA);
    }
    dprintf(INFO, "cospi_exec_flash_cmd_ext out.\n");

    /* disable membank */
    ospi_writel(0, cospi->apb_base + COSPI_REG_CMDCTRL);
    /* Polling OSPI idle status. */
    return cospi_wait_idle(cospi);
}

int cospi_command_read(struct spi_nor *nor, struct spi_nor_cmd *cmd, u8 *buf,
                       const unsigned len)
{
    struct cospi_pdata *cospi = nor->priv_data;
    addr_t reg_base = cospi->apb_base;
    unsigned int reg, ext_reg;
    unsigned int read_len;
    unsigned int index = 0;
    unsigned int mem_bank_len = 16;
    int status;

    if (cospi_wait_idle(cospi)) {
        dprintf(CRITICAL, "Wait ospi idle time out!\n");
        return -1;
    }

    if (!len || len > COSPI_STIG_DATA_LEN_MAX || !buf) {
        dprintf(CRITICAL, "Invalid input argument, len %d rxbuf 0x%p\n", len, buf);
        return -EINVAL;
    }

    cospi_inst_width_set(cospi, cmd->inst_width);

    reg = cmd->opcode << COSPI_REG_CMDCTRL_OPCODE_LSB;
    if (cmd->addr_bytes) {
        reg |= (0x1 << COSPI_REG_CMDCTRL_ADDR_EN_LSB);
        reg |= ((cmd->addr_bytes - 1) & COSPI_REG_CMDCTRL_ADD_BYTES_MASK)
               << COSPI_REG_CMDCTRL_ADD_BYTES_LSB;

        ospi_writel(cmd->addr, cospi->apb_base + COSPI_REG_CMDADDRESS);
    }

    reg |= (0x1 << COSPI_REG_CMDCTRL_RD_EN_LSB);

    /* set dummy */
    reg |= ((cmd->dummy + g_dummy) & COSPI_REG_CMDCTRL_WR_DUMMY_MASK)
           << COSPI_REG_CMDCTRL_WR_DUMMY_LSB;
    if (len <= 8) {
        /* 0 means 1 byte. */
        reg |= (((len - 1) & COSPI_REG_CMDCTRL_RD_BYTES_MASK)
                << COSPI_REG_CMDCTRL_RD_BYTES_LSB);
        status = cospi_exec_flash_cmd(cospi, reg);
        if (status)
            return status;
        reg = ospi_readl(reg_base + COSPI_REG_CMDREADDATALOWER);

        /* Put the read value into rx_buf */
        read_len = (len > 4) ? 4 : len;
        memcpy(buf, &reg, read_len);
        buf += read_len;

        if (len > 4) {
            reg = ospi_readl(reg_base + COSPI_REG_CMDREADDATAUPPER);

            read_len = len - 4;
            memcpy(buf, &reg, read_len);
        }
    }
    else {
        reg |= COSPI_REG_CMDCTRL_MEMBANK_EN_MASK;
        while (len > mem_bank_len) {
            index++;
            mem_bank_len *= (1U << index);
        }
        ext_reg = (index << COSPI_REG_CTDCTRL_MEM_LEN_LSB);

        status = cospi_exec_flash_cmd_ext(cospi, reg, ext_reg, buf, len);
        reg = ospi_readl(reg_base + COSPI_REG_CMDREADDATALOWER);
        reg = ospi_readl(reg_base + COSPI_REG_CMDREADDATAUPPER);
        if (status)
            return status;
    }

    return 0;
}

int cospi_command_write(struct spi_nor *nor, struct spi_nor_cmd *cmd,
                        const u8 *buf, const unsigned len)
{
    dprintf(INFO, "cospi_command_write in.\n");
    struct cospi_pdata *cospi = nor->priv_data;
    unsigned int reg;
    unsigned int data;
    int ret;

    if (cospi_wait_idle(cospi)) {
        dprintf(CRITICAL, "Wait ospi idle time out!\n");
        return -1;
    }

    if (len > 4 || (len && !buf)) {
        dprintf(CRITICAL, "Invalid input argument, len %d buf 0x%p\n", len, buf);
        return EINVAL;
    }

    cospi_inst_width_set(cospi, cmd->inst_width);

    reg = cmd->opcode << COSPI_REG_CMDCTRL_OPCODE_LSB;
    if (cmd->addr_bytes) {
        reg |= (0x1 << COSPI_REG_CMDCTRL_ADDR_EN_LSB);
        reg |= ((cmd->addr_bytes - 1) & COSPI_REG_CMDCTRL_ADD_BYTES_MASK)
               << COSPI_REG_CMDCTRL_ADD_BYTES_LSB;

        ospi_writel(cmd->addr, cospi->apb_base + COSPI_REG_CMDADDRESS);
    }

    if (len) {
        reg |= (0x1 << COSPI_REG_CMDCTRL_WR_EN_LSB);
        reg |= ((len - 1) & COSPI_REG_CMDCTRL_WR_BYTES_MASK)
               << COSPI_REG_CMDCTRL_WR_BYTES_LSB;
        data = 0;
        memcpy(&data, buf, len);
        ospi_writel(data, cospi->apb_base + COSPI_REG_CMDWRITEDATALOWER);
    }

    reg |= (cmd->dummy & COSPI_REG_CMDCTRL_WR_DUMMY_MASK)
           << COSPI_REG_CMDCTRL_WR_DUMMY_LSB;

    ret = cospi_exec_flash_cmd(cospi, reg);
    dprintf(INFO, "cospi_command_write out.\n");
    return ret;
}

static int cospi_stig_erase(struct spi_nor *nor, struct spi_nor_cmd *cmd)
{
    dprintf(INFO, "cospi_stig_erase in.\n");

    int ret;

    struct spi_nor_cmd write_enable_cmd = {
        .opcode = 6,
        .addr_bytes = SPI_NOR_ADDR_0_BYTES,
        /* the write enable cmd inst_width need aline with erase cmd */
        .inst_width = cmd->inst_width,
    };

    ret = cospi_command_write(nor, &write_enable_cmd, NULL, 0);
    if (ret)
        return ret;

    dprintf(INFO, "cospi_stig_erase out.\n");
    return cospi_command_write(nor, cmd, NULL, 0);
}



static void cospi_dma_enable(struct cospi_pdata *cospi, bool enable)
{
    u32 reg;

    cospi_wait_idle(cospi);

    reg = ospi_readl(cospi->apb_base + COSPI_REG_CONFIG);
    reg &= ~COSPI_REG_CONFIG_DMA_MASK;
    reg |= enable << COSPI_REG_CONFIG_DMA_LSB;
    cospi_wait_idle(cospi);
    ospi_writel(reg, cospi->apb_base + COSPI_REG_CONFIG);
}

static int cospi_rx_complete(struct spi_nor *nor, u32 err)
{
    u32 ret = 0;
    u32 indirect_read_done_mask = COSPI_REG_INDIRECTRD_DONE_MASK;
    struct cospi_pdata *cospi = nor->priv_data;
    u32 sram_full = 0;

    if (err) {
        ret = -1;
        goto terminate_rx;
    }

    ret = ospi_readl(cospi->apb_base + COSPI_REG_INDIRECTRD);
    if (!ret)
        return 0;

    sram_full = ret & COSPI_REG_INDIRECTRD_SRAM_FULL_MASK;

    if (nor->data_cmd.queue_mode_en)
        indirect_read_done_mask |= COSPI_REG_INDIRECTRD_Q_DONE_MASK;

    ret = cospi_wait_for_bit_times(cospi->apb_base + COSPI_REG_INDIRECTRD,
                             COSPI_REG_INDIRECTRD_DONE_MASK, 0, 100);
    if (ret) {
        goto terminate_rx;
    }

terminate_rx:
    if (ret) {
        /* Cancel the indirect read */
        ospi_writel(COSPI_REG_INDIRECTRD_CANCEL_MASK,
                    cospi->apb_base + COSPI_REG_INDIRECTRD);
    }

    /* Clear indirect completion status */
    ospi_writel(COSPI_REG_INDIRECTRD_DONE_MASK | sram_full,
                cospi->apb_base + COSPI_REG_INDIRECTRD);
    if (nor->data_cmd.queue_mode_en) {
        /* Clear indirect completion status twice */
        ospi_writel(COSPI_REG_INDIRECTRD_DONE_MASK,
                    cospi->apb_base + COSPI_REG_INDIRECTRD);
    }

    cospi_dma_enable(cospi, 0);

    nor->data_error = ret;
    event_signal(&cospi->complete_event, true);

    return ret;
}

static int cospi_tx_complete(struct spi_nor *nor, u32 err)
{
    u32 ret = 0;
    struct cospi_pdata *cospi = nor->priv_data;

    if (err) {
        ret = -1;
        goto terminate_tx;
    }

    ret = ospi_readl(cospi->apb_base + COSPI_REG_INDIRECTWR);
    if (!ret)
        return 0;

    ret = cospi_wait_for_bit_times(cospi->apb_base + COSPI_REG_INDIRECTWR,
                             COSPI_REG_INDIRECTWR_DONE_MASK, 0, 100);

terminate_tx:
    if (ret) {
        /* Cancel the indirect read */
        ospi_writel(COSPI_REG_INDIRECTWR_CANCEL_MASK,
                    cospi->apb_base + COSPI_REG_INDIRECTWR);
    }
    /* Clear indirect completion status */
    ospi_writel(COSPI_REG_INDIRECTWR_DONE_MASK,
                cospi->apb_base + COSPI_REG_INDIRECTWR);

    cospi_dma_enable(cospi, 0);

    nor->data_error = ret;
    event_signal(&cospi->complete_event, true);

    return ret;
}

static void cospi_tx_irq_handle(enum dma_status status, u32 err, void *context)
{
    struct spi_nor *nor = context;
    struct cospi_pdata *cospi = nor->priv_data;

    if (!status || err) {
        dprintf(INFO, "tx status complet!\n");
        event_signal(&cospi->dma_event, false);
    }
    dprintf(INFO, "tx status %d (0x%x) \n", status, err);
}

static void cospi_rx_irq_handle(enum dma_status status, u32 err, void *context)
{
    struct spi_nor *nor = context;
    struct cospi_pdata *cospi = nor->priv_data;

    if (!status || err) {
        dprintf(INFO, "rx status complet!\n");
        event_signal(&cospi->dma_event, false);
    }
    dprintf(INFO, "rx status %d (0x%x) \n", status, err);
}

static int cospi_dma_trans_setup(struct spi_nor *nor, struct dma_dev_cfg *cfg,
                                 u8 *buf, size_t len, bool use_queue_mode)
{
    struct cospi_pdata *cospi = nor->priv_data;

    enum dma_chan_tr_type ch_type;

    if (cospi->id == 1)
        ch_type = DMA_PERI_OSPI1;
    else
        ch_type = DMA_PERI_OSPI2;

    cospi->dma_chan = hal_dma_chan_req(ch_type);
    if (!cospi->dma_chan) {
        dprintf(CRITICAL, "dma chanel request failed!\n");
        return -1;
    }

    hal_dma_dev_config(cospi->dma_chan, cfg);

    if (use_queue_mode)
        cospi->dma_desc = hal_prep_dma_cyclic(cospi->dma_chan, buf,
                                              len * 2, len, DMA_INTERRUPT);
    else
        cospi->dma_desc =
            hal_prep_dma_dev(cospi->dma_chan, buf, len, DMA_INTERRUPT);

    cospi->dma_desc->context = nor;

    if (cfg->direction == DMA_DEV2MEM) {
        cospi->dma_desc->dmac_irq_evt_handle = cospi_rx_irq_handle;
    }
    else {
        cospi->dma_desc->dmac_irq_evt_handle = cospi_tx_irq_handle;
    }

    hal_dma_submit(cospi->dma_desc);

    return 0;
}

static void cospi_dma_config(struct cospi_pdata *cospi, struct dma_dev_cfg *cfg,
                             struct spi_nor_cmd *cmd, u8 *buf, size_t len)
{
    u32 reg;
    unsigned int transfer_width;
    unsigned int remaining_size;
    unsigned int burst_size;
    unsigned int burst_size_index = 8;
    unsigned int single_size_index = 8;
    const unsigned int burst_size_start = 256;
    unsigned int max_burst;

    /* Enable DMA peripheral interface */
    cospi_dma_enable(cospi, 1);

    if (len % 4) {
        transfer_width = DMA_DEV_BUSWIDTH_1_BYTE;
    }
    else if (len % 8) {
        transfer_width = DMA_DEV_BUSWIDTH_4_BYTES;
    }
    else {
        transfer_width = DMA_DEV_BUSWIDTH_4_BYTES;
    }

    burst_size = burst_size_start;

    /* Caculate the burst type request bytes number */
    while ((burst_size > len) ||
           (burst_size > cospi->fifo_width * cospi->fifo_depth)) {
        burst_size /= 2;
        burst_size_index -= 1;
    }

    max_burst = burst_size / transfer_width;
    if (max_burst >= 4) {
        max_burst = log2_uint(max_burst) - 1;
        remaining_size = len % burst_size;
    }
    else {
        max_burst = DMA_BURST_TR_1ITEM;
        if (transfer_width == DMA_DEV_BUSWIDTH_4_BYTES) {
            burst_size = 4;
            burst_size_index = 2;
        }
        else if (transfer_width == DMA_DEV_BUSWIDTH_1_BYTE) {
            burst_size = 1;
            burst_size_index = 0;
        }

        remaining_size = len;
    }

    if ((transfer_width >= DMA_DEV_BUSWIDTH_4_BYTES) && (remaining_size >= 4)) {
        single_size_index = 2;
    }
    else {
        single_size_index = 0;
    }

    /* Set burst bytes size and single bytes size */
    reg = ((single_size_index & COSPI_REG_DMA_SINGLE_MASK)
           << COSPI_REG_DMA_SINGLE_LSB);

    reg |= ((burst_size_index & COSPI_REG_DMA_BURST_MASK)
            << COSPI_REG_DMA_BURST_LSB);

    ospi_writel(reg, cospi->apb_base + COSPI_REG_DMA);

    if (cmd->type == SPI_NOR_OPS_READ) {
        cfg->direction = DMA_DEV2MEM;
        cfg->src_addr = _paddr((void *)cospi->ahb_base) + cospi->trigger_address;

        /* Indirect read watermark set with burst size */
        ospi_writel(burst_size,
                    cospi->apb_base + COSPI_REG_INDIRECTRDWATERMARK);
    }
    else {
        cfg->direction = DMA_MEM2DEV;
        cfg->dst_addr = _paddr((void *)cospi->ahb_base) + cospi->trigger_address;

        /*
         * Indirect write watermark set,
         * the num add with burst size equal to SRAM tx fifo size.
         */
        ospi_writel(cospi->fifo_depth * cospi->fifo_width - burst_size,
                    cospi->apb_base + COSPI_REG_INDIRECTWRWATERMARK);
    }

    cfg->src_addr_width = transfer_width;
    cfg->dst_addr_width = transfer_width;

    cfg->src_maxburst = max_burst;
    cfg->dst_maxburst = max_burst;
}

static inline void cospi_indirect_trigger(const struct cospi_pdata *cospi,
                                          u32 addr, u32 size, int flag)
{
    if (flag == INDIRECT_READ_FLAG) {
        /* Set indirect read start address */
        ospi_writel(addr, cospi->apb_base + COSPI_REG_INDIRECTRDSTARTADDR);
        /* Set indirect read bytes number */
        ospi_writel(size, cospi->apb_base + COSPI_REG_INDIRECTRDBYTES);
        /* Start the indirect read */
        ospi_writel(COSPI_REG_INDIRECTRD_START_MASK,
                    cospi->apb_base + COSPI_REG_INDIRECTRD);
        udelay(1);
    }
    else {
        /* Set indirect write start address */
        ospi_writel(addr, cospi->apb_base + COSPI_REG_INDIRECTWRSTARTADDR);
        /* Set indirect write bytes number */
        ospi_writel(size, cospi->apb_base + COSPI_REG_INDIRECTWRBYTES);
        /* Start the indirect write */
        ospi_writel(COSPI_REG_INDIRECTWR_START_MASK,
                    cospi->apb_base + COSPI_REG_INDIRECTWR);
        /*
         * There need delay before write data to sram.
         * Note: The delay time will test in real chip.
         */
        udelay(1);
    }
}

static int cospi_indirect_read(struct spi_nor *nor, struct spi_nor_cmd *cmd,
                               u8 *buf, u32 size)
{
    int ret;
    u32 trgger_size;
    s64 remaining = size;
    u32 words_need_read = 0;
    u8 *read_buf = buf;
    struct dma_dev_cfg dma_cfg;
    struct cospi_pdata *cospi = nor->priv_data;
    addr_t ahb_trigger_address = cospi->ahb_base + cospi->trigger_address;

    if (buf == NULL || size == 0)
        return EINVAL;

    if (cmd->queue_mode_en)
        trgger_size = size / 2;
    else
        trgger_size = size;

    if (cmd->use_dma && COSPI_DMA_ENABLE) {
        cospi_dma_config(cospi, &dma_cfg, cmd, buf, (size_t)trgger_size);

        ret = cospi_dma_trans_setup(nor, &dma_cfg, buf, (size_t)trgger_size,
                                    cmd->queue_mode_en);
        if (ret)
            return -1;
    }

    cospi_indirect_trigger(cospi, cmd->addr, trgger_size, INDIRECT_READ_FLAG);

    if (cmd->queue_mode_en) {
        cospi_indirect_trigger(cospi, cmd->addr + trgger_size, trgger_size,
                               INDIRECT_READ_FLAG);
    }

    if (cmd->use_dma && COSPI_DMA_ENABLE) {
        event_wait(&cospi->dma_event);
        if (cospi->dma_desc) {
            hal_dma_free_desc(cospi->dma_desc);
            cospi->dma_desc = NULL;
        }

        event_wait(&cospi->complete_event);
        return nor->data_error;
    }

    while (remaining > 0) {
        ret = cospi_check_rd_data(cospi);
        if (ret <= 0) {
            break;
        }

        words_need_read = ret;
        while (words_need_read) {
            /* read can't exceed the indirect trigger range */
            words_need_read = MIN(words_need_read,
                                  cospi->trigger_range_size / cospi->fifo_width);

            dprintf(INFO, "cospi words_need_read = %d\n", words_need_read);

            if (words_need_read) {
                arch_invalidate_cache_range(ahb_trigger_address,
                                            words_need_read);

                readsl((uint32_t *)ahb_trigger_address,
                       (uint32_t *)read_buf, words_need_read);

                read_buf += words_need_read * cospi->fifo_width;
                remaining -= words_need_read * cospi->fifo_width;

                if (remaining < 0)
                    break;
            }

            words_need_read = cospi_get_rd_sram_level(cospi);
        }
    }

    event_wait(&cospi->complete_event);
    return nor->data_error;
}

static int cospi_indirect_write(struct spi_nor *nor, struct spi_nor_cmd *cmd,
                                u8 *buf, u32 size)
{
    int ret;
    u32 remaining = size;
    u32 write_bytes;
    u8 *write_buf = buf;
    struct dma_dev_cfg dma_cfg;
    struct cospi_pdata *cospi = nor->priv_data;
    addr_t ahb_trigger_address = cospi->ahb_base + cospi->trigger_address;
    u32 addr_page_bonduary_size =
        nor->page_size * 2 - (cmd->addr & (nor->page_size - 1));
    u32 write_fifo_level_mask = COSPI_REG_SDRAMLEVEL_WR_MASK
                                << COSPI_REG_SDRAMLEVEL_WR_LSB;

    if (buf == NULL || size == 0)
        return EINVAL;

    if (cmd->use_dma && COSPI_DMA_ENABLE) {
        cospi_dma_config(cospi, &dma_cfg, cmd, buf, (size_t)size);

        ret = cospi_dma_trans_setup(nor, &dma_cfg, buf, (size_t)size, 0);
        if (ret)
            return -1;
    }

    cospi_indirect_trigger(cospi, cmd->addr, size, INDIRECT_WRITE_FLAG);

    if (cmd->use_dma && COSPI_DMA_ENABLE) {
        event_wait(&cospi->dma_event);
        if (cospi->dma_desc) {
            hal_dma_free_desc(cospi->dma_desc);
            cospi->dma_desc = NULL;
        }

        event_wait(&cospi->complete_event);
        return nor->data_error;
    }

    /* for cpu write, invalidate cache to avoid hitting */
    arch_invalidate_cache_range(ahb_trigger_address, cospi->trigger_range_size);

    while (remaining) {
        if (addr_page_bonduary_size) {
            write_bytes = MIN(remaining, addr_page_bonduary_size);
            addr_page_bonduary_size = 0;
        }
        else
            /* write can't exceed the page size */
            write_bytes = MIN(remaining, nor->page_size);

        writesl((uint32_t *)ahb_trigger_address, (uint32_t *)write_buf,
                DIV_ROUND_UP(write_bytes, cospi->fifo_width));

        write_buf += write_bytes;

        /* Check direct write sram empty status */
        ret = cospi_wait_for_bit(cospi->apb_base + COSPI_REG_SDRAMLEVEL,
                                 write_fifo_level_mask, 1, 1000);

        if (ret) {
            ret = ospi_readl(cospi->apb_base + 0xb0);
            dprintf(CRITICAL, "Indirect write timeout, ret = 0x%x, sram level 0x%08x.\n",
                    ret, readl(cospi->apb_base + COSPI_REG_SDRAMLEVEL));
            return -1;
        }

        remaining -= write_bytes;
    }

    event_wait(&cospi->complete_event);
    return nor->data_error;
}

static int cospi_do_xfer(struct spi_nor *nor, struct spi_nor_cmd *cmd)
{
    int ret = 0;
    struct cospi_pdata *cospi = nor->priv_data;
    uint32_t remaining = cmd->size;
    uint8_t *xfer_buf = (uint8_t *)cmd->buf;
    /* To prevent fifo overflow, set the max xfer length equel 128 * fifo_size. */
    const uint32_t max_xfer_length = cospi->fifo_depth * cospi->fifo_width * 128;
    uint32_t xfer_len;

    while(remaining) {
        if (nor->async_mode && nor->cancel_flag) {
            ret = -1;
            break;
        }

        xfer_len = MIN(remaining, max_xfer_length);

        switch (cmd->type) {
        case SPI_NOR_OPS_READ:
            ret = cospi_indirect_read(nor, &nor->data_cmd, xfer_buf, xfer_len);
            break;
        case SPI_NOR_OPS_WRITE:
            ret = cospi_indirect_write(nor, &nor->data_cmd, xfer_buf, xfer_len);
            break;
        case SPI_NOR_OPS_ERASE:
            break;
        default:
            ret = -1;
        }

        xfer_buf += xfer_len;
        nor->data_cmd.addr += xfer_len;
        remaining -= xfer_len;
    }

    nor->data_present = 0;
    return ret;
}


int cospi_xfer(struct spi_nor *nor, struct spi_nor_cmd *cmd, u8 *buf, u32 size)
{
    dprintf(INFO, "cospi_xfer in.\n");
    /*
     * For ip limit and the performance considerations,
     * we only support 32bit AHB access.
     */
    ASSERT(IS_ALIGNED(buf, 4));
    ASSERT(IS_ALIGNED(size, 4));

    int ret = 0;
    struct cospi_pdata *cospi = nor->priv_data;

    nor->cancel_flag = false;

    if (cmd->type == SPI_NOR_OPS_ERASE) {
        ret = cospi_stig_erase(nor, cmd);
        goto xfer_out;
    }

    if (cmd->type == SPI_NOR_OPS_READ || cmd->type == SPI_NOR_OPS_WRITE) {
        nor->data_present = 1;
        memcpy(&nor->data_cmd, cmd, sizeof(struct spi_nor_cmd));
        nor->data_cmd.buf = (addr_t)buf;
        nor->data_cmd.size = size;
        if (cmd->type == SPI_NOR_OPS_WRITE) {
            cospi_write_setup(nor, cmd);
        }
        else {
            cospi_read_setup(nor, cmd);
        }
    }

    /* Just for debug */
    ospi_readl(cospi->apb_base + COSPI_REG_FLASH_STATUS);

    event_signal(&cospi->xfer_start_event, false);

    if (nor->async_mode) {
        ret = 0;
    }
    else {
        event_wait(&cospi->xfer_done_event);
        ret = nor->data_error;
    }

xfer_out:
    dprintf(INFO, "cospi_xfer out.\n");
    return ret;
}

int cospi_cancel(struct spi_nor *nor)
{
    dprintf(INFO, "cospi_cancel in.\n");

    nor->cancel_flag = true;

    u32 reg;
    struct cospi_pdata *cospi = nor->priv_data;

    if (!nor->data_present)
        return 0;

    switch (nor->data_cmd.type) {
    case SPI_NOR_OPS_READ:
        if (nor->data_cmd.use_dma && COSPI_DMA_ENABLE) {
            if (cospi->dma_desc) {
                hal_dma_terminate(cospi->dma_desc);
            }
            udelay(1);
            cospi_dma_enable(cospi, 0);
        }

        ospi_writel(COSPI_REG_INDIRECTRD_CANCEL_MASK,
                    cospi->apb_base + COSPI_REG_INDIRECTRD);
        /* Clear indirect completion status */
        reg = ospi_readl(cospi->apb_base + COSPI_REG_INDIRECTRD);
        if (reg & COSPI_REG_INDIRECTRD_DONE_MASK)
            ospi_writel(COSPI_REG_INDIRECTRD_DONE_MASK,
                        cospi->apb_base + COSPI_REG_INDIRECTRD);
        break;
    case SPI_NOR_OPS_WRITE:
        if (nor->data_cmd.use_dma && COSPI_DMA_ENABLE) {
            if (cospi->dma_desc) {
                hal_dma_terminate(cospi->dma_desc);
            }
            udelay(1);
            cospi_dma_enable(cospi, 0);
        }

        ospi_writel1(COSPI_REG_INDIRECTWR_CANCEL_MASK,
                    cospi->apb_base + COSPI_REG_INDIRECTWR);

        /* Clear indirect completion status */
        reg = ospi_readl1(cospi->apb_base + COSPI_REG_INDIRECTWR);
        if (reg & COSPI_REG_INDIRECTWR_DONE_MASK)
            ospi_writel1(COSPI_REG_INDIRECTWR_DONE_MASK,
                        cospi->apb_base + COSPI_REG_INDIRECTWR);
        break;
    case SPI_NOR_OPS_ERASE:
        break;
    default:
        break;
    }

    dprintf(INFO, "cospi_cancel out.\n");
    return 0;
}

void ospi_enter_xip(struct spi_nor *nor)
{
    u32 reg;
    struct cospi_pdata *cospi = nor->priv_data;

    /* Set mode bit */
    reg = ospi_readl(cospi->apb_base + COSPI_REG_MODE_BIT);
    reg = reg & ~(0xFF);
    reg |= nor->mode_bit;
    ospi_writel(reg, cospi->apb_base + COSPI_REG_MODE_BIT);

    /* Disable direct access */
    reg = ospi_readl(cospi->apb_base + COSPI_REG_CONFIG);
    reg &= ~COSPI_REG_CONFIG_ENB_DIR_ACC_CTRL;
    ospi_writel(reg, cospi->apb_base + COSPI_REG_CONFIG);

    /* Enable xip mode */
    cospi_wait_idle(cospi);
    reg |= COSPI_REG_CONFIG_ENTER_XIP;
    ospi_writel(reg, cospi->apb_base + COSPI_REG_CONFIG);

    /* Enable direct access */
    reg |= COSPI_REG_CONFIG_ENB_DIR_ACC_CTRL;
    ospi_writel(reg, cospi->apb_base + COSPI_REG_CONFIG);
}

static void cospi_phy_en(struct spi_nor *nor, bool phy_en)
{
    struct cospi_pdata *cospi = nor->priv_data;
    uint32_t reg;

    reg = ospi_readl(cospi->apb_base + COSPI_REG_CONFIG);

    if (phy_en) {
        nor->phy_en = true;
        cospi->phy_mode = COSPI_PHY_MASTER_DLL;
        cospi->capture_delay = 0;

        if (nor->dtr_en) {
            cospi->dqs_en = 1;
            if (cospi->ref_clk_hz / 2 >= 130000000)
                g_dummy = 1;
            else
                g_dummy = 0;
        }
        else {
            cospi->dqs_en = nor->dqs_en;
            if (cospi->ref_clk_hz / 2 >= 130000000)
                g_dummy = 1;
            else
                g_dummy = 0;
        }

        reg |= COSPI_REG_CONFIG_ENABLE_PHY_MASK;

    }
    else {
        nor->phy_en = false;
        cospi->phy_mode = COSPI_PHY_NONE;

        /* If disable phy, use ref_clk tap mode. */
        cospi->dqs_en = 0;
        cospi->rclk_loopback = 0;

        if (nor->dtr_en)
            cospi->master_delay = 2;
        else
            cospi->master_delay = 0;
        cospi->capture_delay = cospi->master_delay +
                               COSPI_READCAPTURE_DELAY_DEFAULT;

        g_dummy = 0;
        reg &= ~COSPI_REG_CONFIG_ENABLE_PHY_MASK;
    }

    if (cospi_wait_idle(cospi)) {
        dprintf(CRITICAL, "Wait ospi idle time out!\n");
        return;
    }

    ospi_writel(reg, cospi->apb_base + COSPI_REG_CONFIG);
    udelay(1);
}

static int cospi_phy_master_dll_init(struct cospi_pdata *cospi)
{
    u32 reg;
    int ret;

    reg = ospi_readl(cospi->apb_base + COSPI_REG_PHYCONFIG);

    reg &= ~COSPI_REG_PHYCONFIG_RST;
    ospi_writel(reg, cospi->apb_base + COSPI_REG_PHYCONFIG);
    reg = 0x04 << COSPI_REG_PHYMASTERCTL_INIT_DELAY_LSB;

    ospi_writel(reg, cospi->apb_base + COSPI_REG_PHYMASTERCTL);
    udelay(1);

    reg = ospi_readl(cospi->apb_base + COSPI_REG_PHYCONFIG);
    reg |= COSPI_REG_PHYCONFIG_RST;
    ospi_writel(reg, cospi->apb_base + COSPI_REG_PHYCONFIG);

    reg = ospi_readl(cospi->apb_base + COSPI_REG_PHYDLLOBS);
    dprintf(INFO, "COSPI_REG_PHYDLLOBS = 0x%x \n", reg);

    reg = ospi_readl(cospi->apb_base + COSPI_REG_PHYDLLOBS);
    dprintf(INFO, "COSPI_REG_PHYDLLOBS = 0x%x \n", reg);

    ret = cospi_wait_for_bit_times(cospi->apb_base + COSPI_REG_PHYDLLOBS,
                             COSPI_REG_PHYDLLOBS_LOOPBACK_LOCK, 0, 1000);

    reg = ospi_readl(cospi->apb_base + COSPI_REG_PHYDLLOBS);
    dprintf(INFO, "COSPI_REG_PHYDLLOBS = 0x%x \n", reg);

    reg = (COSPI_REG_PHYCONFIG_RST & ~COSPI_REG_PHYCONFIG_RESYNC) |
            (0x20 << COSPI_REG_PHYCONFIG_TX_DELAY_LSB) |
            (0x20 << COSPI_REG_PHYCONFIG_RX_DELAY_LSB);

    /* bypass rx dll */
    reg |= COSPI_REG_PHYCONFIG_RX_BYPASS;
    udelay(1);
    ospi_writel(reg, cospi->apb_base + COSPI_REG_PHYCONFIG);
    udelay(10);

    reg |= COSPI_REG_PHYCONFIG_RESYNC;
    /* disable rx dll */
    reg &= ~COSPI_REG_PHYCONFIG_RX_BYPASS;
    ospi_writel(reg, cospi->apb_base + COSPI_REG_PHYCONFIG);

    udelay(10);
    return ret;
}

static int cospi_phy_config(struct spi_nor *nor)
{
    int ret;
    u32 reg;
    struct cospi_pdata *cospi = nor->priv_data;

    switch (cospi->phy_mode) {
    case COSPI_PHY_NONE:
        ret = 0;
        break;
    case COSPI_PHY_BYPASS_DLL:
        reg = 0x04 << COSPI_REG_PHYMASTERCTL_INIT_DELAY_LSB |
              COSPI_REG_PHYMASTERCTL_BYPASS;
        ospi_writel(reg, cospi->apb_base + COSPI_REG_PHYMASTERCTL);
        ret = 0;
        break;
    case COSPI_PHY_MASTER_DLL:
        ret = cospi_phy_master_dll_init(cospi);
        break;
    default:
        ret = -1;
    }

    if (ret)
        return ret;

    return 0;
}

static int cospi_do_training(struct spi_nor *nor, int (*check_callback)(struct spi_nor *nor))
{
    int ret = 0;
    u8 tx_delay, rx_delay;
    u8 rx_delay_start = 0;
    bool train_ok = 0;
    struct cospi_pdata *cospi = nor->priv_data;
    u32 reg, ref_clock_per;

    if (cospi->phy_mode == COSPI_PHY_NONE)
        return 0;

    if (cospi->phy_mode == COSPI_PHY_BYPASS_DLL) {
        /* Calculate each clock cycle time in picoseconds */
        ref_clock_per = 1000000 / (cospi->ref_clk_hz / 1000000);

        if (nor->dtr_en) {
            tx_delay = ref_clock_per / 4 / 164;
        }
        else {
            tx_delay = ref_clock_per / 2 / 164;
        }
    }
    else {
        if (nor->dtr_en) {
            tx_delay = 0x1f;
        }
        else {
            tx_delay = 0x3f;
        }
    }

    for (rx_delay = 0; rx_delay <= 0x7f; rx_delay++) {
        reg = (COSPI_REG_PHYCONFIG_RST & ~COSPI_REG_PHYCONFIG_RESYNC) |
              (tx_delay << COSPI_REG_PHYCONFIG_TX_DELAY_LSB) |
              (rx_delay << COSPI_REG_PHYCONFIG_RX_DELAY_LSB);

        udelay(1);
        ospi_writel(reg, cospi->apb_base + COSPI_REG_PHYCONFIG);
        udelay(1);

        reg |= COSPI_REG_PHYCONFIG_RESYNC;
        ospi_writel(reg, cospi->apb_base + COSPI_REG_PHYCONFIG);

        udelay(1);

        if (check_callback(nor) == 0) {
            if (train_ok == 0) {
                train_ok = 1;
                rx_delay_start = rx_delay;
            }
            dprintf(INFO, "ospi phy training succeful, rx_delay = %d \n", rx_delay);
        }
        else {
            if (train_ok && (rx_delay > (rx_delay_start + 10)))
                break;
            else
                train_ok = 0;
        }
    }

    if (train_ok) {
        rx_delay = (rx_delay_start + rx_delay) / 2;
        cospi->phy_training_succ = true;

        reg = (COSPI_REG_PHYCONFIG_RST & ~COSPI_REG_PHYCONFIG_RESYNC) |
            (tx_delay << COSPI_REG_PHYCONFIG_TX_DELAY_LSB) |
            (rx_delay << COSPI_REG_PHYCONFIG_RX_DELAY_LSB);
        ospi_writel(reg, cospi->apb_base + COSPI_REG_PHYCONFIG);

        udelay(1);
        reg |= COSPI_REG_PHYCONFIG_RESYNC;
        ospi_writel(reg, cospi->apb_base + COSPI_REG_PHYCONFIG);

        dprintf(CRITICAL, "ospi phy training pass!\n");
        ret = 0;
    }
    else {
        cospi_phy_en(nor, 0);
        dprintf(CRITICAL, "ospi phy training failed, exit phy mode!\n");
        ret = -1;
    }

    cospi_wait_idle(cospi);
    return ret;
}

static int cospi_phy_training(struct spi_nor *nor, int (*check_callback)(struct spi_nor *nor))
{
    struct cospi_pdata *cospi = nor->priv_data;
    cospi_wait_idle(cospi);

    if (nor->dtr_en || nor->ddr_training) {
        ospi_writel(0x4, cospi->apb_base + 0x110);
    }
    else {
        ospi_writel(0xc, cospi->apb_base + 0x110);
    }

    udelay(1);

    /* glitch workaround: Enable loopback and disable dqs, bypass rx dll. */
    ospi_writel(0, cospi->apb_base + COSPI_REG_READCAPTURE);
    ospi_writel(COSPI_REG_PHYCONFIG_RX_BYPASS,
                cospi->apb_base + COSPI_REG_PHYCONFIG);

    cospi_phy_en(nor, 1);

    cospi_phy_config(nor);

    /* glitch workaround: recover capture config. */
    cospi_capture_set(nor);

    return cospi_do_training(nor, check_callback);
}

static void cospi_nor_setup(struct spi_nor *nor)
{
    int size_changed;
    struct cospi_pdata *cospi = nor->priv_data;

    size_changed = ((cospi->block_power_index != log2_uint(nor->block_size)) ||
                    (cospi->page_size != nor->page_size));

    if (size_changed) {
        cospi_size_set(nor);
    }
    /*
     * If countroller cs not select this chip,
     * config baudrate delay and capture set.
     */
    if (cospi->current_cs != nor->cs) {
        cospi_cs_set(nor);
        cospi_baudrate_set(nor);
        cospi_delay_set(nor);
        cospi_capture_set(nor);
    }

    return;
}

enum handler_return cospi_irq_handle(void *arg)
{
    u32 status;
    struct spi_nor *nor = arg;
    struct cospi_pdata *cospi = nor->priv_data;

    status = readl(cospi->apb_base + COSPI_REG_IRQSTATUS);
    writel(status, cospi->apb_base + COSPI_REG_IRQSTATUS);

    if (status & COSPI_REG_IRQ_ECC_ERR)
        dprintf(CRITICAL, "ospi ecc error detected!\n");

    if (status & COSPI_REG_IRQSTATUS_INDIRECT_DONE_MASK) {
        cospi_tx_complete(nor, 0);
        cospi_rx_complete(nor, 0);
    }
#if 0
    // TODO: atf irq event
    status = ospi_readl(cospi->apb_base + 0xa04);

    status = readl(cospi->apb_base + 0xa38);
    writel(0, cospi->apb_base + 0xa38);

    status = readl(cospi->apb_base + 0xa44);
    writel(0, cospi->apb_base + 0xa44);
#endif
    return INT_NO_RESCHEDULE;
}

void cospi_lock(struct spi_nor *nor)
{
    struct cospi_pdata *cospi = nor->priv_data;
    mutex_acquire(&cospi->bus_mutex);
    cospi_nor_setup(nor);
}

void cospi_unlock(struct spi_nor *nor)
{
    struct cospi_pdata *cospi = nor->priv_data;
    mutex_release(&cospi->bus_mutex);
}

static int cospi_data_thread(void *arg)
{
    u32 ret;
    struct cospi_pdata *cospi = arg;
    struct spi_nor *nor = cospi->priv;
    struct spi_nor_handle *handle = NULL;

    for (;;) {
        event_wait(&cospi->xfer_start_event);
        nor = cospi->priv;
        ret = cospi_do_xfer(nor, &nor->data_cmd);
        if (nor->async_mode) {
            handle = nor->parent;
            if (handle && handle->event_handle) {
                handle->opt_type = nor->data_cmd.type;
                handle->opt_result =
                    ret ? SPI_NOR_OPT_FAILED : SPI_NOR_OPT_COMPLETE;
                handle->event_handle(handle->opt_type, handle->opt_result);
            }
        }
        else {
            nor->data_error = ret;
            event_signal(&cospi->xfer_done_event, false);
        }
    }

    return 0;
}

int spi_nor_host_init(struct spi_nor *nor)
{
    u32 reg;
    struct spi_nor_config *config_data = &nor->config_data;
    struct cospi_pdata *cospi = &s_cospi;
    nor->priv_data = cospi;
    cospi->priv = nor;

    g_dummy = 0;
    nor->async_mode = 0;
    nor->phy_en = false;

    cospi->id = config_data->id;
    cospi->irq = config_data->irq;
    cospi->ref_clk_hz = config_data->clk;
    cospi->sclk = config_data->bus_clk;
    /* initialize the current cs for no one */
    cospi->current_cs = 0xff;
    /* the default capture delay is 1 */
    cospi->capture_delay = COSPI_READCAPTURE_DELAY_DEFAULT;

    cospi->sram_size_nbit = 8;
    cospi->fifo_depth = 1 << (cospi->sram_size_nbit - 1);
    cospi->fifo_width = 4;

    cospi->ahb_base = (addr_t)_ioaddr(config_data->ahb_base);
    cospi->apb_base = (addr_t)_ioaddr(config_data->apb_base);
    cospi->block_power_index = 0;
    cospi->page_size = 0;

    if (cospi_wait_idle(cospi)) {
        dprintf(CRITICAL, "Wait ospi idle time out!\n");
        return -1;
    }

    /* Clear ospi misc register */
    ospi_writel(0, cospi->apb_base + 0x110);

    /* Disable the controller */
    ospi_writel(0, cospi->apb_base + COSPI_REG_CONFIG);

    /* disable all interrupts */
    ospi_writel(0, cospi->apb_base + COSPI_REG_IRQMASK);

    /* not remap the address */
    ospi_writel(0, cospi->apb_base + COSPI_REG_REMAP);

    /* Set indirect trigger address range width, equal fifo bytes*/
    ospi_writel(7, cospi->apb_base + COSPI_REG_INDIRECTTRIGGER_RANGE);
    cospi->trigger_range_size = 1 << (7 + 2);

    nor->page_size = MIN(nor->page_size, cospi->fifo_depth * cospi->fifo_width);
    nor->page_size = MIN(nor->page_size, cospi->trigger_range_size);

    /* Load indirect trigger address, this address equal to AHB paddr */
    cospi->trigger_address = 0x4000000 - cospi->trigger_range_size;
    ospi_writel(cospi->trigger_address,
                cospi->apb_base + COSPI_REG_INDIRECTTRIGGER);

#if 0
    /* config mpu for disable sarm address cache */
    mpu_add_region(0, cospi->ahb_base + cospi->trigger_address,
                   cospi->trigger_range_size, MPU_REGION_DEVICE);
    mpu_enable(true);
#endif
    /* Configure the read fifo depth equal sram / 2. */
    ospi_writel(cospi->fifo_depth, cospi->apb_base + COSPI_REG_SRAMPARTITION);

    /* Indirect read watermark set with read fifo_depth / 2 */
    ospi_writel(cospi->fifo_depth * cospi->fifo_width / 2,
                cospi->apb_base + COSPI_REG_INDIRECTRDWATERMARK);

    /* Indirect write watermark set with write fifo_depth / 4 */
    ospi_writel(cospi->fifo_depth * cospi->fifo_width / 4,
                cospi->apb_base + COSPI_REG_INDIRECTWRWATERMARK);

    /* Enable Direct Access mode*/
    reg = ospi_readl(cospi->apb_base + COSPI_REG_CONFIG);
    reg |= COSPI_REG_CONFIG_ENB_DIR_ACC_CTRL;
    ospi_writel(reg, cospi->apb_base + COSPI_REG_CONFIG);

    register_int_handler(cospi->irq, &cospi_irq_handle, (void *)nor);
    unmask_interrupt(cospi->irq);

    writel(COSPI_IRQ_STATUS_MASK, cospi->apb_base + COSPI_REG_IRQSTATUS);
    writel(COSPI_IRQ_MASK_RD, cospi->apb_base + COSPI_REG_IRQMASK);
    writel(COSPI_IRQ_MASK_WR, cospi->apb_base + COSPI_REG_IRQMASK);
#if 0
    // TODO: atf irq event
    writel(0, cospi->apb_base + 0xa08);
    writel(0, cospi->apb_base + 0xa10);
    writel(0, cospi->apb_base + 0xa34);
    writel(0, cospi->apb_base + 0xa40);
#endif

    nor->host_ops.reg_read = cospi_command_read;
    nor->host_ops.reg_write = cospi_command_write;
    nor->host_ops.transfer = cospi_xfer;
    nor->host_ops.training = cospi_phy_training;
    nor->host_ops.cancel = cospi_cancel;
    nor->host_ops.lock = cospi_lock;
    nor->host_ops.unlock = cospi_unlock;

    /* Enable the controller */
    cospi_enable(cospi, 1);

    /* de-assert the reset pin */
    reg = ospi_readl(cospi->apb_base + COSPI_REG_CONFIG);
    reg |= COSPI_REG_CONFIG_RESET_SELECT_MASK;
    ospi_writel(reg, cospi->apb_base + COSPI_REG_CONFIG);
    reg |= COSPI_REG_CONFIG_RESET_PIN_MASK;
    ospi_writel(reg, cospi->apb_base + COSPI_REG_CONFIG);
    udelay(1000);
    reg &= ~COSPI_REG_CONFIG_RESET_PIN_MASK;
    ospi_writel(reg, cospi->apb_base + COSPI_REG_CONFIG);
    udelay(1000);

    return 0;
}

void spi_nor_reset_slave(addr_t apb_base)
{
    u32 reg = 0;
    reg = ospi_readl(apb_base + COSPI_REG_CONFIG);
    reg |= COSPI_REG_CONFIG_RESET_SELECT_MASK;
    reg &= ~COSPI_REG_CONFIG_RESET_PIN_MASK;
    ospi_writel(reg, apb_base + COSPI_REG_CONFIG);
}



static void cospi_early_init(uint level)
{
    struct cospi_pdata *cospi = &s_cospi;

    memset(cospi, 0, sizeof(struct cospi_pdata));

    mutex_init(&cospi->bus_mutex);

    event_init(&cospi->dma_event, false, EVENT_FLAG_AUTOUNSIGNAL);
    event_init(&cospi->complete_event, false, EVENT_FLAG_AUTOUNSIGNAL);
    event_init(&cospi->xfer_start_event, false, EVENT_FLAG_AUTOUNSIGNAL);
    event_init(&cospi->xfer_done_event, false, EVENT_FLAG_AUTOUNSIGNAL);
    dprintf(INFO, "creat cospi thread!\n");
    thread_t *thread =
        thread_create("cospi_thread", cospi_data_thread, (void *)cospi,
                      HIGH_PRIORITY, DEFAULT_STACK_SIZE);
    thread_detach(thread);
    thread_resume(thread);
}

#if !XIP
LK_INIT_HOOK(cospi, cospi_early_init, LK_INIT_LEVEL_TARGET);
#endif
