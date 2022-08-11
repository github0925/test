/*
 * reboot_service.c
 *
 * Copyright (c) 2020 SemiDrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */

#include <arch/ops.h>
#include <assert.h>
#include <debug.h>
#include <err.h>
#include <image_cfg.h>
#include <string.h>
#include "cpu_hal.h"
#include "boot.h"
#include "lib/sdrv_common_reg.h"
#include "mem_image.h"
#include "peer_load.h"
#include "res.h"
#include "rstgen_hal.h"
#include "lib/reboot.h"
#include "reboot_service.h"
#if VERIFIED_BOOT
#include "verified_boot.h"
#endif
#include "str.h"

#ifdef DIL_IMAGES_MEMBASE
#define IMG_BACKUP_LOW_BASE (DIL_IMAGES_MEMBASE)
#define IMG_BACKUP_LOW_SZ   (DIL_IMAGES_MEMSIZE)
#else
#define IMG_BACKUP_LOW_BASE (0x0)
#define IMG_BACKUP_LOW_SZ   (0x0)
#endif

#define MB_REG_BASE (0xf4040000u)
#define MB_TMC0     (MB_REG_BASE + 0xc)
#define MB_TMH0     (MB_REG_BASE + 0x0)
#define MB_TMH1     (MB_REG_BASE + 0x4)
#define MB_TMH2     (MB_REG_BASE + 0x8)

#define FM_TMH0_MDP (0xffU << 16U)
#define FV_TMH0_MDP(v) \
            (((v) << 16U) & FM_TMH0_MDP)

#define FM_TMH0_TXMES_LEN   (0x7ffU << 0U)
#define FV_TMH0_TXMES_LEN(v) \
    (((v) << 0U) & FM_TMH0_TXMES_LEN)

#define FM_TMH0_MID (0xffU << 24U)
#define FV_TMH0_MID(v) \
    (((v) << 24U) & FM_TMH0_MID)

#ifndef BPT_SIZE
#define BPT_SIZE           0x800
#endif

#define BPT_TAG            0x42505401
#define IIB_OFFSET         0x20
#define IMG_SZ_IIB_OFFSET  0x28
#define IMG_LDA_IIB_OFFSET 0x2C
#define IMG_EP_IIB_OFFSET  0x34
#define GET_BPT_IMG_SZ(h)  (*((uint32_t *)(addr_t)((h) + IIB_OFFSET + IMG_SZ_IIB_OFFSET)))
#define GET_BPT_IMG_LDA(h) (*((uint32_t *)(addr_t)((h) + IIB_OFFSET + IMG_LDA_IIB_OFFSET)))
#define GET_BPT_IMG_EP(h)  (*((uint32_t *)(addr_t)((h) + IIB_OFFSET + IMG_EP_IIB_OFFSET)))
#define HAS_BPT_HEADER(h)  (*((uint32_t*)(addr_t)h) == BPT_TAG)

#define BM_TMC0_TMC0_MSG_SEND   (0x01U << 0U)

#define ERROR(format, args...) dprintf(CRITICAL, \
                               "ERROR:%s %d "format"\n", __func__, __LINE__,  ##args)

#define DBG(format, args...) dprintf(INFO, \
                               "DGB:%s %d "format"\n", __func__, __LINE__,  ##args)

typedef int (* rb_m_proc)(rb_opc_e, rb_arg *);

static uint32_t verified_image(void *image_addr, uint64_t image_size,
                               const char *pt_name)
{
#if VERIFIED_BOOT
    uint32_t ret;
    uint8_t *vbmeta_buf = (uint8_t *)_ioaddr(VBMETA_MEMBASE);
    uint32_t vbmeta_sz = VBMETA_MEMSIZE;
    struct list_node verified_images_list;
    list_initialize(&verified_images_list);
    ret = add_verified_image_list(&verified_images_list,
                                  image_addr, image_size, pt_name);
    ASSERT(ret);
    ret = add_verified_image_list(&verified_images_list,
                                  vbmeta_buf, vbmeta_sz, VBMETA_PARTITION_NAME);
    ASSERT(ret);

    if (!verify_loaded_images(NULL, &verified_images_list, NULL)) {
        dprintf(ALWAYS, "%s %d verify images fail\n", __func__, __LINE__);
        free_image_info_list(&verified_images_list);
        return ERR_NOT_VALID;
    }

    free_image_info_list(&verified_images_list);
#endif
    return NO_ERROR;
}

