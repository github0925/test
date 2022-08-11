/*
 * Copyright (c) 2019 Semidrive Semiconductor.
 * All rights reserved.
 *
 **/

#include <app.h>
#include <lib/console.h>
#include <platform.h>
#include <string.h>
#include <lib/reg.h>
#include <trace.h>
#include <libfdt.h>
#include <image_cfg.h>
#ifndef BACKDOOR_DDR
#include "partition_parser.h"
#include "ab_partition_parser.h"
#include "bootimg.h"
#include "bootloader_configs.h"
#include "bootloader_message.h"
#include "storage_device.h"
#include <mmc_hal.h>
#include "boot.h"
#include "boot_device_cfg.h"
#include <arch/defines.h>
#include "libavb.h"
#include "ufdt_overlay.h"
#include "libufdt_sysdeps.h"
#include "sdrv_dtmapping.h"
#if SUPPORT_BOARDINFO
#include "boardinfo_hwid_usr.h"
#include "boardinfo_common.h"
#endif
#include "dt_table.h"
#endif
#include "lib/reboot.h"
#include "lib/sdrv_common_reg.h"
#include "smc.h"
#include "target_res.h"
#include "verified_boot.h"
#include "bootloader_qnx.h"

#define DEVICE_TREE_MAX_DEPTH 16
#ifndef BACKDOOR_DDR

#define BOOT_INFO_BTDEV_START  8
#define BOOT_EMMC1_CODE        0x1
#define BOOT_EMMC2_CODE        0x2
#define BOOT_OSPI2_CODE        0x11
#define BOOT_USB_CODE          0x30
#define BOOT_PEER_CODE         0x40
#define SD_BOOT                0xf

#define DTB_PAD_SIZE   1024
#define PART_USER 0

#define MEMORY_BANKS_MAX 4
#define DM_TABLE_MAX_SIZE  1024
#define DM_VERITY_BLK_DEV_PLACEHOLDER  "dm-rd-placeholder"
#define DM_FIELD_SEP " "
#define ROOT_DEVICE_KEY "root="
#define DM_ROOT_DEVICE  "root=/dev/dm-0"
#define DOM0_KERNEL_OFFSET 0x80000
#define XEN_FDT_OFFSET 0x8000000
#define XEN_IMG_OFFSET 0x8200000
#define RECOVERY_PARTITION_NAME "recovery"

#ifndef DDR_SIZE
#define DDR_SIZE 0
#endif

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(array)   (sizeof(array) / sizeof(array[0]))
#endif

#define EXCHANGE_ASCEND_MEM_INFO(a, b) \
    do{                                \
        if (a.base > b.base)           \
        {                              \
            mem_usage_info temp;       \
            temp = a;                  \
            a = b;                     \
            b = temp;                  \
        }                              \
    }while(0)

#define LOCAL_TRACE 1

typedef struct mem_usage_info {
    uint64_t size;
    uint64_t offset;
    uint8_t *base;
} mem_usage_info;

typedef struct dtbo_img_info {
    uint8_t *base;
    uint64_t size;
    char pt_name[MAX_GPT_NAME_SIZE];
} dtbo_img_info;

/* Standard part name for xen */
static const char *xen_part = "xen";
static const char *dom0_kernel = "dom0_kernel";
static const char *dom0_rootfs = "dom0_rootfs";

/* Standard part name for android & linux */
static const char *boot_part = "boot";
static const char *dtb_part = "dtb";

/* Customized part name for Linux */
static const char *alias_dtb = ALIAS_DTB_PARTITION;
static const char *alias_rootfs_part = ALIAS_ROOTFS_PARTITION;
static const char *alias_kernel_part = ALIAS_KERNEL_PARTITION;

static const char *partition_separator = PART_SEPARAT;

/* For cmdline part */
static const char *hide_part = " blkdevparts=";

static char *vbmeta_cmdline = NULL;
static char *dm_table = NULL;
static const char *disk_name;
#endif

static const void *getprop(const void *fdt, const char *node_path,
                           const char *property, int *len)
{
    int offset = fdt_path_offset(fdt, node_path);

    if (offset == -FDT_ERR_NOTFOUND) {
        dprintf(CRITICAL, "node no found\n");
        return NULL;
    }

    return fdt_getprop(fdt, offset, property, len);
}

static int fdt_find_node_name(void *fdt, const char *match)
{
    int node;
    int depth;
    size_t match_len;

    for (node = 0, depth = 0;
            node >= 0 && depth >= 0;
            node = fdt_next_node(fdt, node, &depth)) {
        const char *name = fdt_get_name(fdt, node, NULL);

        if ( depth >= DEVICE_TREE_MAX_DEPTH ) {
            printf("Warning: device tree node `%s' is nested too deep\n",
                   name);
            continue;
        }

        match_len = strlen(match);

        if (strncmp(name, match, match_len) == 0 &&
                (name[match_len] == '@' || name[match_len] == '\0')) {
            return node;
        }
    }

    return 0;
}

static uint64_t get_uint_from_fdt_by_name(void *fdt,
        const char *node_name, const char *prop_name)
{
    uint64_t v = 0;
    int node, len;
    const void *prop_value_p;

    if (!fdt || !prop_name || !node_name)
        goto out;

    fdt_open_into(fdt, fdt, fdt_totalsize(fdt));
    node = fdt_find_node_name(fdt, node_name);

    if (node < 0) {
        dprintf(CRITICAL, "fdt %s node no found.\n", node_name);
        goto out;
    }

    prop_value_p = fdt_getprop(fdt, node, prop_name, &len);

    if (!prop_value_p) {
        dprintf(CRITICAL, "fdt %s not found.\n", prop_name);
        goto out;
    }

    if (len == 4) {
        v = (uint64_t)fdt32_to_cpu(*(uint32_t *)prop_value_p);
    }
    else if (len == 8) {
        v = fdt64_to_cpu(*(uint64_t *)prop_value_p);
    }

out:
    return v;
}

#ifndef BACKDOOR_DDR
static int fdt_find_node_compatible(void *fdt, const char *compatible)
{
    int node, depth, len, l;
    const void *prop;

    for (node = 0, depth = 0;
            node >= 0 && depth >= 0;
            node = fdt_next_node(fdt, node, &depth)) {
        const char *name = fdt_get_name(fdt, node, NULL);

        if (depth >= DEVICE_TREE_MAX_DEPTH ) {
            printf("Warning: device tree node `%s' is nested too deep\n",
                   name);
            continue;
        }

        prop = fdt_getprop(fdt, node, "compatible", &len);

        if (prop == NULL ) {
            continue;
        }

        while (len > 0) {
            if (!strcmp(prop, compatible))
                return node;

            l = strlen(prop) + 1;
            prop += l;
            len -= l;
        }
    }

    return 0;
}

static int fdt_find_or_add_subnode(void *fdt, int parentoffset,
                                   const char *name)
{
    int offset;

    offset = fdt_subnode_offset(fdt, parentoffset, name);

    if (offset == -FDT_ERR_NOTFOUND)
        offset = fdt_add_subnode(fdt, parentoffset, name);

    if (offset < 0)
        printf("%s: %s: %s\n", __func__, name, fdt_strerror(offset));

    return offset;
}

static int fdt_pack_reg(const void *fdt, void *buf, uint64_t *address,
                        uint64_t *size,
                        int n, int nodeoffset)
{
    int i;
    int address_cells = fdt_address_cells(fdt, nodeoffset);
    int size_cells = fdt_size_cells(fdt, nodeoffset);
    char *p = buf;

    for (i = 0; i < n; i++) {
        if (address_cells == 2)
            *(fdt64_t *)p = cpu_to_fdt64(address[i]);
        else
            *(fdt32_t *)p = cpu_to_fdt32(address[i]);

        p += 4 * address_cells;

        if (size_cells == 2)
            *(fdt64_t *)p = cpu_to_fdt64(size[i]);
        else
            *(fdt32_t *)p = cpu_to_fdt32(size[i]);

        p += 4 * size_cells;
    }

    return p - (char *)buf;
}

/* return the num of memory banks */
static int fdt_get_mem_node(void *blob, uint64_t *addr, uint64_t *len)
{
    int nodeoffset, lenp;
    unsigned i, val = 0;
    uint64_t r;
    int root_addr_cells, root_size_cells, addr_cell, size_cell;
    nodeoffset = fdt_find_node_name(blob, "memory");

    if (nodeoffset < 0)
        return nodeoffset;

    const uint32_t *reg = fdt_getprop(blob, nodeoffset, "reg", &lenp);
    root_addr_cells = fdt_address_cells(blob, 0);
    root_size_cells = fdt_size_cells(blob, 0);

    if (reg) {
        for (i = 0; i < lenp / sizeof(uint32_t);) {
            r = 0;
            addr_cell = root_addr_cells;
            size_cell = root_size_cells;

            while (addr_cell--) {
                r = (r << 32) | fdt32_to_cpu(reg[i++]);
            }

            *addr++ = r;
            r = 0;

            while (size_cell--) {
                r = (r << 32) | fdt32_to_cpu(reg[i++]);
            }

            *len++ = r;
            val++;
        }
    }

    return val;
}

