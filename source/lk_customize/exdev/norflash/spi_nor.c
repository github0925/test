/*
 * spi_nor.c
 *
 * Copyright (c) 2021 semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: spi_nor flash driver.
 *
 */
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <debug.h>
#include <errno.h>
#include <platform.h>

#include "spi_nor.h"

#define udelay(x) spin(x)

#define PROTO(_opcode, _dq, _addr_width, _dtr) \
    (_opcode << SNOR_OPCODE_PROTO_LSB | _dq | _addr_width | _dtr)

#define GET_PROTO_OPCODE(x) (((x) >> SNOR_OPCODE_PROTO_LSB) & 0xff)

/* Flash opcodes. */
#define SPINOR_OP_WREN 0x06       /* Write enable */
#define SPINOR_OP_RDSR 0x05       /* Read status register */
#define SPINOR_OP_WRSR 0x01       /* Write status register 1 byte */
#define SPINOR_OP_RDSR2 0x3f      /* Read status register 2 */
#define SPINOR_OP_WRSR2 0x3e      /* Write status register 2 */
#define SPINOR_OP_READ 0x03       /* Read data bytes (low frequency) */
#define SPINOR_OP_READ_FAST 0x0b  /* Read data bytes (high frequency) */
#define SPINOR_OP_READ_1_1_2 0x3b /* Read data bytes (Dual Output SPI) */
#define SPINOR_OP_READ_1_2_2 0xbb /* Read data bytes (Dual I/O SPI) */
#define SPINOR_OP_READ_1_1_4 0x6b /* Read data bytes (Quad Output SPI) */
#define SPINOR_OP_READ_1_4_4 0xeb /* Read data bytes (Quad I/O SPI) */
#define SPINOR_OP_READ_1_1_8 0x8b /* Read data bytes (Octal Output SPI) */
#define SPINOR_OP_READ_1_8_8 0xcb /* Read data bytes (Octal I/O SPI) */
#define SPINOR_OP_PP 0x02         /* Page program (up to 256 bytes) */
#define SPINOR_OP_PP_1_1_4 0x32   /* Quad page program */
#define SPINOR_OP_PP_1_4_4 0x38   /* Quad page program */
#define SPINOR_OP_PP_1_1_8 0x82   /* Octal page program */
#define SPINOR_OP_PP_1_8_8 0xc2   /* Octal page program */
#define SPINOR_OP_BE_4K 0x20      /* Erase 4KiB block */
#define SPINOR_OP_BE_4K_PMC 0xd7  /* Erase 4KiB block on PMC chips */
#define SPINOR_OP_BE_32K 0x52     /* Erase 32KiB block */
#define SPINOR_OP_CHIP_ERASE 0xc7 /* Erase whole flash chip */
#define SPINOR_OP_SE 0xd8         /* Sector erase (usually 64KiB) */
#define SPINOR_OP_RDID 0x9f       /* Read JEDEC ID */
#define SPINOR_OP_RDSFDP 0x5a     /* Read SFDP */
#define SPINOR_OP_RDCR 0x35       /* Read configuration register */
#define SPINOR_OP_RDFSR 0x70      /* Read flag status register */
#define SPINOR_OP_CLFSR 0x50      /* Clear flag status register */
#define SPINOR_OP_RDEAR 0xc8      /* Read Extended Address Register */
#define SPINOR_OP_WREAR 0xc5      /* Write Extended Address Register */
#define SPINOR_OP_RESET_EN 0x66   /* Reset Enable */
#define SPINOR_OP_RESET 0x99      /* Reset */

