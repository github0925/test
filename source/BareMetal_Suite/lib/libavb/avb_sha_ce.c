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

#include <arch.h>
#include <assert.h>
#include <debug.h>
#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "atb_crypto.h"
#include "avb_sha.h"

#define CONTEXT_ID_DEFAULT 0

void avb_sha256_init(AvbSHA256Ctx *ctx)
{
    crypto_hash_start(CONTEXT_ID_DEFAULT, ALG_HASH_SHA256);
}

void avb_sha256_update(AvbSHA256Ctx *ctx, const uint8_t *data, size_t len)
{
    crypto_hash_update(CONTEXT_ID_DEFAULT,  (addr_t)data, len);
}

AVB_ATTR_WARN_UNUSED_RESULT uint8_t *avb_sha256_final(AvbSHA256Ctx *ctx)
{
    U32 hash_sz = AVB_SHA256_DIGEST_SIZE;

    if (!ctx)
        return NULL;

    crypto_hash_finish(CONTEXT_ID_DEFAULT, (addr_t)(&ctx->buf), &hash_sz);
    return ctx->buf;
}

void avb_sha512_init(AvbSHA512Ctx *ctx)
{
    crypto_hash_start(CONTEXT_ID_DEFAULT, ALG_HASH_SHA512);
}

void avb_sha512_update(AvbSHA512Ctx *ctx, const uint8_t *data, size_t len)
{
    crypto_hash_update(CONTEXT_ID_DEFAULT,  (addr_t)data, len);
}

AVB_ATTR_WARN_UNUSED_RESULT uint8_t *avb_sha512_final(AvbSHA512Ctx *ctx)
{
    U32 hash_sz = AVB_SHA512_DIGEST_SIZE;

    if (!ctx)
        return NULL;

    crypto_hash_finish(CONTEXT_ID_DEFAULT, (addr_t)(&ctx->buf), &hash_sz);

    return ctx->buf;
}
