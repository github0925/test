/*
 * Copyright (c) 2019 Semidrive Semiconductor.
 * All rights reserved.
 *
 **/
#include <err.h>
#include <stdio.h>
#include <stdint.h>
#include <app.h>
#include <platform.h>
#include <string.h>
#include <debug.h>
#include <arch.h>
#include <lib/console.h>
#include <lib/reg.h>

#ifndef BACKDOOR_DDR
#include <libfdt.h>
#include "partition_parser.h"
#include "ab_partition_parser.h"
#include "storage_device.h"
#include <mmc_hal.h>
#endif

#include "smc.h"
#include "target_res.h"
#include "bootloader_qnx.h"
#include "bootimg.h"

#define QNX_STARTUP_BASE    KERNEL_LOAD_ADDR
#define QNX_IMAGESCAN_SIZE  (1024 * 1024)

#ifndef BACKDOOR_DDR
/* Standard part name for qnx */
static const char *boot_part = "qnx";
static const char *hypervisor_part = "vmcfg";
#endif

static bool is_hypervisor = false;

static void jump_to_kernel(struct startup_header *startup_hdr)
{
    /* Boot kernel by chain load */
    unsigned long start_addr = startup_hdr->startup_vaddr;
    dprintf(DEFAULT_LOGLEVEL, "qnx vaddr is 0x%lx\n", start_addr);
#if WITH_KERNEL_VM
    uint32_t temp = 0;
    void *entry_qnx = (void *)_ioaddr((paddr_t)(start_addr));

    arch_flush_dcache_all();

    if (is_hypervisor) {
        dprintf(DEFAULT_LOGLEVEL, "jump to EL2 @%p\n", entry_qnx);

        arch_disable_cache(UCACHE);

        /* disable MMU */
        __asm__ volatile("mrs %0, sctlr_el1" :"=r"(temp)::);
        temp &= ~(0x01u);
        __asm__ volatile("msr sctlr_el1, %0" ::"r"(temp):);

        hvc(start_addr, 0, 0, 0, 0, 0, 0, 0);
    }
    else {
        /* No Hypervisor, disable hvc call by clearing SCR_EL3.HCE */
        /* This smc call shall be implemented by ATF */
        /* If ATF is not persistent, preloader's vector will handle and return simply */
        smc(SMC_DIS_HCE, 0, 0, 0, 0, 0, 0, 0);

        dprintf(DEFAULT_LOGLEVEL, "jump to EL1 %p\n", entry_qnx);
        /* AP2 has no ATF, should not call smc or hvc */
        arch_chain_load(entry_qnx, 0, 0, 0, 0);
    }

#else
    dprintf(DEFAULT_LOGLEVEL, "jump to 0x%lx\n", start_addr);
    arch_chain_load((void *)start_addr, 0, 0, 0, 0);
#endif
}

