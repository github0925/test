/*
 * sdrv_types.h
 *
 * Copyright (c) 2020 SemiDrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */

#ifndef __SOCKET_H_
#define __SOCKET_H_
#include "debug.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include "socket.h"
#include "board_diag.h"
#include "func_eth.h"

#define server_ip           "192.168.10.22"
#define config_server_ip    "ifconfig eth0 192.168.10.22"
#define config_client_ip    "ifconfig eth0 192.168.10.11"
#define eth_up              "ifconfig eth0 up"
#define eth_down            "ifconfig eth0 down"
#define ip_id               "ifconfig"

#define ETH_PORT_ENABLE    1
#define ETH_PORT_DISENABLE 0

#define SOCKET_CHANNEL_CREATE_NR 2

#define SOCKET_TCP_CREATE

typedef struct {
    int connfd;
    struct sockaddr_in seraddr;
} socket_client_info_t;

typedef struct {
#define PORT_SWITCH_CMD    0x01
#define SOCKET_VALID_DATA  0x02
    uint8_t cmd;
    uint8_t active_id;
    uint8_t active_port;
    uint8_t valid_data;
} socket_message_t;


extern socket_client_info_t socket_client_info;

void create_socket_ops(void);
void pthread_eth_tcp_server_create(        void *ip);
void pthread_eth_udp_server_create(        void *ip);
void pthread_eth_tcp_client_create(    void *ip);
void pthread_eth_udp_client_create(    void *ip);
int eth_socket_tcp_client_send_package(test_exec_t *exec,
                                       switch_info_t switch_info);
int eth_socket_udp_client_send_package(test_exec_t *exec,
                                       switch_info_t switch_info);
void eth_socket_change_port_active(uint8_t switch_id, uint8_t switch_port);
#endif

