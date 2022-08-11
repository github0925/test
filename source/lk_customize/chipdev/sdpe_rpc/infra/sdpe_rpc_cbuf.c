/*
 * sdpe_rpc_cbuf.c
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <debug.h>
#include <pow2.h>

#include <sdpe_rpc_common.h>
#include <sdpe_rpc_cbuf.h>

void sdpe_rpc_cbuf_init(sdpe_rpc_cbuf_t *buf, uint32_t size)
{
    buf->size = size;
    cbuf_initialize(&buf->cbuf, round_up_pow2_u32(size));
}

void sdpe_rpc_cbuf_deinit(sdpe_rpc_cbuf_t *buf)
{
    cbuf_deinitialize(&buf->cbuf);
    buf->size = 0;
}

int sdpe_rpc_cbuf_write(sdpe_rpc_cbuf_t *buf, uint8_t *data, uint32_t len)
{
    ASSERT(data != NULL);
    ASSERT(len > 0);

    if (cbuf_space_avail(&buf->cbuf) < (len + 1)) {
        dprintf(WARN, "%s() no enough mem for rpc cbuf\n", __func__);
        return -1;
    }

    cbuf_write_char_nosignal(&buf->cbuf, (char)len);
    cbuf_write(&buf->cbuf, data, len, false);
    return 1;
}

uint32_t sdpe_rpc_cbuf_read(sdpe_rpc_cbuf_t *buf, uint8_t *data)
{
    char data_len;

    ASSERT(data != NULL);

    cbuf_read_char(&buf->cbuf, &data_len, true);
    cbuf_read(&buf->cbuf, data, data_len, false);

    return data_len;
}

