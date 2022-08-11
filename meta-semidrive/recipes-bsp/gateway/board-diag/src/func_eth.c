/*
 * func_eth.c
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
#include "sw_timer.h"

static int eth_set_1000t1_psd(int fd, uint8_t port);
static int eth_set_port_mode_into_master_slave(int fd,
        switch_info_t switch_info,
        MASTER_SLAVER_MODE mode);
static int eth_set_SGMII_port_mode_into_master_slave(int fd, uint32_t port,
        MASTER_SLAVER_MODE mode);
static int get_switch_port_id(eth_cmd_t *eth_cmd, switch_info_t *switch_info);
static int eth_enbale_auto_polarity(int fd);
static int eth_enable_TC10_mode(int fd);
static int eth_enable_programming_andremote_wake_up(int fd);
static int eth_switch_deep_sleep_mode(int fd);
static int eth_ignore_p2_p11_sleep_handshake(int fd);
static int eth_power_removal_after_TC10_sleep_handshare(int fd);
static int eth_into_sleep(int fd);
static bool lbu_ops(uint8_t switch_port_id, uint32_t addr, uint32_t *val,
                    uint32_t ops);
static bool phy_ops(uint8_t switch_port_id, uint32_t extern_flag,
                    uint32_t dev_addr, uint32_t addr, uint32_t *val, uint32_t ops);

static int get_switch_fd(uint8_t switch_id)
{
    int fd = -1;
    const char *switch_path[] = {
        MV5050_DEV_PATH,
        MV5072_DEV_PATH
    };

    if (switch_id == 0 || switch_id == 1)
        fd = open(switch_path[switch_id], O_RDWR);

    return fd;
}

static bool eth_dev_diff_mode_ops(test_exec_t *exec)
{
    int ret = -1;
    uint8_t pt = 0;
    int fd_5072 = -1;
    int fd_5050 = -1;
    switch_info_t switch_info;
    bool result = false;
    eth_cmd_t *eth_cmd = (eth_cmd_t *)&exec->cmd;
    uint8_t dev_mode = eth_cmd->master_slave;
    uint8_t test_mode = eth_cmd->test_mode;

    struct mv_data mvbuf = {
        .portaddr = 0,
        .regnum = 0xa11,
        .data[0] = 0,
    };

    ret = get_switch_port_id(eth_cmd, &switch_info);

    if (ret < 0) {
        DBG("get_switch_port_id fail\n");
        goto fail;
    }

    set_para_value(mvbuf.portaddr, switch_info.port);

    if (switch_info.port == 0) {
        DBG("no switch_info.port\n");
        goto fail;
    }

    fd_5050 = get_switch_fd(MV5050_DEV_ID);
    fd_5072 = get_switch_fd(MV5072_DEV_ID);

    set_para_value(mvbuf.data[0], test_mode);

    if ((fd_5050 < 0) || (fd_5072 < 0)) {
        ERROR("open switch device fail\n");
        goto fail;
    }

    /*set 5072/5050 switch port into master/slave dev_mode config*/
    if (switch_info.id == MV5050_DEV_ID) {
        ret = eth_set_port_mode_into_master_slave(fd_5050, switch_info, dev_mode);

        if (ret < 0) {
            ERROR("set_port_mode fail\n");
            goto fail;
        }

        /*set 2122 phy port into master/slave dev_mode*/
        if ((switch_info.port == 6) || (switch_info.port == 7)) {
            ret = eth_set_SGMII_port_mode_into_master_slave(fd_5050, switch_info.port - 5,
                    dev_mode);

            if (ret < 0) {
                ERROR("set_port_mode fail\n");
                goto fail;
            }
        }
    }
    else if (switch_info.id == MV5072_DEV_ID) {
        ret = eth_set_port_mode_into_master_slave(fd_5072, switch_info, dev_mode);

        if (ret < 0) {
            ERROR("set_port_mode fail\n");
            goto fail;
        }

        /*set 2122 phy port into master/slave mode config*/
        if ((switch_info.port == 9) || (switch_info.port == 10)) {
            ret = eth_set_SGMII_port_mode_into_master_slave(fd_5072, switch_info.port - 8,
                    dev_mode);

            if (ret < 0) {
                ERROR("set_port_mode fail\n");
                goto fail;
            }
        }
    }

    /*set 5050/5072 switch port into test mode config*/
    if (switch_info.id == MV5050_DEV_ID) {

        ret = ioctl(fd_5050, MV_SET_TEST_MODE, &mvbuf);

        if (ret < 0) {
            ERROR("ioctl fail\n");
            goto fail;
        }

        if (test_mode == 5) {
            if (switch_info.port == 6)
                pt = 1;
            else if (switch_info.port == 7)
                pt = 2;

            ret = eth_set_1000t1_psd(fd_5050, pt);

            if (ret < 0) {
                ERROR("eth_set_1000t1_psd fail\n");
                goto fail;
            }
        }
    }
    else if (switch_info.id == MV5072_DEV_ID) {

        ret = ioctl(fd_5072, MV_SET_TEST_MODE, &mvbuf);

        if (ret < 0) {
            ERROR("ioctl fail\n");
            goto fail;
        }

        if (test_mode == 5) {
            if (switch_info.port == 9)
                pt = 1;
            else if (switch_info.port == 10)
                pt = 2;

            ret = eth_set_1000t1_psd(fd_5072, pt);

            if (ret < 0) {
                ERROR("eth_set_1000t1_psd fail\n");
                goto fail;
            }
        }
    }

    if (ret < 0) {
        ERROR("ioctl fail\n");
        goto fail;
    }

    if ((test_mode == NORMAL_MODE) &&
            (dev_mode == XMASTER_MODE)) {

#ifdef SOCKET_TCP_CREATE
        ret = eth_socket_tcp_client_send_package(exec, switch_info);
#else
        ret = eth_socket_udp_client_send_package(exec, switch_info);
#endif

        if (ret < 0) {
            ERROR("eth_socket_client_send_package fail\n");
            goto fail;
        }

        result = true;
    }
    else if ((test_mode != NORMAL_MODE) || (dev_mode != XMASTER_MODE)) {
        result = true;
    }

