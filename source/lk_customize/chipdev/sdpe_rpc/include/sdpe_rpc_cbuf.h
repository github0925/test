/*
 * sdpe_rpc_cbuf.h
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */

#ifndef _SDPE_RPC_CBUF_H
#define _SDPE_RPC_CBUF_H

#include <lib/cbuf.h>

typedef struct sdpe_rpc_cbuf {
    cbuf_t cbuf;
    uint32_t size;
} sdpe_rpc_cbuf_t;

void sdpe_rpc_cbuf_init(sdpe_rpc_cbuf_t *buf, uint32_t size);
void sdpe_rpc_cbuf_deinit(sdpe_rpc_cbuf_t *buf);
int sdpe_rpc_cbuf_write(sdpe_rpc_cbuf_t *buf, uint8_t *data, uint32_t len);
uint32_t sdpe_rpc_cbuf_read(sdpe_rpc_cbuf_t *buf, uint8_t *data);

#endif
