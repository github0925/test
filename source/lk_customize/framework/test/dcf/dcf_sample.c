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
#include <lib/console.h>
#include <kernel/event.h>
#include <kernel/semaphore.h>

#include <dcf.h>
#include <ipcc_device.h>
#include <ipcc_rpc.h>

#define CONFIG_RPC_UNITTEST         (0)

#define DCF_ADDR_CLIENT             (100U)
#define DCF_ADDR_SERVER             (200U)

#define IPCC_TEST_EPT               (18)
#define IPCC_TEST_EPT_NAME          "ipcc-test"

static bool test_run_flag;
static semaphore_t rx_sema;
static lk_bigtime_t start_time, end_time;

static int client_rx_callback(void *payload, int payload_len,
                           unsigned long src, void *priv)
{
    struct rpmsg_dcf_instance *rpmsg_dev = (struct rpmsg_dcf_instance *)
            priv;
    char recv_str[32] = {0,};
    ASSERT(payload);
    ASSERT(payload_len);

    strncpy(recv_str, payload, payload_len);
    printf("%s receive data: %s from src %d len = %d\n", __func__, recv_str,
           src, payload_len);

    return DCF_RELEASE;
}

static int loopback_rx_callback(void *payload, int payload_len,
                           unsigned long src, void *priv)
{
    struct rpmsg_dcf_instance *rpmsg_master = (struct rpmsg_dcf_instance *)
            priv;
    char recv_str[32] = {0,};

    ASSERT(payload);
    ASSERT(payload_len);

    strncpy(recv_str, payload, payload_len);
    printf("%s receive data: %s from src %d len = %d\n", __func__, recv_str,
           src, payload_len);

    if (0 == strncmp(payload, "exit", 4)) {
        test_run_flag = false;
    }

    sem_post(&rx_sema, true);

    return DCF_RELEASE;
}

static int remote_ept_rx_callback(void *payload, int payload_len,
                           unsigned long src, void *priv)
{
    struct rpmsg_dcf_instance *rpmsg_remote = (struct rpmsg_dcf_instance *)
            priv;
    char recv_str[32] = {0,};

    ASSERT(payload);
    ASSERT(payload_len);

    strncpy(recv_str, payload, payload_len);
    printf("%s receive data: %s from src %d len = %d\n", __func__, recv_str,
           src, payload_len);

    return DCF_RELEASE;
}

#define TEST_IPCC_PING_PAYSZ        (64)
int do_ipcc_ping(int argc, const cmd_args *argv)
{
    struct ipcc_device *dev;
    struct ipcc_channel *chan;
    unsigned long src;
    int ret = 0;
    char send_buf[IPCC_MB_MTU] = {0,};
    char rx_buf[IPCC_MB_MTU] = {0,};
    int len = TEST_IPCC_PING_PAYSZ;
    unsigned long remote = 0;
    lk_bigtime_t start_time, txdone_time, end_time;
    u32 count = 0;
    u32 round = 1;

    if (argc == 1) {
        remote = 1;
    } else {
        remote = argv[1].u;

        if (argc == 3) {
            round = argv[2].u;
        }

        if (argc == 4) {
            round = argv[2].u;
            len = argv[3].u;
        }
    }

    if (len > IPCC_MB_MTU) {
        printf("len %d exceed MTU %d, trim\n", len, IPCC_MB_MTU);
        len = IPCC_MB_MTU;
    }
    memset(send_buf, COMM_MSG_CCM_ECHO, len);
    printf("PING rproc%d %d(%d) bytes of data\n", remote, len, len + 20);

    dev = ipcc_device_gethandle(remote, 1000);
    if (!dev)
        return -1;

    chan = ipcc_channel_create(dev, IPCC_TEST_EPT, IPCC_TEST_EPT_NAME, false);
    if (!chan) {
        dprintf(0, "ping: create channel fail\n");
        return -1;
    }
    ipcc_channel_start(chan, NULL);

    start_time = current_time_hires();
    txdone_time = start_time; // let compiler happy

    while (round--) {
        ret = ipcc_channel_sendto(chan, IPCC_ECHO_EPT, send_buf, len, 100);
        if (ret < 0) {
            printf("ping: send failed %d\n", ret);
            break;
        }
        txdone_time = current_time_hires();

        ret = ipcc_channel_recvfrom(chan, &src, rx_buf, &len, 1000);
        if (ret < 0) {
            printf("ping: wait for reply timeout\n");
            break;
        }
        count++;

        if (strncmp(rx_buf, send_buf, len)) {
            printf("ping: result unexpected %s\n", rx_buf);
            break;
        }
        rx_buf[0] = 0;

    }
    end_time = current_time_hires();

    printf("%d packets transmitted, %d packets received, %d loss\n", count, count, 0);
    if (count == 1)
        printf("tx-trip avg. = %.2f us\n", (float)(txdone_time - start_time)/count);
    printf("round-trip avg. = %.2f us\n", (float)(end_time - start_time)/count);

    ipcc_channel_stop(chan);
    ipcc_channel_destroy(chan);

    return 0;
}