int fdt_fixup_memory_banks(void *blob, uint64_t start[], uint64_t size[],
                           int banks)
{
    int err, nodeoffset;
    int len;
    uint8_t tmp[MEMORY_BANKS_MAX * 16]; /* Up to 64-bit address + 64-bit size */

    if (banks > MEMORY_BANKS_MAX) {
        printf("%s: num banks %d exceeds hardcoded limit %d."
               " Recompile with higher MEMORY_BANKS_MAX?\n",
               __FUNCTION__, banks, MEMORY_BANKS_MAX);
        return -1;
    }

    err = fdt_check_header(blob);

    if (err < 0) {
        printf("%s: %s\n", __FUNCTION__, fdt_strerror(err));
        return err;
    }

    /* find or create "/memory" node. */
    nodeoffset = fdt_find_node_name(blob, "memory");

    if (nodeoffset < 0)
        return nodeoffset;

    err = fdt_setprop(blob, nodeoffset, "device_type", "memory",
                      sizeof("memory"));

    if (err < 0) {
        printf("WARNING: could not set %s %s.\n", "device_type",
               fdt_strerror(err));
        return err;
    }

    len = fdt_pack_reg(blob, tmp, start, size, banks, nodeoffset);

    err = fdt_setprop(blob, nodeoffset, "reg", tmp, len);

    if (err < 0) {
        printf("WARNING: could not set %s %s.\n",
               "reg", fdt_strerror(err));
        return err;
    }

    return 0;
}

/* Update the memory node on dtb with specific parameter.
 * If the parameter is not specified, follow memory layout declared.
 * And if the memory layout is not specified either, shall not change dtb.
*/
static int fdt_update_memory_range(void *fdt, uint64_t start, uint64_t size,
                                   uint64_t ddr_size)
{
    int ret;
    uint64_t dtb_mem_start, dtb_mem_size;
    uint64_t memory_start[MEMORY_BANKS_MAX] = {0};
    uint64_t memory_size[MEMORY_BANKS_MAX] = {0};

    dprintf(INFO, "ddr size is 0x%llx\n", ddr_size);
    /* Get memory node from dtb */
    ret = fdt_get_mem_node(fdt, memory_start, memory_size);

    if (ret < 0) {
        printf("failed to get memory node for dtb\n");
        return ret;
    }

    dtb_mem_start = memory_start[0];
    dtb_mem_size = memory_size[0];

    if (start) {
        memory_start[0] = start;
    }
    else {
        /* If memory addr not specified, follow memory layout declared */
        if (REE_MEMBASE) {
            memory_start[0] = REE_MEMBASE;
        }
        else {
            return -1;
        }
    }

    if (!size && ddr_size)
        size = ddr_size + 0x40000000 - memory_start[0];

    if (size) {
        memory_size[0] = size;
    }
    else {
        /* If memory size not specified, refer to the size dtb declared */
        if ( memory_start[0] >= dtb_mem_start ) {
            memory_size[0] = dtb_mem_size - (memory_start[0] - dtb_mem_start);
        }
        else {
            memory_size[0] = dtb_mem_size + (dtb_mem_start - memory_start[0]);
        }
    }

    /* Update dtb memory node */
    ret = fdt_fixup_memory_banks(fdt, memory_start, memory_size, ret);
    return ret;
}

static int update_fdt_initrd(void *fdt, unsigned initrd_addr,
                             unsigned initrd_size)
{
    int status, chosen_node;
    unsigned initrd_start, initrd_end;

    status = fdt_open_into(fdt, fdt, fdt_totalsize(fdt));
    chosen_node = fdt_find_node_name(fdt, "chosen");

    if (chosen_node < 0) {
        dprintf(CRITICAL, "fdt chosen node no found.\n");
        return -1;
    }

    initrd_start = cpu_to_fdt32(initrd_addr);
    status = fdt_setprop(fdt, chosen_node, "linux,initrd-start",
                         &initrd_start, sizeof(unsigned));

    if (status) {
        dprintf(CRITICAL, "set initrd-start failed.\n");
        return status;
    }

    initrd_end = cpu_to_fdt32(initrd_addr + initrd_size);

    status = fdt_setprop(fdt, chosen_node, "linux,initrd-end",
                         &initrd_end, sizeof(unsigned));

    if (status) {
        dprintf(CRITICAL, "set initrd-end failed.\n");
        return status;
    }

    fdt_pack(fdt);
    return 0;
}

static int update_fdt_module(void *fdt, uint64_t *addr, uint64_t *size,
                             int banks)
{
    uint8_t tmp[MEMORY_BANKS_MAX * 16]; /* Up to 64-bit address + 64-bit size */
    int status, chosen_node, module_node, len, i;

    status = fdt_open_into(fdt, fdt, fdt_totalsize(fdt));

    if (status) {
        dprintf(CRITICAL, "open fdt failed.\n");
        return status;
    }

    chosen_node = fdt_find_node_name(fdt, "chosen");

    module_node = fdt_subnode_offset(fdt, chosen_node, "module");

    for (i = 0; i < banks; i++) {
        if (addr[i] == 0 && size[i] == 0)
            break;
    }

    banks = i;

    if (!banks)
        return 0;


    len = fdt_pack_reg(fdt, tmp, addr, size, banks, module_node);

    return fdt_setprop(fdt, module_node, "reg", tmp, len);
}

static bool check_xen_compatible(void *fdt)
{
    if (fdt_find_node_compatible(fdt, "xen,multiboot-module") ||
            fdt_find_node_compatible(fdt, "multiboot,module")) {
        printf("find xen compatible!\n");
        return true;
    }

    return false;
}

static bool check_addr_overlap(addr_t start, uint64_t size)
{
    addr_t self = (addr_t)_ioaddr(MEMBASE);

    /* overflow */
    if (start + size <= start) {
        return false;
    }

    if (start >= (self + MEMSIZE)
            || ((start + size) < self)) {
        return true;
    }

    return false;
}

static void hex2str(const uint8_t *hex, uint32_t hex_len, char *str,
                    uint32_t str_len)
{
    const char table[] = "0123456789abcdef\0";
    uint8_t v = 0;

    if (!hex || !str || !hex_len || !str_len || str_len < hex_len * 2)
        return;

    for (uint32_t i = 0; i < hex_len; i++) {
        v = hex[i];
        *str++ = table[v >> 4];
        *str++ = table[v & 0xF];
    }
}

static char *setup_dm_root_device(AvbSlotVerifyData *slot_data)
{
    AvbDescriptor desc;
    uint32_t vbmeta_num;
    size_t num_descriptors;
    size_t vbmeta_size;
    AvbVBMetaData *vbmeta_image;
    AvbHashtreeDescriptor hashtree_desc;
    const AvbDescriptor **descriptors = NULL;
    char part_name[MAX_GPT_NAME_SIZE];
    const uint8_t *desc_partition_name = NULL;
    char fec_args[DM_TABLE_MAX_SIZE / 2] = {0};
    const uint8_t *root_digest, *salt;
    char *root_digest_str = NULL, *salt_str = NULL;

    if (!slot_data)
        return NULL;

    vbmeta_num = slot_data->num_vbmeta_images;

    for (uint32_t i = 0; i < vbmeta_num; i++) {
        vbmeta_image = slot_data->vbmeta_images;
        vbmeta_size = vbmeta_image->vbmeta_size;
        descriptors = avb_descriptor_get_all(vbmeta_image->vbmeta_data,
                                             vbmeta_size, &num_descriptors);

        for (uint32_t n = 0; n < num_descriptors; n++) {

            if (!avb_descriptor_validate_and_byteswap(descriptors[n], &desc))
                continue;

            if  (desc.tag != AVB_DESCRIPTOR_TAG_HASHTREE)
                continue;

            if (!avb_hashtree_descriptor_validate_and_byteswap(
                        (AvbHashtreeDescriptor *)descriptors[n], &hashtree_desc))
                continue;

            desc_partition_name = ((const uint8_t *)descriptors[n])
                                  + sizeof(AvbHashtreeDescriptor);

            if (!avb_validate_utf8(desc_partition_name,
                                   hashtree_desc.partition_name_len))
                continue;

            memcpy(part_name, desc_partition_name, hashtree_desc.partition_name_len);
            part_name[hashtree_desc.partition_name_len] = '\0';

            if (memcmp(alias_rootfs_part, part_name, hashtree_desc.partition_name_len))
                continue;

            dm_table = calloc(1, DM_TABLE_MAX_SIZE);

            if (!dm_table)
                goto fail;

            root_digest_str = calloc(1, hashtree_desc.root_digest_len * 2 + 1);
            salt_str = calloc(1, hashtree_desc.salt_len * 2 + 1);

            if (!root_digest_str || !salt_str)
                goto fail;

            salt = ((const uint8_t *)descriptors[n])
                   + sizeof(AvbHashtreeDescriptor)
                   + hashtree_desc.partition_name_len;

            root_digest = ((const uint8_t *)descriptors[n])
                          + sizeof(AvbHashtreeDescriptor)
                          + hashtree_desc.partition_name_len
                          + hashtree_desc.salt_len;

            hex2str(salt, hashtree_desc.salt_len,
                    salt_str, hashtree_desc.salt_len * 2);

            hex2str(root_digest, hashtree_desc.root_digest_len,
                    root_digest_str, hashtree_desc.root_digest_len * 2);

            if (hashtree_desc.fec_size > 0)
                snprintf(fec_args, sizeof(fec_args),
                         "10 restart_on_corruption ignore_zero_blocks "
                         "use_fec_from_device %s fec_roots %u fec_blocks %llu fec_start %llu",
                         DM_VERITY_BLK_DEV_PLACEHOLDER, hashtree_desc.fec_num_roots,
                         hashtree_desc.fec_offset / hashtree_desc.data_block_size,
                         hashtree_desc.fec_offset / hashtree_desc.data_block_size);

            snprintf(dm_table, DM_TABLE_MAX_SIZE, "%s dm=\"1 vroot none ro 1,"
                     "0 %llu verity %u %s %s %u %u %llu %llu %s %s %s %s\"", DM_ROOT_DEVICE,
                     hashtree_desc.image_size / 512,  hashtree_desc.dm_verity_version,
                     DM_VERITY_BLK_DEV_PLACEHOLDER, DM_VERITY_BLK_DEV_PLACEHOLDER,
                     hashtree_desc.data_block_size, hashtree_desc.hash_block_size,
                     hashtree_desc.image_size / hashtree_desc.data_block_size,
                     hashtree_desc.tree_offset / hashtree_desc.hash_block_size,
                     hashtree_desc.hash_algorithm,
                     root_digest_str, salt_str, fec_args);

            return dm_table;
        }
    }

fail:

    if (dm_table)
        free(dm_table);

    if (root_digest_str)
        free(root_digest_str);

    if (salt_str)
        free(salt_str);

    return NULL;
}

