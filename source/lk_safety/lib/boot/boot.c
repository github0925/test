/*
 * Copyright (c) 2019 Semidrive Semiconductor.
 * All rights reserved.
 */

#include <trace.h>
#include "__regs_base.h"
#include "boot.h"
#include "scr_hal.h"

#define LOCAL_TRACE 0

static boot_info_t g_bootinfo;

/* Check SEC_SCR.L16[49] */
static uint32_t boot_get_pin_scr_overwrite(boot_info_t *bootinfo)
{
    /* Check SEC_SCR.L16[49][8] first */
    uint32_t data = 0;
    data = readl(_ioaddr(APB_SCR_SEC_BASE + ((0x200 + 4 * 49) << 10)));
    if (data & (1 << 8)) {
        LTRACEF("pin from scr\n");
        /* Get pin from SEC_SCR.L16[49][3:0] */
        data &= 0xf;
        bootinfo->boot_ops |= BOOT_INFO_FROM_SCR_MASK;
        bootinfo->boot_pin = data;
        return data;
    }
    return PIN_ERROR;
}

/* Check BT_FUSE0 */
static uint32_t boot_get_pin_fuse(boot_info_t *bootinfo)
{
    uint32_t data;
    data = readl(_ioaddr(APB_EFUSEC_BASE + 0x1000 + FUSE_BT_CGF_INDEX0 * 4));
    /* BT_FUSE0[25], 0 from external boot pin, 1 from fuse */
    if (data & (1 << 25)) {
        LTRACEF("pin from fuse\n");
        /* Get pin from BT_FUSE0[31:26] */
        data = (data & 0x3c000000) >> 26;
        bootinfo->boot_ops |= BOOT_INFO_FROM_FUSE_MASK;
        bootinfo->boot_pin = data;
        return data;
    }
    return PIN_ERROR;
}

/* Check GPIO_B[11:8] */
static uint32_t boot_get_pin_gpio(boot_info_t *bootinfo)
{
    uint32_t data;
    scr_handle_t handle_bootmode;

    /* read gpio boot pin shadow register */
    handle_bootmode = hal_scr_create_handle(SCR_SEC__RO__rstgen_saf_boot_mode_scr_3_0);
    data = hal_scr_get(handle_bootmode);
    hal_scr_delete_handle(SCR_SEC__RO__rstgen_saf_boot_mode_scr_3_0);
    data &= 0xf;
    bootinfo->boot_ops |= BOOT_INFO_FROM_GPIO_MASK;
    bootinfo->boot_pin = data;
    return data;
}

static void update_boot_info(boot_info_t *bootinfo)
{
    uint32_t data, pin;

    if (bootinfo->boot_ops & BOOT_INFO_BOOT_PIN_UPDATE_MASK)
        return;

    pin = boot_get_pin_scr_overwrite(bootinfo);

    if (pin == PIN_ERROR)
        pin = boot_get_pin_fuse(bootinfo);
    if (pin == PIN_ERROR)
        pin = boot_get_pin_gpio(bootinfo);

    data = readl(_ioaddr(APB_EFUSEC_BASE + 0x1000 + FUSE_BT_CGF_INDEX0 * 4));
    /*  BT_FUSE0[24], 0 - Enable, 1 - Disable */
    if (data & 1 << 24) {
        bootinfo->boot_ops |= BOOT_INFO_USB_PROVISION_DIS_MASK;
    }
    /* BT_FUSE0[18] */
    if (data & 1 << 18) {
        bootinfo->boot_ops |= BOOT_INFO_SAFETY_HANDOVER_DIS_MASK;
    }
    /* BT_FUSE0[17], 1 - Disable */
    if (data & 1 << 17) {
        bootinfo->boot_ops |= BOOT_INFO_PEER_LOAD_ON_SAF_FAILURE_DIS_MASK;
    }

    bootinfo->boot_ops |= BOOT_INFO_BOOT_PIN_UPDATE_MASK;
}

uint32_t boot_get_pin(void)
{
    update_boot_info(&g_bootinfo);
    return g_bootinfo.boot_pin;
}

bool is_usb_provision(void)
{
    update_boot_info(&g_bootinfo);
    return !(g_bootinfo.boot_ops & BOOT_INFO_USB_PROVISION_DIS_MASK);
}

bool need_peer_load_on_saf_failure(void)
{
    update_boot_info(&g_bootinfo);
    return !(g_bootinfo.boot_ops & BOOT_INFO_PEER_LOAD_ON_SAF_FAILURE_DIS_MASK);
}

/* For both saf and sec, check the boot block info */
bool is_boot_from_peer_load(void)
{
    struct boot_info_block *boot_info = (struct boot_info_block *)_ioaddr(BT_BLOCK_INFO_LOCATION);
    if (boot_info->tag == BOOT_INFO_MAGIC) {
        if (boot_info->boot_device == PEER_BOOT_FROM_BLK) {
            return true;
        }
        return false;
    }
    LTRACEF("bad block info\n");
    return false;
}
