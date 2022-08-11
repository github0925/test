/*
 * func_eth.h
 *
 * Copyright (c) 2020 SemiDrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */
#ifndef _FUNC_ETH_H
#define _FUNC_ETH_H
#include "sdrv_types.h"
#include "sw_timer.h"
#include <pthread.h>

#define MV5072_PORT_MIN   1
#define MV5072_PORT_MAX   10

#define MV5050_PORT_MIN   1
#define MV5050_PORT_MAX   8

#define MV_PORT_RESERVED  8

#define MV5050_DEV_ID     0
#define MV5072_DEV_ID     1

#define MV5072_DEV_PATH  "/dev/mv5072"
#define MV5050_DEV_PATH  "/dev/mv5050"
#define PING_DEFAULT_IP  "192.168.10.22"
#define MV_GET_CMD                         _IOWR('M', 8, struct mv_data)
#define MV_SET_TEST_MODE                   _IOW('M', 9, struct mv_data)
#define MV_SET_AUTO_NEGOTIATION            _IOW('M', 10, struct mv_data)
#define MV_GET_AUTO_NEGOTIATION_STATE      _IOWR('M', 11, struct mv_data)
#define MV_ACCESS_PHY                      _IOWR('M', 12, struct mv_data)
#define MV_ACCESS_LBU_CMD                  _IOWR('M', 13, struct mv_data)
#define MV_ACCESS_NO_ROUTER_CMD            _IOWR('M', 14, struct mv_data)

#define END_OF_TABLE(x) (x->chn_num == ETH_INVALID_CHN_NUM && x->port_num == ETH_INVALID_PORT_NUM)
#define END_OF_GPIO_TABLE(x) (x->gpio_Pin == PIN_NUM_INVALID && x->pin_num == PIN_NUM_INVALID)

#define LBU_WAKE_ADDR     (0x5000f1d0)
#define LBU_WAKE_BIT_IDX  (28)

#define PHY_WAKE_ADDR     (0x801c)
#define PHY_WAKE_BIT_IDX  (0)

#define MV_INTERNAL_PHY                     0
#define MV_EXTERNAL_PHY                     1

#define MV_5072_INTERNAL_PHY_DEVICE1        1
#define MV_5072_INTERNAL_PHY_PMA_PMD_REG    0x0834
#define MV_5072_INTERNAL_PHY_MASTER_DATA    0xC000
#define MV_5072_INTERNAL_PHY_SLAVE_DATA     0x8000

#define MV_5050_INTERNAL_PHY_DEVICE1        1
#define MV_5050_INTERNAL_PHY_PMA_PMD_REG    0x0834
#define MV_5050_INTERNAL_PHY_MASTER_DATA    0xC000
#define MV_5050_INTERNAL_PHY_SLAVE_DATA     0x8000

#define MV_INTERNAL_PHY_DEVICE3             3
#define MV_INTERNAL_PHY_DROP_COUNT_REG      0x8020
#define MV_INTERNAL_PHY_PACKET_GEN_REG      0x8610
#define MV_INTERNAL_PHY_PACKET_SIZE_REG     0x8611
#define MV_INTERNAL_PHY_CHECKER_CTL_REG     0x8612
#define MV_INTERNAL_PHY_PACKET_CTL_REG      0x8613
#define MV_INTERNAL_PHY_PACKET_COUNT_REG    0x8614
#define MV_INTERNAL_PHY_CRC_COUNT_REG       0x8615
#define MV_INTERNAL_PHY_TC10_REG            0x8707
#define MV_INTERNAL_PHY_SLEEP_REG           0x8702

#define MV_EXTERNAL_PHY_DEVICE1             1
#define MV_EXTERNAL_PHY_DEVICE3             3

#define MV_5072_SMI_POLICY_REG              0x0E
#define MV_5072_PORT_CONTROL_0_REG          0x04
#define MV_5072_PORT_CONTROL_1_REG          0x05
#define MV_5072_PORT_CONTROL_2_REG          0x08
#define MV_5072_PORT_CONTROL_3_REG          0x19

