/*
 * socket_server.c
 *
 * Copyright (c) 2020 SemiDrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include "socket.h"

#define LISTENQ         100

static uint8_t eth_valid_data_ops(uint8_t data)
{
    return data + 1;
}

static int set_no_recv_block(int fd)
{
    int ret = -1;
    struct timeval timeout  = {2, 0};
    struct linger so_linger = {1, 2};
    /*set recv/send timeout*/
    ret = setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout,
                     sizeof(struct timeval));

    if (ret < 0) {
        ERROR("SO_SNDTIMEO fail\n");
        goto out;
    }

    ret = setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout,
                     sizeof(struct timeval));

    if (ret < 0) {
        ERROR("SO_RCVTIMEO fail\n");
        goto out;
    }

    ret = setsockopt(fd, SOL_SOCKET, SO_LINGER,
                     (char *)&so_linger, sizeof(struct linger));

    if (ret < 0) {
        ERROR("SO_LINGER fail\n");
        goto out;
    }

out:

    return ret;
}

static void eth_socket_server_ops(int fd)
{
    int ret = -1;
    uint8_t id_t = 0;
    uint8_t pt_t = 0;
    socket_message_t socket_message = {0};

    while (1) {
        /*receive package*/
        ret = recv(fd, &socket_message, sizeof(socket_message),
                   MSG_WAITALL);

        if (ret < 0) {
            ERROR("recv fail\n");
            goto out;
        }

        uint8_t cmd = socket_message.cmd;

        if (cmd == PORT_SWITCH_CMD) {
            uint8_t id    = socket_message.active_id;
            uint8_t port  = socket_message.active_port;

            /*always set mv5072_port1 as the cmd channel befor send valid data + 1*/
            eth_socket_change_port_active(MV5072_DEV_ID, SOCKET_CHANNEL_CREATE_NR);

            /*send package*/
            ret = send(fd, &socket_message, sizeof(socket_message), 0);

            if (ret < 0) {
                ERROR("send fail\n");
                goto out;
            }

            id_t = id;
            pt_t = port;
            eth_socket_change_port_active(id_t, pt_t);
        }
        else if (cmd == SOCKET_VALID_DATA) {
            uint8_t *data = &socket_message.valid_data;

            eth_socket_change_port_active(id_t, pt_t);

            *data = eth_valid_data_ops(*data);

            ret = send(fd, &socket_message, sizeof(socket_message), 0);

            if (ret < 0) {
                ERROR("send timeout\n");
            }

            goto out;
        }
        else {
            printf("NO CMD \n");
            goto out;
        }

        memset(&socket_message, 0, sizeof(socket_message));
    }

out:

    if (fd >= 0) {
        close(fd);
        fd = -1;
    }

    eth_socket_change_port_active(MV5072_DEV_ID, SOCKET_CHANNEL_CREATE_NR);
}

static void *eth_socket_server_start(void *arg)
{
    int ret      = -1;
    int sockfd   = -1;
    int connfd   = -1;

    socklen_t clilen = sizeof(struct sockaddr);

    struct sockaddr_in seraddr, cliaddr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0) {
        ERROR("socket create fail\n");
        goto fail;
    }

    printf("socket create pass\n");

    /*bind*/
    bzero(&seraddr, sizeof(seraddr));
    seraddr.sin_family = AF_INET;
    seraddr.sin_port = htons(2000);
    seraddr.sin_addr.s_addr = inet_addr((char *)arg);
    ret = bind(sockfd, (struct sockaddr *)&seraddr, sizeof(seraddr));

    if (ret < 0) {
        ERROR("bind fail\n");
        goto fail;
    }

    printf("bind pass\n");

    /*listen*/
    ret = listen(sockfd, LISTENQ);

    if (ret < 0) {
        ERROR("listen fail\n");
        goto fail;
    }

    printf("listen pass\n");

    while (1) {
        /*new connect*/
        connfd = accept(sockfd, (struct sockaddr *)&cliaddr, &clilen);

        if (connfd < 0) {
            ERROR("accept fail\n");
            continue;
        }

        ret = set_no_recv_block(connfd);

        if (ret < 0) {
            if (connfd >= 0) {
                close(connfd);
                connfd = -1;
            }

            ERROR("set_no_recv_block fail\n");
            continue;
        }

        printf("connect successfully client IP : %s, Port : %d!\n",
               inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port));

        eth_socket_server_ops(connfd);
    }

fail:

    if (sockfd >= 0)
        close(sockfd);

    printf("server over with some error!!\n");
    return NULL;
}

void pthread_eth_tcp_server_create(        void *ip)
{
    pthread_t server_thread_id;
    pthread_create(&server_thread_id, NULL, eth_socket_server_start, (char *)ip);
}


