/********************************************************
 *          Copyright(c) 2018   Semidrive               *
 *******************************************************/

#ifndef __MPU_H__
#define __MPU_H__

#define MPU_RGN_32B     4UL
#define MPU_RGN_64B     5UL
#define MPU_RGN_128B    6UL
#define MPU_RGN_256B    7UL
#define MPU_RGN_512B    8UL
#define MPU_RGN_1K      9UL
#define MPU_RGN_2K      10UL
#define MPU_RGN_4K      11UL
#define MPU_RGN_8K      12UL
#define MPU_RGN_16K     13UL
#define MPU_RGN_32K     14UL
#define MPU_RGN_64K     15UL
#define MPU_RGN_128K    16UL
#define MPU_RGN_256K    17UL
#define MPU_RGN_512K    18UL
#define MPU_RGN_1M      19UL
#define MPU_RGN_2M      20UL
#define MPU_RGN_4M      21UL
#define MPU_RGN_8M      22UL
#define MPU_RGN_16M     23UL
#define MPU_RGN_32M     24UL
#define MPU_RGN_64M     25UL
#define MPU_RGN_128M    26UL
#define MPU_RGN_256M    27UL
#define MPU_RGN_512M    28UL
#define MPU_RGN_1G      29UL
#define MPU_RGN_2G      30UL
#define MPU_RGN_4G      31UL

#define BM_MPU_RGN_EN      0x01UL
#define BM_MPU_XN          (0x01UL << 12)
#define FS_MPU_RGN_SZ       1UL
#define FS_MPU_RGN_BASE     5UL
#define FM_MPU_RGN_BASE     (0xFFFFFFFFU << FS_MPU_RGN_BASE)

#define BM_MPU_SHARED       (0x01UL << 2)

#define MPU_NO_ACCESS           (0UL << 8)
#define MPU_PRIV_FULL_ACCESS    (1UL << 8)
#define MPU_FULL_ACCESS         (3UL << 8)
#define MPU_PRIV_READ_ONLY      (5UL << 8)
/* TEX[2:0]     C           B */
#define MPU_ATTR_STRONG_ORDER   ((0UL << 3) | (0UL << 1) | 0UL)
#define MPU_ATTR_nSHARED_DEVICE ((2UL << 3) | (0UL << 1) | 0UL)
#define MPU_ATTR_WRT_NWA        ((0UL << 3) | (1UL << 1) | 0UL)
#define MPU_ATTR_WB_NWA         ((0UL << 3) | (1UL << 1) | 1UL)
#define MPU_ATTR_WBWA           ((1UL << 3) | (1UL << 1) | 1UL)

typedef struct {
    U32 base;
    U32 size;
    U32 attribute;
} mpu_region_desc_t;

void mpu_enable(void);
void mpu_disable(void);
BOOL mpu_is_enabled(void);
U32 mpu_region_cfg(U32 id, const mpu_region_desc_t *);
U32 mpu_setup_regions(const mpu_region_desc_t *rgn_list);
void mpu_dump(U32);
int32_t mpu_get_empty_rgn_id(void);

#endif
