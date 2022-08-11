/*
 * sdpe_rpc_l2.h
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */

#ifndef _SDPE_RPC_L2_H_
#define _SDPE_RPC_L2_H_

typedef struct sdpe_rpc_l2_instance {
    struct sdpe_rpc_transport_dev *transport;
    uint16_t rx_buf_len;
    uint16_t rx_offset;
    uint16_t rx_sn;
    uint16_t reserve;
    uint8_t  *rx_buffer;
} sdpe_rpc_l2_instance_t;

int sdpe_rpc_l2_init(struct sdpe_rpc_l2_instance *rpc_l2,
                     struct sdpe_rpc_transport_dev *transport);
int sdpe_rpc_l2_uninit(struct sdpe_rpc_l2_instance *rpc_l2);
int sdpe_rpc_l2_send(struct sdpe_rpc_l2_instance *rpc_l2,
                     uint8_t *head, uint8_t head_len,
                     uint8_t sdu_num, va_list arg);
int sdpe_rpc_l2_recv(struct sdpe_rpc_l2_instance *rpc_l2,
                     uint8_t *data, uint32_t *data_len);
#endif
