/*
 * dw_i2c.h
 *
 * Copyright (c) 2019 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: I2C driver header.
 *
 * Revision History:
 * -----------------
 */
#ifndef __DW_I2C_H__
#define __DW_I2C_H__

#include <common_hdr.h>

#define LOCAL_TRACE 1
#define DEFAULT_I2C_LOG_LEVEL        1

typedef struct _i2c_reg_type {
    volatile uint32_t ic_con;   /* control register */
    volatile uint32_t ic_tar;   /* target address */
    volatile uint32_t ic_sar;   /* slave address */
    volatile uint32_t ic_reserve1;
    volatile uint32_t ic_data_cmd;  /* 0x10 */
    volatile uint32_t ic_ss_scl_hcnt;
    volatile uint32_t ic_ss_scl_lcnt;
    volatile uint32_t ic_fs_scl_hcnt;
    volatile uint32_t ic_fs_scl_lcnt;   /* 0x20 */
    volatile uint32_t ic_hs_scl_hcnt;
    volatile uint32_t ic_hs_scl_lcnt;
    volatile uint32_t ic_intr_stat;
    volatile uint32_t ic_intr_mask; /* 0x30 */
    volatile uint32_t ic_raw_intr_stat;
    volatile uint32_t ic_rx_tl;
    volatile uint32_t ic_tx_tl;
    volatile uint32_t ic_clr_intr;  /* 0x40 */
    volatile uint32_t ic_clr_rx_under;
    volatile uint32_t ic_clr_rx_over;
    volatile uint32_t ic_clr_tx_over;
    volatile uint32_t ic_clr_rd_req;    /* 0x50 */
    volatile uint32_t ic_clr_tx_abrt;
    volatile uint32_t ic_clr_rx_done;
    volatile uint32_t ic_clr_activity;
    volatile uint32_t ic_clr_stop_det;  /* 0x60 */
    volatile uint32_t ic_clr_start_det;
    volatile uint32_t ic_clr_gen_call;
    volatile uint32_t ic_enable;
    volatile uint32_t ic_status;    /* 0x70 */
    volatile uint32_t ic_txflr;
    volatile uint32_t ic_rxflr;
    volatile uint32_t ic_sda_hold;
    volatile uint32_t ic_tx_abrt_source;/* 0x80 */
    volatile uint32_t ic_reserve4;
    volatile uint32_t ic_dma_cr;
    volatile uint32_t ic_dma_tdlr;
    volatile uint32_t ic_dma_rdlr;  /* 0x90 */
    volatile uint32_t ic_sda_setup;
    volatile uint32_t ic_ack_general_call;
    volatile uint32_t ic_enable_status;
    volatile uint32_t ic_fs_spklen; /* 0xa0 */
    volatile uint32_t ic_reserve5;
    volatile uint32_t ic_clr_restart_det;
    volatile uint32_t ic_scl_stuck_at_low_timeout;
    volatile uint32_t ic_sda_stuck_at_low_timeout;  /* 0xb0 */
    volatile uint32_t ic_clr_scl_stuck_det;
    volatile uint32_t ic_reserve6;
    volatile uint32_t ic_smbus_clk_low_sext;
    volatile uint32_t ic_smbus_clk_low_mext;    /* 0xc0 */
    volatile uint32_t ic_smbus_thigh_max_idle_count;
    volatile uint32_t ic_smbus_intr_stat;
    volatile uint32_t ic_smbus_intr_mask;
    volatile uint32_t ic_smbus_raw_intr_stat;   /* 0xd0 */
    volatile uint32_t ic_clr_smbus_intr;
    volatile uint32_t ic_reserve7;
    volatile uint32_t ic_smbus_udid_lsb;
    volatile uint32_t ic_reserve8;  /* 0xe0 */
    volatile uint32_t ic_reserve9;
    volatile uint32_t ic_reserve10;
    volatile uint32_t ic_reserve11;
    volatile uint32_t ic_reserve12; /* 0xf0 */
    volatile uint32_t ic_comp_param_1;
    volatile uint32_t ic_comp_version;
    volatile uint32_t ic_comp_type;
} i2c_reg_type_t;

/*
 * 1: standard mode (100 kbit/s)
 * 2: fast mode (<=400 kbit/s) or fast mode plus (<=1000Kbit/s)
 * 3: high speed mode (3.4 Mbit/s)
 */
typedef enum {
    I2C_SPEED_STANDARD = 1,
    I2C_SPEED_FAST,
    I2C_SPEED_HIGH,
} i2c_speed_e;

typedef enum {
    ADDR_7BITS,
    ADDR_10BITS
} addr_bits_e;

typedef enum {
    SLAVE_MODE,
    MASTER_MODE,
} i2c_mode_e;

typedef struct {
    vaddr_t io_base;
    i2c_speed_e speed;
    addr_bits_e addr_bits;
    i2c_mode_e  mode;
    bool restart;
    uint32_t slave_addr; /* for slave mode */
} dw_i2c_config_t;

typedef struct {
    dw_i2c_config_t info;
    bool is_configured;
    bool is_added;
    uint32_t cur_addr;
    uint32_t bus;
    mutex_t bus_mutex;
} dw_i2c_context;

/* platform api */
void dw_i2c_add_bus(dw_i2c_context *p_i2c_con,
                    const dw_i2c_config_t *info);

/* function api */
void dw_i2c_init_early(void);
void dw_i2c_init(void);

status_t dw_i2c_transmit(dw_i2c_context *p_i2c_con, uint8_t address,
                         const void *buf, size_t cnt, bool start, bool stop);
status_t dw_i2c_receive(dw_i2c_context *p_i2c_con, uint8_t address,
                        void *buf, size_t cnt, bool start, bool stop);
status_t dw_i2c_write_reg_bytes(dw_i2c_context *p_i2c_con, uint8_t address,
                                uint8_t reg, void *buf, size_t cnt);
status_t dw_i2c_read_reg_bytes(dw_i2c_context *p_i2c_con, uint8_t address,
                               uint8_t reg, void *buf, size_t cnt);
status_t dw_i2c_slave_receive(dw_i2c_context *p_i2c_con, void *buf,
                              size_t cnt, bool start, bool stop);
bool dw_i2c_set_busconfig(dw_i2c_context *p_i2c_con,
                          const dw_i2c_config_t *info);

status_t dw_i2c_scan(dw_i2c_context *p_i2c_con, uint8_t address);
status_t dw_i2c_write_reg(dw_i2c_context *p_i2c_con, uint8_t address,
                          void *reg, size_t cnt);
status_t dw_i2c_write_reg_data(dw_i2c_context *p_i2c_con, uint8_t address,
                               void *reg, size_t cnt_reg, void *data, size_t cnt);
status_t dw_i2c_read_reg_data(dw_i2c_context *p_i2c_con, uint8_t address,
                              void *reg, size_t cnt_reg, void *data, size_t cnt);

#endif  //__DW_I2C_H__
