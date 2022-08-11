/*
 * avb_sha_ce.c
 *
 * Copyright (c) 2020 SemiDrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */

#include <app.h>
#include <arch.h>
#include <assert.h>
#include <debug.h>
#include <err.h>
#include <platform.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <lib/reg.h>
#include <chip_res.h>
#include <trace.h>

#include "avb_sha.h"
#include "crypto_hal.h"
#include "sd_hash.h"
#include "res.h"
#include "storage_device.h"

#define LOCAL_TRACE    0

#define HANDLE_IDX     0
#define FIRST_PART_IDX 1

#define VCE_ID_DEF  (g_ce_mem_res.res_id[0])
extern const domain_res_t g_ce_mem_res;

static void update(const uint8_t *data, size_t len,
                   void *handle, sd_hash_alg_t hash_alg,
                   size_t blz, uint8_t *ctx_block, size_t *ctx_len,
                   uint64_t *ctx_tot_len, bool *first_part)
{
    size_t block_nb;
    uint8_t *data_aligned;
    crypto_status_t status;
    size_t new_len, rem_len, tmp_len;
    const uint8_t *shifted_data;

    data_aligned = ctx_block + blz;
    ASSERT(IS_ALIGNED(ctx_block, CACHE_LINE));
    ASSERT(IS_ALIGNED(data_aligned, CACHE_LINE));

    tmp_len = blz - *ctx_len;
    rem_len = len < tmp_len ? len : tmp_len;
    memcpy(&ctx_block[*ctx_len], data, rem_len);

    if (*ctx_len + len <= blz) {
        *ctx_len += len;
        return;
    }

    LTRACEF("first:%d len:%llu\n", *first_part, (uint64_t)len);
    status = hal_hash_update(handle, hash_alg, *first_part, ctx_block, blz);

    LTRACEF("status:%d\n", status);

    if (*first_part) {
        *first_part = false;
    }

    new_len = len - rem_len;
    block_nb = new_len / blz;
    shifted_data = data + rem_len;

    rem_len = new_len % blz;
#if 0
    if (!rem_len) {
        block_nb--;
        rem_len = blz;
    }
#endif

    if (block_nb) {
#if 0
        uint64_t unaligned_len;
        unaligned_len = round_up((addr_t)shifted_data, CACHE_LINE)
                        - (addr_t)shifted_data;

        if (unaligned_len) {
            ASSERT(blz >= CACHE_LINE);
            memcpy(data_aligned, shifted_data, unaligned_len);
            status |= hal_hash_update(handle, hash_alg, *first_part,
                                      data_aligned, unaligned_len);
            shifted_data += unaligned_len;
        }
        status |= hal_hash_update(handle, hash_alg, *first_part,
                                  shifted_data, block_nb * blz - unaligned_len);
#else
        status |= hal_hash_update(handle, hash_alg, *first_part,
                                  shifted_data, block_nb * blz);
#endif

    }

    LTRACEF("status:%d hash_alg:%d\n", status, hash_alg);
    if (rem_len)
        memcpy(ctx_block, &shifted_data[block_nb * blz], rem_len);

    *ctx_len = rem_len;
    *ctx_tot_len += (block_nb + 1) * blz;
}

void avb_sha256_init(AvbSHA256Ctx *ctx)
{
    bool *first_part;
    addr_t *save_handle;
    void *crypto_handle = NULL;

    hal_crypto_creat_handle(&crypto_handle, VCE_ID_DEF);

    if (crypto_handle == NULL) {
        dprintf(CRITICAL, "%s create handle error\n", __func__);
        return;
    }

    memset(ctx, 0x0, sizeof(AvbSHA256Ctx));
    save_handle = (addr_t *)&ctx->h[HANDLE_IDX * 2];
    *save_handle  = (addr_t)crypto_handle;

    first_part = (bool *)&ctx->h[FIRST_PART_IDX * 2];
    *first_part = true;
}

void avb_sha256_update(AvbSHA256Ctx *ctx, const uint8_t *data, size_t len)
{
    void *handle;
    addr_t *save_handle;
    uint8_t *ctx_block;
    size_t *ctx_len;
    uint64_t *ctx_tot_len;
    bool *first_part;

    if (!len || !data || !ctx)
        return;

    save_handle = (addr_t *)&ctx->h[HANDLE_IDX * 2];
    handle = (void *)(addr_t)(*save_handle);

    if (!handle) {
        dprintf(CRITICAL, "%s handle error!\n", __func__);
        return;
    }

    first_part = (bool *)&ctx->h[FIRST_PART_IDX * 2];
    ctx_block = ctx->block;
    ctx_len = &ctx->len;
    ctx_tot_len = &ctx->tot_len;

    update(data, len, handle, SD_ALG_SHA256,
           AVB_SHA256_BLOCK_SIZE, ctx_block,
           ctx_len, ctx_tot_len, first_part);
}

