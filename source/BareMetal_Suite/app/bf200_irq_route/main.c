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

#define INT_HIGH_LEVEL_SENSITIVE    1U

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

/*
 * Should be arranged in ascending order.
 */
static const uint32_t g_msi_to_irq_mapping[] = {
    [0] = 55U, /* LIN2 (uart8) */
    [1] = 72U, /* I2C9 */
    [2] = 73U, /* I2C10 */
    [3] = 74U, /* I2C11 */
    [4] = 75U, /* I2C12 */
    [5] = 76U, /* I2C13 */
    [6] = 104U, /* USB1 */
    [7] = 105U, /* USB1 wakeup */
    [8] = 106U, /* USB1 chgdet */
    [9] = 110U, /* PCIe */
    [10] = 134U, /* eth1 */
    [11] = 137U, /* eth2 */
    [12] = 142U, /* MSHC1 */
    [13] = 144U, /* MSHC2 */
    [14] = 146U, /* MSHC3 */
    [15] = 148U, /* MSHC4 */
    [16] = 150U, /* CANFD1 */
    [17] = 151U, /* CANFD2 */
    [18] = 152U, /* CANFD3 */
    [19] = 153U, /* CANFD4 */
    [20] = 159U, /* DC1 */
    [21] = 173U, /* CSI */
    [22] = 174U, /* CSI */
    [23] = 175U, /* CSI */
    [24] = 176U, /* CSI */
    [25] = 177U, /* CSI */
    [26] = 181U, /* GPU 9446 */
    [27] = 190U, /* GPU 9226 */
    [28] = 199U, /* VPU1 */
    [29] = 200U, /* VPU2 */
    [30] = 225U /* Timer1 */
};

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

static void gic_enable_grp1_dist(void)
{
    writel(0x02U, GICD_BASE);
}

static void gic_enable_grp1_cpuif(void)
{
    writel(0x02U, GICC_BASE);
}

static void gic_unmask_all_pri(void)
{
    /* Write PMR to the lowest priority. */
    writel(0xFFU, GICC_BASE + 4U);
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

static void gic_set_pri(uint32_t int_id, uint8_t pri)
{
    uint32_t reg_base = GICD_BASE + 0x400U;
    uint32_t reg_num = int_id / 4U;
    uint32_t bit_off = (int_id % 4U) * 8U;
    RMWREG32(reg_base + reg_num * 4U, bit_off, 8U, pri);
}

static void gic_set_int_trigger_type(uint32_t int_id, uint8_t type)
{
    uint32_t reg_base = GICD_BASE + 0xC00U;
    uint32_t reg_num = int_id / 16U;
    uint32_t bit_off = (int_id % 16U) * 2U;
    RMWREG32(reg_base + reg_num * 4U, bit_off, 2U, type);
}

static void gic_enable_ints(uint32_t int_id)
{
    uint32_t reg_base = GICD_BASE + 0x100U;
    uint32_t reg_num = int_id / 32U;
    uint32_t bit_num = int_id % 32U;
    RMWREG32(reg_base + reg_num * 4U, bit_num, 1U, 1U);
}

static void gic_init(void)
{
    gic_assign_all_int_to_grp1();
    gic_unmask_all_pri();

    for (size_t i = 0;
        i < sizeof(g_msi_to_irq_mapping) / sizeof(g_msi_to_irq_mapping[0]);
        i++) {
        uint32_t int_id = g_msi_to_irq_mapping[i];
        gic_set_pri(int_id, 0xD0U);
        gic_set_int_trigger_type(int_id, INT_HIGH_LEVEL_SENSITIVE);
        gic_enable_ints(int_id);
    }

    gic_enable_grp1_dist();
    gic_enable_grp1_cpuif();
}

int __main(int argc, char *argv[])
{
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
    board_setup();
#endif

    gic_init();
    el3_irq_enable();

    INFO("\n\n%s: %s, built on %s at %s\n\n", cpu_str, prod_str, __DATE__, __TIME__);
#if defined(BOARD)
    INFO("Board: %s\n", board_str);
#endif

    while(1);

    return 0;
}

int route_int_num(void)
{
    return sizeof(g_msi_to_irq_mapping) / sizeof(g_msi_to_irq_mapping[0]);
}

const uint32_t *msi_to_irq_map(void)
{
    return g_msi_to_irq_mapping;
}
