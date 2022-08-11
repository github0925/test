/*
 * Copyright (c) 2018  Semidrive
 *
 */
#include <app.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <err.h>
#include <debug.h>
#include <platform.h>
#include <platform/debug.h>
#include <kernel/semaphore.h>
#include <lib/console.h>
#include <kernel/event.h>
#include <dcf.h>

#if defined(CONFIG_USE_IPCC_RPC) && (CONFIG_USE_IPCC_RPC == 1)
#include <ipcc_rpc.h>

#define TEST_DFAULTE_CLIENT         (IPCC_RRPOC_AP1)
#define TEST_DFAULTE_SERVER         (IPCC_RRPOC_SEC)

#define TEST_RPC_EPT_CLIENT         (2000)
#define TEST_RPC_EPT_SERVER         (2048)

/* this is user-defined requst and ack command */
#define TEST_RPC_SAMPLE_HELLO       (IPCC_RPC_CMD_USERBASE)

static rpc_server_handle_t hserver;
rpc_call_result_t ipcc_rpc_say_hello(rpc_server_handle_t hserver, rpc_call_request_t *request)
{
    rpc_call_result_t result;

    result.ack = TEST_RPC_SAMPLE_HELLO+1;
    result.retcode = NO_ERROR;
    result.result[0] = 0;
    result.result[1] = 0;
    result.result[2] = 0;
    result.result[3] = 0;

    printf("%s called\n", __func__);

    return result;
}

static rpc_server_impl_t s_rpc_my_funcs[] = {
    {TEST_RPC_SAMPLE_HELLO, ipcc_rpc_say_hello, IPCC_RPC_NO_FLAGS},
    {0, },
};

int do_start_ipcc_rpc(int argc, const cmd_args *argv)
{
    int rproc = TEST_DFAULTE_CLIENT;

    printf("%s enter\n", __func__);

    if (hserver)
        return 0;

    if (argc > 1) {
        rproc = argv[1].u;
    }

    printf("to start rpc service to rproc:%d\n", rproc);
//    hserver = ipcc_rpc_server_init(rproc, TEST_RPC_EPT_SERVER, false);
    if (!hserver) {
        printf("%s rpc server init failed\n", __func__);
        return -1;
    }

    printf("to install rpc functions\n");
    ipcc_rpc_setup_implement(hserver, s_rpc_my_funcs, ARRAY_SIZE(s_rpc_my_funcs));

    ipcc_rpc_server_start(hserver);

    printf("%s %d->%d rpc server started\n", __func__, dcf_get_this_proc(), rproc);

    return 0;
}

int do_stop_ipcc_rpc(int argc, const cmd_args *argv)
{
    printf("%s enter\n", __func__);

    ipcc_rpc_server_stop(hserver);

    ipcc_rpc_server_exit(hserver);

    hserver = 0;

    printf("%s done\n", __func__);

    return 0;
}

static rpc_client_handle_t hclient;
int do_ipcc_rpc_hello(int argc, const cmd_args *argv)
{
    rpc_call_request_t request;
    rpc_call_result_t result;
    status_t ret;
    int rproc = TEST_DFAULTE_SERVER;
    int count = 1;

    printf("%s\n", __func__);

    if (argc > 1) {
        rproc = argv[1].u;
    }
    if (argc > 2) {
        count = argv[2].u;
    }

    if (!hclient) {
        hclient = ipcc_rpc_create_client(rproc, TEST_RPC_EPT_CLIENT, TEST_RPC_EPT_SERVER);
    }

    while (count--) {
        DCF_INIT_RPC_REQ(request, TEST_RPC_SAMPLE_HELLO);
        ret = ipcc_rpc_client_call(hclient, &request, &result, 1000);
        if(ret < 0) {
            printf("%s rpc call failed ret: %d\n", __func__, ret);
            break;
        }

        printf("%s ack:%x retcode:%d result:%x %x %x\n", __func__,
                    result.ack, result.retcode, result.result[0],
                    result.result[1], result.result[2]);
    }

    return 0;
}

