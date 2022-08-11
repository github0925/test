/********************************************************
 *	        Copyright(c) 2020	Semidrive 		        *
 *******************************************************/

#ifndef __DDR_INIT_HELPER_H__
#define __DDR_INIT_HELPER_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <dw_umctl2.h>
#include "lib/reg.h"

typedef enum {
    DDRPHY_FW_LPDDR4X_1D,
    DDRPHY_FW_LPDDR4X_2D,
    DDRPHY_FW_LPDDR4_1D,
    DDRPHY_FW_LPDDR4_2D,
    DDRPHY_FW_DDR4_1D,
    DDRPHY_FW_DDR4_2D,
    DDRPHY_FW_DDR3_1D
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
};

enum {
    FREQ_DDR_1600 = 0,
    FREQ_DDR_2133 = 1,
    FREQ_DDR_3200 = 2,
    FREQ_DDR_4266 = 3,
    FREQ_DDR_2400 = 5,
};

typedef struct {
    uint8_t act;
    uint8_t rsvd[3];
    uint32_t addr;
    uint32_t val;
    uint32_t rsvd2;
} ddr_wr_t;

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

#define ICCM    0
#define DCCM    1

#if CFG_PARSE_RUN_DDR_INIT_SEQ

#define DDR_AXI_RSTN	    31
#define DDR_CORE_RSTN  		32

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

#define DDR_SEQ_END \
    {.call = {ACT_END}},\
    };

int32_t run_ddr_init_seq(const ddr_act_u *act);

#else

#define DDR_AXI_RSTN		RES_MODULE_RST_SEC_DDR_SW_2
#define DDR_CORE_RSTN  		RES_MODULE_RST_SEC_DDR_SW_3

#define DDR_INFO(fmt, args...)  DDR_DBG(fmt, ##args)

#define DDR_W32(a, v)   writel(v, a)
#define DDR_R32(a)
#define DDR_W32_BITS(a, shift, w, v)   RMWREG32(a, shift, w, v)
#define DDR_POLL_BITS(a, msk, v, tmt)   \
    do {\
        uint32_t cnt = 0;\
        while((cnt++ < (uint32_t)(tmt)) && ((readl(a) & (msk)) != ((v) & (msk))));\
    } while(0)

#define CFG_RST(id, v)  reset_module(id)
#define DDRPHY_WAITFWDONE(fw) \
    do {\
        if(0 != dwc_ddrphy_fw_monitor_loop(fw)){\
            return -1;\
        }\
    } while(0)

#define LOAD_DDR_TRAINING_FW(mem, train_2d) load_ddr_training_fw(mem, train_2d)

#define dwc_ddrphy_phyinit_userCustom_H_readMsgBlock    _dwc_ddrphy_phyinit_userCustom_H_readMsgBlock_
#define timing_reg_update_after_training    _timing_reg_update_after_training_

#define DDR_SEQ_VERSION_PRINT(x)  printf("DDR Sequence Version: 0x%08x\n", (x))

#define CFG_DDR_CLK(freq)

#endif  /* CFG_PARSE_RUN_DDR_INIT_SEQ */

#endif  /* __DDR_INIT_HELPER_H__ */
