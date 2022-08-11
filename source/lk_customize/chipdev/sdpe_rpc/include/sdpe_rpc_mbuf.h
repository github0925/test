/*
 * sdpe_rpc_mbuf.h
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */

#ifndef _SDPE_RPC_MBUF_H_
#define _SDPE_RPC_MBUF_H_

/*
 * @brief Initializes rpc mbuf dev.
 * @param null
 * @return init result
 */
#ifdef SUPPORT_SDPE_RPC_DBUF
int sdpe_rpc_mbuf_init(void);
#else
#define sdpe_rpc_mbuf_init()
#endif

/*
 * @brief deinitializes rpc mbuf dev.
 * @param null
 * @return deinit result
 */
#ifdef SUPPORT_SDPE_RPC_DBUF
void sdpe_rpc_mbuf_deinit(void);
#else
#define sdpe_rpc_mbuf_deinit()
#endif

/*
 * @brief get mbuf.
 * @param len tx data len
 * @return tx data addr
 */
#ifdef SUPPORT_SDPE_RPC_DBUF
void *sdpe_rpc_alloc_mbuf(uint32_t len);
#else
#define sdpe_rpc_alloc_mbuf(len)
#endif

/*
 * @brief free mbuf.
 * @param data mbuf ptr
 * @return null
 */
#ifdef SUPPORT_SDPE_RPC_DBUF
void sdpe_rpc_free_mbuf(void *data);
#else
#define sdpe_rpc_free_mbuf(data)
#endif

#endif
