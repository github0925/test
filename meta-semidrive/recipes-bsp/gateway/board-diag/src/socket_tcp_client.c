/*
 * socket_client.c
 *
 * Copyright (c) 2020 SemiDrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include "socket.h"

socket_client_info_t socket_client_info;
static int eth_socket_client_start(void *arg);
static int eth_socket_clinet_send_data(eth_cmd_t *eth_cmd, uint8_t id,
                                       uint8_t port, uint8_t cmd_type, socket_message_t *socket_message)
{
    int ret = -1;

    set_para_value(socket_message->cmd, cmd_type);
    set_para_value(socket_message->active_id, id);
    set_para_value(socket_message->active_port, port);
    set_para_value(socket_message->valid_data, eth_cmd->valid_data);

    if (socket_client_info.connfd < 0) {
        ERROR("socket create fail\n");
        return ret;
    }

    /*send package*/
    ret = send(socket_client_info.connfd, socket_message, sizeof(*socket_message),
               0);

    if (ret < 0) {
        ERROR("send fail\n");
        return ret;
    }

    /*receive package*/
    ret = recv(socket_client_info.connfd, socket_message, sizeof(*socket_message),
               0);

    if (ret < 0) {
        ERROR("recv fail\n");
        return ret;
    }

    return ret;
}

int eth_socket_tcp_client_send_package(test_exec_t *exec,
                                       switch_info_t switch_info)
{
    int ret = -1;
    uint8_t id = switch_info.id;
    uint8_t port = switch_info.port;
    socket_message_t socket_message = {0};
    eth_cmd_t *eth_cmd = (eth_cmd_t *)&exec->cmd;
    eth_resp_t *resp = (eth_resp_t *)&exec->resp;

    ret = eth_socket_client_start(server_ip);

    if (ret < 0) {
        ERROR("eth_socket_client_start fail \n");
        goto out;
    }

    /*socket client send cmd data*/
    ret = eth_socket_clinet_send_data(eth_cmd, id, port, PORT_SWITCH_CMD,
                                      &socket_message);

    if (ret < 0) {
        ERROR("PORT_SWITCH_CMD send_data fail \n");
        goto out;
    }

    /*close mv5072_port1, active valid data channel*/
    eth_socket_change_port_active(id, port);

    /*socket client send valid data*/
    ret = eth_socket_clinet_send_data(eth_cmd, id, port, SOCKET_VALID_DATA,
                                      &socket_message);

    if (ret < 0) {
        ERROR("SOCKET_VALID_DATA send_data fail \n");
        goto out;
    }

    set_para_value(resp->test_mode, socket_message.valid_data);

out:

    if (socket_client_info.connfd >= 0) {
        close(socket_client_info.connfd);
    }

    return ret;
}

static int eth_socket_client_start(void *arg)
{
    int ret = -1;

    /*open socket interface*/
    struct timeval timeout  = {2, 0};
    struct linger so_linger = {1, 2};
    socket_client_info.connfd = socket(AF_INET, SOCK_STREAM, 0);

    if (socket_client_info.connfd < 0) {
        ERROR("socket fail\n");
        goto out;
    }

    /*link and connect request*/
    memset(&socket_client_info.seraddr, 0, sizeof(socket_client_info.seraddr));
    socket_client_info.seraddr.sin_family = AF_INET;
    socket_client_info.seraddr.sin_port = htons(2000);
    socket_client_info.seraddr.sin_addr.s_addr = inet_addr((char *)arg);

    eth_socket_change_port_active(MV5072_DEV_ID, SOCKET_CHANNEL_CREATE_NR);
    /*wating for server create*/

    ret = setsockopt(socket_client_info.connfd, SOL_SOCKET, SO_SNDTIMEO,
                     (char *)&timeout, sizeof(struct timeval));

    if (ret < 0) {
        ERROR("SO_SNDTIMEO fail\n");
        goto out;
    }

    ret = setsockopt(socket_client_info.connfd, SOL_SOCKET, SO_RCVTIMEO,
                     (char *)&timeout, sizeof(struct timeval));

    if (ret < 0) {
        ERROR("SO_RCVTIMEOs fail\n");
        goto out;
    }

    ret = setsockopt(socket_client_info.connfd, SOL_SOCKET, SO_LINGER,
                     (char *)&so_linger, sizeof(struct linger));

    if (ret < 0) {
        ERROR("SO_LINGER fail\n");
        goto out;
    }

    ret = connect(socket_client_info.connfd,
                  (struct sockaddr *)&socket_client_info.seraddr,
                  sizeof(socket_client_info.seraddr));

    if (ret < 0) {
        ERROR("connect fail\n");
        goto out;
    }

out:

    return ret;
}

void pthread_eth_tcp_client_create(    void *ip)
{
    eth_socket_change_port_active(MV5072_DEV_ID, SOCKET_CHANNEL_CREATE_NR);
}



