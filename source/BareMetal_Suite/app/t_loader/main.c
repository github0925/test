/********************************************************
 *          Copyright(c) 2020   Semidrive               *
 ********************************************************/

#include <common_hdr.h>
#include <mini_libc/mini_libc.h>
#include <service.h>
#include <soc.h>
#include <arch.h>
#include <rstgen/rstgen.h>
#include "uart/uart.h"
#include "shell/shell.h"
#if defined(CFG_RUN_DDR_INIT_SEQ)
#include "ddr/dw_umctl2/inc/ddr_init_helper.h"
#endif
#if defined(BOARD)
#include "board.h"
#endif
#include "boardinfo_hwid_hw.h"
#include "boardinfo_hwid_usr.h"
#include "partition_parser.h"
#include "lib/sdrv_common_reg.h"
#include "mailbox.h"
#include "cksum.h"
#include "mem_image.h"
#include "peer_load.h"
#include "partition_configs.h"
#include "partition_load_configs.h"
#include "ddr_seq_parser.h"
#include "scr.h"
#if VERIFIED_BOOT
#include "verified_boot.h"
#endif
#include "str.h"
#ifdef SUPPORT_VIRT_UART
#include "vuart.h"
#endif

#define MB_MSG_ID_SEC 0
#define MB_MSG_ID_SAF 1

#define DIRECT_ACCESS_GPT  0
#define OSPI_BASE (0x4000000 + 0x2000)

#define DDR_INIT_SEQ_GPT_BASE (OSPI_BASE + 0x405000)
#define DDR_FW_GPT_BASE       (OSPI_BASE + 0x415000)
#define SDPE_GPT_BASE         (OSPI_BASE + 0x455000)
#define SSYSTEM_GPT_BASE      (OSPI_BASE + 0x555000)
#define ROUTING_GPT_BASE      (OSPI_BASE + 0x955000)
#define DIL2_GPT_BASE         (OSPI_BASE + 0x975000)

#define RSTGEN_GENERAL_REG(n)   ((0x50 + (n)*4) << 10)
#define RSTGEN_REMAP_STATUS_REG (APB_RSTGEN_SAF_BASE + RSTGEN_GENERAL_REG(1))

#define ROMC1_STICKY_REG        (APB_ROMC1_BASE + 0x34) //safe romc1
#define ROMC2_STICKY_REG        (APB_ROMC2_BASE + 0x34)
#define REMAP_REG               (APB_SCR_SAF_BASE + (0x400 << 10))
#define REMAP_MASK              (0xFFFFFU)
#define REMAP_DONE              (0x52454d50)   /* 'REMP' */

#define SAFETY_CORE_RST_EN_REG  (0xfc401000)
#define SAFETY_CORE_SW_RST_REG  (0xfc402000)
#define RSTGEN_SAF_CORE_RST_B_SAF_INDEX 0

#define LOCKSTEP_SCR_ADDR 0xfc297000
#define LOCKSTEP_SCR_BIT 0

#define RSTGEN_CORE_RST_RST_LOCK_MASK            ((uint32_t) (1 << 31))
#define RSTGEN_CORE_SW_RST_STATIC_RST_B_STA_MASK ((uint32_t) (1 << 29))
#define RSTGEN_CORE_RST_SW_RST_EN_SHIFT          (0U)
#define RSTGEN_CORE_RST_SW_RST_EN_STA_MASK       ((uint32_t) (1 << 30))
#define RSTGEN_CORE_SW_RST_AUTO_CLR_SHIFT        (1U)
#define RSTGEN_CORE_SW_RST_CORE_RST_STA_MASK     ((uint32_t) (1 << 30))
#define RSTGEN_CORE_SW_RST_STATIC_RST_SHIFT      (0U)

#define APB_SYS_CNT_RW_BASE (0xF1410000)
#define APB_SYS_CNT_RO_BASE (0xF1400000)

#define TSGEN_BASE APB_SYS_CNT_RW_BASE
#define CNTCR 0 /* counter control */