int do_ipcc_iperf(int argc, const cmd_args *argv)
{
    struct ipcc_device *dev;
    struct ipcc_channel *chan;
    int ret = 0;
    char send_buf[IPCC_MB_MTU] = {0,};
    size_t len = TEST_IPCC_PING_PAYSZ;
    unsigned long remote = 0;
    lk_bigtime_t start_time, txdone_time, end_time;
    u32 count = 0;
    u32 round = 100;
    int err_cnt = 0;

    if (argc == 1) {
        remote = 1;
    } else {
        remote = argv[1].u;

        if (argc == 3) {
            round = argv[2].u;
        }

        if (argc == 4) {
            round = argv[2].u;
            len = argv[3].u;
        }
    }

    if (len > IPCC_MB_MTU) {
        printf("len %d exceed MTU %d, trim\n", len, IPCC_MB_MTU);
        len = IPCC_MB_MTU;
    }

    printf("Buffer size rproc%d %d(%d) bytes of data\n", remote, len, len + 20);

    dev = ipcc_device_gethandle(remote, 1000);
    if (!dev)
        return -1;

    chan = ipcc_channel_create(dev, IPCC_TEST_EPT, IPCC_TEST_EPT_NAME, false);
    if (!chan) {
        dprintf(0, "iperf create channel fail\n");
        return -1;
    }
    ipcc_channel_start(chan, NULL);

    memset(send_buf, COMM_MSG_CCM_DROP, len);

    start_time = current_time_hires();

    while (round--) {
        ret = ipcc_channel_sendto(chan, IPCC_ECHO_EPT, send_buf, len, 100);
        if (ret < 0) {
            printf("ipccping: send failed %d\n", ret);
            break;
        }
        count++;
    }
    end_time = current_time_hires();

    printf("%d packets transmitted, %d bytes, %d loss\n", count, count * len, err_cnt);
    printf("throughput avg. = %.2f MB/s\n", (float)(count * len)/(end_time - start_time));
    printf("tx latency avg. = %lld us\n", (end_time - start_time)/count);

    ipcc_channel_stop(chan);
    ipcc_channel_destroy(chan);

    return 0;
}

#include <sys_diagnosis.h>

int do_ipcc_reset_dev(int argc, const cmd_args *argv)
{
    unsigned long remote = IPCC_RRPOC_AP1;
    int signal = WDT5____ovflow_int;

    if (argc != 1)
        remote = argv[1].u;

    if (remote == IPCC_RRPOC_AP2)
        signal = WDT6____ovflow_int;

    ipcc_device_reset_cb(signal, NULL);

    return 0;
}

int do_ipcc_listdev(int argc, const cmd_args *argv)
{
    ipcc_device_show();
    return 0;
}