fail:

    if (fd_5050 >= 0)
        close(fd_5050);

    if (fd_5072 >= 0)
        close(fd_5072);

    return result;
}

static int eth_set_port_mode_into_master_slave(int fd,
        switch_info_t switch_info,
        MASTER_SLAVER_MODE mode)
{
    int ret = -1;
    uint8_t id   = switch_info.id;
    uint8_t port = switch_info.port;

    struct mv_data mvbuf = {
        .portaddr = 0,
    };

    mvbuf.portaddr = port;
    mvbuf.is_write = 1;
    mvbuf.data[0]  = MV_INTERNAL_PHY;

    if (id == MV5072_DEV_ID) {
        mvbuf.regnum   = MV_5072_INTERNAL_PHY_PMA_PMD_REG;
        mvbuf.data[1] = MV_5072_INTERNAL_PHY_DEVICE1;

        if (mode == XMASTER_MODE) {
            mvbuf.data[2] = MV_5072_INTERNAL_PHY_MASTER_DATA;
        }
        else {
            mvbuf.data[2] = MV_5072_INTERNAL_PHY_SLAVE_DATA;
        }
    }
    else if (id == MV5050_DEV_ID) {
        mvbuf.regnum   = MV_5050_INTERNAL_PHY_PMA_PMD_REG;
        mvbuf.data[1] = MV_5050_INTERNAL_PHY_DEVICE1;

        if (mode == XMASTER_MODE) {
            mvbuf.data[2] = MV_5050_INTERNAL_PHY_MASTER_DATA;
        }
        else {
            mvbuf.data[2] = MV_5050_INTERNAL_PHY_SLAVE_DATA;
        }
    }

    ret = ioctl(fd, MV_ACCESS_PHY, &mvbuf);

    DBG("mvbuf.portaddr=%x,mvbuf.data[0]=%x\n", mvbuf.portaddr,
        mvbuf.data[0]);

    return ret;
}

static int eth_set_SGMII_port_mode_into_master_slave(int fd, uint32_t port,
        MASTER_SLAVER_MODE mode)
{
    int ret = -1;

    struct mv_data mvbuf;

    mvbuf.portaddr = port;
    mvbuf.is_write = 1;
    mvbuf.data[0] = MV_EXTERNAL_PHY;
    mvbuf.data[1] = MV_EXTERNAL_PHY_DEVICE1;
    mvbuf.regnum = 0x834;

    if (mode == XMASTER_MODE)
        mvbuf.data[2] = 0xc001;
    else
        mvbuf.data[2] = 0x8001;

    ret = ioctl(fd, MV_ACCESS_PHY, &mvbuf);

    if (ret < 0) {
        ERROR("SGMII port mode into master slave fail\n");
    }

    return ret;
}

static void eth_set_port_mode(MASTER_SLAVER_MODE mode)
{
    int ret;
    int mv5072_fd = -1;
    int mv5050_fd = -1;
    switch_info_t switch_info = {0};

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

        switch_info.id = MV5072_DEV_ID;
        switch_info.port = i;

        ret = eth_set_port_mode_into_master_slave(mv5072_fd, switch_info, mode);

        if (ret < 0) {
            ERROR("ioctl fail\n");
            goto out;
        }
    }

    //P_6 SGMI //P_7 RGMI  //P_8 5050<->5072
    for (uint8_t i = MV5050_PORT_MIN; i <= MV5050_PORT_MAX; i++) {

        if (i == MV_PORT_RESERVED)
            continue;

        switch_info.id = MV5050_DEV_ID;
        switch_info.port = i;

        ret = eth_set_port_mode_into_master_slave(mv5050_fd, switch_info, mode);

        if (ret < 0) {
            ERROR("ioctl fail\n");
            goto out;
        }
    }

    DBG("dev set into negotiation mode ......\n");

