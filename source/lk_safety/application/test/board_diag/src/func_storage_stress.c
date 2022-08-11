/*
 * func_usb.c
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: board_store_stress_deal.c
 *
 * Revision History:
 * -----------------
 */
#include "board_start.h"
#include "func_can.h"
#include "remote_test.h"
#include "board_cfg.h"

#define DDR_LEN  1024*1024
static uint8_t pt[DDR_LEN];
static uint8_t temp[DDR_LEN];

extern bool ddr_check_value_result(uint8_t *pt, uint8_t *temp, uint32_t len);
extern bool spi_nor_self_inspection(uint32_t addr, uint32_t len);

typedef bool (*store_ops)(board_test_exec_t *exec);
static bool store_ddr_stress_check(board_test_exec_t *exec);
static bool store_emmc_stress_check(board_test_exec_t *exec);
static bool store_flash_stress_check(board_test_exec_t *exec);
static bool store_all_test_check(board_test_exec_t *exec);

typedef struct {
    uint32_t start_addr;
    uint32_t end_addr;
    uint32_t offset;
} flash_partion_t;

typedef enum {
    EMMC_TEST  = 0x1,
    DDR_TEST   = 0x2,
    FLASH_TEST = 0x3,
    ALL_TEST   = 0x4
} store_type;

typedef struct {
    store_type type;
    store_ops  sops;
} store_t;

const static store_t store[] = {
    {EMMC_TEST,   store_emmc_stress_check },
    {DDR_TEST,    store_ddr_stress_check  },
    {FLASH_TEST,  store_flash_stress_check},
    {ALL_TEST,    store_all_test_check    }
};

const static flash_partion_t flash_partion = {
    0x000000, 0x800000, 0x1000
};//8M

static bool store_emmc_stress_check(board_test_exec_t *exec)
{
    bool ret = false;
    can_cmd_t *emmc_cmd = (can_cmd_t *)exec->cmd;
    store_type type = emmc_cmd->route_channel_id;

    if (type == EMMC_TEST) {
        set_para_value(emmc_cmd->route_channel_id, CHECK_OPS);
        set_para_value(emmc_cmd->recv_data, 0x0);
        set_para_value(emmc_cmd->standby_data1, 0x0);
        set_para_value(emmc_cmd->standby_data2, 0x0);

        remote_test_send_req(emmc_cmd);

        if ((ret = remote_test_wait_resp(EMMC_TIME_OUT_TICKS, exec)) == true) {
            set_para_value(exec->resp[0], NORMAL_DEAL);
        }
        else {
            set_para_value(exec->resp[0], CMD_PARA_ERR);
        }
    }

    return ret;
}

static bool store_ddr_stress_check(board_test_exec_t *exec)
{
    bool ret = false;
    can_cmd_t *ddr_cmd = (can_cmd_t *)exec->cmd;
    store_type type = ddr_cmd->route_channel_id;

    if (type == DDR_TEST) {
        ret = ddr_check_value_result(pt, temp, DDR_LEN);
    }

    return ret;
}

static bool store_flash_stress_check(board_test_exec_t *exec)
{
    bool ret = false;
    can_cmd_t *flash_cmd = (can_cmd_t *)exec->cmd;
    store_type type = flash_cmd->route_channel_id;
    uint32_t start_addr = flash_partion.start_addr;
    uint32_t end_addr = flash_partion.end_addr;
    uint32_t len = flash_partion.offset;

    if (type == FLASH_TEST) {

        do {
            ret = spi_nor_self_inspection(start_addr, len);
            dprintf(debug_show_null, "spi_nor test start_addr=%x\n", start_addr);

            if (ret != true)
                break;

            set_para_value(start_addr, start_addr + len);

        }
        while (start_addr < end_addr);
    }

    return ret;
}

static bool store_all_test_check(board_test_exec_t *exec)
{
    bool ret = false;
    can_cmd_t *emmc_cmd = (can_cmd_t *)exec->cmd;

    set_para_value(emmc_cmd->route_channel_id, EMMC_TEST);

    ret = store_emmc_stress_check(exec);

    if (ret != true) {
        dprintf(debug_show_dg, "store_emmc_stress fail\n");
        goto out;
    }

    set_para_value(emmc_cmd->route_channel_id, DDR_TEST);

    ret = store_ddr_stress_check(exec);

    if (ret != true) {
        dprintf(debug_show_dg, "store_ddr_stress fail\n");
        goto out;
    }

    set_para_value(emmc_cmd->route_channel_id, FLASH_TEST);

    ret = store_flash_stress_check(exec);

    if (ret != true) {
        dprintf(debug_show_dg, "store_flash_stress fail\n");
        goto out;
    }

out:

    return ret;
}

/*store_stress read by single*/
static bool store_stress_check(board_test_exec_t *exec)
{
    bool ret = false;
    uint8_t store_type;

    can_cmd_t *store_cmd = (can_cmd_t *)exec->cmd;

    if (store_cmd->dev_id == g_step_case_table[STORE_SERIAL_ID].cmd_id) {

        set_para_value(store_type, store_cmd->route_channel_id);

        for (uint8_t i = 0; i < ARRAY_SIZE(store); i++) {
            if (store_type == store[i].type) {
                ret = store[i].sops(exec);
            }
        }
    }

    return ret;
}

/*board_store_stress_deal function start*/
bool board_store_stress_deal(board_test_exec_t *exec, board_test_state_e state)
{
    bool ret = false;
    uint8_t cmdStatus = CMD_PARA_ERR;

    if (state == STATE_SINGLE) {
        ret = store_stress_check(exec);

        if (ret == true)
            set_para_value(cmdStatus, NORMAL_DEAL);

        set_para_value(exec->resp[0], cmdStatus);
        set_para_value(exec->board_response, can_common_response);
    }

    return ret;
}
