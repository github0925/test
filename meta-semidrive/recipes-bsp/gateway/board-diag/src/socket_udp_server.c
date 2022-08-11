/*
 * socket_udp_server.c
 *
 * Copyright (c) 2020 SemiDrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include "socket.h"

static void socket_server_timeout_fn(int sig)
{
    eth_socket_change_port_active(MV5072_DEV_ID, SOCKET_CHANNEL_CREATE_NR);
}

static void socket_server_timeout_init(void)
{
    signal(SIGALRM, socket_server_timeout_fn);
}

static uint8_t eth_valid_data_ops(uint8_t data)
{
    return data + 1;
}

static void *eth_socket_server_start(void *arg)
{
    int ret = -1;
    int sockfd = -1;
    struct sockaddr_in seraddr, cliaddr;
    socklen_t addrlen = sizeof(struct sockaddr);
    uint8_t id_t = 0;
    uint8_t pt_t = 0;

    /*open socket interface*/
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    if (sockfd < 0) {
        ERROR("socket fail\n");
        goto out;
    }

    /*bind*/
    memset(&seraddr, 0, sizeof(seraddr));
    seraddr.sin_family = AF_INET;
    seraddr.sin_port = htons(8888);
    seraddr.sin_addr.s_addr = inet_addr((char *)arg);
    printf("socket create pass\n");
    ret = bind(sockfd, (struct sockaddr *)&seraddr, sizeof(seraddr));

    if (ret < 0) {
        ERROR("bind fail\n");
        goto out;
    }

    socket_server_timeout_init();

    printf("connect successfully client IP : %s, Port : %d!\n",
           inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port));

    while (1) {
        socket_message_t socket_message = {0};
        /*accept package*/

        ret = recvfrom(sockfd, &socket_message, sizeof(socket_message), 0,
                       (struct sockaddr *)&cliaddr, &addrlen);

        if (ret < 0) {
            ERROR("recvfrom fail\n");
            goto fail;
        }

        uint8_t cmd   = socket_message.cmd;

        if (cmd == PORT_SWITCH_CMD) {

            uint8_t id    = socket_message.active_id;
            uint8_t port  = socket_message.active_port;

            /*always set mv5072_port1 as the cmd channel befor send valid data + 1*/
            eth_socket_change_port_active(MV5072_DEV_ID, SOCKET_CHANNEL_CREATE_NR);

            ret = sendto(sockfd, &socket_message, sizeof(socket_message), 0,
                         (struct sockaddr *)&cliaddr, sizeof(cliaddr));

            if (ret < 0) {
                ERROR("sendto fail\n");
                goto fail;
            }

            id_t = id;
            pt_t = port;
            eth_socket_change_port_active(id_t, pt_t);
            alarm(1);
        }
        else if (cmd == SOCKET_VALID_DATA) {
            uint8_t *data = &socket_message.valid_data;

            alarm(0);

            eth_socket_change_port_active(id_t, pt_t);

            *data = eth_valid_data_ops(*data);

            ret = sendto(sockfd, &socket_message, sizeof(socket_message), 0,
                         (struct sockaddr *)&cliaddr, sizeof(cliaddr));

            if (ret < 0) {
                ERROR("send timeout\n");
                goto fail;
            }

            eth_socket_change_port_active(MV5072_DEV_ID, SOCKET_CHANNEL_CREATE_NR);
        }
        else {
            goto fail;
        }

        continue;

fail:
        eth_socket_change_port_active(MV5072_DEV_ID, SOCKET_CHANNEL_CREATE_NR);
    }

out:

    if (sockfd >= 0)
        close(sockfd);

    return NULL;
}

void pthread_eth_udp_server_create(        void *ip)
{
    pthread_t server_thread_id;
    pthread_create(&server_thread_id, NULL, eth_socket_server_start, (char *)ip);
}