/* 4-byte address opcodes - used on Spansion and some Macronix flashes. */
#define SPINOR_OP_READ_4B 0x13       /* Read data bytes (low frequency) */
#define SPINOR_OP_READ_FAST_4B 0x0c  /* Read data bytes (high frequency) */
#define SPINOR_OP_READ_1_1_2_4B 0x3c /* Read data bytes (Dual Output SPI) */
#define SPINOR_OP_READ_1_2_2_4B 0xbc /* Read data bytes (Dual I/O SPI) */
#define SPINOR_OP_READ_1_1_4_4B 0x6c /* Read data bytes (Quad Output SPI) */
#define SPINOR_OP_READ_1_4_4_4B 0xec /* Read data bytes (Quad I/O SPI) */
#define SPINOR_OP_READ_1_1_8_4B 0x7c /* Read data bytes (Octal Output SPI) */
#define SPINOR_OP_READ_1_8_8_4B 0xcc /* Read data bytes (Octal I/O SPI) */
#define SPINOR_OP_PP_4B 0x12         /* Page program (up to 256 bytes) */
#define SPINOR_OP_PP_1_1_4_4B 0x34   /* Quad page program */
#define SPINOR_OP_PP_1_4_4_4B 0x3e   /* Quad page program */
#define SPINOR_OP_PP_1_1_8_4B 0x84   /* Octal page program */
#define SPINOR_OP_PP_1_8_8_4B 0x8e   /* Octal page program */
#define SPINOR_OP_BE_4K_4B 0x21      /* Erase 4KiB block */
#define SPINOR_OP_BE_32K_4B 0x5c     /* Erase 32KiB block */
#define SPINOR_OP_SE_4B 0xdc         /* Sector erase (usually 64KiB) */

/* Double Transfer Rate opcodes - defined in JEDEC JESD216B. */
#define SPINOR_OP_READ_1_1_1_DTR 0x0d
#define SPINOR_OP_READ_1_2_2_DTR 0xbd
#define SPINOR_OP_READ_1_4_4_DTR 0xed

#define SPINOR_OP_READ_1_1_1_DTR_4B 0x0e
#define SPINOR_OP_READ_1_2_2_DTR_4B 0xbe
#define SPINOR_OP_READ_1_4_4_DTR_4B 0xee

#define SPINOR_SECTOR_4K_SIZE (0x1000)
#define SPINOR_SECTOR_32K_SIZE (0x8000)
#define SPINOR_SECTOR_64K_SIZE (0x10000)
#define SPINOR_SECTOR_128K_SIZE (0x20000)

#define SPINOR_ID_CAPACITY_OFFSET 2

uint8_t training_pattern[32] __ALIGNED(CACHE_LINE) = {
    0x44, 0x1c, 0x39, 0x05, 0xd3, 0x7a, 0x3c, 0x04,
    0x16, 0x42, 0x0c, 0x8b, 0x7d, 0x12, 0x89, 0xa2,
    0xb8, 0xb1, 0xf7, 0xe8, 0xb7, 0x49, 0xca, 0x1c,
    0xaa, 0x9b, 0xf2, 0x7e, 0x01, 0x97, 0x60, 0x8c
};
uint8_t training_buf[32] __ALIGNED(CACHE_LINE) = {0};

static char devid[8] = {0};

#define ARRAY_NUMS(array) (sizeof(array) / sizeof(array[0]))

