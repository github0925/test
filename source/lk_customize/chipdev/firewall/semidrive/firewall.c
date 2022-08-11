/*
* Copyright (c) 2019 Semidrive Semiconductor.
* All rights reserved.
*
*/

#include <stdio.h>
#include <reg.h>
#include <assert.h>

#include <lib/sd_sysdef.h>
#include <firewall.h>
#include <mac_reg.h>
#include <mpc_reg.h>
#include <ppc_reg.h>
#include <rpc_reg.h>
#include <mpc.h>
#include <ppc.h>
#include <rpc.h>

//res_id 32bits, bit31~bit0
//bit31~bit30 RAPC type, bit29~bit24 RAPC index, bit23~bit17 category index, bit16~bit12 physical index, bit11~bit0 range/slot/register index
#define RES_ID_RAPC_TYPE_OFFSET 30
#define RES_ID_RAPC_INDEX_OFFSET 24
#define DOM_PER_REG_ONE_DOMAIN_BIT_LEN 4
#define SEC_PER_REG_ONE_DOMAIN_BIT_LEN 8
#define PRI_PER_REG_ONE_DOMAIN_BIT_LEN 8

#define clrbit(x,y)  x&=~(1<<y)

int scr_bit_get(addr_t base_addr, scr_reg_type_t reg_type, uint32_t bit_offset)
{
    uint32_t reg_offset;

    switch (reg_type) {
        case SCR_RW:
            reg_offset = SCR_RW_OFFSET;
            break;

        case SCR_RO:
            reg_offset = SCR_RO_OFFSET;
            break;

        case SCR_L16:
            reg_offset = SCR_L16_OFFSET;
            break;

        case SCR_L31:
            reg_offset = SCR_L31_OFFSET;
            break;

        case SCR_R16W16:
            reg_offset = SCR_R16W16_OFFSET;
            break;

        default:
            return -1;
    }

    uint32_t val = readl(base_addr + ((reg_offset + (bit_offset / 32) * 4) << 10));

    return ((val >> (bit_offset % 32)) & 0x1) ;
}

/* disable firewall response when illegally access, keep silence */
void fw_resp_disable(void)
{
    /* bit 1360 - 1375 for mac, mpc_adsp, mpc_ce2, mpc_iram2,
     * mpc_iram3, mpc_iram4, mpc_sec_platform, mpc_mp_platform,
     * mpc_romc2, mpc_ospi2, mpc_gic2, mpc_gic3, mpc_mu,
     * ppc_apbmux2, ppc_apbmu2, ppc_ce2
     */
    writel((0xFFFF << 16), SCR_BASE_ADDR + ((SCR_L16_OFFSET + (1360 / 32) * 4) << 10));

    /* bit 1392 - 1401 for ppc_apbmux4, ppc_apbmux5, ppc_apbmux6,
     * ppc_apbmux7b, ppc_apbmux8, ppc_ddr, ppc_tcu, ppc_cssys,
     * ppc_scr4k_sid, ppc_scr4k_ssid
     */
    writel((0x3FF << 16), SCR_BASE_ADDR + ((SCR_L16_OFFSET + (1392 / 32) * 4) << 10));

    /* bit 1424 - 1434 for mpc_pcie1, mpc_pcie2, mpc_ddr,
     * mpc_vdsp, mpc_gic4, mpc_gic5, mpc_gpu1, mpc_gpu2,
     * rpc_sec, rpc_soc, mpc_iram5
     */
    writel((0x7FF << 16), SCR_BASE_ADDR + ((SCR_L16_OFFSET + (1424 / 32) * 4) << 10));
}

/* default init
 * firewall action when access illegally, silence in product mode
 * check firewall enable efuse?
 * mac global cfg, enable domain check and set secure core as res mgr, then lock this byte
 * resource manager cfg
 */
int default_cfg(void)
{
    int prod_mode;
    uint32_t value;

    //check product mode, if true then set to silent action
    prod_mode = scr_bit_get(SCR_BASE_ADDR, SCR_RO, SCR_PROD_ENABLE_RO_START_BIT);

    if (1 == prod_mode) {
        fw_resp_disable();
    }

    // mac global config
    value = readl(GLB_MAC_BASE_ADDR + MAC_GLB_CTL);
    value = reg_value(1, value, MAC_GLB_CTL_DOM_CFG_LOCK_SHIFT, MAC_GLB_CTL_DOM_CFG_LOCK_MASK);
    value = reg_value(1, value, MAC_GLB_CTL_PERCK_DIS_LOCK_SHIFT, MAC_GLB_CTL_PERCK_DIS_LOCK_MASK);
    value = reg_value(1, value, MAC_GLB_CTL_DOM_PRO_LOCK_SHIFT, MAC_GLB_CTL_DOM_PRO_LOCK_MASK);
    value = reg_value(1, value, MAC_GLB_CTL_DOM_PRO_EN_SHIFT, MAC_GLB_CTL_DOM_PRO_EN_MASK);
    writel(value, GLB_MAC_BASE_ADDR + MAC_GLB_CTL);
    writel(value, RPC_BASE_ADDR_CKGEN_SOC + RPC_MAC_GLB_CTL_OFFSET + MAC_GLB_CTL);
    writel(value, RPC_BASE_ADDR_SEC + RPC_MAC_GLB_CTL_OFFSET + MAC_GLB_CTL);
#ifdef ENABLE_SAF_FIREWALL
    writel(value, RPC_BASE_ADDR_SAF + RPC_MAC_GLB_CTL_OFFSET + MAC_GLB_CTL);
#endif

    //res manager master config, set secure core as res manager
    writel(0x2, GLB_MAC_BASE_ADDR + MAC_RES_MGR_MA0);
    writel(0x2, RPC_BASE_ADDR_CKGEN_SOC + RPC_MAC_GLB_CTL_OFFSET + MAC_RES_MGR_MA0);
    writel(0x2, RPC_BASE_ADDR_SEC + RPC_MAC_GLB_CTL_OFFSET + MAC_RES_MGR_MA0);
#ifdef ENABLE_SAF_FIREWALL
    writel(0x2, RPC_BASE_ADDR_SAF + RPC_MAC_GLB_CTL_OFFSET + MAC_RES_MGR_MA0);
#endif

    //res manager config
    value = reg_value(1, 0, MAC_RES_MGR_MID_LOCK_SHIFT, MAC_RES_MGR_MID_LOCK_MASK);
    value = reg_value(1, value, MAC_RES_MGR_MID_EN_SHIFT, MAC_RES_MGR_MID_EN_MASK);
    value = reg_value(1, value, MAC_RES_MGR_PRI_PER_LOCK_SHIFT, MAC_RES_MGR_PRI_PER_LOCK_MASK);
    value = reg_value(1, value, MAC_RES_MGR_SEC_PER_LOCK_SHIFT, MAC_RES_MGR_SEC_PER_LOCK_MASK);
    value = reg_value(1, value, MAC_RES_MGR_DID_LOCK_SHIFT, MAC_RES_MGR_DID_LOCK_MASK);
    value = reg_value(0, value, MAC_RES_MGR_DID_SHIFT, MAC_RES_MGR_DID_MASK);
    value = reg_value(1, value, MAC_RES_MGR_DID_EN_SHIFT, MAC_RES_MGR_DID_EN_MASK);
    value = reg_value(1, value, MAC_RES_MGR_RES_MGR_EN_LOCK_SHIFT, MAC_RES_MGR_RES_MGR_EN_LOCK_MASK);
    value = reg_value(1, value, MAC_RES_MGR_RES_MGR_EN_SHIFT, MAC_RES_MGR_RES_MGR_EN_MASK);
    writel(value, GLB_MAC_BASE_ADDR + MAC_RES_MGR);
    writel(value, RPC_BASE_ADDR_CKGEN_SOC + RPC_MAC_GLB_CTL_OFFSET + MAC_RES_MGR);
    writel(value, RPC_BASE_ADDR_SEC + RPC_MAC_GLB_CTL_OFFSET + MAC_RES_MGR);
#ifdef ENABLE_SAF_FIREWALL
    writel(value, RPC_BASE_ADDR_SAF + RPC_MAC_GLB_CTL_OFFSET + MAC_RES_MGR);
#endif

    return 0;
}

