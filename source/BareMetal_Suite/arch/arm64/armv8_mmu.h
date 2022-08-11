/********************************************************
 *          Copyright(c) 2020   Semidrive               *
 ********************************************************/

#ifndef __ARMV8_MMU_H_
#define __ARMV8_MMU_H_

#define TCR_TBI        (0x0 << 20)    /* Top Byte Ignored */
#define TCR_PS_4GB     (0x0 << 16)    /* 32 bits */
#define TCR_PS_64GB    (0x1 << 16)    /* 36 bits */
#define TCR_PS_1TB     (0x2 << 16)
#define TCR_TG0_4KB    (0x0 << 14)
#define TCR_TG0_64KB   (0x01 << 14)
#define TCR_TG0_16KB   (0x02 << 14)
#define TCR_SH0_NON    (0x0 << 12)
#define TCR_SH0_OUTER  (0x2 << 12)
#define TCR_SH0_INNER  (0x3 << 12)
#define TCR_ORGN0_OUTER_NC  (0x0 << 10)
#define TCR_ORGN0_OUTER_WB_WA (0x01 << 10)
#define TCR_ORGN0_OUTER_WT_NOWA (0x02 << 10)
#define TCR_ORGN0_OUTER_WB_NOWA (0x03 << 10)
#define TCR_IRGN0_INNER_NC  (0x0 << 8)
#define TCR_IRGN0_INNER_WB_WA (0x01 << 8)
#define TCR_IRGN0_INNER_WT_NOWA (0x02 << 8)
#define TCR_IRGN0_INNER_WB_NOWA (0x03 << 8)
#define TCR_T0SZ(x)    ((x) << 0)

/*
    Attr<n>[7:4] Meaning
    0000 Device memory. See encoding of Attr<n>[3:0] for the type of Device memory.
    00RW, RW not 00 Normal memory, Outer Write-Through Transient
    0100 Normal memory, Outer Non-cacheable
    01RW, RW not 00 Normal memory, Outer Write-Back Transient
    10RW Normal memory, Outer Write-Through Non-transient
    11RW Normal memory, Outer Write-Back Non-transient
*/
#define OUTER_WT_NOWA       (2 << 4)
#define OUTER_WT_WA         (3 << 4)
#define OUTER_NC            (4 << 4)
#define OUTER_WB_NOWA       (6 << 4)
#define OUTER_WB_WA         (7 << 4)
#define OUTER_WT_NOWA_NT    (10 << 4)
#define OUTER_WT_WA_NT      (11 << 4)
#define OUTER_WB_NOWA_NT    (14 << 4)
#define OUTER_WB_WA_NT      (15 << 4)

/*
    Attr<n>[3:0] Meaning when Attr<n>[7:4] is 0000 Meaning when Attr<n>[7:4] is not 0000
    0000 Device-nGnRnE memory UNPREDICTABLE
    00RW, RW not 00 UNPREDICTABLE Normal memory, Inner Write-Through Transient
    0100 Device-nGnRE memory Normal memory, Inner Non-cacheable
    01RW, RW not 00 UNPREDICTABLE Normal memory, Inner Write-Back Transient
    1000 Device-nGRE memory Normal memory, Inner Write-Through Non-transient (RW=00)
    10RW, RW not 00 UNPREDICTABLE Normal memory, Inner Write-Through Non-transient
    1100 Device-GRE memory Normal memory, Inner Write-Back Non-transient (RW=00)
    11RW, RW not 00 UNPREDICTABLE Normal memory, Inner Write-Back Non-transient
 */
#define DEVICE_nGnRnE   0
#define DEVICE_nGnRE    4
#define DEVICE_nGRE     8
#define DEVICE_GRE      12

#define INNER_WT_NOWA       2
#define INNER_WT_WA         3
#define INNER_NC            4
#define INNER_WB_NOWA       6
#define INNER_WB_WA         7
#define INNER_WT_NOWA_NT    10
#define INNER_WT_WA_NT      11
#define INNER_WB_NOWA_NT    14
#define INNER_WB_WA_NT      15

#define MAIR_ATTR0          (DEVICE_nGnRnE)
#define MAIR_ATTR1          ((OUTER_WB_WA_NT | INNER_WB_WA_NT) << 8)
#define MAIR_ATTR2          ((OUTER_WB_NOWA_NT | INNER_WB_NOWA_NT) << 16)
#define MAIR_ATTR3          ((OUTER_WT_NOWA_NT| INNER_WT_NOWA_NT) << 24)

#define MMU_MAIR_VAL        (MAIR_ATTR0 | MAIR_ATTR1 | MAIR_ATTR2)
/*Bit31 and Bit23 shall be RES1*/
#define MMU_TCR_VAL   ( (0x01u << 31) | (0x01u << 23)\
                        | TCR_PS_64GB | TCR_TG0_4KB \
                        | TCR_SH0_OUTER | TCR_ORGN0_OUTER_WB_WA\
                        | TCR_IRGN0_INNER_WB_WA | TCR_T0SZ(64 - 36))
#define DEVICE_nGnRnE       0
/* The Cortex-A55 core simplifies the coherency logic by downgrading some memory types.
•*  # Memory that is marked as both Inner Write-Back Cacheable and Outer Write-Back Cacheable
 *    is cached in the L1 data cache and the L2 cache.
•*  # All other memory types are Non-cacheable.
 */
