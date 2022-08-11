/*
 * sw_timer.c
 *
 * Copyright (c) 2020 SemiDrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */
#include <pthread.h>
#include <semaphore.h>

#include "sw_timer.h"
#include "gpio-ops.h"
#include "func_eth.h"
#include "socket.h"

static dev_master_slave_t dev_master_slave = {
    .pre_state = 0,
    .cur_state = 1
};

static sem_t sem;

static void monitor_dev_master_slave_mode(void)
{
    GPIO_VALUE result;

    if (gpio_read(G9X_GPIO_RES1_AP, (uint8_t *)&result) != true) {
        ERROR("gpio read fail\n");
        return;
    }

    printf("gpio read result = %d\n", result);

    if (result == ON_LINE) {
        dev_master_slave.master_slave_state = XMASTER_MODE;
    }
    else {
        dev_master_slave.master_slave_state = XSLAVE_MODE;
    }
}

MASTER_SLAVER_MODE get_dev_master_slave_mode(void)
{
    return dev_master_slave.master_slave_state;
}

static void pthread_creat_eth_socket(MASTER_SLAVER_MODE mode)
{
    if (mode == XMASTER_MODE) {
#ifdef SOCKET_TCP_CREATE
        pthread_eth_tcp_client_create((char *)server_ip);
#else
        pthread_eth_udp_client_create((char *)server_ip);
#endif
    }
    else if (mode == XSLAVE_MODE) {
#ifdef SOCKET_TCP_CREATE
        pthread_eth_tcp_server_create((char *)server_ip);
#else
        pthread_eth_udp_server_create((char *)server_ip);
#endif
    }
}

static void eth_dev_static_ip_set(MASTER_SLAVER_MODE mode)
{
    if ((dev_master_slave.cur_state == dev_master_slave.pre_state)
            && (dev_master_slave.status != -1)) {
        ERROR("dev master_slave status fail!\n");
        return;
    }

    if (mode == XMASTER_MODE) {
        dev_master_slave.status = system(config_client_ip);
    }
    else {
        dev_master_slave.status = system(config_server_ip);
    }

    if (dev_master_slave.status == -1) {
        ERROR("dev master_slave status fail!\n");
        return;
    }

    dev_master_slave.pre_state = dev_master_slave.cur_state;
}

static void *eth_socket_create(void *arg)
{
    int ret = -1;
    ret = sem_init(&sem, 0, 0);

    if (ret < 0)
        ERROR("sem_init fail\n");

    while (1) {
        ret = sem_wait(&sem);

        if (ret < 0)
            ERROR("sem_wait fail\n");

        pthread_creat_eth_socket(get_dev_master_slave_mode());
    }

    return NULL;
}

static void signalrm_fn(int sig)
{
    if ((dev_master_slave.status = system(eth_down)) == -1) {
        ERROR("eth down fail!\n");
    }

    usleep(100);
    monitor_dev_master_slave_mode();
    eth_dev_static_ip_set(get_dev_master_slave_mode());
    eth_set_port_mode_early(get_dev_master_slave_mode());
    usleep(100);

    if ((dev_master_slave.status = system(ip_id)) == -1) {
        ERROR("eth ifconfig fail!\n");
    }

    usleep(100);

    if ((dev_master_slave.status = system(eth_up)) == -1) {
        ERROR("eth up fail!\n");
    }

    if (sem_post(&sem) < 0) {
        ERROR("sem_post fail\n");
    }
}

void  create_socket_ops(void    )
{
    pthread_t socket_thread_id;
    pthread_create(&socket_thread_id, NULL, eth_socket_create, NULL);
}

void timer_init(void)
{
    signal(SIGALRM, signalrm_fn);
    alarm(20);
}