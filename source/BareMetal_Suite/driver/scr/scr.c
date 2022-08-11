/********************************************************
 *          Copyright(c) 2019   Semidrive               *
 ********************************************************/

#include <common_hdr.h>
#include <soc.h>
#include "scr_reg.h"
#include "scr.h"

#if defined(SOC_host)
#define SCR_FUNC_TEST
#endif

#if defined(SCR_FUNC_TEST)
#undef SOC_SCR_REG_MAP
#define SOC_SCR_REG_MAP(a)  (a)
#endif

static inline U32 SCR_REG_OFF(U8 type) __attribute__((always_inline));
static inline U32 SCR_REG_OFF(U8 type)
{
    return (type == L16 ? SCR_L16_OFF :
            type == RO ? SCR_RO_OFF :
            type == L31 ? SCR_L31_OFF :
            type == R16W16 ? SCR_R16W16_OFF :
            0);    /*RW*/
}

static inline U8 SCR_BIT_GET(U32 b, U8 type, U32 pos)
__attribute__((always_inline));
static inline U8 SCR_BIT_GET(U32 b, U8 type, U32 pos)
{
    U32 off = (pos) / 32 * 4;
    U32 shift = (pos) % 32;
    off += SCR_REG_OFF(type);
    U32 v = readl(b + SOC_SCR_REG_MAP(off));

    return ((v >> shift) & 0x01u);
}

static inline void SCR_BIT_SET(U32 b,
                               U8 type, U32 pos) __attribute__((always_inline));
static inline void SCR_BIT_SET(U32 b, U8 type, U32 pos)
{
    U32 off = (pos) / 32 * 4;
    U32 shift = (pos) % 32;
    off += SCR_REG_OFF(type);
    U32 v = readl(b + SOC_SCR_REG_MAP(off));
    v |= (0x01u << shift);
    writel(v, b + SOC_SCR_REG_MAP(off));
}

static inline U32 SCR_BITS_RD(U32 b, U8 type,
                              U32 start, U32 width) __attribute__((always_inline));
static inline U32 SCR_BITS_RD(U32 b, U8 type,
                              U32 start, U32 width)
{
    U32 v = 0;
    U32 off = (start / 32) * 4;
    off += SCR_REG_OFF(type);

    if ((start / 32) != ((start + width - 1) / 32)) {
        U32 w1 = 32 - (start % 32);
        U32 w2 = width - w1;
        U32 msk2 = (-1u) >> (32 - w2);
        U32 shift1 = start % 32;
        U32 v1 = readl(b + SOC_SCR_REG_MAP(off));
        U32 v2 = readl(b + SOC_SCR_REG_MAP(off + 4));
        v = (v1 >> shift1) | ((v2 & msk2) << w1);
    } else {
        U32 msk = (-1u) >> (32 - width);
        v = readl(b + SOC_SCR_REG_MAP(off));
        v = (v >> (start % 32)) & msk;
    }

    return v;
}

static inline void SCR_BITS_WR(U32 b, U8 type,
                               U32 start, U32 width, U32 val) __attribute__((always_inline));
static inline void SCR_BITS_WR(U32 b, U8 type,
                               U32 start, U32 width, U32 val)
{
    U32 v = 0;
    U32 off = (start / 32) * 4;
    off += SCR_REG_OFF(type);

    if ((start / 32) != ((start + width - 1) / 32)) {
        U32 w1 = 32 - (start % 32);
        U32 w2 = width - w1;
        U32 msk1 = (-1u) >> (32 - w1), msk2 = (-1u) >> (32 - w2);
        U32 shift1 = start % 32;
        v = readl(b + SOC_SCR_REG_MAP(off));
        v &= ~(msk1 << shift1);
        v |= ((val) & msk1) << shift1;
#if defined(SOC_host)
        DBG("scr_bits_wr: v=0x%x, off=0x%x\n", v, off);
#endif
        writel(v, b + SOC_SCR_REG_MAP(off));
        v = readl(b + SOC_SCR_REG_MAP(off + 4));
        v &= ~msk2;
        v |= (((val) >> w1) & msk2);
#if defined(SOC_host)
        DBG("scr_bits_wr: v=0x%x, off=0x%x\n", v, off);
#endif
        writel(v, b + SOC_SCR_REG_MAP(off + 4));
    } else {
        U32 msk = (-1u) >> (32 - width);
        v = readl(b + SOC_SCR_REG_MAP(off));
        v &= ~(msk << (start % 32));
        v |= ((val) & msk) << (start % 32);
#if defined(SOC_host)
        DBG("scr_bits_wr: v=0x%x, off=0x%x\n", v, off);
#endif
        writel(v, b + SOC_SCR_REG_MAP(off));
    }
}

