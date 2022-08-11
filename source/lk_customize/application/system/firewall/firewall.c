/*
* Copyright (c) 2019 Semidrive Semiconductor.
* All rights reserved.
*
*/

#include <debug.h>
#include <string.h>
#include <app.h>
#include <lib/console.h>
#include <reg.h>

#include <firewall_hal.h>
#if VIRTUALIZATION_EXT
#include <firewall_cfg_xen-x.h>
#else
#include <firewall_cfg.h>
#endif
#include <mac_reg.h>
#include <mpc_reg.h>
#include <ppc_reg.h>
#include <mpc.h>
#include <ppc.h>
#include <res.h>

int cfg_obtain(uint32_t* cfg_count, firewall_cfg_t** fw_cfg, bool* cfg_debug)
{
    if (fw_cfg_count != (sizeof(fw_init_cfg) / sizeof(firewall_cfg_t))) {
        dprintf(CRITICAL, "fw config info not consistent, count: %d, arrary size: %d.\n",
                fw_cfg_count, sizeof(fw_init_cfg) / sizeof(firewall_cfg_t));
        return -1;
    }

    *cfg_debug = (0 == fw_cfg_debug) ? false : true;
    *cfg_count = fw_cfg_count;
    *fw_cfg = fw_init_cfg;

    return 0;
}

int fw_init(void)
{
    int ret = 0;
    uint32_t cfg_count = 0;
    firewall_cfg_t* fw_cfg = NULL;
    bool cfg_debug = false;
    void* firewall_handle = NULL;

    if (0 != cfg_obtain(&cfg_count, &fw_cfg, &cfg_debug)) {
        return -1;
    }

    hal_firewall_creat_handle(&firewall_handle, 0);

    ret = hal_firewall_init(firewall_handle, cfg_count, fw_cfg, cfg_debug);

    hal_firewall_delete_handle(firewall_handle);

    return ret;
}

void firewall_disable(void)
{
    void *firewall_handle = NULL;
    hal_firewall_creat_handle(&firewall_handle, 0);
    hal_firewall_enable(firewall_handle, false);
    hal_firewall_delete_handle(firewall_handle);
}

void rid_config_init(void)
{
    void *firewall_handle = NULL;
    hal_firewall_creat_handle(&firewall_handle, 0);
    hal_rid_init(firewall_handle, rid_cfg_count, rid_init_cfg);
    hal_firewall_delete_handle(firewall_handle);
}

/* hsm resources protectation rules:
 * 1. assign secure core/tcm/vce0 to domain E
 * 2. protect tcm by mpc
 * 3. protect vce0  reg/memory by ppc
 * 4. share all memory to domain E(ddr/iram)
 * 5. enable firewall by mac and set domain E as resource manager
 */
int hsm_firewall(void)
{
    uint32_t value = 0;

    //disable domain permission check
    value = readl(GLB_MAC_BASE_ADDR + MAC_GLB_CTL);
    value &= (~0x1);
    writel(value, GLB_MAC_BASE_ADDR + MAC_GLB_CTL);

    value = readl(GLB_MAC_BASE_ADDR + MAC_RES_MGR);
    value &= ~(0x1 << 2);
    writel(value, GLB_MAC_BASE_ADDR + MAC_RES_MGR);

    //assign resources to domain E -- master
    writel(0xe, GLB_MAC_BASE_ADDR + MAC_MDA_(1)); //secure core
    writel(0xe, GLB_MAC_BASE_ADDR + MAC_MDA_(48)); //vce0

    //assign resources to domain E -- slave
    writel(0x1e, MPC_DOM_(0) + MPC_ADDR_SIZE * 9); //tcm(sec_platform)
    writel(0x1e, MPC_DOM_(0) + MPC_ADDR_SIZE * 5); //vce0 memory
    writel(0x1e, PPC_DOM_(0) + PPC_ADDR_SIZE * 10); //vce0
    writel(0x1e, PPC_DOM_(8) + PPC_ADDR_SIZE * 10); //vce ctrl

    //protect resources by firewall
    addr_t base_addr;
    int index;
    int res = res_get_info_by_id(RES_PLATFORM_R5_SEC_TCMA, &base_addr, &index);
    if (0 != res) {
        dprintf(CRITICAL, "not found resource tcm.\n");
        return res;
    }

    //base_addr = p2ap(base_addr);
    writel(base_addr >> 12, MPC_RGN_START_ADDR_(0) + MPC_ADDR_SIZE * 9);
    writel((0x1 << 30) | ((base_addr + 0x20000 -1) >> 12), MPC_RGN_END_ADDR_(0) + MPC_ADDR_SIZE * 9);
    writel(base_addr >> 12, MPC_RGN_LOW_LIM_(0) + MPC_ADDR_SIZE * 9);
    writel((base_addr + 0x20000 -1) >> 12, MPC_RGN_UP_LIM_(0) + MPC_ADDR_SIZE * 9);

    res = res_get_info_by_id(RES_CE_MEM_CE2_VCE1, &base_addr, &index);
    if (0 != res) {
        dprintf(CRITICAL, "not found resource vce1 memory.\n");
        return res;
    }

    base_addr = p2ap(base_addr);
    writel(base_addr >> 12, MPC_RGN_START_ADDR_(0) + MPC_ADDR_SIZE * 5);
    writel((0x1 << 30) | ((base_addr + 0x2000 -1) >> 12), MPC_RGN_END_ADDR_(0) + MPC_ADDR_SIZE * 5);
    writel(base_addr >> 12, MPC_RGN_LOW_LIM_(0) + MPC_ADDR_SIZE * 5);
    writel((base_addr + 0x2000 -1) >> 12, MPC_RGN_UP_LIM_(0) + MPC_ADDR_SIZE * 5);

    //share all memory to domain E
    uint32_t mpc_offset;
    uint32_t mpc_index[5] = {20, 7, 8, 25, 26};

    for (int j = 0; j < 5; j++) {
        mpc_offset = MPC_ADDR_SIZE * mpc_index[j];

        for (int i = 0; i < 16; i++) {
            value = readl(MPC_DOM_(i) + mpc_offset);

            if (0 != (value & 0x10)) {
                value = readl(MPC_DOM_PER1_(i) + mpc_offset);
                writel(value | 0x7000000, MPC_DOM_PER1_(i) + mpc_offset);

                value = readl(MPC_SEC_PER3_(i) + mpc_offset);
                writel(value | 0x1f0000, MPC_SEC_PER3_(i) + mpc_offset);

                value = readl(MPC_PRI_PER3_(i) + mpc_offset);
                writel(value | 0x1f0000, MPC_PRI_PER3_(i) + mpc_offset);
            }
            else {
                break;
            }
        }
    }

    //enable firewall
    value = readl(GLB_MAC_BASE_ADDR + MAC_RES_MGR);
    value |= ((0xe << 3) | (0x1 << 2));
    writel(value, GLB_MAC_BASE_ADDR + MAC_RES_MGR);

    value = readl(GLB_MAC_BASE_ADDR + MAC_GLB_CTL);
    value |= 0x1;
    writel(value, GLB_MAC_BASE_ADDR + MAC_GLB_CTL);

    return 0;
}

#if defined(WITH_LIB_CONSOLE)
#if SUPPORT_FW_INIT_CMD
STATIC_COMMAND_START
STATIC_COMMAND("firewall_init", "initial firewall with config file", (console_cmd)&fw_init)
STATIC_COMMAND_END(firewall_init);
#endif
#endif

APP_START(firewall_init)
.flags = 0
         APP_END