#define PORT_PAD_POE__DISABLE    (0x0U << 16)
#define PORT_PAD_IS__OUT         (0x1U << 12)
#define PORT_PAD_SR__FAST        (0x0U << 8)
#define PORT_PAD_DS__MID1        (0x1U << 4)
#define PORT_PIN_IN_PULL_DOWN    (0x1U)

#define PORT_PIN_MUX_FV__MIN     (0x0U << 12)
#define PORT_PIN_MUX_FIN__MIN    (0x0U << 8)
#define PORT_PIN_OUT_PUSHPULL    (0x00U)
#define PORT_PIN_MODE_GPIO       (0x00U)

#define BPT_SZ                   0x800
#define IIB_OFFSET               0x20
#define IIB_SZ_OFFSET            0x28

#define SAF_MAIN_RST_ADDR        (APB_SCR_SAF_BASE + (0x104 << 10))

#define FDA_SPL_NAME             "fda_spl"

#define PT_CONFIGS_CNT  (sizeof(pt_configs) / sizeof(pt_configs[0]))

#define USRID_2_SEQID(usr_id, ddr_id) \
        (((usr_id.v.v1.board_type & 0xf) << 12) | ((usr_id.v.v1.board_id_major & 0xf) << 8) | ((usr_id.v.v1.board_id_minor & 0xf) << 4) | (ddr_id & 0xf))

#define INFO_D(fmt,args...) INFO("%s %d "fmt, __func__, __LINE__, ##args)

#if defined(CFG_RUN_DDR_INIT_SEQ)
extern int32_t run_ddr_init_seq(const ddr_act_u *act);
#endif
extern void novm_init(uint level);
extern void heap_init(void);

/* workaround: if .data section is empty, objcopy not work properly */
char *prod_str = "t_loader";
#ifdef TGT_safe
const char *cpu_str = "Saf";
#else
const char *cpu_str = "Sec";
#endif

#if defined(BOARD_x9_ref)
const char *board_str = "BOARD_x9_ref";
#elif defined(BOARD_g9_ref)
const char *board_str = "BOARD_g9_ref";
#else
const char *board_str = "BOARD_not_specified";
#endif
static struct sd_hwid_usr usr_id;

#ifndef DIL2
PT_LOAD_CONFIGS(pt_configs);
#endif
int32_t board_setup(uint32_t part_rev, uint8_t type, uint8_t maj,
                    uint8_t min) __attribute__((weak));
int32_t board_setup(uint32_t part_rev, uint8_t type, uint8_t maj,
                    uint8_t min)
{
    return 0;
}

int32_t board_setup_later(uint32_t part_rev, uint8_t type, uint8_t maj,
                          uint8_t min) __attribute__((weak));
int32_t board_setup_later(uint32_t part_rev, uint8_t type, uint8_t maj,
                          uint8_t min)
{
    return 0;
}

module_e tty = TTY_UART;

#if !defined(DO_NOC_INIT) || defined(PEER_LOAD_DIL2)
static void send_peer_load_msg(int id, uint32_t addr)
{
    struct peer_boot_message msg = MK_PEER_LOAD_MSG(addr);
    DBG("send peer load msg: 0x%llx size:%d\n", addr,
        sizeof(struct peer_boot_message));

    if (id == MB_CPU_CR5_SEC) {
        mb_tx_smsg(MAILBOX, (0x1u << MB_CPU_CR5_SEC), MB_MSG_ID_SEC,
                   (uint8_t *)&msg, sizeof(struct peer_boot_message));
    }
    else if (id == MB_CPU_CR5_SAF) {
        mb_tx_smsg(MAILBOX, (0x1u << MB_CPU_CR5_SAF), MB_MSG_ID_SAF,
                   (uint8_t *)&msg, sizeof(struct peer_boot_message));
    }
}
#endif

