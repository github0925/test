/*
 * dw_i2c.c
 *
 * Copyright (c) 2019 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: I2C driver source file.
 *
 * Revision History:
 * -----------------
 */
#include <__regs_base.h>
#include <assert.h>
#include <dw_i2c.h>
#include <dw_i2c_reg.h>
#include <err.h>
#include <lk/init.h>
#include <stdlib.h>
#include <string.h>
#include <platform/debug.h>
#include <platform/interrupts.h>
#include <reg.h>
#if WITH_KERNEL_VM
#include <kernel/vm.h>
#endif
#include "target_res.h"

#define LOCAL_TRACE 0

#define TX_RX_TIMEOUT 3000 /* us, tx/rx retry times */
#define SPIN_STEP 1

#define WRITE_CMD 0
#define READ_CMD 1

#define SS_LCNT 0x80
#define SS_HCNT 0x60
#define SS_SDA_HOLD 0x40
#define FS_LCNT 0x20
#define FS_HCNT 0x20
#define FS_SDA_HOLD 0x10
#define HS_LCNT 0x2
#define HS_HCNT 0x1
#define HS_SDA_HOLD 0x1
#define DEF_TAR 0x55
#define I2C_LOG 3

static bool does_need_config(dw_i2c_context *p_i2c_con, uint32_t addr)
{
    if (p_i2c_con->cur_addr != addr) {
        dprintf(I2C_LOG, "does_need_config(%d, 0x%x): 0x%x, %d.\n", p_i2c_con->bus,
                addr, p_i2c_con->cur_addr, p_i2c_con->is_configured);
        return true;
    }

    if (!p_i2c_con->is_configured) {
        dprintf(I2C_LOG, "does_need_config(%d, 0x%x): 0x%x, %d.\n", p_i2c_con->bus,
                addr, p_i2c_con->cur_addr, p_i2c_con->is_configured);
        return true;
    }

    return false;
}

static status_t dw_i2c_wait_bus_not_busy(dw_i2c_context *p_i2c_con)
{
    int timeout = 20;
    dw_i2c_config_t *info = &p_i2c_con->info;
    i2c_reg_type_t *i2c_reg = (i2c_reg_type_t *)info->io_base;

    while (i2c_reg->ic_status & (1 << IC_STATUS_ACTIVITY_FIELD_OFFSET)) {
        if (timeout <= 0) {
            dprintf(0, "timeout waiting for bus ready\n");
            return ERR_TIMED_OUT;
        }

        timeout--;
        thread_sleep(1);
    }

    return 0;
}

static void dw_i2c_enable(i2c_reg_type_t *i2c_reg, bool enable)
{
    if (enable)
        i2c_reg->ic_enable = 0x1;
    else
        i2c_reg->ic_enable = 0x0;
}

static u32 dw_i2c_read_clear_intrbits(i2c_reg_type_t *i2c_reg)
{
    u32 stat, val = 0;
    stat = i2c_reg->ic_intr_stat;

    if (stat & DW_IC_INTR_RX_UNDER)
        val += i2c_reg->ic_clr_rx_under;

    if (stat & DW_IC_INTR_RX_OVER)
        val += i2c_reg->ic_clr_rx_over;

    if (stat & DW_IC_INTR_TX_OVER)
        val += i2c_reg->ic_clr_tx_over;

    if (stat & DW_IC_INTR_RD_REQ)
        val += i2c_reg->ic_clr_rd_req;

    if (stat & DW_IC_INTR_TX_ABRT)
        val += i2c_reg->ic_clr_tx_abrt;

    if (stat & DW_IC_INTR_RX_DONE)
        val += i2c_reg->ic_clr_rx_done;

    if (stat & DW_IC_INTR_ACTIVITY)
        val += i2c_reg->ic_clr_activity;

    if (stat & DW_IC_INTR_STOP_DET)
        val += i2c_reg->ic_clr_stop_det;

    if (stat & DW_IC_INTR_START_DET)
        val += i2c_reg->ic_clr_start_det;

    if (stat & DW_IC_INTR_GEN_CALL)
        val += i2c_reg->ic_clr_gen_call;

    return stat;
}

static void dw_i2c_configure_fifo(dw_i2c_context *p_i2c_con)
{
    dw_i2c_config_t *info = &p_i2c_con->info;
    addr_t io_base = info->io_base;
    i2c_reg_type_t *i2c_reg = (i2c_reg_type_t *)io_base;
    i2c_reg->ic_tx_tl = p_i2c_con->tx_fifo_depth / 2;
    i2c_reg->ic_rx_tl = 0;
}