#define MV_PORT_VLAN_MAP_REG                0x06

#define MV_5072_PORT_DISABLE_VALUE          0x70
#define MV_5072_PORT_ENABLE_VALUE           0x7f

#define MV_5072_GLOBAL1_PORT                0x1b
#define MV_5072_GLOBAL2_PORT                0x1c

#define MV_5072_ATU_OP_REG                  0x0B
#define MV_5072_ATU_DATA_REG                0x0C

#define MV_2122_POWER_SPECTRAL_0904_ADDR    0x904
#define MV_2122_POWER_SPECTRAL_0904_VALUE   0xA000

#define MV_2122_POWER_SPECTRAL_FC10_ADDR    0xFC10
#define MV_2122_POWER_SPECTRAL_FC11_ADDR    0xFC11
#define MV_2122_POWER_SPECTRAL_FC12_ADDR    0xFC12
#define MV_2122_POWER_SPECTRAL_FC13_ADDR    0xFC13

#define MV_2122_POWER_SPECTRAL_FC10_VALUE   0x0000
#define MV_2122_POWER_SPECTRAL_FC11_VALUE   0x003D
#define MV_2122_POWER_SPECTRAL_FC12_VALUE   0x0003
#define MV_2122_POWER_SPECTRAL_FC13_VALUE   0x0017

typedef enum {
    NULL_NEGOTIATION = 0X0,
    INTO_NEGOTIATION = 0X1
} NEGOTIATION_MODE;

struct mv_data {
    uint32_t portaddr;
    uint32_t regnum;
    uint32_t is_write;
    uint32_t data[5];
};

typedef struct {
    uint8_t id;
    uint8_t port;
} switch_info_t;

typedef struct {
    uint32_t cur_val;
    uint32_t pre_val;
    uint32_t mv_5050_vcnt_val[MV5050_PORT_MAX + 1];
    uint32_t mv_5072_vcnt_val[MV5072_PORT_MAX + 1];
    uint32_t mv_5050_vcnt_resp[MV5050_PORT_MAX + 1];
    uint32_t mv_5072_vcnt_resp[MV5072_PORT_MAX + 1];
} eth_cnt_info_t;

typedef struct {
    uint8_t eth_channel;
    uint8_t eht_data;
    uint8_t phy_test_type_mode;
    uint8_t phy_test_master_slave;
} can_cmd_info_t;

typedef struct {
    eth_cnt_info_t eth_cnt_info;
    uint8_t switch_port_id;
    can_cmd_info_t can_info;
} eth_dev_info_t;

typedef struct {
    uint8_t dev_id;
    uint8_t channel_id;
    uint8_t test_mode;
    uint8_t valid_data;
    uint8_t master_slave;
    uint8_t data3;
    uint8_t data4;
    uint8_t data5;
} eth_cmd_t;

typedef struct {
    uint8_t cmd_status;
    uint8_t channel_id;
    uint8_t test_mode;
    uint8_t data1;
    uint8_t data2;
    uint8_t data3;
    uint8_t data4;
    uint8_t data5;
} eth_resp_t;

bool eth_100base_t1(test_exec_t *exec, test_state_e state);
bool eth_1000base_t1(test_exec_t *exec, test_state_e state);
bool eth_100base_tx(test_exec_t *exec, test_state_e state);
bool eth_int_ctrl(test_exec_t *exec, test_state_e state);
bool eth_enable_wakesrc(test_exec_t *exec, test_state_e state);
void eth_set_negotiation_early(void);
void eth_get_negotiation(void);
void eth_set_port_mode_early(MASTER_SLAVER_MODE mode);
void eth_set_port_no_route_mode(void);
void eth_set_port_disable_early(void);
void eth_read_register_early(void);
int eth_vlan_config(MASTER_SLAVER_MODE mode);
int eth_vlan_table_config(uint32_t switch_mode, uint8_t port);
#endif