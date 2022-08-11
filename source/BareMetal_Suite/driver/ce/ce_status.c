/********************************************************
 *          Copyright(c) 2020   Semidrive               *
 ********************************************************/

#include <common_hdr.h>
#include "ce_wrapper.h"
#include "cryptoengine_reg.h"
#include "cryptoengine_reg_field_def.h"

U8 ce_get_pke_ops_type(void *self)
{
    U8 ops_type = 0u;

    cryptoengine_t *ce =
        (cryptoengine_t *)(Mcu_GetModuleBase(((crypto_eng_t *)self)->m));

    ops_type = ((ce->ce0_pk_commandreg) & 0x7fU);

    switch (ops_type)
    {
    case SM2_SIGN:
    case SM2_VERIFY:
    case SM2_KEYEX:
    case ECDSA_SIGN_GEN:
    case ECDSA_SIGN_VERIFY:
        ops_type = 0xffu;
        break;
    default:
        ops_type = 0x00u;
        break;
    }

    return ops_type;
}