/* Here's example to implement RPC funtion in server side */
static rpc_call_result_t rpc_default_handler(rpc_server_handle_t hserver, rpc_call_request_t *request)
{
    rpc_call_result_t result;

    printf("%s cmd 0x%x called \n", __func__, request->cmd);
    /* Here starts the function body */




    /* return the result struct with actual data */
    result.ack = request->cmd;
    result.retcode = NO_ERROR;
    result.result[0] = 0x0;
    result.result[1] = 0x1;
    result.result[2] = 0x2;
    result.result[3] = 0x3;

    return result;
}

/* Here's example to register RPC implement in server side */
int do_ipcc_rpc_customize(int argc, const cmd_args *argv)
{
    static rpc_server_impl_t s_myfuncs[] = {
        {SYS_RPC_REQ_UNAME,   rpc_default_handler, IPCC_RPC_NO_FLAGS},
        {SYS_RPC_REQ_LIST,    rpc_default_handler, IPCC_RPC_NO_FLAGS},
        {SYS_RPC_REQ_LOG_ON,  rpc_default_handler, IPCC_RPC_NO_FLAGS},
        {SYS_RPC_REQ_LOG_OFF, rpc_default_handler, IPCC_RPC_NO_FLAGS},
    };
    unsigned int num = ARRAY_SIZE(s_myfuncs);

    rpc_server_impl_t myfunc_single = {
        SYS_RPC_REQ_INQUIRY, rpc_default_handler, IPCC_RPC_NO_FLAGS,
    };

    /* register a bundle of functions */
    start_ipcc_rpc_service(s_myfuncs, num);

    /* register a function with descriptor */
    dcf_register_rpc_implement(&myfunc_single);

    /* register single function */
    dcf_register_rpc_implement_detailed(MOD_RPC_REQ_TEST, rpc_default_handler);

    return 0;
}

int do_dcf_rpc_ping(int argc, const cmd_args *argv)
{
    status_t ret;
    int rproc = IPCC_RRPOC_SEC;
    u32 count = 0;
    u32 round = 1;

    if (argc > 1) {
        rproc = argv[1].u;
    }

    if (argc > 2) {
        round = argv[2].u;
    }
    ret = 0;
    start_time = current_time_hires();

    while (round--) {
        ret = dcf_ping(rproc);
        if(ret < 0) {
            printf("%s failed ret: %d\n", __func__, ret);
            break;
        }
        count++;
    }

    end_time = current_time_hires();

    if (ret == 0) {
        printf("%d RPC called, %d RPC result received, %d loss\n", count, count, 0);
        printf("per RPC call avg. = %.2f us\n", (float)(end_time - start_time)/count);
    } else
        printf("Something wrong with RPC calling\n");

    return 0;
}

int do_dcf_rpc_gettimeofday(int argc, const cmd_args *argv)
{
    status_t ret;
    int rproc = IPCC_RRPOC_SEC;
    int count = 1;

    if (argc > 1) {
        rproc = argv[1].u;
    }

    if (argc > 2) {
        count = argv[2].u;
    }

    while (count--) {
        ret = dcf_gettimeofday(rproc);
        if(ret < 0) {
            printf("%s failed ret: %d\n", __func__, ret);
            break;
        }
    }
    return 0;
}

int do_dcf_rpc_get_uname(int argc, const cmd_args *argv)
{
    rpc_call_request_t request;
    rpc_call_result_t result;
    status_t ret;
    int rproc = IPCC_RRPOC_SEC;
    int count = 1;

    if (argc > 1) {
        rproc = argv[1].u;
    }

    if (argc > 2) {
        count = argv[2].u;
    }

    while (count--) {
        DCF_INIT_RPC_REQ(request, SYS_RPC_REQ_UNAME);
        ret = dcf_call(rproc, &request, &result, 1000);
        if(ret < 0) {
            printf("%s call failed ret: %d\n", __func__, ret);
            break;
        }

        if (result.retcode == ERR_NOT_IMPLEMENTED) {
            printf("%s command not supported, please run rpc_reg_svc in server side\n", __func__);
            break;
        }
        printf("%s ack:%x retcode:%d result:%x %x %x\n", __func__,
                    result.ack, result.retcode, result.result[0],
                    result.result[1], result.result[2]);
    }
    return 0;
}