int write_cfg_to_fw(uint32_t fw_cfg_count, firewall_cfg_t* fw_cfg)
{
    uint32_t orig_val;

    for (uint32_t i = 0; i < fw_cfg_count; i++) {
        if (fw_cfg->overwrite) {
            writel(fw_cfg->reg_val, fw_cfg->reg_addr);
        }
        else {
            orig_val = readl(fw_cfg->reg_addr);
            writel(fw_cfg->reg_val | orig_val, fw_cfg->reg_addr);
        }

        fw_cfg++;
    }

    return 0;
}

/* check step 1:
 * check values of firewall registers with firewall configuration
 */
int fw_cfg_check(uint32_t cfg_count, firewall_cfg_t* fw_cfg)
{
    uint32_t read_val;
    uint32_t value;
    uint32_t prod_mode;

    for (uint32_t i = 0; i < cfg_count; i++) {
        read_val = readl(fw_cfg->reg_addr);

        if ((read_val & (fw_cfg->reg_val)) != fw_cfg->reg_val) {
            dprintf(INFO, "write 0x%x to 0x%lx with %s fail, register value 0x%x\n",
                    fw_cfg->reg_val, fw_cfg->reg_addr, fw_cfg->overwrite ? "overwrite" : "or", (read_val & (fw_cfg->reg_val)));
        }

        fw_cfg++;
    }

    //default config check
    value = reg_value(1, 0x10, MAC_GLB_CTL_DOM_CFG_LOCK_SHIFT, MAC_GLB_CTL_DOM_CFG_LOCK_MASK);
    value = reg_value(1, value, MAC_GLB_CTL_PERCK_DIS_LOCK_SHIFT, MAC_GLB_CTL_PERCK_DIS_LOCK_MASK);
    value = reg_value(1, value, MAC_GLB_CTL_DOM_PRO_LOCK_SHIFT, MAC_GLB_CTL_DOM_PRO_LOCK_MASK);
    value = reg_value(1, value, MAC_GLB_CTL_DOM_PRO_EN_SHIFT, MAC_GLB_CTL_DOM_PRO_EN_MASK);

    read_val = readl(GLB_MAC_BASE_ADDR + MAC_GLB_CTL);

    if (read_val != value) {
        dprintf(CRITICAL, "global reg wrong, addr: 0x%x, value: 0x%x, reg value: 0x%x\n",
                GLB_MAC_BASE_ADDR + MAC_GLB_CTL, value, read_val);
    }

    read_val = readl(RPC_BASE_ADDR_CKGEN_SOC + RPC_MAC_GLB_CTL_OFFSET + MAC_GLB_CTL);

    if (read_val != value) {
        dprintf(CRITICAL, "global reg wrong, addr: 0x%x, value: 0x%x, reg value: 0x%x\n",
                RPC_BASE_ADDR_CKGEN_SOC + RPC_MAC_GLB_CTL_OFFSET + MAC_GLB_CTL, value, read_val);
    }

    read_val = readl(RPC_BASE_ADDR_SEC + RPC_MAC_GLB_CTL_OFFSET + MAC_GLB_CTL);

    if (read_val != value) {
        dprintf(CRITICAL, "global reg wrong, addr: 0x%x, value: 0x%x, reg value: 0x%x\n",
                RPC_BASE_ADDR_SEC + RPC_MAC_GLB_CTL_OFFSET + MAC_GLB_CTL, value, read_val);
    }

#ifdef ENABLE_SAF_FIREWALL
    read_val = readl(RPC_BASE_ADDR_SAF + RPC_MAC_GLB_CTL_OFFSET + MAC_GLB_CTL);

    if (read_val != value) {
        dprintf(CRITICAL, "global reg wrong, addr: 0x%x, value: 0x%x, reg value: 0x%x\n",
                RPC_BASE_ADDR_SAF + RPC_MAC_GLB_CTL_OFFSET + MAC_GLB_CTL, value, read_val);
    }
#endif

    //res manager config
    value = reg_value(1, 0, MAC_RES_MGR_MID_LOCK_SHIFT, MAC_RES_MGR_MID_LOCK_MASK);
    value = reg_value(1, value, MAC_RES_MGR_MID_EN_SHIFT, MAC_RES_MGR_MID_EN_MASK);
    value = reg_value(1, value, MAC_RES_MGR_PRI_PER_LOCK_SHIFT, MAC_RES_MGR_PRI_PER_LOCK_MASK);
    value = reg_value(1, value, MAC_RES_MGR_SEC_PER_LOCK_SHIFT, MAC_RES_MGR_SEC_PER_LOCK_MASK);
    value = reg_value(1, value, MAC_RES_MGR_DID_LOCK_SHIFT, MAC_RES_MGR_DID_LOCK_MASK);
    value = reg_value(0, value, MAC_RES_MGR_DID_SHIFT, MAC_RES_MGR_DID_MASK);
    value = reg_value(1, value, MAC_RES_MGR_DID_EN_SHIFT, MAC_RES_MGR_DID_EN_MASK);
    value = reg_value(1, value, MAC_RES_MGR_RES_MGR_EN_LOCK_SHIFT, MAC_RES_MGR_RES_MGR_EN_LOCK_MASK);
    value = reg_value(1, value, MAC_RES_MGR_RES_MGR_EN_SHIFT, MAC_RES_MGR_RES_MGR_EN_MASK);
    read_val = readl(GLB_MAC_BASE_ADDR + MAC_RES_MGR);

    if (read_val != value) {
        dprintf(CRITICAL, "global reg wrong, addr: 0x%x, value: 0x%x, reg value: 0x%x\n",
                GLB_MAC_BASE_ADDR + MAC_RES_MGR, value, read_val);
    }

    read_val = readl(RPC_BASE_ADDR_CKGEN_SOC + RPC_MAC_GLB_CTL_OFFSET + MAC_RES_MGR);

    if (read_val != value) {
        dprintf(CRITICAL, "global reg wrong, addr: 0x%x, value: 0x%x, reg value: 0x%x\n",
                RPC_BASE_ADDR_CKGEN_SOC + RPC_MAC_GLB_CTL_OFFSET + MAC_RES_MGR, value, read_val);
    }

    read_val = readl(RPC_BASE_ADDR_SEC + RPC_MAC_GLB_CTL_OFFSET + MAC_RES_MGR);

    if (read_val != value) {
        dprintf(CRITICAL, "global reg wrong, addr: 0x%x, value: 0x%x, reg value: 0x%x\n",
                RPC_BASE_ADDR_SEC + RPC_MAC_GLB_CTL_OFFSET + MAC_RES_MGR, value, read_val);
    }

#ifdef ENABLE_SAF_FIREWALL
    read_val = readl(RPC_BASE_ADDR_SAF + RPC_MAC_GLB_CTL_OFFSET + MAC_RES_MGR);

    if (read_val != value) {
        dprintf(CRITICAL, "global reg wrong, addr: 0x%x, value: 0x%x, reg value: 0x%x\n",
                RPC_BASE_ADDR_SAF + RPC_MAC_GLB_CTL_OFFSET + MAC_RES_MGR, value, read_val);
    }
#endif

    //res manager master config, set secure core as res manager
    value = 0x2;
    read_val = readl(GLB_MAC_BASE_ADDR + MAC_RES_MGR_MA0);

    if (read_val != value) {
        dprintf(CRITICAL, "global reg wrong, addr: 0x%x, value: 0x%x, reg value: 0x%x\n",
                GLB_MAC_BASE_ADDR + MAC_RES_MGR_MA0, value, read_val);
    }

    read_val = readl(RPC_BASE_ADDR_CKGEN_SOC + RPC_MAC_GLB_CTL_OFFSET + MAC_RES_MGR_MA0);

    if (read_val != value) {
        dprintf(CRITICAL, "global reg wrong, addr: 0x%x, value: 0x%x, reg value: 0x%x\n",
                RPC_BASE_ADDR_CKGEN_SOC + RPC_MAC_GLB_CTL_OFFSET + MAC_RES_MGR_MA0, value, read_val);
    }

    read_val = readl(RPC_BASE_ADDR_SEC + RPC_MAC_GLB_CTL_OFFSET + MAC_RES_MGR_MA0);

    if (read_val != value) {
        dprintf(CRITICAL, "global reg wrong, addr: 0x%x, value: 0x%x, reg value: 0x%x\n",
                RPC_BASE_ADDR_SEC + RPC_MAC_GLB_CTL_OFFSET + MAC_RES_MGR_MA0, value, read_val);
    }

#ifdef ENABLE_SAF_FIREWALL
    read_val = readl(RPC_BASE_ADDR_SAF + RPC_MAC_GLB_CTL_OFFSET + MAC_RES_MGR_MA0);

    if (read_val != value) {
        dprintf(CRITICAL, "global reg wrong, addr: 0x%x, value: 0x%x, reg value: 0x%x\n",
                RPC_BASE_ADDR_SAF + RPC_MAC_GLB_CTL_OFFSET + MAC_RES_MGR_MA0, value, read_val);
    }
#endif

    //scr register check
    prod_mode = scr_bit_get(SCR_BASE_ADDR, SCR_RO, SCR_PROD_ENABLE_RO_START_BIT);

    if (1 == prod_mode) {
        value = 0xFFFF << 16;
        read_val = readl(SCR_BASE_ADDR + ((SCR_L16_OFFSET + (1360 / 32) * 4) << 10));

        if (read_val != value) {
            dprintf(CRITICAL, "disable fw resp fail, reg offset addr: 0x%x, value: 0x%x, reg value: 0x%x\n",
                    (SCR_L16_OFFSET + (1360 / 32) * 4), value, read_val);
        }

        value = 0x3FF << 16;
        read_val = readl(SCR_BASE_ADDR + ((SCR_L16_OFFSET + (1392 / 32) * 4) << 10));

        if (read_val != value) {
            dprintf(CRITICAL, "disable fw resp fail, reg offset addr: 0x%x, value: 0x%x, reg value: 0x%x\n",
                    (SCR_L16_OFFSET + (1392 / 32) * 4), value, read_val);
        }

        value = 0x7FF << 16;
        read_val = readl(SCR_BASE_ADDR + ((SCR_L16_OFFSET + (1424 / 32) * 4) << 10));

        if (read_val != value) {
            dprintf(CRITICAL, "disable fw resp fail, reg offset addr: 0x%x, value: 0x%x, reg value: 0x%x\n",
                    (SCR_L16_OFFSET + (1424 / 32) * 4), value, read_val);
        }
    }

    return 0;
}

