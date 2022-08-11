#include <common_hdr.h>
#include <atb_crypto.h>
#include <bn.h>
#include <stddef.h>

#include <soc.h>
#include <arch.h>
extern crypto_eng_t *crypto;

static ce_hash_context hash_contexts[HASH_JOB_LIST_NUMBER];
static U8 current_context_index = 0xffu;

static U32 context_to_index(U32 context_id)
{
    U8 i = 0u;
    U8 index = 0xff;

    for ( i = 0u; i < HASH_JOB_LIST_NUMBER; i++) {
        if (context_id == hash_contexts[i].context_id) {
            index = i;
            break;
        }
    }

    return index;
}

/* The "context_id" is cpu_id+job_id*/
/*
    detail:
*/

U32 crypto_hash_start(U32 context_id, U8 algo)
{
    U32 cnt = 0;
    U32 res = 0;
    U8 index;
    U32 block_sz = get_hash_block_sz(algo);
    U16 sa_slot = 0xffff;

    do {
        if ((NULL == crypto) || (NULL == crypto->ops.hash_write_back)) {
            res = CRYPTO_INVALID_PARAs;
            break;
        }
        cnt++;
        if (!is_valid_hash_alg(algo)) {
            DBG("%s: Opps, invalid hash alg %d\n", __FUNCTION__, algo);
            res = CRYPTO_INVALID_ALGO;
            break;
        }
        cnt++;
        if (0xff != context_to_index(context_id)) {
            DBG("%s: Opps, the context is existed %d\n", __FUNCTION__, context_id);
            res = CRYPTO_CONTEXT_ID_EXISTED;
            break;
        }
        cnt++;
        index = context_to_index(EMPTY_CONTEXT_ID);

        if (0xff == index) {
            DBG("%s: Opps, the context pool is ran out %d\n", __FUNCTION__, context_id);
            res = CRYPTO_CONTEXT_POOL_FULLED;
            break;
        }
        cnt++;
        if (block_sz > 64) {
            sa_slot = get_sa_slot(1);
            block_sz = 7;   /*2^7*/
        } else {
            sa_slot = get_sa_slot(0);
            block_sz = 6;   /*2^6*/
        }
        cnt++;
        if (0xffff == sa_slot) {
            DBG("%s: Opps, not enough sa_solt %d\n", __FUNCTION__, context_id);
            res = CRYPTO_CONTEXT_POOL_FULLED;
            break;
        }
        cnt++;
        hash_contexts[index].context_pos = sa_slot;
        hash_contexts[index].context_id = context_id;
        hash_contexts[index].algo = algo;
        hash_contexts[index].total_len = 0;

        if (current_context_index != 0xff &&
            current_context_index != index &&
            (hash_contexts[current_context_index].total_len >> block_sz) != 0) {
            /*save previous job if having started compute digest*/
            res = crypto->ops.hash_write_back(crypto, &hash_contexts[current_context_index]);
        } else {
            res = 0;
        }
        cnt++;
        current_context_index = index;

        if (cnt != 7) {
            FLOW_ERROR_AND_RESET
            return -1;
        }
    } while (0);

    return res;
}

U32 crypto_hash_update(U32 context_id, U32 msg, U32 msg_sz)
{
    U32 cnt = 0UL;
    U32 res = -1;
    U32 block_sz;
    U32 div;
    U32 fill;
    U32 left;
    U32 left_msg_sz;
    U8 index;

    do {
        if ((NULL == crypto) || (NULL == crypto->ops.hash_update)) {
            res = CRYPTO_INVALID_PARAs;
            break;
        }
        cnt++;
        left_msg_sz = msg_sz;
        index = context_to_index(context_id);
        cnt++;
        if (0xff == index) {
            DBG("%s: Opps, the context is not existed %d\n", __FUNCTION__, context_id);
            res = CRYPTO_CONTEXT_ID_NOT_EXISTED;
            break;
        }
        cnt++;
        current_context_index = index;
        block_sz = get_hash_block_sz(hash_contexts[index].algo);
        div = (block_sz > 64) ? 7 : 6;
        left = hash_contexts[index].total_len - (hash_contexts[index].total_len >> div << div);
        fill = block_sz - left;
        cnt++;
        if (left && (left_msg_sz >= fill)) {
            mini_memcpy_s((void *)(uintptr_t)(hash_contexts[index].residual_data + left), (void *)(uintptr_t)msg, fill);
            res = crypto->ops.hash_update(crypto, &hash_contexts[index],
                                          (uintptr_t)hash_contexts[index].residual_data, block_sz);

            if (res != 0) {
                break;
            }

            hash_contexts[index].total_len += fill;
            msg += (U64)fill;
            left_msg_sz -= fill;
            left = 0;
        }
        cnt++;
        if (left_msg_sz >= block_sz) {
            fill = left_msg_sz - left_msg_sz % block_sz;
            crypto->ops.hash_update(crypto, &hash_contexts[index], msg, fill);
            hash_contexts[index].total_len += fill;
            msg += (U64)fill;
            left_msg_sz -= fill;
        }
        cnt++;
        if (left_msg_sz != 0) {
            mini_memcpy_s((void *)(uintptr_t)(hash_contexts[index].residual_data + left), (void *)(uintptr_t)msg, left_msg_sz);
            hash_contexts[index].total_len += left_msg_sz;
        }

        cnt++;
        if (cnt != 7) {
            FLOW_ERROR_AND_RESET
            return -1;
        }
        res = 0;
    } while (0);

    return res;
}