#define NORMAL_WB_WA        1
#define NORMAL_WB_NOWA      2
#define NORMAL_WT           3

/* MMU xlat table descripter definitions */
#define MTD_INVALID 0ull
#define MTD_BLK     (0x01ull << 0)
#define MTD_TBL     (0x03ull << 0)
#define MTD_PAGE    (0x03ull << 0)

#define MTD_NSTBL   (0x01ull << 63)
#define MTD_XNTBL   (0x01ull << 60)
#define MTD_PXNTBL  (0x01ull << 59)

#define MTD_PAGE     (0x03ull << 0)

/* Attribute fields in stage 1 VMSAv8-64 Block and Page descriptors */

#define ATTR_XN (0x01ull << 54)
#define ATTR_PXN (0x01ull << 53)    /* Privileged execute-never bit */
/* A hint bit indicating that the translation table entry is one of a contiguous set or entries*/
#define ATTR_CONT   (0x01ull << 52)
#define ATTR_DBM    (0x01ull << 51) /* Dirty Bit Modifier */
#define ATTR_nG     (0x01ull << 11) /* The not global bit */
#define ATTR_AF         (0x01ull << 10) /* The Access flag */
#define ATTR_SH_NON     (0x00ull << 8)  /* Non-shareable */
#define ATTR_SH_OUTER   (0x02ull << 8)  /* Outer shareable */
#define ATTR_SH_INNER   (0x03ull << 8)  /* Inner shareable */

/*
 * The ARMv8 translation table descriptor format defines AP[2:1] as the Access Permissions bits, and
 * does not define an AP[0] bit.
 * AP[1] is valid only for a stage 1 translation that supports two VA ranges.
 */
#define ATTR_AP_RW      (0x0ull << 6)
#define ATTR_AP_RO      (0x2ull << 6)
#define ATTR_NS         (0x01ull << 5)  /* Non-secure bit */

#define ATTR_ID(x)   ((x) << 2)      /* Stage 1 memory attributes index field, for the MAIR_ELx */
#define OA(a)   (((uint64_t)(a) & 0x0000fffffffff000ull))
/* Next level table */
#define NL_TBL(a)    (((uint64_t)(a) & 0x0000fffffffff000ull))

/* In this implementation,
 *      #1. Only works on EL3.
 *      #2. TCR_ELx.IPS will be set as 001 thus output address (PA) size is 36 bits.
 *          TCR_EL3.PS = 001 (Physical Address Size set as 36 bits also).
 *      #3. Translation granule will be set as 4KB (TCR_EL3.TG0 = 0), TCR_EL3.T0SZ == 28 (64-28=36 bits).
 * Thus there are 3 level translations and start from level3. For a page translation, the scheme
 * looks as below:
 *      IA[35:29]   Level3 Table        6 bits      64 entries      1GB per entry
 *      IA[28:21]   Level2 Table        9 bits      512 entries
 *      IA[20:12]   Level3 (Page)       9 bits      512 entries
 *      IA[11:0]    PA
 */

/* The base address of translation table (ttbrx) has to be page aligneded. For 4KB translation granule,
 * it has to be 4KB aligneded.
 */
#define TBL_ENTRY_BLK(pa, attr)  (OA(pa) | (attr) | MTD_BLK)
#define TBL_ENTRY_PAGE(pa, attr)  (OA(pa) | (attr) | MTD_PAGE)
#define TBL_ENTRY_TBL(pa, attr)  (NL_TBL(pa) | (attr) | MTD_TBL)

#define L0_MAP_SZ_PER_ENTRY     (0x01ull << 39)
#define L1_MAP_SZ_PER_ENTRY     0x40000000ull
#define L2_MAP_SZ_PER_ENTRY     0x200000
#define PAGE_SZ                 0x1000

#define L0_TBL_ENTRY_NUM    2
#define L1_TBL_ENTRY_NUM    64
#define L1_TBL_BASE_ALIGN   0x1000
#define L2_TB_ENTRY_NUM     512
#define L2_TBL_BASE_ALIGN   0x1000
#define L3_TB_ENTRY_NUM     512
#define L3_TBL_BASE_ALIGN   0x1000

#define L1_IDX(a)   (((a) & (L0_MAP_SZ_PER_ENTRY - 1)) / L1_MAP_SZ_PER_ENTRY)
#define L2_IDX(a)   (((a) & (L1_MAP_SZ_PER_ENTRY - 1)) / L2_MAP_SZ_PER_ENTRY)
#define L3_IDX(a)   (((a) & (L2_MAP_SZ_PER_ENTRY - 1)) / PAGE_SZ)
/* */
#define L2_ENTRY_ARRA_IDX(a)      ((((a)/L1_MAP_SZ_PER_ENTRY)*L2_TB_ENTRY_NUM)+L2_IDX(a))

#if !defined(ASSEMBLY)
void enable_mmu_el3(uint64_t *xlat_tbl);
void mmap_level1_range(int level, uint64_t start, uint64_t size, uint64_t attr);
#ifdef  RUNNING_IN_MEMORY  
uint64_t remmap_l2_l3_range(uint64_t start, uint64_t size, uint64_t attr);
#endif
#endif

#endif  /* ifndef (_ARMV8_MMU_H_) */