static uint32_t did_output = 0x10;
int format_output(uint32_t did, uint32_t ip_type, uint32_t ip_index, uint32_t slot_index, addr_t addr_start, addr_t addr_end)
{
    if (did_output != did)  {
        dprintf(CRITICAL, "Resources of domain 0x%x as following:\n", did);
        did_output = did;
    }

    switch (ip_type) {
        case IP_MAC:
            dprintf(CRITICAL, "    Master %d\n", ip_index);
            break;

        case IP_MPC:
            dprintf(CRITICAL, "    MPC %d, start address: 0x%lx, end address: 0x%lx\n", ip_index, addr_start, addr_end);
            break;

        case IP_PPC:
            if (addr_end <= 0) {
                dprintf(CRITICAL, "    PPC %d, slot %d\n", ip_index, slot_index);
            }
            else {
                dprintf(CRITICAL, "    PPC %d, start address: 0x%lx, end address: 0x%lx\n", ip_index, addr_start, addr_end);
            }

            break;

        case IP_RPC:
            dprintf(CRITICAL, "    RPC %d, register: %d\n", ip_index, slot_index);
            break;

        default:
            return -1;
    }

    return 0;
}

/*  */
int res_assign_output(void)
{
    uint32_t dom_used = 0;
    uint32_t dom_id_used[DOM_MAX_COUNT];
    uint32_t read_val;
    uint32_t i, j, k;
    uint32_t addr_offset;
    addr_t addr_start, addr_end;

    //acquire used domains
    for (uint32_t i = 0; i < DOM_MAX_COUNT; i++) {
        read_val = readl(GLB_MAC_BASE_ADDR + MAC_DOM_OWN_(i));

        if (read_val & MAC_DOM_OWN_EN_MASK) {
            dom_id_used[dom_used] = i;
            dom_used++;
        }
    }

    //will do traversal by times of dom_used
    uint32_t mpc_ranges[MPC_COUNT] = {AXI2AHB_SEC_RANGE_COUNT, ADSP1_RANGE_COUNT, DMA1_RANGE_COUNT,
                                      VDSP_RANGE_COUNT, CE1_RANGE_COUNT, CE2_RANGE_COUNT, IRAM1_RANGE_COUNT,
                                      IRAM2_RANGE_COUNT, IRAM3_RANGE_COUNT, SEC_PLATFORM_RANGE_COUNT, MP_PLATFORM_RANGE_COUNT,
                                      ROMC2_RANGE_COUNT, OSPI1_RANGE_COUNT, OSPI2_RANGE_COUNT, GIC2_RANGE_COUNT,
                                      GIC3_RANGE_COUNT, GIC4_RANGE_COUNT, GIC5_RANGE_COUNT, PCIE1_RANGE_COUNT,
                                      PCIE2_RANGE_COUNT, DDR_MEM_RANGE_COUNT, RESERVED2, GPU1_RANGE_COUNT,
                                      GPU2_RANGE_COUNT, MU_RANGE_COUNT, IRAM4_RANGE_COUNT, IRAM5_RANGE_COUNT
                                     };
    uint32_t ppc_slots[PPC_COUNT] = {MUX1_SLAVE_SLOT_COUNT, MUX2_SLAVE_SLOT_COUNT, MUX3_SLAVE_SLOT_COUNT, MUX4_SLAVE_SLOT_COUNT,
                                     MUX5_SLAVE_SLOT_COUNT, MUX6_SLAVE_SLOT_COUNT, MUX7_SLAVE_SLOT_COUNT, MUX8_SLAVE_SLOT_COUNT,
                                     DDR_CFG_SLAVE_SLOT_COUNT, SMMU_SLAVE_SLOT_COUNT, CE2_VIRT_SLOT_COUNT, SCR4K_SID_SLOT_COUNT,
                                     SCR4K_SSID_SLOT_COUNT, CSSYS_SLOT_COUNT
                                    };
    uint32_t ppc_range_enable[PPC_COUNT] = {1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 0, 0, 0};
    uint32_t rpc_base_addr[3] = {RPC_BASE_ADDR_CKGEN_SOC, RPC_BASE_ADDR_SEC, RPC_BASE_ADDR_SAF};

#ifndef ENABLE_SAF_FIREWALL
    uint32_t saf_mpc[] = {2, 4, 6, 12};
    uint32_t saf_ppc[] = {0, 6};
#endif

    for (i = 0; i < dom_used; i++) {
        //check master
        for (j = 0; j < MAC_MASTER_COUNT; j++) {
            read_val = readl(GLB_MAC_BASE_ADDR + MAC_MDA_(j));

            if ((read_val & 0xF) == dom_id_used[i]) {
                format_output(dom_id_used[i], IP_MAC, j, 0, 0, 0);
            }
        }

        addr_offset = 0;

        //check mpc
        for (j = 0; j < MPC_COUNT; j++) {

#ifndef ENABLE_SAF_FIREWALL //bypass safety firewall check
            for (int m = 0; m < 4; m++) {
                if (j == saf_mpc[m]) {
                    goto NEXT_MPC;
                }
            }
#endif

            for (k = 0; k < mpc_ranges[j]; k++) {
                read_val = readl(MPC_DOM_(k) + addr_offset);

                if ((read_val >> 4 & 0x1) && ((read_val & 0xF) == dom_id_used[i])) {
                    read_val = readl(MPC_RGN_START_ADDR_(k) + addr_offset);
                    addr_start = (read_val & 0x3FFFFFFF) << 12;
                    read_val = readl(MPC_RGN_END_ADDR_(k) + addr_offset);
                    addr_end = (read_val & 0x3FFFFFFF) << 12;
                    format_output(dom_id_used[i], IP_MPC, j, k, addr_start, addr_end);
                }
            }

NEXT_MPC:
            addr_offset += MPC_ADDR_SIZE;
        }

        addr_offset = 0;

        //check ppc
        for (j = 0; j < PPC_COUNT; j++) {

#ifndef ENABLE_SAF_FIREWALL //bypass safety firewall check
            for (int m = 0; m < 2; m++) {
                if (j == saf_ppc[m]) {
                    goto NEXT_PPC;
                }
            }
#endif
            for (k = 0; k < ppc_slots[j]; k++) {
                read_val = readl(PPC_DOM_(k) + addr_offset);

                if ((read_val >> 4 & 0x1) && ((read_val & 0xF) == dom_id_used[i])) {
                    format_output(dom_id_used[i], IP_PPC, j, k, 0, 0);
                }
            }

            if (ppc_range_enable[j]) {
                for (k = 0; k < 16; k++) {
                    read_val = readl(PPC_DOM_(k + ppc_slots[j]) + addr_offset);

                    if ((read_val >> 4 & 0x1) && ((read_val & 0xF) == dom_id_used[i])) {
                        read_val = readl(PPC_RGN_START_ADDR_(k) + addr_offset);
                        addr_start = (read_val & 0x3FFFFFFF) << 12;
                        read_val = readl(PPC_RGN_END_ADDR_(k) + addr_offset);
                        addr_end = (read_val & 0x3FFFFFFF) << 12;
                        format_output(dom_id_used[i], IP_PPC, j, k + ppc_slots[j], addr_start, addr_end);
                    }
                }
            }

NEXT_PPC:
            addr_offset += PPC_ADDR_SIZE;
        }

        //check rpc
        uint32_t rpc_count = 3;

#ifndef ENABLE_SAF_FIREWALL //bypass safety firewall check
        rpc_count = 2;
#endif

        for (j = 0; j < rpc_count; j++) {
            for (k = 0; k < (256 * 8 - 2); k++) {
                read_val = readl(rpc_base_addr[j] + (k << 12));

                if (dom_id_used[i] == (read_val & 0xF)) {
                    if ((dom_id_used[i] == 0) && (((read_val >> 5) & 0x3) == 0)) { //special for domain 0
                        read_val = readl(rpc_base_addr[j] + RPC_DOM_PER0_(0));

                        if (0 == (read_val & 0x1)) {
                            continue;
                        }
                    }

                    format_output(dom_id_used[i], IP_RPC, j, k, 0, 0);
                }
            }
        }
    }

    return 0;
}

