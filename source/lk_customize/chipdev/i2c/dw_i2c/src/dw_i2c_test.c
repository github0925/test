/*
 * dw_i2c_test.c
 *
 * Copyright (c) 2019 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: I2C driver for test source file.
 *
 * Revision History:
 * -----------------
 */
#include <platform/interrupts.h>
#include <platform/debug.h>
#include <dw_i2c_test.h>
#include <dw_i2c_reg.h>
#include <assert.h>
#include <err.h>
#include <reg.h>

#define LOCAL_TRACE 0

bool dw_i2c_dump_all_reg_test(i2c_reg_type_t *base)
{
    uint32_t reg_value = 0;

    LTRACEF_LEVEL(DEFAULT_I2C_LOG_LEVEL,
                  "#######################################\n");
    LTRACEF_LEVEL(DEFAULT_I2C_LOG_LEVEL, "dump_all_reg:\n");
    reg_value = base->ic_con;
    LTRACEF_LEVEL(DEFAULT_I2C_LOG_LEVEL, "i2c_control:0x%x\n", reg_value);
    reg_value = base->ic_con;
    LTRACEF_LEVEL(DEFAULT_I2C_LOG_LEVEL, "i2c_control:0x%x\n", reg_value);
    reg_value = base->ic_tar;
    LTRACEF_LEVEL(DEFAULT_I2C_LOG_LEVEL, "i2c_tar:0x%x\n", reg_value);
    reg_value = base->ic_sar;
    LTRACEF_LEVEL(DEFAULT_I2C_LOG_LEVEL, "i2c_sar:0x%x\n", reg_value);
    reg_value = base->ic_data_cmd;
    LTRACEF_LEVEL(DEFAULT_I2C_LOG_LEVEL, "i2c_data_cmd:0x%x\n", reg_value);
    reg_value = base->ic_ss_scl_hcnt;
    LTRACEF_LEVEL(DEFAULT_I2C_LOG_LEVEL, "i2c_ss_scl_hcnt:0x%x\n", reg_value);
    reg_value = base->ic_ss_scl_lcnt;
    LTRACEF_LEVEL(DEFAULT_I2C_LOG_LEVEL, "i2c_ss_scl_lcnt:0x%x\n", reg_value);
    reg_value = base->ic_fs_scl_hcnt;
    LTRACEF_LEVEL(DEFAULT_I2C_LOG_LEVEL, "i2c_fs_scl_hcnt:0x%x\n", reg_value);
    reg_value = base->ic_fs_scl_lcnt;
    LTRACEF_LEVEL(DEFAULT_I2C_LOG_LEVEL, "i2c_fs_scl_lcnt:0x%x\n", reg_value);
    reg_value = base->ic_intr_stat;
    LTRACEF_LEVEL(DEFAULT_I2C_LOG_LEVEL, "i2c_intr_stat:0x%x\n", reg_value);
    reg_value = base->ic_intr_mask;
    LTRACEF_LEVEL(DEFAULT_I2C_LOG_LEVEL, "i2c_intr_mask:0x%x\n", reg_value);
    reg_value = base->ic_raw_intr_stat;
    LTRACEF_LEVEL(DEFAULT_I2C_LOG_LEVEL, "i2c_raw_intr_stat:0x%x\n",
                  reg_value);
    reg_value = base->ic_rx_tl;
    LTRACEF_LEVEL(DEFAULT_I2C_LOG_LEVEL, "i2c_rx_tl:0x%x\n", reg_value);
    reg_value = base->ic_tx_tl;
    LTRACEF_LEVEL(DEFAULT_I2C_LOG_LEVEL, "i2c_tx_tl:0x%x\n", reg_value);
    reg_value = base->ic_clr_intr;
    LTRACEF_LEVEL(DEFAULT_I2C_LOG_LEVEL, "i2c_clr_intr:0x%x\n", reg_value);
    reg_value = base->ic_clr_rx_under;
    LTRACEF_LEVEL(DEFAULT_I2C_LOG_LEVEL, "i2c_clr_rx_under:0x%x\n", reg_value);
    reg_value = base->ic_clr_rx_over;
    LTRACEF_LEVEL(DEFAULT_I2C_LOG_LEVEL, "i2c_clr_rx_over:0x%x\n", reg_value);
    reg_value = base->ic_clr_tx_over;
    LTRACEF_LEVEL(DEFAULT_I2C_LOG_LEVEL, "i2c_clr_tx_over:0x%x\n", reg_value);
    reg_value = base->ic_clr_rd_req;
    LTRACEF_LEVEL(DEFAULT_I2C_LOG_LEVEL, "i2c_clr_rd_req:0x%x\n", reg_value);
    reg_value = base->ic_clr_tx_abrt;
    LTRACEF_LEVEL(DEFAULT_I2C_LOG_LEVEL, "i2c_clr_tx_abrt:0x%x\n", reg_value);
    reg_value = base->ic_clr_rx_done;
    LTRACEF_LEVEL(DEFAULT_I2C_LOG_LEVEL, "i2c_clr_rx_done:0x%x\n", reg_value);
    reg_value = base->ic_clr_activity;
    LTRACEF_LEVEL(DEFAULT_I2C_LOG_LEVEL, "i2c_clr_activity:0x%x\n", reg_value);
    reg_value = base->ic_clr_stop_det;
    LTRACEF_LEVEL(DEFAULT_I2C_LOG_LEVEL, "i2c_clr_stop_det:0x%x\n", reg_value);
    reg_value = base->ic_clr_start_det;
    LTRACEF_LEVEL(DEFAULT_I2C_LOG_LEVEL, "i2c_clr_start_det:0x%x\n",
                  reg_value);
    reg_value = base->ic_clr_gen_call;
    LTRACEF_LEVEL(DEFAULT_I2C_LOG_LEVEL, "i2c_clr_gen_call:0x%x\n", reg_value);
    reg_value = base->ic_enable;
    LTRACEF_LEVEL(DEFAULT_I2C_LOG_LEVEL, "i2c_enable:0x%x\n", reg_value);
    reg_value = base->ic_status;
    LTRACEF_LEVEL(DEFAULT_I2C_LOG_LEVEL, "i2c_status:0x%x\n", reg_value);
    reg_value = base->ic_txflr;
    LTRACEF_LEVEL(DEFAULT_I2C_LOG_LEVEL, "i2c_txflr:0x%x\n", reg_value);
    reg_value = base->ic_rxflr;
    LTRACEF_LEVEL(DEFAULT_I2C_LOG_LEVEL, "i2c_rxflr:0x%x\n", reg_value);
    reg_value = base->ic_sda_hold;
    LTRACEF_LEVEL(DEFAULT_I2C_LOG_LEVEL, "i2c_sda_hold:0x%x\n", reg_value);
    reg_value = base->ic_tx_abrt_source;
    LTRACEF_LEVEL(DEFAULT_I2C_LOG_LEVEL, "i2c_tx_abrt_source:0x%x\n",
                  reg_value);
    reg_value = base->ic_dma_cr;
    LTRACEF_LEVEL(DEFAULT_I2C_LOG_LEVEL, "i2c_dma_cr:0x%x\n", reg_value);
    reg_value = base->ic_dma_tdlr;
    LTRACEF_LEVEL(DEFAULT_I2C_LOG_LEVEL, "i2c_dma_tdlr:0x%x\n", reg_value);
    reg_value = base->ic_dma_rdlr;
    LTRACEF_LEVEL(DEFAULT_I2C_LOG_LEVEL, "i2c_dma_rdlr:0x%x\n", reg_value);
    reg_value = base->ic_sda_setup;
    LTRACEF_LEVEL(DEFAULT_I2C_LOG_LEVEL, "i2c_sda_setup:0x%x\n", reg_value);
    reg_value = base->ic_ack_general_call;
    LTRACEF_LEVEL(DEFAULT_I2C_LOG_LEVEL, "i2c_ack_general_call:0x%x\n",
                  reg_value);
    reg_value = base->ic_enable_status;
    LTRACEF_LEVEL(DEFAULT_I2C_LOG_LEVEL, "i2c_enable_status:0x%x\n",
                  reg_value);
    reg_value = base->ic_fs_spklen;
    LTRACEF_LEVEL(DEFAULT_I2C_LOG_LEVEL, "i2c_fs_spklen:0x%x\n", reg_value);
    reg_value = base->ic_clr_restart_det;
    LTRACEF_LEVEL(DEFAULT_I2C_LOG_LEVEL, "i2c_clr_restart_det:0x%x\n",
                  reg_value);
    reg_value = base->ic_scl_stuck_at_low_timeout;
    LTRACEF_LEVEL(DEFAULT_I2C_LOG_LEVEL, "i2c_scl_stuck_at_low_timeout:0x%x\n",
                  reg_value);
    reg_value = base->ic_sda_stuck_at_low_timeout;
    LTRACEF_LEVEL(DEFAULT_I2C_LOG_LEVEL, "i2c_sda_stuck_at_low_timeout:0x%x\n",
                  reg_value);
    reg_value = base->ic_clr_scl_stuck_det;
    LTRACEF_LEVEL(DEFAULT_I2C_LOG_LEVEL, "i2c_clr_scl_stuck_det:0x%x\n",
                  reg_value);
    reg_value = base->ic_smbus_clk_low_sext;
    LTRACEF_LEVEL(DEFAULT_I2C_LOG_LEVEL, "i2c_smbus_clk_low_sext:0x%x\n",
                  reg_value);
    reg_value = base->ic_smbus_clk_low_mext;
    LTRACEF_LEVEL(DEFAULT_I2C_LOG_LEVEL, "i2c_smbus_clk_low_mext:0x%x\n",
                  reg_value);
    reg_value = base->ic_smbus_thigh_max_idle_count;
    LTRACEF_LEVEL(DEFAULT_I2C_LOG_LEVEL,
                  "i2c_smbus_thigh_max_idle_count:0x%x\n", reg_value);
    reg_value = base->ic_smbus_intr_stat;
    LTRACEF_LEVEL(DEFAULT_I2C_LOG_LEVEL, "i2c_smbus_intr_stat:0x%x\n",
                  reg_value);
    reg_value = base->ic_smbus_intr_mask;
    LTRACEF_LEVEL(DEFAULT_I2C_LOG_LEVEL, "i2c_smbus_intr_mask:0x%x\n",
                  reg_value);
    reg_value = base->ic_smbus_raw_intr_stat;
    LTRACEF_LEVEL(DEFAULT_I2C_LOG_LEVEL, "i2c_smbus_raw_intr_stat:0x%x\n",
                  reg_value);
    reg_value = base->ic_clr_smbus_intr;
    LTRACEF_LEVEL(DEFAULT_I2C_LOG_LEVEL, "i2c_clr_smbus_intr:0x%x\n",
                  reg_value);
    reg_value = base->ic_smbus_udid_lsb;
    LTRACEF_LEVEL(DEFAULT_I2C_LOG_LEVEL, "i2c_smbus_udid_lsb:0x%x\n",
                  reg_value);
    reg_value = base->ic_comp_param_1;
    LTRACEF_LEVEL(DEFAULT_I2C_LOG_LEVEL, "i2c_comp_param_1:0x%x\n", reg_value);
    reg_value = base->ic_comp_version;
    LTRACEF_LEVEL(DEFAULT_I2C_LOG_LEVEL, "i2c_comp_version:0x%x\n", reg_value);
    reg_value = base->ic_comp_type;
    LTRACEF_LEVEL(DEFAULT_I2C_LOG_LEVEL, "i2c_comp_type:0x%x\n", reg_value);
    LTRACEF_LEVEL(DEFAULT_I2C_LOG_LEVEL,
                  "#######################################\n");
    return true;
}