static bool dw_i2c_config(dw_i2c_context *p_i2c_con, uint32_t addr)
{
    dw_i2c_config_t *info = &p_i2c_con->info;
    addr_t io_base = info->io_base;
    i2c_reg_type_t *i2c_reg = (i2c_reg_type_t *)io_base;

    if (!p_i2c_con->is_added)
        return false;

    dprintf(I2C_LOG, "dw_i2c_config(%d, 0x%x) io_base=0x%lx\n", p_i2c_con->bus,
            addr, io_base);
    /* configure */
    i2c_reg->ic_enable &= ~(0x1 << IC_ENABLE_ENABLE_FIELD_OFFSET);
    /* idle time */
    uint32_t val;
    val =
        0x20
        << IC_SMBUS_THIGH_MAX_IDLE_COUNT_SMBUS_THIGH_MAX_BUS_IDLE_CNT_FIELD_OFFSET;
    i2c_reg->ic_smbus_thigh_max_idle_count = val;
    /* speed, addr mode, master mode, */
    /* speed */
    val = info->speed << IC_CON_SPEED_FIELD_OFFSET;

    /* addr bits */
    if (info->mode == SLAVE_MODE) {
        val |= info->addr_bits << IC_CON_IC_10BITADDR_SLAVE_FIELD_OFFSET;
    }
    else {
        val |= info->addr_bits << IC_CON_IC_10BITADDR_MASTER_FIELD_OFFSET;
    }

    /* master/slave mode */
    val |= info->mode << IC_CON_IC_SLAVE_DISABLE_FIELD_OFFSET;
    val |= info->mode << IC_CON_MASTER_MODE_FIELD_OFFSET;
    /* restart en */
    val |= info->restart << IC_CON_IC_RESTART_EN_FIELD_OFFSET;
    i2c_reg->ic_con = val;

    if (info->mode == MASTER_MODE) {
        switch (info->speed) {
            case I2C_SPEED_FAST:
                /* scl */
                /* lcnt */
                val = FS_LCNT << IC_FS_SCL_LCNT_IC_FS_SCL_LCNT_FIELD_OFFSET;
                i2c_reg->ic_fs_scl_lcnt = val;
                /* hcnt */
                val = FS_HCNT << IC_FS_SCL_HCNT_IC_FS_SCL_HCNT_FIELD_OFFSET;
                i2c_reg->ic_fs_scl_hcnt = val;
                /* sda hold */
                val = FS_SDA_HOLD << IC_SDA_HOLD_IC_SDA_TX_HOLD_FIELD_OFFSET;
                val |= FS_SDA_HOLD << IC_SDA_HOLD_IC_SDA_RX_HOLD_FIELD_OFFSET;
                i2c_reg->ic_sda_hold = val;
                break;

            case I2C_SPEED_HIGH:
                /* scl */
                /* lcnt */
                val = HS_LCNT << IC_HS_SCL_LCNT_IC_HS_SCL_LCNT_FIELD_OFFSET;
                i2c_reg->ic_hs_scl_lcnt = val;
                /* hcnt */
                val = HS_HCNT << IC_HS_SCL_HCNT_IC_HS_SCL_HCNT_FIELD_OFFSET;
                i2c_reg->ic_hs_scl_hcnt = val;
                /* sda hold */
                val = HS_SDA_HOLD << IC_SDA_HOLD_IC_SDA_TX_HOLD_FIELD_OFFSET;
                val |= HS_SDA_HOLD << IC_SDA_HOLD_IC_SDA_RX_HOLD_FIELD_OFFSET;
                i2c_reg->ic_sda_hold = val;
                break;

            case I2C_SPEED_STANDARD:
            default:
                /* scl */
                /* lcnt */
                val = SS_LCNT << IC_SS_SCL_LCNT_IC_SS_SCL_LCNT_FIELD_OFFSET;
                i2c_reg->ic_ss_scl_lcnt = val;
                /* hcnt */
                val = SS_HCNT << IC_SS_SCL_HCNT_IC_SS_SCL_HCNT_FIELD_OFFSET;
                i2c_reg->ic_ss_scl_hcnt = val;
                /* sda hold */
                val = SS_SDA_HOLD << IC_SDA_HOLD_IC_SDA_TX_HOLD_FIELD_OFFSET;
                val |= SS_SDA_HOLD << IC_SDA_HOLD_IC_SDA_RX_HOLD_FIELD_OFFSET;
                i2c_reg->ic_sda_hold = val;
                break;
        }
    }

    /* target addr */
    if (info->mode == SLAVE_MODE) {
        val = addr << IC_SAR_IC_SAR_FIELD_OFFSET;
        i2c_reg->ic_sar = val;
    }
    else {
        val = 0 << IC_TAR_SPECIAL_FIELD_OFFSET;
        val |= addr << IC_TAR_IC_TAR_FIELD_OFFSET;
        i2c_reg->ic_tar = val;
    }

    p_i2c_con->tx_fifo_depth = ((i2c_reg->ic_comp_param_1 >> 16) & 0xff) + 1;
    p_i2c_con->rx_fifo_depth = ((i2c_reg->ic_comp_param_1 >> 8) & 0xff) + 1;
    dprintf(I2C_LOG, "i2c_reg->ic_comp_param_1=0x%x, %d, %d\n",
            i2c_reg->ic_comp_param_1, p_i2c_con->tx_fifo_depth,
            p_i2c_con->rx_fifo_depth);
    dw_i2c_configure_fifo(p_i2c_con);

    /* enable */
    if (p_i2c_con->info.poll == 1)
        i2c_reg->ic_enable |= 0x1 << IC_ENABLE_ENABLE_FIELD_OFFSET;

    p_i2c_con->cur_addr = addr;
    p_i2c_con->is_configured = true;
    return true;
}

