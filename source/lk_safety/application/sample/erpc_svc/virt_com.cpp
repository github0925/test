#include <kernel/event.h>
#include <kernel/thread.h>
#include <kernel/spinlock.h>
#include <string.h>

#include "virt_com.h"
#include "virCom.h"
#include "virComCbk_server.h"
#include "virComCbk.h"
#include "erpc_manually_constructed.h"
#include "erpc_rpmsg_lite_transport.h"
#include "erpc_message_buffer.h"
#include "erpc_simple_server.h"
#include "erpc_basic_codec.h"
#include "erpc_setup_mbf_rpmsg.h"
#include "erpc_client_manager.h"
#include "erpc_transport_setup.h"
#include "erpc_mbf_setup.h"
#include "rpmsg_lite.h"
#include "rpmsg_queue.h"
#include "rpmsg_ns.h"
#include "dcf.h"

using namespace erpc;

static bool vircom_inited = false;
event_t vircom_server_rx_event;
spin_lock_t vircom_spin_lock = SPIN_LOCK_INITIAL_VALUE;

erpc_transport_t server_erpc_transport;
erpc_transport_t client_erpc_transport;

erpc_mbf_t server_erpc_mbf;
erpc_mbf_t client_erpc_mbf;

static ManuallyConstructed<RPMsgTransport> server_rpmsg_transport;
static ManuallyConstructed<RPMsgTransport> client_rpmsg_transport;

static ManuallyConstructed<RPMsgMessageBufferFactory> server_rpmsg_mbf;
static ManuallyConstructed<RPMsgMessageBufferFactory> client_rpmsg_mbf;

static ManuallyConstructed<SimpleServer> simple_server;
static ManuallyConstructed<BasicCodecFactory> server_codec_factory;
static ManuallyConstructed<Crc16> server_crc16;

static ManuallyConstructed<ClientManager> simple_client;
static ManuallyConstructed<BasicCodecFactory> client_codec_factory;
static ManuallyConstructed<Crc16> client_crc16;

extern SimpleServer *g_server;
extern ClientManager *g_client;

static void vircom_client_init(void)
{
    client_rpmsg_transport.construct();
    client_rpmsg_transport->init(VIRCOM_CLIENT_ADDR,
                                 VIRCOM_SERVER_ADDR,
                                 VIRCOM_SHM_BASE,
                                 VIRCOM_SHM_SIZE,
                                 IPCC_RRPOC_MP);
    client_erpc_transport = reinterpret_cast<erpc_transport_t>(client_rpmsg_transport.get());
    dprintf(VIRCOM_DEBUG, "%s: client erpc transport inited!\n", __func__);

    client_rpmsg_mbf.construct(reinterpret_cast<RPMsgBaseTransport *>(client_erpc_transport)->get_rpmsg_lite_instance());
    client_erpc_mbf = reinterpret_cast<erpc_mbf_t>(client_rpmsg_mbf.get());
    dprintf(VIRCOM_DEBUG, "%s: client erpc mbf inited!\n", __func__);

    client_codec_factory.construct();
    simple_client.construct();
    Transport *castedTransport = reinterpret_cast<Transport *>(client_erpc_transport);
    client_crc16.construct();
    castedTransport->setCrc16(client_crc16.get());
    simple_client->setTransport(castedTransport);
    simple_client->setCodecFactory(client_codec_factory);
    simple_client->setMessageBufferFactory(reinterpret_cast<MessageBufferFactory *>(client_erpc_mbf));
    g_client = simple_client;
    dprintf(VIRCOM_DEBUG, "%s: client erpc inited!\n", __func__);
}

static int vircom_server_daemon(void *arg)
{
    while (1)
    {
        event_wait(&vircom_server_rx_event);
        dprintf(VIRCOM_DEBUG, "%s\n", __func__);
        g_server->poll();
    }

    return 0;
}

static void vircom_server_rx_cb(void)
{
    dprintf(VIRCOM_DEBUG, "%s\n", __func__);
    event_signal(&vircom_server_rx_event, false);
}

static void vircom_server_init(void)
{
    event_init(&vircom_server_rx_event, false, EVENT_FLAG_AUTOUNSIGNAL);

    server_rpmsg_transport.construct();
    server_rpmsg_transport->init(VIRCOM_CBK_SERVER_ADDR,
                                 VIRCOM_CBK_CLIENT_ADDR,
                                 VIRCOM_CBK_SHM_BASE,
                                 IPCC_RRPOC_MP,
                                 NULL,
                                 (char *)"vircom_ept",
                                 vircom_server_rx_cb);
    server_erpc_transport = reinterpret_cast<erpc_transport_t>(server_rpmsg_transport.get());
    dprintf(VIRCOM_DEBUG, "%s: server erpc transport inited!\n", __func__);

    server_rpmsg_mbf.construct(reinterpret_cast<RPMsgBaseTransport *>(server_erpc_transport)->get_rpmsg_lite_instance());
    server_erpc_mbf = reinterpret_cast<erpc_mbf_t>(server_rpmsg_mbf.get());
    dprintf(VIRCOM_DEBUG, "%s: server erpc mbf inited!\n", __func__);

    server_codec_factory.construct();
    simple_server.construct();
    Transport *castedTransport = reinterpret_cast<Transport *>(server_erpc_transport);
    server_crc16.construct();
    castedTransport->setCrc16(server_crc16.get());
    simple_server->setTransport(castedTransport);
    simple_server->setCodecFactory(server_codec_factory);
    simple_server->setMessageBufferFactory(reinterpret_cast<MessageBufferFactory *>(server_erpc_mbf));
    g_server = simple_server;
    dprintf(VIRCOM_DEBUG, "%s: server erpc inited!\n", __func__);

    g_server->addService(static_cast<erpc::Service *>(create_virComCbk_service()));
    dprintf(VIRCOM_DEBUG, "%s: erpc added service to server!\n", __func__);

    thread_t *t = thread_create( "vircom-server",
                                 vircom_server_daemon,
                                 NULL,
                                 DEFAULT_PRIORITY,
                                 DEFAULT_STACK_SIZE );
    thread_resume(t);
}

bool vircom_init(void)
{
    spin_lock_saved_state_t status;

    if(vircom_inited == true) {
        return true;
    }

    spin_lock_irqsave(&vircom_spin_lock, status);

    vircom_client_init();
    vircom_server_init();
    vircom_inited = true;
    spin_unlock_irqrestore(&vircom_spin_lock, status);

    dprintf(VIRCOM_DEBUG, "%s: vircom init success\n", __func__);

    return true;
}