/*
 * dw_dmac_mux.h
 *
 * Copyright (c) 2019 Semidrive Semiconductor.
 * All rights reserved.
 *
 * dw dma controller head
 *
 * Revision History:
 * -----------------
 * 0.1, 3/29/2019 yishao init version
 */
#ifndef __DW_DMAC_MUX_H
#define __DW_DMAC_MUX_H
#include <__regs_base.h>
#include <sys/types.h>
#define DMAC_MUX_CHANN_NUMB (8)
typedef enum
{
  DMA_MUX_CAN5 = 0,
  DMA_MUX_CAN6 = 1,
  DMA_MUX_CAN7 = 2,
  DMA_MUX_CAN8 = 3,
  DMA_MUX_DC1 = 4,
  DMA_MUX_DC2 = 5,
  DMA_MUX_DC3 = 6,
  DMA_MUX_DC4 = 7,
  DMA_MUX_DC5 = 8,
  DMA_MUX_DDR_SS = 9,
  DMA_MUX_DP1 = 10,
  DMA_MUX_DP2 = 11,
  DMA_MUX_DP3 = 12,
  DMA_MUX_ENET2_REQ0 = 13,
  DMA_MUX_ENET2_REQ1 = 14,
  DMA_MUX_ENET2_REQ2 = 15,
  DMA_MUX_ENET2_REQ3 = 16,
  DMA_MUX_I2C5_TX = 17,
  DMA_MUX_I2C5_RX = 18,
  DMA_MUX_I2C6_TX = 19,
  DMA_MUX_I2C6_RX = 20,
  DMA_MUX_I2C7_TX = 21,
  DMA_MUX_I2C7_RX = 22,
  DMA_MUX_I2C8_TX = 23,
  DMA_MUX_I2C8_RX = 24,
  DMA_MUX_I2C9_TX = 25,
  DMA_MUX_I2C9_RX = 26,
  DMA_MUX_I2C10_TX = 27,
  DMA_MUX_I2C10_RX = 28,
  DMA_MUX_I2C11_TX = 29,
  DMA_MUX_I2C11_RX = 30,
  DMA_MUX_I2C12_TX = 31,
  DMA_MUX_I2C12_RX = 32,
  DMA_MUX_I2C13_TX = 33,
  DMA_MUX_I2C13_RX = 34,
  DMA_MUX_I2C14_TX = 35,
  DMA_MUX_I2C14_RX = 36,
  DMA_MUX_I2C15_TX = 37,
  DMA_MUX_I2C15_RX = 38,
  DMA_MUX_I2C16_TX = 39,
  DMA_MUX_I2C16_RX = 40,
  DMA_MUX_I2S_MC1_RX = 41,
  DMA_MUX_I2S_MC1_TX = 42,
  DMA_MUX_I2S_MC2_RX = 43,
  DMA_MUX_I2S_MC2_TX = 44,
  DMA_MUX_I2S_SC3_RX = 45,
  DMA_MUX_I2S_SC3_TX = 46,
  DMA_MUX_I2S_SC4_RX = 47,
  DMA_MUX_I2S_SC4_TX = 48,
  DMA_MUX_I2S_SC5_RX = 49,
  DMA_MUX_I2S_SC5_TX = 50,
  DMA_MUX_I2S_SC6_RX = 51,
  DMA_MUX_I2S_SC6_TX = 52,
  DMA_MUX_I2S_SC7_RX = 53,
  DMA_MUX_I2S_SC7_TX = 54,
  DMA_MUX_I2S_SC8_RX = 55,
  DMA_MUX_I2S_SC8_TX = 56,
  DMA_MUX_OSPI2_RX = 57,
  DMA_MUX_OSPI2_TX = 58,
  DMA_MUX_PWM3_REQ = 59,
  DMA_MUX_PWM4_REQ = 60,
  DMA_MUX_PWM5_REQ = 61,
  DMA_MUX_PWM6_REQ = 62,
  DMA_MUX_PWM7_REQ = 63,
  DMA_MUX_PWM8_REQ = 64,
  DMA_MUX_SPDIF1_REQ = 65,
  DMA_MUX_SPDIF2_REQ = 66,
  DMA_MUX_SPDIF3_REQ = 67,
  DMA_MUX_SPDIF4_REQ = 68,
  DMA_MUX_SPI5_RX = 69,
  DMA_MUX_SPI5_TX = 70,
  DMA_MUX_SPI6_RX = 71,
  DMA_MUX_SPI6_TX = 72,
  DMA_MUX_SPI7_RX = 73,
  DMA_MUX_SPI7_TX = 74,
  DMA_MUX_SPI8_RX = 75,
  DMA_MUX_SPI8_TX = 76,
  DMA_MUX_TIMER3_REQA = 77,
  DMA_MUX_TIMER3_REQB = 78,
  DMA_MUX_TIMER3_REQC = 79,
  DMA_MUX_TIMER3_REQD = 80,
  DMA_MUX_TIMER3_OVF = 81,
  DMA_MUX_TIMER4_REQA = 82,
  DMA_MUX_TIMER4_REQB = 83,
  DMA_MUX_TIMER4_REQC = 84,
  DMA_MUX_TIMER4_REQD = 85,
  DMA_MUX_TIMER4_OVF = 86,
  DMA_MUX_TIMER5_REQA = 87,
  DMA_MUX_TIMER5_REQB = 88,
  DMA_MUX_TIMER5_REQC = 89,
  DMA_MUX_TIMER5_REQD = 90,
  DMA_MUX_TIMER5_OVF = 91,
  DMA_MUX_TIMER6_REQA = 92,
  DMA_MUX_TIMER6_REQB = 93,
  DMA_MUX_TIMER6_REQC = 94,
  DMA_MUX_TIMER6_REQD = 95,
  DMA_MUX_TIMER6_OVF = 96,
  DMA_MUX_TIMER7_REQA = 97,
  DMA_MUX_TIMER7_REQB = 98,
  DMA_MUX_TIMER7_REQC = 99,
  DMA_MUX_TIMER7_REQD = 100,
  DMA_MUX_TIMER7_OVF = 101,
  DMA_MUX_TIMER8_REQA = 102,
  DMA_MUX_TIMER8_REQB = 103,
  DMA_MUX_TIMER8_REQC = 104,
  DMA_MUX_TIMER8_REQD = 105,
  DMA_MUX_TIMER8_OVF = 106,
  DMA_MUX_UART9_TX = 107,
  DMA_MUX_UART9_RX = 108,
  DMA_MUX_UART10_TX = 109,
  DMA_MUX_UART10_RX = 110,
  DMA_MUX_UART11_TX = 111,
  DMA_MUX_UART11_RX = 112,
  DMA_MUX_UART12_TX = 113,
  DMA_MUX_UART12_RX = 114,
  DMA_MUX_UART13_TX = 115,
  DMA_MUX_UART13_RX = 116,
  DMA_MUX_UART14_TX = 117,
  DMA_MUX_UART14_RX = 118,
  DMA_MUX_UART15_TX = 119,
  DMA_MUX_UART15_RX = 120,
  DMA_MUX_UART16_TX = 121,
  DMA_MUX_UART16_RX = 122,
  DMA_MUX_ADC = 123,
  DMA_MUX_CAN9 = 124,
  DMA_MUX_CAN10 = 125,
  DMA_MUX_CAN11 = 126,
  DMA_MUX_CAN12 = 127,
  DMA_MUX_CAN13 = 128,
  DMA_MUX_CAN14 = 129,
  DMA_MUX_CAN15 = 130,
  DMA_MUX_CAN16 = 131,
  DMA_MUX_CAN17 = 132,
  DMA_MUX_CAN18 = 133,
  DMA_MUX_CAN19 = 134,
  DMA_MUX_CAN20 = 135,
  DMA_MUX_SIZE,
  DMA_MUX_ERR_NO = 144,
  DMA_MUX_EXTRA_PORT = 255,
} DMR_REQ_PERI;

