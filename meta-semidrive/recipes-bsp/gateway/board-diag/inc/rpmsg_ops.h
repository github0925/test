/*
 * rpmsg-ops.h
 *
 * Copyright (c) 2020 SemiDrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */

#ifndef _RPMSG_OPS_H
#define _RPMSG_OPS_H

#define SF                              0xF0                                       /* Single frame */
#define MF(nr)                          ((nr) & 0x0F)                              /* Multiple frame, nr: frame number */
#define EF(nr)                          (((nr) & 0x0F) | 0x10)                     /* Ending frame */
#define IS_SF(payload)                  ((payload)->flow_ctrl == 0xF0)             /* Is single frame? */
#define GET_FRAME_NUM(payload)          (((payload)->flow_ctrl) & 0x0F)
#define IS_EF(payload)                  ((((payload)->flow_ctrl) & 0xF0) == 0x10)  /* Is ending frame? */
#define FC_SIZE                         1
#define REMAIN_PACKET_SIZE(size, pos)   ((size) - (pos) + FC_SIZE)

#define RPMSG_HEADER_LEN 16
#define MAX_RPMSG_BUFF_SIZE (512 - RPMSG_HEADER_LEN)

struct rpmsg_payload {
    unsigned char flow_ctrl;
    unsigned char data[0];
} __attribute__((packed));

int open_rpmsg_ep(int ep_num, bool block);
int close_rpmsg_ep(int ep_fd);

#endif
