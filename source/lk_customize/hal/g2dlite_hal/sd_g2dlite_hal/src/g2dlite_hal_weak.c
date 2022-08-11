/*
* g2dlite_hal_weak.c
*
* Copyright (c) 2019 Semidrive Semiconductor.
* All rights reserved.
*
* Description:
*
* Revision History:
* -----------------
* 011, 04/27/2020 BI create this file
*/
#include <res.h>
#include <g2dlite_api.h>
#include <g2dlite_hal.h>
#include <g2dlite_drv.h>
#include <g2dlite_common.h>


static bool hal_g2dlite_addr_to_irq(uint32_t addr, uint32_t *irq_num)
{
    return true;
}

static struct g2dlite_instance* hal_g2dlite_get_instance(uint32_t res_glb_idx)
{
    return NULL;
}

bool hal_g2dlite_creat_handle(void **handle, uint32_t res_glb_idx)
{
    return true;
}

void hal_g2dlite_init(void *handle)
{
}

void hal_g2dlite_update(void *handle, struct g2dlite_input *input)
{
}

void hal_g2dlite_blend(void *handle, struct g2dlite_input *input)
{
}

void hal_g2dlite_scaler(void *handle, struct g2dlite_input *input)
{
}

void hal_g2dlite_csc(void *handle, struct g2dlite_input *input)
{
}

void hal_g2dlite_rotaion(void *handle, struct g2dlite_input *input)
{
}

void hal_g2dlite_fill_rect(void *handle, u32 color, u8 g_alpha,
    addr_t aaddr, u8 bpa, u32 astride, struct g2dlite_output_cfg *output)
{
}

void hal_g2dlite_fastcopy(void *handle, struct fcopy_t *in,
    struct fcopy_t *out)
{
}

void hal_g2dlite_clut_setting(void *handle, char *clut_table)
{
}
