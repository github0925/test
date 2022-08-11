/*
 * SEMIDRIVE Copyright Statement
 * Copyright (c) SEMIDRIVE. All rights reserved
 *
 * This software and all rights therein are owned by SEMIDRIVE, and are
 * protected by copyright law and other relevant laws, regulations and
 * protection. Without SEMIDRIVE's prior written consent and/or related rights,
 * please do not use this software or any potion thereof in any form or by any
 * means. You may not reproduce, modify or distribute this software except in
 * compliance with the License. Unless required by applicable law or agreed to
 * in writing, software distributed under the License is distributed on
 * an "AS IS" basis, WITHOUT WARRANTIES OF ANY KIND, either express or implied.
 *
 * You should have received a copy of the License along with this program.
 * If not, see <http://www.semidrive.com/licenses/>.
 */

#include <stdlib.h>
#include <assert.h>
#include <bits.h>
#include <debug.h>
#include <stdio.h>
#include <err.h>
#include <kernel/thread.h>
#include <kernel/event.h>
#include <lk/init.h>
#include <lib/console.h>
#include <lib/bytes.h>
#include <lib/reg.h>
#include <app.h>

/* must include this for ipcc interface */
#include <dcf.h>

/* must include this for thread priority
 * set thread proirity is system policy
 */
#include <sys_priority.h>

/* include local headers */
#include "sample_service_api.h"
#include "sample_service_config.h"

/*
 * This is RPMSG channel info:
 * remote processor, endpoint and name
 * the endpoint exist in Linux rpmsg sysfs:
 * /sys/bus/rpmsg/devices/soc:ipcc@[0|1].[EPT-NAME].-1.[EPT]
 * name should be less than 16 bytes
 */
#define REMOTE_PROCESSOR             (DP_CA_AP1)

#ifndef SRV_SAMPLE_EPT_NAME
#define SRV_SAMPLE_EPT_NAME          "safety,ssd"
#endif

#ifndef SRV_SAMPLE_EPT
#define SRV_SAMPLE_EPT               (48)
#endif

/* VENDOR ID 1 is reserved for Semidrive */
#define SAMPLE_VENDOR_ID             (1)
/* VERSION # self-defined */
#define SAMPLE_VERSION               (0)

/*
 * service thread stuff
 */
#define SAMPLE_INIT_LEVEL            (LK_INIT_LEVEL_PLATFORM + 1)
#define SAMPLE_SERVICE_STACK_SIZE    DEFAULT_STACK_SIZE
#define WAIT_RPMSG_DEV_TIMEOUT       (10000U)

/* HERE IS RPMSG MTU (may 492 Bytes) */
#define RPMSG_MTU                   (DCF_BUFFER_PAYLOAD_SIZE)
#define PAYLOAD_MIN_SIZE            (1)
#define PAYLOAD_MAX_SIZE            (RPMSG_MTU - sizeof(struct payload_t))
#define NUM_PAYLOADS                (PAYLOAD_MAX_SIZE/PAYLOAD_MIN_SIZE)

/* HERE IS service main object */
struct sample_service_info_t {
    bool binitialized;
    event_t initial_event;

    struct ipcc_channel *chan;
    int status;

    /* Receive buffer */
    struct payload_t *payload;
    int payload_len;

    /* vendor fill these data */
    u16 vendor_id;
    void *vendor_priv;
    u32 capability;

    /* Implement RPC function in need
     * These ops transit the status of service
     */
    int (*get_version)(struct sample_service_info_t *ts);
    int (*get_config)(struct sample_service_info_t *ts, u8 *cfg, u16 maxlen);
    int (*set_config)(struct sample_service_info_t *ts, u8 *cfg, u16 len);
    int (*open)(struct sample_service_info_t *ts);
    int (*close)(struct sample_service_info_t *ts);
    int (*start)(struct sample_service_info_t *ts);
    int (*stop)(struct sample_service_info_t *ts);
};

static struct sample_service_info_t sample_service_info;

static int sample_sendto_client(struct sample_service_info_t *ssinfo,
        unsigned long src, char *t_payload, uint16_t t_payload_len)
{
    return ipcc_channel_sendto(ssinfo->chan, src, t_payload,
                        t_payload_len, WAIT_RPMSG_DEV_TIMEOUT);
}

static int sample_message_process(struct sample_service_info_t *ssinfo)
{
    struct payload_t *payload;

    /*
     * TODO:
     */

    payload = (struct payload_t *) ssinfo->payload;
    dprintf(ALWAYS, "%s: Received %d bytes %x %x %x\n", __func__,
         ssinfo->payload_len, payload->magic,payload->num, payload->size);

    return 0;
}

