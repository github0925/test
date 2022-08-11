/*
 * Copyright (c) 2008 Travis Geiselbrecht
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#pragma once

#define SHIFT_4K        (12)
#define SHIFT_16K       (14)
#define SHIFT_64K       (16)

/* arm specific stuff */
#ifdef ARM64_LARGE_PAGESIZE_64K
#define PAGE_SIZE_SHIFT (SHIFT_64K)
#elif ARM64_LARGE_PAGESIZE_16K
#define PAGE_SIZE_SHIFT (SHIFT_16K)
#else
#define PAGE_SIZE_SHIFT (SHIFT_4K)
#endif
#define USER_PAGE_SIZE_SHIFT SHIFT_4K

#define PAGE_SIZE (1UL << PAGE_SIZE_SHIFT)
#define USER_PAGE_SIZE (1UL << USER_PAGE_SIZE_SHIFT)

#if ARM64_CPU_CORTEX_A53 || ARM64_CPU_CORTEX_A57 || ARM64_CPU_CORTEX_A72 || ARM64_CPU_CORTEX_A55
#define CACHE_LINE 64
#else
#define CACHE_LINE 32
#endif

/* SPSR definitions for secure el1 jump to el3*/
#define U(_x)    (_x)
#define BIT_64(nr)    (U(1) << (nr))

#define SPSR_FIQ_BIT    (U(1) << 0)
#define SPSR_IRQ_BIT    (U(1) << 1)
#define SPSR_ABT_BIT    (U(1) << 2)
#define SPSR_AIF_SHIFT    U(6)
#define SPSR_AIF_MASK    U(0x7)

#define SPSR_E_SHIFT    U(9)
#define SPSR_E_MASK    U(0x1)
#define SPSR_E_LITTLE    U(0)
#define SPSR_E_BIG    U(1)

#define SPSR_T_SHIFT    U(5)
#define SPSR_T_MASK    U(0x1)
#define SPSR_T_ARM    U(0)
#define SPSR_T_THUMB    U(1)

#define SPSR_MODE_SHIFT    U(0)
#define SPSR_MODE_MASK    U(0x7)

#define SPSR_SSBS_BIT    BIT_32(23)

#define DISABLE_ALL_EXCEPTIONS \
        (SPSR_FIQ_BIT | SPSR_IRQ_BIT | SPSR_ABT_BIT)

#define MODE_SP_SHIFT    U(0x0)
#define MODE_SP_MASK    U(0x1)

#define MODE_RW_SHIFT    U(0x4)
#define MODE_RW_MASK    U(0x1)

#define MODE_EL_SHIFT    U(0x2)
#define MODE_EL_MASK    U(0x3)

#define MODE_SP_ELX    U(0x1)

#define MODE_RW_SHIFT    U(0x4)
#define MODE_RW_MASK    U(0x1)
#define MODE_RW_64    U(0x0)
#define MODE_RW_32    U(0x1)

#define MODE_EL_SHIFT    U(0x2)
#define MODE_EL_MASK    U(0x3)
#define MODE_EL3    U(0x3)

#define SPSR_DAIF_SHIFT    U(6)
#define SPSR_DAIF_MASK    U(0xf)

#define SPSR_SSBS_BIT_AARCH64   BIT_64(12)

#define SPSR_64(el, sp, daif)    \
    (((MODE_RW_64 << MODE_RW_SHIFT) |    \
    (((el) & MODE_EL_MASK) << MODE_EL_SHIFT) |    \
    (((sp) & MODE_SP_MASK) << MODE_SP_SHIFT) |    \
    (((daif) & SPSR_DAIF_MASK) << SPSR_DAIF_SHIFT)) &    \
    (~(SPSR_SSBS_BIT_AARCH64)))

#define SMC_RUN_IMAGE_BL31    0x31 // random number
// fast smc call, disable SCR_EL3.HCE
#define SMC_DIS_HCE   U(0xc4000021)
#define SCR_NS_BIT    (U(1) << 0)
#define SMCCC_ARCH_FEATURES    U(0x80000001)

#define SMC_OK    U(0x0)


#define SCTLR_M_BIT    (U(1) << 0)
#define SCTLR_A_BIT    (U(1) << 1)
#define SCTLR_C_BIT    (U(1) << 2)
#define SCTLR_CP15BEN_BIT    (U(1) << 5)
#define SCTLR_ITD_BIT    (U(1) << 7)
#define SCTLR_Z_BIT    (U(1) << 11)
#define SCTLR_I_BIT    (U(1) << 12)
#define SCTLR_V_BIT    (U(1) << 13)

#define HSR_EC_SHIFT   26
#define HSR_EC_HVC64   0x16
