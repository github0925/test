/*
 * spi_nor.h
 *
 * Copyright (c) 2018 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: ospi controlle header.
 *
 * Revision History:
 * -----------------
 * 0.1, 11/3/2019 init version
 */
#ifndef __SPI_NOR_H__
#define __SPI_NOR_H__

#include <stdbool.h>
#include <sys/types.h>
#include <kernel/mutex.h>
#include <spi_nor_hal.h>

#ifdef WITH_ON_SEMIDRIVE
#include <__regs_ap_iomux_ctrl_ap.h>
#include <__regs_ap_iomux_ctrl_safety.h>

#define IOMUX_REG_CORE1 REG_AP_APB_IOMUX_CTRL_SAFETY
#define IOMUX_REG_CORE2 REG_AP_APB_IOMUX_CTRL_AP

#define _REG_CORE_PAD_PIN_NAME(id, pin, core)                                  \
    core##_IO_PAD_CONFIG_OSPI##id##_##pin
#define REG_CORE_PAD_PIN_NAME(id, pin, core)                                   \
    _REG_CORE_PAD_PIN_NAME(id, pin, core)

#define _GET_IOMUX_CTRL(id) IOMUX_REG_CORE##id
#define GET_IOMUX_CTRL(id) _GET_IOMUX_CTRL(id)

#define OSPI_GET_PIN_PAD_ADDR(id, pin)                                         \
    REG_CORE_PAD_PIN_NAME(id, pin, GET_IOMUX_CTRL(id))
#endif

#undef BIT
#define BIT(nr) (1u << (nr))

#define SNOR_OPCODE_PROTO_LSB (24u)
#define SNOR_DTR_PROTO        BIT(16)

#define SNOR_ADDR_WIDTH_PROTO_LSB  (12u)
#define SNOR_ADDR_WIDTH_PROTO_MASK (0xFu)
#define SNOR_ADDR_0_BYTES          (0u << SNOR_ADDR_WIDTH_PROTO_LSB)
#define SNOR_ADDR_1_BYTES          (1u << SNOR_ADDR_WIDTH_PROTO_LSB)
#define SNOR_ADDR_2_BYTES          (2u << SNOR_ADDR_WIDTH_PROTO_LSB)
#define SNOR_ADDR_3_BYTES          (3u << SNOR_ADDR_WIDTH_PROTO_LSB)
#define SNOR_ADDR_4_BYTES          (4u << SNOR_ADDR_WIDTH_PROTO_LSB)
#define SNOR_GET_ADDR_BYTES(x)     (((x) >> SNOR_ADDR_WIDTH_PROTO_LSB)&\
                                      SNOR_ADDR_WIDTH_PROTO_MASK)

#define SNOR_INST_LANS_PROTO_LSB  (8u)
#define SNOR_INST_LANS_PROTO_MASK (0xFu)
#define SNOR_INST_SINGLE_LANS     (0u << SNOR_INST_LANS_PROTO_LSB)
#define SNOR_INST_DUAL_LANS       (1u << SNOR_INST_LANS_PROTO_LSB)
#define SNOR_INST_QUAD_LANS       (2u << SNOR_INST_LANS_PROTO_LSB)
#define SNOR_INST_OCTAL_LANS      (3u << SNOR_INST_LANS_PROTO_LSB)
#define SNOR_GET_INST_LANS(x)     (((x) >> SNOR_INST_LANS_PROTO_LSB)&\
                                     SNOR_INST_LANS_PROTO_MASK)

#define SNOR_ADDR_LANS_PROTO_LSB  (4u)
#define SNOR_ADDR_LANS_PROTO_MASK (0xFu)
#define SNOR_ADDR_SINGLE_LANS     (0u << SNOR_ADDR_LANS_PROTO_LSB)
#define SNOR_ADDR_DUAL_LANS       (1u << SNOR_ADDR_LANS_PROTO_LSB)
#define SNOR_ADDR_QUAD_LANS       (2u << SNOR_ADDR_LANS_PROTO_LSB)
#define SNOR_ADDR_OCTAL_LANS      (3u << SNOR_ADDR_LANS_PROTO_LSB)
#define SNOR_GET_ADDR_LANS(x)     (((x) >> SNOR_ADDR_LANS_PROTO_LSB)&\
                                     SNOR_ADDR_LANS_PROTO_MASK)

