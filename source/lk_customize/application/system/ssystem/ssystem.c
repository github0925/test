/*
 * ssystem.c
 *
 * Copyright (c) 2019 SemiDrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */
#include <app.h>
#include <assert.h>
#include <bits.h>
#include <debug.h>
#include <stdio.h>
#include <lib/console.h>
#include <lib/bytes.h>
#include <lib/reg.h>
#include <lib/sdrv_common_reg.h>

#include "chip_res.h"
#include "clkgen_hal.h"
#include "cpu_hal.h"

#include "rstgen_hal.h"
#include "res.h"
#include "scr_hal.h"
#include "ssystem.h"
#include <firewall_hal.h>
#include <uart_hal.h>
#include <target_res.h>
#include <target.h>
#include <trace.h>
#include <property.h>
#include <dcf.h>
#include "ssystem_configs.h"
#if VERIFIED_BOOT
#include "verified_boot.h"
#endif
#include "mem_image.h"
#include "image_cfg.h"
#include <crypto_hal.h>
#include <sd_aes.h>
#include <sem_hw.h>
#include <wdg_hal.h>
#if ENABLE_PIN_DELTA_CONFIG
#include <boardinfo_hwid_usr.h>
#endif

#if VERIFIED_BOOT
#define SSYSTEM_STACK_SZ  4096
#else
#define SSYSTEM_STACK_SZ  2048
#endif

/* hsm decrypt with aes cbc mode, 128 key
 * todo: swith hardcode key to efuse
 */
int image_descrypt(uint8_t *src, uint8_t *dst, uint32_t size)
{
    int result;
    void *crypto_handle = 0;
    uint8_t iv[] = {
        0xca, 0x69, 0xd8, 0x87, 0x9b, 0xcb, 0x53, 0x9e,
        0xf0, 0x25, 0xc1, 0x95, 0xf3, 0xad, 0x15, 0x17
    };
    uint8_t key[] = {
        0x9f, 0xd7, 0xd6, 0xbe, 0x16, 0xe1, 0xcd, 0x6e,
        0xe2, 0xdb, 0x10, 0x1c, 0x7c, 0xbb, 0x2f, 0xbe
    };
    hal_crypto_creat_handle(&crypto_handle, VCE_ID_GENERAL_SUPPORT_PKA);
    result = hal_aes_init(crypto_handle, SD_FCT_CBC, OPERATION_DEC,
                          HAL_EXT_MEM, key, NULL, 16, iv, 16);

    if (result) {
        hal_crypto_delete_handle(crypto_handle);
        return result;
    }

    result = hal_aes_final(crypto_handle, src, size, 0, dst, size);
    hal_crypto_delete_handle(crypto_handle);
    return result;
}

int hsm_firewall(void);
int hsm_protect(void)
{
    //disable debug
    scr_handle_t handle  = hal_scr_create_handle(SCR_SEC__L31__cr5_sec_dbgen);
    bool res = hal_scr_set(handle, 0x0);
    hal_scr_delete_handle(handle);

    if (!res) {
        dprintf(CRITICAL, "disable secure core debug fail.\n");
        return -1;
    }

    //firewall config for hsm protect
    return hsm_firewall();
}

int scr_init(void);
void firewall_disable(void);
void rid_config_init(void);
void fw_init(void);

int system_config_init(void)
{
    int ret = scr_init();

    if (ret) {
        dprintf(CRITICAL, "scr init fail.\n");
        return -1;
    }

#if !FIREWALL_ENABLE
    firewall_disable();
#else
    fw_init();
#endif

    rid_config_init();
    return ret;
}

static void _ssystem_main(void)
{
    hal_trng_init(NULL,NULL);
    target_quiesce();
}

int ssystem_main(int argc, const cmd_args *argv)
{
    _ssystem_main();
    return 0;
}

status_t ssystem_server_init(void);

static void ssystem_entry(const struct app_descriptor *app, void *args)
{
    /* earlier start rpc service before other part init */
    _ssystem_main();
    ssystem_server_init();
}

static void boot_mp(uint64_t entry)
{
    bool ret;
    void *cpu_handle = NULL;
    ret = hal_cpu_create_handle(&cpu_handle);
    ASSERT(ret);
    hal_cpu_boot(cpu_handle, CPU_ID_MP, entry);
    hal_cpu_release_handle(cpu_handle);
}

static void setup_uart_clk(void)
{
    void *handle = NULL;
    bool ret = false;
    clkgen_app_ip_cfg_t clk = {4, 0, 1};
    ret = hal_clock_creat_handle(&handle);
    ASSERT(ret);
    ret = hal_clock_ipclk_set(handle, RES_IP_SLICE_SEC_UART_SEC0, &clk);
    ASSERT(ret);
    ret = hal_clock_ipclk_set(handle, RES_IP_SLICE_SEC_UART_SEC1, &clk);
    ASSERT(ret);
    hal_clock_release_handle(handle);
}

void os_platform_early_init(void)
{
    mem_image_entry_t info;
    addr_t seeker_base = DIL_IMAGES_MEMBASE;

    setup_uart_clk();

    system_config_init();

    if (mem_image_seek(seeker_base, "hsm_fw", &info)) {
        dprintf(CRITICAL, "can't find hsm image!\n");
    } else {
        if (image_descrypt((uint8_t *)(unsigned int)info.base, (uint8_t *)R5_SEC_TCMB_BASE,
                           info.sz)) {
            dprintf(CRITICAL, "decrypt hsm fail!\n");
        }
    }

#ifdef SUPPORT_FAST_BOOT
#if defined(PLATFORM_G9X)||defined(PLATFORM_G9Q)
    // TODO: firewall early config
    sdrv_common_reg_set_value(SDRV_REG_STATUS,
                              SDRV_REG_STATUS_FIREWALL_EARLY_DONE,
                              SDRV_REG_STATUS_FIREWALL_EARLY_DONE);

    while (1) {
        if (sdrv_common_reg_get_value(SDRV_REG_STATUS,
                                      SDRV_REG_STATUS_SDPE_LD_DONE)) {
            break;
        }
    }

    if (image_descrypt((uint8_t *)SDPE_MEMBASE, (uint8_t *)SDPE_MEMBASE,
                       SDPE_MEMSIZE))
        dprintf(CRITICAL, "decrypt sdpe fail!\n");

    boot_mp(SDPE_MEMBASE);
#endif
#endif
}

// LK console cmd
#if defined(WITH_LIB_CONSOLE)
#include <lib/console.h>

STATIC_COMMAND_START STATIC_COMMAND("ssystem", "Secure R5 ssystem",
                                    (console_cmd)&ssystem_main)
STATIC_COMMAND_END(ssystem);

#endif

APP_START(ssystem)
.flags = APP_FLAG_CUSTOM_STACK_SIZE,
.stack_size = SSYSTEM_STACK_SZ,
.entry = ssystem_entry,
APP_END
