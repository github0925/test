/*
* g2dlite_hal.c
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

struct g2dlite_instance g2dlite_handle = {0};

static bool hal_g2dlite_addr_to_irq(uint32_t addr, uint32_t *irq_num)
{
    for (int i = 0; i < G2DLITE_MAX_NUM; i++) {
        if (g2dlite_addr2irq_table[i].addr == addr) {
            *irq_num = g2dlite_addr2irq_table[i].irq_num;
            return true;
        }
    }

    return false;
}

static struct g2dlite_instance* hal_g2dlite_get_instance(uint32_t res_glb_idx)
{
    addr_t addr;
    int index, ret;
    struct g2dlite_instance *inst = NULL;

    ret = res_get_info_by_id(res_glb_idx, &addr, &index);
    if (ret < 0) {
        LOGE("get info by id[0x%x]failed\n", res_glb_idx);
        return NULL;
    }

    inst = &g2dlite_handle;
    inst->reg_addr = addr;

    if (!hal_g2dlite_addr_to_irq(inst->reg_addr, &inst->irq_num)) {
        LOGE("get irq by addr[0x%x]failed\n", (unsigned int)inst->reg_addr);
        return NULL;
   }

    return inst;
}

bool hal_g2dlite_creat_handle(void **handle, uint32_t res_glb_idx)
{
    static struct g2dlite_instance *g2dlite_inst = NULL;

    if (g2dlite_inst) {
        *handle = g2dlite_inst;
        return true;
    }

    g2dlite_inst = hal_g2dlite_get_instance(res_glb_idx);

    if (!g2dlite_inst) {
        *handle = NULL;
        return false;
    }

    *handle = g2dlite_inst;
    return true;
}

void hal_g2dlite_init(void *handle)
{
    static bool inited = false;

    if (inited)
        return;

    sd_g2dlite_init(handle);
    inited = true;
}

void hal_g2dlite_update(void *handle, struct g2dlite_input *input)
{
    sd_g2dlite_update(handle, input);
}

void hal_g2dlite_blend(void *handle, struct g2dlite_input *input)
{
    sd_g2dlite_update(handle, input);
}

void hal_g2dlite_scaler(void *handle, struct g2dlite_input *input)
{
    sd_g2dlite_update(handle, input);
}

void hal_g2dlite_csc(void *handle, struct g2dlite_input *input)
{
    sd_g2dlite_update(handle, input);
}

void hal_g2dlite_rotaion(void *handle, struct g2dlite_input *input)
{
    sd_g2dlite_rotaion(handle, input);
}

void hal_g2dlite_fill_rect(void *handle, u32 color, u8 g_alpha,
    addr_t aaddr, u8 bpa, u32 astride, struct g2dlite_output_cfg *output)
{
    struct g2dlite_bg_cfg bgcfg;

    bgcfg.en = 1;
    bgcfg.color = color;
    bgcfg.g_alpha = g_alpha;
    bgcfg.aaddr = aaddr;
    bgcfg.bpa = bpa;
    bgcfg.astride = astride;

    sd_g2dlite_fill_rect(handle, &bgcfg, output);
}

void hal_g2dlite_fastcopy(void *handle, struct fcopy_t *in,
    struct fcopy_t *out)
{
    sd_g2dlite_fastcopy(handle, in, out);
}

void hal_g2dlite_clut_setting(void *handle, char *clut_table)
{
    sd_g2dlite_clut_setting(handle, clut_table);
}
