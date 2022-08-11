/*
 * Copyright (c) 2020 Semidrive Semiconductor, Inc.
 * All rights reserved.
 */

#include <reg.h>
#include <stdlib.h>
#include <stdio.h>
#include <trace.h>
#include <string.h>
#include <err.h>
#include <platform.h>
#include <platform/debug.h>
#include "ipcc_rpc.h"
#include "rpmsg_common.h"

#if CONFIG_USE_IPCC_RPC
#define IPCC_RPCS_CHN_NAME	"rpmsg-ipcc-rpc"
#endif

#define IPC_RPC_DEFAULT_FUNCS     (16)

/* RPC server status */
#define IPCC_RPC_S_RUNNING        (1)
#define IPCC_RPC_S_STOPPED        (0)

int ipcc_rpc_client_init(rpc_client_handle_t hclient,
                       struct ipcc_device *dev, u32 client_id, u32 server_id)
{
    struct ipcc_channel *ichan;

    ichan = ipcc_channel_create(dev, client_id, "rpc-client", false);
    if (!ichan) {
        free(hclient);
        dprintf(0, "rpc: No channel available for client\n");
        return ERR_NO_RESOURCES;
    }
    ipcc_channel_start(ichan, NULL);

    hclient->rpchan = ichan;
    hclient->service_id = server_id;
    dprintf(2, "rpc: channel %p created %p\n", ichan, ichan->parent);

    return 0;
}

void ipcc_rpc_client_destroy(rpc_client_handle_t hclient)
{
    if (!hclient) {
        return;
    }

    if (hclient->rpchan) {
        ipcc_channel_stop(hclient->rpchan);
        ipcc_channel_destroy(hclient->rpchan);
        hclient->rpchan = NULL;
        hclient->service_id = 0;
    }

}

status_t ipcc_rpc_client_call(rpc_client_handle_t hclient, rpc_call_request_t *req, rpc_call_result_t *result, lk_time_t timeout)
{
    status_t ret;
    int ret_size = sizeof(rpc_call_result_t);
    unsigned long src;

    if (!hclient || !hclient->rpchan)
        return DCF_ERR_DEV_ID;

    dprintf(1, "rpc: channel %p called %p\n", hclient->rpchan, hclient->rpchan->parent);

    ret = ipcc_channel_sendto(hclient->rpchan, hclient->service_id,
                              (char*)req, sizeof(rpc_call_request_t),
                              timeout);
    if (ret < 0) {
        dprintf(0, "rpc: ipcc send channel failed: ret: %d\n", ret);
        return ret;
    }
    dprintf(2, "rpc: cmd sent, wait response from srv=%d\n", hclient->service_id);

    /* TODO: use timeout reciever */
    ret = ipcc_channel_recvfrom(hclient->rpchan, &src, (char*)result, &ret_size, timeout);
    if (ret < 0) {
        dprintf(0, "rpc: rx failed ret: %d\n", ret);
        return ret;
    }

    if (hclient->service_id != src) {
        dprintf(0, "rpc: Data from %ld != expected %d, discard\n", src, hclient->service_id);
        return DCF_ERR_BAD_SRVID;
    }

    dprintf(1, "rpc: rx OK from %d\n", hclient->service_id);

    return ret;
}

rpc_call_result_t ipcc_rpc_server_ping(rpc_server_handle_t handle, rpc_call_request_t *req)
{
    rpc_call_result_t result;

    result.ack = IPCC_RPC_ACK_PING;
    result.retcode = NO_ERROR;
    result.result[0] = req->param[0];
    result.result[1] = req->param[1];
    result.result[2] = req->param[2];
    result.result[3] = req->param[3];

    return result;
}

rpc_call_result_t ipcc_rpc_server_gettimeofday(rpc_server_handle_t handle, rpc_call_request_t *req)
{
    rpc_call_result_t result;

    result.ack = IPCC_RPC_ACK_GETTIMEOFDAY;
    result.retcode = NO_ERROR;
    result.result[0] = current_time();
    result.result[1] = 0x00008800;
    result.result[2] = 0x00880000;
    result.result[3] = 0x88000000;

    return result;
}

/* Here defines builtin RPC functions shared by all RPC service */
static rpc_server_impl_t s_rpc_default_funcs[IPCC_RPC_FUNC_MAX] = {
    {IPCC_RPC_REQ_PING, ipcc_rpc_server_ping, IPCC_RPC_NO_FLAGS},
    {IPCC_RPC_REQ_GETTIMEOFDAY, ipcc_rpc_server_gettimeofday, IPCC_RPC_NO_FLAGS},
    {0, },
};

/* Try to find a RPC implement functions registered in the RPC service */
rpc_server_func_t find_func_implement(rpc_server_handle_t handle, u32 command)
{
    unsigned int i;
    rpc_server_impl_t *caller = &s_rpc_default_funcs[0];

    /* firstly find builtin functions */
    for (i = 0; i < ARRAY_SIZE(s_rpc_default_funcs); i++,  caller++) {
        if (caller->command == command) {
            return caller->func;
        }
    }

    /* then find user registered functions */
    if(handle->func_table) {
        caller = handle->func_table;
        for (i = 0; i < handle->func_used; i++, caller++) {
            if (caller->command == command) {
                return caller->func;
            }
        }
    }

    return NULL;
}

