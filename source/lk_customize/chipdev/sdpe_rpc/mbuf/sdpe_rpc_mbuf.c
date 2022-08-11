/*
 * sdpe_rpc_mbuf.c
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */

#ifdef SUPPORT_SDPE_RPC_DBUF

#include <stdlib.h>
#include <kernel/event.h>
#include <kernel/thread.h>
#include <kernel/mutex.h>
#include <string.h>

#include "sdpe_rpc_cfg.h"
#include "sdpe_rpc_mbuf.h"

#define MBUF_LOCK(lock)         mutex_acquire(lock)
#define MBUF_UNLOCK(lock)       mutex_release(lock)

#define MBUF_AHEAD              4
#define MBUF_MAGIC              0xcd

typedef struct sdpe_rpc_vring {
    uint16_t widx;
    uint16_t ridx;
    uint16_t free_idx[0];
} sdpe_rpc_vring_t;

typedef struct sdpe_rpc_vring_desc {
    uint16_t elem_size;
    uint16_t elem_num;
    mutex_t lock;
    struct sdpe_rpc_vring *ring;
    uint8_t *buf;
} sdpe_rpc_vring_desc_t;

typedef struct sdpe_rpc_mbuf {
    struct sdpe_rpc_vring_desc vring_desc[2];
    struct sdpe_rpc_vring_desc *tx_vring;
    struct sdpe_rpc_vring_desc *rx_vring;
} sdpe_rpc_mbuf_t;

typedef struct sdpe_rpc_mbuf_dev {
    uint8_t mbuf_num;
    struct sdpe_rpc_mbuf *mbuf[SDPE_RPC_MAX_MBUF_NUM];
} *sdpe_rpc_mbuf_dev_t;

static struct sdpe_rpc_mbuf_dev g_sdpe_rpc_mbuf_dev;

static void sdpe_rpc_mbuf_dump(void)
{
    struct sdpe_rpc_mbuf_dev *dev = &g_sdpe_rpc_mbuf_dev;

    for (uint8_t cnt = 0; cnt < dev->mbuf_num; cnt++) {
        struct sdpe_rpc_mbuf *mbuf = dev->mbuf[cnt];
        dprintf(ALWAYS, "mbuf %d:\n", cnt);
        dprintf(ALWAYS, "tx vring 0x%x: %d %d\n",
                (int)mbuf->tx_vring->ring,
                mbuf->tx_vring->ring->ridx,
                mbuf->tx_vring->ring->widx);
        dprintf(ALWAYS, "rx vring 0x%x: %d %d\n",
                (int)mbuf->rx_vring->ring,
                mbuf->rx_vring->ring->ridx,
                mbuf->rx_vring->ring->widx);
        dprintf(ALWAYS, "\n");
    }
}

static inline void sdpe_rpc_add_mbuf_head(uint8_t *head, uint8_t mbuf_idx)
{
    head[0] = MBUF_MAGIC;
    head[1] = mbuf_idx;
}

static inline uint8_t sdpe_rpc_get_mbuf_idx(uint8_t *head)
{
    return head[1];
}

static void sdpe_rpc_vring_add_element(struct sdpe_rpc_vring_desc *vring_desc)
{
    struct sdpe_rpc_vring *vring = vring_desc->ring;
    for (uint16_t cnt = 0; cnt < vring_desc->elem_num; cnt++) {
        vring->free_idx[cnt] = cnt;
    }
    vring->ridx = 0;
    vring->widx = vring_desc->elem_num;
}

static void *sdpe_rpc_vring_alloc(struct sdpe_rpc_vring_desc *vring_desc)
{
    struct sdpe_rpc_vring *vring = vring_desc->ring;
    uint16_t index;
    uint8_t *data;

    MBUF_LOCK(&vring_desc->lock);

    if (vring->widx == vring->ridx) {
        dprintf(ALWAYS, "alloc fail: vring buffer empty,%d,%d\n",
                vring->widx, vring->ridx);
        MBUF_UNLOCK(&vring_desc->lock);
        return NULL;
    }

    index = vring->free_idx[vring->ridx];
    data = &vring_desc->buf[vring_desc->elem_size*index];
    DSB;
    vring->ridx = (vring->ridx + 1) % (vring_desc->elem_num + 1);

    MBUF_UNLOCK(&vring_desc->lock);

    return data;
}

static void spde_rpc_vring_free(struct sdpe_rpc_vring_desc *vring_desc, void *data)
{
    struct sdpe_rpc_vring *vring = vring_desc->ring;
    uint16_t index;

    MBUF_LOCK(&vring_desc->lock);

    index = (((uint8_t*)data) - vring_desc->buf) / vring_desc->elem_size;
    vring->free_idx[vring->widx] = index;
    DSB;
    vring->widx = (vring->widx + 1) % (vring_desc->elem_num + 1);

    MBUF_UNLOCK(&vring_desc->lock);
}

