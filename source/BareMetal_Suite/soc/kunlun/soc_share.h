/********************************************************
 *          Copyright(c) 2018   Semidrive               *
 ********************************************************/

#ifndef __SOC_SHARE_H__
#define __SOC_SHARE_H__

#define BM_BT_PIN_HANDOVER_TO_SEC   (0x01u << 4)

typedef enum {
    /* bit2-0 */
    BT_PIN_eMMC1 = 1,
    BT_PIN_eMMC2 = 2,
    BT_PIN_SD3 = 3,
    BT_PIN_OSPI2 = 4,
    BT_PIN_PCIE = 6,

    BT_PIN_USB = 8,
    BT_PIN_DEBUG = 15,

} bt_pin_sec_e;

#define WDOG_TMOUT_us_to_CLKs(us)   (us)

#define SOC_RPC_REG_ADDR_MAP(a)     ((a) << 10)
#define SOC_IOMUX_REG_MAP(a)        SOC_RPC_REG_ADDR_MAP(a)
#define SOC_CKGEN_REG_MAP(a)        SOC_RPC_REG_ADDR_MAP(a)
#define SOC_RSTGEN_REG_MAP(a)       SOC_RPC_REG_ADDR_MAP(a)
#define SOC_SCR_REG_MAP(a)          SOC_RPC_REG_ADDR_MAP(a)

void soc_remap_to_zero(U32 addr);

#define SOC_FA_CFG_FUSE_BANK    26u

typedef enum {
    DBG_CPU_CR5_SAF,
    DBG_CPU_CR5_SEC,
    DBG_CPU_CR5_MP,
    DBG_CPU_AP1,
    DBG_CPU_AP2,
    DBG_CPU_VDSP,
    DBG_CPU_ADSP,
    DBG_GPU1,
    DBG_GPU2,
    DBG_CSSYS,
    DBG_CPU_ID_MAX,
} dbg_cpu_id_e;

#endif  /* __SOC_SHARE_H__ */
