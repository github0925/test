/********************************************************
 *      Copyright(c) 2018   Semidrive  Semiconductor    *
 *      All rights reserved.                            *
 ********************************************************/

#ifndef __SC_SS_PFPLL_H__
#define __SC_SS_PFPLL_H__

#define PLL_FBDIV(x)    (x)
#define PLL_REFDIV(x)   (x)
#define PLL_POSTDIV1(x) (x)
#define PLL_FRAC(x)     (x)

#define DIV_A_V(v)  (U8)(v)
#define DIV_B_V(v)  (U8)((v) >> 8)
#define DIV_C_V(v)  (U8)((v) >> 16)
#define DIV_D_V(v)  (U8)((v) >> 24)

#define DIV_ABCD(a, b, c, d)    \
        (((a) & 0xf) \
         | (((b) & 0xf) << 8) \
         | (((c) & 0xf) << 16)\
         | (((d) & 0xf) << 24))

void sc_pfpll_program(U32 base, U32 fbdiv, U32 refdiv, U32 frac,
                      U32 postdiv1, U32 div_abcd);
bool sc_pfpll_is_enabled(U32 base);

#endif  /* __SC_SS_PFPLL_H__ */
