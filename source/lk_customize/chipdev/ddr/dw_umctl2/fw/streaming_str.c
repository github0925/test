/********************************************************
 *	        Copyright(c) 2019	Semidrive 		        *
 *******************************************************/

#include <stdint.h>
#include <dw_umctl2.h>
#include <ddr_init_helper.h>

#if LPDDR4X || CFG_DDR_TRAINING_4_ALL
extern train_string_t lpddr4x_pmu_train_1d_string[];
extern train_string_t lpddr4x_pmu_train_2d_string[];
#endif
#if LPDDR4 || CFG_DDR_TRAINING_4_ALL
extern train_string_t lpddr4_pmu_train_1d_string[];
extern train_string_t lpddr4_pmu_train_2d_string[];
#endif
#if DDR4 || CFG_DDR_TRAINING_4_ALL
extern train_string_t ddr4_pmu_train_1d_string[];
extern train_string_t ddr4_pmu_train_2d_string[];
#endif
#if DDR3 || CFG_DDR_TRAINING_4_ALL
extern train_string_t ddr3_pmu_train_1d_string[];
#endif

const char *find_streaming_string(uint32_t fw, uint32_t id)
{
    const train_string_t *p =
#if LPDDR4X || CFG_DDR_TRAINING_4_ALL
                    (fw == DDRPHY_FW_LPDDR4X_1D) ? &lpddr4x_pmu_train_1d_string[0] :
                    (fw == DDRPHY_FW_LPDDR4X_2D) ? &lpddr4x_pmu_train_2d_string[0] :
#endif
#if LPDDR4 || CFG_DDR_TRAINING_4_ALL
                    (fw == DDRPHY_FW_LPDDR4_1D) ? &lpddr4_pmu_train_1d_string[0] :
                    (fw == DDRPHY_FW_LPDDR4_2D) ? &lpddr4_pmu_train_2d_string[0] :
#endif
#if DDR4 || CFG_DDR_TRAINING_4_ALL
                    (fw == DDRPHY_FW_DDR4_1D) ? &ddr4_pmu_train_1d_string[0] :
                    (fw == DDRPHY_FW_DDR4_2D) ? &ddr4_pmu_train_2d_string[0] :
#endif
#if DDR3 || CFG_DDR_TRAINING_4_ALL
                    (fw == DDRPHY_FW_DDR3_1D) ? &ddr3_pmu_train_1d_string[0] :
#endif
                    NULL;
    const char *str = NULL;

    if (NULL != p) {
        for(; p->id != 0; p++) {
            if (p->id == id) {
                str = p->str;
                break;
            }
        }
    }

    return str;
}
