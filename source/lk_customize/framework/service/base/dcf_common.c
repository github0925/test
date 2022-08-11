/*
* Copyright (c) 2019 Semidrive Semiconductor Inc.
* All rights reserved.
*
* Description: domain communication high level api for secure
*
*/
#include <reg.h>
#include <stdio.h>
#include <trace.h>
#include <string.h>
#include <stdlib.h>
#include <platform/debug.h>
#include <mbox_hal.h>
#include <res.h>
#include "dcf_common.h"

#if defined(CONFIG_SUPPORT_DCF) && (CONFIG_SUPPORT_DCF == 1)

void start_dcf_service(void);

#if defined(CONFIG_RPMSG_SERVICE) && (CONFIG_RPMSG_SERVICE == 1)
#include <chip_res.h>
#include <dcf_configs.h>
#include <share_mem_info.h>
#include <rpmsg_rtos.h>

#if ARM_WITH_MPU
#include <arch/arm/mpu.h>
#endif

static struct rpmsg_dev_config *rpmsg_cfg;
static uint32_t rpmsg_cfg_num;

/* TODO: mpu hardware require the base modular size
 * limit the size for now, fix this once we need more region size
 */
#define MAX_SHM_REGION_SIZE     (0x100000)
/* add shared memory region for IPC */
int init_shm_domain_area(int region_base)
{
    int region = region_base;
#if ARM_WITH_MPU
    unsigned i;
    uint32_t actual_size;

    for (i = 0; i < share_mem_list.share_mem_num; i++) {
        /* add shared memory region for IPC */
        actual_size = share_mem_list.share_mem_info[i].size;
        if (actual_size > MAX_SHM_REGION_SIZE)
            actual_size = MAX_SHM_REGION_SIZE;

        mpu_add_region(region++,
                        share_mem_list.share_mem_info[i].paddr,
                        actual_size,
                        MPU_REGION_STRONGORDERED);
        dprintf(1, "dcf: add region %lx %x\n",
                share_mem_list.share_mem_info[i].paddr, actual_size);
    }
#endif
    return region;
}

paddr_t dcf_get_remote_shm_base(domain_cpu_id_t proc)
{
    unsigned i;

    for (i = 0; i < rpmsg_cfg_num; i++) {
        if (proc == rpmsg_cfg[i].remote_proc) {
            return rpmsg_cfg[i].shm_phys_base;
        }
    }

    return 0;
}

uint32_t dcf_get_remote_shm_size(domain_cpu_id_t proc)
{
    unsigned i;

    for (i = 0; i < rpmsg_cfg_num; i++) {
        if (proc == rpmsg_cfg[i].remote_proc) {
            return rpmsg_cfg[i].shm_size;
        }
    }

    return 0;
}

static int get_curr_proc(void)
{
    return dcf_get_this_proc();
}

/*
 * Return code
 * 1: this is master device
 * 0: this is remote device
 * -1: not found, should be disabled
 */
static int rpmsg_is_master_device(uint32_t res_id)
{
    unsigned i;

    for (i = 0; i < dcf_configs_list.config_num; i++) {
        if (dcf_configs_list.dcf_config[i].res_id == res_id)
            return (dcf_configs_list.dcf_config[i].mid_master == get_curr_proc());
    }

    dprintf(1, "NOT FOUND rpmsg res:%x in dcf_config\n", res_id);

    return -1;
}

static int generate_mbox_addr(int curr_proc, int remote_proc)
{
    int i = 0;
    int base = 0;
    int a, b;

    if (curr_proc == remote_proc)
        return -1;

    if ((curr_proc >= DP_CPU_MAX) || (remote_proc >= DP_CPU_MAX))
        return -1;

    a = MIN(curr_proc, remote_proc);
    b = MAX(curr_proc, remote_proc);
    while (i < b) {
        base += i++;
    }

    return IPCC_ADDR_RPMSG + base + a;
}

bool has_cross_mem_space(int id)
{
    return ((share_mem_list.share_mem_info[id].mid_b == 3) ||
               (share_mem_list.share_mem_info[id].mid_b == 4));
}

/*
 * get shm and dcf config from chipcfg like dts db
 */
struct rpmsg_dev_config *init_rpmsg_dev_config(void)
{
    unsigned i;
    struct rpmsg_dev_config *pcfg;
    uint32_t actual_size;

    if (rpmsg_cfg)
        return rpmsg_cfg;

    rpmsg_cfg_num = share_mem_list.share_mem_num;
    rpmsg_cfg = malloc(rpmsg_cfg_num * sizeof(struct rpmsg_dev_config));
    if (!rpmsg_cfg) {
        dprintf(0, "No memory for rpmsg config db\n");
        return NULL;
    }

    memset(rpmsg_cfg, 0, (rpmsg_cfg_num * sizeof(struct rpmsg_dev_config)));
    pcfg = rpmsg_cfg;