int do_dcf_rpc_failsafe(int argc, const cmd_args *argv)
{
    rpc_call_request_t request;
    rpc_call_result_t result;
    status_t ret;
    int rproc = IPCC_RRPOC_SEC;
    int count = 1;

    if (argc > 1) {
        rproc = argv[1].u;
    }

    if (argc > 2) {
        count = argv[2].u;
    }

    while (count--) {
        // invalid command to test
        DCF_INIT_RPC_REQ(request, 0x123232aa);
        ret = dcf_call(rproc, &request, &result, 1000);
        if(ret < 0) {
            printf("%s rpc call failed ret: %d\n", __func__, ret);
            break;
        }

        if (result.retcode == ERR_NOT_IMPLEMENTED) {
            printf("%s command not supported, please run rpc_reg_svc in server side\n", __func__);
            break;
        }
    }
    return 0;
}

#if CONFIG_USE_SYS_PROPERTY
#include <property.h>

void start_sample_property(void)
{
    start_property_service(NULL, 0);
}

void property_usage(const char *cmd)
{
    printf("%s [id] [val]", cmd);
    printf("\t:Set a property [id] with value [val]\n");
    printf("\t\t\t:Use getprop to show all property id\n");
}

int do_set_property(int argc, const cmd_args *argv)
{
    status_t ret = 0;
    int round = 1;
    int count = 0;
    int property_id = 1;
    int val = 0;
    int readback;

    if (argc < 3) {
        property_usage(argv[0].str);
        return 0;
    }

    property_id = argv[1].i;
    val = argv[2].i;
    if (argc == 4)
        round = argv[3].i;

    start_sample_property();

    start_time = current_time_hires();
    while (count < round) {
        count++;
        ret = system_property_set(property_id, val);
        if (ret < 0)
            break;

        system_property_get(property_id, &readback);
    }
    end_time = current_time_hires();

    if (ret == 0)
        printf("set property(%d) round %d in %lld us\n", property_id, round,
                end_time - start_time);

    return ret;
}

static int property_observer[DMP_ID_MAX];
static void property_changed(int property, int old, int new, void *observer)
{
    status_t ret = 0;
    int status = 0;
    int i;

    printf("property(%d) onchanged %d->%d observer=%s\n", property, old, new, observer);

    /* Test reenter set/get property */
    printf("Test property set/get in callback\n");
    for (i = 0;i < DMP_ID_MAX; i++) {
        ret = system_property_get(i, &status);
        if (ret == 0) {
            printf("callback get property-%d = 0x%x\n", i, status);
        }
    }
    ret = system_property_set(property, 0);
    ret = system_property_set(property, new);
    ret = system_property_get(property, &status);
    if (ret == 0) {
        printf("callback get property-%d = 0x%x\n", property, status);
        printf("Test property set/get successful\n");
    }
}

int do_get_property(int argc, const cmd_args *argv)
{
    status_t ret = 0;
    int status = 0;
    int round = 1;
    int count = 0;
    int property_id = 1;

    start_sample_property();

    if (argc == 1) {
        int i;

        for (i = 0;i < DMP_ID_MAX; i++) {
            start_time = current_time_hires();
            ret = system_property_get(i, &status);
            end_time = current_time_hires();
            if (ret == 0) {
                printf("property-%d = 0x%x in %lld us\n", i, status, end_time - start_time);
                system_property_observe(i, property_changed, (void*)"shell");
            }
        }
        return 0;
    }

    if (argc == 2) {
        property_id = argv[1].i;
    }

    if (argc == 3) {
        property_id = argv[1].i;
        round = argv[2].i;
    }

    while (count < round) {
        count++;
        ret = system_property_get(property_id, &status);
        if (ret < 0)
            break;
    }

    if (ret == 0)
        printf("get property(%d) round %d\n", property_id, round);

    return 0;
}
#endif