int share_resource(uint32_t res_id, uint32_t dom_id, bool shared)
{
    rapc_type_t rapc_type;
    uint32_t rapc_index;
    uint32_t slot_index;
    uint32_t value;
    uint32_t per_index;
    uint32_t offset;
    uint32_t reg_addr;

    //bit31~bit30 RAPC type, bit29~bit24 RAPC index, bit23~bit17 category index, bit16~bit12 physical index, bit11~bit0 range/slot/register index
    //-------------------------------------------------------------
    //|31|30|29|28|27|26|25|24|23|22|21|20|19|18|17|16|15|14|13|12|11|10|09|08|07|06|05|04|03|02|01|00|
    //|type |      index      |      category      |   physical   |     range/slot/register index     |
    //-------------------------------------------------------------
    rapc_type = (res_id >> RES_ID_RAPC_TYPE_OFFSET) & 0x3;
    rapc_index = (res_id >> RES_ID_RAPC_INDEX_OFFSET) & 0x3f;
    slot_index = res_id & 0xfff;

    //domain id max is 16, the mpc register has two bytes(64bits) to share res, DOM_PER0 and DOM_PER1
    //the first byte is domain 1~8 share info, the second byte is domain 9~16 share info
    //one domain use 4bit, bit0, enable, bit1~2 rw ro wo none, bit3 lock.detail:
    //-------------------------------------------------------------
    //|31|30|29|28|27|26|25|24|23|22|21|20|19|18|17|16|15|14|13|12|11|10|09|08|07|06|05|04|03|02|01|00|
    //|  domain8  |  domain7  |  domain6  |  domain5  |  domain4  |  domain3  |  domain2  |  domain1  |
    //-------------------------------------------------------------
    per_index = dom_id / 8;
    offset = dom_id % 8;

    if (dom_id > 0xf) {
        return -1;
    }

    switch (rapc_type) {
        case 0: //share_mpc_to_domain
            if (per_index == 0) {
                reg_addr = MPC_DOM_PER0_(slot_index) + MPC_ADDR_SIZE * rapc_index;
            }
            else {
                reg_addr = MPC_DOM_PER1_(slot_index) + MPC_ADDR_SIZE * rapc_index;
            }

            break;

        case 1: //share_ppc_to_domain
            if (per_index == 0) {
                reg_addr = PPC_DOM_PER0_(slot_index) + PPC_ADDR_SIZE * rapc_index;
            }
            else {
                reg_addr = PPC_DOM_PER1_(slot_index) + PPC_ADDR_SIZE * rapc_index;
            }

            break;

        default:
            return -1;
    }

    value = readl(reg_addr);

    if (shared) {
        value |= (0x7 << (offset * DOM_PER_REG_ONE_DOMAIN_BIT_LEN));
    }
    else {
        clrbit(value, (offset * DOM_PER_REG_ONE_DOMAIN_BIT_LEN));
        clrbit(value, ((offset * DOM_PER_REG_ONE_DOMAIN_BIT_LEN) + 1));
        clrbit(value, ((offset * DOM_PER_REG_ONE_DOMAIN_BIT_LEN) + 2));
    }

    writel(value, reg_addr);

    return 0;
}

