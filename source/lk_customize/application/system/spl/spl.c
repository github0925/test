/*
 * spl.c
 *
 * Copyright (c) 2019 Semidrive Semiconductor.
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
#include <string.h>
#include <lib/console.h>
#include <lib/reg.h>
#include <lib/reboot.h>
#include <lib/sdrv_common_reg.h>
#include <trace.h>

#include <mbox_hal.h>
#include <chip_res.h>
#include "clkgen_hal.h"
#include "ckgen_cfg.h"
#include "ddr_init.h"
#include <lk/init.h>
#ifndef BACKDOOR_DDR
#include "fastboot_common.h"
#include "md5.h"
#include "mmc_hal.h"
#include "partition_parser.h"
#include "scr_hal.h"
#include "storage_device.h"
#include "boot_device_cfg.h"
#include "spl_configs.h"
#include "partition_load_configs.h"
#endif
#include "pll_hal.h"
#include "peer_load.h"
#include "rstgen_hal.h"
#include "pll.h"
#include "res.h"
#include "boot.h"
#if !NO_DDR
#include <module_helper_hal.h>
#endif
#include "image_cfg.h"
#if CFG_PARSE_RUN_DDR_INIT_SEQ
#include "ddr_init_helper.h"
#include "crc32.h"
#endif

#if VERIFIED_BOOT
#include "verified_boot.h"
#endif

#ifndef SSYSTEM_BASE
#error "SSYSTEM_BASE not define"
#endif

#ifndef SSYSTEM_MAX_SIZE
#error "SSYSTEM_MAX_SIZE not define"
#endif
/* Boot mode get from pin/fuse/scr instead of bt block info, remove this later */

#ifndef DLOADER_BASE
#error "DLOADER_BASE not define"
#endif

#ifndef DLOADER_MAX_SIZE
#error "DLOADER_MAX_SIZE not define"
#endif

/* Peer load part name for both sec and saf */
#define PEER_LOAD_PART "peer_load"

#define DEFAULT_BOOT_DEVICE    "emmc1"
#define BOOT_INFO_BTDEV_START  8
#define BOOT_EMMC1_CODE        0x1
#define BOOT_EMMC2_CODE        0x2
#define BOOT_OSPI2_CODE        0x11
#define BOOT_USB_CODE          0x30
#define BOOT_PEER_CODE         0x40
#define SD_BOOT                0xf
#define BOOT_MEMDISK           0xf0

#define BOOT_EMMC_GPT_START    0
#define BOOT_OSPI_GPT_START    0x20000

#define SWITCH_USER_PART    0
#define SWITCH__BOOT_PART1  1
#define SWITCH__BOOT_PART2  2

#define SSYSTEM_PT_NAME  "ssystem"
#define DLOADER_PT_NAME   "dloader"
#define OSPIPROG_PT_NAME   "ospiprog"

#define RSTGEN_USB1_MODULE_ADDR (APB_RSTGEN_SEC_BASE+((0x100+(0x4*50))<<10))
#define RSTGEN_USB2_MODULE_ADDR (APB_RSTGEN_SEC_BASE+((0x100+(0x4*51))<<10))
#define USB1_CK_GATE_ADDR (APB_CKGEN_SOC_BASE + ((0x400 + 4*50)<<10))
#define USB1_CK_GATE_REF_ADDR (APB_CKGEN_SOC_BASE + ((0x400 + 4*52)<<10))

#define MAC_GLB_CTRL_BASE       (0xf0bc0000)
#define FIREWALL_PERCK_DIS_BIT  (0x2)
#define FIREWALL_DOMCK_DIS_BIT  (0x0)

#define SCR_NOC2DDR_FIFO_WRAP_BYPASS_ADDR (APB_SCR_SEC_BASE+(0x45c<<10))
#define SCR_NOC2DDR_FIFO_WRAP_BYPASS_OFFSET 2

#ifndef ARRAYSIZE
#define ARRAYSIZE(A) (sizeof(A)/sizeof(A[0]))
#endif

#define LOCAL_TRACE 0 // log on 0 -> 1

extern void platform_cpu_reset(addr_t);

struct boot_device_info {
    uint8_t code;
    const char *name;
    uint32_t gpt_start;
};

struct original_image_info {
    uint32_t size;
    uint32_t offset;
};

#ifndef BACKDOOR_DDR
static uint8_t md5_received[MD5_LEN];

