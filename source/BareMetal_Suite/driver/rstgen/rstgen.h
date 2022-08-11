/********************************************************
 *          Copyright(c) 2018   Semidrive               *
 *******************************************************/

#include <common_hdr.h>
#ifndef __RSTGEN_H__
#define __RSTGEN_H__

void rg_module_reset(U32 base, U32 id, U8 val);
U32 rg_rd_gpr(U32 base, U32 id);
void rg_wr_gpr(U32 base, U32 id, U32 v);
U32 rg_get_reset_source(U32 base);
U32 rg_glb_reset_en(U32 base, U32 msk);
U32 rg_glb_reset_dis(U32 base, U32 msk);
void rg_glb_self_reset(U32 base, U8 v);
void rg_glb_other_reset(U32 base, U8 v);
U32 rg_get_reset_source(U32 base);
void ramfunc_rg_core_reset(U32 base, U32 id);
void rg_core_reset(U32 base, U32 id);
void rg_en_isolation(U32 base, U8 id);
void rg_dis_isolation(U32 base, U8 id);

#endif  /* __RSTGEN_H__ */
