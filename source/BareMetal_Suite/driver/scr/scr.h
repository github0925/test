/********************************************************
 *          Copyright(c) 2019   Semidrive               *
 ********************************************************/

#ifndef __SCR_H__
#define __SCR_H__

typedef enum {
    RW,
    RO,
    L16,
    L31,
    R16W16,
} scr_reg_type_e;

#define SCR_L31_LCK_BIT(bit)    ((bit)/32*32 + 31)
#define SCR_L16_LCK_BIT(bit)    ((bit) + 16)

U8 scr_bit_get(U32 b, U8 type, U32 pos);
void scr_bit_set(U32 b, U8 type, U32 pos);
void scr_bit_clr(U32 b, U8 type, U32 pos);
U32 scr_bits_rd(U32 b, U8 type, U32 start, U32 width);
void scr_bits_wr(U32 b, U8 type, U32 start, U32 width, U32 val);

void ramfunc_scr_bit_set(U32 b, U8 type, U32 pos);
void ramfunc_scr_bits_wr(U32 b, U8 type, U32 start, U32 width, U32 val);
U8 ramfunc_scr_bit_get(U32 b, U8 type, U32 pos);
#endif  /* __SCR_H__ */
