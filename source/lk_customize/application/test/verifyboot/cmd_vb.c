/*
 * cmd_vb.c
 *
 * Copyright (c) 2020 SemiDrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */

#include <app.h>
#include <arch.h>
#include <assert.h>
#include <debug.h>
#include <err.h>
#include <platform.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <lib/console.h>
#include <lib/reg.h>
#include <trace.h>

#include <chip_res.h>
#include "clkgen_hal.h"
#include "ckgen_cfg.h"
#include "md5.h"
#include "mmc_hal.h"
#include "partition_parser.h"
#include "storage_device.h"
#include "verified_boot.h"
#include "res.h"
#include "libavb.h"
#include "sd_x509.h"
#include "internal.h"

#define BOOT_EMMC_GPT_START  0
#define PT_CERT_NAME       "ssystem"
#define PT_FOR_TEST        "ssystem"

//static const char *CA =
//"D800731042FE57353D8D0EA12B121C5B30D89DB5FB7186E615E10428D9E9C73C77E97278F58912B68BD97D3463BA857908379F43B6D29A96DBE13591F883A5A803A2E069F85F9A4C650C75B79751666435DF09A0522BC5FD1133BBC31F0FD52E400C3F889DB14894F5592B59D1063305941852C4F9B558EA53E7E40092C8FFB9";

static uint8_t g_modulus[] = {
    0xD8, 0x00, 0x73, 0x10, 0x42, 0xFE, 0x57, 0x35, 0x3D, 0x8D, 0x0E, 0xA1, 0x2B, 0x12, 0x1C, 0x5B,
    0x30, 0xD8, 0x9D, 0xB5, 0xFB, 0x71, 0x86, 0xE6, 0x15, 0xE1, 0x04, 0x28, 0xD9, 0xE9, 0xC7, 0x3C,
    0x77, 0xE9, 0x72, 0x78, 0xF5, 0x89, 0x12, 0xB6, 0x8B, 0xD9, 0x7D, 0x34, 0x63, 0xBA, 0x85, 0x79,
    0x08, 0x37, 0x9F, 0x43, 0xB6, 0xD2, 0x9A, 0x96, 0xDB, 0xE1, 0x35, 0x91, 0xF8, 0x83, 0xA5, 0xA8,
    0x03, 0xA2, 0xE0, 0x69, 0xF8, 0x5F, 0x9A, 0x4C, 0x65, 0x0C, 0x75, 0xB7, 0x97, 0x51, 0x66, 0x64,
    0x35, 0xDF, 0x09, 0xA0, 0x52, 0x2B, 0xC5, 0xFD, 0x11, 0x33, 0xBB, 0xC3, 0x1F, 0x0F, 0xD5, 0x2E,
    0x40, 0x0C, 0x3F, 0x88, 0x9D, 0xB1, 0x48, 0x94, 0xF5, 0x59, 0x2B, 0x59, 0xD1, 0x06, 0x33, 0x05,
    0x94, 0x18, 0x52, 0xC4, 0xF9, 0xB5, 0x58, 0xEA, 0x53, 0xE7, 0xE4, 0x00, 0x92, 0xC8, 0xFF, 0xB9
};


static int read_cert_raw(uint32_t *entry_out, uint32_t *size_out)
{
    static storage_device_t *storage  = NULL;
    static partition_device_t  *ptdev = NULL;
    unsigned long long ptn     = 0;
    static unsigned long long size    = 0;
    static void *entry                = NULL;
    struct mmc_cfg mmc_cfg = {
        .voltage = MMC_VOL_1_8,
        .max_clk_rate = MMC_CLK_100MHZ,
        .bus_width = MMC_BUS_WIDTH_8BIT,
    };

    if (storage) {
        *entry_out = (uint32_t)entry;
        *size_out = size;
        return 0;
    }

    storage = setup_storage_dev(MMC, RES_MSHC_SD1, &mmc_cfg);

    ASSERT(storage != NULL);
    ptdev =  ptdev_setup(storage, BOOT_EMMC_GPT_START);
    ASSERT(ptdev != NULL);
    ptdev_read_table(ptdev);

    entry = (void *)_ioaddr((paddr_t)SSYSTEM_BASE);
    ptn  = ptdev_get_offset(ptdev, PT_CERT_NAME);
    size = ptdev_get_size(ptdev, PT_CERT_NAME);

    if (!ptn || size > SSYSTEM_MAX_SIZE) {
        LTRACEF("get partition offset error\n");
        return -1;
    }

    /* Load image from storage. */
    if (storage->read(storage, ptn, (uint8_t *)entry, size) < 0) {
        LTRACEF("read partition error\n");
        return -1;
    }

    *entry_out = (uint32_t)entry;
    *size_out = size;
    return 0;
}


