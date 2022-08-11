/*
 * eth_port_socket.c
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
#include <unistd.h>

#include <sys/ioctl.h>

#include "board_diag.h"
#include "cfg.h"
#include "debug.h"
#include "gpio-ops.h"
#include "func_eth.h"
#include "socket.h"

#if 0
static int eth_dev_enable_one_port(uint8_t switch_id, uint8_t port,
                                   uint8_t flag)
{
    int ret = -1;
    struct mv_data mvbuf = {
        .portaddr = 0,
    };

    mvbuf.portaddr = port;
    mvbuf.regnum   = MV_5072_PORT_CONTROL_0_REG;
    mvbuf.is_write = 1;

    if (flag == ETH_PORT_DISENABLE)
        mvbuf.data[0] = MV_5072_PORT_DISABLE_VALUE;
    else if (flag == ETH_PORT_ENABLE)
        mvbuf.data[0]  = MV_5072_PORT_ENABLE_VALUE;
    else
        goto out;

    ret = ioctl(switch_id, MV_GET_CMD, mvbuf);

    if (ret < 0) {
        ERROR("ioctl fail\n");
        goto out;
    }

    return 0;
out:
    return -1;
}

int _5050_eth_dev_eabale_one_port(uint8_t port, bool flg)
{
    int ret = -1;
    int mv5050_fd = -1;

    if (port == MV_PORT_RESERVED)
        goto out;

    if (port > 10)
        goto out;

    mv5050_fd = open(MV5050_DEV_PATH, O_RDWR);

    if (mv5050_fd < 0) {
        ERROR("open mv5050 device fail fd %d\n", mv5050_fd);
        goto out;
    }

    ret = eth_dev_enable_one_port(mv5050_fd, port, flg);

out:

    if (mv5050_fd >= 0)
        close(mv5050_fd);

    return ret;
}

int _5072_eth_dev_eabale_one_port(uint8_t port, bool flg)
{
    int ret = -1;
    int mv5072_fd = -1;

    if (port >= MV_PORT_RESERVED)
        goto out;

    mv5072_fd = open(MV5072_DEV_PATH, O_RDWR);

    if (mv5072_fd < 0) {
        ERROR("open mv5072_fd device fail fd %d\n", mv5072_fd);
        goto out;
    }

    ret = eth_dev_enable_one_port(mv5072_fd, port, flg);

out:

    if (mv5072_fd >= 0)
        close(mv5072_fd);

    return ret;
}
#endif
#if 0
static void eth_set_port_no_route(void)
{
    int ret;
    int mv5072_fd = -1;
    int mv5050_fd = -1;
    struct mv_data mvbuf = {
        .portaddr = 0,
    };

    mv5072_fd = open(MV5072_DEV_PATH, O_RDWR);

    if (mv5072_fd < 0) {
        ERROR("open mv5072 device fail fd %d\n", mv5072_fd);
        goto out;
    }

    mv5050_fd = open(MV5050_DEV_PATH, O_RDWR);

    if (mv5050_fd < 0) {
        ERROR("open mv5050 device fail fd %d\n", mv5050_fd);
        goto out;
    }

    for (uint8_t i = MV5072_PORT_MIN; i <= MV5072_PORT_MAX; i++) {

        if (i == MV_PORT_RESERVED)
            continue;

        mvbuf.portaddr = i;
        mvbuf.regnum   = 0x33;
        mvbuf.is_write = 1;
        mvbuf.data[0] = 0x801f;
        ret = ioctl(mv5072_fd, MV_GET_CMD, &mvbuf);

        if (ret < 0) {
            ERROR("ioctl fail\n");
            goto out;
        }
    }

    //P_6 SGMI //P_7 RGMI  //P_8 5050<->5072
    for (uint8_t i = MV5050_PORT_MIN; i <= MV5050_PORT_MAX; i++) {

        if (i == MV_PORT_RESERVED)
            continue;

        mvbuf.portaddr = i;
        mvbuf.regnum   = 0x33;
        mvbuf.is_write = 1;
        mvbuf.data[0] = 0x801f;
        ret = ioctl(mv5050_fd, MV_GET_CMD, &mvbuf);

        if (ret < 0) {
            ERROR("ioctl fail\n");
            goto out;
        }
    }

    DBG("mvbuf.portaddr=%x,mvbuf.data[0]=%x\n", mvbuf.portaddr,
        mvbuf.data[0]);

out:

    if (mv5072_fd >= 0)
        close(mv5072_fd);

    if (mv5050_fd >= 0)
        close(mv5050_fd);
}
#endif

#if 0
static void eth_set_port_disable(void)
{
    int ret;
    int mv5072_fd = -1;
    int mv5050_fd = -1;
    struct mv_data mvbuf = {
        .portaddr = 0,
    };

    mv5072_fd = open(MV5072_DEV_PATH, O_RDWR);

    if (mv5072_fd < 0) {
        ERROR("open mv5072 device fail fd %d\n", mv5072_fd);
        goto out;
    }

    mv5050_fd = open(MV5050_DEV_PATH, O_RDWR);

    if (mv5050_fd < 0) {
        ERROR("open mv5050 device fail fd %d\n", mv5050_fd);
        goto out;
    }

    for (uint8_t i = MV5072_PORT_MIN; i <= MV5072_PORT_MAX; i++) {

        if (i == MV_PORT_RESERVED)
            continue;

        mvbuf.portaddr = i;
        mvbuf.regnum   = MV_5072_PORT_CONTROL_0_REG;
        mvbuf.is_write = 1;
        mvbuf.data[0] = MV_5072_PORT_DISABLE_VALUE;

        ret = ioctl(mv5072_fd, MV_GET_CMD, &mvbuf);

        if (ret < 0) {
            ERROR("ioctl fail\n");
            goto out;
        }

        mvbuf.portaddr = i;
        mvbuf.regnum   = MV_5072_PORT_CONTROL_0_REG;
        mvbuf.is_write = 0;

        DBG("mvbuf.portaddr=%x,mvbuf.data[0]=%x\n", mvbuf.portaddr,
            mvbuf.data[0]);
    }

    //P_6 SGMI //P_7 SRGMI  //P_8 5050<->5072
    for (uint8_t i = MV5050_PORT_MIN; i <= MV5050_PORT_MAX; i++) {

        if (i == MV_PORT_RESERVED)
            continue;

        mvbuf.portaddr = i;
        mvbuf.regnum   = MV_5072_PORT_CONTROL_0_REG;
        mvbuf.is_write = 1;
        mvbuf.data[0] = MV_5072_PORT_DISABLE_VALUE;

        ret = ioctl(mv5050_fd, MV_GET_CMD, &mvbuf);

        if (ret < 0) {
            ERROR("ioctl fail\n");
            goto out;
        }

        mvbuf.portaddr = i;
        mvbuf.regnum   = MV_5072_PORT_CONTROL_0_REG;
        mvbuf.is_write = 0;

        DBG("mvbuf.portaddr=%x,mvbuf.data[0]=%x\n", mvbuf.portaddr,
            mvbuf.data[0]);
    }

    DBG("dev set into negotiation mode ......\n");

out:

    if (mv5072_fd >= 0)
        close(mv5072_fd);

    if (mv5050_fd >= 0)
        close(mv5050_fd);
}
#endif

#if 0
static int eth_dev_atu_clear_entries(uint8_t switch_id, uint8_t switch_port,
                                     struct mv_data *mvbuf)
{
    int fd;
    int ret;

    fd = get_switch_fd(switch_id);

    mvbuf->portaddr = MV_5072_GLOBAL1_PORT;
    mvbuf->regnum   = MV_5072_ATU_DATA_REG;
    mvbuf->is_write = 0;

    ret = ioctl(fd, MV_GET_CMD, mvbuf);

    if (ret < 0) {
        ERROR("ioctl fail\n");
        goto out;
    }

    DBG("atu data=0x%x\n", mvbuf->data[0]);

    mvbuf->is_write = 1;

    if (switch_id == 1) {
        mvbuf->data[0] = mvbuf->data[0] & 0x18000;
        mvbuf->data[0] = mvbuf->data[0] | (switch_port << 4) | (0x1f << 9) | 0xf;
    }
    else {
        mvbuf->data[0] = mvbuf->data[0] & 0x8000;
        mvbuf->data[0] = mvbuf->data[0] | (switch_port << 4) | (0xf << 8) | 0xf;
    }

    DBG("atu data change=0x%x\n", mvbuf->data[0]);
    ret = ioctl(fd, MV_GET_CMD, mvbuf);

    if (ret < 0) {
        ERROR("ioctl fail\n");
        goto out;
    }

    mvbuf->portaddr = MV_5072_GLOBAL1_PORT;
    mvbuf->regnum   = MV_5072_ATU_OP_REG;
    mvbuf->is_write = 0;

    ret = ioctl(fd, MV_GET_CMD, mvbuf);

    if (ret < 0) {
        ERROR("ioctl fail\n");
        goto out;
    }

    DBG("atu op=0x%x\n", mvbuf->data[0]);

    mvbuf->is_write = 1;
    mvbuf->data[0] = mvbuf->data[0] & ~0x7000;
    mvbuf->data[0] |= (1 << 12) | (1 << 15);

    ret = ioctl(fd, MV_GET_CMD, mvbuf);

    if (ret < 0) {
        ERROR("ioctl fail\n");
        goto out;
    }

    mvbuf->is_write = 0;

    while (1) {
        ret = ioctl(fd, MV_GET_CMD, mvbuf);

        if (ret < 0) {
            ERROR("ioctl fail\n");
            goto out;
        }

        if ((mvbuf->data[0] & (1 << 15)) == 0) {
            break;
        }

        usleep(100);
    }

    return 0;
out:
    return -1;
}
#endif

#if 0
static void eth_dev_into_negotiation_get(void)
{
    int ret;
    int mv5072_fd = -1;
    int mv5050_fd = -1;
    struct mv_data mvbuf = {
        .portaddr = 0,
        .regnum = 0xa11,
    };

    mv5072_fd = open(MV5072_DEV_PATH, O_RDWR);

    if (mv5072_fd < 0) {
        ERROR("open mv5072 device fail fd %d\n", mv5072_fd);
        goto out;
    }

    mv5050_fd = open(MV5050_DEV_PATH, O_RDWR);

    if (mv5050_fd < 0) {
        ERROR("open mv5050 device fail fd %d\n", mv5050_fd);
        goto out;
    }

    for (uint8_t i = MV5072_PORT_MIN; i <= MV5072_PORT_MAX; i++) {

        if (i == MV_PORT_RESERVED)
            continue;

        set_para_value(mvbuf.portaddr, i);

        ret = ioctl(mv5072_fd, MV_GET_AUTO_NEGOTIATION_STATE, &mvbuf);

        if (ret < 0) {
            ERROR("ioctl fail\n");
            goto out;
        }

        DBG("mvbuf.portaddr=%x,mvbuf.data[0]=%x\n", mvbuf.portaddr,
            mvbuf.data[0]);
    }

    //P_6 SGMI //P_7 RGMI  //P_8 5050<->5072
    for (uint8_t i = MV5050_PORT_MIN; i <= MV5050_PORT_MAX; i++) {

        if (i == MV_PORT_RESERVED)
            continue;

        set_para_value(mvbuf.portaddr, i);
        ret = ioctl(mv5050_fd, MV_GET_AUTO_NEGOTIATION_STATE, &mvbuf);

        if (ret < 0) {
            ERROR("ioctl fail\n");
            goto out;
        }

        DBG("mvbuf.portaddr=%x,mvbuf.data[0]=%x\n", mvbuf.portaddr,
            mvbuf.data[0]);
    }

    DBG("dev set into negotiation mode ......\n");
out:

    if (mv5072_fd >= 0)
        close(mv5072_fd);

    if (mv5050_fd >= 0)
        close(mv5050_fd);
}

static void get_eth_set_negotiation(void)
{
    eth_dev_into_negotiation_set(INTO_NEGOTIATION);
}
#endif

#if 0
static void eth_dev_into_negotiation_set(uint32_t nego_mode)
{
    int ret;
    int mv5072_fd = -1;
    int mv5050_fd = -1;
    struct mv_data mvbuf = {
        .portaddr = 0,
        .regnum = 0xa11,
    };

    mv5072_fd = open(MV5072_DEV_PATH, O_RDWR);

    if (mv5072_fd < 0) {
        ERROR("open mv5072 device fail fd %d\n", mv5072_fd);
        goto out;
    }

    mv5050_fd = open(MV5050_DEV_PATH, O_RDWR);

    if (mv5050_fd < 0) {
        ERROR("open mv5050 device fail fd %d\n", mv5050_fd);
        goto out;
    }

    for (uint8_t i = MV5072_PORT_MIN; i <= MV5072_PORT_MAX; i++) {

        if (i == MV_PORT_RESERVED)
            continue;

        set_para_value(mvbuf.portaddr, i);
        set_para_value(mvbuf.data[0], nego_mode);

        ret = ioctl(mv5072_fd, MV_SET_AUTO_NEGOTIATION, &mvbuf);

        if (ret < 0) {
            ERROR("ioctl fail\n");
            goto out;
        }

        ret = ioctl(mv5072_fd, MV_GET_AUTO_NEGOTIATION_STATE, &mvbuf);

        if (ret < 0) {
            ERROR("ioctl fail\n");
            goto out;
        }

        DBG("mvbuf.portaddr=%x,mvbuf.data[0]=%x\n", mvbuf.portaddr,
            mvbuf.data[0]);
    }

    //P_6 SGMI //P_7 RGMI  //P_8 5050<->5072
    for (uint8_t i = MV5050_PORT_MIN; i <= MV5050_PORT_MAX; i++) {

        if (i == MV_PORT_RESERVED)
            continue;

        set_para_value(mvbuf.portaddr, i);
        set_para_value(mvbuf.data[0], nego_mode);

        ret = ioctl(mv5050_fd, MV_SET_AUTO_NEGOTIATION, &mvbuf);

        if (ret < 0) {
            ERROR("ioctl fail\n");
            goto out;
        }

        ret = ioctl(mv5050_fd, MV_GET_AUTO_NEGOTIATION_STATE, &mvbuf);

        if (ret < 0) {
            ERROR("ioctl fail\n");
            goto out;
        }

        DBG("mvbuf.portaddr=%x,mvbuf.data[0]=%x\n", mvbuf.portaddr,
            mvbuf.data[0]);
    }

    DBG("dev set into negotiation mode ......\n");
out:

    if (mv5072_fd >= 0)
        close(mv5072_fd);

    if (mv5050_fd >= 0)
        close(mv5050_fd);
}
#endif
