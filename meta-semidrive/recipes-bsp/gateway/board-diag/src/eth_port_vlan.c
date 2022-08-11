/*
 * eth_port_vlan.c
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

static int eth_vlan_write_mgmt_ext(int fd, uint16_t ext_bits)
{
    struct mv_data mvbuf;

    mvbuf.is_write = 1;
    mvbuf.portaddr = MV_5072_GLOBAL1_PORT;
    mvbuf.regnum = 0x16;
    mvbuf.data[0] = ext_bits;

    return ioctl(fd, MV_GET_CMD, &mvbuf);
}

static int eth_vlan_table_5072_port_active(uint8_t port)
{
    int ret = -1;
    int fd = -1;
    int i;
    struct mv_data mvbuf;

    fd = open(MV5072_DEV_PATH, O_RDWR);

    if (fd < 0) {
        ERROR("5072_vlan_table fail\n");
        goto out;
    }

    mvbuf.is_write = 1;

    mvbuf.regnum = MV_PORT_VLAN_MAP_REG;
    mvbuf.portaddr = 0;
    mvbuf.data[0] = 0x7fe;
    eth_vlan_write_mgmt_ext(fd, 1);
    ret = ioctl(fd, MV_GET_CMD, &mvbuf);

    if (ret < 0) {
        ERROR("ioctl fail\n");
        goto out;
    }

    mvbuf.regnum = MV_PORT_VLAN_MAP_REG;
    mvbuf.portaddr = 11;
    mvbuf.data[0] = (1 << port) + 1;
    ret = ioctl(fd, MV_GET_CMD, &mvbuf);

    if (ret < 0) {
        ERROR("ioctl fail\n");
        goto out;
    }

    for (i = 1; i <= 10; i++) {
        mvbuf.portaddr = i;

        if (i == port) {
            eth_vlan_write_mgmt_ext(fd, 1);
        }

        mvbuf.data[0] = 1;
        ret = ioctl(fd, MV_GET_CMD, &mvbuf);

        if (ret < 0) {
            ERROR("ioctl fail\n");
            goto out;
        }
    }

    mvbuf.regnum = MV_5072_PORT_CONTROL_2_REG;
    mvbuf.portaddr = 0;
    mvbuf.data[0] = 0x2080;
    ret = ioctl(fd, MV_GET_CMD, &mvbuf);

    if (ret < 0) {
        ERROR("ioctl fail\n");
        goto out;
    }

    for (i = 1; i <= 11; i++) {
        mvbuf.portaddr = i;
        mvbuf.is_write = 0;
        ret = ioctl(fd, MV_GET_CMD, &mvbuf);

        if (ret < 0) {
            ERROR("ioctl fail\n");
            goto out;
        }

        mvbuf.is_write = 1;
        mvbuf.data[0] &= ~(1 << 7);
        ret = ioctl(fd, MV_GET_CMD, &mvbuf);

        if (ret < 0) {
            ERROR("ioctl fail\n");
            goto out;
        }
    }

out:

    if (fd >= 0)
        close(fd);

    return ret;
}

static int eth_vlan_table_5050_all_port_negative(void)
{
    int ret = -1;
    int fd = -1;
    int i;
    struct mv_data mvbuf;

    fd = open(MV5050_DEV_PATH, O_RDWR);

    if (fd < 0) {
        ERROR("5050_vlan_table fail\n");
        goto out;
    }

    mvbuf.is_write = 1;
    mvbuf.regnum = MV_PORT_VLAN_MAP_REG;

    mvbuf.portaddr = 0;
    mvbuf.data[0] = 0x1fe;
    ret = ioctl(fd, MV_GET_CMD, &mvbuf);

    if (ret < 0) {
        ERROR("ioctl fail\n");
        goto out;
    }

    for (i = 1; i <= 8; i++) {
        mvbuf.portaddr = i;
        mvbuf.data[0] = 1;
        ret = ioctl(fd, MV_GET_CMD, &mvbuf);

        if (ret < 0) {
            ERROR("ioctl fail\n");
            goto out;
        }
    }

    mvbuf.regnum = MV_5072_PORT_CONTROL_2_REG;
    mvbuf.portaddr = 0;
    mvbuf.data[0] = 0x2080;
    ret = ioctl(fd, MV_GET_CMD, &mvbuf);

    if (ret < 0) {
        ERROR("ioctl fail\n");
        goto out;
    }

    for (i = 1; i <= 8; i++) {
        mvbuf.portaddr = i;
        mvbuf.is_write = 0;
        ret = ioctl(fd, MV_GET_CMD, &mvbuf);

        if (ret < 0) {
            ERROR("ioctl fail\n");
            goto out;
        }

        mvbuf.is_write = 1;
        mvbuf.data[0] &= ~(1 << 7);
        ret = ioctl(fd, MV_GET_CMD, &mvbuf);

        if (ret < 0) {
            ERROR("ioctl fail\n");
            goto out;
        }
    }

out:

    if (fd >= 0)
        close(fd);

    return ret;
}

static int eth_vlan_table_5050_port_active(uint8_t port)
{
    int ret = -1;
    int fd = -1;
    int i;
    struct mv_data mvbuf;

    eth_vlan_table_5072_port_active(8);
    usleep(100);
    fd = open(MV5050_DEV_PATH, O_RDWR);

    if (fd < 0) {
        ERROR("5050_vlan_table fail\n");
        goto out;
    }

    mvbuf.is_write = 1;
    mvbuf.regnum = MV_PORT_VLAN_MAP_REG;

    mvbuf.portaddr = 0;
    mvbuf.data[0] = 0x1fe;
    ret = ioctl(fd, MV_GET_CMD, &mvbuf);

    if (ret < 0) {
        ERROR("ioctl fail\n");
        goto out;
    }

    mvbuf.portaddr = 8;
    mvbuf.data[0] = (1 << port) + 1;
    ret = ioctl(fd, MV_GET_CMD, &mvbuf);

    if (ret < 0) {
        ERROR("ioctl fail\n");
        goto out;
    }

    for (i = 1; i <= 7; i++) {
        mvbuf.portaddr = i;

        if (i == port) {
            mvbuf.data[0] = (1 << 8) + 1;
        }
        else {
            mvbuf.data[0] = 1;
        }

        ret = ioctl(fd, MV_GET_CMD, &mvbuf);

        if (ret < 0) {
            ERROR("ioctl fail\n");
            goto out;
        }
    }

    mvbuf.regnum = MV_5072_PORT_CONTROL_2_REG;
    mvbuf.portaddr = 0;
    mvbuf.data[0] = 0x2080;
    ret = ioctl(fd, MV_GET_CMD, &mvbuf);

    if (ret < 0) {
        ERROR("ioctl fail\n");
        goto out;
    }

    for (i = 1; i <= 8; i++) {
        mvbuf.portaddr = i;
        mvbuf.is_write = 0;
        ret = ioctl(fd, MV_GET_CMD, &mvbuf);

        if (ret < 0) {
            ERROR("ioctl fail\n");
            goto out;
        }

        mvbuf.is_write = 1;
        mvbuf.data[0] &= ~(1 << 7);
        ret = ioctl(fd, MV_GET_CMD, &mvbuf);

        if (ret < 0) {
            ERROR("ioctl fail\n");
            goto out;
        }
    }

out:

    if (fd >= 0)
        close(fd);

    return ret;
}

int eth_vlan_table_config(uint32_t switch_mode, uint8_t port)
{
    int ret = -1;

    if (MV5050_DEV_ID == switch_mode) {
        if ((ret = eth_vlan_table_5050_port_active(port)) < 0)
            goto out;
    }
    else if (MV5072_DEV_ID == switch_mode) {
        if ((ret = eth_vlan_table_5050_all_port_negative()) < 0)
            goto out;

        if ((ret = eth_vlan_table_5072_port_active(port)) < 0)
            goto out;
    }

out:

    if (ret < 0) {
        ERROR("switch_mode = %x eth_vlan_table_port_active fail\n", switch_mode);
    }

    return ret;
}

void eth_socket_change_port_active(uint8_t switch_id, uint8_t switch_port)
{
    if (switch_id == MV5050_DEV_ID) {
        eth_vlan_table_5050_port_active(switch_port);
    }
    else if (switch_id == MV5072_DEV_ID) {
        eth_vlan_table_5072_port_active(switch_port);
    }
}

#if 0
//static int eth_vlan_start_send_packet(int fd);
//static void eth_vlan_check_5050_packet_count(int fd,
//        eth_cnt_info_t *eth_cnt_info);
//static void eth_vlan_check_5072_packet_count(int fd,
//        eth_cnt_info_t *eth_cnt_info);
//static bool eth_vlan_vcount_caculate(uint8_t switch_id, uint8_t switch_port,
//                                     struct mv_data *mvbuf, eth_cnt_info_t *eth_cnt_info);
//static int eth_vlan_stop_send_packet(int fd);
//static int eth_vlan_5050_reset_counts(int fd);
//static int eth_vlan_5072_reset_counts(int fd);

int eth_vlan_config(MASTER_SLAVER_MODE mode)
{
    int ret;

    ret = eth_vlan_5050_config(mode);

    if (ret < 0) {
        ERROR("eth_5050_vlan_config fail\n");
        return ret;
    }

    ret = eth_vlan_5072_config(mode);

    if (ret < 0) {
        ERROR("eth_5072_vlan_config fail\n");
        return ret;
    }

    return 0;
}

static int eth_vlan_5072_config(MASTER_SLAVER_MODE mode)
{
    int fd = -1;
    int ret = -1;

    fd = open(MV5072_DEV_PATH, O_RDWR);

    if (fd < 0) {
        ERROR("open 5072 device fail fd %d\n", fd);
        goto out;
    }

    ret = eth_vlan_5072_set_map(fd, mode);

    if (ret < 0) {
        ERROR("eth_vlan_5072_set_map fail\n");
        goto out;
    }

    ret = eth_vlan_5072_reset_counts(fd);

    if (ret < 0) {
        ERROR("eth_vlan_5072_reset_counts fail\n");
        goto out;
    }

out:

    if (fd >= 0)
        close(fd);

    return ret;
}

static int eth_vlan_5050_config(MASTER_SLAVER_MODE mode)
{
    int fd = -1;
    int ret = -1;

    fd = open(MV5050_DEV_PATH, O_RDWR);

    if (fd < 0) {
        ERROR("open 5050 device fail fd %d\n", fd);
        goto out;
    }

    ret = eth_vlan_5050_set_map(fd);

    if (ret < 0) {
        ERROR("eth_vlan_5050_set_map fail\n");
        goto out;
    }

    ret = eth_vlan_5050_reset_counts(fd);

    if (ret < 0) {
        ERROR("eth_vlan_5050_reset_counts fail\n");
        goto out;
    }

out:

    if (fd >= 0)
        close(fd);

    return ret;
}

static int eth_vlan_5072_set_map(int fd, MASTER_SLAVER_MODE mode)
{
    struct mv_data mvbuf;
    uint8_t port;
    int ret = -1;

    /* Master VLAN, p1->p2, p2->p3, p3->p4, p4->p5, p5->p6, p6->p9, p9->p10, p10->p8, p8->p11, p11->p1
     * Slave VLAN, p1->p2, p2->p3, p3->p4, p4->p5, p5->p6, p6->p9, p9->p10, p10->p8, p8->p1
     */

    mvbuf.is_write = 1;
    mvbuf.regnum = MV_PORT_VLAN_MAP_REG;

    mvbuf.portaddr = 0;
    mvbuf.data[0] = 0x0000;
    ret = ioctl(fd, MV_GET_CMD, &mvbuf);

    if (ret < 0) {
        ERROR("ioctl fail\n");
        return ret;
    }

    for (port = 1; port <= 5; port++) {

        mvbuf.portaddr = port;
        mvbuf.data[0] = 1 << (port + 1);
        ret = ioctl(fd, MV_GET_CMD, &mvbuf);

        if (ret < 0) {
            ERROR("ioctl fail\n");
            return ret;
        }
    }

    mvbuf.portaddr = 6;
    mvbuf.data[0] = 0x0200;
    ret = ioctl(fd, MV_GET_CMD, &mvbuf);

    if (ret < 0) {
        ERROR("ioctl fail\n");
        return ret;
    }

    mvbuf.portaddr = 9;
    mvbuf.data[0] = 0x0400;
    ret = ioctl(fd, MV_GET_CMD, &mvbuf);

    if (ret < 0) {
        ERROR("ioctl fail\n");
        return ret;
    }

    mvbuf.portaddr = 10;
    mvbuf.data[0] = 0x0100;
    ret = ioctl(fd, MV_GET_CMD, &mvbuf);

    if (ret < 0) {
        ERROR("ioctl fail\n");
        return ret;
    }

    mvbuf.portaddr = 8;

    if (mode == XMASTER_MODE) {
        mvbuf.data[0] = 0x0800;
    }
    else {
        mvbuf.data[0] = 0x0002;
    }

    ret = ioctl(fd, MV_GET_CMD, &mvbuf);

    if (ret < 0) {
        ERROR("ioctl fail\n");
        return ret;
    }

    mvbuf.portaddr = 11;

    if (mode == XMASTER_MODE) {
        mvbuf.data[0] = 0x0002;
    }
    else {
        mvbuf.data[0] = 0x0000;
    }

    ret = ioctl(fd, MV_GET_CMD, &mvbuf);

    if (ret < 0) {
        ERROR("ioctl fail\n");
        return ret;
    }

    return 0;
}

