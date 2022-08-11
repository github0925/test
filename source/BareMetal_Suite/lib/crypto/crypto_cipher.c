#include <common_hdr.h>
#include <atb_crypto.h>
#include <bn.h>
#include <stddef.h>

extern crypto_eng_t *crypto;

static ce_cipher_context cipher_contexts[CIPHER_JOB_LIST_NUMBER];
static U8 current_context_index = 0xffu;

static U32 context_to_index(U32 context_id)
{
    U8 i = 0u;
    U8 index = 0xff;

    for ( i = 0u; i < CIPHER_JOB_LIST_NUMBER; i++) {
        if (context_id == cipher_contexts[i].context_id) {
            index = i;
            break;
        }
    }

    return index;
}

U32 cipher_start (U32 context_id, cipher_type_e type, cipher_mode_e mode,
                 cipher_op_e ops, U32 key, U32 key_sz,
                 U32 iv, U32 iv_sz)
{
    U32 res = -1;

    U8 index = 0xff;
    U16 sa_slot = 0xffff;

    do {

        if ((NULL == crypto)
            || ((0 == iv) && (mode != MODE_ECB)) || (0 == key)
            || (key_sz > sizeof(sym_key_t))) {
            DBG("%s: Opps, NULL PTRs\n", __FUNCTION__);
            res = CRYPTO_INVALID_PARAs;
            break;
        }

        U8 *key_addr;

        key_addr = (uint8_t *)(uintptr_t)key;

        if (0xff != context_to_index(context_id)) {
            DBG("%s: Opps, the context is existed %d\n", __FUNCTION__, context_id);
            res = CRYPTO_CONTEXT_ID_EXISTED;
            break;
        }

        index = context_to_index(EMPTY_CONTEXT_ID);
        if (0xff == index) {
            DBG("%s: Opps, the context pool is ran out %d\n", __FUNCTION__, context_id);
            res = CRYPTO_CONTEXT_POOL_FULLED;
            break;
        }

        if (MODE_ECB == mode){
            sa_slot = 0xffff;
        } else {
            sa_slot = get_sa_slot(0);
        }
        cipher_contexts[index].context_pos = sa_slot;
        sa_slot = get_sa_slot(0);
        cipher_contexts[index].key_pos = sa_slot;
        cipher_contexts[index].context_id = context_id;
        cipher_contexts[index].algo_mode = mode;
        cipher_contexts[index].key_len = type;
        cipher_contexts[index].operation = ops;

        res = crypto->ops.cipher_start(crypto, &cipher_contexts[index], key_addr, key_sz, iv, iv_sz);
    } while (0);

    return res;
}

U32 cipher_update (U32 context_id, U32 in, U32 in_sz, U32 out, U32 *out_sz)
{
    U32 res = -1;
    U8 index;

    do {
        /** TODO: check module is busy*/

        if ((NULL == crypto) || (0 == in)||
            (0 == out) || (NULL == out_sz)) {
            DBG("%s: Opps, NULL PTRs\n", __FUNCTION__);
            res = CRYPTO_INVALID_PARAs;
            break;
        }

        /* At present, cipher only support block aligned data.*/
        if ( 0 != in_sz % 16) {
            res = CRYPTO_UNALIGNED_DATA;
            break;
        }

        index = context_to_index(context_id);
        if (0xff == index) {
            DBG("%s: Opps, the context is not existed %d\n", __FUNCTION__, context_id);
            res = CRYPTO_CONTEXT_ID_NOT_EXISTED;
            break;
        }

        current_context_index = index;

        res = crypto->ops.cipher_update(crypto, &cipher_contexts[index], in, in_sz, out, out_sz);

    } while(0);

    return res;
}

U32 cipher_finish (U32 context_id)
{
    U32 res = -1;
    U8 index;

    do {

        if (NULL == crypto) {
            DBG("%s: Opps, NULL PTRs\n", __FUNCTION__);
            res = CRYPTO_INVALID_PARAs;
            break;
        }

        index = context_to_index(context_id);
        if (0xff == index) {
            DBG("%s: Opps, the context is not existed %d\n", __FUNCTION__, context_id);
            res = CRYPTO_CONTEXT_ID_NOT_EXISTED;
            break;
        }

        current_context_index = 0xff;
        if (cipher_contexts[index].context_pos != 0xffff) {
            relese_sa_slot(cipher_contexts[index].context_pos, 0);
        }
        if (cipher_contexts[index].key_pos != 0xffff) {
            relese_sa_slot(cipher_contexts[index].key_pos, 0);
        }
        cipher_contexts[index].algo_mode = 0xff;
        cipher_contexts[index].context_id = 0xffffffff;
        cipher_contexts[index].context_pos = 0xffff;
        cipher_contexts[index].key_pos = 0xffff;
        res = 0;
    } while(0);

    return res;
}

U32 crypto_cipher_init( void )
{
    for (int i = 0; i < CIPHER_JOB_LIST_NUMBER; i++) {
        cipher_contexts[i].algo_mode = 0xff;
        cipher_contexts[i].context_id = 0xffffffff;
        cipher_contexts[i].context_pos = 0xffff;
        cipher_contexts[i].key_pos = 0xffff;
    }
    return 0;
}


int32_t crypto_cipher_enc(cipher_type_e type, cipher_mode_e mode,
                          uintptr_t key, uint32_t key_sz,
                          uintptr_t iv, uint32_t iv_sz,
                          uintptr_t msg, int msg_sz,
                          uintptr_t cipher, int *c_sz)
{
    int32_t res = -1;
    uint8_t * key_addr;

    do {
        if ((NULL == crypto) || (NULL == crypto->ops.cipher_enc)
            || (0 == msg) || (0 == cipher) || (NULL == c_sz)
            || ((0 == iv) && (mode != MODE_ECB)) || (0 == key)
            || (key_sz > 64)) {
            DBG("%s: Opps, NULL PTRs\n", __FUNCTION__);
            res = CRYPTO_INVALID_PARAs;
            break;
        }

        key_addr = (uint8_t *)(uintptr_t)key;

        if (!is_valid_cipher_type(type) || !is_valid_cipher_mode(mode)) {
            res = CRYPTO_INVALID_PARAs;
            break;
        }

        res = crypto->ops.cipher_enc(crypto, type, mode, key_addr, key_sz, iv, iv_sz, msg, msg_sz, cipher, c_sz);
    } while (0);

    return res;
}

int32_t crypto_cipher_dec(cipher_type_e type, cipher_mode_e mode,
                          uint32_t key, uint32_t key_sz,
                          uint32_t iv, uint32_t iv_sz,
                          uint32_t msg, int *msg_sz,
                          uint32_t cipher, int c_sz)
{
    int32_t res = -1;
    uint8_t * key_addr;

    do {
        if ((NULL == crypto) || (NULL == crypto->ops.cipher_dec)
            || (0 == msg) || (0 == cipher) || (NULL == msg_sz)
            || ((0 == iv) && (mode != MODE_ECB)) || (0 == key)
            || (key_sz > 64)) {
            break;
        }

        key_addr = (uint8_t *)(uintptr_t)key;

        if (!is_valid_cipher_type(type) || !is_valid_cipher_mode(mode)) {
            res = -2;
            break;
        }

        res = crypto->ops.cipher_dec(crypto, type, mode, key_addr, key_sz, iv, iv_sz, cipher, c_sz, msg, msg_sz);

    } while (0);

    return res;
}