int do_ipcc_rpc_ping(int argc, const cmd_args *argv)
{
    rpc_call_request_t request;
    rpc_call_result_t result;
    status_t ret;
    int rproc = TEST_DFAULTE_SERVER;
    int count = 1;

    printf("%s\n", __func__);

    if (argc > 1) {
        rproc = argv[1].u;
    }
    if (argc > 2) {
        count = argv[2].u;
    }

    if (!hclient) {
        hclient = ipcc_rpc_create_client(rproc, TEST_RPC_EPT_CLIENT, TEST_RPC_EPT_SERVER);
    }

    while (count--) {
        DCF_INIT_RPC_REQ4(&request, IPCC_RPC_REQ_PING, 0x11223344,
                          0x55667788, 0x99AABBCC, 0xDDEEFF00);
        ret = ipcc_rpc_client_call(hclient, &request, &result, 1000);
        if(ret < 0) {
            printf("%s rpc call failed ret: %d\n", __func__, ret);
            break;
        }

        printf("%s ack:%x retcode:%d result:%x %x %x\n", __func__,
                    result.ack, result.retcode, result.result[0],
                    result.result[1], result.result[2]);
    }
    return 0;
}

int do_rpc_gettimeofday(int argc, const cmd_args *argv)
{
    rpc_call_request_t request;
    rpc_call_result_t result;
    status_t ret;
    int rproc = TEST_DFAULTE_SERVER;
    int count = 1;

    printf("%s\n", __func__);

    if (argc > 1) {
        rproc = argv[1].u;
    }
    if (argc > 2) {
        count = argv[2].u;
    }

    if (!hclient) {
        hclient = ipcc_rpc_create_client(rproc, TEST_RPC_EPT_CLIENT, TEST_RPC_EPT_SERVER);
    }

    while (count--) {
        DCF_INIT_RPC_REQ(request, IPCC_RPC_REQ_GETTIMEOFDAY);
        ret = ipcc_rpc_client_call(hclient, &request, &result, 1000);
        if(ret < 0) {
            printf("%s rpc call failed ret: %d\n", __func__, ret);
            break;
        }

        printf("%s ack:%x retcode:%d result:%x %x %x\n", __func__,
                    result.ack, result.retcode, result.result[0],
                    result.result[1], result.result[2]);
    }

    return 0;
}

void rpc_ut_usage(const char *cmd)
{

    printf("%s start", cmd);
    printf("\t\t:start a ipc rpc service\n");

    printf("%s stop", cmd);
    printf("\t\t:stop a ipc rpc service\n");

    printf("%s hello", cmd);
    printf("\t\t:client call hello in a server by rpc\n");

    printf("%s ping", cmd);
    printf("\t\t:client call, server echo by rpc\n");

    printf("%s gettime", cmd);
    printf("\t\t:client request server's local time\n");

}

int do_rpc_ut_cmd(int argc, const cmd_args *argv)
{
    if (argc < 2) {
        rpc_ut_usage(argv[0].str);
        goto out;
    }

    if (!strcmp(argv[1].str, "start")) {
        do_start_ipcc_rpc(argc-1, &argv[1]);
    }
    if (!strcmp(argv[1].str, "stop")) {
        do_stop_ipcc_rpc(argc-1, &argv[1]);
    }
    if (!strcmp(argv[1].str, "hello")) {
        do_ipcc_rpc_hello(argc-1, &argv[1]);
    }
    if (!strcmp(argv[1].str, "ping")) {
        do_ipcc_rpc_ping(argc-1, &argv[1]);
    }
    if (!strcmp(argv[1].str, "gettime")) {
        do_rpc_gettimeofday(argc-1, &argv[1]);
    }
out:
    return 0;
}

#endif // WITH_LIB_COMMUNICATION==1