static int verify_cert(const uint8_t *tbs, uint32_t tbs_len,
                       const uint8_t *sig, uint32_t sig_len,
                       const uint8_t *modulus, uint32_t modulus_len,
                       const uint8_t *expo, uint32_t expo_len,
                       hash_type hash)
{
    uint32_t ret = 0;
    uint8_t *modulus_align = NULL;
    uint8_t *tbs_align = NULL;
    uint8_t *sig_align = NULL;
    uint8_t *expo_align = NULL;

    modulus_align = memalign(CACHE_LINE, modulus_len);

    if (!modulus_align) {
        LTRACEF("modulus  allocat memory fail\n");
        ret = -1;
        goto end;
    }

    memcpy(modulus_align, modulus, modulus_len);

    tbs_align = memalign(CACHE_LINE, tbs_len);

    if (!tbs_align) {
        LTRACEF("tbs  allocat memory fail\n");
        ret = -1;
        goto end;
    }

    memcpy(tbs_align, tbs, tbs_len);

    sig_align = memalign(CACHE_LINE, sig_len);

    if (!sig_align) {
        LTRACEF("sig  allocat memory fail\n");
        ret = -1;
        goto end;
    }

    memcpy(sig_align, sig, sig_len);

    expo_align = memalign(CACHE_LINE, expo_len);

    if (!expo_align) {
        LTRACEF("exponent  allocat memory fail\n");
        ret = -1;
        goto end;
    }

    memcpy(expo_align, expo, expo_len);
    //LTRACEF("tbs:\n" );
    //hexdump8(tbs_align, tbs_len);

    //LTRACEF("\n\n%s %d sign:\n" );
    //hexdump8(sig_align, sig_len);
#if USE_CE_VERIFY
    ret = ce_verify_cert(tbs_align, tbs_len,
                         sig_align, sig_len,
                         modulus_align, modulus_len,
                         expo_align, expo_len, hash);

    LTRACEF("\n\n ce ret:%d\n\n",   ret);
#else
    ret = boringssl_verify_cert(tbs_align, tbs_len,
                                sig_align, sig_len,
                                modulus_align, modulus_len,
                                expo_align, expo_len, hash);

    LTRACEF("\n\n boringssl ret:%d\n",   ret);
#endif
end:

    if (tbs_align)
        free(tbs_align);

    if (modulus_align)
        free(modulus_align);

    if (sig_align)
        free(sig_align);

    return ret;

}

static int parse_cert(uint8_t *buffer, uint32_t size)
{
    int ret = 0;
    X509_RSA_CERT cert_parse = {0};
    hash_type hash = HASH_TYPE_SHA256;
    uint8_t NID_sha256rsa[] = NID_SHA256WITHRSAENCRYPTION;
    uint8_t NID_sha512rsa[] = NID_SHA512WITHRSAENCRYPTION;

    ret = X509_RsaCertParse(buffer, size, &cert_parse);
    LTRACEF("ret:%d\n",   ret);

    LTRACEF("tbs:\n" );
    hexdump8(cert_parse.tbs, cert_parse.tbs_len);

    LTRACEF("\n\n modulus:\n" );
    hexdump8(cert_parse.n, cert_parse.n_len);

    LTRACEF("\n\n e:\n" );
    hexdump8(cert_parse.e, cert_parse.e_len);

    LTRACEF("\n\n sign:\n" );
    hexdump8(cert_parse.sign, cert_parse.sign_len);

    LTRACEF("\n\n alg:\n" );
    hexdump8(cert_parse.alg, cert_parse.alg_len);

    if (!memcmp(cert_parse.alg,
                NID_sha256rsa,
                sizeof NID_sha256rsa)) {
        hash = HASH_TYPE_SHA256;
        LTRACEF("alg:%s \n",   "sha256");
    }
    else if (!memcmp(cert_parse.alg,
                     NID_sha512rsa,
                     sizeof NID_sha512rsa)) {
        hash = HASH_TYPE_SHA512;
        LTRACEF("alg:%s \n",   "sha512");
    }
    else {
        LTRACEF("unsupport alg\n");
        return 1;
    }

    verify_cert(cert_parse.tbs, cert_parse.tbs_len,
                cert_parse.sign, cert_parse.sign_len,
                g_modulus, sizeof(g_modulus),
                cert_parse.e, cert_parse.e_len,
                hash);
    return 0;
}

