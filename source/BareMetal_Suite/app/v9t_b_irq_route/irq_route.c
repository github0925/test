/********************************************************
 *          Copyright(c) 2020   Semidrive               *
 ********************************************************/

#include <soc.h>
#include <helper.h>
#include <debug.h>

#define PCIE_MSI_ADDR 0x31000054U
#define PCIE_MSI_UPPER_ADDR 0x31000058U

#define INT_ID_ADDR  0x3841B000U    /* General register 7 */
#define msi_ready() (readl(INT_ID_ADDR) == 0xA5A5A5A5U)
#define set_int_id(id)  writel((id), INT_ID_ADDR)

void send_msi(uint32_t id)
{
    uint64_t v = readl(PCIE_MSI_ADDR);
    v |= (uint64_t)readl(PCIE_MSI_UPPER_ADDR) << 32;
    if(!v) {
        DBG("msi not ready!\n");
        /* Write AEOIR to finish this interrupt. */
        writel(id, GICC_BASE + 0x24U);
        return;
    }

    /* Polling until msi ready,
     * Linux on side A AP1 will set msi ready after MSI
     * handler completed.
     *
     * Both ready flag and pcie are in device-nGnRE memory,
     * and "side B wait msi ready -> side B set interrupt id ->
     * send msi -> side A handle msi -> side A set msi ready"
     * is always serialized, so no extra synchronization needed.
     */
    while (!msi_ready());
    set_int_id(id);

    uint32_t val = readl(0x3100005c);

    writel(val, 0x500000000);
}

void do_irq(void)
{
    uint32_t int_id;

    /* Read AIAR to acknowledge non-secure interrupts. */
    int_id = readl(GICC_BASE + 0x20U);
    int_id &= 0x1FFU;

    DBG("%s: int id = %d\n", __func__, int_id);

    send_msi(int_id);
}