static fastboot_t *fb_data;
static bool do_md5_check;
static reboot_args_t reboot_args;

PT_LOAD_CONFIGS(spl_configs);

#define SPL_CONFIGS_COUNT (sizeof(spl_configs)/sizeof(spl_configs[0]))

#endif

struct clock_info {
    uint32_t resid;
    clkgen_app_ip_cfg_t clk;
};

#define spl_ip_clk {\
    {\
        .resid = RES_IP_SLICE_SEC_I2C_SEC0,\
        .clk = {2, 0, 0},\
    },\
    {\
        .resid = RES_IP_SLICE_SEC_I2C_SEC1,\
        .clk = {2, 0, 0},\
    },\
    {\
        .resid = RES_IP_SLICE_SEC_UART_SEC0,\
        .clk = {4, 0, 1},\
    },\
    {\
        .resid = RES_IP_SLICE_SEC_UART_SEC1,\
        .clk = {4, 0, 1},\
    },\
    {\
        .resid = RES_IP_SLICE_SEC_EMMC1,\
        .clk = {4, 0, 0},\
    },\
    {\
        .resid = RES_IP_SLICE_SEC_EMMC2,\
        .clk = {4, 0, 0},\
    },\
    {\
        .resid = RES_IP_SLICE_SEC_EMMC3,\
        .clk = {4, 0, 0},\
    },\
    {\
        .resid = RES_IP_SLICE_SEC_EMMC4,\
        .clk = {4, 0, 1},\
    },\
    {\
        .resid = RES_IP_SLICE_SEC_OSPI2,\
        .clk = {7, 0, 0},\
    },\
    {\
        .resid = RES_IP_SLICE_SEC_TRACE,\
        .clk = {4, 0, 1},\
    },\
    {\
        .resid = RES_IP_SLICE_SEC_SYS_CNT,\
        .clk = {2, 0, 7},\
    },\
    {\
        .resid = RES_IP_SLICE_SEC_MSHC_TIMER,\
        .clk = {2, 0, 23},\
    },\
    {\
        .resid = RES_IP_SLICE_SEC_HPI_CLK600,\
        .clk = {4, 0, 0},\
    },\
    {\
        .resid = RES_IP_SLICE_SEC_HPI_CLK800,\
        .clk = {4, 0, 0},\
    }\
}

static void setup_pll(uint32_t resid)
{
    pll_handle_t pll;
    pll =  hal_pll_create_handle(resid);

    if (pll == (pll_handle_t)0) {
        printf("pll res 0x%x not belong this domain\n", resid);
        return;
    }

    hal_pll_config(pll, NULL);
    hal_pll_delete_handle(pll);
}
#if (DDR_FREQ == 4266)
#define DDR_STATE DDR_1066M
#elif (DDR_FREQ == 3200)
#define DDR_STATE DDR_800M
#elif (DDR_FREQ == 2133)
#define DDR_STATE DDR_532M
#elif (DDR_FREQ == 1600)
#define DDR_STATE DDR_400M
#elif (DDR_FREQ == 800)
#define DDR_STATE DDR_200M
#else
#define DDR_STATE DDR_200M
#endif
static uint8_t char2hex(char c)
{
    if (c >= '0' && c <= '9') {
        return c - '0';
    }
    else if (c >= 'a' && c <= 'f') {
        return c - 'a' + 10;
    }
    else if (c >= 'A' && c <= 'F') {
        return c - 'A' + 10;
    }

    return 0;
}

static void str2hex(const char *str, uint32_t str_len, uint8_t *hex,
                    uint32_t hex_len)
{
    for (uint32_t i = 0; i < str_len / 2 && i < hex_len; i++) {
        hex[i] = 0;
        hex[i] = (char2hex(str[i * 2]) & 0xF) << 4;
        hex[i] |= char2hex(str[i * 2 + 1]) & 0xF;
    }
}