static int eth_vlan_5050_set_map(int fd)
{
    struct mv_data mvbuf;
    uint8_t port;
    int ret = -1;

    /* Set vlan p1->p2, p2->p3, p3->p4, p4->p5, p5->p6, p6->p7, p7->p8, p8->p1 */

    mvbuf.regnum   = MV_PORT_VLAN_MAP_REG;
    mvbuf.is_write = 1;
    mvbuf.portaddr = 0;
    mvbuf.data[0] = 0x0000;
    ret = ioctl(fd, MV_GET_CMD, &mvbuf);

    if (ret < 0) {
        ERROR("ioctl fail\n");
        return ret;
    }

    for (port = MV5050_PORT_MIN; port <= MV5050_PORT_MAX; port++) {

        mvbuf.portaddr = port;

        if (port != MV5050_PORT_MAX) {
            mvbuf.data[0] = 1 << (port + 1);
        }
        else {
            mvbuf.data[0] = 0x0002;
        }

        ret = ioctl(fd, MV_GET_CMD, &mvbuf);

        if (ret < 0) {
            ERROR("ioctl fail\n");
            return ret;
        }
    }

    return 0;
}
#endif
#if 0
static int eth_vlan_5072_reset_counts(int fd)
{
    struct mv_data mvbuf;
    uint8_t port;
    int ret = -1;

    mvbuf.is_write = 1;

    /* Reset port 1~6 CRC counter and packet counter */

    mvbuf.data[0] = MV_INTERNAL_PHY;
    mvbuf.data[1] = MV_INTERNAL_PHY_DEVICE3;
    mvbuf.data[2] = 0x0010;

    for (port = 1; port <= 6; port++) {

        mvbuf.portaddr = port;

        mvbuf.regnum = MV_INTERNAL_PHY_CHECKER_CTL_REG;
        ret = ioctl(fd, MV_ACCESS_PHY, &mvbuf);

        if (ret < 0) {
            ERROR("ioctl fail\n");
            return ret;
        }

        mvbuf.regnum = MV_INTERNAL_PHY_PACKET_GEN_REG;
        ret = ioctl(fd, MV_ACCESS_PHY, &mvbuf);

        if (ret < 0) {
            ERROR("ioctl fail\n");
            return ret;
        }
    }

    /* Reset port 7 CRC counter and packet counter */

    //eth_write_100base_tx_reg(fd, 6, 18, 0x10);
    //eth_write_100base_tx_reg(fd, 6, 16, 0x10);

    /* Reset P9 packet/crc counter */

    mvbuf.portaddr = 1;
    mvbuf.regnum = 0xfd07;
    mvbuf.data[0] = MV_EXTERNAL_PHY;
    mvbuf.data[1] = MV_EXTERNAL_PHY_DEVICE3;
    mvbuf.data[2] = 0x0008;
    ret = ioctl(fd, MV_ACCESS_PHY, &mvbuf);

    if (ret < 0) {
        ERROR("ioctl fail\n");
        return ret;
    }

    /* Reset P10 packet/crc counter */

    mvbuf.portaddr = 2;
    mvbuf.regnum = 0xfd07;
    mvbuf.data[0] = MV_EXTERNAL_PHY;
    mvbuf.data[1] = MV_EXTERNAL_PHY_DEVICE3;
    mvbuf.data[2] = 0x0008;
    ret = ioctl(fd, MV_ACCESS_PHY, &mvbuf);

    if (ret < 0) {
        ERROR("ioctl fail\n");
        return ret;
    }

    /* Reset P9 packet/crc counter */

    mvbuf.portaddr = 1;
    mvbuf.regnum = 0xfd07;
    mvbuf.data[0] = MV_EXTERNAL_PHY;
    mvbuf.data[1] = MV_EXTERNAL_PHY_DEVICE3;
    mvbuf.data[2] = 0x0005;
    ret = ioctl(fd, MV_ACCESS_PHY, &mvbuf);

    if (ret < 0) {
        ERROR("ioctl fail\n");
        return ret;
    }

    /* Reset P10 packet/crc counter */

    mvbuf.portaddr = 2;
    mvbuf.regnum = 0xfd07;
    mvbuf.data[0] = MV_EXTERNAL_PHY;
    mvbuf.data[1] = MV_EXTERNAL_PHY_DEVICE3;
    mvbuf.data[2] = 0x0005;
    ret = ioctl(fd, MV_ACCESS_PHY, &mvbuf);

    if (ret < 0) {
        ERROR("ioctl fail\n");
        return ret;
    }

    return 0;
}