static struct flash_info spi_nor_ids[] = {
    /* miron */
    {
        .name = "mt35xu",
        .flash_id = {0x2c, 0x5b},
        .read_proto = PROTO(SPINOR_OP_READ_1_8_8_4B, SNOR_PROTO_1_8_8,
                            SNOR_ADDR_4_BYTES, 0),
        .write_proto = PROTO(SPINOR_OP_PP_1_8_8_4B, SNOR_PROTO_1_8_8,
                             SNOR_ADDR_4_BYTES, 0),
        .erase_proto =
            PROTO(SPINOR_OP_BE_4K_4B, SNOR_PROTO_1_1_1, SNOR_ADDR_4_BYTES, 0),
        .sector_size = SPINOR_SECTOR_4K_SIZE,
        .read_dummy = 16,
        .page_size = 256,
    },
    /* issi */
    {
        .name = "is25wp",
        .flash_id = {0x9d, 0x70},
        .read_proto = PROTO(SPINOR_OP_READ_1_1_4, SNOR_PROTO_1_1_4,
                            SNOR_ADDR_3_BYTES, 0),
        .write_proto = PROTO(SPINOR_OP_PP_1_1_4, SNOR_PROTO_1_1_4,
                             SNOR_ADDR_3_BYTES, 0),
        .erase_proto =
            PROTO(SPINOR_OP_BE_4K, SNOR_PROTO_1_1_1, SNOR_ADDR_3_BYTES, 0),
        .sector_size = SPINOR_SECTOR_4K_SIZE,
        .read_dummy = 8,
        .page_size = 256,
    },
    /* winbond */
    {
        .name = "w25q",
        .flash_id = {0xef, 0x60},
        .read_proto = PROTO(SPINOR_OP_READ_1_1_4, SNOR_PROTO_1_1_4,
                            SNOR_ADDR_3_BYTES, 0),
        .write_proto = PROTO(SPINOR_OP_PP_1_1_4, SNOR_PROTO_1_1_4,
                             SNOR_ADDR_3_BYTES, 0),
        .erase_proto =
            PROTO(SPINOR_OP_BE_4K, SNOR_PROTO_1_1_1, SNOR_ADDR_3_BYTES, 0),
        .sector_size = SPINOR_SECTOR_4K_SIZE,
        .read_dummy = 6,
        .page_size = 256,
    },
    /* giga */
    {
        .name = "gd25lx",
        .flash_id = {0xc8, 0x68},
        .read_proto = PROTO(SPINOR_OP_READ_1_8_8_4B, SNOR_PROTO_1_8_8,
                            SNOR_ADDR_4_BYTES, 0),
        .write_proto = PROTO(SPINOR_OP_PP_1_8_8_4B, SNOR_PROTO_1_8_8,
                             SNOR_ADDR_4_BYTES, 0),
        .erase_proto =
            PROTO(SPINOR_OP_BE_4K_4B, SNOR_PROTO_1_1_1, SNOR_ADDR_4_BYTES, 0),
        .sector_size = SPINOR_SECTOR_4K_SIZE,
        .read_dummy = 16,
        .page_size = 256,
    },
    {
        .name = "gd25q",
        .flash_id = {0xc8, 0x40},
        .read_proto = PROTO(SPINOR_OP_READ_FAST, SNOR_PROTO_1_1_1,
                            SNOR_ADDR_3_BYTES, 0),
        .write_proto = PROTO(SPINOR_OP_PP, SNOR_PROTO_1_1_1,
                             SNOR_ADDR_3_BYTES, 0),
        .erase_proto =
            PROTO(SPINOR_OP_BE_4K, SNOR_PROTO_1_1_1, SNOR_ADDR_3_BYTES, 0),
        .sector_size = SPINOR_SECTOR_4K_SIZE,
        .read_dummy = 8,
        .page_size = 256,
    },
    {
        .name = "gd25lb",
        .flash_id = {0xc8, 0x67},
        .read_proto = PROTO(SPINOR_OP_READ_1_1_4_4B, SNOR_PROTO_1_1_4,
                            SNOR_ADDR_4_BYTES, 0),
        .write_proto = PROTO(SPINOR_OP_PP_1_4_4_4B, SNOR_PROTO_1_4_4,
                             SNOR_ADDR_4_BYTES, 0),
        .erase_proto =
            PROTO(SPINOR_OP_BE_4K_4B, SNOR_PROTO_1_1_1, SNOR_ADDR_4_BYTES, 0),
        .sector_size = SPINOR_SECTOR_4K_SIZE,
        .read_dummy = 8,
        .page_size = 256,
    }
};

static int get_status(struct spi_nor *nor, u8 *status)
{
    u8 reg[2] = {0};
    int ret;

    struct spi_nor_cmd read_cmd = {
        .opcode = SPINOR_OP_RDSR,
        .addr_bytes = SPI_NOR_ADDR_0_BYTES,
        .dummy = 0,
    };

    ret = nor->host_ops.reg_read(nor, &read_cmd, reg, 2);
    if (ret)
        return ret;

    *status = reg[0];
    return 0;
}

