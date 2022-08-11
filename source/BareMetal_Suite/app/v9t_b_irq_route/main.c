/********************************************************
 *          Copyright(c) 2020   Semidrive               *
 ********************************************************/

#include <common_hdr.h>
#include <mini_libc/mini_libc.h>
#include <service.h>
#include <soc.h>
#include <arch.h>
#include <lib/mmio.h>
#include <armv8_mmu.h>
#include "rstgen/rstgen.h"
#include "uart/uart.h"
#include "shell/shell.h"
#if defined(BOARD)
#include "board.h"
#endif

#define el3_irq_enable()  do { \
                                uint32_t scr = 0U; \
                                __asm__ __volatile__("mrs %0, scr_el3 \n" \
                                                     "orr %0, %0, #2  \n" \
                                                     "msr scr_el3, %0 \n" \
                                                     "isb             \n" \
                                                     "msr daifclr, #2 \n" \
                                                     : "+r"(scr)); \
                            } while (0)

/* workaround: if .data section is empty, objcopy not work properly */
char *prod_str = "v9t_b_irq_route app";
const char *cpu_str = "AP2";
#if defined(BOARD_x9_ref)
const char *board_str = "BOARD_x9_ref";
#elif defined(BOARD_g9_ref)
const char *board_str = "BOARD_g9_ref";
#else
const char *board_str = "BOARD_not_specified";
#endif

module_e tty = TTY_UART;

/* Flat mapping */
static void mmu_config(void)
{
    /* Peripheral: 0~0x40000000 */
    mmap_level1_range(1, 0, 0x40000000,
                      ATTR_ID(DEVICE_nGnRnE) | ATTR_XN | ATTR_AF | ATTR_AP_RW);
    /* DDR: 0x40000000~0x80000000 */
    mmap_level1_range(1, 0x40000000, 0x40000000,
                      ATTR_ID(NORMAL_WB_WA) | ATTR_nG | ATTR_AF | ATTR_AP_RW);
    /* PCIE mapped IO: 0x500000000~0x700000000 */
    for (size_t i = 0x500000000; i < 0x700000000; i += 0x40000000) {
        mmap_level1_range(1, i, 0x40000000,
                          ATTR_ID(DEVICE_nGnRnE) | ATTR_XN | ATTR_AF | ATTR_AP_RW);
    }

    enable_mmu();
}

static void gic_assign_all_int_to_grp1(void)
{
    uint32_t int_num, i;

    int_num = readl(GICD_BASE + 4U);
    int_num &= 0x1FU;
    int_num = (int_num + 1U) * 32U;

    for (i = 0U; i < int_num; i += 32U) {
        uint32_t reg_idx = i / 32U * 4U;
        /* Clear all irq pending status. */
        writel(0xFFFFFFFFU, GICD_BASE + 0x280U + reg_idx);
        /* Disable all irq. */
        writel(0xFFFFFFFFU, GICD_BASE + 0x180U + reg_idx);
        /* Assign all irq to group 1. */
        writel(0xFFFFFFFFU, GICD_BASE + 0x80U + reg_idx);
    }
}

static void gic_enable_all_ints(void)
{
    /* Write PMR to the lowest priority in
     * secure mode. This will enable non-secure
     * mode writing to PMR with value corresponding
     * to lower half of priority range.
     */
    writel(0xFFU, GICC_BASE + 4U);
}

static void gic_pre_init(void)
{
    gic_assign_all_int_to_grp1();
    gic_enable_all_ints();
}

static void cpu2_reset(void)
{
    #define RESET_FLAG_ADDR 0x3841A000U /* General reg 6 */
    #define RESET_FLAG  (1U << 31)

    uint32_t reset_flag_val = readl(RESET_FLAG_ADDR);

    if (!(reset_flag_val & RESET_FLAG)) {
        reset_flag_val |= RESET_FLAG;
        /* Should not add dsb instruction,
         * otherwise CPU2 will hang, the reason is unknown.
         */
        writel(reset_flag_val, RESET_FLAG_ADDR);
        rg_core_reset(APB_RSTGEN_SEC_BASE, RSTGEN_SEC_CORE_RST_B_CPU2_INDEX);
        while (1) {
            __asm__ __volatile__("wfi");
        }
    }
}

int __main(int argc, char *argv[])
{
    /* Shuld be fixed:
     * On V9Ts, need to reset CPU2 before any memory
     * access, otherwise CPU2 will hang after
     * seemingly fixed number of HPI transaction.
     * The reason is unknown.
     */
    cpu2_reset();

    mmu_config();

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
#if defined(BOARD)
    board_setup(0, 0, 0, 0);
#endif

    gic_pre_init();
    el3_irq_enable();

    INFO("\n\n%s: %s, built on %s at %s\n\n", cpu_str, prod_str, __DATE__, __TIME__);
#if defined(BOARD)
    INFO("Board: %s\n", board_str);
#endif

    while(1) {
        __asm__ __volatile__("wfi");
    }

    return 0;
}
