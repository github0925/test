/*
* Copyright (c) 2019 Semidrive Semiconductor Inc.
* All rights reserved.
*
* Description: domain communication high level api for secure
*
*/
#include <reg.h>
#include <stdio.h>
#include <trace.h>
#include <string.h>
#include <stdlib.h>
#include <err.h>
#include <platform/debug.h>
#include <mbox_hal.h>
#include <res.h>

#include "dcf.h"

#if defined(CONFIG_SUPPORT_DCF) && (CONFIG_SUPPORT_DCF == 1)

#if defined(CONFIG_IPCC_RPMSG) && (CONFIG_IPCC_RPMSG == 1)

extern struct ipcc_dev_config ipcc_instances[];
#define MAX_DCF_SERVICES 6

static struct ipcc_service services[MAX_DCF_SERVICES];

ipcc_service_handle_t dcf_get_service_handle(int rproc)
{
    if (rproc > MAX_DCF_SERVICES) {
        dprintf(0, "rproc %d exceed limit service\n", rproc);
        return NULL;
    }

    return &services[rproc];
}

static ipcc_service_handle_t alloc_service_handle(struct ipcc_device *dev)
{
    int rproc = ipcc_device_get_rproc(dev);
    ipcc_service_handle_t hserver = NULL;

    if (!services[rproc].rpdev) {
        hserver = &services[rproc];
        memset(hserver, 0, sizeof(*hserver));
        hserver->rpdev = dev;
        hserver->capabilities = 0xF;
        hserver->rproc = rproc;
    }

    return hserver;
}

void ipcc_start_service(int rproc)
{
    struct ipcc_device *dev = ipcc_device_gethandle(rproc, 5000);
    ipcc_service_handle_t hserver;

    if (!dev)
        return;

    hserver = alloc_service_handle(dev);
    if (!hserver) {
        printf("Not found rproc %d service\n", rproc);
        return;
    }

    if (hserver->ready)
        return;

#if CONFIG_USE_IPCC_RPC
    if (rproc != dcf_get_this_proc()) {
        ipcc_rpc_client_init(&hserver->rpc_client, dev, IPCC_RPC_CLIENT_EPT, IPCC_RPC_MAIN_EPT);
        ipcc_rpc_server_init(&hserver->rpc_server, dev, IPCC_RPC_MAIN_EPT, 0);
        ipcc_rpc_server_start(&hserver->rpc_server);
        hserver->capabilities |= 0x40;
    }
#endif

#if CONFIG_USE_IPCC_TTY
    start_tty_device(dev);
    hserver->capabilities |= 0x20;
#endif
    hserver->rpmsg_dev = dcf_get_raw_device(rproc);

    hserver->ready = true;
}

void start_ipcc_rpc_service(rpc_server_impl_t *tables, int num)
{
    unsigned int i;

    for (i = 0; i < MAX_DCF_SERVICES; i++) {
         dcf_setup_func(&services[i], tables, num);
    }
}

void dcf_register_rpc_implement(rpc_server_impl_t *fn)
{
    unsigned int i;

    for (i = 0; i < MAX_DCF_SERVICES; i++) {
        dcf_setup_func(&services[i], fn, 1);
    }
}

void dcf_register_rpc_implement_detailed(u32 cmd, rpc_server_func_t fn)
{
    unsigned int i;

    for (i = 0; i < MAX_DCF_SERVICES; i++) {
        dcf_setup_func_detail(&services[i], cmd, fn);
    }
}

extern struct sys_property_value dom_properties[];
extern unsigned int dom_properties_num;

static int start_dcf_property(void)
{
    return start_property_service(dom_properties, dom_properties_num);
}

/* to register signal event callback */
#include <sys_diagnosis.h>

void start_dcf_service(void)
{
    unsigned int i;
    struct ipcc_dev_config *conf;

    dprintf(2, "[%lld] %s: enter\n", current_time_hires(), __func__);

    for (i = 0; i < MAX_DCF_SERVICES; i++) {
        conf = &ipcc_instances[i];
        if (conf->devname[0]) {
            ipcc_device_probe(conf);
            ipcc_start_service(conf->rproc);
        } else
            break;
    }
    start_dcf_property();

    /*
     *  Only monitor AP1 & AP2 reboot signal from wdt
     * FIXME: MP core not support sysd feature
     */
    if (dcf_get_this_proc() != DP_CR5_MPC)
        sysd_register_handler(ipcc_device_reset_cb, NULL, 2, WDT5____ovflow_int, WDT6____ovflow_int);

    dprintf(1, "[%lld] %s: started\n", current_time_hires(), __func__);
}