static int waite_idle(struct spi_nor *nor)
{
    int ret = 0;
    u8 flash_status;
    lk_time_t timeout = current_time() + 1000;

    while (1) {
        ret = get_status(nor, &flash_status);
        if (ret) {
            dprintf(CRITICAL, "spi_nor get flash status failed, ret: %d!\n",
                    ret);
            break;
        }
        dprintf(INFO, "flash_status = 0x%x \n", flash_status);
        if (!(flash_status & BIT(0)))
            break;
        if (current_time() > timeout) {
            ret = -ETIMEDOUT;
            dprintf(CRITICAL, "wait flash idle timeout, ret = %d!\n", ret);
            break;
        }
        udelay(1);
    }

    return ret;
}

static struct flash_info* flash_id_read(struct spi_nor *nor, char *buf)
{
    struct flash_info* info = NULL;

    struct spi_nor_cmd read_cmd = {
        .opcode = SPINOR_OP_RDID,
        .dummy = 0,
        .addr_bytes = SPI_NOR_ADDR_0_BYTES,
    };

    nor->host_ops.lock(nor);
    nor->host_ops.reg_read(nor, &read_cmd, (u8 *)buf, 8);
    nor->host_ops.unlock(nor);

    for (u32 i = 0; i < ARRAY_NUMS(spi_nor_ids); i++) {
            info = &spi_nor_ids[i];
            if (!memcmp(info->flash_id, buf, 2)) {
                info->size = 1 << buf[SPINOR_ID_CAPACITY_OFFSET];
                return info;
            }
    }

    return info;
}

int spi_nor_read(struct spi_nor *nor, uint64_t src, uint8_t *dst,
                 uint64_t length)
{
    int ret;
    struct spi_nor_cmd read_cmd = {
        .use_dma = 1,
        .type = SPI_NOR_OPS_READ,
        .opcode = GET_PROTO_OPCODE(nor->info->read_proto),
        .dummy = nor->info->read_dummy,
        .inst_width = SNOR_GET_INST_LANS(nor->info->read_proto),
        .addr_width = SNOR_GET_ADDR_LANS(nor->info->read_proto),
        .data_width = SNOR_GET_DATA_LANS(nor->info->read_proto),
        .addr_bytes = SNOR_GET_ADDR_BYTES(nor->info->read_proto),
        .addr = src,
    };

    nor->host_ops.lock(nor);

    /* wait for flash idle */
    ret = waite_idle(nor);

    if (ret)
        goto read_out;

    ret = nor->host_ops.transfer(nor, &read_cmd, (u8 *)dst, length);
read_out:
    nor->host_ops.unlock(nor);

    if (ret)
        dprintf(CRITICAL, "spi_nor read data failed, ret: %d!\n", ret);

    return ret;
}

static int training_data_check(struct spi_nor *nor)
{
    uint32_t addr = 0 + nor->block_size;

    if (spi_nor_read(nor, addr, training_buf, 32)) {
        dprintf(CRITICAL, "%s: read taining data error\n", __FUNCTION__);
        return -1;
    };

    if(memcmp(training_buf, training_pattern, 32))
        return -1;

    return 0;
}

int spi_nor_erase(struct spi_nor *nor, uint64_t dst, uint64_t length)
{
    int ret = 0;
    uint64_t remaining = length;
    struct spi_nor_cmd erase_cmd = {
        .type = SPI_NOR_OPS_ERASE,
        .opcode = GET_PROTO_OPCODE(nor->info->erase_proto),
        .inst_width = SNOR_GET_INST_LANS(nor->info->erase_proto),
        .addr_width = SNOR_GET_ADDR_LANS(nor->info->erase_proto),
        .addr_bytes = SNOR_GET_ADDR_BYTES(nor->info->erase_proto),
        .addr = dst,
    };

    nor->host_ops.lock(nor);

    while (remaining) {
        ret = waite_idle(nor);

        if (ret)
            break;

        ret = nor->host_ops.transfer(nor, &erase_cmd, NULL, 0);

        if (ret)
            break;

        erase_cmd.addr += nor->block_size;
        remaining -= nor->block_size;
    }

    nor->host_ops.unlock(nor);

    if (ret) {
        dprintf(CRITICAL, "spi_nor erase failed, ret: %d!\n", ret);
    }

    return ret;
}