AVB_ATTR_WARN_UNUSED_RESULT uint8_t *avb_sha256_final(AvbSHA256Ctx *ctx)
{
    void *handle;
    bool *first_part;
    uint64_t total_len;
    addr_t *save_handle;

    if (!ctx)
        return NULL;

    save_handle = (addr_t *)&ctx->h[HANDLE_IDX * 2];
    handle = (void *)(addr_t)(*save_handle);
    first_part = (bool *)&ctx->h[FIRST_PART_IDX * 2];

    if (!handle) {
        dprintf(CRITICAL, "%s handle error!\n", __func__);
        goto end;
    }

    if (*first_part) {
        hal_hash(handle, SD_ALG_SHA256,
                 ctx->block, ctx->len, ctx->buf,
                 AVB_SHA256_DIGEST_SIZE);
    }
    else {
        total_len = ctx->tot_len + ctx->len;
        hal_hash_finish(handle, SD_ALG_SHA256,
                        ctx->block, ctx->len, ctx->buf,
                        AVB_SHA256_DIGEST_SIZE, total_len);
        LTRACEF("total:%llu\n", total_len);
    }

end:

    if (handle)
        hal_crypto_delete_handle(handle);

    return ctx->buf;
}

void avb_sha512_init(AvbSHA512Ctx *ctx)
{
    bool *first_part;
    addr_t *save_handle;
    void *crypto_handle = NULL;

    hal_crypto_creat_handle(&crypto_handle, VCE_ID_DEF);

    if (crypto_handle == NULL) {
        dprintf(CRITICAL, "%s create handle error\n", __func__);
        return;
    }

    memset(ctx, 0x0, sizeof(AvbSHA512Ctx));
    save_handle = (addr_t *)&ctx->h[HANDLE_IDX];
    *save_handle  = (addr_t)crypto_handle;

    first_part = (bool *)&ctx->h[FIRST_PART_IDX];
    *first_part = true;
}

void avb_sha512_update(AvbSHA512Ctx *ctx, const uint8_t *data, size_t len)
{
    void *handle;
    size_t *ctx_len;
    bool *first_part;
    uint8_t *ctx_block;
    addr_t *save_handle;
    uint64_t *ctx_tot_len;

    if (!len || !data || !ctx)
        return;

    save_handle = (addr_t *)&ctx->h[HANDLE_IDX];
    handle = (void *)(addr_t)(*save_handle);

    if (!handle) {
        dprintf(CRITICAL, "%s handle error!\n", __func__);
        return;
    }

    first_part = (bool *)&ctx->h[FIRST_PART_IDX];
    ctx_block = ctx->block;
    ctx_len = &ctx->len;
    ctx_tot_len = &ctx->tot_len;

    update(data, len, handle, SD_ALG_SHA512,
           AVB_SHA512_BLOCK_SIZE, ctx_block,
           ctx_len, ctx_tot_len, first_part);
}

AVB_ATTR_WARN_UNUSED_RESULT uint8_t *avb_sha512_final(AvbSHA512Ctx *ctx)
{
    void *handle;
    bool *first_part;
    uint64_t total_len;
    addr_t *save_handle;

    if (!ctx)
        return NULL;

    save_handle = (addr_t *)&ctx->h[HANDLE_IDX];
    handle = (void *)(addr_t)(*save_handle);
    first_part = (bool *)&ctx->h[FIRST_PART_IDX];

    if (!handle) {
        dprintf(CRITICAL, "%s handle error!\n", __func__);
        goto end;
    }

    if (*first_part) {
        hal_hash(handle, SD_ALG_SHA512,
                 ctx->block, ctx->len, ctx->buf,
                 AVB_SHA512_DIGEST_SIZE);
    }
    else {
        total_len = ctx->tot_len + ctx->len;
        hal_hash_finish(handle, SD_ALG_SHA512,
                        ctx->block, ctx->len, ctx->buf,
                        AVB_SHA512_DIGEST_SIZE, total_len);
        LTRACEF("total:%llu\n", total_len);
    }

end:

    if (handle)
        hal_crypto_delete_handle(handle);

    return ctx->buf;
}