static void setup_default_clk(void)
{
    void *handle = NULL;
    bool ret = false;

    ret = hal_clock_creat_handle(&handle);
    ASSERT(ret);
#if 0
    clkgen_app_ip_cfg_t clk = {4, 0, 0};
    ret = hal_clock_ipclk_set(handle, RES_IP_SLICE_SEC_HPI_CLK600, &clk);
    ASSERT(ret);
    ret = hal_clock_ipclk_set(handle, RES_IP_SLICE_SEC_HPI_CLK800, &clk);
    ASSERT(ret);
#else
    uint32_t i;
    static struct clock_info ip[] = spl_ip_clk;

    for (i = 0; i < ARRAYSIZE(ip); i++) {
        ret = hal_clock_ipclk_set(handle, ip[i].resid, &(ip[i].clk));
        assert(ret);
    }

#endif

    hal_clock_release_handle(handle);
}

void setup_noc_clk(void)
{
    void *handle = NULL;
    clkgen_app_bus_cfg_t bus_noc_cfg = {0};
    bool ret = false;
    ret = hal_clock_creat_handle(&handle);
    ASSERT(ret);
    bus_noc_cfg.clk_src_select_a_num = 4;
    bus_noc_cfg.clk_src_select_b_num = 4;
    bus_noc_cfg.clk_a_b_select = 0;
    bus_noc_cfg.pre_div_a = 0;
    bus_noc_cfg.pre_div_b = 0;
    bus_noc_cfg.post_div = 1;
    bus_noc_cfg.m_div = 1;
    bus_noc_cfg.n_div = 5;
    bus_noc_cfg.p_div = 5;
    bus_noc_cfg.q_div = 7;
    hal_clock_busclk_set(handle, RES_BUS_SLICE_SOC_NOC_BUS_CLOCK_CTL,
                         &bus_noc_cfg);
    hal_clock_release_handle(handle);
}

/* usb module power supply */
static void setup_usb_iso(void)
{
    void *handle = NULL;
    bool ret = true;
    ret = hal_rstgen_creat_handle(&handle, RES_GLOBAL_RST_SEC_RST_EN);

    if (!ret) {
        dprintf(CRITICAL, "%s usb module supply rst create handle fail!\n",
                __func__);
        return;
    }

    /*get handle ok and enable rstgen is true*/
    ret = hal_rstgen_init(handle);

    if (!ret) {
        dprintf(CRITICAL, "%s usb module supply rst init fail!\n", __func__);
    }

    ret = hal_rstgen_iso_disable(handle, RES_ISO_EN_SEC_USB);

    if (!ret) {
        dprintf(CRITICAL, "%s usb module supply setup fail!\n", __func__);
    }

    hal_rstgen_release_handle(handle);
}

static void clock_enable(uint32_t resid)
{
    void *handle = NULL;
    bool ret = false;
    ret = hal_clock_creat_handle(&handle);
    ASSERT(ret);
    ret = hal_clock_enable(handle, resid);
    ASSERT(ret);
    hal_clock_release_handle(handle);
}

static void setup_usb_clk(void)
{
    dprintf(CRITICAL, "usb1 ckgen_lp_gating_disable \n");
    RMWREG32(USB1_CK_GATE_ADDR, 1, 1, 0x0);
    RMWREG32(USB1_CK_GATE_REF_ADDR, 1, 1, 0x0);
}

static bool reg_poll_value(vaddr_t reg, int start, int width,
                           uint32_t value, int retrycount)
{
    uint32_t v = 0;
    int count = retrycount;

    do {
        v = readl(reg);

        if (((v >> start) & ((1 << width) - 1)) == value) {
            return true;
        }
    }
    while (--retrycount);

    dprintf(CRITICAL, "%s timeount:%d\n", __func__, count);
    return false;
}

static void reset_usb(void)
{
    RMWREG32(RSTGEN_USB1_MODULE_ADDR, 0, 2, 0x3);
    reg_poll_value(RSTGEN_USB1_MODULE_ADDR, 30, 1, 1, 5000);
}

void setup_his_clk(void)
{
    void *handle = NULL;
    clkgen_app_uuu_cfg_t uuu_clk = {0};
    bool ret = false;
    ret = hal_clock_creat_handle(&handle);
    ASSERT(ret);
    uuu_clk.uuu_input_clk_sel = uuu_input_pll_clk;
    uuu_clk.low_power_mode_en = 0;
    uuu_clk.m_div = 0;
    uuu_clk.n_div = 9;
    uuu_clk.p_div = 1;
    uuu_clk.q_div = 3;
    ret = hal_clock_uuuclk_set(handle, RES_UUU_WRAP_SOC_HIS_BUS, &uuu_clk);
    dprintf(CRITICAL, "%s ret:%d\n", __func__, ret);
    hal_clock_release_handle(handle);
}