static bool get_cmdline_by_verified_image(partition_device_t *ptdev,
        struct list_node *head)
{
#if VERIFIED_BOOT
    AvbSlotVerifyData *slot_data = NULL;

    if (!ptdev || !head) {
        dprintf(CRITICAL, "invalid params for verify image\n");
        return false;
    }

    if (!vbmeta_cmdline) {
        if (!verify_loaded_images(ptdev, head, &slot_data)) {
            dprintf(CRITICAL, "verify partition fail\n");
            return false;
        }

        vbmeta_cmdline = strdup(slot_data->cmdline);

        if (!vbmeta_cmdline) {
            dprintf(CRITICAL, "allocate memory for vbmeta cmdline fail!\n");
            avb_slot_verify_data_free(slot_data);
            return false;
        }

        dprintf(INFO, "vbmeta_cmdline:%s\n", vbmeta_cmdline);
        setup_dm_root_device(slot_data);
        avb_slot_verify_data_free(slot_data);
    }
    else if (!verify_loaded_images(ptdev, head, NULL)) {
        dprintf(CRITICAL, "verify partition fail\n");
        return false;
    }

#endif
    return true;
}

static uint8_t *find_free_space(uint8_t *start, uint64_t total_sz,
                                mem_usage_info *mem_usage_sorted,
                                uint32_t mem_usage_count,
                                uint64_t request_sz)
{
    uint8_t *min_base = mem_usage_sorted[0].base;
    uint8_t *max_base = mem_usage_sorted[mem_usage_count - 1].base +
                        mem_usage_sorted[mem_usage_count - 1].size;
    uint8_t *result = NULL;
    mem_usage_info *free_space = NULL;

    if (start > min_base
            || (start + total_sz) < max_base)
        return NULL;

    free_space = calloc(1, sizeof(mem_usage_info) * (mem_usage_count + 1));

    if (!free_space)
        goto out;

    free_space[0].base = start;
    free_space[0].size = mem_usage_sorted[0].base - start;

    free_space[mem_usage_count].base = mem_usage_sorted[mem_usage_count - 1].base +
                                       mem_usage_sorted[mem_usage_count - 1].size;
    free_space[mem_usage_count].size = start + total_sz -
                                       free_space[mem_usage_count].base;

    for (uint32_t i = 0; i < mem_usage_count - 1; i++) {
        free_space[i + 1].base = mem_usage_sorted[i].base + mem_usage_sorted[i].size;
        free_space[i + 1].size = mem_usage_sorted[i + 1].base - free_space[i + 1].base;
    }

    for (uint32_t i = 0; i < mem_usage_count + 1; i++) {
        if (free_space[i].size >= request_sz)
            result = free_space[i].base;
    }

out:

    if (free_space)
        free(free_space);

    return result;
}

static bool verify_bootimage(partition_device_t *ptdev,
                             const char *partition,
                             uint8_t **load_addr,
                             uint64_t *load_size,
                             mem_usage_info *mem_usage_info,
                             uint32_t mem_count,
                             bool *no_overlap)
{
    uint8_t *buf = NULL;
    uint32_t block_size = 0;
    uint64_t round = 0;
    AvbFooter footer = {0};
    uint64_t original_image_size = 0;
    storage_device_t *storage = NULL;
    uint64_t ptn = ptdev_get_offset(ptdev, partition);
    uint64_t pt_size = ptdev_get_size(ptdev, partition);

    struct list_node head;
    struct image_load_info img_info = {0};

    if (!avb_get_footer_from_partition(ptdev, partition, &footer)) {
        dprintf(CRITICAL, "get boot partition footer fail\n");
        return false;
    }

    if (!ptn || !pt_size) {
        dprintf(CRITICAL, "no boot partition found.\n");
        return false;
    }

    storage = ptdev->storage;
    block_size =  storage->get_block_size(storage);
    original_image_size = footer.original_image_size;
    round = round_up(original_image_size, block_size);

    buf = (uint8_t *)_ioaddr(REE_MEMBASE);

    *no_overlap = true;
    buf = find_free_space(buf, REE_MEMSIZE, mem_usage_info, mem_count, round);

    if (!buf) {
        buf = (uint8_t *)_ioaddr(REE_MEMBASE);
        *no_overlap = false;
    }

    if (!check_addr_overlap((addr_t)buf, round) || round > REE_MEMSIZE) {
        *no_overlap = false;
        buf = (uint8_t *)_ioaddr(REE_MEMBASE + (REE_MEMSIZE - round));

        if (!check_addr_overlap((addr_t)buf, round) || round > REE_MEMSIZE) {
            dprintf(CRITICAL, "no enough memory to verify bootimage.\n");
            return false;
        }
    }

    if (storage->read(storage, ptn, buf, round_up(original_image_size,
                      block_size))) {
        dprintf(CRITICAL, "read bootimage fail!\n");
        return false;
    }

    img_info.addr = (addr_t)buf;
    img_info.size = original_image_size;
    img_info.name = "boot";
    list_initialize(&head);
    list_add_tail(&head, &img_info.node);

    if (load_addr)
        *load_addr = buf;

    if (load_size)
        *load_size = original_image_size;

    /* Do not use "ivi$boot" or "ap2$boot".
     * Becase the partition name is "boot" in the boot hash descriptor included from
     * vbmeta image which is created by Android build system
     * where the name of boot is not "ivi$boot" or "ap2$boot", but "boot"
     */
    return get_cmdline_by_verified_image(ptdev, &head);
}

static bool is_recovery(void)
{
    reboot_args_t reboot_args;
    static uint32_t recovery = 0;

    if (recovery & 0x1u)
        return (recovery >> 0x1u);

    reboot_args.val = sdrv_common_reg_get_u32(SDRV_REG_BOOTREASON);
    recovery |= 0x1u;

    if (reboot_args.args.reason == HALT_REASON_SW_RECOVERY) {
        /* Clear boot reason register */
        reboot_args.args.reason = HALT_REASON_UNKNOWN;
        sdrv_common_reg_set_u32(reboot_args.val, SDRV_REG_BOOTREASON);
        dprintf(CRITICAL, "The boot reason is RECOVERY!\n");
        recovery |= 0x2u;
        return true;
    }

    return false;
}

static bool recovery_partition_exist(partition_device_t *ptdev)
{
    uint64_t ptn;
    bool ret = true;

    if (!ptdev || !ptdev->storage) {
        dprintf(CRITICAL, "partition/storage device is NULL\n");
        goto out;
    }

    ptn = ptdev_get_offset(ptdev, RECOVERY_PARTITION_NAME);

    if (ptn == 0)
        ret = false;

out:
    return ret;
}

static const char *get_android_boot_mode(partition_device_t *ptdev)
{
    uint64_t ptn;
    uint32_t bk_sz = 0;
    const char *misc = "misc";
    storage_device_t *storage;
    struct bootloader_message *boot_msg = NULL;

    if (is_recovery() || recovery_partition_exist(ptdev)) {
        return "androidboot.force_normal_boot=0";
    }

    if (!ptdev || !ptdev->storage) {
        dprintf(CRITICAL, "partition/storage device is NULL\n");
        goto normal;
    }

    storage = ptdev->storage;
    bk_sz = storage->get_block_size(storage);
    boot_msg = memalign(bk_sz, sizeof(struct bootloader_message));

    if (!boot_msg) {
        dprintf(CRITICAL, "Fail to allocate memory for bootloader message!\n");
        goto normal;
    }

    memset(boot_msg, 0x0, sizeof(struct bootloader_message));
    ptn = ptdev_get_offset(ptdev, misc);

    if (!ptn
            || storage->read(storage, ptn, (uint8_t *)boot_msg,
                             ROUNDUP(sizeof(struct bootloader_message), bk_sz))) {
        dprintf(CRITICAL, "Fail to read misc partition!\n");
        goto normal;
    }

    if (boot_msg->command[0] != 0) {
        free(boot_msg);
        dprintf(ALWAYS, "Enter recovery mode,command:%s\n", boot_msg->command);
        return "androidboot.force_normal_boot=0";
    }

normal:

    if (boot_msg)
        free(boot_msg);

    return "androidboot.force_normal_boot=1";
}

static char *get_serialno(void)
{
    static char property[96];
    uint64_t usb_serialno = 0x0123456789;

    memset(property, 0x0, sizeof(property));
#if SUPPORT_BOARDINFO
    boardinfo_get_serialno((uint32_t *)&usb_serialno);
    usb_serialno = avb_htobe64(usb_serialno);
#endif
    sprintf(property, " androidboot.serialno=%016llX ", usb_serialno);
    return property;
}

static const char *get_verified_state(void)
{
    if (get_device_locked()) {
        return "androidboot.verifiedbootstate=green";
    }
    else {
        return  "androidboot.verifiedbootstate=orange";
    }
}