#if defined(CFG_RUN_DDR_INIT_SEQ)
static uint8_t buffer[DEFAULT_BLK_SZ * 2]__ALIGNED(DEFAULT_BLK_SZ);
static bool get_matched_ddr_seq(uint32_t *offset, uint32_t *size)
{
    uint64_t ret;
    uint8_t ddr_id = 0;
    uint32_t default_off = 0, default_sz = 0;
    seq_hdr_t *header;
    seq_entry_t *entry;
    uint32_t entry_off;
    uint32_t i = 0;
    *offset = 0;
    *size = 0;
    ret = load_partition_by_name("ddr_init_seq", buffer,
                                 DEFAULT_BLK_SZ * 2, 0);

    if (!ret) {
        WARN("Opps, fail to load seq header!\n");
        goto out;
    }

    header = (seq_hdr_t *)buffer;

    if (header->tag != SEQ_HDR_TAG
            || header->num > MAX_SEQ_NUM
            || !header->num) {
        DBG("no valid ddr seq header!\n");
        goto out;
    }

    entry_off = sizeof(seq_hdr_t);
    entry = (seq_entry_t *)&buffer[entry_off];

    for (i = 0; i < header->num; i++, entry++) {
        if (entry->tag != SEQ_ENTRY_TAG
                || !entry->size) {
            DBG("invalid ddr seq entry!\n");
            goto out;
        }

        if (i == 0) {
            default_off = entry->start;
            default_sz = entry->size;
        }

        if (entry->id == (USRID_2_SEQID(usr_id, ddr_id) & entry->msk)) {
            *offset = entry->start;
            *size = entry->size;
            INFO("found matched ddr seq, i:%u id:%u mask:%u!\n", i, entry->id,
                 entry->msk);
            break;
        }
    }

    if (i == header->num) {
        DBG("don't found matched ddr seq, use the first!\n");
        *offset = default_off;
        *size = default_sz;
    }

    return true;
out:
    return false;
}

static void load_ddr_data(void)
{
	#ifdef DDR4_USED

     ddr_act_u ddr_init_seq[2048] __ALIGNED(DEFAULT_BLK_SZ); //ddr4
	 #else
		 
     ddr_act_u ddr_init_seq[1024] __ALIGNED(DEFAULT_BLK_SZ); //lpddr4
	 #endif
    uint32_t seq_off = 0, seq_sz = 0;
    get_matched_ddr_seq(&seq_off, &seq_sz);

    if (seq_sz > sizeof(ddr_init_seq)) {
        WARN("Opps, DDR_Init_Seq too large!\n");
        return;
    }
    else if (seq_off == 0) {
        seq_sz = sizeof(ddr_init_seq);
    }

    DBG("seq offset:%u seq size:%u!\n", seq_off, seq_sz);
    uint64_t ret;
    assert(ROUNDUP(seq_sz, DEFAULT_BLK_SZ) <= sizeof(ddr_init_seq));
    ret = load_partition_by_name("ddr_init_seq", (uint8_t *)(&ddr_init_seq[0]),
                                 ROUNDUP(seq_sz, DEFAULT_BLK_SZ), seq_off);

    if (!ret) {
        WARN("Opps, DDR_Init_Seq not found\n");
        return;
    }

    if ((0 != ddr_init_seq[0].call.act)
            && (0xffu != ddr_init_seq[0].call.act)) {
        DBG("To run DDR init sequence...\n");

        if (0 != run_ddr_init_seq(&ddr_init_seq[0])) {
            WARN("Opps, DDR init failed\n ");
        }
        else {
            DBG("DDR init OK.\n");
        }
    }
    else {
        WARN("Opps, DDR init seq invalid\n ");
    }
}
#endif