static void set_scr(uint64_t resid, uint32_t v)
{
    scr_handle_t handle;

    handle  = hal_scr_create_handle(resid);

    hal_scr_set(handle, v);

    hal_scr_delete_handle(handle);
}

/*
 * SPL initial HW setup.
 *
 * Clocks that have been configured by ROM:
 *   PLL_SAF: PLL 1/2
 *   PLL_SEC: PLL 3/4/5
 *   CKGEN_SAF: PLAT_SAF including Saf_R5 and FAB_SAF
 *   CKGEN_SEC: PLAT_SEC including Sec_R5, FAB_SEC and FAB_SEC_m
 */
static void spl_init(void)
{
    reboot_args = (reboot_args_t)sdrv_common_reg_get_u32(SDRV_REG_BOOTREASON);
#if ENABLE_SD_SCR
    set_scr(SCR_SEC__L31__noc2ddr_fifo_wrap_bypass, 0x1);
#else
    uint32_t data = 0;
    paddr_t phy_addr_temp = SCR_NOC2DDR_FIFO_WRAP_BYPASS_ADDR;

    data = readl(phy_addr_temp);
    LTRACEF("before ddr init = 0x%x, data = 0x%x\n", (uint32_t)phy_addr_temp, data);

    // set noc2ddr_fifo_wrap_bypass before ddr init
    data = data | (1 << SCR_NOC2DDR_FIFO_WRAP_BYPASS_OFFSET) ;
    writel(data, phy_addr_temp);
#endif

    /* Initialize DDR controller & PHY. */
    /* if reboot for sw update, ddr has been inited by dil, skip */
    if (reboot_args.args.reason != HALT_REASON_SW_UPDATE) {
#if !NO_DDR
#if !CFG_PARSE_RUN_DDR_INIT_SEQ
        printf("change ddr state to %d\n", DDR_STATE);
        module_set_state(PER_ID_DDR, DDR_STATE);

#endif
        printf("DDR init\n");

        if (0 != ddr_init()) {
            printf("Opps, DDR initialization failed\n");
        }
        else {
            printf("DDR initialization done.\n");
        }
#endif
    }
}

void early_mbox_init(void)
{
    hal_mb_cfg_t hal_cfg;
    void *spl_mbox_handle;
    hal_mb_create_handle(&spl_mbox_handle, RES_MB_MB_MEM);

    if (spl_mbox_handle != NULL) {
        hal_mb_init(spl_mbox_handle, &hal_cfg);
    }
}

#ifndef BACKDOOR_DDR
static int send_peer_load_msg(uint32_t parameter)
{
    hal_mb_client_t cl;
    hal_mb_chan_t *mchan;
    status_t ret;
    struct peer_boot_message msg;
    early_mbox_init();
    msg.tag = PEER_LOAD_MSG_TAG;
    msg.size = PEER_LOAD_MSG_LEN;
    msg.command = PEER_LOAD_MSG_CMD;
    msg.version = PEER_LOAD_MSG_VERSION;
    msg.parameter = parameter;
    cl = hal_mb_get_client();

    if (!cl) {
        dprintf(CRITICAL, "mailbox get client error!\n");
        goto cl_fail;
    }

    mchan = hal_mb_request_channel(cl, true, NULL, IPCC_RRPOC_SAF);

    if (!mchan) {
        dprintf(CRITICAL, "mailbox request channel error!\n");
        goto chan_fail;
    }

    ret = hal_mb_send_data_rom(mchan, (u8 *)&msg, PEER_LOAD_MSG_LEN);

    if (ret != NO_ERROR) {
        dprintf(CRITICAL, "mailbox send data to rom error ret:%d\n", ret);
    }

    hal_mb_free_channel(mchan);
chan_fail:
    hal_mb_put_client(cl);
cl_fail:
    return 0;
}

static void disable_firewall(void)
{
    uint32_t mac_g_v = readl(MAC_GLB_CTRL_BASE);
    dprintf(ALWAYS, "%s %d FE mac:0x%0x\n", __func__, __LINE__, mac_g_v);
    mac_g_v &= ~(0x1u << FIREWALL_DOMCK_DIS_BIT);
    writel(mac_g_v, MAC_GLB_CTRL_BASE);
}