out:

    if (mv5072_fd >= 0)
        close(mv5072_fd);

    if (mv5050_fd >= 0)
        close(mv5050_fd);
}

static int  eth_set_1000t1_psd(int fd, uint8_t port)
{
    int ret = -1;

    struct mv_data mvbuf_2122[4] = {
        {
            .portaddr = 1,
            .regnum = MV_2122_POWER_SPECTRAL_FC10_ADDR,
            .is_write = 1,
            .data[0] = MV_EXTERNAL_PHY,
            .data[1] = MV_EXTERNAL_PHY_DEVICE3,
            .data[2] = MV_2122_POWER_SPECTRAL_FC10_VALUE
        },
        {
            .portaddr = 1,
            .regnum = MV_2122_POWER_SPECTRAL_FC11_ADDR,
            .is_write = 1,
            .data[0] = MV_EXTERNAL_PHY,
            .data[1] = MV_EXTERNAL_PHY_DEVICE3,
            .data[2] = MV_2122_POWER_SPECTRAL_FC11_VALUE
        },
        {
            .portaddr = 1,
            .regnum = MV_2122_POWER_SPECTRAL_FC12_ADDR,
            .is_write = 1,
            .data[0] = MV_EXTERNAL_PHY,
            .data[1] = MV_EXTERNAL_PHY_DEVICE3,
            .data[2] = MV_2122_POWER_SPECTRAL_FC12_VALUE
        },
        {
            .portaddr = 1,
            .regnum = MV_2122_POWER_SPECTRAL_FC13_ADDR,
            .is_write = 1,
            .data[0] = MV_EXTERNAL_PHY,
            .data[1] = MV_EXTERNAL_PHY_DEVICE3,
            .data[2] = MV_2122_POWER_SPECTRAL_FC13_VALUE
        }
    };

    for (uint8_t i = 0; i < (sizeof(mvbuf_2122) / sizeof(mvbuf_2122[0])); i++) {
        mvbuf_2122[i].portaddr = port;
        ret = ioctl(fd, MV_ACCESS_PHY, &mvbuf_2122[i]);
    }

    return ret;
}

static void eth_set_SGMII_port_mode(MASTER_SLAVER_MODE mode)
{
    int ret = -1;
    int mv5072_fd = -1;
    int mv5050_fd = -1;

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

    for (uint8_t i = 1; i <= 2 ; i++) {
        ret = eth_set_SGMII_port_mode_into_master_slave(mv5050_fd, i, mode);

        if (ret < 0) {
            ERROR("set SGMII port mode fail\n");
            goto out;
        }

        ret = eth_set_SGMII_port_mode_into_master_slave(mv5072_fd, i, mode);

        if (ret < 0) {
            ERROR("set SGMII port mode fail\n");
            goto out;
        }
    }

out:

    if (mv5072_fd >= 0)
        close(mv5072_fd);

    if (mv5050_fd >= 0)
        close(mv5050_fd);
}

static int get_switch_port_id(eth_cmd_t *eth_cmd, switch_info_t *switch_info)
{
    uint8_t dev_id = eth_cmd->dev_id;
    uint8_t channel = eth_cmd->channel_id;
    const eth_chn_table_t *eth_chn_table = NULL;

    if (dev_id == 0x36) {
        eth_chn_table = eth_100_base_t1;
    }
    else if (dev_id == 0x37) {
        eth_chn_table = eth_1000_base_t1;
    }
    else if (dev_id == 0x38) {
        eth_chn_table = eth_100_base_tx;
    }
    else {
        return -1;
    }

    for (uint8_t i = 0; i != ETH_INVALID_CHN_NUM; i++) {
        if (channel == eth_chn_table[i].chn_num) {
            switch_info->id = (eth_chn_table[i].port_num >> 4) & 0xf;
            switch_info->port = eth_chn_table[i].port_num & 0xf;
            return 1;
        }
    }

    return -1;
}

static bool exec_single_port(test_exec_t *exec)
{
    bool ret = false;

    ret = eth_dev_diff_mode_ops(exec);

    return ret;
}