#else
void start_dcf_service(void)
{

}
#endif

#if CONFIG_USE_IPCC_RPC
int dcf_setup_func(struct ipcc_service *hserver, rpc_server_impl_t *tables, int num)
{
    if (!hserver)
        return DCF_ERR_DEV_ID;

    if (!hserver->ready)
        return ERR_NOT_READY;

    if (hserver->rproc == dcf_get_this_proc()) {
        /* Not allowed to loop RPC, skip this */
        return NO_ERROR;
    }

    return ipcc_rpc_setup_implement(&hserver->rpc_server, tables, num);
}

int dcf_setup_func_detail(struct ipcc_service *hserver, u32 cmd, rpc_server_func_t fn)
{
    rpc_server_impl_t fd;

    if (!hserver)
        return DCF_ERR_DEV_ID;

    if (!hserver->ready)
        return ERR_NOT_READY;

    if (hserver->rproc == dcf_get_this_proc()) {
        dprintf(0, "Not allowed to loop RPC\n");
        return DCF_ERR_PARAM;
    }

    fd.command = cmd;
    fd.flags = IPCC_RPC_NO_FLAGS;
    fd.func = fn;

    return ipcc_rpc_setup_implement(&hserver->rpc_server, &fd, 1);
}

int __dcf_call(struct ipcc_service *hserver, rpc_call_request_t *request, rpc_call_result_t *result, lk_time_t timeout)
{
    if (!hserver)
        return DCF_ERR_DEV_ID;

    return ipcc_rpc_client_call(&hserver->rpc_client, request, result, timeout);
}

status_t __dcf_ping(struct ipcc_service *hserver)
{
    rpc_call_result_t result = DCF_RPC_RES_INITVALUE;
    rpc_call_request_t request = DCF_RPC_REQ_INITVALUE;
    status_t ret = 0;

    DCF_INIT_RPC_REQ4(request, IPCC_RPC_REQ_PING, 0xff, 0xff00, 0xff0000, 0xff000000);
    ret = ipcc_rpc_client_call(&hserver->rpc_client, &request, &result, 1000);
    if (ret < 0) {
        dprintf(0, "rpc: call_func:%x fail ret: %d\n", request.cmd, ret);
        return ret;
    }

    ret = result.retcode;
    dprintf(1, "rpc: get result: %x %d %x %x %x\n", result.ack, ret,
                result.result[0], result.result[1], result.result[2]);
    ASSERT(result.ack == IPCC_RPC_ACK_PING);
    ASSERT(ret == NO_ERROR);

    if (memcmp((char*)request.param, (char*)result.result, sizeof(result.result)))
        dprintf(0, "rpc: echo not match\n");

    return ret;
}

status_t __dcf_gettimeofday(struct ipcc_service *hserver)
{
    rpc_call_result_t result = DCF_RPC_RES_INITVALUE;
    rpc_call_request_t request = DCF_RPC_REQ_INITVALUE;
    status_t ret = 0;

    DCF_INIT_RPC_REQ(request, IPCC_RPC_REQ_GETTIMEOFDAY);
    ret = ipcc_rpc_client_call(&hserver->rpc_client, &request, &result, 1000);
    if (ret < 0) {
        dprintf(0, "rpc: call-func:%x fail ret: %d\n", request.cmd, ret);
        return ret;
    }
    ret = result.retcode;
    dprintf(0, "rpc: get result: %x %d %x %x %x\n", result.ack, ret,
                result.result[0], result.result[1], result.result[2]);
    ASSERT(result.ack == IPCC_RPC_ACK_GETTIMEOFDAY);
    ASSERT(ret == NO_ERROR);
    return ret;
}
#endif //if defined(CONFIG_IPCC_RPC) && (CONFIG_IPCC_RPC == 1)

#endif //if defined(CONFIG_SUPPORT_DCF) && (CONFIG_SUPPORT_DCF == 1)