static int eth_vlan_5050_reset_counts(int fd)
{
    struct mv_data mvbuf;
    uint8_t port;
    int ret = -1;

    mvbuf.is_write = 1;

    /* Reset and Enable port 1~5 CRC counter and packet counter */

    mvbuf.data[0] = MV_INTERNAL_PHY;
    mvbuf.data[1] = MV_INTERNAL_PHY_DEVICE3;
    mvbuf.data[2] = 0x0010;

    for (port = 1; port <= 5; port++) {

        mvbuf.portaddr = port;

        mvbuf.regnum = MV_INTERNAL_PHY_CHECKER_CTL_REG;
        ret = ioctl(fd, MV_ACCESS_PHY, &mvbuf);

        if (ret < 0) {
            ERROR("ioctl fail\n");
            return ret;
        }

        mvbuf.regnum = MV_INTERNAL_PHY_PACKET_GEN_REG;
        ret = ioctl(fd, MV_ACCESS_PHY, &mvbuf);

        if (ret < 0) {
            ERROR("ioctl fail\n");
            return ret;
        }
    }

    /* Reset P6 packet/crc counter */

    mvbuf.portaddr = 2;
    mvbuf.regnum = 0xfd07;
    mvbuf.data[0] = MV_EXTERNAL_PHY;
    mvbuf.data[1] = MV_EXTERNAL_PHY_DEVICE3;
    mvbuf.data[2] = 0x0008;
    ret = ioctl(fd, MV_ACCESS_PHY, &mvbuf);

    if (ret < 0) {
        ERROR("ioctl fail\n");
        return ret;
    }

    /* Reset P7 packet/crc counter */

    mvbuf.portaddr = 1;
    mvbuf.regnum = 0xfd07;
    mvbuf.data[0] = MV_EXTERNAL_PHY;
    mvbuf.data[1] = MV_EXTERNAL_PHY_DEVICE3;
    mvbuf.data[2] = 0x0008;
    ret = ioctl(fd, MV_ACCESS_PHY, &mvbuf);

    if (ret < 0) {
        ERROR("ioctl fail\n");
        return ret;
    }

    /* Enable P6 packet/crc counter */

    mvbuf.portaddr = 2;
    mvbuf.regnum = 0xfd07;
    mvbuf.data[0] = MV_EXTERNAL_PHY;
    mvbuf.data[1] = MV_EXTERNAL_PHY_DEVICE3;
    mvbuf.data[2] = 0x0005;
    ret = ioctl(fd, MV_ACCESS_PHY, &mvbuf);

    if (ret < 0) {
        ERROR("ioctl fail\n");
        return ret;
    }

    /* Enable P7 packet/crc counter */

    mvbuf.portaddr = 1;
    mvbuf.regnum = 0xfd07;
    mvbuf.data[0] = MV_EXTERNAL_PHY;
    mvbuf.data[1] = MV_EXTERNAL_PHY_DEVICE3;
    mvbuf.data[2] = 0x0005;
    ret = ioctl(fd, MV_ACCESS_PHY, &mvbuf);

    if (ret < 0) {
        ERROR("ioctl fail\n");
        return ret;
    }

    return 0;
}