static int sample_main_task(void *token)
{
    struct sample_service_info_t *ssinfo = token;
    struct ipcc_device *dev;
    unsigned long src;
    int ret = 0;

    ssinfo->payload = malloc(RPMSG_MTU);
    if (!ssinfo->payload) {
        printf("ssinfo alloc mem for payload fail\n");
        return -1;
    }
    memset(ssinfo->payload, 0, RPMSG_MTU);

    dev = ipcc_device_gethandle(REMOTE_PROCESSOR, WAIT_RPMSG_DEV_TIMEOUT);
    if (!dev) {
        free(ssinfo->payload);
        dprintf(ALWAYS, "ipcc device not enabled, exit\n", REMOTE_PROCESSOR);
        return ERR_NOT_FOUND;
    }
    /* Create rpmsg channel and endpoint */
    ssinfo->chan = ipcc_channel_create(dev, SRV_SAMPLE_EPT,
                                        SRV_SAMPLE_EPT_NAME, true);
    if (ssinfo->chan)
        ipcc_channel_start(ssinfo->chan, NULL);

    event_signal(&ssinfo->initial_event, false);

    while (1) {
        if (ssinfo->status == SSP_ST_INVALID)
            break;
        /*
         * receive rpmsg payload with maximux size
         */
        ssinfo->payload_len = RPMSG_MTU;
        ret = ipcc_channel_recvfrom(ssinfo->chan, &src, (char*)ssinfo->payload,
                            &ssinfo->payload_len, WAIT_RPMSG_DEV_TIMEOUT);
        if (ret < 0) {
            continue;
        }

        /*
         * process rpmsg payload
         * the payload format should be aligned with client side
         */
        sample_message_process(ssinfo);

        /*
         * send back payload as echo command for the sample,
         * TODO: Here add user-defined content
         */
        sample_sendto_client(ssinfo, src, (char*)ssinfo->payload,
                             ssinfo->payload_len);

    }

    free(ssinfo->payload);
    ipcc_channel_destroy(ssinfo->chan);

    return 0;
}

/*
 * The callback is for AMP RPC from kernel mode,
 * ignore here if client rpmsg endpoint is from user land application
 */
static rpc_call_result_t sample_service_ioctl_cb(rpc_server_handle_t hserver,
        rpc_call_request_t *request)
{
    struct sample_service_info_t *ssinfo = &sample_service_info;
    rpc_call_result_t result = {0,};
    struct sample_request *ctl = (struct sample_request *) &request->param[0];
    struct sample_reply *r = (struct sample_reply *) &result.result[0];

    if (!request) {
        result.retcode = ERR_INVALID_ARGS;
        dprintf(ALWAYS, "%s:instance=%#x, request err\n", __func__, ctl->instance);
        return result;
    }

    result.ack = request->cmd;

    switch (ctl->op) {
        case SSP_OP_GET_VERSION:
            if (ssinfo->get_version)
                ssinfo->get_version(ssinfo);

            result.retcode = 0;
            break;

        case SSP_OP_GET_CONFIG:
            /* TODO: define the config data struct */
            result.retcode = 0;
            break;

        case SSP_OP_OPEN:
            if (ssinfo->open)
                ssinfo->open(ssinfo);

            ssinfo->status = SSP_ST_STOPPED;
            result.retcode = 0;
            break;
        case SSP_OP_CLOSE:
            if (ssinfo->close)
                ssinfo->close(ssinfo);

            ssinfo->status = SSP_ST_IDLE;
            result.retcode = 0;
            break;
        case SSP_OP_START:
            if (ssinfo->start)
                ssinfo->start(ssinfo);

            ssinfo->status = SSP_ST_RUNNING;
            result.retcode = 0;
            break;
        case SSP_OP_STOP:
            if (ssinfo->stop)
                ssinfo->stop(ssinfo);

            ssinfo->status = SSP_ST_STOPPING;
            result.retcode = 0;
            break;
        default:
            result.retcode = ERR_INVALID_ARGS;
            dprintf(ALWAYS, "%s:instance=%#x, op err\n", __func__, ctl->instance);
            break;
    }

    dprintf(ALWAYS, "%s:instance=%#x, cmd=%d done\n", __func__, ctl->instance, ctl->op);
    return result;
}

void sample_service_init(void)
{
    struct sample_service_info_t *ssinfo = &sample_service_info;
    rpc_server_impl_t devfuncs[] = {
        {MOD_RPC_REQ_SSP_IOCTL, sample_service_ioctl_cb, IPCC_RPC_NO_FLAGS},
    };

    thread_t *mthread;
    int ret;

    if (ssinfo->binitialized)
        return;

    start_ipcc_rpc_service(devfuncs, ARRAY_SIZE(devfuncs));

    event_init(&ssinfo->initial_event, false, EVENT_FLAG_AUTOUNSIGNAL);
    mthread = thread_create("sampled", sample_main_task, ssinfo, THREAD_PRI_SAMPLE, SAMPLE_SERVICE_STACK_SIZE);
    thread_detach_and_resume(mthread);

    ret = event_wait_timeout(&ssinfo->initial_event, WAIT_RPMSG_DEV_TIMEOUT);
    if (ret < 0)
        return;

    ssinfo->status = SSP_ST_IDLE;
    ssinfo->binitialized = true;
}

void sample_service_entry(uint level)
{
    sample_service_init();
}

/* uncomment this for auto boot service */
//LK_INIT_HOOK(sample_service_entry, sample_service_entry, SAMPLE_INIT_LEVEL)

void sample_test_entry(int argc, const cmd_args *argv)
{
    sample_service_init();
    printf("sample service started\n");
}

#if defined(WITH_LIB_CONSOLE)
STATIC_COMMAND_START STATIC_COMMAND("sspd", "run a sample service",
                                    (console_cmd)&sample_test_entry)
STATIC_COMMAND_END(samplesrv);
#endif