static uint32_t verify_dloader(const void *data, unsigned sz,
                               struct original_image_info *out_info)
{
#if VERIFIED_BOOT
    AvbOps *avb_ops = NULL;
    uint32_t original_image_size = sz;
    uint32_t original_image_offset = 0;
    char const *request_partition[2]  = {NULL, NULL};
    AvbSlotVerifyData *slot_data = NULL;
    AvbSlotVerifyResult verify_ret = AVB_SLOT_VERIFY_RESULT_OK;
    AvbSlotVerifyFlags verify_flags   = AVB_SLOT_VERIFY_FLAGS_NONE;
    AvbHashtreeErrorMode verity_flags = AVB_HASHTREE_ERROR_MODE_EIO;
    AvbFooter footer = {0};
    const uint8_t *vbmeta_pos = NULL;
    uint32_t vbmeta_size = 0;
    struct public_key_blob *pk_blob = NULL;

    if (!avb_get_footer_from_buffer(data, sz, &footer)) {
        dprintf(CRITICAL, "cann't get footer");
        return false;
    }

    vbmeta_pos = data + footer.vbmeta_offset;
    vbmeta_size = footer.vbmeta_size;
    original_image_size = avb_get_image_size_from_bpt(data, BPT_SIZE);
    original_image_offset = BPT_SIZE;

    /* Here, it means there is no bpt header in dloader */
    if (!original_image_size) {
        original_image_size = footer.original_image_size;
        original_image_offset = 0;
    }
    else {
        pk_blob = avb_get_public_key_blob_from_bpt(data, BPT_SIZE);
    }

    avb_ops = avb_ops_new(NULL, pk_blob);

    if (!avb_ops) {
        dprintf(CRITICAL, "ops allocate memory error!");
        return false;
    }

    avb_add_preload_image_info(avb_ops, (addr_t)vbmeta_pos, vbmeta_size,
                               VBMETA_PARTITION_NAME);
    avb_add_preload_image_info(avb_ops, (addr_t)data, sz, DLOADER_PT_NAME);
    request_partition[0] = DLOADER_PT_NAME;
    verify_ret = avb_slot_verify(avb_ops, request_partition, "\0",
                                 verify_flags, verity_flags, &slot_data);
    LTRACEF("verify ret:%s\n!", avb_slot_verify_result_to_string(verify_ret));
    avb_ops_free(avb_ops);

    if (verify_ret == AVB_SLOT_VERIFY_RESULT_OK) {
        avb_slot_verify_data_free(slot_data);
    }
    else {
        dprintf(CRITICAL, "verify fail:%s",
                avb_slot_verify_result_to_string(verify_ret));
        return false;
    }

    out_info->size = original_image_size;
    out_info->offset = original_image_offset;
    return true;
#else
    out_info->size = sz;
    out_info->offset = 0;
    return true;
#endif
}

static void cmd_flash_proc(fastboot_t *fb, const char *arg, void *data,
                           unsigned sz)
{
    paddr_t entry = 0;
    uint8_t md5_calc[MD5_LEN] = {0};
    const char *err_msg = "";
    struct original_image_info img_info = {0};
    dprintf(INFO, "%s arg:%s data:%p sz:%d\n", __func__, arg, data, sz);
    md5(data, sz, md5_calc);

    if (do_md5_check && memcmp(md5_received, md5_calc, MD5_LEN)) {
        dprintf(CRITICAL, "%s md5 check fail!\n", __func__);
        err_msg = "md5 check fail";
        hexdump8(md5_received, MD5_LEN);
        hexdump8(md5_calc, MD5_LEN);
        goto fail;
    }

    if (!strncmp(arg, "emmc", 4)) {
        if (!verify_dloader(data, sz, &img_info)) {
            err_msg = "verify dloader fail\n";
            goto fail;
        }

        fastboot_common_okay(fb, "");
        fastboot_common_stop(fb);
        dprintf(INFO, "reset into dloader\n");
        entry = DLOADER_BASE;
        memmove((void *)entry, data + img_info.offset, img_info.size);
        arch_clean_cache_range(entry, img_info.size);
        platform_cpu_reset(entry);
    }
    else if (!strncmp(arg, "ospi", 4)) {
        memmove((void *)_ioaddr(IRAM4_BASE), data, sz);
        arch_clean_cache_range(_ioaddr(IRAM4_BASE), ROUNDUP(sz, CACHE_LINE));
        /* If rom usb act as winusb, safety rom cann't access iram2/iram3/iram4
         * unless firewall disabled
         */
        disable_firewall();
        /* Send message to rom */
        send_peer_load_msg(IRAM4_BASE);
        fastboot_common_okay(fb, "");
    }
    else {
        dprintf(ALWAYS, "%s %d da name error\n", __func__, __LINE__);
        goto fail;
    }

    return;
fail:
    fastboot_common_fail(fb, err_msg);
    return;
}