#ifndef BACKDOOR_DDR
static int parse_qnx_bootimage(struct startup_header *startup_hdr,
                               partition_device_t *ptdev)
{
    int ret;
    storage_device_t *storage_dev = ptdev->storage;
    uint32_t block_size =  storage_dev->get_block_size(storage_dev);
    /* Load bootimage head */
    int boot_ptn = ptdev_get_offset(ptdev, boot_part);
    uint64_t boot_size = ptdev_get_size(ptdev, boot_part);
    int hypervisor_ptn = ptdev_get_offset(ptdev, hypervisor_part);
    uint64_t hypervisor_size = ptdev_get_size(ptdev, hypervisor_part);
    uint64_t check_size = 0;
    unsigned long image_addr = 0;
    unsigned long ram_base = 0;
    void *tmp_buf ;

    if (!boot_ptn || !boot_size) {
        dprintf(DEFAULT_LOGLEVEL, "no qnx partition found.\n");
        return -1;
    }

    if  (hypervisor_ptn && hypervisor_size) {
        is_hypervisor = true;
    }
    else {
        is_hypervisor = false;
    }

    tmp_buf = memalign(block_size, ROUNDUP(sizeof(struct startup_header),
                                           block_size));
#if WITH_KERNEL_VM
    ram_base = _ioaddr((paddr_t)(QNX_STARTUP_BASE));
#else
    ram_base = QNX_STARTUP_BASE;
#endif

    while (check_size < boot_size) {

        ret = storage_dev->read(storage_dev, boot_ptn + check_size, (uint8_t *)tmp_buf,
                                block_size);

        if (ret) {
            dprintf(DEFAULT_LOGLEVEL, "failed to read qnx partition.\n");
            return -1;
        }

        /* image_addr is the start address of last valid startup_hdr */
        image_addr = image_scan(startup_hdr, (unsigned long)tmp_buf,
                                (unsigned long)tmp_buf + block_size);
        dprintf(DEFAULT_LOGLEVEL, "image:0x%lx, tmp_buf:%p\n", image_addr, tmp_buf);

        if (image_addr != (unsigned long)(-1)) {
            dprintf(DEFAULT_LOGLEVEL, "found qnx image header:at 0x%lx\n", image_addr);
            memcpy ((unsigned char *)startup_hdr, (unsigned char *)image_addr,
                    sizeof(struct startup_header));
            break;

        }

        check_size += block_size;
    }

    free(tmp_buf);

    if (check_size >= boot_size) {
        dprintf(DEFAULT_LOGLEVEL, "not qnx ifs image header found\n");
        return -1;
    }

    check_size += image_addr - (unsigned long)tmp_buf;

    dprintf(DEFAULT_LOGLEVEL,
            "ram_paddr %x, paddr_bias 0x%x, startup_size:0x%x, preboot_size:0x%x, stored_size:0x%x\n",
            startup_hdr->ram_paddr,
            startup_hdr->paddr_bias,
            startup_hdr->startup_size,
            startup_hdr->preboot_size,
            startup_hdr->stored_size
           );

    /* copy the whole image to ram base */
    ret = storage_dev->read(storage_dev, boot_ptn, (uint8_t *)ram_base,
                            ALIGN((startup_hdr->preboot_size + startup_hdr->stored_size), block_size));

    if (ret) {
        dprintf(DEFAULT_LOGLEVEL, "failed to read ifs image \n");
        return -1;
    }

    dprintf(DEFAULT_LOGLEVEL, "read kernel to ram_base=0x%lx 0x%x\n",
            ram_base, ALIGN((startup_hdr->preboot_size + startup_hdr->stored_size),
                            block_size));

    /* copy startup to the specified ram and setup imagefs_paddr */
    image_setup(startup_hdr, ram_base + startup_hdr->preboot_size);

    return 0;
}

int bootloader_entry_qnx(partition_device_t *ptdev,
                         struct startup_header *startup_hdr)
{
    int ret = 0;

    ret = parse_qnx_bootimage(startup_hdr, ptdev);

    if (ret) {
        dprintf(DEFAULT_LOGLEVEL, "failed load and parse qnx bootimage.\n");
        free(startup_hdr);
        return ret;
    }
    else {
        jump_to_kernel(startup_hdr);
        return 0;
    }
}

#else
/* QNX kernel image is already loaded to address QNX_STARTUP_BASE
 * by chain loader previously.
 */
static int parse_kernel_image(struct startup_header *startup_hdr)
{
    unsigned long image_addr = 0;
    unsigned long ram_base = 0;

#if WITH_KERNEL_VM
    ram_base = _ioaddr((paddr_t)(QNX_STARTUP_BASE));
#else
    ram_base = QNX_STARTUP_BASE;
#endif

    /* image_addr is the start address of last valid startup_hdr */
    image_addr = image_scan(startup_hdr, (unsigned long)ram_base,
                            ram_base + QNX_IMAGESCAN_SIZE);

    if (image_addr == (unsigned long)(-1)) {
        dprintf(DEFAULT_LOGLEVEL, "ERR: qnx kernel image NOT FOUND\n");
        return -1;
    }

    dprintf(DEFAULT_LOGLEVEL, "qnx kernel image found at 0x%lx\n", image_addr);
    memcpy((void *)startup_hdr, (void *)image_addr, sizeof(struct startup_header));

    dprintf(DEFAULT_LOGLEVEL,
            "ram_paddr %x, paddr_bias 0x%x, startup_size:0x%x, preboot_size:0x%x, stored_size:0x%x\n",
            startup_hdr->ram_paddr,
            startup_hdr->paddr_bias,
            startup_hdr->startup_size,
            startup_hdr->preboot_size,
            startup_hdr->stored_size
           );
    /* the whole image begins from ram base */
    dprintf(DEFAULT_LOGLEVEL, "kernel image (0x%lx 0x%x)\n", ram_base,
            (startup_hdr->preboot_size + startup_hdr->stored_size));

    /* copy startup to the specified ram and setup imagefs_paddr */
    image_setup(startup_hdr, ram_base + startup_hdr->preboot_size);

    return 0;
}

int bootloader_entry_qnx_backdoor_ddr(void)
{
    struct startup_header startup_hdr;

    /* Boot kernel by chain load */
    if (parse_kernel_image(&startup_hdr) == 0) {
        jump_to_kernel(&startup_hdr);
        return 0;
    }
    else {
        return -1;
    }
}
#endif
