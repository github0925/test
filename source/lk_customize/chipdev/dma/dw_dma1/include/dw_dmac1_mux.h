/*
 * dw_dmac1_mux.h
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
#ifndef __DW_DMAC1_MUX_H
#define __DW_DMAC1_MUX_H
#include <__regs_base.h>
#include <sys/types.h>
#define DMAC1_MUX_CHANN_NUMB (8)
typedef enum
{
  DMA1_MUX_CAN1 = 0,
  DMA1_MUX_CAN2 = 1,
  DMA1_MUX_CAN3 = 2,
  DMA1_MUX_CAN4 = 3,
  DMA1_MUX_ENET1_REQ0 = 4,
  DMA1_MUX_ENET1_REQ1 = 5,
  DMA1_MUX_ENET1_REQ2 = 6,
  DMA1_MUX_ENET1_REQ3 = 7,
  DMA1_MUX_I2C1_TX = 8,
  DMA1_MUX_I2C1_RX = 9,
  DMA1_MUX_I2C2_TX = 10,
  DMA1_MUX_I2C2_RX = 11,
  DMA1_MUX_I2C3_TX = 12,
  DMA1_MUX_I2C3_RX = 13,
  DMA1_MUX_I2C4_TX = 14,
  DMA1_MUX_I2C4_RX = 15,
  DMA1_MUX_I2S_SC1_RX = 16,
  DMA1_MUX_I2S_SC1_TX = 17,
  DMA1_MUX_I2S_SC2_RX = 18,
  DMA1_MUX_I2S_SC2_TX = 19,
  DMA1_MUX_OSPI1_RX = 20,
  DMA1_MUX_OSPI1_TX = 21,
  DMA1_MUX_PWM1_REQ = 22,
  DMA1_MUX_PWM2_REQ = 23,
  DMA1_MUX_SPI1_RX = 24,
  DMA1_MUX_SPI1_TX = 25,
  DMA1_MUX_SPI2_RX = 26,
  DMA1_MUX_SPI2_TX = 27,
  DMA1_MUX_SPI3_RX = 28,
  DMA1_MUX_SPI3_TX = 29,
  DMA1_MUX_SPI4_RX = 30,
  DMA1_MUX_SPI4_TX = 31,
  DMA1_MUX_TIMER1_REQA = 32,
  DMA1_MUX_TIMER1_REQB = 33,
  DMA1_MUX_TIMER1_REQC = 34,
  DMA1_MUX_TIMER1_REQD = 35,
  DMA1_MUX_TIMER1_OVF = 36,
  DMA1_MUX_TIMER2_REQA = 37,
  DMA1_MUX_TIMER2_REQB = 38,
  DMA1_MUX_TIMER2_REQC = 39,
  DMA1_MUX_TIMER2_REQD = 40,
  DMA1_MUX_TIMER2_OVF = 41,
  DMA1_MUX_UART1_TX = 42,
  DMA1_MUX_UART1_RX = 43,
  DMA1_MUX_UART2_TX = 44,
  DMA1_MUX_UART2_RX = 45,
  DMA1_MUX_UART3_TX = 46,
  DMA1_MUX_UART3_RX = 47,
  DMA1_MUX_UART4_TX = 48,
  DMA1_MUX_UART4_RX = 49,
  DMA1_MUX_UART5_TX = 50,
  DMA1_MUX_UART5_RX = 51,
  DMA1_MUX_UART6_TX = 52,
  DMA1_MUX_UART6_RX = 53,
  DMA1_MUX_UART7_TX = 54,
  DMA1_MUX_UART7_RX = 55,
  DMA1_MUX_UART8_TX = 56,
  DMA1_MUX_UART8_RX = 57,
  DMA1_MUX_SIZE,
  DMA1_MUX_ERR_NO = 144,
  DMA1_MUX_EXTRA_PORT = 255,
} DMR1_REQ_PERI;

typedef enum
{
  DMA1_MUX_RD = 0x1, /* enum as a bit map RX -> RD PERi to MEM */
  DMA1_MUX_WR = 0x2, /* MEM to PERI -> TX */
  DMA1_MUX_BOTH = (DMA1_MUX_RD | DMA1_MUX_WR),
} DMR1_MUX_DIRECT;

typedef struct dmac1_mux_param
{
  DMR1_REQ_PERI port;
  u64 start_addr;
  u64 len;
  DMR1_MUX_DIRECT direct;
} dmac1_mux_param_t;

typedef union {
  struct
  {
    volatile u32 wr_hdsk : 8;         /* RW */
    volatile u32 updated_wr_hdsk : 8; /* RO write */
    volatile u32 rd_hdsk : 8;         /* RW */
    volatile u32 updated_rd_hdsk : 8; /* RO write */
  };
  volatile u32 v;
} dmac1_mux_ch_t;

typedef union {
  struct
  {
    volatile u32 en : 1; /* DMA mux enable mux. */
    volatile u32 rsvd : 31;
  };
  volatile u32 v;
} dmac1_mux_en_t;

typedef struct
{
  volatile dmac1_mux_ch_t ch[DMAC1_MUX_CHANN_NUMB];
  volatile dmac1_mux_en_t en_reg;
} dmac1_mux_t;

/* Auto calculate hand shaking number for address. */
int get_dma1_hs_id(u64 addr, DMR1_MUX_DIRECT direct);

#endif
