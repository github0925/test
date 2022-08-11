/********************************************************
 *          Copyright(c); 2018  Semidrive               *
 ********************************************************/

#ifndef __CKGEN_H__
#define __CKGEN_H__

#include <common_hdr.h>
#include "ckgen_reg.h"

#define SLICE_ID(x) (x)
#define PREDIV(x)   (x)
#define POSTDIV(x)  (x)
#define SLICE_SRC(x)    (x)
#define UUU_ID(x)   (x)

#define DIV_MNPQ(m, n, p, q)    \
    (FV_CKGEN_UUU_SLICE_M_DIV_NUM(m) \
      | FV_CKGEN_UUU_SLICE_N_DIV_NUM(n)\
      | FV_CKGEN_UUU_SLICE_P_DIV_NUM(p)\
      | FV_CKGEN_UUU_SLICE_Q_DIV_NUM(q))

typedef enum {
    PATH_A = 1,
    PATH_B,
} slice_path_e;

typedef enum {
    UUU_SEL_CKGEN_SOC = 0,
    UUU_SEL_PLL = 3,
} uuu_src_sel_e;

void ckgen_ip_slice_cfg(U32 base, U32 slice_id,
                        U32 src_sel, U32 pre_div, U32 post_div);
void ckgen_bus_slice_cfg(U32 base, U32 slice_id,
                         U32 path, U32 src_sel, U32 pre_div);
void ckgen_bus_slice_postdiv_update(U32 base, U32 slice_id, U32 post_div);
void ckgen_bus_slice_switch(U32 base, U32 slice_id);
void ckgen_core_slice_cfg(U32 base, U32 slice_id, U32 path, U32 src_sel);
void ckgen_core_slice_postdiv_update(U32 base, U32 slice_id, U32 post_div);
void ckgen_core_slice_switch(U32 base, U32 slice_id);
void ckgen_cg_en(U32 base, U32 id);
void ckgen_cg_dis(U32 base, U32 id);
void ckgen_uuu_slice_cfg(U32 base, U32 slice_id, U32 src_sel, U32 div_mnpq);
void ckgen_ip_slice_gate(U32 base, U32 slice_id);
void ckgen_ip_slice_ungate(U32 base, U32 slice_id);

#endif  /* __CKGEN_H__ */