static bool dw_i2c_switch_target(dw_i2c_context *p_i2c_con, uint32_t addr)
{
    dw_i2c_config_t *info = &p_i2c_con->info;
    addr_t io_base = info->io_base;
    i2c_reg_type_t *i2c_reg = (i2c_reg_type_t *)io_base;
    uint32_t val;

    if (!p_i2c_con->is_added)
        return false;

    dprintf(I2C_LOG, "dw_i2c_switch_target(%d, 0x%x) io_base=0x%lx\n",
            p_i2c_con->bus, addr, io_base);
    i2c_reg->ic_enable &= ~(0x1 << IC_ENABLE_ENABLE_FIELD_OFFSET);
    val = 0 << IC_TAR_SPECIAL_FIELD_OFFSET;
    val |= (addr << IC_TAR_IC_TAR_FIELD_OFFSET);
    i2c_reg->ic_tar = val;

    if (p_i2c_con->info.poll == 1)
        i2c_reg->ic_enable |= 0x1 << IC_ENABLE_ENABLE_FIELD_OFFSET;

    p_i2c_con->cur_addr = addr;
    return true;
}

static bool is_rxfifo_empty(dw_i2c_context *p_i2c_con)
{
    dw_i2c_config_t *info = &p_i2c_con->info;
    i2c_reg_type_t *i2c_reg = (i2c_reg_type_t *)info->io_base;
    uint32_t val = i2c_reg->ic_status;
    return (val & (1 << IC_STATUS_RFNE_FIELD_OFFSET)) ? false : true;
}

static bool is_txfifo_full(dw_i2c_context *p_i2c_con)
{
    dw_i2c_config_t *info = &p_i2c_con->info;
    i2c_reg_type_t *i2c_reg = (i2c_reg_type_t *)info->io_base;
    uint32_t val = i2c_reg->ic_status;
    return (val & (1 << IC_STATUS_TFNF_FIELD_OFFSET)) ? false : true;
}

void dw_i2c_add_bus(dw_i2c_context *p_i2c_con, const dw_i2c_config_t *info)
{
    dprintf(I2C_LOG, "add:%d.\n", p_i2c_con->bus);

    if (p_i2c_con->is_added) {
        printf("can't reconfig a enabled bus\n");
        return;
    }

    p_i2c_con->info = *info;
    p_i2c_con->is_added = true;
    p_i2c_con->is_configured = false;
}

static void i2c_dw_read(dw_i2c_context *p_i2c_con)
{
    dw_i2c_config_t *info = &p_i2c_con->info;
    i2c_reg_type_t *i2c_reg = (i2c_reg_type_t *)info->io_base;
    int rx_valid;
    dprintf(I2C_LOG, "%s(): i2c_reg->ic_txflr=%d, i2c_reg->ic_rxflr=%d\n",
            __func__, i2c_reg->ic_txflr, i2c_reg->ic_rxflr);

    for (; p_i2c_con->msg_read_idx < p_i2c_con->msgs_num;
            p_i2c_con->msg_read_idx++) {
        u32 len;
        u8 *buf;

        if (!(p_i2c_con->msgs[p_i2c_con->msg_read_idx].flags & I2C_M_RD))
            continue;

        if (!(p_i2c_con->status & STATUS_READ_IN_PROGRESS)) {
            len = p_i2c_con->msgs[p_i2c_con->msg_read_idx].len;
            buf = p_i2c_con->msgs[p_i2c_con->msg_read_idx].buf;
        }
        else {
            len = p_i2c_con->rx_buf_len;
            buf = p_i2c_con->rx_buf;
        }

        rx_valid = i2c_reg->ic_rxflr;

        for (; len > 0 && rx_valid > 0; len--, rx_valid--) {
            *buf = i2c_reg->ic_data_cmd;
            buf++;
            p_i2c_con->rx_outstanding--;
        }

        if (len > 0) {
            p_i2c_con->status |= STATUS_READ_IN_PROGRESS;
            p_i2c_con->rx_buf_len = len;
            p_i2c_con->rx_buf = buf;
            return;
        }
        else
            p_i2c_con->status &= ~STATUS_READ_IN_PROGRESS;
    }
}
static void i2c_dw_xfer_msg(dw_i2c_context *p_i2c_con)
{
    dw_i2c_config_t *info = &p_i2c_con->info;
    i2c_reg_type_t *i2c_reg = (i2c_reg_type_t *)info->io_base;
    u8 *buf = p_i2c_con->tx_buf;
    u32 buf_len = p_i2c_con->tx_buf_len;
    bool need_restart = false;
    int tx_limit, rx_limit;
    u32 intr_mask = DW_IC_INTR_MASTER_MASK;

    for (; p_i2c_con->msg_write_idx < p_i2c_con->msgs_num;
            p_i2c_con->msg_write_idx++) {
        if (!(p_i2c_con->status & STATUS_WRITE_IN_PROGRESS)) {
            buf = p_i2c_con->msgs[p_i2c_con->msg_write_idx].buf;
            buf_len = p_i2c_con->msgs[p_i2c_con->msg_write_idx].len;

            if (p_i2c_con->msg_write_idx > 0)
                need_restart = true;
        }

        tx_limit = p_i2c_con->tx_fifo_depth - i2c_reg->ic_txflr;
        rx_limit = p_i2c_con->rx_fifo_depth - i2c_reg->ic_rxflr;

        while ((buf_len > 0) && (tx_limit > 0) && (rx_limit > 0)) {
            u32 cmd = 0;

            if ((p_i2c_con->msg_write_idx == p_i2c_con->msgs_num - 1)
                    && (buf_len == 1))    //
                cmd |= 1 << IC_DATA_CMD_STOP_FIELD_OFFSET;

            if (need_restart) {
                cmd |= 1 << IC_DATA_CMD_RESTART_FIELD_OFFSET;       //restart
                need_restart = false;
            }

            if (p_i2c_con->msgs[p_i2c_con->msg_write_idx].flags & I2C_M_RD) {
                /* Avoid rx buffer overrun */
                if (p_i2c_con->rx_outstanding >= (int)p_i2c_con->rx_fifo_depth)
                    break;

                i2c_reg->ic_data_cmd = cmd | 0x100; //0x100: [8]    //0:write   1:read
                rx_limit--;
                p_i2c_con->rx_outstanding++;
            }
            else {
                i2c_reg->ic_data_cmd = cmd | *buf++;
            }

            tx_limit--;
            buf_len--;
        }

        p_i2c_con->tx_buf = buf;
        p_i2c_con->tx_buf_len = buf_len;

        if (buf_len > 0) {
            p_i2c_con->status |= STATUS_WRITE_IN_PROGRESS;
            break;
        }
        else
            p_i2c_con->status &= ~STATUS_WRITE_IN_PROGRESS;
    }

    /*
     * If i2c_msg index search is completed, we don't need TX_EMPTY
     * interrupt any more.
     */
    if (p_i2c_con->msg_write_idx == p_i2c_con->msgs_num)
        intr_mask &= ~DW_IC_INTR_TX_EMPTY;

    i2c_reg->ic_intr_mask = intr_mask;
}
static enum handler_return i2c_irq_handler(void *data)
{
    dw_i2c_context *p_i2c_con = (dw_i2c_context *)data;
    enum handler_return ret = INT_NO_RESCHEDULE;
    dw_i2c_config_t *info = &p_i2c_con->info;
    i2c_reg_type_t *i2c_reg = (i2c_reg_type_t *)info->io_base;
    u32 stat;
    mask_interrupt(info->irq);

