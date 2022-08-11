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
#include <dw_i2c.h>
#include <dw_i2c_reg.h>
#include <error.h>

#define TX_RX_TIMEOUT 300 /* us, tx/rx retry times */
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
#define I2C_LOG 1

/* wrappers to make this driver happy */
#define dprintf(x, fmt, args...)
extern void udelay(uint32_t us);
#define spin(x) udelay(x)
#define printf(fmt, args...)
#define mutex_acquire(x)
#define mutex_release(x)

static bool does_need_config(dw_i2c_context *p_i2c_con, uint32_t addr)
{
    dprintf(I2C_LOG, "does_need_config(%d, 0x%x): 0x%x, %d.\n", p_i2c_con->bus,
            addr, p_i2c_con->cur_addr, p_i2c_con->is_configured);

    if (p_i2c_con->cur_addr != addr)
        return true;

    if (!p_i2c_con->is_configured)
        return true;

    return false;
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
    val = 0x20 <<
          IC_SMBUS_THIGH_MAX_IDLE_COUNT_SMBUS_THIGH_MAX_BUS_IDLE_CNT_FIELD_OFFSET;
    i2c_reg->ic_smbus_thigh_max_idle_count = val;

    /* speed, addr mode, master mode, */
    /* speed */
    val = info->speed << IC_CON_SPEED_FIELD_OFFSET;

    /* addr bits */
    if (info->mode == SLAVE_MODE) {
        val |= info->addr_bits << IC_CON_IC_10BITADDR_SLAVE_FIELD_OFFSET;
    } else {
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
    } else {
        val = 0 << IC_TAR_SPECIAL_FIELD_OFFSET;
        val |= addr << IC_TAR_IC_TAR_FIELD_OFFSET;
        i2c_reg->ic_tar = val;
    }

    /* enable */
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
            p_i2c_con->bus, addr,
            io_base);

    i2c_reg->ic_enable &= ~(0x1 << IC_ENABLE_ENABLE_FIELD_OFFSET);

    val = 0 << IC_TAR_SPECIAL_FIELD_OFFSET;
    val |= (addr << IC_TAR_IC_TAR_FIELD_OFFSET);
    i2c_reg->ic_tar = val;

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

bool dw_i2c_set_busconfig(dw_i2c_context *p_i2c_con,
                          const dw_i2c_config_t *info)
{
    dprintf(I2C_LOG, "dw_i2c_set_busconfig() .\n");

    if ((p_i2c_con->info.io_base == info->io_base)
        && (p_i2c_con->info.speed == info->speed) \
        && (p_i2c_con->info.addr_bits == info->addr_bits)
        && (p_i2c_con->info.mode == info->mode) \
        && (p_i2c_con->info.restart == info->restart)
        && (p_i2c_con->info.slave_addr == info->slave_addr)) {
        printf("already configed bus %d.\n", p_i2c_con->bus);
        return false;
    }

    p_i2c_con->info = *info;
    p_i2c_con->is_added = true;
    p_i2c_con->is_configured = false;

    if (info->mode == SLAVE_MODE) {
        if (dw_i2c_config(p_i2c_con, info->slave_addr))
            return true;
    } else if (info->mode == MASTER_MODE) {
        if (dw_i2c_config(p_i2c_con, DEF_TAR))
            return true;
    }

    return false;
}

status_t dw_i2c_transmit(dw_i2c_context *p_i2c_con, uint8_t address,
                         const void *buf, size_t cnt, bool start, bool stop)
{
    if (does_need_config(p_i2c_con, address)
        && !dw_i2c_switch_target(p_i2c_con, address))
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
        } else if ((i == cnt - 1) && stop) { /* send a stop at last one */
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
                        void *buf, size_t cnt, bool start, bool stop)
{
    if (does_need_config(p_i2c_con, address)
        && !dw_i2c_switch_target(p_i2c_con, address))
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
        } else if ((i == cnt - 1) && stop) { /* send a stop at last one */
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
                              size_t cnt, bool start, bool stop)
{
    uint8_t address;
    dw_i2c_config_t *info = &p_i2c_con->info;
    address = info->slave_addr;

    if (does_need_config(p_i2c_con, address)
        && !dw_i2c_config(p_i2c_con, address))
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
        } else if ((i == cnt - 1) && stop) {    /* send a stop at last one */
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
    mutex_acquire(&p_i2c_con->bus_mutex);
    ret = dw_i2c_transmit(p_i2c_con, address, &reg, 1, true, false);

    if (ret != NO_ERROR)
        goto err;

    ret = dw_i2c_transmit(p_i2c_con, address, buf, cnt, false, true);

err:
    mutex_release(&p_i2c_con->bus_mutex);
    return ret;
}

status_t dw_i2c_read_reg_bytes(dw_i2c_context *p_i2c_con, uint8_t address,
                               uint8_t reg, void *buf, size_t cnt)
{
    status_t ret;
    mutex_acquire(&p_i2c_con->bus_mutex);
    ret = dw_i2c_transmit(p_i2c_con, address, &reg, 1, true, false);

    if (ret != NO_ERROR)
        goto err;

    ret = dw_i2c_receive(p_i2c_con, address, buf, cnt, false, true);

err:
    mutex_release(&p_i2c_con->bus_mutex);
    return ret;
}


status_t dw_i2c_scan_(dw_i2c_context *p_i2c_con, uint8_t address,
                      bool start, bool stop)
{
    if (does_need_config(p_i2c_con, address)
        && !dw_i2c_switch_target(p_i2c_con, address))
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
    mutex_acquire(&p_i2c_con->bus_mutex);
    ret = dw_i2c_scan_(p_i2c_con, address, true, true);

    if (ret != NO_ERROR)
        goto err;

err:
    mutex_release(&p_i2c_con->bus_mutex);

    return ret;
}

status_t dw_i2c_write_reg(dw_i2c_context *p_i2c_con, uint8_t address,
                          void *reg, size_t cnt)
{
    status_t ret;
    mutex_acquire(&p_i2c_con->bus_mutex);
    ret = dw_i2c_transmit(p_i2c_con, address, reg, cnt, true, true);

    if (ret != NO_ERROR)
        goto err;

err:
    mutex_release(&p_i2c_con->bus_mutex);
    return ret;
}

status_t dw_i2c_write_reg_data(dw_i2c_context *p_i2c_con, uint8_t address,
                               void *reg, size_t cnt_reg, void *data, size_t cnt)
{
    status_t ret;
    mutex_acquire(&p_i2c_con->bus_mutex);
    ret = dw_i2c_transmit(p_i2c_con, address, reg, cnt_reg, true, false);

    if (ret != NO_ERROR)
        goto err;

    ret = dw_i2c_transmit(p_i2c_con, address, data, cnt, false, true);

err:
    mutex_release(&p_i2c_con->bus_mutex);
    return ret;
}

status_t dw_i2c_read_reg_data(dw_i2c_context *p_i2c_con, uint8_t address,
                              void *reg, size_t cnt_reg, void *data, size_t cnt)
{
    status_t ret;
    mutex_acquire(&p_i2c_con->bus_mutex);
    ret = dw_i2c_transmit(p_i2c_con, address, reg, cnt_reg, true, false);

    if (ret != NO_ERROR)
        goto err;

    ret = dw_i2c_receive(p_i2c_con, address, data, cnt, false, true);
err:
    mutex_release(&p_i2c_con->bus_mutex);
    return ret;
}