int do_rpc_ut_cmd(int argc, const cmd_args *argv);

static void ipcc_usage(const char *cmd)
{
    cmd_args argv;

    printf("Usage: here's a IPCC debug and unit test toolbox\n");
    printf("%s commands:\n\n", cmd);

    printf("%s ping", cmd);
    printf("\t\t:ping remote proc ipcc service\n");

    printf("%s iperf", cmd);
    printf("\t\t:performance benchmark\n");

    printf("%s reset", cmd);
    printf("\t\t:reset device connection\n");

    printf("%s rpc_echo", cmd);
    printf("\t\t:RPC call remote echo function\n");

    printf("%s rpc_gettimeofday", cmd);
    printf("\t:RPC call remote to get domain time\n");

    printf("%s rpc_reg_svc", cmd);
    printf("\t:register a set of customized RPC func in server\n");

    printf("%s rpc_uname", cmd);
    printf("\t\t:RPC call remote to get the domain uname\n");

    printf("%s rpc_failsafe", cmd);
    printf("\t:RPC call remote a not-exist function\n");

#if CONFIG_USE_SYS_PROPERTY
    printf("%s setprop", cmd);
    printf("\t\t:RPC call to set property of domain\n");
    argv.str = "     setprop";
    do_set_property(0, &argv);

    printf("%s getprop", cmd);
    printf("\t\t:RPC call to get property of domain\n");
#endif

#if CONFIG_RPC_UNITTEST
    printf("%s rpc_ut", cmd);
    printf("\t\t:RPC call unit test extension\n");
    argv.str = "     rpc_ut";
    do_rpc_ut_cmd(0, &argv);
#endif

}

int do_ipcc_cmd(int argc, const cmd_args *argv)
{
    if (argc < 2) {
        ipcc_usage(argv[0].str);
        goto out;
    }

    if (!strcmp(argv[1].str, "ping")) {
        do_ipcc_ping(argc-1, &argv[1]);
    }
    if (!strcmp(argv[1].str, "iperf")) {
        do_ipcc_iperf(argc-1, &argv[1]);
    }
    if (!strcmp(argv[1].str, "reset")) {
        do_ipcc_reset_dev(argc-1, &argv[1]);
    }
    if (!strcmp(argv[1].str, "list")) {
        do_ipcc_listdev(argc-1, &argv[1]);
    }
    if (!strcmp(argv[1].str, "rpc_echo")) {
        do_dcf_rpc_ping(argc-1, &argv[1]);
    }
    if (!strcmp(argv[1].str, "rpc_reg_svc")) {
        do_ipcc_rpc_customize(argc-1, &argv[1]);
    }
    if (!strcmp(argv[1].str, "rpc_gettimeofday")) {
        do_dcf_rpc_gettimeofday(argc-1, &argv[1]);
    }
    if (!strcmp(argv[1].str, "rpc_uname")) {
        do_dcf_rpc_get_uname(argc-1, &argv[1]);
    }
    if (!strcmp(argv[1].str, "rpc_failsafe")) {
        do_dcf_rpc_failsafe(argc-1, &argv[1]);
    }
#if CONFIG_USE_SYS_PROPERTY
    if (!strcmp(argv[1].str, "setprop")) {
        do_set_property(argc-1, &argv[1]);
    }
    if (!strcmp(argv[1].str, "getprop")) {
        do_get_property(argc-1, &argv[1]);
    }
#endif
#if CONFIG_RPC_UNITTEST
    if (!strcmp(argv[1].str, "rpc_ut")) {
        do_rpc_ut_cmd(argc-1, &argv[1]);
    }
#endif

out:
    return 0;
}

#if defined(WITH_LIB_CONSOLE)
STATIC_COMMAND_START
STATIC_COMMAND("ipcc", "ipcc toolbox", do_ipcc_cmd)
STATIC_COMMAND_END(dcf_sample);
#endif

APP_START(dcf_sample)
.flags = 0
         APP_END