static bool eth_exec_case(test_exec_t *exec, test_state_e state,
                          const eth_chn_table_t *table)
{
    bool ret = false;
    uint32_t respCanID = SINGLE_RESP;
    eth_resp_t *resp = (eth_resp_t *)&exec->resp;
    eth_cmd_t *eth_cmd = (eth_cmd_t *)&exec->cmd;

    set_para_value(resp->cmd_status, CMD_PARA_ERR);
    set_para_value(resp->channel_id, eth_cmd->channel_id);
    set_para_value(resp->test_mode, eth_cmd->test_mode);
    set_para_value(resp->data2, eth_cmd->master_slave);

    if (state == STATE_SINGLE) {
        ret = exec_single_port(exec);

        if (ret != false) {
            set_para_value(resp->cmd_status, NORMAL_DEAL);
        }
    }

    common_response(exec, respCanID);

    return true;
}

bool eth_100base_t1(test_exec_t *exec, test_state_e state)
{
    return eth_exec_case(exec, state, eth_100_base_t1);
}

bool eth_1000base_t1(test_exec_t *exec, test_state_e state)
{
    return eth_exec_case(exec, state, eth_1000_base_t1);
}

bool eth_100base_tx(test_exec_t *exec, test_state_e state)
{
    return eth_exec_case(exec, state, eth_100_base_tx);
}

bool eth_int_ctrl(test_exec_t *exec, test_state_e state)
{
    bool ret = false;
    bool found = false;
    bool opt_ret = false;
    CMD_STATUS cmdStatus = CMD_PARA_ERR;
    test_cmd_t *gpio_cmd = (test_cmd_t *)exec->cmd;
    const gpio_value_table_t *gpio = eth_int_table;

    for (; !END_OF_GPIO_TABLE(gpio); gpio++) {
        if (gpio_cmd->route_channel_id != gpio->pin_num
                || gpio->gpio_Pin == PIN_NUM_INVALID) {
            continue;
        }

        opt_ret = gpio_write(gpio->gpio_Pin, gpio_cmd->recv_data);
        cmdStatus = opt_ret ? NORMAL_DEAL : CMD_PARA_ERR;
        found = true;
        break;
    }

    if (!found || (found && !opt_ret)) {
        set_para_value(exec->resp[0], cmdStatus);
    }
    else {
        set_para_value(exec->resp[0], cmdStatus);
        set_para_value(exec->resp[1], gpio_cmd->route_channel_id);
        set_para_value(exec->resp[2], gpio_cmd->recv_data);
    }

    common_response(exec, SINGLE_RESP) ? (ret = true) : (ret = false);
    return ret;
}

static bool eth_5072_sleep_ops(test_exec_t *exec)
{
    bool ret = false;
    int  retval = -1;
    int  fd  = -1;

    fd = open(MV5072_DEV_PATH, O_RDWR);

    if (fd < 0) {
        ERROR("open fd device fail fd %d\n", fd);
        goto out;
    }

    set_para_value(exec->resp[0], CMD_PARA_ERR);
    set_para_value(exec->resp[1], exec->cmd[0]);

    retval = eth_enbale_auto_polarity(fd);

    if (retval < 0) {
        ERROR("eth_enbale_auto_polarity err\n");
        goto out;
    }

    retval = eth_enable_TC10_mode(fd);

    if (retval < 0) {
        ERROR("eth_enable_TC10 err\n");
        goto out;
    }

    retval = eth_enable_programming_andremote_wake_up(fd);

    if (retval < 0) {
        ERROR("eth_enable_programming err\n");
        goto out;
    }

    retval = eth_switch_deep_sleep_mode(fd);

    if (retval < 0) {
        ERROR("eth_switch_deep_sleep err\n");
        goto out;
    }

    retval = eth_ignore_p2_p11_sleep_handshake(fd);

    if (retval < 0) {
        ERROR("eth_ignore_sleep_handshak err\n");
        goto out;
    }

    retval = eth_power_removal_after_TC10_sleep_handshare(fd);

    if (retval < 0) {
        ERROR("eth_sleep_handsharel err\n");
        goto out;
    }

    if (get_dev_master_slave_mode() == XSLAVE_MODE) {
        retval = eth_into_sleep(fd);

        if (retval < 0) {
            ERROR("eth_into_sleep err\n");
            goto out;
        }

        printf("start_eth_into_sleep!!\n");
    }

    ret = true;
out:

    return ret;
}

static bool eth_5050_sleep_ops(test_exec_t *exec)
{
    bool ret = true;
    uint32_t val = 0;
    uint8_t lbu_idx[] = {0x00};

    for (uint32_t i = 0; i < ARRAY_SIZE(lbu_idx); i++) {
        val = 0;
        ret = lbu_ops(lbu_idx[i], LBU_WAKE_ADDR, &val, 0);
        val |= 0x1u << LBU_WAKE_BIT_IDX;
        ret = (ret && lbu_ops(lbu_idx[i], LBU_WAKE_ADDR, &val, 1));

        if (!ret) {
            ERROR("set lbu error i:%d\n", i);
            goto out;
        }
    }

    ret = true;
out:
    return ret;
}

