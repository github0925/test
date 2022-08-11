/********************************************************
 *  Copyright(c) 2019   Semidrive       *
 *  All Right Reserved.
 *******************************************************/

#ifndef __HELPER_H__
#define __HELPER_H__

#include <types_def.h>

#if !defined(VTEST)

#define readl(a)    *(volatile U32*)((intptr_t)(a))
#define writel(v, a)    *((volatile U32*)((intptr_t)(a))) = (U32) (v)
#define readh(a)    *((volatile U16*)((intptr_t)(a)))
#define writeh(v, a)    *((volatile U16*)((intptr_t)(a))) = (U16) (v)
#define readb(a)    *((volatile U8*)((intptr_t)(a)))
#define writeb(v, a)    *((volatile U8*)((intptr_t)(a))) = (U8) (v)

#define REG64(addr) ((volatile uint64_t *)(uintptr_t)(addr))
#define REG32(addr) ((volatile uint32_t *)(uintptr_t)(addr))
#define REG16(addr) ((volatile uint16_t *)(uintptr_t)(addr))
#define REG8(addr) ((volatile uint8_t *)(uintptr_t)(addr))

#define RMWREG64(addr, startbit, width, val) *REG64(addr) = (*REG64(addr) & ~(((1ul<<(width)) - 1) << (startbit))) | ((val) << (startbit))
#define RMWREG32(addr, startbit, width, val) *REG32(addr) = (*REG32(addr) & ~(((1ull<<(width)) - 1) << (startbit))) | ((val) << (startbit))
#define RMWREG16(addr, startbit, width, val) *REG16(addr) = (*REG16(addr) & ~(((1ul<<(width)) - 1) << (startbit))) | ((val) << (startbit))
#define RMWREG8(addr, startbit, width, val) *REG8(addr) = (*REG8(addr) & ~(((1ul<<(width)) - 1) << (startbit))) | ((val) << (startbit))

#define REG_RMWREG64(addr, startbit, width, val)    RMWREG64(addr, startbit, width, val)
#define REG_RMWREG32(addr, startbit, width, val)    RMWREG32(addr, startbit, width, val)
#define REG_RMWREG16(addr, startbit, width, val)    RMWREG16(addr, startbit, width, val)
#define REG_RMWREG8(addr, startbit, width, val)     RMWREG8(addr, startbit, width, val)

#define GET32_FLD(d, s, w)  ((readl(d)>>(s)) & ((0x01u<<(w)) - 1))
#else

extern void rd(uint32_t, uint32_t *);
extern void wr(uint32_t, uint32_t);
extern void rd_halfword(uint32_t, uint32_t *);
extern void wr_halfword(uint32_t, uint32_t);
extern void rd_byte(uint32_t, uint32_t *);
extern void wr_byte(uint32_t, uint32_t);
extern void io_printf(const char *fmt, ...);

static inline uint32_t readl(uint32_t a)
{
    uint32_t t = 0;
    rd(a, &t);
    return t;
}
static inline void writel(uint32_t v, uint32_t a)
{
    //DBG("%s: v=0x%x, a=0x%x\n", __FUNCTION__, v, a);
    wr(a, v);
}

static inline uint16_t readh(uint32_t a)
{
    uint32_t t = 0;
    rd_halfword(a, &t);
    return (uint16_t)t;
}
static inline void writeh(uint16_t v, uint32_t a)
{
    wr_halfword(a, (uint32_t)v);
}

static inline uint8_t readb(uint32_t a)
{
    uint32_t t = 0;
    rd_byte(a, &t);
    return (uint8_t)t;
}
static inline void writeb(uint8_t v, uint32_t a)
{
    wr_byte(a, (uint32_t)v);
}

#define REG32(addr) ((volatile uint32_t *)(uintptr_t)(addr))
#define RMWREG32(addr, startbit, width, val) *REG32(addr) = (*REG32(addr) & ~(((1<<(width)) - 1) << (startbit))) | ((val) << (startbit))

#define REG_RMWREG32(a, start, width, v)    \
    do {\
        uint32_t t = readl((uint32_t)(uintptr_t)a);\
        t &= ~(((1<<(width)) - 1) << (start));\
        t |= (v) << (start);\
        writel(t, (uint32_t)(uintptr_t)a);   \
    } while (0)

#define vaddr_to_paddr(a)  soc_to_dma_address((uintptr_t)(a))

#endif  /* VTEST */

#define BIT_(x)     (0x01u << (x))
#define BIT(x, bit) ((x) & (1UL << (bit)))
#define BIT_SHIFT(x, bit) (((x) >> (bit)) & 1)
#define BITS(x, high, low) ((x) & (((1UL<<((high)+1))-1) & ~((1UL<<(low))-1)))
#define BITS_SHIFT(x, high, low) (((x) >> (low)) & ((1UL<<((high)-(low)+1))-1))
#define BIT_SET(x, bit) (((x) & (1UL << (bit))) ? 1 : 0)

#define UNUSED_VAR(x)   (x) = (x)

#define __IN_BSS2__
#define __RAM_FUNC__
#define __WEAK__    __attribute__((weak))

#define FLD_OFFSET(st, fld)     ((U32)(uintptr_t)(&(((st*)0)->fld)))

#define GET_BITS(v, s, w)   \
        (((v) >> (s)) & (0xffffffff >> (32 - (w))))

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

#define ROUNDUP(a, b) (((a) + ((b)-1)) & ~((b)-1))
#define ROUNDDOWN(a, b) ((a) & ~((b)-1))

#define ALIGN(a, b) ROUNDUP(a, b)
#define IS_ALIGNED(a, b) (!(((uintptr_t)(a)) & (((uintptr_t)(b))-1)))

#define TIMEOUT_1ms     1000UL
#define TIMEOUT_1s      (1000ULL*1000ULL)
#define TIMEOUT_1min    (TIMEOUT_1s*60ULL)

#define B2W(b0, b1, b2, b3) \
    ((b0) | ((b1) << 8) | ((b2) << 16) | ((b3) << 24))

#endif  /* __HELPER_H__ */
