/********************************************************
 *          Copyright(c) 2018   Semidrive               *
 ********************************************************/

#ifndef __SOC_SEC_H__
#define __SOC_SEC_H__

#define MB_CPU_SELF         MB_CPU_CR5_SEC
#define MB_CPU_FRD          MB_CPU_CR5_SAF
#define MB_MSG_ID_TX            1
#define MB_MSG_ID_RX            0

#include "__regs_base.h"

#define TCM_SYSTEM_ADDR        SEC_TCM_SYSTEM_ADDR

#define IRAM_BASE  IRAM2_BASE
#define IRAM_SIZE  (0x40000u * 3)
#define IRAM_END    (IRAM_BASE + IRAM_SIZE - 1)

#define CE_SMEM_BASE    CE2_MEM_BASE

#define FUSE_ACC_DOM    FUSE_ACC_AP

#define USBSS_TB_CTRL_BASE_ADDR (APB_TB_CTRL_BASE + 0xfc00)

#define SOC_GET_ROMC_BASE()    APB_ROMC2_BASE

#define SCR_BASE    APB_SCR_SEC_BASE

#define CRYPTO_ENG  CRYPTO_ENG2

#define FUSE_WDOG_EN()  FUSE_SEC_WDOG_EN()

void fw_assign_sec_to_dom(uint32_t did);
uint32_t fw_get_sec_dom(void);
U32 soc_get_bt_pin_prvsn(void);

#define EIC_BASE    APB_EIC_SEC_BASE
#define EIC_EN_ID   48
#define SEM_BASE    APB_SEM2_BASE

#define CE_SERAM_SIZE_in_KB     64
#define CE_SESRAM_SASIZE_in_KB    32

#define CKGEN_RBASE     APB_CKGEN_SEC_BASE
#define OSPI_CLK_SLICE_ID   21

#define SOC_SEC_MUX2_IO_2_L16_BIT(io)    \
    (SCR_SEC_GPIO_MUX2_GPIO_SEL_15_0_L16_START_BIT +\
      ((io) / 16 * 32 + (io) % 16))
#define SOC_SEC_GPIO_IO_START   48
#define SEC_GPIO_IO(io)    (SOC_SEC_GPIO_IO_START + (io))

#define SOC_IS_PEER_ACCESSIBLE()     true

#define FUSE_JUMP_ENTRY_OVERRIDE_EN() FUSE_SEC_JUMP_ENTRY_OVERRIDE_EN()
#define FUSE_JUMP_ENTRY_OVERRIDE_VAL() FUSE_SEC_JUMP_ENTRY_OVERRIDE_VAL()

#define UART_ROOT_CLK_FREQ  60000000u

#define GIC_RBASE   GIC2_BASE

#define CE2_RBASE   CE2_REG_BASE

#define MAX_INTERRUPT_NUM  IRQ_GIC2_MAX_INT_NUM

#define MB_ID_THIS_CPU   MB_CPU_CR5_SEC
#define APP_VECTOR_TBL  0x00140000

void sec_soc_vector_init(void);

#endif
