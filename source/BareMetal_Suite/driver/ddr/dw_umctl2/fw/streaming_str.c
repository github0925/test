/********************************************************
 *          Copyright(c) 2019   Semidrive               *
 *******************************************************/

#include <stdint.h>
#include <dw_umctl2.h>
#include <ddr_init_helper.h>
#include <partition_parser.h>

extern train_string_t lpddr4x_pmu_train_1d_string[];
extern train_string_t lpddr4x_pmu_train_2d_string[];
extern train_string_t lpddr4_pmu_train_1d_string[];
extern train_string_t lpddr4_pmu_train_2d_string[];
extern train_string_t ddr4_pmu_train_1d_string[];
extern train_string_t ddr4_pmu_train_2d_string[];
extern train_string_t ddr3_pmu_train_1d_string[];

const char *find_streaming_string(uint32_t fw, uint32_t id)
{
#if defined(CFG_DDR_FW_IN_FLASH)

#define DDR_FW_STRING_LD_ADDR   0x4C0000u

    static uint32_t str_rdy = 0;
    train_string_t *p = NULL;

    if (0 == (str_rdy & (0x01u << fw))) {
        const uint8_t * base = NULL;
        /* train_string_t array */
        const char *ts_name = (fw == DDRPHY_FW_LPDDR4X_1D) ? "lpddr4x_pmu_train_1d_string.bin.rodata" :
                              (fw == DDRPHY_FW_LPDDR4X_2D) ? "lpddr4x_pmu_train_2d_string.bin.rodata" :
                              (fw == DDRPHY_FW_LPDDR4_1D) ? "lpddr4_pmu_train_1d_string.bin.rodata" :
                              (fw == DDRPHY_FW_LPDDR4_2D) ? "lpddr4_pmu_train_2d_string.bin.rodata" :
                              (fw == DDRPHY_FW_DDR4_1D) ? "ddr4_pmu_train_1d_string.bin.rodata" :
                              (fw == DDRPHY_FW_DDR4_2D) ? "ddr4_pmu_train_2d_string.bin.rodata" :
                              (fw == DDRPHY_FW_DDR3_1D) ? "ddr3_pmu_train_1d_string.bin.rodata" :
                              NULL;
        const char *str_name = (fw == DDRPHY_FW_LPDDR4X_1D) ? "lpddr4x_pmu_train_1d_string.bin.string" :
                               (fw == DDRPHY_FW_LPDDR4X_2D) ? "lpddr4x_pmu_train_2d_string.bin.string" :
                               (fw == DDRPHY_FW_LPDDR4_1D) ? "lpddr4_pmu_train_1d_string.bin.string" :
                               (fw == DDRPHY_FW_LPDDR4_2D) ? "lpddr4_pmu_train_2d_string.bin.string" :
                               (fw == DDRPHY_FW_DDR4_1D) ? "ddr4_pmu_train_1d_string.bin.string" :
                               (fw == DDRPHY_FW_DDR4_2D) ? "ddr4_pmu_train_2d_string.bin.string" :
                               (fw == DDRPHY_FW_DDR3_1D) ? "ddr3_pmu_train_1d_string.bin.string" :
                               NULL;

        void *buf = (void *) DDR_FW_STRING_LD_ADDR;
        void *text = NULL;
        base = get_partition_addr_by_name("ddr_fw");
        if (NULL != base ) {
            hdr_entry_t *hdr = (hdr_entry_t *)base;

            for (int i = 0; i < DDR_FW_ENTRIES_CNT_MAX; i++, hdr++) {
                if (0 == mini_strncmp_s(ts_name, hdr->name, sizeof(hdr->name))) {
                    mini_memcpy_s(buf, (void *)(base + hdr->offset), ROUNDUP(hdr->size, 16));
                    p = (train_string_t *)buf;
                    buf += ROUNDUP(hdr->size, 16);
                    break;
                }
            }

            hdr = (hdr_entry_t *)base;

            for (int i = 0; i < DDR_FW_ENTRIES_CNT_MAX; i++, hdr++) {
                if (0 == mini_strncmp_s(str_name, hdr->name, sizeof(hdr->name))) {
                    mini_memcpy_s(buf, (void *)(base + hdr->offset), ROUNDUP(hdr->size, 16));
                    text = buf;
                    buf += ROUNDUP(hdr->size, 16);
                    break;
                }
            }
        }else{
            uint64_t entry_sz;
            if (!load_ddr_training_fw_piecewise(ts_name, "ddr_fw", buf, &entry_sz, false))
            {
                p = (train_string_t *)buf;
                buf += entry_sz;
            }

            if (!load_ddr_training_fw_piecewise(str_name, "ddr_fw", buf, &entry_sz, false))
            {
                text = buf;
                buf += entry_sz;
            }
        }

        if (NULL == p || NULL == text) {
            return NULL;
        }

        for (; p->id != 0; p++) {
            p->str += (uint32_t)text;
        }

        p = (train_string_t *)DDR_FW_STRING_LD_ADDR;

        str_rdy |= (0x01u << fw);
    } else {
        p = (train_string_t *)DDR_FW_STRING_LD_ADDR;
    }

#else
    const train_string_t *p = (fw == DDRPHY_FW_LPDDR4X_1D) ? &lpddr4x_pmu_train_1d_string[0] :
                              (fw == DDRPHY_FW_LPDDR4X_2D) ? &lpddr4x_pmu_train_2d_string[0] :
                              (fw == DDRPHY_FW_LPDDR4_1D) ? &lpddr4_pmu_train_1d_string[0] :
                              (fw == DDRPHY_FW_LPDDR4_2D) ? &lpddr4_pmu_train_2d_string[0] :
                              (fw == DDRPHY_FW_DDR4_1D) ? &ddr4_pmu_train_1d_string[0] :
                              (fw == DDRPHY_FW_DDR4_2D) ? &ddr4_pmu_train_2d_string[0] :
                              (fw == DDRPHY_FW_DDR3_1D) ? &ddr3_pmu_train_1d_string[0] :
                              NULL;
#endif
    const char *str = NULL;

    if (NULL != p) {
        for (; p->id != 0; p++) {
            if (p->id == id) {
                str = p->str;
                break;
            }
        }
    }

    return str;
}
