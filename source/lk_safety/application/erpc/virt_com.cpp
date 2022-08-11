/*
 * virt_com.cpp
 *
 * Copyright (c) 2019 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: eRPC service.
 *
 * Revision History:
 * -----------------
 */
#include <kernel/semaphore.h>
#include <kernel/thread.h>
#include <string.h>

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

#include "mbox_hal.h"
#include "virt_com.h"

/* Local ERPC servers */
#if VLIN_SERVER_SUPPORT
#include "lin_drv_server.h"
#endif
#if VCAN_SERVER_SUPPORT
#undef ERPC_TYPE_DEFINITIONS
#include "vcan_server.h"
#else
#include "vcan_cb_server.h"
#endif
#if SDPE_CTRL_SERVER_SUPPORT
#include "sdpe_ctrl_server.h"
#else
#include "sdpe_cb_server.h"
#endif

using namespace erpc;

static bool vircom_inited = false;
static semaphore_t vircom_server_rx_sem;

static erpc_transport_t server_erpc_transport;
static erpc_transport_t client_erpc_transport;

static erpc_mbf_t server_erpc_mbf;
static erpc_mbf_t client_erpc_mbf;

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
#if VLIN_SERVER_SUPPORT || VCAN_SERVER_SUPPORT || SDPE_CTRL_SERVER_SUPPORT
    client_rpmsg_transport->init(VIRCOM_CBK_CLIENT_ADDR,
                                 VIRCOM_CBK_SERVER_ADDR,
                                 VIRCOM_CBK_SHM_BASE,
                                 VIRCOM_CBK_SHM_SIZE,
                                 IPCC_RRPOC_SAF);
#else
    client_rpmsg_transport->init(VIRCOM_CLIENT_ADDR,
                                 VIRCOM_SERVER_ADDR,
                                 VIRCOM_SHM_BASE,
                                 VIRCOM_SHM_SIZE,
                                 IPCC_RRPOC_MP);
#endif
    client_erpc_transport = reinterpret_cast<erpc_transport_t>
                            (client_rpmsg_transport.get());
    dprintf(VIRCOM_DEBUG, "%s: client erpc transport inited!\n", __func__);

    client_rpmsg_mbf.construct(reinterpret_cast<RPMsgBaseTransport *>
                               (client_erpc_transport)->get_rpmsg_lite_instance());
    client_erpc_mbf = reinterpret_cast<erpc_mbf_t>(client_rpmsg_mbf.get());
    dprintf(VIRCOM_DEBUG, "%s: client erpc mbf inited!\n", __func__);

    client_codec_factory.construct();
    simple_client.construct();
    Transport *castedTransport = reinterpret_cast<Transport *>
                                 (client_erpc_transport);
    client_crc16.construct();
    castedTransport->setCrc16(client_crc16.get());
    simple_client->setTransport(castedTransport);
    simple_client->setCodecFactory(client_codec_factory);
    simple_client->setMessageBufferFactory(
        reinterpret_cast<MessageBufferFactory *>(client_erpc_mbf));
    g_client = simple_client;
    dprintf(VIRCOM_DEBUG, "%s: client erpc inited!\n", __func__);
}

static int vircom_server_daemon(void *arg)
{
    while (1) {
        sem_wait(&vircom_server_rx_sem);
        dprintf(INFO, "%s\n", __func__);
        g_server->poll();
    }

    return 0;
}

static void vircom_server_rx_cb(void)
{
    dprintf(INFO, "%s\n", __func__);
    sem_post(&vircom_server_rx_sem, false);
}

static void vircom_server_init(void)
{
    sem_init(&vircom_server_rx_sem, 0);

    server_rpmsg_transport.construct();
#if VLIN_SERVER_SUPPORT || VCAN_SERVER_SUPPORT || SDPE_CTRL_SERVER_SUPPORT
    server_rpmsg_transport->init(VIRCOM_SERVER_ADDR,
                                 VIRCOM_CLIENT_ADDR,
                                 VIRCOM_SHM_BASE,
                                 IPCC_RRPOC_SAF,
                                 NULL,
                                 NULL,
                                 vircom_server_rx_cb);
#else
    server_rpmsg_transport->init(VIRCOM_CBK_SERVER_ADDR,
                                 VIRCOM_CBK_CLIENT_ADDR,
                                 VIRCOM_CBK_SHM_BASE,
                                 IPCC_RRPOC_MP,
                                 NULL,
                                 NULL,
                                 vircom_server_rx_cb);
#endif
    server_erpc_transport = reinterpret_cast<erpc_transport_t>
                            (server_rpmsg_transport.get());
    dprintf(VIRCOM_DEBUG, "%s: server erpc transport inited!\n", __func__);

    server_rpmsg_mbf.construct(reinterpret_cast<RPMsgBaseTransport *>
                               (server_erpc_transport)->get_rpmsg_lite_instance());
    server_erpc_mbf = reinterpret_cast<erpc_mbf_t>(server_rpmsg_mbf.get());
    dprintf(VIRCOM_DEBUG, "%s: server erpc mbf inited!\n", __func__);

    server_codec_factory.construct();
    simple_server.construct();
    Transport *castedTransport = reinterpret_cast<Transport *>
                                 (server_erpc_transport);
    server_crc16.construct();
    castedTransport->setCrc16(server_crc16.get());
    simple_server->setTransport(castedTransport);
    simple_server->setCodecFactory(server_codec_factory);
    simple_server->setMessageBufferFactory(
        reinterpret_cast<MessageBufferFactory *>(server_erpc_mbf));
    g_server = simple_server;
    dprintf(VIRCOM_DEBUG, "%s: server erpc inited!\n", __func__);

    /* Create function services. */
#if VLIN_SERVER_SUPPORT
    g_server->addService(static_cast<erpc::Service *>
                         (create_lin_drv_service()));
#endif
#if VCAN_SERVER_SUPPORT
    g_server->addService(static_cast<erpc::Service *>
                         (create_vcan_service()));
#else
    g_server->addService(static_cast<erpc::Service *>
                         (create_vcan_cb_service()));
#endif
#if SDPE_CTRL_SERVER_SUPPORT
    g_server->addService(static_cast<erpc::Service *>
                         (create_sdpe_ctrl_service()));
#else
    g_server->addService(static_cast<erpc::Service *>
                         (create_sdpe_cb_service()));
#endif

    dprintf(VIRCOM_DEBUG, "%s: erpc added service to server!\n", __func__);

    thread_t *t = thread_create( "vircom-server",
                                 vircom_server_daemon,
                                 NULL,
                                 HIGH_PRIORITY - 1,
                                 DEFAULT_STACK_SIZE );
    thread_resume(t);
}

bool vircom_init(void)
{
    if (vircom_inited == true) {
        return true;
    }

    vircom_server_init();
    vircom_client_init();

    vircom_inited = true;

    dprintf(VIRCOM_DEBUG, "%s: vircom init success\n", __func__);

    return true;
}