#ifdef DIL2
static void reset_safety_cr5(uint32_t entry)
{
    entry = entry >> 12;
    scr_bits_wr(APB_SCR_SAF_BASE, L31,
                SCR_SAF_REMAP_CR5_SAF_AR_ADDR_OFFSET_19_0_L31_START_BIT,
                SCR_SAF_REMAP_CR5_SAF_AR_ADDR_OFFSET_19_0_L31_WIDTH,
                entry);
    scr_bits_wr(APB_SCR_SAF_BASE, L31,
                SCR_SAF_REMAP_CR5_SAF_AR_REMAP_OVRD_EN_L31_START_BIT,
                SCR_SAF_REMAP_CR5_SAF_AR_REMAP_OVRD_EN_L31_WIDTH,
                0x1);
    /* Read the register back to make sure the APB transaction is
     * completed before we reset ourself.
     */
    readl(REMAP_REG);
    writel(REMAP_DONE, RSTGEN_REMAP_STATUS_REG);
    RMWREG32(ROMC1_STICKY_REG, 0, 1, 1);
    readl(ROMC1_STICKY_REG);
    /* disable lockstep */
    writel(readl(LOCKSTEP_SCR_ADDR) | (0x3 << LOCKSTEP_SCR_BIT),
           LOCKSTEP_SCR_ADDR);
    __asm__ volatile("dsb sy" ::: "memory");
    __asm__ volatile("isb" ::: "memory");
    __asm__ volatile("cpsid iaf" ::: "memory");
    rg_core_reset(APB_RSTGEN_SAF_BASE, RSTGEN_SAF_CORE_RST_B_SAF_INDEX);

    while (1) {
        __asm__ volatile("wfi");
    }
}
#endif

#if 0
static void time_statistics_gpio_cfg(uint8_t val)
{
    uint32_t gpio_ctrl_addr = APB_GPIO1_BASE + (56) * 0x10;
    uint32_t io_pad_addr    = APB_IOMUXC_SEC_BASE + (0x20 << 10);
    uint32_t io_mux_addr    = APB_IOMUXC_SEC_BASE + (0x220 << 10);
    uint32_t pad_val = ((uint32_t)PORT_PAD_POE__DISABLE | PORT_PAD_IS__OUT |
                        PORT_PAD_SR__FAST | PORT_PAD_DS__MID1 | PORT_PIN_IN_PULL_DOWN);
    uint32_t mux_val = ((uint32_t)PORT_PIN_MUX_FV__MIN | PORT_PIN_MUX_FIN__MIN
                        |
                        PORT_PIN_OUT_PUSHPULL | PORT_PIN_MODE_GPIO);
    writel(pad_val, io_pad_addr);
    writel(mux_val, io_mux_addr);
    writel(val, gpio_ctrl_addr);
}
#endif

static void load_safety_image(void)
{
    uint8_t *entry = (uint8_t *)DIL2_MEMBASE;
#ifdef PEER_LOAD_DIL2
    arch_disable_cache(ICACHE | DCACHE);
    __asm__ volatile("cpsid iaf" ::: "memory");
    /* safety needs to access iram2 */
    writel(0x10, 0xf0bc0000);
    send_peer_load_msg(MB_CPU_CR5_SAF, (uint32_t)entry);
    /* ROMC Stick Reg
     *
     * [0]: ROMC_STICKY_REMAP_EN. This bit is AND'ed with
     *      SCR remap_en bit, to enable remapping.
     */
    RMWREG32(ROMC2_STICKY_REG, 0, 1, 1);

    while (1) {
        __asm__ volatile("wfi");
    }

#else
#if VERIFIED_BOOT
    bool ret;
    size_t entry_img_sz = DIL2_MEMSIZE;
    void *vbmeta_buffer = (void *)(addr_t)VBMETA_MEMBASE;
    uint32_t vbmeta_size = VBMETA_MEMSIZE;
    static struct list_node verified_images_list = LIST_INITIAL_VALUE(
                verified_images_list);
    ret = add_verified_image_list(&verified_images_list, vbmeta_buffer,
                                  vbmeta_size, VBMETA_PARTITION_NAME);
    assert(ret);
    ret = add_verified_image_list(&verified_images_list, entry, entry_img_sz,
                                  "dil2");
    assert(ret);

    if (!verify_loaded_images(NULL, &verified_images_list, NULL)) {
        WARN("%s %d verify images fail\n", __func__, __LINE__);
        return;
    }

#endif
    arch_disable_cache(ICACHE | DCACHE);
    ((fv_v)entry)();
#endif
}

