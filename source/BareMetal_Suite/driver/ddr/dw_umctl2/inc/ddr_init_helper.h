/********************************************************
 *          Copyright(c) 2020   Semidrive               *
 *******************************************************/

#ifndef __DDR_INIT_HELPER_H__
#define __DDR_INIT_HELPER_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "DWC_ddr_umctl2_reg.h"
#include "dwc_ddrphy_top_reg.h"

typedef enum {
    DDRPHY_FW_LPDDR4X_1D,
    DDRPHY_FW_LPDDR4X_2D,
    DDRPHY_FW_LPDDR4_1D,
    DDRPHY_FW_LPDDR4_2D,
    DDRPHY_FW_DDR4_1D,
    DDRPHY_FW_DDR4_2D,
    DDRPHY_FW_DDR3_1D,
    DDRPHY_FW_LPDDR4X_DIAG,
    DDRPHY_FW_LPDDR4_DIAG,
    DDRPHY_FW_DDR4_DIAG
} ddrphy_fw_e;

enum {
    ACT_WR = 1,
    ACT_WR_BITS,
    ACT_POLL_BITS,
    ACT_FUNC_CALL,
    ACT_END = 0xffu,
};

enum {
    FUNC_INFO = 1,
    FUNC_RESET,
    FUNC_LOADFW,
    FUNC_FW_MON,
    FUNC_CFG_CLK,
    FUNC_GET_TRAIN_PARA,
    FUNC_USE_TRAIN_PARA,
    FUNC_PRINT_SEQ_VERSION,
    FUNC_UDELAY,
    FUNC_DIAG,
};

enum {
    FREQ_DDR_1600 = 0,
    FREQ_DDR_2133 = 1,
    FREQ_DDR_3200 = 2,
    FREQ_DDR_4266 = 3,
    FREQ_DDR_800 = 4,
	FREQ_DDR_2400 = 5,
};

typedef struct {
    uint8_t act;
    uint8_t rsvd[3];
    uint32_t addr;
    uint32_t val;
    uint32_t rsvd2;
} ddr_wr_t;

#define APB_DDRCTRL_BASE (0xf3000000u)
#define APB_DDRPHY_BASE (0xf2000000u)

typedef struct {
    uint8_t act;
    uint8_t rsvd[3];
    uint32_t addr;
    uint32_t val;
    uint8_t shift;
    uint8_t width;
    uint8_t rsvd2[2];
} ddr_wr_bits_t;

typedef struct {
    uint8_t act;
    uint8_t tmt_h;
    uint16_t tmt;
    uint32_t addr;
    uint32_t val;
    uint32_t msk;
} ddr_poll_bits_t;

typedef struct {
    uint8_t act;
    uint8_t rsvd[2];
    uint8_t func;
    uint32_t para[3];
} ddr_func_call_t;

typedef union {
    ddr_wr_t wr;
    ddr_wr_bits_t wrbits;
    ddr_poll_bits_t poll;
    ddr_func_call_t call;
} ddr_act_u;

enum ddr_mem_type {
    ICCM = 0,
    DCCM = 1,
    DIAGI = 2,
    DIAGD = 3,
    MAXCCM
};

#define DDR_AXI_RSTN        31
#define DDR_CORE_RSTN       32

#define DDR_W32(a, v)   {.wr = {ACT_WR, {0}, (a), (v)}},
#define DDR_R32(a)
#define DDR_W32_BITS(a, shift, w, v)   \
    {.wrbits = {ACT_WR_BITS, {0}, a, v, shift, w}},
#define DDR_POLL_BITS(a, msk, v, tmt)   \
    {.poll = {ACT_POLL_BITS, (uint8_t)(tmt << 16), (uint16_t)tmt, a, v, msk}},
#define CFG_RST(id, v)\
    {.call = {ACT_FUNC_CALL, {0}, FUNC_RESET, {id, v, 0}}},
#define DDRPHY_WAITFWDONE(fw)\
    {.call = {ACT_FUNC_CALL, {0}, FUNC_FW_MON, {fw, 0, 0}}},
#define LOAD_DDR_TRAINING_FW(mem, train_2d) \
    {.call = {ACT_FUNC_CALL, {0}, FUNC_LOADFW, {mem, train_2d, 0}}},
#define DDR_INFO(fmt, args...) \
    {.call = {ACT_FUNC_CALL, {0}, FUNC_INFO, {0, 0, 0}}},
#define CFG_DDR_CLK(freq) \
    {.call = {ACT_FUNC_CALL, {0}, FUNC_CFG_CLK, {freq, 0, 0}}},

#define dwc_ddrphy_phyinit_userCustom_H_readMsgBlock(x)\
    {.call = {ACT_FUNC_CALL, {0}, FUNC_GET_TRAIN_PARA, {0, 0, 0}}},
#define timing_reg_update_after_training()  \
    {.call = {ACT_FUNC_CALL, {0}, FUNC_USE_TRAIN_PARA, {0, 0, 0}}},

#define DDR_SEQ_VERSION_PRINT(x)\
    {.call = {ACT_FUNC_CALL, {0}, FUNC_PRINT_SEQ_VERSION, {(x), 0, 0}}},

#define RUN_DIAG(x)\
    {.call = {ACT_FUNC_CALL, {0}, FUNC_DIAG, {(x), 0, 0}}},

#define DDR_SEQ_BEGIN   \
    const ddr_act_u ddr_init_seq[] __attribute__((section("ddr_init_seq_sec")))= {

#define DDR_SEQ_END \
    {.call = {ACT_END}},\
    };

typedef struct {
    char name[48];
    uint32_t offset;
    uint32_t size;
    uint8_t rsvd[8];
} hdr_entry_t;

int gpt_partion_search(const char *name, uint32_t *base, uint32_t *len);

#endif  /* __DDR_INIT_HELPER_H__ */