static void cmd_md5_proc(fastboot_t *fb, const char *arg, void *data,
                         unsigned sz)
{
    char  md5_received_str[MD5_LEN * 2 + 1] = {0};
    memset(md5_received, 0x0, MD5_LEN);
    memcpy(md5_received_str, arg, MD5_LEN * 2);
    fastboot_common_okay(fb, "");
    str2hex(md5_received_str, MD5_LEN * 2, md5_received, MD5_LEN);
    do_md5_check = true;
}

static void entry_download_mode(bool usb_init)
{
    static char max_download_size[MAX_RSP_SIZE];
    dprintf(INFO, "SPL entry download mode\n");
    snprintf(max_download_size, sizeof(max_download_size), "\t0x%x",
             DLOADER_MAX_SIZE);

    if (usb_init) {
        dprintf(INFO, "setup usb iso\n");
        setup_usb_iso();

        dprintf(INFO, "setup PLL HIS\n");
        setup_pll(RES_PLL_PLL_HIS);

        setup_his_clk();
        dprintf(INFO, "set usb clk\n");

        setup_usb_clk();
        dprintf(INFO, "rest usb\n");
        reset_usb();

    }

    do_md5_check = false;
    fastboot_register_var("max-download-size",
                          (const char *) max_download_size);
    fastboot_register_var("dev-stage", "spl");
    fastboot_register_cmd("flash:", cmd_flash_proc);
    fastboot_register_cmd("md5:", cmd_md5_proc);
    fb_data = fastboot_common_init((void *)DLOADER_BASE, DLOADER_MAX_SIZE);
}


static int entry_peer_load(partition_device_t *ptdev)
{
    storage_device_t *storage = ptdev->storage;
    uint64_t safety_ptn = ptdev_get_offset(ptdev, PEER_LOAD_PART);
    uint64_t safety_size = ptdev_get_size(ptdev, PEER_LOAD_PART);

    if (!safety_ptn || !safety_size) {
        dprintf(CRITICAL, "no peer load part found.\n");
        return -1;
    }

    /* Transfor over IRAM4 */
    void *buffer = (void *)IRAM4_BASE;
    uint32_t ret = storage->read(storage, safety_ptn, (uint8_t *)buffer,
                                 safety_size);

    if (ret) {
        dprintf(CRITICAL, "failed to load saf spl.\n");
        return -1;
    }

    /* Safety binary security verify */
    /* TODO */
    /* Send message to rom */
    send_peer_load_msg(IRAM4_BASE);
    return 0;
}

static int default_complete(void *config, void *arg)
{
    /*TODO, secure boot verify */
    struct pt_load_config *_config = config;
#if VERIFIED_BOOT
    partition_device_t  *ptdev = arg;
    bool ret = false;
    static struct list_node verified_images_list = LIST_INITIAL_VALUE(
                verified_images_list);
    ret = add_verified_image_list(&verified_images_list,
                                  (void *)(addr_t)_config->load_addr,
                                  _config->load_size, _config->pt_name);
    ASSERT(ret);
#endif

    if (_config->flags & PT_KICK_F) {
#if VERIFIED_BOOT

        if (!verify_loaded_images(ptdev, &verified_images_list, NULL)) {
            dprintf(ALWAYS, "%s %d verify images fail\n", __func__, __LINE__);
            free_image_info_list(&verified_images_list);
            return -1;
        }

        free_image_info_list(&verified_images_list);
#endif
        platform_cpu_reset(_config->load_addr);
    }

    return 0;
}