static void load_partition_by_configs(struct pt_load_config *configs,
                                      uint32_t cnt)
{
    uint64_t ret;
    addr_t seeker_base = IMG_BACKUP_LOW_BASE;
    addr_t seeker_size = IMG_BACKUP_LOW_SZ;
    uint64_t img_size;
    mem_image_entry_t info;
    DBG("PT cnt:%u\n", cnt);
    mem_image_init(seeker_base, seeker_size);

    for (uint32_t i = 0; i < cnt; i++, configs++) {
        if (configs->flag & PT_LD_DECD) {
            info.base = configs->base;
            info.sz = configs->sz;

            if (!info.base || !info.sz) {
                WARN("dedicated addr & sz cann't be zero!\n");
                return;
            }

            img_size = get_img_actual_size(configs->pt_name, NULL);
            DBG("didcated image %s base:%u size:%u%u\n", configs->pt_name,
                (uint32_t)info.base, (uint32_t)(img_size >> 32), (uint32_t)img_size);
        }
        else {
            img_size = get_img_actual_size(configs->pt_name, NULL);

            if (!img_size) {
                WARN("fail to get image size\n");
                continue;
            }

            if (!mem_image_get_entry(seeker_base, CACHE_LINE, DEFAULT_BLK_SZ, img_size,
                                     configs->pt_name, &info)) {
                WARN("fail to get image base %s\n", configs->pt_name);
                continue;
            }

            DBG("%s,base:%u%u sz:%u%u\n", configs->pt_name,
                (uint32_t)(info.base >> 32),
                (uint32_t)info.base, (uint32_t)(info.sz >> 32), (uint32_t)info.sz);
        }

        ret = load_partition_by_name(configs->pt_name, (void *)(addr_t)info.base,
                                     info.sz, 0);

        if (!ret) {
            WARN("fail to load partition:%s base:%llu buf_sz:%llu\n",
                 configs->pt_name, info.base, info.sz);
            mem_image_delete_entry(seeker_base, info.base);
        }

        /* Here, no need  to flush cache,
        *  we will disable & clean & invalidate cache before jumping to dil2
        * */
    }
}
static int load_fda_spl(void)
{
    uint64_t img_size;
    reboot_args_t reboot_args = (reboot_args_t)sdrv_common_reg_get_u32(
                                    SDRV_REG_BOOTREASON);

    if (reboot_args.args.reason == HALT_REASON_SW_UPDATE) {
        addr_t seeker_base = IMG_BACKUP_LOW_BASE;
        mem_image_entry_t info;
        img_size = get_img_actual_size(FDA_SPL_NAME, NULL);

        if (!img_size) {
            WARN("fail to get %s image size\n", FDA_SPL_NAME);
            return -1;
        }

        if (mem_image_get_entry(seeker_base, CACHE_LINE, DEFAULT_BLK_SZ, img_size,
                                FDA_SPL_NAME, &info)) {
            if (!load_partition_by_name(FDA_SPL_NAME, (void *)(addr_t)(info.base),
                                        info.sz, 0)) {
                WARN("fail to load fda_spl!\n");
                return -1;
            }
        }
    }

    return 0;
}