    if (!i2c_reg->ic_enable || !i2c_reg->ic_raw_intr_stat) {
        unmask_interrupt(info->irq);
        return ret;
    }

    stat = dw_i2c_read_clear_intrbits(i2c_reg);
    dprintf(I2C_LOG, "irq: stat=0x%x\n", stat);

    if (stat & (1 << IC_INTR_STAT_R_TX_ABRT_FIELD_OFFSET)) {
        p_i2c_con->status = STATUS_IDLE;
        p_i2c_con->abort = DW_IC_ERR_TX_ABRT;
        /*
         * Anytime TX_ABRT is set, the contents of the tx/rx
         * buffers are flushed. Make sure to skip them.
         */
        i2c_reg->ic_intr_mask = 0x0;
        goto tx_aborted;
    }

    if (stat & (1 << IC_INTR_STAT_R_RX_FULL_FIELD_OFFSET))
        i2c_dw_read(p_i2c_con);

    if (stat & (1 << IC_INTR_STAT_R_TX_EMPTY_FIELD_OFFSET))
        i2c_dw_xfer_msg(p_i2c_con);

tx_aborted:

    if ((stat & ((1 << IC_INTR_STAT_R_TX_ABRT_FIELD_OFFSET) |
                 (1 << IC_INTR_STAT_R_STOP_DET_FIELD_OFFSET)))) {
        event_signal(&p_i2c_con->completion, false);
    }

