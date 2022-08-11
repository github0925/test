/*
 * res.c
 *
 * Copyright (c) 2019 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: Domain resource management source file.
 *
 * Revision History:
 * -----------------
 */
#include "chip_res.h"
#include "res.h"

/* Generated resources of this domain. */
#include "domain_res.h"
#include "paddr_calc_base.h"

#define MPC_MAX_NUM 27
#define PPC_MAX_NUM 14
#define RPC_MAX_NUM 3

#define SCR_INDEX 512
#define RSTGEN_INDEX 1024
#define IOMUXC_INDEX 1280
#define RPC_ADDR_OFFSET 0x100000

#if EMULATION_PLATFORM_FPGA //temp solution for FPGA platform
#define FPGA_TIMER_BASE 0x30400000
#define FPGA_UART_BASE  0x30420000
#define FPGA_DC0_BASE   0x30440000
#define FPGA_DC1_BASE   0x30460000
#define FPGA_DC2_BASE   FPGA_DC0_BASE
#define FPGA_DC3_BASE   FPGA_DC0_BASE
#define FPGA_DC4_BASE   FPGA_DC0_BASE
#define FPGA_DP0_BASE   0x30500000
#define FPGA_DP1_BASE   0x30520000
#define FPGA_DP2_BASE   FPGA_DP0_BASE
#define FPGA_I2C_BASE   0x30480000
#define FPGA_I2C1_BASE  0x304e0000
#define FPGA_I2C2_BASE  0x305A0000
#define FPGA_I2C3_BASE  0x305C0000
#define FPGA_I2C4_BASE  0x305e0000
#define FPGA_TSGEN_BASE 0x304c0000
#define FPGA_CSI0_BASE  0x30540000
#define VPU_CODA988_REG_BASE 0x30620000
#define VPU_WAV412_REG_BASE 0x30600000
#define VPU_CODAJ12_REG_BASE 0x30600000
#define FPGA_I2S_SC1_BASE 0x30580000

#define TIMER_CATEGORY_IDX 31
#define UART_CATEGORY_IDX  33
#define DC_CATEGORY_IDX    43
#define APBMUX1_BASE_AP 0x30000000
#endif

/* Parse address for memory type resources*/
int32_t parse_paddr_mpc(uint32_t res_id, addr_t *paddr)
{
    uint32_t cat_id = (res_id >> 17) & 0x7F;
    if (cat_id >= MPC_CATEGORY_MAX) {
        return -1;
    }

    for (int i = 0; i < mem_info_init[cat_id]->res_num; i++) {
        if (res_id == mem_info_init[cat_id]->mem_info[i].res_id) {
            *paddr = mem_info_init[cat_id]->mem_info[i].paddr;
            return 0;
        }
    }

    return -1;
}

/* Parse address for peripheral type resources */
int32_t parse_paddr_ppc(uint32_t ppc_index, uint32_t slot_index, addr_t * paddr)
{
    uint32_t ppc_base_addr[PPC_MAX_NUM] = {APBMUX1_IP_BASE, APBMUX2_IP_BASE, APBMUX3_IP_BASE, APBMUX4_IP_BASE,
                                APBMUX5_IP_BASE, APBMUX6_IP_BASE, APBMUX7_IP_BASE, APBMUX8_IP_BASE,
                                APB_DDR_CFG_BASE, APB_SMMU_BASE, APB_CE2_REG_BASE, APB_SCR4K_SID_BASE,
                                APB_SCR4K_SSID_BASE, APB_CSSYS_BASE};
    uint32_t ppc_slot_size[PPC_MAX_NUM] = {APBMUX1_IP_SIZE, APBMUX2_IP_SIZE, APBMUX3_IP_SIZE, APBMUX4_IP_SIZE,
                                APBMUX5_IP_SIZE, APBMUX6_IP_SIZE, APBMUX7_IP_SIZE, APBMUX8_IP_SIZE,
                                APB_DDR_CFG_SIZE, APB_SMMU_SIZE, APB_CE2_REG_SIZE, APB_SCR4K_SID_SIZE,
                                APB_SCR4K_SSID_SIZE, APB_CSSYS_SIZE};

    *paddr = ppc_base_addr[ppc_index] + ppc_slot_size[ppc_index] * slot_index;

    return 0;
}