static int parse_bootimage(boot_img_hdr *hdr, partition_device_t *ptdev)
{
    int ret;
    uint8_t *kernel, *ramdisk, *tags, *second;
    unsigned page_size = 0;
    storage_device_t *storage_dev = ptdev->storage;
    uint32_t block_size =  storage_dev->get_block_size(storage_dev);
    /* Load bootimage head */
    uint64_t boot_ptn, boot_size;
    uint64_t kernel_offset, ramdisk_offset, second_offset;

    if (is_recovery() && recovery_partition_exist(ptdev)) {
        boot_part = RECOVERY_PARTITION_NAME;
    }

    boot_ptn = ptdev_get_offset(ptdev, boot_part);
    boot_size = ptdev_get_size(ptdev, boot_part);

    if (!boot_ptn || !boot_size) {
        dprintf(CRITICAL, "no %s partition found.\n", boot_part);
        return -1;
    }

    ret = storage_dev->read(storage_dev, boot_ptn, (uint8_t *)hdr,
                            ALIGN(sizeof(boot_img_hdr), block_size));

    if (ret) {
        dprintf(CRITICAL, "failed to read boot partition.\n");
        return -1;
    }

    /* Check and parse bootimage */
    if (memcmp(hdr->magic, BOOT_MAGIC, BOOT_MAGIC_SIZE)) {
        dprintf(CRITICAL, "bad bootimage file\n");
        return -1;
    }

    page_size = hdr->page_size;
    dprintf(INFO, "page size %u\n", page_size);
    assert(IS_ALIGNED(page_size, block_size));

    dprintf(INFO,
            "kernel_addr %p, ramdisk_addr %p, tags_addr %p, second_addr %p\n",
            (void *)(uint64_t)(hdr->kernel_addr),
            (void *)(uint64_t)(hdr->ramdisk_addr),
            (void *)(uint64_t)(hdr->tags_addr), (void *)(uint64_t)(hdr->second_addr));
    /* Get virtual addresses since the hdr saves physical addresses. */
    kernel = (uint8_t *)_ioaddr((paddr_t)(hdr->kernel_addr));
    ramdisk = (uint8_t *)_ioaddr((paddr_t)(hdr->ramdisk_addr));
    tags = (uint8_t *)_ioaddr((paddr_t)(hdr->tags_addr));
    second = (uint8_t *)_ioaddr((paddr_t)(hdr->second_addr));

    kernel_offset  =  page_size;
    ramdisk_offset = kernel_offset + ALIGN(hdr->kernel_size, page_size);
    second_offset  = ramdisk_offset + ALIGN(hdr->ramdisk_size, page_size);

#if VERIFIED_BOOT
    bool no_overlap = false;
    uint64_t bootimg_size;
    uint8_t *bootimg_load_addr = NULL;
    mem_usage_info mem_info[3] = {
        {
            .base = kernel,
            .size = ALIGN(hdr->kernel_size, page_size),
            .offset = kernel_offset,
        },
        {
            .base = ramdisk,
            .size = ALIGN(hdr->ramdisk_size, page_size),
            .offset = ramdisk_offset,
        },
        {
            .base = second,
            .size = ALIGN(hdr->second_size, page_size),
            .offset = second_offset,
        }
    };

    EXCHANGE_ASCEND_MEM_INFO(mem_info[0], mem_info[1]);
    EXCHANGE_ASCEND_MEM_INFO(mem_info[0], mem_info[2]);
    EXCHANGE_ASCEND_MEM_INFO(mem_info[1], mem_info[2]);

    if (!verify_bootimage(ptdev, boot_part, &bootimg_load_addr, &bootimg_size,
                          mem_info, ARRAY_SIZE(mem_info), &no_overlap)) {
        return -1;
    }

    if (no_overlap) {
        memcpy(kernel, bootimg_load_addr + kernel_offset, hdr->kernel_size);
        arch_clean_cache_range((addr_t)kernel, hdr->kernel_size);
        memcpy(ramdisk, bootimg_load_addr + ramdisk_offset, hdr->ramdisk_size);
        arch_clean_cache_range((addr_t)ramdisk, hdr->ramdisk_size);

        if (hdr->second_size > 0) {
            memcpy(second, bootimg_load_addr + second_offset, hdr->second_size);
            arch_clean_cache_range((addr_t)second, hdr->second_size);
        }

        goto out;
    }

#endif

    kernel_offset  += boot_ptn;
    ramdisk_offset += boot_ptn;
    second_offset  += boot_ptn;

    ret = storage_dev->read(storage_dev, kernel_offset, kernel,
                            ALIGN(hdr->kernel_size, page_size));

    if (ret) {
        dprintf(CRITICAL, "failed to read kernel from bootimage.\n");
        return -1;
    }

    ret = storage_dev->read(storage_dev, ramdisk_offset, ramdisk,
                            ALIGN(hdr->ramdisk_size, page_size));

    if (ret) {
        dprintf(CRITICAL, "failed to read ramdisk from bootimage.\n");
        return -1;
    }

    /* second_size is 0 mostly, no second bootloader in bootimage */
    if (hdr->second_size > 0) {
        ret = storage_dev->read(storage_dev, second_offset, second,
                                ALIGN(hdr->second_size, page_size));

        if (ret) {
            dprintf(CRITICAL, "failed to read second bootloader from bootimage.\n");
            return -1;
        }
    }

out:
    dprintf(INFO,
            "kernel %p size:0x%0x, ramdisk %p size:0x%0x, tags %p, second %p size:0x%0x\n",
            kernel, ALIGN(hdr->kernel_size, page_size),
            ramdisk, ALIGN(hdr->ramdisk_size, page_size),
            tags,
            second, ALIGN(hdr->second_size, page_size));

    return 0;
}

static int generate_part_cmdline(unsigned char *cmdline,
                                 partition_device_t *ptdev)
{
    uint32_t block_size;
    bool add_blkdev_def = false;

    storage_device_t *storage_dev = ptdev->storage;
    block_size = storage_dev->get_block_size(storage_dev);

    unsigned partition_cnt = ptdev_get_partition_count(ptdev);

    struct partition_entry *partition_entries =
        ptdev_get_partition_entries(ptdev);

    /* Report active part only */
    int slot = ptdev_find_boot_slot(ptdev);
    const char *suffix = suffix_slot[slot];

    for (unsigned int i = 0; i < partition_cnt; i++) {
        uint64_t size;
        uint64_t offset;
        char partition[128] = {0};
        char *separator =  NULL;
        char *ptr_pname = (char *)partition_entries[i].name;

        if (((separator = strstr(ptr_pname, partition_separator))
                && part_is_active(partition_entries + i))
                || !strncmp(ptr_pname, VBMETA_PARTITION_NAME,
                            strlen(VBMETA_PARTITION_NAME))) {

            if (strncmp(ptr_pname, VBMETA_PARTITION_NAME,
                        strlen(VBMETA_PARTITION_NAME))) {
                /* Remove parent name and the separator */
                ptr_pname = separator + strlen(partition_separator);
            }
            else if (!part_is_active(partition_entries + i)) {
                continue;
            }

            /* Remove suffix name */
            char *raw_name  = strstr(ptr_pname, suffix);

            if (raw_name) {
                raw_name[0] = '\0';
            }

            dprintf(INFO, "ptr_pname = %s\n", ptr_pname);

            if (!add_blkdev_def) {
                strcat((char *)cmdline, hide_part);
                strcat((char *)cmdline, disk_name);
                add_blkdev_def = true;
            }

            size = partition_entries[i].size * block_size;
            offset = partition_entries[i].first_lba * block_size;
            sprintf(partition, "%llu@%llu(%s),", size, offset, ptr_pname);
            strcat((char *)cmdline, partition);
        }
    }

    /* Remove last separator */
    if (add_blkdev_def) {
        int len = strlen((const char *)cmdline);
        cmdline[len - 1] = '\0';
    }

    dprintf(CRITICAL, "cmdline with part: %s\n", cmdline);
    /* Append a/b slot_suffix */
    /* TODO */

    return 0;
}

/* Pass all sub part to xen, move to preloader later */
static int generate_active_part_cmdline(unsigned char *cmdline,
                                        partition_device_t *ptdev)
{
    uint32_t block_size;
    bool add_blkdev_def = false;

    storage_device_t *storage_dev = ptdev->storage;
    block_size = storage_dev->get_block_size(storage_dev);

    unsigned partition_cnt = ptdev_get_partition_count(ptdev);

    struct partition_entry *partition_entries =
        ptdev_get_partition_entries(ptdev);

    /* Report active part only */
    int slot = ptdev_find_boot_slot(ptdev);
    const char *suffix = suffix_slot[slot];

    for (unsigned int i = 0; i < partition_cnt; i++) {
        uint64_t size;
        uint64_t offset;
        char partition[128] = {0};
        char *ptr_pname = (char *)partition_entries[i].name;

        if (part_is_active(partition_entries + i)) {
            /* Remove suffix name */
            char *raw_name  = strstr(ptr_pname, suffix);

            if (raw_name) {
                raw_name[0] = '\0';
            }

            dprintf(INFO, "ptr_pname = %s\n", ptr_pname);

            if (!add_blkdev_def) {
                strcat((char *)cmdline, hide_part);
                strcat((char *)cmdline, disk_name);
                add_blkdev_def = true;
            }

            size = partition_entries[i].size * block_size;
            offset = partition_entries[i].first_lba * block_size;
            sprintf(partition, "%llu@%llu(%s),", size, offset, ptr_pname);
            strcat((char *)cmdline, partition);
        }
    }

    /* Remove last separator */
    if (add_blkdev_def) {
        int len = strlen((const char *)cmdline);
        cmdline[len - 1] = '\0';
    }

    dprintf(CRITICAL, "cmdline sub part: %s\n", cmdline);
    /* Append a/b slot_suffix */
    /* TODO */

    return 0;
}

