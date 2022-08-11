/********************************************************
 *          Copyright(c) 2018   Semidrive               *
 *******************************************************/

#ifndef __FUSEMAP_H__
#define __FUSEMAP_H__

#if !defined(ASSEMBLY)
#include <soc_hal.h>

#include "fuse_hdr_gen.h"

#define ROT_HASH_SZ     32

#define FUSE_ROT_SEC_START_ID       (0x90/4)
#define FUSE_ROT_SAFETY_START_ID    (0xB0/4)
#define FUSE_ROT_SEMIDRIVE_START_ID (0x110/4)

#define ECC_FUSE_START  8
#define ECC_FUSE_END    167
#define ECC_VAL_START   196
#define RED_VAL_START1   188
#define RED_VAL_START2   236

U32 fuse_get_parity_type(U32 id);
U32 fuse_get_red_pos(U32 id);
U32 fuse_get_ecc_pos(U32 id, U32 *wd, U32 *shift);
U32 fuse_get_cores(void);
#else

#define FM_SEC_EARLY_FIREWALL   ((0x01 << 30) | (0x01 << 1))
#define FUSE_REG_OFF_ROM_CFG       (0x128c)
#define FUSE_REG_OFF_FA_CFG        (0x12A0)
#define FUSE_REG_OFF_MISC_CTL0     (0x12A4)

#endif
#endif  /* __FUSEMAP_H__ */