    unmask_interrupt(info->irq);
    return ret;
}
bool dw_i2c_set_busconfig(dw_i2c_context *p_i2c_con,
                          const dw_i2c_config_t *info)
{
    dprintf(I2C_LOG, "dw_i2c_set_busconfig() .\n");

    if ((p_i2c_con->info.io_base == info->io_base) &&
            (p_i2c_con->info.speed == info->speed) &&
            (p_i2c_con->info.addr_bits == info->addr_bits) &&
            (p_i2c_con->info.mode == info->mode) &&
            (p_i2c_con->info.restart == info->restart) &&
            (p_i2c_con->info.poll == info->poll) &&
            (p_i2c_con->info.slave_addr == info->slave_addr)) {
        dprintf(0, "already configed bus %d.\n", p_i2c_con->bus);
        return false;
    }

    p_i2c_con->info = *info;
    p_i2c_con->is_added = true;
    p_i2c_con->is_configured = false;
    p_i2c_con->timeout = 1000;
    p_i2c_con->isr = false;
    dprintf(I2C_LOG, "%s:  bus %d, info->irq=%d, poll=%d.\n", __func__,
            p_i2c_con->bus, info->irq, p_i2c_con->info.poll);

    if (info->mode == SLAVE_MODE) {
        if (!dw_i2c_config(p_i2c_con, info->slave_addr))
            return false;
    }
    else if (info->mode == MASTER_MODE) {
        if (!dw_i2c_config(p_i2c_con, DEF_TAR))
            return false;
    }

    return true;
}
void dw_i2c_init_after(dw_i2c_context *p_i2c_con)
{
    if (!event_initialized(&p_i2c_con->completion))
        event_init(&p_i2c_con->completion, false, EVENT_FLAG_AUTOUNSIGNAL);
}
status_t dw_i2c_transfer(dw_i2c_context *p_i2c_con)
{
    dw_i2c_config_t *info = &p_i2c_con->info;
    i2c_reg_type_t *i2c_reg = (i2c_reg_type_t *)info->io_base;
    status_t ret;
    u32 val = 0;
    dprintf(I2C_LOG, "%s(): p_i2c_con->msgs_num=%d.\n", __func__,
            p_i2c_con->msgs_num);

    if ((p_i2c_con->info.poll == 0) && (p_i2c_con->isr == false)) {
        register_int_handler(info->irq, i2c_irq_handler, p_i2c_con);
        unmask_interrupt(info->irq);
        p_i2c_con->isr = true;
    }

    p_i2c_con->msg_write_idx = 0;
    p_i2c_con->msg_read_idx = 0;
    p_i2c_con->status = STATUS_IDLE;
    p_i2c_con->abort = 0;
    p_i2c_con->rx_outstanding = 0;

    if (does_need_config(p_i2c_con, p_i2c_con->msgs[0].addr) &&
            !dw_i2c_switch_target(p_i2c_con, p_i2c_con->msgs[0].addr))
        return ERR_NOT_CONFIGURED;

    dw_i2c_enable(i2c_reg, false);
    i2c_reg->ic_intr_mask = 0;
    dw_i2c_enable(i2c_reg, true);
    dprintf(I2C_LOG,
            "%s(): i2c_reg->ic_clr_intr=0x%x, i2c_reg->ic_intr_mask=0x%x\n", __func__,
            i2c_reg->ic_clr_intr, i2c_reg->ic_intr_mask);
    val +=  i2c_reg->ic_clr_intr;
    i2c_reg->ic_intr_mask = DW_IC_INTR_MASTER_MASK;
    ret = event_wait_timeout(&p_i2c_con->completion, p_i2c_con->timeout);

    if (ret) {
        dw_i2c_enable(i2c_reg, false);
        return ERR_TIMED_OUT;
    }

    dw_i2c_enable(i2c_reg, false);

    if (!p_i2c_con->abort && !p_i2c_con->status) {
        return NO_ERROR;
    }

    return ERR_IO;
}

status_t dw_i2c_transmit(dw_i2c_context *p_i2c_con, uint8_t address,
                         const void *buf, size_t cnt, bool start, bool stop)
{
    if (does_need_config(p_i2c_con, address) &&
            !dw_i2c_switch_target(p_i2c_con, address))
        return ERR_NOT_CONFIGURED;

    dw_i2c_config_t *info = &p_i2c_con->info;
    i2c_reg_type_t *i2c_reg = (i2c_reg_type_t *)info->io_base;
    char *p = (char *)buf;
    uint32_t val;

    for (size_t i = 0; i < cnt; i++) {
        int count = TX_RX_TIMEOUT / SPIN_STEP;
        val = p[i] | (WRITE_CMD << IC_DATA_CMD_CMD_FIELD_OFFSET);

        /* send a start at first one */
        if ((i == 0) && start) {
            val |= 1 << IC_DATA_CMD_RESTART_FIELD_OFFSET;
        }
        else if ((i == cnt - 1) && stop) {   /* send a stop at last one */
            val |= 1 << IC_DATA_CMD_STOP_FIELD_OFFSET;
        }

        while (is_txfifo_full(p_i2c_con) && --count)
            spin(SPIN_STEP);

        if (count == 0) { /* time out */
            /* should be reconfig in next operation */
            p_i2c_con->cur_addr = 0;
            return ERR_TIMED_OUT;
        }

        i2c_reg->ic_data_cmd = val;
    }

    return NO_ERROR;
}

status_t dw_i2c_receive(dw_i2c_context *p_i2c_con, uint8_t address,
                        void *buf,
                        size_t cnt, bool start, bool stop)
{
    if (does_need_config(p_i2c_con, address) &&
            !dw_i2c_switch_target(p_i2c_con, address))
        return ERR_NOT_CONFIGURED;

    dw_i2c_config_t *info = &p_i2c_con->info;
    i2c_reg_type_t *i2c_reg = (i2c_reg_type_t *)info->io_base;
    char *p = (char *)buf;
    uint32_t val;

    for (size_t i = 0; i < cnt; i++) {
        int count = TX_RX_TIMEOUT / SPIN_STEP;
        val = READ_CMD << IC_DATA_CMD_CMD_FIELD_OFFSET;

        /* send a start at first one */
        if ((i == 0) && start) {
            val |= 1 << IC_DATA_CMD_RESTART_FIELD_OFFSET;
        }
        else if ((i == cnt - 1) && stop) {   /* send a stop at last one */
            val |= 1 << IC_DATA_CMD_STOP_FIELD_OFFSET;
        }

        i2c_reg->ic_data_cmd = val;

        /* need check if data ready */
        while (is_rxfifo_empty(p_i2c_con) && --count)
            spin(SPIN_STEP);

        if (count == 0) { /* time out */
            /* should be reconfig in next operation */
            p_i2c_con->cur_addr = 0;
            return ERR_TIMED_OUT;
        }

        val = i2c_reg->ic_data_cmd;
        p[i] = val & 0xff;
    }

    return NO_ERROR;
}