int firewall_init(uint32_t cfg_count, firewall_cfg_t* fw_cfg, bool cfg_debug)
{
    write_cfg_to_fw(cfg_count, fw_cfg);

    default_cfg();

    if (cfg_debug) {
        //check step 1: check register with cfg value
        fw_cfg_check(cfg_count, fw_cfg);

        //check step 2: output config by domain
        res_assign_output();
    }

    return 0;
}

void firewall_enable(bool fw_enable)
{
    uint32_t value = readl(GLB_MAC_BASE_ADDR + MAC_GLB_CTL);
    uint32_t dom_per_en = fw_enable ? 1 : 0;

    value = reg_value(dom_per_en, value, MAC_GLB_CTL_DOM_PRO_EN_SHIFT, MAC_GLB_CTL_DOM_PRO_EN_MASK);

    writel(value, GLB_MAC_BASE_ADDR + MAC_GLB_CTL);
    writel(value, RPC_BASE_ADDR_CKGEN_SOC + RPC_MAC_GLB_CTL_OFFSET + MAC_GLB_CTL);
    writel(value, RPC_BASE_ADDR_SEC + RPC_MAC_GLB_CTL_OFFSET + MAC_GLB_CTL);

#ifdef ENABLE_SAF_FIREWALL
    writel(value, RPC_BASE_ADDR_SAF + RPC_MAC_GLB_CTL_OFFSET + MAC_GLB_CTL);
#endif
}

