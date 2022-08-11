#include "rpmsg_port.h"
#include "can_proxy.h"

#define RPMSG_PORT_VERSION  (0x1001)

#define CP_BAUDRATE_250K_500K   (1)
#define CP_BAUDRATE_500K_1M     (2)
#define CP_BAUDRATE_500K_2M     (3)
#define CP_BAUDRATE_1M_2M       (4)
#define CP_BAUDRATE_1M_5M       (5)

typedef struct cp_if_rpmsg_handle_t
{
    struct rpmsg_channel* chan;
    const char* name;
    uint16_t srv_id;
    uint16_t ept_id;
    int rproc;
}cp_if_rpmsg_handle_t;

enum sts_op_type {
    CP_OP_GET_VERSION = 1,
    CP_OP_GET_CONFIG,
    CP_OP_START,
    CP_OP_STOP,
};

/* Do not exceed 32 bytes so far */
struct canctl_cmd {
    u16 op;
    u16 tag;
    union {
        struct {
            u16 what;
        } s;
    } msg;
};

/* Do not exceed 16 bytes so far */
struct canctl_result {
    u16 op;
    u16 tag;
    union {
        /** used for get_version */
        struct {
            u16 version;
            u16 id;
            u16 vendor;
        } v;
        /** used for get_config */
        struct {
            u8 baudrate;
            u8 canfd_support;
            u8 instances;
            u8 padding;
        } cfg;
        u8 data[12];
    } msg;
};

extern const rpmsg_if_map_t rpmsg_if_id_tlb[RIFN];


static rpmsg_if_id_t rpmsg_chan2if_id(struct rpmsg_channel* chan)
{
    for(uint32_t i=0;i<sizeof(rpmsg_if_id_tlb)/sizeof(rpmsg_if_id_tlb[0]);i++)
    {
        if(rpmsg_if_id_tlb[i].port.ept_id == chan->addr &&
         rpmsg_if_id_tlb[i].port.rproc == chan->rproc)
        {
           return rpmsg_if_id_tlb[i].if_id;
        }
    }

    return RIFN;
}

static rpmsg_if_id_t rpmsg_port2if_id(int rproc, int endpoint)
{
    for(uint32_t i=0;i<sizeof(rpmsg_if_id_tlb)/sizeof(rpmsg_if_id_tlb[0]);i++)
    {
        if(rpmsg_if_id_tlb[i].port.ept_id == endpoint &&
         rpmsg_if_id_tlb[i].port.rproc == rproc)
        {
           return rpmsg_if_id_tlb[i].if_id;
        }
    }

    return RIFN;
}

static void rpmsg_common_cb(struct rpmsg_channel *chan, struct dcf_message *msg, int len)
{

    cp_if_frame_t* frame = malloc(sizeof(cp_if_frame_t)+len);
    ASSERT(frame);

    frame->bus = cpifBusRPMSG;
    frame->if_id = rpmsg_chan2if_id(chan);
    frame->len = len;


    memcpy(frame->data,msg,len);

    CPDBG("==== RPMSG recv data %d ====\n",cp_if_get_frame_size(frame));

    for(uint32_t i=0;i<cp_if_get_frame_size(frame);i++)
    {
        CPDBG("0x%x ",((uint8_t*)frame)[i]);
    }
    CPDBG("\n");

    cp_if_frame_post(frame);
    free(frame);
}