void *sdpe_rpc_alloc_mbuf(uint32_t len)
{
    struct sdpe_rpc_mbuf_dev *dev = &g_sdpe_rpc_mbuf_dev;
    struct sdpe_rpc_mbuf *mbuf;
    struct sdpe_rpc_vring_desc *tx_vring;
    uint8_t *ptr;

    for (uint8_t cnt = 0; cnt < dev->mbuf_num; cnt++) {
        mbuf = dev->mbuf[cnt];
        tx_vring = mbuf->tx_vring;
        if (len > tx_vring->elem_size) {
            continue;
        }
        ptr = sdpe_rpc_vring_alloc(tx_vring);
        if (ptr) {
            sdpe_rpc_add_mbuf_head(ptr, cnt);
            ptr += MBUF_AHEAD;
        }
        return ptr;
    }

    dprintf(ALWAYS, "no buffer to get\n");
    return NULL;
}

void sdpe_rpc_free_mbuf(void *data)
{
    struct sdpe_rpc_mbuf_dev *dev = &g_sdpe_rpc_mbuf_dev;
    struct sdpe_rpc_mbuf *mbuf;
    uint8_t *ptr;
    uint8_t mbuf_idx;

    if (data == NULL) {
        return;
    }

    ptr = data - MBUF_AHEAD;
    ASSERT(ptr[0] == MBUF_MAGIC);
    mbuf_idx = sdpe_rpc_get_mbuf_idx(ptr);
    if (mbuf_idx >= SDPE_RPC_MAX_MBUF_NUM) {
        dprintf(ALWAYS, "mbuf free error: mbuf_idx wrong %d\n", mbuf_idx);
        return;
    }

    mbuf = dev->mbuf[mbuf_idx];
    spde_rpc_vring_free(mbuf->rx_vring, ptr);
}

void sdpe_rpc_mbuf_deinit(void)
{
    sdpe_rpc_mbuf_dev_t dev = &g_sdpe_rpc_mbuf_dev;

    for (uint8_t cnt = 0; cnt < dev->mbuf_num; cnt++) {
        free(dev->mbuf[cnt]);
    }
}

static uint8_t *sdpe_rpc_one_mbuf_init(struct sdpe_rpc_mbuf *mbuf, uint8_t *start_addr,
                                       const struct sdpe_rpc_mbuf_cfg *cfg)
{
    struct sdpe_rpc_vring_desc *vring_desc = mbuf->vring_desc;
    uint8_t *ptr;

    vring_desc[0].elem_num = cfg->num;
    vring_desc[0].elem_size = ALIGN(cfg->size, 4);
    vring_desc[1].elem_num = vring_desc[0].elem_num;
    vring_desc[1].elem_size = vring_desc[0].elem_size;

    mutex_init(&vring_desc[0].lock);
    mutex_init(&vring_desc[1].lock);

    ptr = start_addr;
    vring_desc[0].ring = (struct sdpe_rpc_vring*)ptr;

    ptr += ALIGN(sizeof(struct sdpe_rpc_vring) + \
                 sizeof(uint16_t)*(vring_desc[0].elem_num + 1), 4);
    vring_desc[1].ring = (struct sdpe_rpc_vring*)ptr;

    ptr += ALIGN(sizeof(struct sdpe_rpc_vring) + \
                 sizeof(uint16_t)*(vring_desc[1].elem_num + 1), 4);
    vring_desc[0].buf = ptr;

    ptr += ALIGN(vring_desc[0].elem_num*(vring_desc[0].elem_size + MBUF_AHEAD), 4);
    vring_desc[1].buf = ptr;

    ptr += ALIGN(vring_desc[1].elem_num*(vring_desc[1].elem_size + MBUF_AHEAD), 4);

#ifdef SUPPORT_SDPE_RPC_SERVER
    mbuf->tx_vring = &vring_desc[0];
    mbuf->rx_vring = &vring_desc[1];
#else
    mbuf->rx_vring = &vring_desc[0];
    mbuf->tx_vring = &vring_desc[1];
#endif

    sdpe_rpc_vring_add_element(mbuf->tx_vring);

    return ptr;
}

int sdpe_rpc_mbuf_init(void)
{
    const struct sdpe_rpc_mbuf_cfgs *cfgs = &g_sdpe_rpc_mbuf_cfg;
    struct sdpe_rpc_mbuf_dev *dev = &g_sdpe_rpc_mbuf_dev;
    uint8_t *ptr = (uint8_t*)cfgs->base;

    if (cfgs->buf_num > SDPE_RPC_MAX_MBUF_NUM) {
        dprintf(ALWAYS, "mbuf num error:%d\n", cfgs->buf_num);
        return -1;
    }

    for (uint8_t cnt = 0; cnt < cfgs->buf_num; cnt++) {
        dev->mbuf[cnt] = malloc(sizeof(struct sdpe_rpc_mbuf));
        if (dev->mbuf[cnt] == NULL) {
            dprintf(ALWAYS, "alloc mbuf fail\n");
            sdpe_rpc_mbuf_deinit();
            return -1;
        }

        ptr = sdpe_rpc_one_mbuf_init(dev->mbuf[cnt], ptr,
                                     &(cfgs->cfg[cnt]));
        dev->mbuf_num++;
    }

    return 0;
}
#endif