static bool eth_2122_sleep_ops(test_exec_t *exec)
{
    bool ret = true;
    uint32_t val = 0;
    uint8_t phy_idx[] = {0x01, 0x02, 0x11, 0x12};

    for (uint32_t i = 0; i < ARRAY_SIZE(phy_idx); i++) {
        val = 0;
        ret = phy_ops(phy_idx[i], 1, 3, PHY_WAKE_ADDR, &val, 0);
        DBG("first read val:0x%0x\n", val);
        val |= 0x1u;
        ret = (ret && phy_ops(phy_idx[i], 1, 3, PHY_WAKE_ADDR, &val, 1));

        val = 0;
        phy_ops(phy_idx[i], 1, 3, PHY_WAKE_ADDR, &val, 0);
        DBG("read back val:0x%0x\n", val);

        if (!ret) {
            ERROR("set phy wake up error i:%d\n", i);
            goto out;
        }
    }

    ret = true;
out:
    return ret;
}

bool eth_enable_wakesrc(test_exec_t *exec, test_state_e state)
{
    bool ret = false;
    set_para_value(exec->resp[0], CMD_PARA_ERR);
    set_para_value(exec->resp[1], exec->cmd[0]);

    ret = eth_5050_sleep_ops(exec);

    if (ret != true) {
        ERROR("eth_5050_sleep_ops\n");
        goto fail;
    }

    ret = eth_2122_sleep_ops(exec);

    if (ret != true) {
        ERROR("eth_2122_sleep_ops\n");
        goto fail;
    }

    ret = eth_5072_sleep_ops(exec);

    if (ret != true) {
        ERROR("eth_5072_sleep_ops\n");
        goto fail;
    }

    if (ret)
        set_para_value(exec->resp[0], NORMAL_DEAL);

fail:

    common_response(exec, SINGLE_RESP) ? (ret = true) : (ret = false);
    return ret;
}

static int eth_enbale_auto_polarity(int fd)
{
    int ret = -1;

    if (fd < 0) {
        ERROR("open fd device fail fd %d\n", fd);
        goto fail;
    }

    struct mv_data mvbuf[] = {
        {
            .portaddr = 1,
            .regnum = MV_5072_INTERNAL_PHY_SLAVE_DATA,
            .is_write = 1,
            .data[0] = MV_INTERNAL_PHY,
            .data[1] = MV_INTERNAL_PHY_DEVICE3,
            .data[2] = 0x200
        },
        {
            .portaddr = 2,
            .regnum = MV_5072_INTERNAL_PHY_SLAVE_DATA,
            .is_write = 1,
            .data[0] = MV_INTERNAL_PHY,
            .data[1] = MV_INTERNAL_PHY_DEVICE3,
            .data[2] = 0x200
        },
        {
            .portaddr = 3,
            .regnum = MV_5072_INTERNAL_PHY_SLAVE_DATA,
            .is_write = 1,
            .data[0] = MV_INTERNAL_PHY,
            .data[1] = MV_INTERNAL_PHY_DEVICE3,
            .data[2] = 0x200
        },
        {
            .portaddr = 4,
            .regnum = MV_5072_INTERNAL_PHY_SLAVE_DATA,
            .is_write = 1,
            .data[0] = MV_INTERNAL_PHY,
            .data[1] = MV_INTERNAL_PHY_DEVICE3,
            .data[2] = 0x200
        },
        {
            .portaddr = 5,
            .regnum = MV_5072_INTERNAL_PHY_SLAVE_DATA,
            .is_write = 1,
            .data[0] = MV_INTERNAL_PHY,
            .data[1] = MV_INTERNAL_PHY_DEVICE3,
            .data[2] = 0x200
        },
        {
            .portaddr = 6,
            .regnum = MV_5072_INTERNAL_PHY_SLAVE_DATA,
            .is_write = 1,
            .data[0] = MV_INTERNAL_PHY,
            .data[1] = MV_INTERNAL_PHY_DEVICE3,
            .data[2] = 0x200
        },
        {
            .portaddr = 7,
            .regnum = MV_5072_INTERNAL_PHY_SLAVE_DATA,
            .is_write = 1,
            .data[0] = MV_INTERNAL_PHY,
            .data[1] = MV_INTERNAL_PHY_DEVICE3,
            .data[2] = 0x200
        }
    };

    for (uint8_t i = 0; i < ARRAY_SIZE(mvbuf); i++) {
        ret = ioctl(fd, MV_ACCESS_PHY, &mvbuf[i]);

        if (ret < 0) {
            ERROR("ioctl MV_ACCESS_PHY fail\n");
            goto fail;
        }
    }

fail:

    return ret;
}

/**
 * @brief switch lbu register setter/getter
 * @param addr: register address
 * @param val: pointer to val
 * @param ops: 0 - get val, 1 - set val
 * @return true, if successfully, otherwise false
 */