static int kick_module(uint32_t id, uint64_t entry)
{
    bool ret;
    void *handle;
    ret = hal_cpu_create_handle(&handle);
    ASSERT(ret);
    hal_cpu_boot(handle, id, entry);
    hal_cpu_release_handle(handle);
    return NO_ERROR;
}

static int reboot_saf(rb_opc_e opc, rb_arg *arg)
{
    addr_t entry;
    uint64_t img_sz;

    DBG("E");

    if (!arg)
        return ERR_INVALID_ARGS;

    entry = _ioaddr(arg->entry);
    img_sz = arg->sz;
    if (NO_ERROR != verified_image((void *)entry, img_sz,
                                   "safety_os")) {
        return ERR_NOT_VALID;
    }
    kick_module(CPU_ID_SAF, entry);
    return NO_ERROR;
}

static int reboot_ap1(rb_opc_e opc, rb_arg *arg)
{
    uint64_t entry;
    addr_t load_addr;
    mem_image_entry_t info;
    addr_t seeker_base = IMG_BACKUP_LOW_BASE;
    uint32_t ret;
    void *handle = NULL;
    ret = hal_rstgen_creat_handle(&handle, RES_GLOBAL_RST_SEC_RST_EN);

    if (!ret) {
        dprintf(CRITICAL, "reboot_ap1 hal_rstgen_creat_handle fail \n");
        return -1;
    }

    ret = hal_rstgen_init(handle);

    if (!ret) {
        hal_rstgen_release_handle(handle);
        return -1;
    }

    //hold module reset
    hal_rstgen_module_ctl(handle, RES_MODULE_RST_SEC_CPU1_SS, 0x0);
    hal_rstgen_module_ctl(handle, RES_MODULE_RST_SEC_CPU1_CORE0_WARM, 0x0);
    hal_rstgen_module_ctl(handle, RES_MODULE_RST_SEC_CPU1_CORE1_WARM, 0x0);
    hal_rstgen_module_ctl(handle, RES_MODULE_RST_SEC_CPU1_CORE2_WARM, 0x0);
    hal_rstgen_module_ctl(handle, RES_MODULE_RST_SEC_CPU1_CORE3_WARM, 0x0);
    hal_rstgen_module_ctl(handle, RES_MODULE_RST_SEC_CPU1_CORE4_WARM, 0x0);
    hal_rstgen_module_ctl(handle, RES_MODULE_RST_SEC_CPU1_CORE5_WARM, 0x0);
    hal_rstgen_module_ctl(handle, RES_MODULE_RST_SEC_CPU1_SCU_WARM, 0x0);
    //hold core reset
    hal_rstgen_core_ctl(handle, RES_CORE_RST_SEC_CPU1_CORE_ALL_SW, 0x0);
    //hold gic4 reset
    hal_rstgen_module_reset(handle, RES_MODULE_RST_SEC_GIC4);
    /* workaround solution, reset pcie1/phy/pcie2
       pcie driver init will clear pcie registers
     */
    hal_rstgen_module_ctl(handle, RES_MODULE_RST_SEC_PCIEPHY, 0x0);
    hal_rstgen_module_ctl(handle, RES_MODULE_RST_SEC_PCIE1, 0x0);
    hal_rstgen_module_ctl(handle, RES_MODULE_RST_SEC_PCIE2, 0x0);
    hal_rstgen_module_ctl(handle, RES_MODULE_RST_SEC_PCIEPHY, 0x1);
    hal_rstgen_module_ctl(handle, RES_MODULE_RST_SEC_PCIE1, 0x1);
    hal_rstgen_module_ctl(handle, RES_MODULE_RST_SEC_PCIE2, 0x1);
    hal_rstgen_release_handle(handle);
    entry = arg->entry;

    if (!entry) {
#ifdef AP1_PRELOADER_MEMBASE
        DBG("use default entry for ap1\n");
        entry = AP1_PRELOADER_MEMBASE;
#else
        panic("ap1 entry is 0!\n");
#endif
    }

    if (is_str_resume(STR_AP1)) {
        kick_module(CPU_ID_AP1, entry);
        return NO_ERROR;
    }

    load_addr = ap2p(entry);

    if (mem_image_seek(seeker_base, "preloader", &info)) {
        ERROR("don't find preloader for ap1 image!");
        return ERR_NOT_FOUND;
    }

    DBG("back base:0x%llx sz:%llu target addr:0x%lx", info.base, info.sz,
        load_addr);

    if (NO_ERROR != verified_image((void *)(addr_t)info.base, info.sz,
                                   "preloader")) {
        return ERR_NOT_VALID;
    }

    memcpy((void *)load_addr, (void *)(addr_t)info.base, info.sz);
    arch_clean_cache_range(load_addr, info.sz);
    kick_module(CPU_ID_AP1, entry);
    return NO_ERROR;
}