U32 crypto_hash_finish(U32 context_id, U32 hash, U32 *hash_sz)
{
    U32 cnt = 0UL;
    U32 res = 0;
    U32 block_sz;
    U32 div;
    U32 left;
    U8 index;
    U64 padding_len;

    do {
        if ((NULL == crypto) || (NULL == crypto->ops.hash_finish)) {
            res = CRYPTO_INVALID_PARAs;
            break;
        }
        cnt++;
        index = context_to_index(context_id);

        if (0xff == index) {
            DBG("%s: Opps, the context is not existed %d\n", __FUNCTION__, context_id);
            res = CRYPTO_CONTEXT_ID_NOT_EXISTED;
            break;
        }
        cnt++;
        current_context_index = index;
        block_sz = get_hash_block_sz(hash_contexts[index].algo);
        div = (block_sz > 64) ? 7 : 6;
        U32 padding_r = block_sz / 8 * 7;
        /* Padding Start */
        left = hash_contexts[index].total_len - (hash_contexts[index].total_len >> div << div);
        hash_contexts[index].residual_data[left++] = 0x80U;
        cnt++;
        padding_len = hash_contexts[index].total_len * (U64)8;
        if (left <= padding_r) {
            mini_memset_s(hash_contexts[index].residual_data + left, 0x00U, padding_r - left);
        } else {
            mini_memset_s(hash_contexts[index].residual_data + left, 0x00U, block_sz - left);
            left = 0;
            res = crypto->ops.hash_update(crypto, &hash_contexts[index],
                                          (uintptr_t)hash_contexts[index].residual_data, block_sz);
            mini_memset_s(hash_contexts[index].residual_data, 0x00U, padding_r);
            hash_contexts[index].total_len = ROUNDUP(hash_contexts[index].total_len, block_sz);
        }
        cnt++;

        if (128 == block_sz) {
            mini_memset_s(hash_contexts[index].residual_data + padding_r, 0x00U, 8);

            for (int k = 0; k < 8; k++) {
                *(hash_contexts[index].residual_data + 127 - k) = *(U8 *)(uintptr_t)((U32)(
                            uintptr_t)&padding_len + k);
            }
        } else {
            for (int k = 0; k < 8; k++) {
                *(hash_contexts[index].residual_data + 63 - k) = *(U8 *)(uintptr_t)((U32)(
                            uintptr_t)&padding_len + k);
            }
        }
        cnt++;
        res = crypto->ops.hash_finish(crypto, &hash_contexts[index], hash, hash_sz);

        if (res != 0) {
            break;
        }
        cnt++;
        if (*hash_sz > 32) {
            relese_sa_slot(hash_contexts[index].context_pos, 1);
        } else {
            relese_sa_slot(hash_contexts[index].context_pos, 0);
        }

        hash_contexts[index].context_pos = 0xffff;
        hash_contexts[index].algo = 0xff;
        hash_contexts[index].total_len = 0;
        cnt++;
        current_context_index = 0xffU;
        hash_contexts[index].context_id = 0xffffffffUL;

        if (cnt != 7) {
            FLOW_ERROR_AND_RESET
            return -1;
        }
        res = 0;
    } while (0);

    return res;
}

U32 crypto_hash_init( void )
{
    for (int i = 0; i < HASH_JOB_LIST_NUMBER; i++) {
        hash_contexts[i].context_id  = 0xffffffff;
        hash_contexts[i].context_pos = 0xffff;
        hash_contexts[i].algo        = 0xff;
        hash_contexts[i].total_len   = 0x00;
    }

    return 0;
}

U32 crypto_hash(U8 alg, U32 msg, U32 msg_sz, U32 hash, U32 *hash_sz)
{
    U32 res = -1;

    do {
        if ((NULL == crypto) || (NULL == crypto->ops.hash)) {
            res = CRYPTO_INVALID_PARAs;
            break;
        }

        if (!is_valid_hash_alg(alg)) {
            DBG("%s: Opps, invalid hash alg %d\n", __FUNCTION__, alg);
            res = CRYPTO_INVALID_ALGO;
            break;
        }

        res = crypto->ops.hash(crypto, alg, msg, msg_sz, hash, hash_sz);

    } while (0);

    return res;
}