/* Parse address and index for register level resources */
int32_t parse_paddr_rpc(uint32_t rpc_index, uint32_t slot_index, addr_t * paddr, int32_t * index)
{
    if (slot_index >= 2046) {
        return -1;
    }

    uint32_t addr_offset = 0;

    if (slot_index >= IOMUXC_INDEX) {
        *index = slot_index - IOMUXC_INDEX;
        addr_offset = 5;
    }
    else if (slot_index >= RSTGEN_INDEX) {
        *index = slot_index - RSTGEN_INDEX;
        addr_offset = 4;
    }
    else if (slot_index >= SCR_INDEX) {
        *index = slot_index - SCR_INDEX;
        addr_offset = 2;
    }
    else {
        *index = slot_index;
    }

    switch (rpc_index) {
        case 0: //RPC_SOC
            *paddr = APB_RPC_SOC_BASE + addr_offset * RPC_ADDR_OFFSET;
            break;
        case 1: //RPC_SEC
            *paddr = APB_RPC_SEC_BASE + addr_offset * RPC_ADDR_OFFSET;
            break;
        case 2: //RPC_SAF
            *paddr = APB_RPC_SAF_BASE + addr_offset * RPC_ADDR_OFFSET;
            break;
        default:
            return -1;
        break;
    }

    return 0;
}

/* Parse resource ID to acquire base address and index of resource */
int32_t res_parse_info(uint32_t res_id, addr_t * paddr, int32_t * index)
{
    uint32_t rapc_type;
    uint32_t rapc_index;
    uint32_t slot_index;
    int32_t res = 0;

    *index = -1;

    rapc_type = res_id >> 30;
    rapc_index = (res_id >> 24) & 0x3F;
    slot_index = res_id & 0xFFF;

    switch (rapc_type) {
        case 0: //MPC
            if (rapc_index >= MPC_MAX_NUM) {
                return -1;
            }

            res = parse_paddr_mpc(res_id, paddr);
            break;
        case 1: //PPC
            if (rapc_index >= PPC_MAX_NUM) {
                return -1;
            }

            *index = (res_id >> 12) & 0x1F;

#if EMULATION_PLATFORM_FPGA //temp solution for FPGA platform
            {
                uint32_t cat_id = (res_id >> 17) & 0x7F;

                if (TIMER_CATEGORY_IDX == cat_id) {
                    *paddr = FPGA_TIMER_BASE + APBMUX1_IP_BASE - APBMUX1_BASE_AP;
                }
                else if (UART_CATEGORY_IDX == cat_id) {
                    *paddr = FPGA_UART_BASE + APBMUX1_IP_BASE - APBMUX1_BASE_AP;
                }
                else if (DC_CATEGORY_IDX == cat_id) {
                    if (res_id == RES_DC_DC2) {
                        *paddr = FPGA_DC1_BASE + APBMUX1_IP_BASE - APBMUX1_BASE_AP;
                    }
                    else {
                        *paddr = FPGA_DC0_BASE + APBMUX1_IP_BASE - APBMUX1_BASE_AP;
                    }
                }
                else if (res_id == RES_DP_DP1 || res_id == RES_DP_DP3) {
                    *paddr = FPGA_DP0_BASE + APBMUX1_IP_BASE - APBMUX1_BASE_AP;
                }
                else if (res_id == RES_DP_DP2) {
                    *paddr = FPGA_DP1_BASE + APBMUX1_IP_BASE - APBMUX1_BASE_AP;
                }
                else if (res_id == RES_I2C_I2C1 || res_id == RES_I2C_I2C5) {
                    *paddr = FPGA_I2C1_BASE + APBMUX1_IP_BASE - APBMUX1_BASE_AP;
                }
                else if (res_id == RES_I2C_I2C2 || res_id == RES_I2C_I2C6) {
                    *paddr = FPGA_I2C2_BASE + APBMUX1_IP_BASE - APBMUX1_BASE_AP;
                }
                else if (res_id == RES_I2C_I2C3 || res_id == RES_I2C_I2C7) {
                    *paddr = FPGA_I2C3_BASE + APBMUX1_IP_BASE - APBMUX1_BASE_AP;
                }
                else if (res_id == RES_I2C_I2C4 || res_id == RES_I2C_I2C8) {
                    *paddr = FPGA_I2C4_BASE + APBMUX1_IP_BASE - APBMUX1_BASE_AP;
                }
                else if (res_id == RES_SYS_CNT_SYS_CNT_RO || res_id == RES_SYS_CNT_SYS_CNT_RW) {
                    *paddr = FPGA_TSGEN_BASE + APBMUX1_IP_BASE - APBMUX1_BASE_AP;
                }
                else if (res_id == RES_CSI_CSI1) {
                    *paddr = FPGA_CSI0_BASE + APBMUX1_IP_BASE - APBMUX1_BASE_AP;
                }
                else if (res_id == RES_VPU_VPU1) {
                    *paddr = VPU_WAV412_REG_BASE + APBMUX1_IP_BASE - APBMUX1_BASE_AP;
                }
                else if (res_id == RES_VPU_VPU2) {
                    *paddr = VPU_CODA988_REG_BASE + APBMUX1_IP_BASE - APBMUX1_BASE_AP;
                }
                else if (res_id == RES_MJPEG_MJPEG) {
                    *paddr = VPU_CODAJ12_REG_BASE + APBMUX1_IP_BASE - APBMUX1_BASE_AP;
                }
                else if (res_id == RES_I2S_SC_I2S_SC1) {
                    *paddr = FPGA_I2S_SC1_BASE + APBMUX1_IP_BASE - APBMUX1_BASE_AP;
                }
                else {
#endif
            res = parse_paddr_ppc(rapc_index, slot_index, paddr);
#if EMULATION_PLATFORM_FPGA //temp solution for FPGA platform
                }
            }
#endif
            break;
        case 2: //RPC
            if (rapc_index >= PPC_MAX_NUM) {
                return -1;
            }

            res = parse_paddr_rpc(rapc_index, slot_index, paddr, index);
            break;
        case 3: //unprotected
            res = -1;
            for (int i = 0; i < mem_info_unprotected.res_num; i++) {
                if (res_id == mem_info_unprotected.mem_info[i].res_id) {
                    *paddr = mem_info_unprotected.mem_info[i].paddr;
                    res = 0;
                    break;
                }
            }
            break;
        default:
            return -1;
    }

    return res;
}