static int reboot_ap2(rb_opc_e opc, rb_arg *arg)
{
    uint64_t entry;
    addr_t load_addr;
    mem_image_entry_t info;
    addr_t seeker_base = IMG_BACKUP_LOW_BASE;
    uint32_t ret;
    const char *pt_name;
    void *handle = NULL;
    ret = hal_rstgen_creat_handle(&handle, RES_GLOBAL_RST_SEC_RST_EN);

    if (!ret) {
        dprintf(CRITICAL, "reboot_ap2 hal_rstgen_creat_handle fail \n");
        return -1;
    }

    ret = hal_rstgen_init(handle);

    if (!ret) {
        hal_rstgen_release_handle(handle);
        return -1;
    }

    //hold module reset
    hal_rstgen_module_ctl(handle, RES_MODULE_RST_SEC_CPU2_SS, 0x0);
    //hold core reset
    hal_rstgen_core_ctl(handle, RES_CORE_RST_SEC_CPU2_CORE_SW, 0x0);
    //hold gic5 reset
    hal_rstgen_module_reset(handle, RES_MODULE_RST_SEC_GIC5);
    hal_rstgen_release_handle(handle);
    entry = arg->entry;

    if (!entry) {
#ifdef AP2_PRELOADER_MEMBASE
        DBG("use default entry for ap2\n");
        entry = AP2_PRELOADER_MEMBASE;
#else
        panic("ap2 entry is 0!\n");
#endif
    }

    if (is_str_resume(STR_AP2)) {
        kick_module(CPU_ID_AP2, entry);
        return NO_ERROR;
    }

    load_addr = ap2p(entry);
    pt_name = "cluster_preloader";
    ret = mem_image_seek(seeker_base, pt_name, &info);

    if (ret) {
        pt_name = "preloader";
        ret = mem_image_seek(seeker_base, "preloader", &info);

        if (ret) {
            ERROR("don't find preloader for ap2 image!");
            return ERR_NOT_FOUND;
        }
    }

    DBG("back base:0x%llx sz:%llu target addr:0x%lx", info.base, info.sz,
        load_addr);

    if (NO_ERROR != verified_image((void *)(addr_t)info.base, info.sz,
                                   pt_name)) {
        return ERR_NOT_VALID;
    }

    memcpy((void *)load_addr, (void *)(addr_t)info.base, info.sz);
    arch_clean_cache_range(load_addr, info.sz);
    kick_module(CPU_ID_AP2, entry);
    return NO_ERROR;
}

static int reboot_mp(rb_opc_e opc, rb_arg *arg)
{
#ifdef SDPE_MEMBASE
    mem_image_entry_t info;
    addr_t target_addr = SDPE_MEMBASE;
    size_t seeker_size = IMG_BACKUP_LOW_SZ;
    addr_t seeker_base = IMG_BACKUP_LOW_BASE;
    uint32_t ret;
    void *handle = NULL;
    ret = hal_rstgen_creat_handle(&handle, RES_GLOBAL_RST_SEC_RST_EN);

    if (!ret) {
        dprintf(CRITICAL, "reboot_mp hal_rstgen_creat_handle fail \n");
        return -1;
    }

    ret = hal_rstgen_init(handle);

    if (!ret) {
        hal_rstgen_release_handle(handle);
        return -1;
    }

    //hold core reset
    hal_rstgen_core_ctl(handle, RES_CORE_RST_SEC_CR5_MP_SW, 0x0);
    //hold gic4 reset
    hal_rstgen_module_ctl(handle, RES_MODULE_RST_SEC_GIC3, 0x0);
    hal_rstgen_release_handle(handle);
    mem_image_init(seeker_base, seeker_size);

    if (mem_image_seek(seeker_base, "sdpe_fw", &info)) {
        ERROR("don't find sdpe_fw image!");
        return ERR_NOT_FOUND;
    }

    if (NO_ERROR != verified_image((void *)(addr_t)info.base, info.sz,
                                   "sdpe_fw")) {
        dprintf(CRITICAL, "verified_image  sdpe_fw fail \n");
        return ERR_NOT_VALID;
    }

    /* Wait for ssystem disabling firewall. */
    while (1) {
        if (sdrv_common_reg_get_value(SDRV_REG_STATUS,
                                      SDRV_REG_STATUS_FIREWALL_EARLY_DONE)) {
            break;
        }
    }

    memcpy((void *)target_addr, (void *)(addr_t)info.base, info.sz);
    arch_clean_cache_range(target_addr, info.sz);
    /* FIXME: To start mp, it should send request to HSM.
     * Temporarily, set the gpr instead.
     * */
    sdrv_common_reg_set_value(SDRV_REG_STATUS, SDRV_REG_STATUS_SDPE_LD_DONE,
                              SDRV_REG_STATUS_SDPE_LD_DONE);
#endif
    return NO_ERROR;
}