static int vb_verify_cert(int argc, const cmd_args *argv)
{
    uint32_t entry = 0;
    uint32_t size = 0;

    read_cert_raw(&entry, &size);
    parse_cert((uint8_t *)entry, size);
    return 0;
}

static int vb_verify_partition(int argc, const cmd_args *argv)
{
    storage_device_t *storage  = NULL;
    partition_device_t  *ptdev = NULL;
    uint32_t block_size = 0;
    uint64_t ptn  = 0;
    uint64_t size = 0;
    void *addr = NULL;
    int ret = 0;
    struct AvbOps *ops = NULL;
    AvbFooter footer = {0};
    const char *request_partition[2] = {PT_FOR_TEST, NULL};
    AvbSlotVerifyFlags verify_flags = AVB_SLOT_VERIFY_FLAGS_NONE;
    AvbHashtreeErrorMode verity_flags =
        AVB_HASHTREE_ERROR_MODE_RESTART_AND_INVALIDATE;
    AvbSlotVerifyData *slot_data = NULL;
    AvbSlotVerifyResult verify_ret = AVB_SLOT_VERIFY_RESULT_OK;
    const char *slot_suffix = "\0";

    struct mmc_cfg mmc_cfg = {
        .voltage = MMC_VOL_1_8,
        .max_clk_rate = MMC_CLK_100MHZ,
        .bus_width = MMC_BUS_WIDTH_8BIT,
    };

    storage = setup_storage_dev(MMC, RES_MSHC_SD1, &mmc_cfg);
    ASSERT(storage != NULL);
    ptdev =  ptdev_setup(storage, BOOT_EMMC_GPT_START);
    ASSERT(ptdev != NULL);
    ptdev_read_table(ptdev);

    ptn  = ptdev_get_offset(ptdev, PT_FOR_TEST);
    avb_get_footer_from_partition(ptdev, PT_FOR_TEST, &footer);
    //size = ptdev_get_size(ptdev, PT_FOR_TEST);
    size = footer.original_image_size;
    if (!ptn || !size) {
        LTRACEF("get %s partition info error!", PT_FOR_TEST);
        ret = -1;
        goto out;
    }

    ops = avb_ops_new(ptdev, NULL);

    if (!ops) {
        LTRACEF("ops allocate memory error!");
        ret = -1;
        goto out;
    }

    addr = (void *)_ioaddr((paddr_t)SSYSTEM_BASE);
    block_size = storage->get_block_size(storage);

    if (storage->read(storage, ptn, addr, round_up(size, block_size))) {
        LTRACEF("allocate memory error!");
        ret = -1;
        goto out;
    }

    avb_add_preload_image_info(ops, (addr_t)addr, size, PT_FOR_TEST);

    verify_ret = avb_slot_verify(ops, request_partition, slot_suffix,
                                 verify_flags, verity_flags, &slot_data);
    LTRACEF("verify ret:%s\n!", avb_slot_verify_result_to_string(verify_ret));
out:

    if (verify_ret == AVB_SLOT_VERIFY_RESULT_OK) {
        LTRACEF("num vbmeta:%d num partition:%d cmdline:%s!",
                slot_data->num_vbmeta_images,
                slot_data->num_loaded_partitions, slot_data->cmdline);
        avb_slot_verify_data_free(slot_data);
    }

    avb_ops_free(ops);
    return ret;
}

#if defined(WITH_LIB_CONSOLE)
#include <lib/console.h>

STATIC_COMMAND_START
//STATIC_COMMAND("vb_init", "parse x509 cert",
//               (console_cmd)&boringssl_parse_cert)
STATIC_COMMAND("vb_vcert", "verify x509 cert",
               (console_cmd)&vb_verify_cert)
STATIC_COMMAND("vb_test", "test verifyboot",
               (console_cmd)&vb_verify_partition)
STATIC_COMMAND_END(vbmeta);
#endif