static void spl_launch_nextimage(void)
{
    int ret;
    bool usb_init;
    uint32_t pin = boot_get_pin();
    storage_device_t *storage  = NULL;
    partition_device_t  *ptdev = NULL;
    boot_device_cfg_t *btdev_cfg = NULL;

    dprintf(INFO, "boot reason reg:0x%0x\n", reboot_args.args.reason);

    usb_init = (reboot_args.args.reason == HALT_REASON_SW_UPDATE);

    if (pin == BOOT_PIN_8 || pin == BOOT_PIN_13
            || pin == BOOT_PIN_5 || usb_init) {
        entry_download_mode(usb_init);
        return;
    }

    btdev_pin_cfg_t *btdev_pin = find_btdev(pin);

    if (btdev_pin) {
        btdev_cfg = btdev_pin->ap;
    }

    if (btdev_cfg)
        storage = setup_storage_dev(btdev_cfg->device_type,
                                    btdev_cfg->res_idex, (void *)&btdev_cfg->cfg);

    ASSERT(storage != NULL);
    /* Parse GPT partition table for most boot mode */
    ptdev = ptdev_setup(storage, BOOT_EMMC_GPT_START);
    ASSERT(ptdev != NULL);
    ptdev_read_table(ptdev);
    register_load_complete_for_all(spl_configs, SPL_CONFIGS_COUNT,
                                   default_complete, ptdev);
    ret = load_all_partition(spl_configs, SPL_CONFIGS_COUNT, ptdev);

    if (ret) {
        dprintf(CRITICAL, "%s load all partiton error!\n", __func__);
    }
}
#endif

void spl_boot(void)
{
#ifndef BACKDOOR_DDR
    spl_launch_nextimage();
#else
    void *entry = NULL;
    entry = (void *)_ioaddr((paddr_t)SSYSTEM_BASE);
    platform_cpu_reset((addr_t)_paddr(entry));
#endif
}

void _spl_main(void)
{
    //dprintf(ALWAYS, "%s %d \n", __func__, __LINE__);
    spl_init();
    spl_boot();
}

int spl_main(int argc, const cmd_args *argv)
{
    _spl_main();
    return 0;
}

static void spl_entry(const struct app_descriptor *app, void *args)
{
    _spl_main();
}

static const uint32_t refclk_ResIdx[] = {
    RES_SCR_L16_SEC_FSREFCLK_HPI,
    RES_SCR_L16_SEC_FSREFCLK_CPU1,
    RES_SCR_L16_SEC_FSREFCLK_CPU2,
    RES_SCR_L16_SEC_FSREFCLK_GPU1,
    RES_SCR_L16_SEC_FSREFCLK_GPU2,
    RES_SCR_L16_SEC_FSREFCLK_HIS,
    RES_SCR_L16_SEC_FSREFCLK_VPU,
    RES_SCR_L16_SEC_FSREFCLK_VSN,
    RES_SCR_L16_SEC_FSREFCLK_DISP,
    RES_SCR_L16_SEC_FSREFCLK_DDR_SS,
};

static void refclk_config(void)
{
    void *g_sec_handle;
    bool ret = true;
    ret = hal_clock_creat_handle(&g_sec_handle);

    if (!ret) {
        printf("clkgen creat handle failed\n");
        return;
    }

    uint8_t glb_res_idx_size = sizeof(refclk_ResIdx) / sizeof(
                                   refclk_ResIdx[0]);

    for (uint8_t i = 0; i < glb_res_idx_size; i++) {
        ret = hal_clock_osc_init(g_sec_handle, refclk_ResIdx[i], xtal_saf_24M,
                                 true);
    }

    hal_clock_release_handle(g_sec_handle);
}

static void pll_ckgen_init(void)
{
    /* If USB boot mode, ROM code has initialized HPI clock
    * Otherwise,setup HPI bus clock since we will access DDR.
    */
    uint32_t pin = boot_get_pin();

    if (pin != BOOT_PIN_8 && pin != BOOT_PIN_13 && pin != BOOT_PIN_5
            && reboot_args.args.reason != HALT_REASON_SW_UPDATE) {
        setup_pll(RES_PLL_PLL_HPI);
    }

#ifdef BACKDOOR_DDR
    setup_pll(RES_PLL_PLL4);
#endif
    //hal_sec_clock_set_default();
    setup_default_clk();
    setup_noc_clk();
    sdrv_common_reg_set_value(SDRV_REG_STATUS,
                              SDRV_REG_STATUS_NOC_INIT_DONE,
                              SDRV_REG_STATUS_NOC_INIT_DONE);
}

void os_platform_early_init(void)
{
    refclk_config();
    pll_ckgen_init();
}

