/*
* Copyright (c) 2019 Semidrive Semiconductor.
* All rights reserved.
*
*/

#include <string.h>

#include <sx_pke_conf.h>
#include <ce_reg.h>
#include <sx_errors.h>
#include <sx_dma.h>
#include <sx_pke_funcs.h>

uint32_t modular_common(uint32_t vce_id,
                        uint32_t op,
                        block_t ptra,
                        block_t ptrb,
                        block_t ptrn,
                        block_t ptrc)
{
    //Set pointer register
    pke_set_config(vce_id, 0, 1, 2, 3);

    // Set command to enable byte-swap
    pke_set_command(vce_id, op, ptra.len, BA414EP_LITTLEEND, 0);

    /* Load verification parameters */
    mem2CryptoRAM_rev(vce_id, ptra, ptra.len, BA414EP_MEMLOC_0, true);
    mem2CryptoRAM_rev(vce_id, ptrb, ptrb.len, BA414EP_MEMLOC_1, true);
    mem2CryptoRAM_rev(vce_id, ptrn, ptrn.len, BA414EP_MEMLOC_3, true);

    uint32_t status = pke_start_wait_status(vce_id);

    if (status) {
        return status;
    }

    CryptoRAM2mem_rev(vce_id, ptrc, ptrc.len, BA414EP_MEMLOC_2, true);

    return status;
}

/* C= A + B mod N */
uint32_t modular_add(uint32_t vce_id,
                     block_t ptra,
                     block_t ptrb,
                     block_t ptrn,
                     block_t ptrc)
{
    return modular_common(vce_id, BA414EP_OPTYPE_MOD_ADD, ptra, ptrb, ptrn, ptrc);
}

/* C= A - B mod N */
uint32_t modular_sub(uint32_t vce_id,
                     block_t ptra,
                     block_t ptrb,
                     block_t ptrn,
                     block_t ptrc)
{
    return modular_common(vce_id, BA414EP_OPTYPE_MOD_SUB, ptra, ptrb, ptrn, ptrc);
}

/* C= A * B mod N */
uint32_t modular_multi(uint32_t vce_id,
                       block_t ptra,
                       block_t ptrb,
                       block_t ptrn,
                       block_t ptrc)
{
    return modular_common(vce_id, BA414EP_OPTYPE_MOD_MULT_ODD, ptra, ptrb, ptrn, ptrc);
}

/* C= B mod N */
uint32_t modular_reduce(uint32_t vce_id,
                        uint32_t op,
                        block_t ptrb,
                        block_t ptrn,
                        block_t ptrc)
{
    //Set pointer register
    pke_set_config(vce_id, 0, 1, 2, 3);

    // Set command to enable byte-swap
    pke_set_command(vce_id, op, ptrb.len, BA414EP_LITTLEEND, 0);

    /* Load verification parameters */
    mem2CryptoRAM_rev(vce_id, ptrb, ptrb.len, BA414EP_MEMLOC_1, true);
    mem2CryptoRAM_rev(vce_id, ptrn, ptrn.len, BA414EP_MEMLOC_3, true);

    uint32_t status = pke_start_wait_status(vce_id);

    if (status) {
        return status;
    }

    CryptoRAM2mem_rev(vce_id, ptrc, ptrc.len, BA414EP_MEMLOC_2, true);

    return status;
}

/* C= B mod N */
uint32_t modular_reduce_odd(uint32_t vce_id,
                            block_t ptrb,
                            block_t ptrn,
                            block_t ptrc)
{
    return modular_reduce(vce_id, BA414EP_OPTYPE_MOD_RED_ODD, ptrb, ptrn, ptrc);
}

/* C= A / B mod N */
uint32_t modular_div(uint32_t vce_id,
                     block_t ptra,
                     block_t ptrb,
                     block_t ptrn,
                     block_t ptrc)
{
    return modular_common(vce_id, BA414EP_OPTYPE_MOD_DIV_ODD, ptra, ptrb, ptrn, ptrc);
}