static bool lbu_ops(uint8_t switch_port_id, uint32_t addr, uint32_t *val,
                    uint32_t ops)
{
    int fd = -1;
    bool ret = false;
    uint8_t switch_id;
    struct mv_data mvbuf = {
        .is_write = ops,
        .data = {
            [0] = addr,
            [1] = *val,
        }
    };

    switch_id = (switch_port_id >> 4) & 0x0f;
    fd = get_switch_fd(switch_id);
    DBG("switch_id:%d\n", switch_id);

    if (fd < 0) {
        ERROR("open switch device fail\n");
        goto fail;
    }

    if (ioctl(fd, MV_ACCESS_LBU_CMD, &mvbuf) < 0) {
        ERROR("access reg:0x%0x val:0x%0x fail\n", addr, *val);
        goto fail;
    }

    if (ops == 0)
        *val = mvbuf.data[1];

    ret = true;
fail:

    if (fd >= 0)
        close(fd);

    return ret;
}

/**
 * @brief switch phy register setter/getter
 * @param extern_flag: 0 - internal phy, 1 - extern phy
 * @param dev_addr:
 * @param addr: register address
 * @param val: pointer to val
 * @param ops: 0 - get val, 1 - set val
 * @return true, if successfully, otherwise false
 */
static bool phy_ops(uint8_t switch_port_id, uint32_t extern_flag,
                    uint32_t dev_addr, uint32_t addr, uint32_t *val, uint32_t ops)
{
    int fd = -1;
    bool ret = false;
    uint8_t switch_id;
    uint8_t switch_port;
    struct mv_data mvbuf = {
        .regnum = addr,
        .is_write = ops,
        .data = {
            [0] = extern_flag,
            [1] = dev_addr,
            [2] = *val,
        }
    };

    switch_id = (switch_port_id >> 4) & 0x0f;
    switch_port = switch_port_id & 0x0f;

    DBG("switch id:%d port:%d\n", switch_id, switch_port);

    if (switch_port == 0) {
        ERROR("switch port error\n");
        goto fail;
    }

    mvbuf.portaddr = switch_port;
    fd = get_switch_fd(switch_id);

    if (fd < 0) {
        ERROR("open switch device fail\n");
        goto fail;
    }

    if (ioctl(fd, MV_ACCESS_PHY, &mvbuf) < 0) {
        ERROR("access reg:0x%0x val:0x%0x fail\n", addr, *val);
        goto fail;
    }

    if (ops == 0)
        *val = mvbuf.data[0];

    ret = true;
fail:

    if (fd >= 0)
        close(fd);

    return ret;
}

static int eth_enable_TC10_mode(int fd)
{
    int ret = -1;

    if (fd < 0) {
        ERROR("open fd device fail fd %d\n", fd);
        goto fail;
    }

    struct mv_data mvbuf[] = {
        {
            .portaddr = 1,
            .regnum = MV_INTERNAL_PHY_TC10_REG,
            .is_write = 1,
            .data[0] = MV_INTERNAL_PHY,
            .data[1] = MV_INTERNAL_PHY_DEVICE3,
            .data[2] = 0x1
        },
        {
            .portaddr = 2,
            .regnum = MV_INTERNAL_PHY_TC10_REG,
            .is_write = 1,
            .data[0] = MV_INTERNAL_PHY,
            .data[1] = MV_INTERNAL_PHY_DEVICE3,
            .data[2] = 0x1
        },
        {
            .portaddr = 3,
            .regnum = MV_INTERNAL_PHY_TC10_REG,
            .is_write = 1,
            .data[0] = MV_INTERNAL_PHY,
            .data[1] = MV_INTERNAL_PHY_DEVICE3,
            .data[2] = 0x1
        },
        {
            .portaddr = 4,
            .regnum = MV_INTERNAL_PHY_TC10_REG,
            .is_write = 1,
            .data[0] = MV_INTERNAL_PHY,
            .data[1] = MV_INTERNAL_PHY_DEVICE3,
            .data[2] = 0x1
        },
        {
            .portaddr = 5,
            .regnum = MV_INTERNAL_PHY_TC10_REG,
            .is_write = 1,
            .data[0] = MV_INTERNAL_PHY,
            .data[1] = MV_INTERNAL_PHY_DEVICE3,
            .data[2] = 0x1
        },
        {
            .portaddr = 6,
            .regnum = MV_INTERNAL_PHY_TC10_REG,
            .is_write = 1,
            .data[0] = MV_INTERNAL_PHY,
            .data[1] = MV_INTERNAL_PHY_DEVICE3,
            .data[2] = 0x1
        },
        {
            .portaddr = 7,
            .regnum = MV_INTERNAL_PHY_TC10_REG,
            .is_write = 1,
            .data[0] = MV_INTERNAL_PHY,
            .data[1] = MV_INTERNAL_PHY_DEVICE3,
            .data[2] = 0x1
        }
    };

    for (uint8_t i = 0; i < ARRAY_SIZE(mvbuf); i++) {
        ret = ioctl(fd, MV_ACCESS_PHY, &mvbuf[i]);

        if (ret < 0) {
            ERROR("ioctl MV_ACCESS_PHY fail\n");
            goto fail;
        }
    }

    ret = 1;
fail:

    return ret;
}