static int generate_storage_cmdline(unsigned char *cmdline, char *storage_type)
{
    char storage[48] = {0};

    sprintf(storage, " androidboot.storage_type=%s", storage_type);
    strcat((char *)cmdline, storage);
    return 0;
}
#define COMMAND_LINE_SIZE   2048
#define COMMAND_LINE_PARTITION 1024

static int node_offset(void *fdt, const char *node_path)
{
    int offset = fdt_path_offset(fdt, node_path);

    if (offset == -FDT_ERR_NOTFOUND)
        offset = fdt_add_subnode(fdt, 0, node_path);

    return offset;
}

static int setprop_string(void *fdt, const char *node_path,
                          const char *property, const char *string)
{
    int offset = node_offset(fdt, node_path);

    if (offset < 0)
        return offset;

    return fdt_setprop_string(fdt, offset, property, string);
}

static void subst_dm_root_device_bootargs(void *fdt)
{
    int len = 0;
    uint32_t copy_len = 0;
    uint32_t blk_dev_len = 0;
    char blk_dev[256] = {0};
    const char *fdt_bootargs;
    uint32_t placeholder_len = strlen(DM_VERITY_BLK_DEV_PLACEHOLDER);
    char *cmdline, *ptr, *rd_key, *rd_val_end, *from, *to, *dm_replace;

    if (!dm_table || !strlen(dm_table))
        return;

    cmdline = calloc(1, COMMAND_LINE_SIZE);
    ptr = cmdline;

    if (!cmdline)
        return;

    int ret = fdt_open_into(fdt, fdt,
                            fdt_totalsize(fdt) + DTB_PAD_SIZE + COMMAND_LINE_SIZE);

    if (ret)
        dprintf(CRITICAL, "failed to open dtb.\n");

    /* Copy the fdt command line into the buffer */
    fdt_bootargs = getprop(fdt, "/chosen", "bootargs", &len);

    if (fdt_bootargs) {
        if (len < COMMAND_LINE_SIZE) {
            memcpy(ptr, fdt_bootargs, len);
            /* len is the length of the string
            * including the NULL terminator */
            ptr += len - 1;
        }
    }

    if ((rd_key = strstr(cmdline, ROOT_DEVICE_KEY)) != NULL) {
        rd_key += strlen(ROOT_DEVICE_KEY);
        rd_val_end = strstr(rd_key, DM_FIELD_SEP);

        if (rd_val_end || (rd_key + strlen(rd_key)) == ptr) {
            rd_val_end = rd_val_end ? rd_val_end : ptr;
            memcpy(blk_dev, rd_key, rd_val_end - rd_key);
            rd_key -= strlen(ROOT_DEVICE_KEY);

            /* remove root=xxxx */
            memmove(rd_key, rd_val_end + 1, ptr - rd_val_end);

            ptr -=  rd_val_end - rd_key + 1;
        }
    }

    blk_dev_len = strlen(blk_dev);

    if (blk_dev_len) {
        while ((dm_replace = strstr(dm_table, DM_VERITY_BLK_DEV_PLACEHOLDER))) {
            copy_len = strlen(dm_table) - (dm_replace + placeholder_len - dm_table) + 1;
            from = dm_replace + placeholder_len;
            to = dm_replace + blk_dev_len;

            memmove(to, from, copy_len);
            memcpy(dm_replace, blk_dev, blk_dev_len);
        }
    }

    /* And append the ATAG_CMDLINE */
    len = strlen(dm_table);
    dprintf(ALWAYS, "final dm_table:%s\n", dm_table);

    if (ptr - cmdline + len + 2 < COMMAND_LINE_SIZE) {
        *ptr++ = ' ';
        memcpy(ptr, dm_table, len);
        ptr += len;
    }

    *ptr = '\0';

    setprop_string(fdt, "/chosen", "bootargs", cmdline);

    fdt_pack(fdt);
    free(cmdline);
}

static void remove_root_dev_fdt_bootargs(void *fdt)
{
    int len = 0;
    const char *fdt_bootargs;
    char *rd_key, *rd_val_end;
    char *cmdline  = calloc(1, COMMAND_LINE_SIZE);
    char *ptr = cmdline;

    int ret = fdt_open_into(fdt, fdt,
                            fdt_totalsize(fdt) + DTB_PAD_SIZE + COMMAND_LINE_SIZE);

    if (ret) {
        dprintf(CRITICAL, "failed to open dtb.\n");
    }

    /* Copy the fdt command line into the buffer */
    fdt_bootargs = getprop(fdt, "/chosen", "bootargs", &len);

    if (fdt_bootargs && (len < COMMAND_LINE_SIZE)) {
        memcpy(ptr, fdt_bootargs, len);
        /* len is the length of the string
        * including the NULL terminator */
        ptr += len - 1;

        if ((rd_key = strstr(cmdline, ROOT_DEVICE_KEY)) != NULL) {
            rd_key += strlen(ROOT_DEVICE_KEY);
            rd_val_end = strstr(rd_key, DM_FIELD_SEP);

            if (rd_val_end || (rd_key + strlen(rd_key) - 1) == ptr) {
                rd_val_end = rd_val_end ? rd_val_end : ptr;
                rd_key -= strlen(ROOT_DEVICE_KEY);

                /* remove root=xxxx */
                memmove(rd_key, rd_val_end + 1, ptr - rd_val_end);

                ptr -=  rd_val_end - rd_key + 1;
            }
        }

        *ptr = '\0';
        setprop_string(fdt, "/chosen", "bootargs", cmdline);
    }

    fdt_pack(fdt);
    free(cmdline);
}

static void merge_fdt_bootargs(void *fdt, const char *fdt_cmdline)
{
    char *cmdline  = calloc(1, COMMAND_LINE_SIZE);

    const char *fdt_bootargs;
    char *ptr = cmdline;
    int len = 0;

    int ret = fdt_open_into(fdt, fdt,
                            fdt_totalsize(fdt) + DTB_PAD_SIZE + COMMAND_LINE_SIZE);

    if (ret) {
        dprintf(CRITICAL, "failed to open dtb.\n");
    }

    /* Copy the fdt command line into the buffer */
    fdt_bootargs = getprop(fdt, "/chosen", "bootargs", &len);

    if (fdt_bootargs)
        if (len < COMMAND_LINE_SIZE) {
            memcpy(ptr, fdt_bootargs, len);
            /* len is the length of the string
            * including the NULL terminator */
            ptr += len - 1;
        }

    /* And append the ATAG_CMDLINE */
    if (fdt_cmdline) {
        len = strlen(fdt_cmdline);

        if (ptr - cmdline + len + 2 < COMMAND_LINE_SIZE) {
            *ptr++ = ' ';
            memcpy(ptr, fdt_cmdline, len);
            ptr += len;
        }
    }

    *ptr = '\0';

    setprop_string(fdt, "/chosen", "bootargs", cmdline);

    fdt_pack(fdt);
    free(cmdline);
}

static int partition_load(partition_device_t *ptdev, const char *part_name,
                          void *buf, uint64_t buf_size, uint64_t *loaded_size_p)
{
    int ret = -1;
    uint32_t bk_sz;
    AvbFooter footer;
    uint8_t *block_buf = NULL;
    storage_device_t *storage_dev = ptdev->storage;
    uint64_t ptn = ptdev_get_offset(ptdev, part_name);
    uint64_t part_size = ptdev_get_size(ptdev, part_name);

    dprintf(INFO, "ptn %llx, size %llu\n", ptn, part_size);

    if (!ptn || !part_size) {
        dprintf(INFO, "partition %s no found.\n", part_name);
        goto out;
    }

    bk_sz = storage_dev->get_block_size(storage_dev);
    block_buf = memalign(bk_sz, bk_sz);

    if (block_buf
            && !storage_dev->read(storage_dev, ptn + part_size - bk_sz, block_buf, bk_sz)) {
        if (avb_footer_validate_and_byteswap((AvbFooter *)(block_buf + bk_sz - sizeof(
                AvbFooter)), &footer))
            part_size = footer.original_image_size;
    }

    if (buf_size < part_size) {
        dprintf(CRITICAL, "partition %s size is too large.\n", part_name);
        goto out;
    }

    ret = storage_dev->read(storage_dev, ptn, buf, round_up(part_size, bk_sz));

    if (ret) {
        dprintf(CRITICAL, "failed to load %s partition.\n", part_name);
        ret = -1;
        goto out;
    }

    if (loaded_size_p != NULL) {
        *loaded_size_p = round_up(part_size, bk_sz);
    }

out:

    if (block_buf)
        free(block_buf);

    return ret;
}

static void update_dt_psci_method(void *fdt, const char *method)
{
    int len = 0, ret;
    const char *method_old = NULL;
    fdt_open_into(fdt, fdt, fdt_totalsize(fdt) + strlen(method) + 1);
    int psci_node = fdt_find_node_compatible(fdt, "arm,psci");

    if (!psci_node)
        psci_node = fdt_find_node_compatible(fdt, "arm,psci-1.0");

    if (psci_node) {
        method_old = fdt_getprop(fdt, psci_node, "method", &len);

        if (method_old && strncmp(method_old, method, len)) {
            ret = fdt_setprop_string(fdt, psci_node, "method", method);

            if (ret) {
                dprintf(CRITICAL, "Failed to replace psci method to %s\n",
                        method);
            }

            dprintf(CRITICAL, "Current psci method is %s\n", method_old);
        }
    }

    fdt_pack(fdt);
}

