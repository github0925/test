/********************************************************
 *      Copyright(c) 2019   Semidrive  Semiconductor    *
 *      All rights reserved.                            *
 ********************************************************/

#include <common_hdr.h>
#include <soc.h>
#include <debug.h>
#include "soc_share.h"

void soc_read_uuid(U8 *id, U32 sz)
{
    if (sz >= 8) {
        U32 v[2];
        v[0] = soc_read_fuse(FUSE_UUID_START);
        v[1] = soc_read_fuse(FUSE_UUID_START + 1);
        U8 *p = (U8 *)v;
        memcpy(id, p, 8);
    }
}

void soc_read_dev_id(U8 *id, U32 sz)
{
    if (sz >= 8) {
        U32 v[2];
        v[0] = soc_read_fuse(FUSE_DID_START);
        v[1] = soc_read_fuse(FUSE_DID_START + 1);
        U8 *p = (U8 *)v;
        memcpy(id, p, 8);
        DBG("%s: dump did as\n", __FUNCTION__);
        DBG_ARRAY_DUMP(id, 8);
    }
}
