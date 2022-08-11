/*
 * Copyright (c) 2020 Semidrive Semiconductor, Inc.
 * All rights reserved.
 */
#ifndef __IPCC_RPC_H__
#define __IPCC_RPC_H__

#include "ipcc_device.h"

#define IPCC_RPC_MAX_PARAMS     (8)
#define IPCC_RPC_MAX_RESULT     (4)
#define IPCC_RPC_FUNC_MAX       (16)
#define IPCC_RPC_NO_FLAGS       (0)

/* Here defines default RPC cmd */
#define IPCC_RPC_REQ_PING           (0x1000)
#define IPCC_RPC_ACK_PING           (IPCC_RPC_REQ_PING+1)
#define IPCC_RPC_REQ_GETTIMEOFDAY   (0x1002)
#define IPCC_RPC_ACK_GETTIMEOFDAY   (IPCC_RPC_REQ_GETTIMEOFDAY+1)
/* Here defines system level RPC cmd */
#define SYS_RPC_REQ_BASE            (0x2000)
#define SYS_RPC_REQ_SET_PROPERTY    (SYS_RPC_REQ_BASE + 0)
#define SYS_RPC_REQ_GET_PROPERTY    (SYS_RPC_REQ_BASE + 1)
#define SYS_RPC_REQ_UNAME           (SYS_RPC_REQ_BASE + 2)
#define SYS_RPC_REQ_LIST            (SYS_RPC_REQ_BASE + 3)
#define SYS_RPC_REQ_LOG_ON          (SYS_RPC_REQ_BASE + 4)
#define SYS_RPC_REQ_LOG_OFF         (SYS_RPC_REQ_BASE + 5)
#define SYS_RPC_REQ_INQUIRY         (SYS_RPC_REQ_BASE + 6)
	/* Here defined module level RPC cmd */
#define MOD_RPC_REQ_BASE            (0x3000)
#define MOD_RPC_REQ_TEST            (MOD_RPC_REQ_BASE + 0)
#define MOD_RPC_REQ_OPEN_DC         (MOD_RPC_REQ_BASE + 1)
#define MOD_RPC_REQ_CLOS_DC         (MOD_RPC_REQ_BASE + 2)
#define MOD_RPC_REQ_SET_DCPARAM     (MOD_RPC_REQ_BASE + 3)
#define MOD_RPC_REQ_GET_DCPARAM     (MOD_RPC_REQ_BASE + 4)
/* Here defined Sample RPC cmd */
#define MOD_RPC_REQ_SSP_IOCTL       (MOD_RPC_REQ_BASE + 6)
	/* Here defined backlight RPC cmd */
#define MOD_RPC_REQ_BL_IOCTL        (MOD_RPC_REQ_BASE + 8)
	/* Here defined safe touch screen cmd */
#define MOD_RPC_REQ_STS_IOCTL       (MOD_RPC_REQ_BASE + 10)
	/* Here defined display controller cmd */
#define MOD_RPC_REQ_DC_IOCTL        (MOD_RPC_REQ_BASE + 16)
	/* Here defined camera controller cmd */
#define MOD_RPC_REQ_SCS_IOCTL       (MOD_RPC_REQ_BASE + 24)
    /* Here defined audio service cmd */
#define MOD_RPC_REQ_AUDIO_SERVICE   (MOD_RPC_REQ_BASE + 32)
/* Safe CAN proxy ioctl RPC cmd */
#define MOD_RPC_REQ_CP_IOCTL        (MOD_RPC_REQ_BASE + 0x40)

typedef struct _rpc_call_request_ {
    struct dcf_message hdr;
    u32         cmd;
    u32         param[IPCC_RPC_MAX_PARAMS];
} rpc_call_request_t;

typedef struct _rpc_call_result_ {
    struct dcf_message hdr;
    u32         ack;
    u32         retcode;
    u32         result[IPCC_RPC_MAX_RESULT];
} rpc_call_result_t;

#define DCF_RPC_REQLEN         ((IPCC_RPC_MAX_PARAMS + 4)*sizeof(u32))
#define DCF_RPC_REQ_INITVALUE  {{0, }, 0,}
#define DCF_RPC_RES_INITVALUE  {0,}

#define DCF_INIT_RPC_HDR(msg, sz) \
		DCF_MSG_INIT_HDR((msg), COMM_MSG_RPCALL, \
		sz, DCF_MSGF_STD)

#define DCF_INIT_RPC_REQ(req, op) \
		({DCF_INIT_RPC_HDR(&(req).hdr, 4); \
		(req).cmd = op;})

#define DCF_INIT_RPC_REQ1(req, op, param1) \
		({DCF_INIT_RPC_HDR(&(req).hdr, 8); \
		(req).cmd = op; \
		(req).param[0] = param1;})

#define DCF_INIT_RPC_REQ4(req, op, param1, param2, param3, param4) \
		({DCF_INIT_RPC_HDR(&(req).hdr, 20); \
		(req).cmd = op;          \
		(req).param[0] = param1; \
		(req).param[1] = param2; \
		(req).param[2] = param3; \
		(req).param[3] = param4;})

#define DCF_RPC_PARAM(req, type) (type *)(&req.param[0])

typedef struct ipcc_rpc_client {
    u32         service_id;
    struct ipcc_channel *rpchan;

} *rpc_client_handle_t;

typedef struct ipcc_rpc_server *rpc_server_handle_t;

typedef rpc_call_result_t (*rpc_server_func_t)(rpc_server_handle_t hserver,
                                               rpc_call_request_t *request);

typedef struct _rpc_server_impl_ {
    u32         command;
    rpc_server_func_t   func;
    u32         flags;

} rpc_server_impl_t;

typedef struct ipcc_rpc_server {
    u32                 service_id;
    struct ipcc_device  *rpdev;
    struct ipcc_channel *rpchan;
    u32                 status;

    rpc_server_impl_t   *func_table;
    u32                 func_max;
    u32                 func_used;
    thread_t            *rpc_thread;

} *rpc_server_handle_t;

status_t ipcc_rpc_client_init(rpc_client_handle_t hclient,
                        struct ipcc_device *dev, u32 client_id, u32 server_id);

status_t ipcc_rpc_client_call(rpc_client_handle_t hclient,
                                rpc_call_request_t *request,
                                rpc_call_result_t *result, lk_time_t timeout);

status_t ipcc_rpc_server_init(rpc_server_handle_t handle,
                        struct ipcc_device *dev, int server_id, int threads);

status_t ipcc_rpc_server_start(rpc_server_handle_t hserver);

status_t ipcc_rpc_server_stop(rpc_server_handle_t hserver);

status_t ipcc_rpc_setup_implement(rpc_server_handle_t hserver,
                                         rpc_server_impl_t *tables, int num);
void ipcc_rpc_server_exit(rpc_server_handle_t hserver);

#endif //__IPCC_RPC_H__