/* C= 1 / B mod N */
uint32_t modular_inverse(uint32_t vce_id,
                         uint32_t op,
                         block_t ptrb,
                         block_t ptrn,
                         block_t ptrc)
{
    //Set pointer register
    pke_set_config(vce_id, 0, 1, 2, 3);

    // Set command to enable byte-swap
    pke_set_command(vce_id, op, ptrb.len, BA414EP_LITTLEEND, 0);

    /* Load verification parameters */
    mem2CryptoRAM_rev(vce_id, ptrb, ptrb.len, BA414EP_MEMLOC_1, true);
    mem2CryptoRAM_rev(vce_id, ptrn, ptrn.len, BA414EP_MEMLOC_3, true);

    uint32_t status = pke_start_wait_status(vce_id);

    if (status) {
        return status;
    }

    CryptoRAM2mem_rev(vce_id, ptrc, ptrc.len, BA414EP_MEMLOC_2, true);

    return status;
}

/* C= 1 / B mod N */
uint32_t modular_inverse_odd(uint32_t vce_id,
                             block_t ptrb,
                             block_t ptrn,
                             block_t ptrc)
{
    return modular_inverse(vce_id, BA414EP_OPTYPE_MOD_INV_ODD, ptrb, ptrn, ptrc);
}

/* C= sqrt(A) mod N */
uint32_t modular_square_root(uint32_t vce_id,
                             block_t ptra,
                             block_t ptrn,
                             block_t ptrc)
{
    //Set pointer register
    pke_set_config(vce_id, 7, 0, 8, 0);

    // Set command to enable byte-swap
    pke_set_command(vce_id, BA414EP_OPTYPE_MOD_SQRT, ptra.len, BA414EP_LITTLEEND, 0);

    /* Load verification parameters */
    mem2CryptoRAM_rev(vce_id, ptra, ptra.len, BA414EP_MEMLOC_7, true);
    mem2CryptoRAM_rev(vce_id, ptrn, ptrn.len, BA414EP_MEMLOC_0, true);

    uint32_t status = pke_start_wait_status(vce_id);

    if (status) {
        return status;
    }

    CryptoRAM2mem_rev(vce_id, ptrc, ptrc.len, BA414EP_MEMLOC_8, true);

    return status;
}

/* C= A * B */
uint32_t multiplicate(uint32_t vce_id,
                      block_t ptra,
                      block_t ptrb,
                      block_t ptrc)
{
    //Set pointer register
    pke_set_config(vce_id, 0, 1, 2, 3);

    // Set command to enable byte-swap
    pke_set_command(vce_id, BA414EP_OPTYPE_MULT, ptra.len, BA414EP_LITTLEEND, 0);

    /* Load verification parameters */
    mem2CryptoRAM_rev(vce_id, ptra, ptra.len, BA414EP_MEMLOC_0, true);
    mem2CryptoRAM_rev(vce_id, ptrb, ptrb.len, BA414EP_MEMLOC_1, true);

    uint32_t status = pke_start_wait_status(vce_id);

    if (status) {
        return status;
    }

    CryptoRAM2mem_rev(vce_id, ptrc, ptrc.len, BA414EP_MEMLOC_2, true);

    return status;
}

/* C= 1 / B mod N */
uint32_t modular_inverse_even(uint32_t vce_id,
                              block_t ptrb,
                              block_t ptrn,
                              block_t ptrc)
{
    return modular_inverse(vce_id, BA414EP_OPTYPE_MOD_INV_EVEN, ptrb, ptrn, ptrc);
}

/* C= B mod N */
uint32_t modular_reduce_even(uint32_t vce_id,
                             block_t ptrb,
                             block_t ptrn,
                             block_t ptrc)
{
    return modular_reduce(vce_id, BA414EP_OPTYPE_MOD_RED_EVEN, ptrb, ptrn, ptrc);
}