/* check rootfs type, return 1: no ramfs, 0: ramfs, -1: error */
static int check_root_type(void *fdt)
{
    int chosen_node, len;
    const char *fdt_bootargs;
    fdt_open_into(fdt, fdt, fdt_totalsize(fdt));
    chosen_node = fdt_find_node_name(fdt, "chosen");

    if (chosen_node < 0) {
        dprintf(CRITICAL, "fdt chosen node no found.\n");
        return -1;
    }

    fdt_bootargs = getprop(fdt, "/chosen", "bootargs", &len);

    if (strstr(fdt_bootargs, "skip_initramfs"))
        return 1;

    if (strstr(fdt_bootargs, "noinitrd"))
        return 1;

    if (strstr(fdt_bootargs, "mmcblk"))
        return 1;

    const void *initrd = fdt_getprop(fdt, chosen_node, "linux,initrd-start", &len);

    if (initrd) {
        /* find initrd node */
        return 0;
    }

    return -1;
}

static void *relocate_fdt(void *src, void *dest)
{
    int size;
    size = fdt_totalsize(src);
    return memcpy(dest, src, size);
}

static int update_root_dev(partition_device_t *part_dev, unsigned char *cmdline,
                           const char *indication)
{
    char *root_dev;
    unsigned int index;
    char tmp[64] = {0};

    if (!disk_name) {
        dprintf(CRITICAL, "unknown disk type, can't update rootfs\n");
        return -1;
    }

    index = ptdev_get_index(part_dev, indication);

    if (index == (unsigned)INVALID_PTN) {
        dprintf(CRITICAL, "failed to update rootfs device node\n");
        return -1;
    }

    root_dev = calloc(64, 1);

    if (!root_dev)
        return -1;

    strlcpy(tmp, disk_name, strlen(disk_name));
    /* kernel organize part num from p1, however bootloader from p0 */
    sprintf(root_dev, " root=/dev/%sp%u", tmp, index + 1);
    strcat((char *)cmdline, root_dev);
    free(root_dev);
    return 0;
}

typedef int (*dtbo_find_match_fn)(struct dt_table_entry *dt_table_entry);

#if SUPPORT_BOARDINFO
int dtbo_find_hw_match(struct dt_table_entry *dt_table_entry)
{
    uint32_t chip_id, board_type, board_id;
    uint32_t chip_dt, type_dt, board_dt;

    chip_id = get_part_id(PART_CHIPID);

    board_type = get_part_id(PART_BOARD_TYPE);

    board_id = (get_part_id(PART_BOARD_ID_MAJ) << SDRV_BOARDID_MAJOR_OFFSET)
               + get_part_id(PART_BOARD_ID_MIN);

    chip_dt = fdt32_to_cpu(dt_table_entry->custom[1]);
    type_dt = fdt32_to_cpu(dt_table_entry->custom[2]);
    board_dt = fdt32_to_cpu(dt_table_entry->custom[3]);

    if (chip_dt != SDRV_CHIPID_ALL && chip_dt != chip_id)
        return -1;

    if (type_dt != SDRV_BOARD_TYPE_ALL && type_dt != board_type)
        return -1;

    if (board_dt != SDRV_BOARDID_ALL && board_dt != board_id)
        return -1;

    return 0;
}
#else
int dtbo_find_hw_match(struct dt_table_entry *dt_table_entry)
{
    return -1;
}
#endif


static struct fdt_header *dtbo_apply_overlay(struct fdt_header *fdt,
        struct fdt_header *dtbo)
{
    uint32_t fdt_size, dtbo_size;

    fdt_size = fdt_totalsize(fdt);
    dtbo_size = fdt_totalsize(dtbo);

    return ufdt_apply_overlay(fdt, fdt_size, (void *)dtbo, dtbo_size);
}

/* find and apply match dtbos */
static int dtbo_find_apply_match(void *fdt, void *dtbo_image_buf,
                                 dtbo_find_match_fn dtbo_find_match_fn)
{
    struct dt_table_header *dt_table_header = dtbo_image_buf;
    struct dt_table_entry *dt_table_entry = NULL;
    uint32_t dtbo_count = 0;
    uint32_t dtbo_entry_offset = 0;
    struct fdt_header *dtbo, *tmp_fdt;
    struct fdt_header *patched_fdt;
    int ret;

    if (!dtbo_find_match_fn) {
        dprintf(CRITICAL, "bad dtbo match fn\n");
        return -1;
    }

    if (fdt32_to_cpu(dt_table_header->magic) != DT_TABLE_MAGIC) {
        dprintf(CRITICAL, "bad dtbo header magic\n");
        return -1;
    }

    dtbo_entry_offset = fdt32_to_cpu(dt_table_header->dt_entries_offset);
    dtbo_count = fdt32_to_cpu(dt_table_header->dt_entry_count);

    if (!dtbo_count) {
        dprintf(CRITICAL, "no dtbo found\n");
        return -1;
    }

    tmp_fdt = calloc(1, fdt_totalsize(fdt));
    memcpy(tmp_fdt, fdt, fdt_totalsize(fdt));

    dt_table_entry = (struct dt_table_entry *)(dtbo_image_buf + dtbo_entry_offset);

    for (uint32_t i = 0; i < dtbo_count; i++) {
        if (dtbo_find_match_fn(&dt_table_entry[i]))
            continue;

        dtbo = (struct fdt_header *)(dtbo_image_buf + fdt32_to_cpu(
                                         dt_table_entry[i].dt_offset));
        ret = fdt_check_header(dtbo);

        if (ret) {
            dprintf(CRITICAL, "bad dtbo binary %d\n", i);
            continue;
        }

        patched_fdt = dtbo_apply_overlay(tmp_fdt, dtbo);

        if (!patched_fdt) {
            dprintf(CRITICAL, "failed to apply overlay\n");
            continue;
        }

        free(tmp_fdt);
        tmp_fdt = patched_fdt;
    }

    memcpy(fdt, tmp_fdt, fdt_totalsize(tmp_fdt));
    free(tmp_fdt);
    return 0;
}

/* fdt buffer shall be large enough to carry dtb + dtbo */
static bool load_dtbo_image(partition_device_t *ptdev,
                            dtbo_img_info *dtbo)
{
    uint64_t part_size;
    int ret;
    void *dtbo_buf;
    uint32_t block_size;
    storage_device_t *storage = NULL;
    char dtbo_part[32] = {0};

    part_size = ptdev_get_size(ptdev, "dtbo");

    if (!part_size) {
        strcat(dtbo_part, PART_SEPARAT);
        strcat(dtbo_part, "dtbo");
        part_size = ptdev_get_size(ptdev, dtbo_part);

        if (!part_size) {
            dprintf(CRITICAL, "dtbo part %s no found\n", dtbo_part);
            return false;
        }
    }
    else {
        strcat(dtbo_part, "dtbo");
    }

    storage = ptdev->storage;
    block_size =  storage->get_block_size(storage);

    dtbo_buf = memalign(block_size, part_size);

    if (!dtbo_buf) {
        dprintf(CRITICAL, "failed to alloc memory\n");
        return false;
    }

    ret = partition_load(ptdev, dtbo_part, dtbo_buf, part_size, NULL);

    if (ret) {
        dprintf(CRITICAL, "failed to load dtbo image\n");
        return false;
    }

    //dtbo_find_apply_match(fdt, dtbo_buf, dtbo_find_hw_match);
    /* TODO */
    /* dtbo_find_apply_match(fdt, dtbo_buf, dtbo_find_feature_match); */
    if (dtbo) {
        dtbo->base = dtbo_buf;
        dtbo->size = part_size;
        memcpy(dtbo->pt_name, dtbo_part, MIN(strlen(dtbo_part), MAX_GPT_NAME_SIZE));
    }

    return true;
}



/**/