#if defined(WITH_LIB_CONSOLE)

int cmd_spl_init(int argc, const cmd_args *argv)
{
    spl_init();
    return 0;
}

int cmd_spl_boot(int argc, const cmd_args *argv)
{
    spl_boot();
    return 0;
}

#if CFG_DDR_VOLTAGE_ADJUST
extern int setup_ddr_voltage(uint32_t vdd_id, uint32_t mv);
int cmd_ddr_voltage(int argc, const cmd_args *argv)
{
    if (argc != 3) {
        printf("Opps, invalid paras\n");
        return -1;
    }

    uint32_t id = strtoul(argv[1].str, NULL, 0);
    uint32_t mv = strtoul(argv[2].str, NULL, 0);

    if ((id < 1) || (id > 3)) {
        printf("Opps, invalid vdd_id\n");
        return -2;
    }

    //printf("%s: vdd_id=%d, voltage=%d\n", __FUNCTION__, id, mv);

    if (((1 == id) && ((mv < 990) || (mv > 1210)))
            || ((2 == id) && ((mv < 540) || (mv > 660)))
            || ((3 == id) && ((mv < 1620) || (mv > 1890)))) {
        printf("Opps, para 'voltage' out of range\n");
        return -3;
    }

    if (0 != setup_ddr_voltage(id, mv)) {
        printf("Opps, failed to setup ddr voltage\n");
        return -4;
    }

    return 0;
}
#endif  /* CFG_DDR_VOLTAGE_ADJUST */

int cmd_board_rst(int argc, const cmd_args *argv)
{
    printf("Opps, not implemented yet.\n");
    return 0;
}

#if CFG_PARSE_RUN_DDR_INIT_SEQ
extern ddr_act_u ddr_init_seq[];
extern const uint32_t ddr_init_seq_sz_max;
int cmd_load_ddr_seq(int argc, const cmd_args *argv)
{
    uint8_t c;
    uint32_t rcvd = 0;
    uint8_t *p = (uint8_t *)ddr_init_seq;
    int res = -1;
    c = getchar();

    if (c == 0x81) {
        c = 0xc1;
        putchar(c);    /* Handshaked */
        /* Get size */
        uint32_t sz = 0;

        for (int i = 0; i < 4; i++) {
            c = getchar();
            sz |= c << (i * 8);
        }

        if (sz < ddr_init_seq_sz_max) {
            c = 0xc1;
            putchar(c);    /* Handshaked */

            while (rcvd < sz) {
                c = getchar();
                *p = c;
                p++;
                rcvd++;
            }

            /* Get CRC */
            uint32_t crc = 0;

            for (int i = 0; i < 4; i++) {
                c = getchar();
                crc |= c << (i * 8);
            }

            if (crc != Crc32_ComputeBuf(0, (const void *)&ddr_init_seq[0], sz)) {
                c = 0xc2;
            }
            else {
                c = 0xc1;
                res = 0;
            }
        }
        else {
            c = 0xc2;
        }

        putchar(c);
    }

    return res;
}
#endif  /* CFG_PARSE_RUN_DDR_INIT_SEQ */
#endif  /* defined(WITH_LIB_CONSOLE) */

#if defined(WITH_LIB_CONSOLE)
#include <lib/console.h>

STATIC_COMMAND_START

STATIC_COMMAND("spl", "Secure R5 spl", (console_cmd)&spl_main)
STATIC_COMMAND("spl_init", "spl init", (console_cmd)&cmd_spl_init)
STATIC_COMMAND("spl_boot", "spl boot", (console_cmd)&cmd_spl_boot)
#if CFG_DDR_VOLTAGE_ADJUST
STATIC_COMMAND("ddr_voltage", "ddr_voltage vdd_id voltage",
               (console_cmd)&cmd_ddr_voltage)
#endif
STATIC_COMMAND("board_rst", "board rst", (console_cmd)&cmd_board_rst)
#if CFG_PARSE_RUN_DDR_INIT_SEQ
STATIC_COMMAND("load_ddr_seq", "start to load ddr init sequence from TTY",
               (console_cmd)&cmd_load_ddr_seq)
#endif

STATIC_COMMAND_END(spl);

#endif

#if !CFG_PARSE_RUN_DDR_INIT_SEQ
APP_START(spl)
.flags = 0,
.entry = spl_entry,
APP_END
#endif