static int eth_vlan_stop_send_packet(int fd)
{
    struct mv_data mvbuf;
    int ret = -1;

    mvbuf.is_write = 1;
    mvbuf.portaddr = 1;
    mvbuf.data[0] = MV_INTERNAL_PHY;
    mvbuf.data[1] = MV_INTERNAL_PHY_DEVICE3;

    mvbuf.regnum = MV_INTERNAL_PHY_PACKET_GEN_REG;
    mvbuf.data[2] = 0x0000;

    ret = ioctl(fd, MV_ACCESS_PHY, &mvbuf);

    if (ret < 0) {
        ERROR("ioctl fail\n");
        goto out;
    }

out:
    return ret;
}

int eth_vlan_start_send_packet(int fd)
{
    struct mv_data mvbuf;
    int ret = -1;

    mvbuf.is_write = 1;
    mvbuf.portaddr = 1;
    mvbuf.data[0] = MV_INTERNAL_PHY;
    mvbuf.data[1] = MV_INTERNAL_PHY_DEVICE3;

    /* Disable packet generator constant payload,
     * and set the inter-packet gap to be 12 byte
     */

    mvbuf.regnum = MV_INTERNAL_PHY_PACKET_CTL_REG;
    mvbuf.data[2] = 0x000b;

    ret = ioctl(fd, MV_ACCESS_PHY, &mvbuf);

    if (ret < 0) {
        ERROR("ioctl fail\n");
        goto out;
    }

    /* Set packet burst to be continuous */

    mvbuf.regnum = MV_INTERNAL_PHY_PACKET_SIZE_REG;
    mvbuf.data[2] = 0x0000;

    ret = ioctl(fd, MV_ACCESS_PHY, &mvbuf);

    if (ret < 0) {
        ERROR("ioctl fail\n");
        goto out;
    }

    /* Enable P1 CRC checker, packet counter and PACKET GENERATOR,
     * set payload to be random and packet length to be 64 bytes
     */

    mvbuf.regnum = MV_INTERNAL_PHY_PACKET_GEN_REG;
    mvbuf.data[2] = 0x0018;

    ret = ioctl(fd, MV_ACCESS_PHY, &mvbuf);

    if (ret < 0) {
        ERROR("ioctl fail\n");
        goto out;
    }

out:
    return ret;
}