    for (i = 0; i < rpmsg_cfg_num; i++, pcfg++) {
        pcfg->shm_phys_base = share_mem_list.share_mem_info[i].paddr;
        actual_size = share_mem_list.share_mem_info[i].size;
        if (actual_size > MAX_SHM_REGION_SIZE)
            actual_size = MAX_SHM_REGION_SIZE;

        pcfg->shm_size = actual_size;

        pcfg->is_master = rpmsg_is_master_device(share_mem_list.share_mem_info[i].res_id);
        pcfg->remote_proc = share_mem_list.share_mem_info[i].mid_b;
        pcfg->ext.mbox_dst = pcfg->ext.mbox_src =
            generate_mbox_addr(share_mem_list.share_mem_info[i].mid_a, share_mem_list.share_mem_info[i].mid_b);
        pcfg->pa_spacex = has_cross_mem_space(i);

        dprintf(INFO, "dcf: mbaddr: %x\n", pcfg->ext.mbox_dst);
    }

    return rpmsg_cfg;
}

#define IS_PA_FROM_APU(pa)      (pa > SHM_AP_DDR_BASE)
/*
 * TODO: to refine the address translation between RPU and APU
 * APU and RPU has a address offset for the exact physical address
 * the offset is defined in SHM_PA_R_A_OFFSET
 * the domain dcf should implement these two interfaces
 */
bool dcf_is_pa_to_apu(paddr_t pa)
{
    struct rpmsg_dev_config *pcfg = rpmsg_cfg;
    unsigned i;

    for (i = 0; i < rpmsg_cfg_num; i++, pcfg++) {
        if (pcfg->pa_spacex && MEM_IN_RANGE(pa, pcfg->shm_phys_base, pcfg->shm_size)) {
            return true;
        }
    }

    return false;
}

bool dcf_is_rpu_area(paddr_t pa)
{
    struct rpmsg_dev_config *pcfg = rpmsg_cfg;
    unsigned i;

    for (i = 0; i < rpmsg_cfg_num; i++, pcfg++) {
        if (!pcfg->pa_spacex && MEM_IN_RANGE(pa, pcfg->shm_phys_base, pcfg->shm_size)) {
            return true;
        }
    }

    return false;
}

paddr_t platform_shm_get_local(paddr_t remote_pa)
{
    paddr_t local_pa = 0;
    if (IS_PA_FROM_APU(remote_pa))
        local_pa = ap2p(remote_pa);
    else if (dcf_is_rpu_area(remote_pa)){
        local_pa = remote_pa;
    } else {
        dprintf(INFO, "[%d]remote PA: 0x%lx out of range\n",
                dcf_get_this_proc(), remote_pa);
    }

    if (local_pa != remote_pa)
        dprintf(INFO, "[%d]import PA: 0x%lx -> 0x%lx\n",
                dcf_get_this_proc(), remote_pa, local_pa);

    return local_pa;
}

paddr_t platform_shm_get_remote(paddr_t local_pa)
{
    paddr_t remote_pa = 0;
    if (dcf_is_pa_to_apu(local_pa))
        remote_pa = p2ap(local_pa);
    else if (dcf_is_rpu_area(local_pa)) {
        remote_pa = local_pa;
    } else {
        dprintf(INFO, "[%d]local PA: 0x%lx out of range\n", dcf_get_this_proc(), local_pa);
    }

    if (local_pa != remote_pa)
        dprintf(INFO, "[%d]export PA: 0x%lx -> 0x%lx\n", dcf_get_this_proc(), local_pa, remote_pa);

    return remote_pa;
}

int platform_get_rpmsg_config(struct rpmsg_dev_config **p)
{
    *p = init_rpmsg_dev_config();
    return rpmsg_cfg_num;
}
#endif //if defined(CONFIG_SHM_RPMSG_VIRTIO) && (CONFIG_SHM_RPMSG_VIRTIO == 1)

#endif //if defined(CONFIG_SUPPORT_DCF) && (CONFIG_SUPPORT_DCF == 1)

static void *mbox_handle;
void mbox_init(void)
{
    hal_mb_cfg_t hal_cfg;

    hal_mb_create_handle(&mbox_handle, RES_MB_MB_MEM);
    if (mbox_handle != NULL) {
        hal_mb_init(mbox_handle, &hal_cfg);
    }
}

void dcf_early_init(void)
{
    return;
}

static bool dcf_inited;

void dcf_init(void)
{
    if (dcf_inited)
        return;

    mbox_init();

#if defined(CONFIG_SUPPORT_DCF) && (CONFIG_SUPPORT_DCF == 1)

    start_dcf_service();
    dcf_file_init();

#if defined(CONFIG_RPMSG_SERVICE) && (CONFIG_RPMSG_SERVICE == 1)
    rpmsg_rtos_init();
#endif

#endif
    dcf_inited = true;
}