int rid_init(uint32_t cfg_count, firewall_cfg_t * rid_cfg)
{
    return write_cfg_to_fw(cfg_count, rid_cfg);
}

/* rules of permission set:
 * 1. enble/disable access_deny, will set read/write permission ctrl/secure-nonsecure ctrl/privilege-user ctrl.
 * 2. secure/non-secure/privilege/user permission must be with at least one of write and read permission.
 * 3. enable read/write permisson without secure/non-secure/privilege/user, enable read/write permission ctrl and
 *    secure/non-secure/privilege/user permission.
 * 4. disable read/write permission without secure/non-secure/privilege/user, disable read/write permission ctrl
 *    and secure/non-secure/privilege/user permission.
 * 5. enable secure/non-secure/privilege/user with write/read permission, enable read/write permission and secure/
 *    non-secure/privilege/user.
 * 6. disable secure/non-secure/privilege/user with write/read permission, just disable secure/non-secure/privilege/user.
 * 7. always not set enable bit to 0.
 * 8. maybe miss some situation, specially 5/6???? disable "enable is false" for 5/6.
 * 9. rpc just has share and unshare status, so enable means share, disable means unshare.
 * 10. rpc permission setting sequence: 1(self domain), 2, 3, 0
 *
 * todo:
 * 1. check validation of res_id
 * 2. check dom_id is resource domain owner or not
 * 3. check safety resources
 */