static bool eth_vlan_vcount_caculate(uint8_t switch_id, uint8_t switch_port,
                                     struct mv_data *mvbuf, eth_cnt_info_t *eth_cnt_info)
{
    bool ret = false;

    if (switch_id == MV5050_DEV_ID) {
        if (eth_cnt_info->mv_5050_vcnt_val[switch_port]) {
            eth_cnt_info->mv_5050_vcnt_val[switch_port] = 0;
            eth_cnt_info->mv_5050_vcnt_resp[switch_port]++;
            mvbuf->data[0] = eth_cnt_info->mv_5050_vcnt_resp[switch_port];
            ret = true;
        }
    }
    else {
        if (eth_cnt_info->mv_5072_vcnt_val[switch_port]) {
            eth_cnt_info->mv_5072_vcnt_val[switch_port] = 0;
            eth_cnt_info->mv_5072_vcnt_resp[switch_port]++;
            mvbuf->data[0] = eth_cnt_info->mv_5072_vcnt_resp[switch_port];
            ret = true;
        }
    }

    return ret;
}

static void eth_vlan_check_5072_packet_count(int fd,
        eth_cnt_info_t *eth_cnt_info)
{
    struct mv_data mvbuf;
    int ret = -1;

    /* Port1~6 Link Drop, Packet, CRC Counter */

    mvbuf.is_write = 0;
    mvbuf.data[1] = MV_INTERNAL_PHY_DEVICE3;

    for (uint8_t port = 1; port <= 6; port++) {
        mvbuf.portaddr = port;

        mvbuf.data[0] = MV_INTERNAL_PHY;
        mvbuf.regnum = MV_INTERNAL_PHY_DROP_COUNT_REG;
        ret = ioctl(fd, MV_ACCESS_PHY, &mvbuf);

        if (ret < 0) {
            ERROR("ioctl fail\n");
            goto out;
        }

        DBG("5072 port:%d, link drop:%d\n", port, mvbuf.data[0]);

        mvbuf.data[0] = MV_INTERNAL_PHY;
        mvbuf.regnum = MV_INTERNAL_PHY_PACKET_COUNT_REG;
        ret = ioctl(fd, MV_ACCESS_PHY, &mvbuf);

        if (ret < 0) {
            ERROR("ioctl fail\n");
            goto out;
        }

        eth_cnt_info->mv_5072_vcnt_val[port] = mvbuf.data[0];
        DBG("5072 port:%d, packet count:%d\n", port, mvbuf.data[0]);

        mvbuf.data[0] = MV_INTERNAL_PHY;
        mvbuf.regnum = MV_INTERNAL_PHY_CRC_COUNT_REG;
        ret = ioctl(fd, MV_ACCESS_PHY, &mvbuf);

        if (ret < 0) {
            ERROR("ioctl fail\n");
            goto out;
        }

        DBG("5072 port:%d, crc count:%d\n", port, mvbuf.data[0]);
    }

#if 0
    /* Port7 CRC/Packet Counter */

    mvbuf.data[0] = eth_read_100base_tx_reg(fd, 6, 17);
    eth_cnt_info->mv_5072_vcnt_val[7] = (mvbuf.data[0] >> 8);
    DBG("5072 port:%d, packet count:%d\n", 7, (mvbuf.data[0] >> 8));
    DBG("5072 port:%d, crc count:%d\n", 7, (mvbuf.data[0] & 0xff));
#endif
    /* Port9 CRC/Packet Counter */

    mvbuf.is_write = 0;
    mvbuf.portaddr = 1;
    mvbuf.regnum = 0xfd08;
    mvbuf.data[0] = MV_EXTERNAL_PHY;
    mvbuf.data[1] = MV_EXTERNAL_PHY_DEVICE3;
    ret = ioctl(fd, MV_ACCESS_PHY, &mvbuf);

    if (ret < 0) {
        ERROR("ioctl fail\n");
        goto out;
    }

    eth_cnt_info->mv_5072_vcnt_val[9] = mvbuf.data[0];
    DBG("5072 port:%d, packet count:%d\n", 9, mvbuf.data[0]);

    /* Port10 CRC/Packet Counter */

    mvbuf.is_write = 0;
    mvbuf.portaddr = 2;
    mvbuf.regnum = 0xfd08;
    mvbuf.data[0] = MV_EXTERNAL_PHY;
    mvbuf.data[1] = MV_EXTERNAL_PHY_DEVICE3;
    ret = ioctl(fd, MV_ACCESS_PHY, &mvbuf);

    if (ret < 0) {
        ERROR("ioctl fail\n");
        goto out;
    }

    eth_cnt_info->mv_5072_vcnt_val[10] = mvbuf.data[0];
    DBG("5072 port:%d, packet count:%d\n", 10, mvbuf.data[0]);

out:
    return;
}

