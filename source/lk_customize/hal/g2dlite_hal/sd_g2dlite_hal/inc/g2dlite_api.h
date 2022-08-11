/*
* g2dlite_api.h
*
* Copyright (c) 2019 Semidrive Semiconductor.
* All rights reserved.
*
* Description:
*
* Revision History:
* -----------------
* 011, 07/28/2020 BI create this file
*/

#ifndef __G2DLITE_API_H__
#define __G2DLITE_API_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

#define G2DLITE_LAYER_MAX_NUM       2

enum {
    BLEND_PIXEL_NONE = 0,
    BLEND_PIXEL_PREMULTI,
    BLEND_PIXEL_COVERAGE
};

typedef enum {
    ROTATION_TYPE_NONE    = 0x0,
    ROTATION_TYPE_ROT_90  = 0x1,
    ROTATION_TYPE_HFLIP   = 0x2,
    ROTATION_TYPE_VFLIP   = 0x4,
    ROTATION_TYPE_ROT_180 = ROTATION_TYPE_VFLIP | ROTATION_TYPE_HFLIP,
    ROTATION_TYPE_ROT_270 = ROTATION_TYPE_ROT_90 | ROTATION_TYPE_VFLIP | ROTATION_TYPE_HFLIP,
    ROTATION_TYPE_VF_90   = ROTATION_TYPE_VFLIP | ROTATION_TYPE_ROT_90,
    ROTATION_TYPE_HF_90   = ROTATION_TYPE_HFLIP | ROTATION_TYPE_ROT_90,
} rotation_type;

typedef enum {
    PD_NONE = 0,
    PD_SRC = 0x1,
    PD_DST = 0x2
} PD_LAYER_TYPE;

typedef enum {
    CLEAR = 0,
    SRC,
    DST,
    SRC_OVER,
    DST_OVER,
    SRC_IN,
    DST_IN,
    SRC_OUT,
    DST_OUT,
    SRC_ATOP,
    DST_ATOP,
    XOR,
    DARKEN,
    LIGHTEN,
    MULTIPLY,
    SCREEN,
    ADD,
    OVERLAY,
    SRC_SUB,
    DES_SUB
} pd_mode_t;

struct rect_t {
    u32 x;
    u32 y;
    u32 w;
    u32 h;
};

struct ckey_range_t{
    u32 r_dn;
    u32 r_up;
    u32 g_dn;
    u32 g_up;
    u32 b_dn;
    u32 b_up;
};

struct ckey_t {
    u8 en;
    u32 alpha;
    struct ckey_range_t range;
};

struct rle_t {
    u32 en;
    u32 rle_y_len;
    u32 rle_y_checksum;
    u32 rle_data_size;
};

struct clut_t {
    u32 en;
    addr_t addr;
};

struct pd_t {
    u32 en;
    u32 zorder;
    pd_mode_t mode;
    u8 alpha_need;
};

struct fcopy_t {
    addr_t addr;
    u32 width;
    u32 height;
    u32 stride;
};

struct g2dlite_bg_cfg {
    u32 en;
    u32 color;
    u8 g_alpha;
    u8 zorder;

    addr_t aaddr;
    u8 bpa;
    u32 astride;
    PD_LAYER_TYPE pd_type;
};

struct g2dlite_input_cfg{
    u8 layer;
    u8 layer_en;
    u32 fmt;
    u32 zorder;
    struct rect_t src;
    u64 addr[4];//YUVA
    u32 src_stride[4];
    struct rect_t dst;
    u32 blend;
    u8 alpha;
    struct ckey_t ckey;
    struct rle_t rle;
    struct clut_t clut;
    PD_LAYER_TYPE pd_type;
};

struct g2dlite_output_cfg{
    u32 width;
    u32 height;
    u32 fmt;
    u64 o_x;
    u64 o_y;
    u64 addr[4];
    u32 stride[4];
    u32 rotation;
};

struct g2dlite_input{
    u8 layer_num;
    struct pd_t pd_info;
    struct g2dlite_bg_cfg bg_layer;
    struct g2dlite_input_cfg layer[G2DLITE_LAYER_MAX_NUM];
    struct g2dlite_output_cfg output;
};

struct g2dlite_instance {
    int index;
    addr_t reg_addr;
    u32 irq_num;
};

struct hal_g2dlite_addr2irq_t {
    u32 addr;
    u32 irq_num;
};

bool hal_g2dlite_creat_handle(void **handle, u32 res_glb_idx);
void hal_g2dlite_init(void *handle);
void hal_g2dlite_update(void *handle, struct g2dlite_input *input);
void hal_g2dlite_blend(void *handle, struct g2dlite_input *input);
void hal_g2dlite_scaler(void *handle, struct g2dlite_input *input);
void hal_g2dlite_csc(void *handle, struct g2dlite_input *input);
void hal_g2dlite_rotaion(void *handle, struct g2dlite_input *input);
void hal_g2dlite_fill_rect(void *handle, u32 color, u8 g_alpha,
    addr_t aaddr, u8 bpa, u32 astride, struct g2dlite_output_cfg *output);

/**
 *@in:
 *@@addr:input buffer address
 *@@width@height: input buffer size = in->width * in->height (uint is Word)
 *@@stride:input buffer pitch (uint is Byte)
 *@out:
 *@@addr:out buffer address
 *@@width@height: input buffer size = out->width * out->height (uint is Word)
 *@@stride:out buffer pitch (uint is Byte), must align to 128 bits
 */
void hal_g2dlite_fastcopy(void *handle, struct fcopy_t *in,
    struct fcopy_t *out);

void hal_g2dlite_clut_setting(void *handle, char *clut_table);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif //__G2DLITE_API_H__