int __main(int argc, char *argv[])
{
    //time_statistics_gpio_cfg(0x5);
#if defined(DEBUG_ENABLE) || defined(INFO_LEVEL)
    soc_deassert_reset(TTY_UART);
    soc_pin_cfg(TTY_UART, NULL);
    soc_config_clk(TTY_UART, UART_FREQ1);
    uart_cfg_t uart_cfg;
    memclr(&uart_cfg, sizeof(uart_cfg));
    uart_cfg.parity = UART_PARITY_NONE;
    uart_cfg.stop = STOP_1BIT;
    uart_cfg.baud_rate = 115200u;
    uart_init(TTY_UART, &uart_cfg);
#endif
    tmr_enable();
#if defined(SUPPORT_HEAP)
    novm_init(0);
    heap_init();
#endif
    DBG("\n\n%s: %s, built on %s at %s\n\n", cpu_str, prod_str, __DATE__,
        __TIME__);
#ifndef DIL2
    struct sd_hwid_stor hwid = {0};
#if defined(BOARD_INFO)
    board_info(&hwid, sizeof(struct sd_hwid_stor));
#endif
    sys_recognize_hwid_from_hw(&hwid, &usr_id);
    DBG("Board hwid board type:%u  major:%u minor:%u\n",
        get_part_id(&usr_id, PART_BOARD_TYPE),
        get_part_id(&usr_id, PART_BOARD_ID_MAJ),
        get_part_id(&usr_id, PART_BOARD_ID_MIN));
#ifdef TGT_safe

    /* wait for safe main rst b */
    while (!(GET32_FLD(SAF_MAIN_RST_ADDR, 27, 1)));

#endif
#if defined(BOARD)
    enum sd_chipid_e chipid = get_part_id(&usr_id, PART_CHIPID);
    int part_ver = get_part_id(&usr_id, PART_REV);
    enum sd_board_type_e type = get_part_id(&usr_id, PART_BOARD_TYPE);

    if (chipid == CHIPID_UNKNOWN)
        part_ver = 1;

    board_setup(part_ver, type, get_part_id(&usr_id, PART_BOARD_ID_MAJ),
                get_part_id(&usr_id, PART_BOARD_ID_MIN));
    DBG("Board: %s\n", board_str);
#endif
#if defined(CFG_RUN_DDR_INIT_SEQ)

    if (is_str_enter())
        ddr_self_exit();
    else
        load_ddr_data();

#endif
#if defined(BOARD)
    board_setup_later(get_part_id(&usr_id, PART_REV), get_part_id(&usr_id,
                      PART_BOARD_TYPE), get_part_id(&usr_id, PART_BOARD_ID_MAJ),
                      get_part_id(&usr_id, PART_BOARD_ID_MIN));
    DBG("Board: %s\n", board_str);
#endif

    if (is_str_resume())
        load_partition_by_configs(str_get_pt_configs(), str_get_pt_configs_cnt());
    else
        load_partition_by_configs(pt_configs, PT_CONFIGS_CNT);

#if defined(IAR_DBG_ON)
    uint32_t rval;
    rval = readl(APB_SCR_SEC_BASE + (274 << 12));
    rval = (rval & (~0xF)) | 0xF;
    writel(rval, APB_SCR_SEC_BASE + (274 << 12));

    while (1);

#endif

#ifdef SUPPORT_VIRT_UART
    virt_uart_init(VIRT_UART_MEMBASE, VIRT_UART_MEMSIZE);
#endif

    load_fda_spl();
    load_safety_image();
#else
    DBG("dil2 reset to safety os base:%u\n", SAFETY_MEMBASE);
    uint8_t *entry = (uint8_t *)(addr_t)SAFETY_MEMBASE;
#if VERIFIED_BOOT
    bool ret;
    size_t entry_img_sz = SAFETY_MEMSIZE;
    void *vbmeta_buffer = (void *)(addr_t)VBMETA_MEMBASE;
    uint32_t vbmeta_size = VBMETA_MEMSIZE;
    static struct list_node verified_images_list = LIST_INITIAL_VALUE(
                verified_images_list);
    ret = add_verified_image_list(&verified_images_list, vbmeta_buffer,
                                  vbmeta_size, VBMETA_PARTITION_NAME);
    assert(ret);
    ret = add_verified_image_list(&verified_images_list, entry, entry_img_sz,
                                  "safety_os");
    assert(ret);

    if (!verify_loaded_images(NULL, &verified_images_list, NULL)) {
        WARN("%s %d verify images fail\n", __func__, __LINE__);
        return -1;
    }

#endif
    arch_disable_cache(ICACHE | DCACHE);
    reset_safety_cr5((addr_t)entry);
#endif
    //shell_loop();
    return 0;
}