static int eth_enable_programming_andremote_wake_up(int fd)
{
    int ret = -1;

    if (fd < 0) {
        ERROR("open fd device fail fd %d\n", fd);
        goto fail;
    }

    struct mv_data mvbuf[] = {
        {
            .portaddr = 1,
            .regnum = 0x8701,
            .is_write = 1,
            .data[0] = MV_INTERNAL_PHY,
            .data[1] = MV_INTERNAL_PHY_DEVICE3,
            .data[2] = 0x8000
        },
        {
            .portaddr = 2,
            .regnum = 0x8701,
            .is_write = 1,
            .data[0] = MV_INTERNAL_PHY,
            .data[1] = MV_INTERNAL_PHY_DEVICE3,
            .data[2] = 0x8000
        },
        {
            .portaddr = 3,
            .regnum = 0x8701,
            .is_write = 1,
            .data[0] = MV_INTERNAL_PHY,
            .data[1] = MV_INTERNAL_PHY_DEVICE3,
            .data[2] = 0x8000
        },
        {
            .portaddr = 4,
            .regnum = 0x8701,
            .is_write = 1,
            .data[0] = MV_INTERNAL_PHY,
            .data[1] = MV_INTERNAL_PHY_DEVICE3,
            .data[2] = 0x8000
        },
        {
            .portaddr = 5,
            .regnum = 0x8701,
            .is_write = 1,
            .data[0] = MV_INTERNAL_PHY,
            .data[1] = MV_INTERNAL_PHY_DEVICE3,
            .data[2] = 0x8000
        },
        {
            .portaddr = 6,
            .regnum = 0x8701,
            .is_write = 1,
            .data[0] = MV_INTERNAL_PHY,
            .data[1] = MV_INTERNAL_PHY_DEVICE3,
            .data[2] = 0x8000
        },
        {
            .portaddr = 7,
            .regnum = 0x8701,
            .is_write = 1,
            .data[0] = MV_INTERNAL_PHY,
            .data[1] = MV_INTERNAL_PHY_DEVICE3,
            .data[2] = 0x8000
        }
    };

    for (uint8_t i = 0; i < ARRAY_SIZE(mvbuf); i++) {
        ret = ioctl(fd, MV_ACCESS_PHY, &mvbuf[i]);

        if (ret < 0) {
            ERROR("ioctl MV_ACCESS_PHY fail\n");
            goto fail;
        }
    }

    ret = 1;
fail:

    return ret;
}

static int eth_switch_deep_sleep_mode(int fd)
{
    int ret = -1;

    if (fd < 0) {
        ERROR("open fd device fail fd %d\n", fd);
        goto fail;
    }

    struct mv_data mvbuf_cmd = {
        .portaddr = 0,
        .regnum = 0x1A,
        .is_write = 1,
        .data[0] = 0xb208,
    };

    ret = ioctl(fd, MV_GET_CMD, &mvbuf_cmd);

    if (ret < 0) {
        ERROR("ioctl MV_GET_CMD fail\n");
        goto fail;
    }

    ret = 1;
fail:

    return ret;
}