#define SNOR_DATA_LANS_PROTO_LSB  (0u)
#define SNOR_DATA_LANS_PROTO_MASK (0xFu)
#define SNOR_DATA_SINGLE_LANS     (0u << SNOR_DATA_LANS_PROTO_LSB)
#define SNOR_DATA_DUAL_LANS       (1u << SNOR_DATA_LANS_PROTO_LSB)
#define SNOR_DATA_QUAD_LANS       (2u << SNOR_DATA_LANS_PROTO_LSB)
#define SNOR_DATA_OCTAL_LANS      (3u << SNOR_DATA_LANS_PROTO_LSB)
#define SNOR_GET_DATA_LANS(x)     (((x) >> SNOR_DATA_LANS_PROTO_LSB)&\
                                     SNOR_DATA_LANS_PROTO_MASK)

#define SNOR_PROTO_1_1_1                                                       \
    (SNOR_INST_SINGLE_LANS | SNOR_ADDR_SINGLE_LANS | SNOR_DATA_SINGLE_LANS)
#define SNOR_PROTO_1_1_2                                                       \
    (SNOR_INST_SINGLE_LANS | SNOR_ADDR_SINGLE_LANS | SNOR_DATA_DUAL_LANS)
#define SNOR_PROTO_1_1_4                                                       \
    (SNOR_INST_SINGLE_LANS | SNOR_ADDR_SINGLE_LANS | SNOR_DATA_QUAD_LANS)
#define SNOR_PROTO_1_1_8                                                       \
    (SNOR_INST_SINGLE_LANS | SNOR_ADDR_SINGLE_LANS | SNOR_DATA_OCTAL_LANS)
#define SNOR_PROTO_1_2_2                                                       \
    (SNOR_INST_SINGLE_LANS | SNOR_ADDR_DUAL_LANS | SNOR_DATA_DUAL_LANS)
#define SNOR_PROTO_1_4_4                                                       \
    (SNOR_INST_SINGLE_LANS | SNOR_ADDR_QUAD_LANS | SNOR_DATA_QUAD_LANS)
#define SNOR_PROTO_1_8_8                                                       \
    (SNOR_INST_SINGLE_LANS | SNOR_ADDR_OCTAL_LANS | SNOR_DATA_OCTAL_LANS)
#define SNOR_PROTO_2_2_2                                                       \
    (SNOR_INST_DUAL_LANS | SNOR_ADDR_DUAL_LANS | SNOR_DATA_DUAL_LANS)
#define SNOR_PROTO_4_4_4                                                       \
    (SNOR_INST_QUAD_LANS | SNOR_ADDR_QUAD_LANS | SNOR_DATA_QUAD_LANS)
#define SNOR_PROTO_8_8_8                                                       \
    (SNOR_INST_OCTAL_LANS | SNOR_ADDR_OCTAL_LANS | SNOR_DATA_OCTAL_LANS)
#define SNOR_PROTO_1_1_1_DTR (SNOR_PROTO_1_1_1 | SNOR_DTR_PROTO)
#define SNOR_PROTO_1_2_2_DTR (SNOR_PROTO_1_2_2 | SNOR_DTR_PROTO)
#define SNOR_PROTO_1_4_4_DTR (SNOR_PROTO_1_4_4 | SNOR_DTR_PROTO)
#define SNOR_PROTO_1_8_8_DTR (SNOR_PROTO_1_8_8 | SNOR_DTR_PROTO)

#define SPI_NOR_MAX_ID_LEN 6

//#define WITH_OSPI_DEBUG
#ifdef WITH_OSPI_DEBUG
#define ospi_readl(reg)                                                        \
    readl(reg);                                                                \
    dprintf(CRITICAL, "r(0x%lx, r(0x%08x)\n", reg, readl(reg));
#define ospi_writel(val, reg)                                                  \
    writel(val, reg);                                                          \
    dprintf(CRITICAL, "w(0x%lx, 0x%08x), r(0x%08x)\n", reg, val, readl(reg));
#else
#define ospi_writel(val, reg) writel(val, reg)
#define ospi_readl(reg) readl(reg)

#endif

#define ospi_readl1(reg)                                                       \
    readl(reg);                                                                \
    dprintf(CRITICAL, "r(0x%lx, r(0x%08x)\n", reg, readl(reg));
#define ospi_writel1(val, reg)                                                 \
    writel(val, reg);                                                          \
    dprintf(CRITICAL, "w(0x%lx, 0x%08x), r(0x%08x)\n", reg, val, readl(reg));