void scr_bit_set(U32 b, U8 type, U32 pos)
{
    SCR_BIT_SET(b, type, pos);
}

void ramfunc_scr_bit_set(U32 b, U8 type, U32 pos) __RAM_FUNC__;
void ramfunc_scr_bit_set(U32 b, U8 type, U32 pos)
{
    SCR_BIT_SET(b, type, pos);
}

void scr_bits_wr(U32 b, U8 type, U32 start, U32 width, U32 val)
{
    SCR_BITS_WR(b, type, start, width, val);
}

void ramfunc_scr_bits_wr(U32 b, U8 type,
                         U32 start, U32 width, U32 val) __RAM_FUNC__;
void ramfunc_scr_bits_wr(U32 b, U8 type, U32 start, U32 width, U32 val)
{
    SCR_BITS_WR(b, type, start, width, val);
}

U32 scr_bits_rd(U32 b, U8 type, U32 start, U32 width)
{
    return SCR_BITS_RD(b, type, start, width);
}

void scr_bit_clr(U32 b, U8 type, U32 pos)
{
    U32 off = (pos) / 32 * 4;
    U32 shift = (pos) % 32;
    off += SCR_REG_OFF(type);
    U32 v = readl(b + SOC_SCR_REG_MAP(off));
    v &= ~(0x01u << shift);
    writel(v, b + SOC_SCR_REG_MAP(off));
}

U8 scr_bit_get(U32 b, U8 type, U32 pos)
{
    return  SCR_BIT_GET(b, type, pos);
}

U8 ramfunc_scr_bit_get(U32 b, U8 type, U32 pos) __RAM_FUNC__;
U8 ramfunc_scr_bit_get(U32 b, U8 type, U32 pos)
{
    return  SCR_BIT_GET(b, type, pos);
}