int _bootloader_main(void)
{
    int ret = 0, root_type;
    boot_img_hdr *hdr;
    unsigned char *cmdline_board;
    char *storage_type;
    dtbo_img_info dtbo_info;
    partition_device_t *ptdev;
    storage_device_t *storage_dev = NULL;
    boot_device_cfg_t *btdev_cfg = NULL;
    unsigned ramdisk_addr, ramdisk_size;
    uint64_t memory_start[MEMORY_BANKS_MAX] = {0};
    uint64_t memory_size[MEMORY_BANKS_MAX] = {0};
    void *rootfs_buf = NULL;
    void *fdt, *dom0_buf, *dtb_buf, *xen_buf, *kernel;
    unsigned long dtb_p;
    struct list_node verified_images_list;
    struct startup_header  *startup_hdr = NULL;

#if SUPPORT_BOARDINFO
    char *hwid, *property;
#endif

    // 0 means don't using ddr size.
    uint64_t ddr_size = DDR_SIZE;

    uint32_t pin = boot_get_pin();
    btdev_pin_cfg_t *btdev_pin = find_btdev(pin);

    if (btdev_pin) {
        btdev_cfg =  btdev_pin->ap;
    }

    if (btdev_cfg) {
        disk_name = btdev_cfg->disk_name;
        storage_type = btdev_cfg->storage_type;
        storage_dev = setup_storage_dev(btdev_cfg->device_type,
                                        btdev_cfg->res_idex, (void *)&btdev_cfg->cfg);
    }

    list_initialize(&verified_images_list);

    if (!storage_dev) {
        dprintf(CRITICAL, "failed to setup storage device.\n");
        return -1;
    }

    /* TODO, switch part firstly */
    //storage_dev->switch_part(storage, PART_USER);
    /* Read partition table */
    ptdev = ptdev_setup(storage_dev, 0);

    if (ptdev) {
        ptdev_read_table(ptdev);
    }
    else {
        dprintf(CRITICAL, "can't find storage device\n");
        return -1;
    }

    uint32_t block_size =  storage_dev->get_block_size(storage_dev);

boot_qnx:
    startup_hdr = memalign(block_size, ROUNDUP(sizeof(struct startup_header),
                           block_size));

    if (!startup_hdr) {
        dprintf(CRITICAL, "alloc memory failed\n");
        goto error;
    }
    else {
        /* never return if load qnx correctly*/
        bootloader_entry_qnx(ptdev, startup_hdr);
    }

    fdt = (void *)_ioaddr(REE_MEMBASE + BOARD_TARS_OFFSET);

    ret = partition_load(ptdev, dtb_part, fdt,
                         BOARD_KERNEL_OFFSET - BOARD_TARS_OFFSET, NULL);

    if (ret) {
        dprintf(INFO, "DTB load failed.\n");
        goto boot_android;
    }

    ret = fdt_check_header(fdt);

    if (ret) {
        dprintf(INFO, "not a flattened device tree.\n");
        goto boot_android;
    }

    if (!check_xen_compatible(fdt)) {
        dprintf(INFO, "no xen compatible found.\n");
        goto boot_android;
    }

boot_xen:
    /* TODO */
    /* No support for virtualization*/
    /* load_dtbo_image(ptdev, fdt); */

    /* load rootfs */
    root_type = check_root_type(fdt);

    if (root_type == 0) {
        /* ramfs type */
        rootfs_buf = (void *)_ioaddr(REE_MEMBASE + BOARD_RAMDISK_OFFSET);

        ret = partition_load(ptdev, dom0_rootfs, rootfs_buf, ROOTFS_MEMSIZE, NULL);

        if (ret) {
            dprintf(INFO, "rootfs load failed.\n");
            rootfs_buf = NULL;
            goto boot_android;
        }
    }

    ret = fdt_get_mem_node(fdt, memory_start, memory_size);

    if (ret < 0) {
        dprintf(INFO, "failed to get memory node from fdt\n");
        goto boot_android;
    }

    /* load dom0 linux */
    uint64_t dom0_img_sz = DOM0_MEMSIZE;
    dom0_buf = (void *)_ioaddr(memory_start[0] + DOM0_KERNEL_OFFSET);
    ret = partition_load(ptdev, dom0_kernel, dom0_buf, DOM0_MEMSIZE, &dom0_img_sz);

    if (ret) {
        dprintf(INFO, "dom0 load failed.\n");
        goto boot_android;
    }

    /* load xen */
    xen_buf = (void *)_ioaddr(memory_start[0] + XEN_IMG_OFFSET);
    ret = partition_load(ptdev, xen_part, xen_buf, HYP_MEMSIZE, NULL);

    if (ret) {
        dprintf(INFO, "xen load failed.\n");
        goto boot_android;
    }

    fdt = relocate_fdt(fdt, (void *)_ioaddr(memory_start[0] + XEN_FDT_OFFSET));

    if (!add_verified_image_list(&verified_images_list, fdt,
                                 BOARD_KERNEL_OFFSET - BOARD_TARS_OFFSET, dtb_part)
            || !add_verified_image_list(&verified_images_list, dom0_buf, DOM0_MEMSIZE,
                                        dom0_kernel)
            || !add_verified_image_list(&verified_images_list, xen_buf, HYP_MEMSIZE,
                                        xen_part)
            || (rootfs_buf
                && !add_verified_image_list(&verified_images_list, rootfs_buf, ROOTFS_MEMSIZE,
                                            dom0_rootfs))) {
        goto error;
    }

    if (!get_cmdline_by_verified_image(ptdev, &verified_images_list)) {
        goto error;
    }

    free_image_info_list(&verified_images_list);

    /* update initrd */
    if (root_type == 0)
        ret =  update_fdt_initrd(fdt, REE_MEMBASE + BOARD_RAMDISK_OFFSET,
                                 ROOTFS_MEMSIZE);

    /* update xen module reg node */
    memory_start[0] = memory_start[0] + DOM0_KERNEL_OFFSET;
    memory_size[0] = (DOM0_MEMSIZE > dom0_img_sz) ? dom0_img_sz : DOM0_MEMSIZE;
    ret = update_fdt_module(fdt, memory_start, memory_size, 1);

    /* Report all active sub part */
    cmdline_board = calloc(1, COMMAND_LINE_PARTITION);

    if (!cmdline_board) {
        dprintf(CRITICAL, "alloc cmdline buf failed\n");
        goto error;
    }

#if SUPPORT_CMDLINE_PART
    ret = generate_active_part_cmdline(cmdline_board, ptdev);

    if (ret) {
        dprintf(INFO, "failed to get sub part.\n");
        free(cmdline_board);
        goto boot_android;
    }

#endif

    if (root_type == 1)
        update_root_dev(ptdev, cmdline_board, dom0_rootfs);

    merge_fdt_bootargs(fdt, (const char *)cmdline_board);
    merge_fdt_bootargs(fdt, vbmeta_cmdline);
    merge_fdt_bootargs(fdt, get_verified_state());
    merge_fdt_bootargs(fdt, get_serialno());
#if 0
    merge_fdt_bootargs(fdt, get_android_boot_mode());
#endif
    free(cmdline_board);

    if (vbmeta_cmdline)
        free(vbmeta_cmdline);

    fdt_pack(fdt);

    arch_clean_cache_range((addr_t)fdt, BOARD_KERNEL_OFFSET);

    unsigned long fdt_p = _paddr(fdt);
    unsigned long xen_p = _paddr(xen_buf);

    /* hvc call */
    hvc(xen_p, fdt_p, 0, 0, 0, 0, 0, 0);
    return 0;

boot_android:

    /* No Hypervisor, disable hvc call by clearing SCR_EL3.HCE */
    /* This smc call shall be implemented by ATF */
    /* If ATF is not persistent, preloader's vector will handle and return simply */
    smc(SMC_DIS_HCE, 0, 0, 0, 0, 0, 0, 0);

    /* Load and parse boot partition */
    hdr = memalign(block_size, ROUNDUP(sizeof(boot_img_hdr), block_size));

    if (!hdr) {
        dprintf(CRITICAL, "alloc memory failed\n");
        goto error;
    }

    ret = parse_bootimage(hdr, ptdev);

    if (ret) {
        dprintf(INFO, "failed load and parse bootimage.\n");
        free(hdr);
        goto boot_linux;
    }

    ret = partition_load(ptdev, dtb_part, fdt,
                         BOARD_KERNEL_OFFSET - BOARD_TARS_OFFSET, NULL);

    if (ret) {
        dprintf(INFO, "%s dtb load failed.\n", dtb_part);
        free(hdr);
        goto boot_linux;
    }

    ret = fdt_check_header(fdt);

    if (ret) {
        dprintf(INFO, "sub dtb not a flattened device tree.\n");
        free(hdr);
        goto boot_linux;
    }

    memset(&dtbo_info, 0x0, sizeof(dtbo_info));

    if (load_dtbo_image(ptdev, &dtbo_info)
            && !add_verified_image_list(&verified_images_list, dtbo_info.base,
                                        dtbo_info.size,
                                        dtbo_info.pt_name)) {
        goto error;
    }

    if (!add_verified_image_list(&verified_images_list, fdt,
                                 BOARD_KERNEL_OFFSET - BOARD_TARS_OFFSET, dtb_part)) {
        goto error;
    }

    if (!get_cmdline_by_verified_image(ptdev, &verified_images_list)) {
        goto error;
    }

    free_image_info_list(&verified_images_list);

    if (dtbo_info.base)
        dtbo_find_apply_match(fdt, dtbo_info.base, dtbo_find_hw_match);

    /* update memory range */
    ret = fdt_update_memory_range(fdt, HYP_MEMBASE, 0, ddr_size);

    if (ret)
        dprintf(CRITICAL, "failed to update memory range.\n");


    uint64_t ret_smc = smc(SMCCC_ARCH_FEATURES, SMCCC_ARCH_FEATURES, 0, 0,
                           0, 0, 0, 0);

    if (ret_smc != SMC_OK) {
        update_dt_psci_method(fdt, "native");
    }

    /* relocate dtb */
    dtb_buf = (void *)_ioaddr((paddr_t)(hdr->tags_addr));

    if (dtb_buf != fdt) {
        dprintf(CRITICAL, "tags addr do not meet memory layout.\n");
        /* follow memory layout */
        hdr->tags_addr = REE_MEMBASE + BOARD_TARS_OFFSET;
        dtb_buf = fdt;
    }

    ramdisk_addr = hdr->ramdisk_addr;
    ramdisk_size = hdr->ramdisk_size;

    ret = update_fdt_initrd(dtb_buf, ramdisk_addr, ramdisk_size);

    if (ret) {
        dprintf(CRITICAL, "failed to update initrd.\n");
        free(hdr);
        goto error;
    }

    /* Load dtbo partition */
    /* TODO */

    /* Select dtbo and  merge dtb */
    /* TODO  */

    /* Generate additional cmdline */
    cmdline_board = calloc(1, COMMAND_LINE_PARTITION);

    if (!cmdline_board) {
        dprintf(CRITICAL, "alloc cmdline buf failed\n");
        free(hdr);
        goto error;
    }

    strcpy((char *)cmdline_board, (const char *)&hdr->cmdline);

    dprintf(INFO, "board cmdline %s\n", cmdline_board);
#if SUPPORT_CMDLINE_PART
    ret = generate_part_cmdline(cmdline_board, ptdev);

    if (ret) {
        dprintf(CRITICAL, "failed to get part cmdline.\n");
        free(cmdline_board);
        free(hdr);
        goto error;
    }

#elif AB_SLOT
    int slot;
    slot = ptdev_find_boot_slot(ptdev);
    strcat((char *)cmdline_board, " androidboot.slot_suffix=");
    strcat((char *)cmdline_board, suffix_slot[slot]);
#endif
#if SUPPORT_BOARDINFO
    hwid = calloc(sizeof(char), 64);
    property = calloc(sizeof(char), 96);

    get_hwid_friendly_name(hwid, 64);
    /* android property: ro.boot.hwid */
    sprintf(property, " androidboot.hwid=%s ", hwid);
    merge_fdt_bootargs(dtb_buf, property);

    free(hwid);
    free(property);
#endif

    ret = generate_storage_cmdline(cmdline_board, storage_type);
    /* Update cmdline */
    merge_fdt_bootargs(dtb_buf, (const char *)cmdline_board);
    merge_fdt_bootargs(dtb_buf, vbmeta_cmdline);
    merge_fdt_bootargs(fdt, get_verified_state());
    merge_fdt_bootargs(fdt, get_android_boot_mode(ptdev));
    merge_fdt_bootargs(fdt, get_serialno());

    /* Boot kernel by chain load */
    void *entry_p = (void *)_ioaddr((paddr_t)(hdr->kernel_addr));
    dtb_p = _paddr(dtb_buf);
    /* Clean up before boot kernel */
    free(cmdline_board);
    free(hdr);

    if (vbmeta_cmdline)
        free(vbmeta_cmdline);

    if (dm_table)
        free(dm_table);

    if (dtbo_info.base)
        free(dtbo_info.base);

    arch_chain_load(entry_p, dtb_p, 0, 0, 0);
    return 0;

boot_linux:

    fdt = (void *)_ioaddr(REE_MEMBASE + BOARD_TARS_OFFSET);
    ret = partition_load(ptdev, alias_dtb, fdt,
                         BOARD_KERNEL_OFFSET - BOARD_TARS_OFFSET, NULL);

    if (ret) {
        dprintf(CRITICAL, " %s dtb part load failed.\n", alias_dtb);
        goto error;
    }

    ret = fdt_check_header(fdt);

    if (ret) {
        dprintf(CRITICAL, "%s dtb is not a flattened device tree.\n", alias_dtb);
        goto error;
    }

    kernel = (void *)_ioaddr(REE_MEMBASE + BOARD_KERNEL_OFFSET);
    ret = partition_load(ptdev, alias_kernel_part, kernel,
                         BOARD_RAMDISK_OFFSET - BOARD_KERNEL_OFFSET, NULL);

    if (ret) {
        dprintf(CRITICAL, " %s kernel part load failed.\n", alias_kernel_part);
        goto error;
    }

    root_type = check_root_type(fdt);

    if (root_type == 0) {
        /* ramfs type */
        rootfs_buf = (void *)_ioaddr(REE_MEMBASE + BOARD_RAMDISK_OFFSET);

        ret = partition_load(ptdev, alias_rootfs_part, rootfs_buf, ROOTFS_MEMSIZE,
                             NULL);

        if (ret) {
            dprintf(CRITICAL, "rootfs load failed.\n");
            goto error;
        }
    }

    memset(&dtbo_info, 0x0, sizeof(dtbo_info));

    if (load_dtbo_image(ptdev, &dtbo_info)
            && !add_verified_image_list(&verified_images_list, dtbo_info.base,
                                        dtbo_info.size,
                                        dtbo_info.pt_name)) {
        goto error;
    }

    if (!add_verified_image_list(&verified_images_list, fdt,
                                 BOARD_KERNEL_OFFSET - BOARD_TARS_OFFSET, alias_dtb)
            || !add_verified_image_list(&verified_images_list, kernel,
                                        BOARD_RAMDISK_OFFSET - BOARD_KERNEL_OFFSET, alias_kernel_part)
            || (rootfs_buf
                && !add_verified_image_list(&verified_images_list, rootfs_buf, ROOTFS_MEMSIZE,
                                            alias_rootfs_part))) {
        goto error;
    }

    if (!get_cmdline_by_verified_image(ptdev, &verified_images_list)) {
        goto error;
    }

    free_image_info_list(&verified_images_list);

    if (dtbo_info.base)
        dtbo_find_apply_match(fdt, dtbo_info.base, dtbo_find_hw_match);


    /* update memory range */
    ret = fdt_update_memory_range(fdt, HYP_MEMBASE, 0, ddr_size);

    if (ret)
        dprintf(CRITICAL, "failed to update memory range.\n");


    ret_smc = smc(SMCCC_ARCH_FEATURES, SMCCC_ARCH_FEATURES, 0, 0,
                  0, 0, 0, 0);

    if (ret_smc != SMC_OK) {
        update_dt_psci_method(fdt, "native");
    }

    /* update initrd */
    if (root_type == 0)
        ret =  update_fdt_initrd(fdt, REE_MEMBASE + BOARD_RAMDISK_OFFSET,
                                 ROOTFS_MEMSIZE);

    /* Report all active part */
    cmdline_board = calloc(1, COMMAND_LINE_PARTITION);

    if (!cmdline_board) {
        dprintf(CRITICAL, "alloc cmdline buf failed\n");
        goto error;
    }

#if SUPPORT_CMDLINE_PART
    ret = generate_active_part_cmdline(cmdline_board, ptdev);

    if (ret) {
        dprintf(CRITICAL, "failed to get all part.\n");
        free(cmdline_board);
        goto error;
    }

#endif

    if (root_type == 1) {
        remove_root_dev_fdt_bootargs(fdt);

        if (is_recovery()) {
            update_root_dev(ptdev, cmdline_board, "recovery");
        }
        else {
            update_root_dev(ptdev, cmdline_board, alias_rootfs_part);
        }
    }

    merge_fdt_bootargs(fdt, (const char *)cmdline_board);
    subst_dm_root_device_bootargs(fdt);
    free(cmdline_board);

    if (vbmeta_cmdline)
        free(vbmeta_cmdline);

    if (dm_table)
        free(dm_table);

    if (dtbo_info.base)
        free(dtbo_info.base);

    fdt_pack(fdt);

    arch_clean_cache_range((addr_t)fdt, BOARD_KERNEL_OFFSET);

    /* Boot kernel */
    dtb_p = _paddr(fdt);

    arch_chain_load(kernel, dtb_p, 0, 0, 0);
    return 0;

error:
    dprintf(CRITICAL, "no os is available to boot\n");
    return -1;

}
#endif

