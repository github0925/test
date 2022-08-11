/********************************************************
 *          Copyright(c) 2018   Semidrive               *
 *******************************************************/

/**
 * @file    mpu.h
 * @brief   header file of mpu routines
 */

#include <common_hdr.h>
#include <arch.h>
#include "mpu.h"

//#define MPU_DEBUG
#ifdef MPU_DEBUG
#define MPU_DBG(fmt, args...)   DBG(fmt, ##args)
#else
#define MPU_DBG(fmt, args...)
#endif

void mpu_enable(void)
{
    U32 reg = arch_rd_sctlr();

    reg |= BM_SCTLR_M;
    dsb();
    arch_wr_sctlr(reg);
    dsb();
    isb();
}

void mpu_disable(void)
{
    U32 reg = arch_rd_sctlr();

    reg &= ~BM_SCTLR_M;
    dsb();
    arch_wr_sctlr(reg);
    dsb();
    isb();
}

BOOL mpu_is_enabled(void)
{
    U32 reg = arch_rd_sctlr();

    return (reg & BM_SCTLR_M) ? TRUE : FALSE;
}

int32_t mpu_get_empty_rgn_id(void)
{
    int i = 0;

    for (; i < 16; i++) {
        arch_wr_mpurgnr(i);
        U32 v = arch_rd_mpurser();

        if (!(v & BM_MPU_RGN_EN)) {
            break;
        }
    }

    if (i == 16) {
        return -1;
    } else {
        DBG("%s: free mpu rgn found: rng_%d\n", __FUNCTION__, i);
        return i;
    }
}

U32 mpu_region_cfg(U32 id, const mpu_region_desc_t *rgn)
{
    U32 res = -1U;
    U32 cnt = 0;

    if ((NULL != rgn)
        && (id < 16)) {
        U32 base = rgn->base;
        U32 size = rgn->size;
        cnt++;
        U32 attr = rgn->attribute;
        MPU_DBG("base:size:attr:0x%x 0x%x 0x%x\n", base, size, attr);

        if ((size >= MPU_RGN_32B) && (size <= MPU_RGN_4G)) {
            /* base shall size aligned */
            if (0 == (base & (0xFFFFFFFFU >> (32 - (size + 1))))) {
                arch_wr_mpurgnr(id);
                arch_wr_mpurbar(base & FM_MPU_RGN_BASE);
                cnt++;
                arch_wr_mpurser((size << FS_MPU_RGN_SZ) | BM_MPU_RGN_EN);
                arch_wr_mpuracr(attr);
                cnt++;

                MPU_DBG("MPU rgn %d dump: rgnr:0x%x, rbar:0x%x, rser:0x%x, racr:0x%x\n",
                        id, arch_rd_mpurgnr(), arch_rd_mpurbar(),
                        arch_rd_mpurser(), arch_rd_mpuracr());

                res = 0;
            } else {
                DBG("%s: Opps with id=%d, base not aligned to size\n",
                    __FUNCTION__, id);
                cnt = 0;
            }
        } else {
            DBG("%s: Opps with id=%d, invalid size %d\n",
                __FUNCTION__, id, size);
            cnt = 0;
        }
    }

    if (0 == res) {
        assert(3 == cnt);
    }

    return res;
}

U32 mpu_setup_regions(const mpu_region_desc_t *rgn_list)
{
    U32 res = -1U;
    U32 cnt = 0;

    do {
        if (NULL != rgn_list) {
            const mpu_region_desc_t *rgn = rgn_list;
            U32 err = 0;

            for (U32 id = 0; rgn->size != 0; rgn++, id++) {
                cnt++;

                if (0 != mpu_region_cfg(id, rgn)) {
                    err |= (0x01u << id);
                    break;
                }
            }

            rgn = rgn_list;

            for (; rgn->size != 0; rgn++) {
                cnt--;
            }

            if (0 == err) {
                res = 0;
            }
        }
    } while (0);

    if (0 == res) {
        assert(0 == cnt);
    }

    return res;
}

#if defined(DEBUG_ENABLE)
void mpu_dump(U32 num)
{
    DBG("MPU dumping...\n");

    for (U32 i = 0; (i < num) && (i < 16); i++) {
        arch_wr_mpurgnr(i);
        U32 mpurbar = arch_rd_mpurbar();
        U32 mpurser = arch_rd_mpurser();
        U32 mpuracr = arch_rd_mpuracr();
        DBG(" region%d: mpurbar:0x%x, mpurser:0x%x, mpuracr:0x%x\n",
            i, mpurbar, mpurser, mpuracr);
    }
}
#endif
