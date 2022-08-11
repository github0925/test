/********************************************************
 *      Copyright(c) 2018   Semidrive  Semiconductor    *
 *      All rights reserved.                            *
 ********************************************************/

#include <common_hdr.h>
#include <fuse_ctrl/fuse_ctrl.h>
#include <fusemap.h>

U32 fuse_get_parity_type(U32 id)
{
    fuse_pari_type_e t = PARITY_NONE;

    if ((id >= ECC_FUSE_START) && (id <= ECC_FUSE_END)) {
        t = PARITY_ECC;
    } else if ((id < ECC_FUSE_START) ||
               ((id > ECC_FUSE_END) && (id < RED_VAL_START1))) {
        t = PARITY_RED;
    }

    return t;
}

U32 fuse_get_red_pos(U32 id)
{
    U32 pos = 0;

    if (id < ECC_FUSE_START) {
        pos = RED_VAL_START1 + id;
    } else if ((id > ECC_FUSE_END) && (id < RED_VAL_START1)) {
        pos = RED_VAL_START2 + (id - (ECC_FUSE_END + 1));
    }

    return pos;
}

U32 fuse_get_ecc_pos(U32 id, U32 *wd, U32 *shift)
{
    *wd = ECC_VAL_START + (id - ECC_FUSE_START) / 4;
    *shift = ((id - ECC_FUSE_START) % 4) * 8;

    return 0;
}