bool dw_i2c_read_only_reg_test(vaddr_t base)
{
    uint32_t reg_read;

    reg_read = ((i2c_reg_type_t *)base)->ic_status;

    if (reg_read & 0x1) {
        LTRACEF("i2c is activity:0x%x\n", reg_read);
        return true;
    }
    else {
        LTRACEF("i2c is idle:0x%x\n", reg_read);
        dw_i2c_dump_all_reg_test((i2c_reg_type_t *)base);
        return true;
    }
}

bool dw_i2c_rw_reg_test(vaddr_t base)
{
    uint32_t reg_read, reg_read_new, reg_write;

    reg_read = ((i2c_reg_type_t *)base)->ic_enable;
    reg_write = reg_read;

    if (reg_write & 0x1) {
        reg_write &= ~0x1;
    }
    else {
        reg_write |= 0x1;
    }

    ((i2c_reg_type_t *)base)->ic_enable = reg_write;
    reg_read_new = ((i2c_reg_type_t *)base)->ic_enable;

    if (reg_read_new == reg_read) {
        LTRACEF("i2c reg write is ok:0x%x\n", reg_read);
        reg_write = reg_read;
        ((i2c_reg_type_t *)base)->ic_enable = reg_write;
        return true;
    }
    else {
        LTRACEF("i2c reg write is error:0x%x\n", reg_read);
        dw_i2c_dump_all_reg_test((i2c_reg_type_t *)base);
        return false;
    }
}