int permission_set(uint32_t res_id, uint32_t dom_id, permission_type_t permission, bool enable)
{
    //check validity of parameters
    if ((dom_id > 0xf) || (permission >= INVALID_PERMISSION)) {
        return -1;
    }

    //no read/write permission, but other not none
    if ((0 == (permission & 0x3)) && (0 != (permission & 0x3c))) {
        return -1;
    }

    rapc_type_t rapc_type;
    uint32_t rapc_index;
    uint32_t slot_index;
    uint32_t per_value, sec_value, pri_value;
    uint32_t per_index, sec_pri_index;
    uint32_t per_offset, sec_offset, pri_offset;
    addr_t reg_addr_per, reg_addr_sec, reg_addr_pri;

    sec_value = 0;
    per_value = 0;
    pri_value = 0;

    //bit31~bit30 RAPC type, bit29~bit24 RAPC index, bit23~bit17 category index, bit16~bit12 physical index, bit11~bit0 range/slot/register index
    //-------------------------------------------------------------
    //|31|30|29|28|27|26|25|24|23|22|21|20|19|18|17|16|15|14|13|12|11|10|09|08|07|06|05|04|03|02|01|00|
    //|type |      index      |      category      |   physical   |     range/slot/register index     |
    //-------------------------------------------------------------
    rapc_type = (res_id >> RES_ID_RAPC_TYPE_OFFSET) & 0x3;
    rapc_index = (res_id >> RES_ID_RAPC_INDEX_OFFSET) & 0x3f;
    slot_index = res_id & 0xfff;

    //domain id max is 16, the mpc register has two bytes(64bits) to share res, DOM_PER0 and DOM_PER1
    //the first byte is domain 1~8 share info, the second byte is domain 9~16 share info
    //one domain use 4bit, bit0, enable, bit1~2 rw ro wo none, bit3 lock.detail:
    //-------------------------------------------------------------
    //|31|30|29|28|27|26|25|24|23|22|21|20|19|18|17|16|15|14|13|12|11|10|09|08|07|06|05|04|03|02|01|00|
    //|  domain8  |  domain7  |  domain6  |  domain5  |  domain4  |  domain3  |  domain2  |  domain1  |
    //-------------------------------------------------------------
    per_index = dom_id / 8;
    per_offset = dom_id % 8;
    sec_pri_index = dom_id / 4;
    sec_offset = dom_id % 4;
    pri_offset = dom_id % 4;

    if (RAPC_RPC == rapc_type) {
        goto RPC_CONFIG;
    }

    switch (rapc_type) {
        case RAPC_MPC: //set mpc permission
            if (per_index == 0) {
                reg_addr_per = MPC_DOM_PER0_(slot_index) + MPC_ADDR_SIZE * rapc_index;
            }
            else {
                reg_addr_per = MPC_DOM_PER1_(slot_index) + MPC_ADDR_SIZE * rapc_index;
            }

            switch (sec_pri_index) {
                case 0:
                    reg_addr_sec = MPC_SEC_PER0_(slot_index) + MPC_ADDR_SIZE * rapc_index;
                    reg_addr_pri = MPC_PRI_PER0_(slot_index) + MPC_ADDR_SIZE * rapc_index;
                    break;
                case 1:
                    reg_addr_sec = MPC_SEC_PER1_(slot_index) + MPC_ADDR_SIZE * rapc_index;
                    reg_addr_pri = MPC_PRI_PER1_(slot_index) + MPC_ADDR_SIZE * rapc_index;
                    break;
                case 2:
                    reg_addr_sec = MPC_SEC_PER2_(slot_index) + MPC_ADDR_SIZE * rapc_index;
                    reg_addr_pri = MPC_PRI_PER2_(slot_index) + MPC_ADDR_SIZE * rapc_index;
                    break;
                case 3:
                    reg_addr_sec = MPC_SEC_PER3_(slot_index) + MPC_ADDR_SIZE * rapc_index;
                    reg_addr_pri = MPC_PRI_PER3_(slot_index) + MPC_ADDR_SIZE * rapc_index;
                    break;
                default:
                    return -1;
            }

            break;

        case RAPC_PPC: //set ppc permission
            if (per_index == 0) {
                reg_addr_per = PPC_DOM_PER0_(slot_index) + PPC_ADDR_SIZE * rapc_index;
            }
            else {
                reg_addr_per = PPC_DOM_PER1_(slot_index) + PPC_ADDR_SIZE * rapc_index;
            }

            switch (sec_pri_index) {
                case 0:
                    reg_addr_sec = PPC_SEC_PER0_(slot_index) + PPC_ADDR_SIZE * rapc_index;
                    reg_addr_pri = PPC_PRI_PER0_(slot_index) + PPC_ADDR_SIZE * rapc_index;
                    break;
                case 1:
                    reg_addr_sec = PPC_SEC_PER1_(slot_index) + PPC_ADDR_SIZE * rapc_index;
                    reg_addr_pri = PPC_PRI_PER1_(slot_index) + PPC_ADDR_SIZE * rapc_index;
                    break;
                case 2:
                    reg_addr_sec = PPC_SEC_PER2_(slot_index) + PPC_ADDR_SIZE * rapc_index;
                    reg_addr_pri = PPC_PRI_PER2_(slot_index) + PPC_ADDR_SIZE * rapc_index;
                    break;
                case 3:
                    reg_addr_sec = PPC_SEC_PER3_(slot_index) + PPC_ADDR_SIZE * rapc_index;
                    reg_addr_pri = PPC_PRI_PER3_(slot_index) + PPC_ADDR_SIZE * rapc_index;
                    break;
                default:
                    return -1;
            }

            break;

        default:
            return -1;
    }

    if (ACCESS_DENY == permission) {
        per_value = readl(reg_addr_per);
        sec_value = readl(reg_addr_sec);
        pri_value = readl(reg_addr_pri);

        if (true == enable) {
            per_value = (per_value & (~(0x7 << (per_offset * DOM_PER_REG_ONE_DOMAIN_BIT_LEN))))
                        | (0x1 << (per_offset * DOM_PER_REG_ONE_DOMAIN_BIT_LEN));
            sec_value = (sec_value & (~(0x1f << (sec_offset * SEC_PER_REG_ONE_DOMAIN_BIT_LEN))))
                        | (0x1 << (pri_offset * SEC_PER_REG_ONE_DOMAIN_BIT_LEN));
            pri_value = (pri_value & (~(0x1f << (sec_offset * PRI_PER_REG_ONE_DOMAIN_BIT_LEN))))
                        | (0x1 << (pri_offset * PRI_PER_REG_ONE_DOMAIN_BIT_LEN));
        }
        else {
            per_value |= (0x7 << (per_offset * DOM_PER_REG_ONE_DOMAIN_BIT_LEN));
            sec_value |= (0x1f << (sec_offset * SEC_PER_REG_ONE_DOMAIN_BIT_LEN));
            pri_value |= (0x1f << (pri_offset * PRI_PER_REG_ONE_DOMAIN_BIT_LEN));
        }

        writel(per_value, reg_addr_per);
        writel(sec_value, reg_addr_sec);
        writel(pri_value, reg_addr_pri);
    }
    else {
        if (0 == (permission & 0x3c)) {//just set read/write permission
            per_value = readl(reg_addr_per);
            sec_value = readl(reg_addr_sec);
            pri_value = readl(reg_addr_pri);

            if (true == enable) {//enable permission bit and set enable bit to 1
                per_value |= (((permission & 0x3) << 1) | 0x1) << (per_offset * DOM_PER_REG_ONE_DOMAIN_BIT_LEN);
                sec_value |= (((permission & 0x3) << 3) | ((permission & 0x3) << 1) | 0x1)
                             << (sec_offset * SEC_PER_REG_ONE_DOMAIN_BIT_LEN);
                pri_value |= (((permission & 0x3) << 3) | ((permission & 0x3) << 1) | 0x1)
                             << (pri_offset * PRI_PER_REG_ONE_DOMAIN_BIT_LEN);
            }
            else { //clear read/write permission bit and set enable bit to 1
                per_value = (per_value & (~(((permission & 0x3) << 1) << (per_offset * DOM_PER_REG_ONE_DOMAIN_BIT_LEN))))
                            | (0x1 << (per_offset * DOM_PER_REG_ONE_DOMAIN_BIT_LEN));
                sec_value = (sec_value & (~((((permission & 0x3) << 3) | ((permission & 0x3) << 1))
                            << (sec_offset * SEC_PER_REG_ONE_DOMAIN_BIT_LEN))))
                            | (0x1 << (sec_offset * SEC_PER_REG_ONE_DOMAIN_BIT_LEN));
                pri_value = (pri_value & (~((((permission & 0x3) << 3) | ((permission & 0x3) << 1))
                            << (pri_offset * PRI_PER_REG_ONE_DOMAIN_BIT_LEN))))
                            | (0x1 << (pri_offset * PRI_PER_REG_ONE_DOMAIN_BIT_LEN));
            }

            writel(per_value, reg_addr_per);
            writel(sec_value, reg_addr_sec);
            writel(pri_value, reg_addr_pri);
        }
        else { //sec/nonsecure/privilege/user with read/write permission
            if (true == enable) {
                per_value = readl(reg_addr_per);
                per_value |= (((permission & 0x3) << 1) | 0x1) << (per_offset * DOM_PER_REG_ONE_DOMAIN_BIT_LEN);
                writel(per_value, reg_addr_per);

                uint32_t value = 0;

                if (permission & (SECURE_ACCESS | NONSECURE_ACCESS)) {
                    sec_value = readl(reg_addr_sec);

                    if (permission & SECURE_ACCESS) {
                        value |= (permission & 0x3) << 1;
                    }

                    if (permission & NONSECURE_ACCESS) {
                        value |= (permission & 0x3) << 3;
                    }

                    sec_value |= (value | 0x1) << (sec_offset * SEC_PER_REG_ONE_DOMAIN_BIT_LEN);
                    writel(sec_value, reg_addr_sec);
                }

                if (permission & (PRIVILEGE_ACCESS | USER_ACCESS)) {
                    value = 0;
                    pri_value = readl(reg_addr_pri);

                    if (permission & PRIVILEGE_ACCESS) {
                        value |= (permission & 0x3) << 1;
                    }

                    if (permission & USER_ACCESS) {
                        value |= (permission & 0x3) << 3;
                    }

                    pri_value |= (value | 0x1)  << (pri_offset * PRI_PER_REG_ONE_DOMAIN_BIT_LEN);
                    writel(pri_value, reg_addr_pri);
                }
            }
            else { //disable, just set sec/nonsecure/privilege/user permission
                uint32_t value = 0;

                if (permission & (SECURE_ACCESS | NONSECURE_ACCESS)) {
                    sec_value = readl(reg_addr_sec);

                    if (permission & SECURE_ACCESS) {
                        value |= (permission & 0x3) << 1;
                    }

                    if (permission & NONSECURE_ACCESS) {
                        value |= (permission & 0x3) << 3;
                    }

                    sec_value = (sec_value & (~(value << (sec_offset * SEC_PER_REG_ONE_DOMAIN_BIT_LEN))))
                                | (0x1 << (sec_offset * SEC_PER_REG_ONE_DOMAIN_BIT_LEN));
                    writel(sec_value, reg_addr_sec);
                }

                if (permission & (PRIVILEGE_ACCESS | USER_ACCESS)) {
                    value = 0;
                    pri_value = readl(reg_addr_pri);

                    if (permission & PRIVILEGE_ACCESS) {
                        value |= (permission & 0x3) << 1;
                    }

                    if (permission & USER_ACCESS) {
                        value |= (permission & 0x3) << 3;
                    }

                    pri_value = (pri_value & (~(value << (pri_offset * PRI_PER_REG_ONE_DOMAIN_BIT_LEN))))
                                | (0x1 << (pri_offset * PRI_PER_REG_ONE_DOMAIN_BIT_LEN));
                    writel(pri_value, reg_addr_pri);
                }
            }
        }
    }

    return 0;

RPC_CONFIG:
    if (slot_index > 2045) { //register index from 0 to 2045
        dprintf(CRITICAL, "rpc register not exist.\n");
        return -1;
    }

    uint32_t per_sel = 0;
    addr_t reg_addr_rpc = 0;
    uint32_t rpc_value = 0;
    uint32_t res_dom = 0;

    switch (rapc_index) {
        case RPC_SOC:
            reg_addr_rpc = RPC_BASE_ADDR_CKGEN_SOC + (slot_index << 12);
            reg_addr_per = RPC_BASE_ADDR_CKGEN_SOC;
            break;
        case RPC_SEC:
            reg_addr_rpc = RPC_BASE_ADDR_SEC + (slot_index << 12);
            reg_addr_per = RPC_BASE_ADDR_SEC;
            break;
        case RPC_SAF:
            reg_addr_rpc = RPC_BASE_ADDR_SAF + (slot_index << 12);
            reg_addr_per = RPC_BASE_ADDR_SAF;
            break;

        default:
            dprintf(CRITICAL, "index of rpc type out of range.\n");
            return -1;
    }

    rpc_value = readl(reg_addr_rpc);
    res_dom = rpc_value & (~0xf);
    per_index = 4 * res_dom;

    //select config index
    if (true == enable) {
        int i = 0;
        for (; i < 4; i++) {
            if (1 == i) {
                per_index++;
                continue;
            }

            if (0 == dom_id / 8) {
                per_value = readl(reg_addr_per + RPC_DOM_PER0_(per_index));
            }
            else {
                per_value = readl(reg_addr_per + RPC_DOM_PER1_(per_index));
            }

            if ((0 != (per_value & (0x6 << (4 * (dom_id % 8)))))
                && (0x77777777 != (per_value & 0x7777777))) {//exclude all shared

                per_sel = i;
                break;
            }

            per_index++;
        }

        if (4 == i) {
            dprintf(CRITICAL, "can't find correspond configs.\n");
            return -1;
        }

    }
    else {
        per_sel = 1;
    }

    rpc_value = (rpc_value & (~(0x60))) | (per_sel << 5);
    writel(rpc_value, reg_addr_rpc);

    return 0;
}