int spi_nor_init(struct spi_nor *nor)
{
    uint32_t training_addr;
    uint32_t i;

    nor->cssot_ns = 30;
    nor->cseot_ns = 30;
    nor->csdads_ns = 30;
    nor->csda_ns = 30;

    int ret = 0;
    nor->id = devid;

    if (spi_nor_host_init(nor)) {
        dprintf(CRITICAL, "spi_nor init controller failed\n");
        ret = -1;
        goto init_out;
    }

    /* read flash device id */
    nor->info = flash_id_read(nor, devid);

    if (!nor->info) {
        dprintf(CRITICAL, "spi_nor read flash id error, id:\n");
        hexdump8(devid, 8);
        ret = -1;
        goto init_out;
    }

    nor->size = nor->info->size;
    nor->block_size = nor->info->sector_size;
    nor->page_size = nor->info->page_size;
    training_addr = nor->block_size + 0;

    if (training_data_check(nor)) {
        /* test spi_nor read write */
        if (spi_nor_erase(nor, training_addr, nor->block_size)) {
            dprintf(CRITICAL, "%s: erase error\n", __FUNCTION__);
            ret = -1;
            goto init_out;
        }

        if (spi_nor_write(nor, training_addr, training_pattern, 32)) {
            dprintf(CRITICAL, "%s: write error\n", __FUNCTION__);
            ret = -1;
            goto init_out;
        }

        /* wait for flash idle */
        ret = waite_idle(nor);
    }

    if (ret) {
        goto init_out;
    }

    ret = nor->host_ops.training(nor, training_data_check);

    /* wait for flash idle */
    ret = waite_idle(nor);

    if (ret)
        goto init_out;

    if (spi_nor_read(nor, training_addr, training_buf, 32)) {
        dprintf(CRITICAL, "%s: read taining data error\n", __FUNCTION__);
        ret = -1;
        goto init_out;
    };

    uint8_t *p2 = training_pattern;

    uint8_t *p3 = training_buf;

    for (i = 0; i < 32; i++, p2++, p3++) {
        if (*p2 != *p3) {
            dprintf(CRITICAL, "\n spi_nor training data error: 0x%x - 0x%x : i=%d\n",
                    *p3, *p2, i);
            ret = -1;
            goto init_out;
        }
    }

init_out:
    return ret;
}

int spi_nor_write(struct spi_nor *nor, uint64_t dst, const uint8_t *src_buf,
                  uint64_t length)
{
    int ret = 0;
    struct spi_nor_cmd write_cmd = {
        .use_dma = 1,
        .type = SPI_NOR_OPS_WRITE,
        .opcode = GET_PROTO_OPCODE(nor->info->write_proto),
        .dummy = 0,
        .inst_width = SNOR_GET_INST_LANS(nor->info->write_proto),
        .addr_width = SNOR_GET_ADDR_LANS(nor->info->write_proto),
        .data_width = SNOR_GET_DATA_LANS(nor->info->write_proto),
        .addr_bytes = SNOR_GET_ADDR_BYTES(nor->info->write_proto),
        .addr = dst,
    };

    nor->host_ops.lock(nor);

    /* wait for flash idle */
    ret = waite_idle(nor);
    if (ret)
        goto write_out;

    ret = nor->host_ops.transfer(nor, &write_cmd, (void *)src_buf, length);

write_out:
    nor->host_ops.unlock(nor);
    if (ret) {
        dprintf(CRITICAL, "spi_nor write array failed, ret: %d!\n", ret);
    }
    return ret;
}

void spi_nor_deinit(struct spi_nor *nor) { return; }

inline int spi_nor_cancel(struct spi_nor *nor)
{
    return nor->host_ops.cancel(nor);
}

inline uint64_t spi_nor_get_capacity(struct spi_nor *nor)
{
    return nor ? nor->size : 0;
}

inline uint64_t spi_nor_get_flash_id(struct spi_nor *nor)
{
    uint64_t flash_id = nor ? *(uint64_t *)&nor->id[0] : 0;
    return flash_id;
}
