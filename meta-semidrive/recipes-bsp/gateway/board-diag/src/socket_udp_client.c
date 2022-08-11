/*
 * socket_udp_client.c
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
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include "socket.h"

socket_client_info_t socket_client_info;

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

    ret = sendto(socket_client_info.connfd, socket_message, sizeof(*socket_message),
                 0, (struct sockaddr *)&socket_client_info.seraddr,
                 sizeof(socket_client_info.seraddr));

    if (ret < 0) {
        ERROR("sendto fail\n");
        return ret;
    }

    bzero(socket_message, sizeof(*socket_message));

    ret = recvfrom(socket_client_info.connfd, socket_message,
                   sizeof(*socket_message), 0, NULL, NULL);

    if (ret < 0) {
        ERROR("recvfrom fail\n");
        return ret;
    }

    return ret;
}

int eth_socket_udp_client_send_package(test_exec_t *exec,
                                       switch_info_t switch_info)
{
    int ret = -1;
    uint8_t id = switch_info.id;
    uint8_t port = switch_info.port;
    socket_message_t socket_message = {0};
    eth_cmd_t *eth_cmd = (eth_cmd_t *)&exec->cmd;
    eth_resp_t *resp = (eth_resp_t *)&exec->resp;
    /*always set mv5072_port2 as the cmd channel befor send valid data + 1*/
    eth_socket_change_port_active(MV5072_DEV_ID, SOCKET_CHANNEL_CREATE_NR);
    /*socket client send cmd data*/
    ret = eth_socket_clinet_send_data(eth_cmd, id, port, PORT_SWITCH_CMD,
                                      &socket_message);

    if (ret < 0) {
        ERROR("PORT_SWITCH_CMD send_data fail \n");
        goto out;
    }

    /*close mv5072_port2, active valid data channel*/
    eth_socket_change_port_active(id, port);
    /*socket client send valid data*/
    ret = eth_socket_clinet_send_data(eth_cmd, MV5072_DEV_ID,
                                      SOCKET_CHANNEL_CREATE_NR, SOCKET_VALID_DATA,
                                      &socket_message);

    if (ret < 0) {
        ERROR("SOCKET_VALID_DATA send_data fail \n");
        goto out;
    }

    set_para_value(resp->data1, socket_message.valid_data);

    return ret;
out:

    if (socket_client_info.connfd > 0) {
        close(socket_client_info.connfd);
        pthread_eth_udp_client_create("192.168.10.22");
        usleep(100);
    }

    return ret;
}

static void eth_socket_client_start(void *arg)
{
    int ret = -1;
    struct timeval timeout = {1, 0};
    /*open socket interface*/
    socket_client_info.connfd = socket(AF_INET, SOCK_DGRAM, 0);

    if (socket_client_info.connfd < 0) {
        ERROR("socket fail\n");
        return;
    }

    /*link and connect request*/
    memset(&socket_client_info.seraddr, 0, sizeof(socket_client_info.seraddr));
    socket_client_info.seraddr.sin_family = AF_INET;
    socket_client_info.seraddr.sin_port = htons(8888);
    socket_client_info.seraddr.sin_addr.s_addr = inet_addr((char *)arg);

    /*wating for server create*/
    sleep(2);

    ret = setsockopt(socket_client_info.connfd, SOL_SOCKET, SO_SNDTIMEO,
                     (char *)&timeout, sizeof(struct timeval));

    if (ret < 0) {
        ERROR("setsockopt SO_SNDTIMEO fail\n");
        return;
    }

    ret = setsockopt(socket_client_info.connfd, SOL_SOCKET, SO_RCVTIMEO,
                     (char *)&timeout, sizeof(struct timeval));

    if (ret < 0) {
        ERROR("setsockopt SO_RCVTIMEOs fail\n");
        return;
    }

    printf("link and connect success\n");
}

void pthread_eth_udp_client_create(    void *ip)
{
    eth_socket_client_start(ip);
}

