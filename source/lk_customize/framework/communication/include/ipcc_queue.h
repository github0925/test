/*
 * Copyright (c) 2019 Semidrive Semiconductor, Inc.
 * All rights reserved.
 */
#ifndef __IPCC_QUEUE_H__
#define __IPCC_QUEUE_H__

/*! \typedef ipcc_queue_handle_t
    \brief IPCC queue handle type.
*/
typedef void *ipcc_queue_handle_t;

ipcc_queue_handle_t ipcc_queue_create(uint32_t depth, void *user);
int ipcc_queue_destroy(ipcc_queue_handle_t q);
int ipcc_queue_rx_cb(void *payload, int payload_len, unsigned long src, void *priv);
int ipcc_queue_recv(ipcc_queue_handle_t q, unsigned long *src, char *data,
                    uint32_t maxlen, uint32_t *len);
int ipcc_queue_recv_timed(ipcc_queue_handle_t q, unsigned long *src, char *data,
                            uint32_t maxlen, uint32_t *len, lk_time_t timeout);
#endif //__IPCC_QUEUE_H__