status_t dw_i2c_slave_receive(dw_i2c_context *p_i2c_con, void *buf,
                              size_t cnt,
                              bool start, bool stop)
{
    uint8_t address;
    dw_i2c_config_t *info = &p_i2c_con->info;
    address = info->slave_addr;

    if (does_need_config(p_i2c_con, address) &&
            !dw_i2c_config(p_i2c_con, address))
        return ERR_NOT_CONFIGURED;

    /* dw_i2c_config_t *info = &g_i2c[bus].info; */
    i2c_reg_type_t *i2c_reg = (i2c_reg_type_t *)info->io_base;
    char *p = (char *)buf;
    uint32_t val;

    for (size_t i = 0; i < cnt; i++) {
        int count = TX_RX_TIMEOUT / SPIN_STEP;
        val = READ_CMD << IC_DATA_CMD_CMD_FIELD_OFFSET;

        /* send a start at first one */
        if ((i == 0) && start) {
            val |= 1 << IC_DATA_CMD_RESTART_FIELD_OFFSET;
        }
        else if ((i == cnt - 1) && stop) {   /* send a stop at last one */
            val |= 1 << IC_DATA_CMD_STOP_FIELD_OFFSET;
        }

        i2c_reg->ic_data_cmd = val;

        /* need check if data ready */
        while (is_rxfifo_empty(p_i2c_con) && --count)
            spin(SPIN_STEP);

        if (count == 0) { /* time out */
            /* should be reconfig in next operation */
            return ERR_TIMED_OUT;
        }

        val = i2c_reg->ic_data_cmd;
        p[i] = val & 0xff;
    }

    return NO_ERROR;
}

status_t dw_i2c_write_reg_bytes(dw_i2c_context *p_i2c_con, uint8_t address,
                                uint8_t reg, void *buf, size_t cnt)
{
    status_t ret;
    spin_lock_saved_state_t states;
    spin_lock_irqsave(&p_i2c_con->bus_lock, states);
    //mutex_acquire(&p_i2c_con->bus_mutex);
    ret = dw_i2c_transmit(p_i2c_con, address, &reg, 1, true, false);

    if (ret != NO_ERROR)
        goto err;

    ret = dw_i2c_transmit(p_i2c_con, address, buf, cnt, false, true);
err:
    //mutex_release(&p_i2c_con->bus_mutex);
    spin_unlock_irqrestore(&p_i2c_con->bus_lock, states);
    return ret;
}

status_t dw_i2c_read_reg_bytes(dw_i2c_context *p_i2c_con, uint8_t address,
                               uint8_t reg, void *buf, size_t cnt)
{
    status_t ret;
    spin_lock_saved_state_t states;
    spin_lock_irqsave(&p_i2c_con->bus_lock, states);
    //mutex_acquire(&p_i2c_con->bus_mutex);
    ret = dw_i2c_transmit(p_i2c_con, address, &reg, 1, true, false);

    if (ret != NO_ERROR)
        goto err;

    ret = dw_i2c_receive(p_i2c_con, address, buf, cnt, false, true);
err:
    //mutex_release(&p_i2c_con->bus_mutex);
    spin_unlock_irqrestore(&p_i2c_con->bus_lock, states);
    return ret;
}

status_t dw_i2c_scan_(dw_i2c_context *p_i2c_con, uint8_t address,
                      bool start,
                      bool stop)
{
    if (does_need_config(p_i2c_con, address) &&
            !dw_i2c_switch_target(p_i2c_con, address))
        return ERR_NOT_CONFIGURED;

    dw_i2c_config_t *info = &p_i2c_con->info;
    i2c_reg_type_t *i2c_reg = (i2c_reg_type_t *)info->io_base;
    uint32_t val;
    val = READ_CMD << IC_DATA_CMD_CMD_FIELD_OFFSET;
    val |= 1 << IC_DATA_CMD_RESTART_FIELD_OFFSET;
    val |= 1 << IC_DATA_CMD_STOP_FIELD_OFFSET;
    i2c_reg->ic_data_cmd = val;
    return NO_ERROR;
}

status_t dw_i2c_scan(dw_i2c_context *p_i2c_con, uint8_t address)
{
    status_t ret;
    spin_lock_saved_state_t states;
    spin_lock_irqsave(&p_i2c_con->bus_lock, states);
    ret = dw_i2c_scan_(p_i2c_con, address, true, true);

    if (ret != NO_ERROR)
        goto err;

err:
    //mutex_release(&p_i2c_con->bus_mutex);
    spin_unlock_irqrestore(&p_i2c_con->bus_lock, states);
    return ret;
}