#if defined(SCR_FUNC_TEST)
U32 scr_array[512];
void scr_func_test(void)
{
    memclr(scr_array, sizeof(scr_array));

    U32 b = (U32)(uintptr_t)&scr_array[0];
    DBG("b=0x%x\n", b);
    scr_bits_wr(b, RW, 0, 4, (-1u));
    DBG("scr_array[0] = 0x%x\n", scr_array[0]);
    scr_bits_wr(b, RW, 4, 1, (-1u));
    DBG("scr_array[0] = 0x%x\n", scr_array[0]);

    scr_bits_wr(b, RW, 28, 8, (-1u));
    DBG("scr_array[0] = 0x%x\n", scr_array[0]);
    DBG("scr_array[1] = 0x%x\n", scr_array[1]);

    scr_bits_wr(b, RW, 60, 32, (-1u));
    DBG("scr_array[0] = 0x%x\n", scr_array[0]);
    DBG("scr_array[1] = 0x%x\n", scr_array[1]);
    DBG("scr_array[2] = 0x%x\n", scr_array[2]);

    if (0xf000001f != scr_array[0]
        || 0xf000000f != scr_array[1]
        || 0x0fffffff != scr_array[2]) {
        DBG("%s: Opps, failed.\n", __FUNCTION__);
    }

    scr_bit_set(b, RW, 100);
    DBG("scr_array[3]=0x%x\n", scr_array[100 / 32]);
    DBG("scr_bit_get(b, RW, 100)=0x%x\n", scr_bit_get(b, RW, 100));

    if (!scr_bit_get(b, RW, 100)) {
        DBG("%s: Opps, failed.\n", __FUNCTION__);
    }

    scr_bit_clr(b, RW, 100);
    DBG("scr_array[3]=0x%x\n", scr_array[100 / 32]);
    DBG("scr_bit_get(b, RW, 100)=0x%x\n", scr_bit_get(b, RW, 100));

    if (scr_bit_get(b, RW, 100)) {
        DBG("%s: Opps, failed.\n", __FUNCTION__);
    }

    scr_array[SCR_RO_OFF / 4 + 1] = 0x30;

    if (0x3 != scr_bits_rd(b, RO, 36, 4)) {
        DBG("%s: Opps, failed.\n", __FUNCTION__);
    }

    scr_bit_set(b, L16, 100);
    DBG("scr_array[SCR_L16_OFF/4 + 3]=0x%x\n", scr_array[SCR_L16_OFF / 4 + 100 / 32]);
    DBG("scr_bit_get(b, L16, 100)=0x%x\n", scr_bit_get(b, L16, 100));

    if (!scr_bit_get(b, L16, 100)) {
        DBG("%s: Opps, failed.\n", __FUNCTION__);
    }

    scr_bit_set(b, L31, 100);
    DBG("scr_array[SCR_L31_OFF/4 + 3]=0x%x\n", scr_array[SCR_L31_OFF / 4 + 100 / 32]);
    DBG("scr_bit_get(b, L31, 100)=0x%x\n", scr_bit_get(b, L31, 100));

    if (!scr_bit_get(b, L31, 100)) {
        DBG("%s: Opps, failed.\n", __FUNCTION__);
    }

    scr_bits_wr(b, L16, 32, 7, (-1));
    DBG("scr_array[SCR_L16_OFF/4 + 32/32]=0x%x\n", scr_array[SCR_L16_OFF / 4 + 32 / 32]);
    U32 v = scr_bits_rd(b, L16, 32, 7);
    DBG("scr_bits_rd(b, L16, 32, 7) as 0x%x\n", v);

    if ((scr_array[SCR_L16_OFF / 4 + 32 / 32] != 0x7fu) || (0x7fu != v)) {
        DBG("%s: Opps, failed.\n", __FUNCTION__);
    }

    scr_bits_wr(b, L31, 32, 7, (-1));
    DBG("scr_array[SCR_L31_OFF/4 + 32/32]=0x%x\n", scr_array[SCR_L31_OFF / 4 + 32 / 32]);
    v = scr_bits_rd(b, L31, 32, 7);
    DBG("scr_bits_rd(b, L31, 32, 7) as 0x%x\n", v);

    if ((scr_array[SCR_L31_OFF / 4 + 32 / 32] != 0x7fu) || (0x7fu != v)) {
        DBG("%s: Opps, failed.\n", __FUNCTION__);
    }

    scr_array[SCR_L31_OFF / 4] = 0;
    scr_array[SCR_L31_OFF / 4 + 1] = 0;
    scr_bits_wr(b, L31, 0, 32, (-1));
    DBG("scr_array[SCR_L31_OFF/4 - (+1)] = 0x%x_0x%x\n",
        scr_array[SCR_L31_OFF / 4], scr_array[SCR_L31_OFF / 4 + 1]);
    v = scr_bits_rd(b, L31, 0, 32);

    if (scr_array[SCR_L31_OFF / 4] != (-1) || scr_array[SCR_L31_OFF / 4 + 1] != 0
        || v != (-1)) {
        DBG("%s: Opps, failed.\n", __FUNCTION__);
    }

    scr_array[SCR_L31_OFF / 4] = 0;
    scr_array[SCR_L31_OFF / 4 + 1] = 0;
    scr_bits_wr(b, L31, 31, 32, (-1));
    v = scr_bits_rd(b, L31, 31, 32);
    DBG("scr_array[SCR_L31_OFF/4 - (+1)] = 0x%x_0x%x, v=0x%x\n",
        scr_array[SCR_L31_OFF / 4], scr_array[SCR_L31_OFF / 4 + 1], v);

    if (scr_array[SCR_L31_OFF / 4] != (0x01u << 31) ||
        scr_array[SCR_L31_OFF / 4 + 1] != 0x7fffffffu || v != (-1)) {
        DBG("%s: Opps, failed.\n", __FUNCTION__);
    }
}
#endif
