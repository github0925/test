/********************************************************
 *          Copyright(c) 2020   Semidrive               *
 ********************************************************/

#include <common_hdr.h>
#include <soc.h>
#ifdef RUNNING_IN_MEMORY
#include "ddr_map.h"
#endif
#include "armv8_mmu.h"

extern void  arch_inv_cache_all(void);
#ifdef  RUNNING_IN_MEMORY
static uint64_t xlat_tbl_1[L1_TBL_ENTRY_NUM] __attribute__((aligned(L1_TBL_BASE_ALIGN)));
static uint64_t xlat_tbl_2[L2_TB_ENTRY_NUM*2] __attribute__((aligned(L2_TBL_BASE_ALIGN)));
static uint64_t xlat_tbl_3[L3_TB_ENTRY_NUM*2] __attribute__((aligned(L3_TBL_BASE_ALIGN)));
// extern uint64_t __stack_end;
#else
static uint64_t xlat_tbl_1[L1_TBL_ENTRY_NUM] __attribute__((aligned(L1_TBL_BASE_ALIGN)));
static uint64_t xlat_tbl_2[L2_TB_ENTRY_NUM] __attribute__((aligned(L2_TBL_BASE_ALIGN)));
static uint64_t xlat_tbl_3[L3_TB_ENTRY_NUM] __attribute__((aligned(L3_TBL_BASE_ALIGN)));
#endif
void init_xlat_table(void)
{
    uint64_t pa = 0ull;

    int s_idx = 0, e_idx = 0;
    /* 0 ~ 0x3fffffff */
    xlat_tbl_1[L1_IDX(0)] = TBL_ENTRY_TBL(&xlat_tbl_2[0], 0);
    {
        /* 0 ~ 0x1fffff: Normal */
        xlat_tbl_2[L2_IDX(0)] = TBL_ENTRY_TBL(&xlat_tbl_3[0], 0);
        {
            pa = IRAM2_BASE;
            s_idx = L3_IDX(IRAM2_BASE);
            e_idx = L3_IDX(0x200000ull - 1);

            for (int i = s_idx; i <= e_idx; i++) {
                xlat_tbl_3[i] = TBL_ENTRY_PAGE(pa,
                                               ATTR_ID(NORMAL_WB_WA) | ATTR_SH_OUTER | ATTR_nG | ATTR_AF | ATTR_AP_RW);
                pa += PAGE_SZ;
            }
        }
        /* 0x200000 ~ 0x3fffffff: Device */
        s_idx = L2_IDX(0x200000ull);
        e_idx = L2_IDX(0x40000000ull - 1);

        for (int i = s_idx; i <= e_idx; i++) {
            xlat_tbl_2[i] =  TBL_ENTRY_BLK(pa, ATTR_ID(DEVICE_nGnRnE) | ATTR_XN | ATTR_AF | ATTR_AP_RW);
            pa += L2_MAP_SZ_PER_ENTRY;
        }
    }
    /* The second 1G memory*/    
    #ifdef  RUNNING_IN_MEMORY
    /* 0x40000000 - 0x80000000 */
    xlat_tbl_1[L1_IDX(pa)] = TBL_ENTRY_TBL(&xlat_tbl_2[L2_ENTRY_ARRA_IDX(pa)], 0);
    {
        /* 0x40000000 - 0x40200000 */
        xlat_tbl_2[L2_ENTRY_ARRA_IDX(pa)] = TBL_ENTRY_TBL(&xlat_tbl_3[L3_TB_ENTRY_NUM+L3_IDX(pa)], 0);
        {
            
            s_idx = L3_TB_ENTRY_NUM + L3_IDX(pa);
            e_idx = L3_TB_ENTRY_NUM + L3_IDX(pa+L2_MAP_SZ_PER_ENTRY-1);
            for (int i = s_idx; i <= e_idx; i++) {
                /* 0x40000000 - 0x40040000: Can execution */
                if ((i-s_idx) < DDR_SQUEESER_IMAGE_SIZE/PAGE_SZ){
                    xlat_tbl_3[i] = TBL_ENTRY_PAGE(pa,
                                ATTR_ID(NORMAL_WB_WA) |ATTR_SH_OUTER| ATTR_nG | ATTR_AF | ATTR_AP_RO);
                }else{
                    /* 0x40040000 - 0x40200000: Normal */
                    xlat_tbl_3[i] = TBL_ENTRY_PAGE(pa,
                                ATTR_ID(NORMAL_WB_WA) |ATTR_SH_OUTER| ATTR_XN |ATTR_nG | ATTR_AF | ATTR_AP_RW);
                }
                pa += PAGE_SZ;
            }
        }
        /* 0x40200000 - 0x80000000ull: Normal */
        s_idx = L2_ENTRY_ARRA_IDX(pa);
        e_idx = L2_ENTRY_ARRA_IDX(0x80000000);

        for (int i = s_idx; i <= e_idx; i++) {
            if(pa>=(uint64_t)__stack_end){
                xlat_tbl_2[i] =  TBL_ENTRY_BLK(pa, ATTR_ID(DEVICE_nGnRnE) | ATTR_XN | ATTR_AF | ATTR_AP_RW);
            }else{
                xlat_tbl_2[i] =  TBL_ENTRY_BLK(pa, ATTR_ID(NORMAL_WB_WA) | ATTR_XN | ATTR_AF | ATTR_AP_RW);
            }
            pa += L2_MAP_SZ_PER_ENTRY;
        }

    }
    s_idx = L1_IDX(0x80000000ull);
    e_idx =  L1_IDX(0x800000000ull - 1);

    for (int i = s_idx; i <= e_idx; i++) {
        xlat_tbl_1[i] = TBL_ENTRY_BLK(pa,
                                      ATTR_ID(DEVICE_nGnRnE) | ATTR_XN | ATTR_AF | ATTR_AP_RW);
        pa += L1_MAP_SZ_PER_ENTRY;
    }
    #else
    /* 0x40000000 - 0x800000000: Normal */
    s_idx = L1_IDX(0x40000000ull);
    e_idx =  L1_IDX(0x800000000ull - 1);

    for (int i = s_idx; i <= e_idx; i++) {
        xlat_tbl_1[i] = TBL_ENTRY_BLK(pa,
                                      ATTR_ID(DEVICE_nGnRnE) | ATTR_XN | ATTR_AF | ATTR_AP_RW);
        pa += L1_MAP_SZ_PER_ENTRY;
    }
    #endif
}
#ifdef  RUNNING_IN_MEMORY  
uint64_t remmap_l2_l3_range(uint64_t start, uint64_t size, uint64_t attr)
{
    uint32_t s_idx, e_idx;
    uint64_t pa = start;
   
   
    s_idx = L3_TB_ENTRY_NUM + L3_IDX(pa);
    if( size >= L2_MAP_SZ_PER_ENTRY){
        e_idx = L3_TB_ENTRY_NUM + L3_IDX( DDR_MEMORY_BASE + L2_MAP_SZ_PER_ENTRY - 1);
    }else{
        e_idx = L3_TB_ENTRY_NUM + L3_IDX( pa + size - 1 );
    }

    for (int i = s_idx; i <= e_idx; i++) {
        /* 0x40060000 - 0x40200000: Normal */
        xlat_tbl_3[i] = TBL_ENTRY_PAGE(pa,attr);
        pa += PAGE_SZ;
    }
    /* 0x40200000 - 0x80000000ull: Normal */
    s_idx = L2_ENTRY_ARRA_IDX(pa);
    e_idx = L2_ENTRY_ARRA_IDX(0x80000000);

    for (int i = s_idx; i <= e_idx; i++) {
        xlat_tbl_2[i] =  TBL_ENTRY_BLK(pa, attr);
        pa += L2_MAP_SZ_PER_ENTRY;
    }
    return pa-L2_MAP_SZ_PER_ENTRY;
}
#endif

void mmap_level1_range(int level, uint64_t start, uint64_t size, uint64_t attr)
{
    uint32_t s_idx, e_idx;
    uint64_t pa = start;

    if (1 == level) {
        s_idx = L1_IDX(start);
        e_idx =  L1_IDX(start + size - 1);

        for (int i = s_idx; i <= e_idx; i++) {
            xlat_tbl_1[i] = TBL_ENTRY_BLK(pa, attr);
            pa += L1_MAP_SZ_PER_ENTRY;
        }
    }

    __asm__ volatile("tlbi alle3");
    __asm__ volatile("isb");
    __asm__ volatile("dsb sy");
}

void enable_mmu(void)
{
    arch_inv_cache_all();
    enable_mmu_el3(&xlat_tbl_1[0]);
}
