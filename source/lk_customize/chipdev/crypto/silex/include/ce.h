/*
* Copyright (c) 2019 Semidrive Semiconductor.
* All rights reserved.
*
*/

#ifndef _CE_H
#define _CE_H

#include <reg.h>
#include <lib/sd_sysdef.h>
#include <debug.h>
#include <kernel/event.h>
#include <irq.h>
#include <string.h>
#include <lib/reg.h>

//TODO: need consider ce count in domain
#if CE_IN_SAFETY_DOMAIN
#define VCE_COUNT 1
#define ZONE_VCE0_INT CE1_CE0_INT_O_NUM
#define ZONE_TRNG_INT CE1_TRNG_IRQ_NUM
#else
#define VCE_COUNT 8
#define ZONE_VCE0_INT CE2_CE0_INT_O_NUM
#define ZONE_TRNG_INT CE2_TRNG_IRQ_NUM
#endif

#define CE2_SRAM_PUB_HASH_HMAC_KEY_SIZE 0x40 //64 byte

/** ram offset
 * |                             sram public                                 |                           sram secure                       |
 * |----------------------------4k(0x1000)-----------------------------------|----------------------------4k(0x1000)-----------------------|
 * |         |hmac key 64 byte      |hash update iv 64 byte| hash out 64 byte|aes iv 16 byte|aes key 32byte|aes xkey 32byte|               |
 * |         |offset      0xf40     |offset      0xf80     |offset   0xfc0   |offset 0x0    |offset 0x10   |offset 0x30    |0ffset 0x50    |
*/
#define CE2_SRAM_PUB_HASH_HMAC_KEY_ADDR_OFFSET 0xf40
#define CE2_SRAM_PUB_HASH_MULT_PART_UPDATE_IV_ADDR_OFFSET 0xf80
#define CE2_SRAM_PUB_HASH_OUT_ADDR_OFFSET 0xfc0

#define CE2_SRAM_SEC_AES_IV_ADDR_OFFSET 0x0
#define CE2_SRAM_SEC_AES_KEY_ADDR_OFFSET 0x10
#define CE2_SRAM_SEC_AES_XKEY_ADDR_OFFSET 0x30

/**
 Enumeration of data/iv/key/context address type
*/
typedef enum ce_addr_type_t {
    SRAM_PUB = 0,
    SRAM_SEC = 1,
    KEY_INT = 2,
    EXT_MEM = 3,
    PKE_INTERNAL = 4
} ce_addr_type_t;

typedef struct block_t {
    uint8_t* addr;   /* Start address of the data (FIFO or contiguous memory) */
    uint32_t len;    /* Length of data expressed in bytes */
    ce_addr_type_t addr_type; /* address type */
} block_t;

struct mem_node {
    size_t size;
    size_t is_used;
    uint8_t* ptr;
};

extern  event_t g_ce_signal[VCE_COUNT];
extern  event_t g_trng_signal;
extern  bool g_ce_inited;
extern volatile uint32_t g_int_flag;

/** Helper to build a ::block_t from a pair of data address and size */
#define BLOCK_T_CONV(array, length, addr_type) { (uint8_t *) (array), (uint32_t) (length), addr_type}

struct mem_node* ce_malloc(size_t size);
void ce_free(struct mem_node* mem_node);

static inline block_t block_t_convert(const volatile void* array, uint32_t length, ce_addr_type_t addr_type)
{
#if defined __cplusplus
    //'compound literals' are not valid in C++
    block_t  blk = BLOCK_T_CONV(array, length, addr_type);
    return blk;
#else
    //'compound literal' (used below) is valid in C99
    return (block_t)BLOCK_T_CONV(array, length, addr_type);
#endif
}

static uint32_t switch_addr_type(ce_addr_type_t addr_type_s)
{
    uint32_t addr_type_d;

    switch (addr_type_s) {
        case SRAM_PUB:
            addr_type_d = 0x1;
            break;

        case SRAM_SEC:
            addr_type_d = 0x3;
            break;

        case KEY_INT:
            addr_type_d = 0x4;
            break;

        case EXT_MEM:
            addr_type_d = 0x0;
            break;

        case PKE_INTERNAL:
            addr_type_d = 0x2;
            break;

        default:
            addr_type_d = 0x0;
    }

    return addr_type_d;
}

addr_t addr_switch_to_ce(uint32_t vce_id, ce_addr_type_t addr_type_s, addr_t addr);
//int32_t ce_init(void);
int32_t ce_init(uint32_t vce_id);
int32_t ce_globle_init(void);

void clean_cache(addr_t start, uint32_t size);
void flush_cache(addr_t start, uint32_t size);
void clean_cache_block(block_t* data, uint32_t ce_id);
void invalidate_cache_block(block_t* data, uint32_t ce_id);

static inline void printf_binary(const char* info, const void* content, uint32_t content_len)
{
    dprintf(CRITICAL, "%s: \n", info);
    uint32_t i = 0;

    dprintf(CRITICAL, "%p ", content);

    for (; i < content_len; i++) {
        dprintf(CRITICAL, "0x%02x ", *((uint8_t*)(content) + i));

        if (0 != i && 0 == (i + 1) % 16) {
            dprintf(CRITICAL, "\n");
        }
    }

    if (!(i % 16)) {
        dprintf(CRITICAL, "\n");
    }
}

#endif