static void eth_vlan_check_5050_packet_count(int fd,
        eth_cnt_info_t *eth_cnt_info)
{
    struct mv_data mvbuf;
    int ret = -1;

    /* Port1~5 Link Drop, Packet, CRC Counter */

    mvbuf.is_write = 0;
    mvbuf.data[1] = MV_INTERNAL_PHY_DEVICE3;

    for (uint8_t port = 1; port <= 5; port++) {
        mvbuf.portaddr = port;

        mvbuf.data[0] = MV_INTERNAL_PHY;
        mvbuf.regnum = MV_INTERNAL_PHY_DROP_COUNT_REG;
        ret = ioctl(fd, MV_ACCESS_PHY, &mvbuf);

        if (ret < 0) {
            ERROR("ioctl fail\n");
            goto out;
        }

        DBG("5050 port:%d, link drop:%d\n", port, mvbuf.data[0]);

        mvbuf.data[0] = MV_INTERNAL_PHY;
        mvbuf.regnum = MV_INTERNAL_PHY_PACKET_COUNT_REG;
        ret = ioctl(fd, MV_ACCESS_PHY, &mvbuf);

        if (ret < 0) {
            ERROR("ioctl fail\n");
            goto out;
        }

        eth_cnt_info->mv_5050_vcnt_val[port] = mvbuf.data[0];
        DBG("5050 port:%d, packet count:%d\n", port, mvbuf.data[0]);

        mvbuf.data[0] = MV_INTERNAL_PHY;
        mvbuf.regnum = MV_INTERNAL_PHY_CRC_COUNT_REG;
        ret = ioctl(fd, MV_ACCESS_PHY, &mvbuf);

        if (ret < 0) {
            ERROR("ioctl fail\n");
            goto out;
        }

        DBG("5050 port:%d, crc count:%d\n", port, mvbuf.data[0]);
    }

    /* Port6 CRC/Packet Counter */

    mvbuf.is_write = 0;
    mvbuf.portaddr = 2;
    mvbuf.regnum = 0xfd08;
    mvbuf.data[0] = MV_EXTERNAL_PHY;
    mvbuf.data[1] = MV_EXTERNAL_PHY_DEVICE3;
    ret = ioctl(fd, MV_ACCESS_PHY, &mvbuf);

    if (ret < 0) {
        ERROR("ioctl fail\n");
        goto out;
    }

    eth_cnt_info->mv_5050_vcnt_val[6] = mvbuf.data[0];
    DBG("5050 port:%d, packet count:%d\n", 6, mvbuf.data[0]);

    /* Port7 CRC/Packet Counter */

    mvbuf.is_write = 0;
    mvbuf.portaddr = 1;
    mvbuf.regnum = 0xfd08;
    mvbuf.data[0] = MV_EXTERNAL_PHY;
    mvbuf.data[1] = MV_EXTERNAL_PHY_DEVICE3;
    ret = ioctl(fd, MV_ACCESS_PHY, &mvbuf);

    if (ret < 0) {
        ERROR("ioctl fail\n");
        goto out;
    }

    eth_cnt_info->mv_5050_vcnt_val[7] = mvbuf.data[0];
    DBG("5050 port:%d, packet count:%d\n", 7, mvbuf.data[0]);

out:
    return;
}
#endif
#if 0
static void eth_dev_static_ip_set(MASTER_SLAVER_MODE mode)
{
    if ((dev_master_slave.cur_state == dev_master_slave.pre_state)
            && (dev_master_slave.status != -1))
        return;

    if ((dev_master_slave.status = system(eth_up)) == -1)
        return;

    usleep(100);

    if (mode == XMASTER_MODE) {
        dev_master_slave.status = system(master_ip);
    }
    else {
        dev_master_slave.status = system(slave_ip);
    }

    if (dev_master_slave.status == -1)
        return;

    usleep(100);

    if ((dev_master_slave.status = system(ip_id)) == -1)
        return;

    dev_master_slave.pre_state = dev_master_slave.cur_state;

}
#endif