status_t dw_i2c_write_reg(dw_i2c_context *p_i2c_con, uint8_t address,
                          void *reg,
                          size_t cnt)
{
    status_t ret;
    spin_lock_saved_state_t states;
    spin_lock_irqsave(&p_i2c_con->bus_lock, states);
    ret = dw_i2c_transmit(p_i2c_con, address, reg, cnt, true, true);

    if (ret != NO_ERROR)
        goto err;

err:
    //mutex_release(&p_i2c_con->bus_mutex);
    spin_unlock_irqrestore(&p_i2c_con->bus_lock, states);
    return ret;
}
/* Wait FIFO clear and then read abort reg.  */
inline u32 wait_fifo_empty(i2c_reg_type_t *i2c_reg)
{
    u32 wait_cnt = 0;
    u32 status = 0;

    while (wait_cnt < 500) {
        status = i2c_reg->ic_status;

        if (status == 0x6) {
            /* printf("FIFO Clear!\n"); */
            break;
        }

        spin(100);
        wait_cnt++;
    }

    u32 abrt = i2c_reg->ic_clr_tx_abrt;
    return (abrt);
}
static status_t dw_i2c_wait_fifo_clear(dw_i2c_context *p_i2c_con)
{
    dw_i2c_config_t *info = &p_i2c_con->info;
    i2c_reg_type_t *i2c_reg = (i2c_reg_type_t *)info->io_base;
    // u32 stat = i2c_reg->ic_raw_intr_stat;
    // u32 res = i2c_reg->ic_clr_tx_abrt;
    wait_fifo_empty(i2c_reg);
    return (i2c_reg->ic_status);
}
#if 0
status_t dw_i2c_write_reg_data(dw_i2c_context *p_i2c_con, uint8_t address,
                               void *reg, size_t cnt_reg, void *data,
                               size_t cnt)
{
    status_t ret;
    spin_lock_saved_state_t states;
    spin_lock_irqsave(&p_i2c_con->bus_lock, states);
    ret = dw_i2c_transmit(p_i2c_con, address, reg, cnt_reg, true, false);

    if (ret != NO_ERROR)
        goto err;

    ret = dw_i2c_transmit(p_i2c_con, address, data, cnt, false, true);
    dw_i2c_wait_fifo_clear(p_i2c_con);
err:
    //mutex_release(&p_i2c_con->bus_mutex);
    spin_unlock_irqrestore(&p_i2c_con->bus_lock, states);
    return ret;
}

status_t dw_i2c_read_reg_data(dw_i2c_context *p_i2c_con, uint8_t address,
                              void *reg, size_t cnt_reg, void *data, size_t cnt)
{
    status_t ret;
    spin_lock_saved_state_t states;
    spin_lock_irqsave(&p_i2c_con->bus_lock, states);
    ret = dw_i2c_transmit(p_i2c_con, address, reg, cnt_reg, true, false);

    if (ret != NO_ERROR)
        goto err;

    ret = dw_i2c_receive(p_i2c_con, address, data, cnt, false, true);
    dw_i2c_wait_fifo_clear(p_i2c_con);
err:
    //mutex_release(&p_i2c_con->bus_mutex);
    spin_unlock_irqrestore(&p_i2c_con->bus_lock, states);
    return ret;
}
#else
status_t dw_i2c_write_reg_data(dw_i2c_context *p_i2c_con, uint8_t address,
                               void *reg, size_t cnt_reg, void *data,
                               size_t cnt)
{
    status_t ret;

    if (p_i2c_con->info.poll == 1) {
        spin_lock_saved_state_t states;
        spin_lock_irqsave(&p_i2c_con->bus_lock, states);
        ret = dw_i2c_transmit(p_i2c_con, address, reg, cnt_reg, true, false);

        if (ret != NO_ERROR)
            goto err;

        ret = dw_i2c_transmit(p_i2c_con, address, data, cnt, false, true);
        dw_i2c_wait_fifo_clear(p_i2c_con);
err:
        spin_unlock_irqrestore(&p_i2c_con->bus_lock, states);
        return ret;
    }

    mutex_acquire(&p_i2c_con->bus_mutex);
    struct i2c_msg msg;
    u8 *addr_buf;
    ret = dw_i2c_wait_bus_not_busy(p_i2c_con);

    if (ret < 0) {
        mutex_release(&p_i2c_con->bus_mutex);
        return ret;
    }

    addr_buf = (u8 *)malloc(cnt_reg + cnt);
    memcpy(addr_buf, reg, cnt_reg);
    memcpy(addr_buf + cnt_reg, data, cnt);
    msg.flags = I2C_M_WR;
    msg.addr  = address;
    msg.len   = cnt_reg + cnt;
    msg.buf   = (u8 *)addr_buf;
    p_i2c_con->msgs = &msg;
    p_i2c_con->msgs_num = 1;
    dprintf(I2C_LOG,
            "\n%s: addr=0x%x, reg[0]=0x%x, cnt_reg=%d, data[0]=0x%x, cnt=%d\n",
            __func__, address, ((u8 *)reg)[0], (u32)cnt_reg, ((u8 *)data)[0],
            (u32)cnt);
    ret = dw_i2c_transfer(p_i2c_con);
    free(addr_buf);
    mutex_release(&p_i2c_con->bus_mutex);
    return ret;
}

