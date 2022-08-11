/*
 * func_ospi.c
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: ii4 Test App.
 *
 * Revision History:
 * -----------------
 */
#include <stdlib.h>
#include <string.h>
#include <kernel/event.h>
#include <spi_nor_hal.h>
#include "board_start.h"
#include "board_cfg.h"
#include "func_can.h"

#define get_can_resp_cmdStatus(ret) (ret==true ? NORMAL_DEAL:CMD_PARA_ERR)

struct spi_nor_test_cfg {
    uint32_t id;
    struct spi_nor_cfg config;
};

static struct spi_nor_test_cfg spi_nor_cfg_data = {
    .id = RES_OSPI_REG_OSPI1,
    .config =
    {
        .cs = SPI_NOR_CS0,
        .bus_clk = SPI_NOR_CLK_25MHZ,
    },
};

static int spi_nor_test_tansfer(struct spi_nor_handle *dev, u64 addr, u32 data,
                                u64 len)
{
    u32 ret = 0;
    u32 erase_block_size;

    erase_block_size = dev->block_size;

    uint8_t *mem1 = memalign(CACHE_LINE, ROUNDUP(len, CACHE_LINE));
    uint8_t *mem2 = memalign(CACHE_LINE, ROUNDUP(len, CACHE_LINE));

    memset(mem1, 0, len);
    memset(mem2, 0, len);

    uint32_t i;
    volatile uint32_t *p1 = (volatile uint32_t *)mem1;

    for (i = 0; i < len / 4; i++, p1++) {
        if (data == UINT32_MAX) {
            *p1 = rand();
        }
        else {
            *p1 = data;
        }
    }

    /* test spi_nor read write */
    ret = hal_spi_nor_erase(dev, ROUNDDOWN(addr, erase_block_size),
                            ROUNDUP(len, erase_block_size));

    if (ret) {
        dprintf(debug_show_dg, "%s: erase error, ret = %d\n", __FUNCTION__, ret);
        goto tansfer_out;
    }

    ret = hal_spi_nor_write(dev, addr, mem1, len);

    if (ret) {
        dprintf(debug_show_dg, "%s: write error, ret = %d\n", __FUNCTION__, ret);
        goto tansfer_out;
    }

    ret = hal_spi_nor_read(dev, addr, mem2, len);

    if (ret) {
        dprintf(debug_show_dg, "%s: read error, ret = %d\n", __FUNCTION__, ret);
        goto tansfer_out;
    }

    ret = memcmp(mem1, mem2, len);

    if (ret) {
        dprintf(debug_show_dg, "%s: data compare error!\n", __FUNCTION__);
    }

tansfer_out:
    dprintf(debug_show_null, "\n");
    free(mem1);
    free(mem2);
    return ret;
}

bool spi_nor_self_inspection(uint32_t addr, uint32_t len)
{
    dprintf(debug_show_null, "spi_nor test start!\n");
    void *handle;
    int ret;
    spi_nor_length_t capacity;

    dprintf(debug_show_null, " spi_nor test creat handle ...\n");
    ret = hal_spi_nor_creat_handle(&handle, spi_nor_cfg_data.id);
    dprintf(debug_show_null, " spi_nor test creat handle result: %s\n",
            ret ? "PASS" : "FAILED");

    if (!ret)
        return false;

    ((struct spi_nor_handle *)handle)->config = &spi_nor_cfg_data.config;
    dprintf(debug_show_null, " spi_nor test init spi_nor device ...\n");
    ret = hal_spi_nor_init(handle);
    dprintf(debug_show_null, " spi_nor test init spi_nor device result: %s\n",
            ret ? "FAILED" : "PASS");

    if (ret) {
        ret = 1;
        goto release_handle;
    }

    dprintf(debug_show_null, " spi_nor get capacity ...\n");
    capacity = hal_spi_nor_get_capacity(handle);
    dprintf(debug_show_null, " spi_nor get capacity = %llu result: %s\n", capacity,
            capacity ? "PASS" : "FAILED");

    if (!capacity) {
        ret = 1;
        goto release_handle;
    }

    dprintf(debug_show_null, "\n spi_nor test transfer ...\n");
    ret += spi_nor_test_tansfer(handle, addr, UINT32_MAX, len);
    dprintf(debug_show_null, " spi_nor test transfer result: %s\n",
            ret ? "FAILED" : "PASS");

    if (ret) {
        ret = 1;
        goto release_handle;
    }

release_handle:
    dprintf(debug_show_null, " spi_nor test release handle ...\n");
    ret += hal_spi_nor_release_handle(&handle);
    dprintf(debug_show_null, " spi_nor test release handle result: %s\n",
            ret ? "PASS" : "FAILED");

    if (ret != 1)
        return false;

    dprintf(debug_show_null, "\nspi_nor test end!\n");
    return true;
}

static CMD_STATUS flash_check_value_result(void)
{
    bool ret = false;

    ret = spi_nor_self_inspection(0x1000, 256);

    return (CMD_STATUS)get_can_resp_cmdStatus(ret);
}

static bool _flash_single_read(board_test_exec_t *exec)
{
    bool ret = false;
    can_resp_t *flash_resp = (can_resp_t *)exec->resp;

    if (exec->cmd[0] == g_step_case_table[FLASH_SERIAL_ID].cmd_id) {
        flash_resp->cmd_status = flash_check_value_result();
        set_para_value(ret, true);
    }

    return ret;
}

static bool _flash_period_read(board_test_exec_t *exec)
{
    bool ret = false;
    set_para_value(exec->cmd[0], SUBCMD_FLASH);
    ret = _flash_single_read(exec);
    set_para_value(exec->peridic_resp_id, PERIODIC_RESP_FLASH);

    return ret;
}

bool board_flash_reply_deal(board_test_exec_t *exec, board_test_state_e state)
{
    bool ret = false;

    if (state == STATE_SINGLE) {
        ret = _flash_single_read(exec);
    }
    else if (state == STATE_PERIODIC) {
        ret = _flash_period_read(exec);
    }

    set_para_value(exec->board_response, can_common_response);

    return ret;
}