enum spi_nor_size {
    SPI_NOR_SIZE_512M = 0,
    SPI_NOR_SIZE_1G,
    SPI_NOR_SIZE_2G,
    SPI_NOR_SIZE_4G,
};

struct spi_nor_config {
    uint8_t id;
    uint8_t cs;
    uint32_t clk;
    uint32_t bus_clk;
    addr_t apb_base;
    addr_t ahb_base;
    unsigned int irq;
    bool octal_ddr_en;
};

enum spi_nor_ops {
    SPI_NOR_OPS_READ = 1,
    SPI_NOR_OPS_WRITE,
    SPI_NOR_OPS_ERASE,
    SPI_NOR_OPS_LOCK,
    SPI_NOR_OPS_UNLOCK,
};

struct spi_nor_cmd {
    bool use_dma;
    bool ddr_en;
    bool queue_mode_en;
    enum spi_nor_ops type;
    u8 opcode;
    u8 dummy;

#define SPI_NOR_SINGLE_LANS (0)
#define SPI_NOR_DUAL_LANS (1)
#define SPI_NOR_QUAD_LANS (2)
#define SPI_NOR_OCTAL_LANS (3)
    /* address can be transfer by DQ lans num */
    u8 inst_width;
    u8 addr_width;
    u8 data_width;

#define SPI_NOR_ADDR_0_BYTES (0)
#define SPI_NOR_ADDR_1_BYTES (1)
#define SPI_NOR_ADDR_2_BYTES (2)
#define SPI_NOR_ADDR_3_BYTES (3)
#define SPI_NOR_ADDR_4_BYTES (4)
    /* 0 bytes(no need addr) 3 bytes or 4 bytes */
    u8 addr_bytes;
    u32 addr;

    addr_t buf;
    u32 size;
};

struct flash_info {
    const char *name;
    u8 flash_id[SPI_NOR_MAX_ID_LEN];

    u32 read_proto;
    u32 write_proto;
    u32 erase_proto;

    u8 read_dummy;

    u16 sector_size;
    u16 page_size;

    u32 size;
};

struct spi_nor {
    char *id;
    struct spi_nor_config config_data;
    u8 cs;
    enum spi_nor_size size;
    bool phy_en;
    bool dtr_en;
    bool dqs_en;
    bool ddr_training;
    bool async_mode;

    u8 mode_bit;

    u32 global_read_dummy;
    u32 status_dummy;

    /* use for write protect */
    u32 block_size;

    /* use for program boundaries */
    u32 page_size;

    u32 cssot_ns;
    u32 cseot_ns;
    u32 csdads_ns;
    u32 csda_ns;

    bool data_present;
    u32 data_error;
    bool cancel_flag;
    struct spi_nor_cmd data_cmd;
    void *parent;
    void *priv_data;

    struct flash_info *info;

    struct spi_nor_host_ops {
        int (*reg_write)(struct spi_nor *nor, struct spi_nor_cmd *cmd,
                         const u8 *buf, const unsigned len);
        int (*reg_read)(struct spi_nor *nor, struct spi_nor_cmd *cmd, u8 *buf,
                        const unsigned len);
        int (*transfer)(struct spi_nor *nor, struct spi_nor_cmd *cmd, u8 *buf,
                        u32 size);
        int (*training)(struct spi_nor *nor, int (*check_callback)(struct spi_nor *nor));
        int (*cancel)(struct spi_nor *nor);

        void (*lock)(struct spi_nor *nor);

        void (*unlock)(struct spi_nor *nor);
    } host_ops;
};

int spi_nor_host_init(struct spi_nor *nor);

int spi_nor_init(struct spi_nor *nor);

void spi_nor_deinit(struct spi_nor *nor);

int spi_nor_read(struct spi_nor *nor, uint64_t src, uint8_t *dst,
                 uint64_t length);

int spi_nor_write(struct spi_nor *nor, uint64_t dst, const uint8_t *src_buf,
                  uint64_t length);

int spi_nor_erase(struct spi_nor *nor, uint64_t dst, uint64_t length);

int spi_nor_cancel(struct spi_nor *nor);

uint64_t spi_nor_get_capacity(struct spi_nor *nor);

uint64_t spi_nor_get_flash_id(struct spi_nor *nor);

void spi_nor_reset_slave(addr_t apb_base);

#endif