rpc_call_result_t ipcc_rpc_server_nofunc(rpc_server_handle_t handle, rpc_call_request_t *req)
{
    rpc_call_result_t result;

    result.ack = req->cmd;
    result.retcode = ERR_NOT_IMPLEMENTED;

    return result;
}

/* Call RPC implement function registered in the RPC service */
static void ipcc_rpc_server_call_reply(rpc_server_handle_t handle, rpc_call_request_t *req, int src)
{
    rpc_server_func_t func;
    rpc_call_result_t reply;
    status_t ret;

    func = find_func_implement(handle, req->cmd);

    if (func) {
        /* Execute this implement function */
        reply = func(handle, req);
        dprintf(2, "rpc: called from client: %d \n", src);

    } else {
        dprintf(0, "rpc: Not found, cmd=0x%x from %d:%d\n",
                   req->cmd, ipcc_device_get_rproc(handle->rpdev), src);
        reply = ipcc_rpc_server_nofunc(handle, req);
    }

    /* if execution is no error, reply to the client in client_id */
    ret = ipcc_channel_sendto(handle->rpchan, src,
                         (char*)&reply, sizeof(rpc_call_result_t), DCF_BLOCK);
    if (ret < 0) {
        dprintf(0, "rpc: reply failed: ret: %d\n", ret);
        return;
    }

    dprintf(1, "rpc: reply cmd: 0x%x to %d done\n", req->cmd, src);
}

static void rpc_server_dispatch(struct ipcc_channel *chan, struct dcf_message *mssg, int len, int src)
{
    rpc_server_handle_t handle = ipcc_channel_get_context(chan);
    rpc_call_request_t request;

    ASSERT(handle);
    ASSERT(mssg);
    ASSERT(len == sizeof(rpc_call_request_t));
    memcpy(&request, mssg, len);

    dprintf(1, "rpc: Receive cmd: 0x%x from %d\n", request.cmd, src);

    if (handle->status == IPCC_RPC_S_RUNNING) {
        ipcc_rpc_server_call_reply(handle, &request, src);
    }
}

int ipcc_rpc_server_init(rpc_server_handle_t handle,
                        struct ipcc_device *dev, int server_id, int threads)
{
    if (!dev) {
        dprintf(0, "Bad ipcc device handle\n");
        return ERR_INVALID_ARGS;
    }

    handle->rpdev = dev;
    handle->status = IPCC_RPC_S_STOPPED;
    handle->rpchan = ipcc_channel_create(dev, server_id, IPCC_RPCS_CHN_NAME, true);
    if (!handle->rpchan) {
        dprintf(0, "rpc: No channel available\n");
        return ERR_NO_RESOURCES;
    }

    if (threads) {
        dprintf(0, "rpc: Thread mode is not supported yet!\n");
        return ERR_NOT_IMPLEMENTED;
    }

    handle->func_max = IPC_RPC_DEFAULT_FUNCS;
    handle->func_used = 0;
    handle->func_table = malloc(IPC_RPC_DEFAULT_FUNCS * sizeof(rpc_server_impl_t));
    if (!handle->func_table) {
        dprintf(ALWAYS, "rpc: failed to alloc function table (svc-%d) \n", server_id);
        return ERR_NO_MEMORY;
    }
    handle->service_id = server_id;
    ipcc_channel_set_context(handle->rpchan, handle);
    ipcc_channel_start(handle->rpchan, rpc_server_dispatch);

    return NO_ERROR;
}

status_t ipcc_rpc_server_start(rpc_server_handle_t handle)
{
    handle->status = IPCC_RPC_S_RUNNING;

    return NO_ERROR;
}

status_t ipcc_rpc_server_stop(rpc_server_handle_t handle)
{
    handle->status = IPCC_RPC_S_STOPPED;

    return NO_ERROR;
}

status_t ipcc_rpc_setup_implement(rpc_server_handle_t handle,
                                         rpc_server_impl_t *tables, int num)
{
    rpc_server_impl_t *ptr = NULL;
    int left = handle->func_max - handle->func_used;

    if (left >= num) {
        ptr = handle->func_table + handle->func_used;
        handle->func_used += num;
    }

    if (!ptr) {
        dprintf(ALWAYS, "rpc: TODO: func table exceed max\n");
        dprintf(0, "rpc: dev=%s srv=%d left=%d func=%d\n",
                    handle->rpdev->config.devname, handle->service_id,
                    left, handle->func_used);

        return ERR_NO_MEMORY;
    }

    memcpy(ptr, tables, sizeof(rpc_server_impl_t) * num);

    dprintf(1, "%s %d installed %d RPC functions \n",
                handle->rpdev->config.devname,
                handle->service_id, handle->func_used);

    return NO_ERROR;
}

void ipcc_rpc_server_exit(rpc_server_handle_t handle)
{

    if (handle->rpc_thread)
        thread_join(handle->rpc_thread, NULL, 1000);

    if (handle->func_table)
        free(handle->func_table);

    if (handle->rpchan)
        ipcc_channel_destroy(handle->rpchan);

    return;
}