status_t dw_i2c_read_reg_data(dw_i2c_context *p_i2c_con, uint8_t address,
                              void *reg, size_t cnt_reg, void *data, size_t cnt)
{
    status_t ret;

    if (p_i2c_con->info.poll == 1) {
        spin_lock_saved_state_t states;
        spin_lock_irqsave(&p_i2c_con->bus_lock, states);
        ret = dw_i2c_transmit(p_i2c_con, address, reg, cnt_reg, true, false);

        if (ret != NO_ERROR)
            goto err;

        ret = dw_i2c_receive(p_i2c_con, address, data, cnt, false, true);
        dw_i2c_wait_fifo_clear(p_i2c_con);
err:
        spin_unlock_irqrestore(&p_i2c_con->bus_lock, states);
        return ret;
    }

    mutex_acquire(&p_i2c_con->bus_mutex);
    struct i2c_msg msgs[2];
    ret = dw_i2c_wait_bus_not_busy(p_i2c_con);

    if (ret < 0) {
        mutex_release(&p_i2c_con->bus_mutex);
        return ret;
    }

    msgs[0].flags = I2C_M_WR;
    msgs[0].addr  = address;
    msgs[0].len   = cnt_reg;
    msgs[0].buf   = (u8 *)reg;
    msgs[1].flags = I2C_M_RD;
    msgs[1].addr  = address;
    msgs[1].len   = cnt;
    msgs[1].buf   = (u8 *)data;
    p_i2c_con->msgs = msgs;
    p_i2c_con->msgs_num = 2;
    dprintf(I2C_LOG, "\n%s: addr=0x%x, reg[0]=0x%x, cnt_reg=%d, cnt=%d\n",
            __func__, address, ((u8 *)reg)[0], (u32)cnt_reg, (u32)cnt);
    ret = dw_i2c_transfer(p_i2c_con);
    mutex_release(&p_i2c_con->bus_mutex);
    return ret;
}

status_t dw_i2c_common_xfer(dw_i2c_context *p_i2c_con,
                            struct i2c_msg *msgs, int num)
{
    status_t ret;
    spin_lock_saved_state_t states;

    if (num <= 0) {
        dprintf(0, "%s: i2c msg num invalid\n", __func__);
        return -1;
    }

    if (p_i2c_con->info.poll == 1) {
        spin_lock_irqsave(&p_i2c_con->bus_lock, states);

        if (num == 1) {
            if (msgs[0].flags & I2C_M_RD) {
                ret = dw_i2c_receive(p_i2c_con, msgs[0].addr, msgs[0].buf, msgs[0].len,
                                     true, true);
            }
            else {
                ret = dw_i2c_transmit(p_i2c_con, msgs[0].addr, msgs[0].buf, msgs[0].len,
                                      true, true);
            }

            dw_i2c_wait_fifo_clear(p_i2c_con);
        }
        else {
            for (int i = 0; i < num; i++) {
                if (msgs[i].flags & I2C_M_RD) {
                    if (i == 0)
                        ret = dw_i2c_receive(p_i2c_con, msgs[i].addr, msgs[i].buf, msgs[i].len,
                                             true, false);
                    else if (i == num - 1)
                        ret = dw_i2c_receive(p_i2c_con, msgs[i].addr, msgs[i].buf, msgs[i].len,
                                             false, true);
                    else
                        ret = dw_i2c_receive(p_i2c_con, msgs[i].addr, msgs[i].buf, msgs[i].len,
                                             false, false);
                }
                else {
                    if (i == 0)
                        ret = dw_i2c_transmit(p_i2c_con, msgs[i].addr, msgs[i].buf, msgs[i].len,
                                              true, false);
                    else if (i == num - 1)
                        ret = dw_i2c_transmit(p_i2c_con, msgs[i].addr, msgs[i].buf, msgs[i].len,
                                              false, true);
                    else
                        ret = dw_i2c_transmit(p_i2c_con, msgs[i].addr, msgs[i].buf, msgs[i].len,
                                              false, false);
                }

                dw_i2c_wait_fifo_clear(p_i2c_con);

                if (ret != NO_ERROR)
                    break;
            }
        }

        spin_unlock_irqrestore(&p_i2c_con->bus_lock, states);
    }
    else {
        mutex_acquire(&p_i2c_con->bus_mutex);
        ret = dw_i2c_wait_bus_not_busy(p_i2c_con);

        if (ret < 0) {
            mutex_release(&p_i2c_con->bus_mutex);
            return ret;
        }

        p_i2c_con->msgs = msgs;
        p_i2c_con->msgs_num = num;
        dprintf(I2C_LOG, "%s: addr=0x%x, msg num=0x%x\n", __func__, msgs[0].addr,
                num);
        ret = dw_i2c_transfer(p_i2c_con);
        mutex_release(&p_i2c_con->bus_mutex);
    }

    return ret;
}

#endif