static void send_peer_load_msg(uint32_t para)
{
    volatile uint32_t *tmc_reg = (uint32_t *)MB_TMC0;
    volatile uint32_t *tmh0 = (uint32_t *)MB_TMH0;
    volatile uint32_t *tmh1 = (uint32_t *)MB_TMH1;
    volatile uint32_t *tmh2 = (uint32_t *)MB_TMH2;
    uint64_t val = 0ull;
    uint32_t mask = 0x2;
    uint8_t msg_id = 0;
    struct peer_boot_message msg_ori = MK_PEER_LOAD_MSG(para);
    uint32_t len = sizeof(struct peer_boot_message);
    uint8_t *msg = (uint8_t *)&msg_ori;

    for (uint32_t i = 0; i < len; i++) {
        val |= ((uint64_t)msg[i] << (i * 8));
    }

    *tmh0  = FV_TMH0_MDP(mask) | FV_TMH0_TXMES_LEN((len + 1) / 2) |
             FV_TMH0_MID(
                 msg_id);
    *tmh1 = (uint32_t)val;

    if (len > 4) {
        *tmh2 = (uint32_t)(val >> 32);
    }

    tmc_reg[msg_id] |= BM_TMC0_TMC0_MSG_SEND;
}

static int reboot_sec(rb_opc_e opc, rb_arg *arg)
{
    mem_image_entry_t info;
    const char *pt_name;
    uint32_t ret;
    size_t seeker_size = IMG_BACKUP_LOW_SZ;
    addr_t seeker_base = IMG_BACKUP_LOW_BASE;
    mem_image_init(seeker_base, seeker_size);
    pt_name = "fda_spl";
    ret = mem_image_seek(seeker_base, pt_name, &info);

    if (ret) {
        pt_name = "ssystem";
        ret = mem_image_seek(seeker_base, pt_name, &info);

        if (ret) {
            ERROR("don't find ssystem image!");
            return ERR_NOT_FOUND;
        }
    }

    if (arg->flags & RB_COLD && (boot_get_pin() == BOOT_PIN_0)) {
        send_peer_load_msg(info.base);
    }
    else if (HAS_BPT_HEADER(info.base)) {
        /* TODO verify ssystem image */
        DBG("kick security core, base:%llu entry:%u size:%llu\n", info.base,
            GET_BPT_IMG_EP(info.base), info.sz - BPT_SIZE);

        if (NO_ERROR != verified_image((void *)(addr_t)(info.base),
                                       info.sz, pt_name)) {
            return ERR_NOT_VALID;
        }

        memcpy((void *)(addr_t)GET_BPT_IMG_LDA(info.base),
               (void *)(addr_t)(info.base + BPT_SIZE), info.sz - BPT_SIZE);
        arch_clean_cache_range(GET_BPT_IMG_LDA(info.base), info.sz - BPT_SIZE);
        kick_module(CPU_ID_SEC, GET_BPT_IMG_EP(info.base));
    }
    else {
        ERROR("ssystem format error!");
        return ERR_INVALID_ARGS;
    }

    return NO_ERROR;
}

static rb_m_proc proc[RB_MAX_M] = {
    reboot_saf,
    reboot_sec,
    reboot_mp,
    reboot_ap1,
    reboot_ap2
};

int reboot_module(rb_module_e m, rb_opc_e opc, rb_arg *arg)
{
    if (m < 0 || m >= RB_MAX_M || !arg) {
        ERROR("invalid module id:%d or arg", m);
        return ERR_NOT_FOUND;
    }

    return (proc[m])(opc, arg);
}