#if 0
static int eth_write_100base_tx_reg(int fd, uint16_t page_addr,
                                    uint16_t reg_addr, uint16_t data)
{
    struct mv_data mvbuf;
    int ret = -1;

    mvbuf.is_write = 1;
    mvbuf.data[0] = 0;

    /* write page addr*/

    mvbuf.portaddr = MV_5072_GLOBAL2_PORT;
    mvbuf.regnum = 0x19;
    mvbuf.data[0] = page_addr;

    ret = ioctl(fd, MV_GET_CMD, &mvbuf);

    if (ret < 0) {
        ERROR("ioctl fail\n");
        return ret;
    }

    mvbuf.portaddr = MV_5072_GLOBAL2_PORT;
    mvbuf.regnum = 0x18;
    mvbuf.data[0] = 0x96f6;

    ret = ioctl(fd, MV_GET_CMD, &mvbuf);

    if (ret < 0) {
        ERROR("ioctl fail\n");
        return ret;
    }

    /* write data */

    mvbuf.portaddr = MV_5072_GLOBAL2_PORT;
    mvbuf.regnum = 0x19;
    mvbuf.data[0] = data;

    ret = ioctl(fd, MV_GET_CMD, &mvbuf);

    if (ret < 0) {
        ERROR("ioctl fail\n");
        return ret;
    }

    mvbuf.portaddr = MV_5072_GLOBAL2_PORT;
    mvbuf.regnum = 0x18;
    mvbuf.data[0] = 0x96e0 | reg_addr;

    ret = ioctl(fd, MV_GET_CMD, &mvbuf);

    if (ret < 0) {
        ERROR("ioctl fail\n");
        return ret;
    }

    return 0;
}
#endif
#if 0
static int eth_read_100base_tx_reg(int fd, uint16_t page_addr,
                                   uint16_t reg_addr)
{
    struct mv_data mvbuf;
    int ret = -1;

    mvbuf.is_write = 1;
    mvbuf.data[0] = 0;

    /* write page addr*/

    mvbuf.portaddr = MV_5072_GLOBAL2_PORT;
    mvbuf.regnum = 0x19;
    mvbuf.data[0] = page_addr;

    ret = ioctl(fd, MV_GET_CMD, &mvbuf);

    if (ret < 0) {
        ERROR("ioctl fail\n");
        return ret;
    }

    mvbuf.portaddr = MV_5072_GLOBAL2_PORT;
    mvbuf.regnum = 0x18;
    mvbuf.data[0] = 0x96f6;

    ret = ioctl(fd, MV_GET_CMD, &mvbuf);

    if (ret < 0) {
        ERROR("ioctl fail\n");
        return ret;
    }

    /* read data */

    mvbuf.portaddr = MV_5072_GLOBAL2_PORT;
    mvbuf.regnum = 0x18;
    mvbuf.data[0] = 0x9ae0 | reg_addr;

    ret = ioctl(fd, MV_GET_CMD, &mvbuf);

    if (ret < 0) {
        ERROR("ioctl fail\n");
        return ret;
    }

    mvbuf.is_write = 0;
    mvbuf.portaddr = MV_5072_GLOBAL2_PORT;
    mvbuf.regnum = 0x19;
    mvbuf.data[0] = 0;

    ret = ioctl(fd, MV_GET_CMD, &mvbuf);

    if (ret < 0) {
        ERROR("ioctl fail\n");
        return ret;
    }

    return mvbuf.data[0];
}
#endif