/* Get resource info by ID. */
const int32_t res_get_info_by_id(uint32_t res_id, addr_t * paddr, int32_t * index)
{
    uint32_t cat_id = (res_id >> 17) & 0x7F;

    if (cat_id >= (sizeof(g_res_cat) / sizeof(g_res_cat[0]))) {
        return -1;
    }

    if (NULL != g_res_cat[cat_id]) { //check avaibility of resource
        for (int i = 0; i < g_res_cat[cat_id]->res_num; i++) {
            if (g_res_cat[cat_id]->res_id[i] == res_id)
                return res_parse_info(res_id, paddr, index);
        }
    }

    return -1;
}

struct addr_map {
    paddr_t src;
    size_t size;
    paddr_t dst;
};

static struct addr_map addrmap_tab[] = {
    addrmap_def
};

/* Transform address from r core to a core */
paddr_t p2ap(paddr_t pa)
{
    int i = 0;
    struct addr_map *addr;
    for (i = 0; i < (int)(sizeof(addrmap_tab) / sizeof(addrmap_tab[0])); i++) {
        addr = &addrmap_tab[i];
        if (pa >= addr->src && (pa - addr->src) < addr->size) {
            return (pa - addr->src + addr->dst);
        }
    }
    return pa;
}

/* Transform address from a core to r core */
paddr_t ap2p(paddr_t va)
{
    int i = 0;
    struct addr_map *addr;
    for (i = 0; i < (int)(sizeof(addrmap_tab) / sizeof(addrmap_tab[0])); i++) {
        addr = &addrmap_tab[i];
        if (va >= addr->dst && (va - addr->dst) < addr->size) {
            return (va - addr->dst + addr->src);
        }
    }
    return va;
}