/*
 * permission disable/enable operation
 * !!!!just support mpc/ppc type resources
 */
int permission_enable(uint32_t res_id, bool enable)
{
    rapc_type_t rapc_type;
    uint32_t rapc_index;
    uint32_t slot_index;
    uint32_t per_value;
    addr_t reg_addr_per;

    //bit31~bit30 RAPC type, bit29~bit24 RAPC index, bit23~bit17 category index, bit16~bit12 physical index, bit11~bit0 range/slot/register index
    //-------------------------------------------------------------
    //|31|30|29|28|27|26|25|24|23|22|21|20|19|18|17|16|15|14|13|12|11|10|09|08|07|06|05|04|03|02|01|00|
    //|type |      index      |      category      |   physical   |     range/slot/register index     |
    //-------------------------------------------------------------
    rapc_type = (res_id >> RES_ID_RAPC_TYPE_OFFSET) & 0x3;
    rapc_index = (res_id >> RES_ID_RAPC_INDEX_OFFSET) & 0x3f;
    slot_index = res_id & 0xfff;

    switch (rapc_type) {
        case RAPC_MPC: //get mpc register address
            if (slot_index > 15) {
                return -1;
            }

            reg_addr_per = MPC_DOM_(slot_index) + MPC_ADDR_SIZE * rapc_index;
            break;

        case RAPC_PPC: //get ppc register address
            if (slot_index > 63) {
                return -1;
            }

            reg_addr_per = PPC_DOM_(slot_index) + PPC_ADDR_SIZE * rapc_index;
            break;

        default:
            return -1;
    }

    //disable/enable permission
    per_value = readl(reg_addr_per);

    if (enable) {
        uint32_t per0 = readl(reg_addr_per + 0x4);
        uint32_t per1 = readl(reg_addr_per + 0x8);
        writel(per_value | 0x10, reg_addr_per);
        writel(per0, reg_addr_per + 0x4);
        writel(per1, reg_addr_per + 0x8);
    }
    else {
        writel(per_value & (~0x10), reg_addr_per);
    }

    return 0;
}