int bootloader_main(int argc, const cmd_args *argv)
{
#ifndef BACKDOOR_DDR
    _bootloader_main();
#endif
    return 0;
}


static void bootloader_entry(const struct app_descriptor *app, void *args)
{

#ifndef BACKDOOR_DDR
    _bootloader_main();
#else
    /* Boot kernel by chain load */
    /* never return if load qnx correctly*/
    bootloader_entry_qnx_backdoor_ddr();

    /* Boot kernel by chain load */
    void *entry_p = (void *)_ioaddr((paddr_t)(KERNEL_LOAD_ADDR));
    unsigned long dtb_p = (unsigned long)DTB_LOAD_ADDR;
#if VERIFIED_BOOT
    bool ret;
    void *fdt;
    uint8_t *vbmeta_buf = (uint8_t *)_ioaddr(VBMETA_MEMBASE + 0x10000000);
    uint32_t vbmeta_sz = VBMETA_MEMSIZE;
    uint64_t initrd_start, initrd_end, ramdisk_sz;
    struct list_node verified_images_list = LIST_INITIAL_VALUE(
            verified_images_list);

    fdt = (void *)_ioaddr(dtb_p);
    initrd_start = get_uint_from_fdt_by_name(fdt, "chosen", "linux,initrd-start");
    initrd_end   = get_uint_from_fdt_by_name(fdt, "chosen", "linux,initrd-end");

    if (!initrd_start || !initrd_end || initrd_end <= initrd_start) {
        printf  ("initrd_start:0x%llx end:0x%llx\n", initrd_start, initrd_end);
        return;
    }

    ramdisk_sz = initrd_end - initrd_start;
    ret = add_verified_image_list(&verified_images_list,
                                  (void *)entry_p,
                                  KERNEL_LOAD_SIZE, "cluster_kernel");
    ASSERT(ret);
    ret = add_verified_image_list(&verified_images_list,
                                  (void *)fdt,
                                  DTB_LOAD_SIZE, "cluster_dtb");
    ASSERT(ret);
    ret = add_verified_image_list(&verified_images_list,
                                  (void *)_ioaddr(initrd_start),
                                  ramdisk_sz, "cluster_ramdisk");
    ASSERT(ret);

    ret = add_verified_image_list(&verified_images_list,
                                  vbmeta_buf, vbmeta_sz,
                                  VBMETA_PARTITION_NAME);
    ASSERT(ret);

    if (!verify_loaded_images(NULL, &verified_images_list, NULL)) {
        dprintf(ALWAYS, "%s %d verify images fail\n", __func__, __LINE__);
        free_image_info_list(&verified_images_list);
        return;
    }

    free_image_info_list(&verified_images_list);
#endif

    //assign_gic_non_sec();
    arch_chain_load(entry_p, dtb_p, 0, 0, 0);
#endif
}

#if defined(WITH_LIB_CONSOLE)
STATIC_COMMAND_START
STATIC_COMMAND("bootloader",  "bootloader",
               (console_cmd)&bootloader_main)
STATIC_COMMAND_END(bootloader);
#endif

APP_START(bootloader)
.flags = 0,
.entry = bootloader_entry,
APP_END