#if 0
void eth_read_register(void)
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
        mvbuf.regnum   = 0x19;
        mvbuf.is_write = 1;
        mvbuf.data[0]  = 0x80;
        ret = ioctl(mv5072_fd, MV_GET_CMD, &mvbuf);

        if (ret < 0) {
            ERROR("ioctl fail\n");
            goto out;
        }

        DBG("i = %d 5072_ Port Control 3 = %x\n", i, mvbuf.data[0]);

        DBG("mvbuf.portaddr=%x,mvbuf.data[0]=%x\n", mvbuf.portaddr,
            mvbuf.data[0]);
    }

    //P_6 SGMI //P_7 RGMI  //P_8 5050<->5072
    for (uint8_t i = MV5050_PORT_MIN; i <= MV5050_PORT_MAX; i++) {

        if (i == MV_PORT_RESERVED)
            continue;

        mvbuf.portaddr = i;
        mvbuf.regnum   = 0x19;
        mvbuf.is_write = 1;
        mvbuf.data[0]  = 0x80;
        ret = ioctl(mv5050_fd, MV_GET_CMD, &mvbuf);

        if (ret < 0) {
            ERROR("ioctl fail\n");
            goto out;
        }

        DBG("i = %d 5050_ Port Control 3 = %x\n", i, mvbuf.data[0]);

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