static int eth_ignore_p2_p11_sleep_handshake(int fd)
{
    int ret = -1;

    if (fd < 0) {
        ERROR("open fd device fail fd %d\n", fd);
        goto fail;
    }

    struct mv_data mvbuf[] = {
        {
            .portaddr = 2,
            .regnum = MV_5072_INTERNAL_PHY_PMA_PMD_REG,
            .is_write = 1,
            .data[0] = MV_INTERNAL_PHY,
            .data[1] = MV_5072_INTERNAL_PHY_DEVICE1,
            .data[2] = 0x8000
        },
        {
            .portaddr = 3,
            .regnum = MV_5072_INTERNAL_PHY_PMA_PMD_REG,
            .is_write = 1,
            .data[0] = MV_INTERNAL_PHY,
            .data[1] = MV_5072_INTERNAL_PHY_DEVICE1,
            .data[2] = 0x8000
        },
        {
            .portaddr = 4,
            .regnum = MV_5072_INTERNAL_PHY_PMA_PMD_REG,
            .is_write = 1,
            .data[0] = MV_INTERNAL_PHY,
            .data[1] = MV_5072_INTERNAL_PHY_DEVICE1,
            .data[2] = 0x8000
        },
        {
            .portaddr = 5,
            .regnum = MV_5072_INTERNAL_PHY_PMA_PMD_REG,
            .is_write = 1,
            .data[0] = MV_INTERNAL_PHY,
            .data[1] = MV_5072_INTERNAL_PHY_DEVICE1,
            .data[2] = 0x8000
        },
        {
            .portaddr = 6,
            .regnum = MV_5072_INTERNAL_PHY_PMA_PMD_REG,
            .is_write = 1,
            .data[0] = MV_INTERNAL_PHY,
            .data[1] = MV_5072_INTERNAL_PHY_DEVICE1,
            .data[2] = 0x8000
        },
        {
            .portaddr = 7,
            .regnum = MV_5072_INTERNAL_PHY_PMA_PMD_REG,
            .is_write = 1,
            .data[0] = MV_INTERNAL_PHY,
            .data[1] = MV_5072_INTERNAL_PHY_DEVICE1,
            .data[2] = 0x8000
        }
    };

    for (uint8_t i = 0; i < ARRAY_SIZE(mvbuf); i++) {
        ret = ioctl(fd, MV_ACCESS_PHY, &mvbuf[i]);

        if (ret < 0) {
            ERROR("ioctl MV_ACCESS_PHY fail\n");
            goto fail;
        }
    }

    struct mv_data mvbuf_cmd[] = {
        {
            .portaddr = 2,
            .regnum = 0x1A,
            .is_write = 1,
            .data[0] = 0xb4c0,
        },
        {
            .portaddr = 3,
            .regnum = 0x1A,
            .is_write = 1,
            .data[0] = 0xb4c0,
        },
        {
            .portaddr = 4,
            .regnum = 0x1A,
            .is_write = 1,
            .data[0] = 0xb4c0,
        },
        {
            .portaddr = 5,
            .regnum = 0x1A,
            .is_write = 1,
            .data[0] = 0xb4c0,
        },
        {
            .portaddr = 6,
            .regnum = 0x1A,
            .is_write = 1,
            .data[0] = 0xb4c0,
        },
        {
            .portaddr = 7,
            .regnum = 0x1A,
            .is_write = 1,
            .data[0] = 0xb4c0,
        },
        {
            .portaddr = 8,
            .regnum = 0x1A,
            .is_write = 1,
            .data[0] = 0xb4c0,
        },
        {
            .portaddr = 9,
            .regnum = 0x1A,
            .is_write = 1,
            .data[0] = 0xb4c0,
        },
        {
            .portaddr = 10,
            .regnum = 0x1A,
            .is_write = 1,
            .data[0] = 0xb4c0,
        },
        {
            .portaddr = 11,
            .regnum = 0x1A,
            .is_write = 1,
            .data[0] = 0xb4c0,
        }
    };

    for (uint8_t i = 0; i < ARRAY_SIZE(mvbuf_cmd); i++) {
        ret = ioctl(fd, MV_GET_CMD, &mvbuf_cmd[i]);

        if (ret < 0) {
            ERROR("ioctl MV_ACCESS_PHY fail\n");
            goto fail;
        }
    }

    ret = 1;
fail:

    return ret;
}

static int eth_power_removal_after_TC10_sleep_handshare(int fd)
{
    int ret = -1;

    if (fd < 0) {
        ERROR("open fd device fail fd %d\n", fd);
        goto fail;
    }

    struct mv_data mvbuf_cmd = {
        .portaddr = 1,
        .regnum = 0x1A,
        .is_write = 1,
        .data[0] = 0xb481,
    };

    ret = ioctl(fd, MV_GET_CMD, &mvbuf_cmd);

    if (ret < 0) {
        ERROR("ioctl MV_GET_CMD fail\n");
        goto fail;
    }

    ret = 1;
fail:

    return ret;
}

static int eth_into_sleep(int fd)
{
    int ret = -1;

    if (fd < 0) {
        ERROR("open fd device fail fd %d\n", fd);
        goto fail;
    }

    struct mv_data mvbuf = {
        .portaddr = 1,
        .regnum = MV_INTERNAL_PHY_SLEEP_REG,
        .is_write = 1,
        .data[0] = MV_INTERNAL_PHY,
        .data[1] = MV_INTERNAL_PHY_DEVICE3,
        .data[2] = 0x1
    };

    ret = ioctl(fd, MV_ACCESS_PHY, &mvbuf);

    if (ret < 0) {
        ERROR("ioctl MV_GET_CMD fail\n");
        goto fail;
    }

    ret = 1;
fail:

    return ret;
}

static void eth_socket_port_init(MASTER_SLAVER_MODE mode)
{
    eth_vlan_table_config(MV5072_DEV_ID, SOCKET_CHANNEL_CREATE_NR);
}

void eth_set_port_mode_early(MASTER_SLAVER_MODE mode)
{
    eth_set_port_mode(mode);
    eth_set_SGMII_port_mode(mode);
    eth_socket_port_init(mode);
}
