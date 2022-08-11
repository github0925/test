/*
 * Copyright (c) 2019 Semidrive Semiconductor, Inc.
 * All rights reserved.
 */

#ifndef __SEMIDRIVE_DCF_PROTO_H__
#define __SEMIDRIVE_DCF_PROTO_H__

/* global definitions applied for all DCF participator */
#define DCF_MSG_MAX_LEN		(128)
#define DCF_NAM_MAX_LEN		(16)

typedef enum {
	COMM_MSG_INVALID = 0,
	COMM_MSG_CORE,
	COMM_MSG_RPCALL,
	COMM_MSG_DISPLAY,
	COMM_MSG_CAN,
	COMM_MSG_TOUCH,
	COMM_MSG_BACKLIGHT,
	COMM_MSG_STORAGE,
	COMM_MSG_POWER,
	COMM_MSG_CAMERA,
	COMM_MSG_LOGGER,
	COMM_MSG_DEBUGGER,
	COMM_MSG_PROPERTY,
	COMM_MSG_SVCMGR,
	COMM_MSG_FIREWALL,
	COMM_MSG_REBOOT,
	/* Communication Control Message CMD_ID */
	COMM_MSG_CCM_BASE = 0xA0,
	COMM_MSG_CCM_ECHO = 0xA5,
	COMM_MSG_CCM_ACK  = 0xA6,
	COMM_MSG_CCM_DROP = 0xAA,
	COMM_MSG_MAX,
} dcf_msg_type;

/* DCF message definition
 * msg base header is 4 B long
 * msg extention header is 4 bytes indicated by opflag
 * msg body 0 ~ 4K-1 bytes data filled by use case
 */
struct dcf_msg_extend {
	union {
		u32 dat32;
		struct {
			u8 src; 	/* app-id for source, 0 if not used */
			u8 dst; 	/* app-id for destination, 0 if not used */
		} addr;
	} u;
} __attribute__((__packed__));

struct dcf_message {
	u32 msg_type : 8;	/* dcf_msg_type */
	/* Optional features
		SHORT: 1 msg without body
		FRAGMENT: 1
		SYNC: 1
		ACK:  1
		CKSM: 1, checksum exist in the end of packet
	*/
	u32 opflags  : 8;
	u32 msg_len  : 12;	/* msg body len */
	u32 reserved : 4;	/* future use */
	struct dcf_msg_extend option[1];  /* msg option if exist indicated by flag */
	u8 data[0];			/* payload start */
} __attribute__((__packed__));

#define DCF_MSGF_STD		(0x00)
#define DCF_MSGF_OPT		(0x01)
#define DCF_MSGF_SHT		(0x02)
#define DCF_MSGF_FRA		(0x04)
#define DCF_MSGF_SYN		(0x08)
#define DCF_MSGF_ACK		(0x10)
#define DCF_MSGF_CKS		(0x20)

#define DCF_MSG_SIZE(msg)	(msg->msg_len)
#define DCF_MSG_DATA(msg, type)	((type *)&msg->data[0])
#define DCF_MSG_TYPE(msg) 	(msg->msg_type)
#define DCF_MSG_HLEN		(sizeof(struct dcf_message))
#define DCF_MSG_MAX_DLEN	(DCF_MSG_MAX_LEN-sizeof(struct dcf_message))
#define DCF_MSG_OPT(msg) 	(&msg->option[0])
#define DCF_MSGE_ASRC(msg)	(msg)->option[0].u.addr.src
#define DCF_MSGE_ADST(msg)	(msg)->option[0].u.addr.dst

#define DCF_MSG_INIT_HDR(msg, type, len, flags) \
		({msg->msg_type = type; \
		msg->opflags  = flags; \
		msg->msg_len  = len; \
		msg->option[0].u.dat32 = 0x0;})

/* type:COMM_MSG_RPCALL message format
 * the RPC message struct defined ipcc_rpc.h
 */


#endif /* __SEMIDRIVE_DCF_PROTO_H__ */
