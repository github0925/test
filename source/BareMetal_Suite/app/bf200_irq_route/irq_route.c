/********************************************************
 *          Copyright(c) 2020   Semidrive               *
 ********************************************************/

#include <soc.h>
#include <helper.h>
#include <debug.h>

#define MSI_ID_ADDR  0x3841A000U
#define msi_id_ready() (readl(MSI_ID_ADDR) == ~0U)
#define msi_set_id(id)    writel((id), MSI_ID_ADDR)

extern int route_int_num(void);
extern const uint32_t *msi_to_irq_map(void);

static int irq_to_msi(uint32_t irq_nr)
{
    int start = 0;
    int end = route_int_num() - 1;
    int mid;
    const uint32_t *map = msi_to_irq_map();

    do {
        mid = (start + end) / 2;

        if (map[mid] == irq_nr) {
            return mid;
        }
        else if (irq_nr < map[mid]) {
            end = mid - 1;
        }
        else {
            start = mid + 1;
        }
    } while (start <= end);

    return -1;
}

void send_msi(uint32_t id)
{
    uint64_t v = readl(0x31000054);
    v |= (uint64_t)readl(0x31000058) << 32;
    if(!v) {
        DBG("msi not ready!\n");

        const uint32_t *map = msi_to_irq_map();
        /* Write AEOIR to finish this interrupt. */
        writel(map[id], GICC_BASE + 0x24U);
        return;
    }

    while (!msi_id_ready());
    msi_set_id(id);

    uint32_t val = readl(0x3100005c);

    writel(val + id, 0x500000000);
}

void do_irq(void)
{
    int msi_id;
    uint32_t int_id;

    /* Read AIAR to acknowledge non-secure interrupts. */
    int_id = readl(GICC_BASE + 0x20U);
    int_id &= 0x1FFU;

    DBG("%s: int id = %d\n", __func__, int_id);

    msi_id = irq_to_msi(int_id);
    DBG("%s: msi id = %d\n", __func__, msi_id);
    if (msi_id < 0) {
        /* Write AEOIR to finish this invalid interrupt. */
        writel(int_id, GICC_BASE + 0x24U);
        return;
    }

    send_msi(msi_id);
}