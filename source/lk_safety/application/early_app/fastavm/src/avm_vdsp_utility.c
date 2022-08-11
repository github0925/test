/*
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
#include <lib/elf.h>

#include "chip_res.h"
#include "clkgen_hal.h"
#include "rstgen_hal.h"
#include "res.h"
#include "scr_hal.h"

static void vdsp_core_rst(void)
{
    addr_t phy_addr = 0;
    int32_t idx = 0;
    ASSERT(!res_get_info_by_id(RES_CORE_RST_SEC_VDSP_SW, &phy_addr, &idx));
    rstgen_core_reset(phy_addr,2);
    return;
}

void vdsp_rst_stall(uint32_t vector_base)
{
    scr_handle_t handle_altvec;
    scr_handle_t handle_vecsel;
    scr_handle_t handle_stall;
    int ret = 0;

    handle_altvec = hal_scr_create_handle(SCR_SEC__L31__vdsp_altresetvec_31_4);
    handle_vecsel = hal_scr_create_handle(SCR_SEC__R16W16__vdsp_statvectorsel);
    handle_stall  = hal_scr_create_handle(SCR_SEC__R16W16__vdsp_runstall);

    hal_scr_get(handle_altvec);
    hal_scr_get(handle_vecsel);
    hal_scr_get(handle_stall);


    uint32_t addr = vector_base;
    dprintf(INFO, "start from 0x%x\n", addr);

    ret = hal_scr_set(handle_altvec, (uint32_t)(addr >> 4));
    ASSERT(ret);

    ret = hal_scr_set(handle_vecsel, (uint32_t)(1));
    ASSERT(ret);

    ret = hal_scr_set(handle_stall, (uint32_t)(1));
    ASSERT(ret);

    hal_scr_delete_handle(handle_altvec);
    hal_scr_delete_handle(handle_vecsel);
    hal_scr_delete_handle(handle_stall);

    vdsp_core_rst();
}

void vdsp_go(void)
{
    scr_handle_t handle_stall;
    int ret = 0;

    handle_stall  = hal_scr_create_handle(SCR_SEC__R16W16__vdsp_runstall);

    hal_scr_get(handle_stall);

    ret = hal_scr_set(handle_stall, (uint32_t)(0));
    ASSERT(ret);

    hal_scr_delete_handle(handle_stall);

}

/* load elf exec file to vdsp dram & ddr */
int vdsp_load_firmware(uint32_t elf_base,uint32_t elf_len, void* dummy_binary_base)
{
    (void)dummy_binary_base;

    status_t ret = 0;

    uint32_t start = elf_base;
    uint32_t len   = elf_len;
    dprintf(INFO, "start from 0x%x, len=0x%x\n", start, len);

    elf_handle_t elf;
    ret = elf_open_handle_memory(&elf, (void *)start, len);
    if (ret < 0) {
        dprintf(CRITICAL, "unable to open elf handle\n");
        return ret;
    }

    ret = elf_load(&elf);
    if (ret < 0) {
        dprintf(CRITICAL, "elf processing failed, ret: %d\n", ret);
    }
    elf_close_handle(&elf);

    return ret;
}