static rpc_call_result_t rpmsg_canctl_cb(rpc_server_handle_t hserver,
        rpc_call_request_t *request)
{
    rpc_call_result_t result = {0,};
    struct canctl_cmd *ctl = (struct canctl_cmd *) &request->param[0];
    struct canctl_result *r = (struct canctl_result *) &result.result[0];
    struct cp_if_t* cpif = NULL;
    result.ack = request->cmd;
    rpmsg_if_id_t if_id;

    if (!request) {
        result.retcode = ERR_INVALID_ARGS;
        dprintf(ALWAYS, "%s:instance=%x\n", __func__, ctl->tag);
        return result;
    }

    if_id = rpmsg_port2if_id(hserver->rpchan->rproc, ctl->tag);
    if (if_id == RIFN) {
        result.retcode = ERR_NOT_VALID;
        dprintf(ALWAYS, "%s:Bad instance=%d\n", __func__, ctl->tag);
        return result;
    }

    cpif = cp_get_interface(cpifBusRPMSG, if_id);
    if (cpif->if_state == cpifStateUnkown) {
        result.retcode = ERR_NOT_READY;
        dprintf(ALWAYS, "%s:instance=%d not ready\n", __func__, ctl->tag);
        return result;
    }

    switch (ctl->op) {
        case CP_OP_GET_VERSION:
            // TODO: read from can driver
            r->msg.v.version = RPMSG_PORT_VERSION;
            r->msg.v.id = 0;
            r->msg.v.vendor = 0x0;
            result.retcode = 0;
            break;

        case CP_OP_GET_CONFIG:
            // TODO: read from can driver
            r->msg.cfg.baudrate = CP_BAUDRATE_1M_5M;
            r->msg.cfg.canfd_support = 1;
            r->msg.cfg.instances = MAX_INTERFACE_PER_BUS;
            result.retcode = 0;
            break;

        case CP_OP_START:
        case CP_OP_STOP:
            if (cpif->if_state == cpifStateStopped || (cpif->if_state == cpifStateInited)) {
                cpif->if_state = cpifStateRunning;
                dprintf(ALWAYS, "%s:cpif=%d, started\n", __func__, if_id);
            } else {
                cpif->if_state = cpifStateStopped;
                dprintf(ALWAYS, "%s:cpif=%d, stopped\n", __func__, if_id);
            }
            result.retcode = 0;
            break;

        default:
            result.retcode = ERR_INVALID_ARGS;
            dprintf(ALWAYS, "%s:cpif=%d, op err\n", __func__, ctl->tag);
            break;
    }

    dprintf(1, "%s:cpif=%d, cmd=%d done\n", __func__, if_id, ctl->op);
    return result;
}

int rpmsg_if_init(struct cp_if_t* cpif)
{
    rpc_server_impl_t devfuncs[] = {
        {MOD_RPC_REQ_CP_IOCTL, rpmsg_canctl_cb, IPCC_RPC_NO_FLAGS},
    };

    // some ugly workaround.
    static int inited = 0;
    if(!inited)
    {
        start_ipcc_rpc_service(devfuncs, ARRAY_SIZE(devfuncs));
        inited = 1;
    }

    cp_if_rpmsg_handle_t* handle = malloc(sizeof(cp_if_rpmsg_handle_t));
    if(!handle)
    {
        CPDBG("rpmsg interface malloc fail\n");
        free(handle);
        return -1;
    }

    rpmsg_cfg_container_t* cc = cpif->container;

    handle->ept_id = cc->ept_id;
    handle->name = cc->name;
    handle->rproc = cc->rproc;

    handle->chan = rpmsg_channel_create(handle->rproc, handle->ept_id, handle->name);
    CPDBG("Create rpmsg chan %s %d-%d\n",handle->name,handle->rproc,handle->ept_id);

    if(!handle->chan)
    {
        CPDBG("Create rpmsg chan %s %d-%d fail\n",handle->name,handle->rproc,handle->ept_id);
        free(handle);
        return -1;
    }

    cpif->container = handle;
    cpif->ctn_size = sizeof(cp_if_rpmsg_handle_t);

    rpmsg_channel_start(handle->chan, rpmsg_common_cb);

    cpif->if_state = cpifStateInited;

    return 0;

}

int rpmsg_if_rxcb(struct cp_if_t* cpif, void* data, int len)
{
    // rpmsg_if_frame_t* frame = data;

    if (cpif->if_state == cpifStateRunning)
        cp_bundle_attach(TARGET_CAN_BUNDLE,data,len);

    return len;
}

int rpmsg_if_tx(struct cp_if_t* cpif,void* from, int len)
{
    cp_if_rpmsg_handle_t* handle = cpif->container;
    struct cp_can_frame *cpframe = (struct cp_can_frame *) from;

    if (cpif->if_state != cpifStateRunning)
        return ERR_BAD_STATE;

    dprintf(1, "CP:Send CAN %d %d to RPMSG\n",cpframe->can_id, cpframe->len);

    for(int i=0;i<len;i++)
    {
        dprintf(1, "0x%x ",((uint8_t*)from)[i]);
    }
    dprintf(1, "\n");

    /* Send the entire cp can frame */
    return rpmsg_channel_sendmsg(handle->chan,from,CP_CAN_MTU,100);
}