typedef enum
{
  DMA_MUX_RD = 0x1, /* enum as a bit map RX -> RD PERi to MEM */
  DMA_MUX_WR = 0x2, /* MEM to PERI -> TX */
  DMA_MUX_BOTH = (DMA_MUX_RD | DMA_MUX_WR),
} DMR_MUX_DIRECT;

typedef struct dmac_mux_param
{
  DMR_REQ_PERI port;
  u64 start_addr;
  u64 len;
  DMR_MUX_DIRECT direct;
} dmac_mux_param_t;

typedef union {
  struct
  {
    volatile u32 wr_hdsk : 8;         /* RW */
    volatile u32 updated_wr_hdsk : 8; /* RO write */
    volatile u32 rd_hdsk : 8;         /* RW */
    volatile u32 updated_rd_hdsk : 8; /* RO wrtie */
  };
  volatile u32 v;
} dmac_mux_ch_t;

typedef union {
  struct
  {
    volatile u32 en : 1; /* DMA mux enable mux. */
    volatile u32 rsvd : 31;
  };
  volatile u32 v;
} dmac_mux_en_t;

typedef struct
{
  volatile dmac_mux_ch_t ch[DMAC_MUX_CHANN_NUMB];
  volatile dmac_mux_en_t en_reg;
} dmac_mux_t;

/* Auto calculate hand shaking number for address. */
int get_dma_hs_id(u64 addr, DMR_MUX_DIRECT direct);

#endif
