//*****************************************************************************
//
// disp_hal.h - Prototypes for the disp hal
//
// Copyright (c) 2019-2029 Semidrive Incorporated.  All rights reserved.
// Software License Agreement
// SDM  (semidrive display module)
//
//
//*****************************************************************************

#include "disp_hal.h"
#include "sdm_panel.h"
#include "disp_drv.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <platform.h>
#include <kernel/thread.h>
#include <kernel/spinlock.h>
#include <system_configs_parse.h>

#include <disp_ssystem_link.h>
#if WITH_HAL_MODULE_HELPER_HAL
#include <module_helper_hal.h>
#endif

#include <disp_res.h>

/********** global **********/
display_handle g_instances[DISPLAY_TYPE_MAX] = {0};
spin_lock_t disp_spin_lock = SPIN_LOCK_INITIAL_VALUE;
int g_display_num = 0;

typedef struct
{
    int enum_res_id;
    uint32_t resources[8];
} enum_res_to_idx;

enum_res_to_idx table_dp_res[] = {
    //dp
    {DP_RES_DP1, {RES_DP_DP1}},
    {DP_RES_DP2, {RES_DP_DP2}},
    {DP_RES_DP3, {RES_DP_DP3}},
    {DP_RES_DP1_DP2, {RES_DP_DP1, DP_RES_DP2}},
    {-1, {0}},
};
enum_res_to_idx table_dc_res[] = {
    //dc
    {DC_RES_DC1, {RES_DC_DC1}},
    {DC_RES_DC2, {RES_DC_DC2}},
    {DC_RES_DC3, {RES_DC_DC3}},
    {DC_RES_DC4, {RES_DC_DC4}},
    {DC_RES_DC5, {RES_DC_DC5}},
    {DC_RES_DC1_DC2, {RES_DC_DC1, RES_DC_DC2}},
    {DC_RES_DC3_DC4, {RES_DC_DC3, RES_DC_DC4}},
    {-1, {0}},
};

enum_res_to_idx table_if_res[] = {
    //lvds
    {IF_RES_LVDS1, {RES_LVDS_LVDS1}},
    {IF_RES_LVDS2, {RES_LVDS_LVDS2}},
    {IF_RES_LVDS3, {RES_LVDS_LVDS3}},
    {IF_RES_LVDS4, {RES_LVDS_LVDS4}},
    {IF_RES_LVDS1_LVDS2, {RES_LVDS_LVDS1, RES_LVDS_LVDS2}},
    {IF_RES_LVDS3_LVDS4, {RES_LVDS_LVDS3, RES_LVDS_LVDS4}},
    {IF_RES_LVDS1_4, {RES_LVDS_LVDS1, RES_LVDS_LVDS2, RES_LVDS_LVDS3, RES_LVDS_LVDS4}},

    //mipi
    {IF_RES_MIPI_DSI1, {RES_MIPI_DSI_MIPI_DSI1}},
    {IF_RES_MIPI_DSI2, {RES_MIPI_DSI_MIPI_DSI2}},
    {IF_RES_DSI1_DSI2, {RES_MIPI_DSI_MIPI_DSI1, RES_MIPI_DSI_MIPI_DSI2}},
    {-1, {0}},
};

uint8_t dc_pll[5] = {
#if defined(TARGET_REFERENCE_X9U)
    /*X9 high(plus)*/
    DSP_CLK_PLL_LVDS1, //DC1 safety Infotainmen
    DSP_CLK_PLL_LVDS2, //DC2 safety Cluster
    DSP_CLK_PLL_LVDS3, //DC3 safety ControlPanel
    DSP_CLK_PLL_LVDS1, //DC4 safety Entertainment
    DSP_CLK_PLL_LVDS4  //DC5 ecockpit HUD
#else
    /*X9 high(plus)*/
    DSP_CLK_PLL_LVDS1, //DC1 safety Infotainmen
    DSP_CLK_PLL_LVDS2, //DC2 safety Cluster
    DSP_CLK_PLL_LVDS3, //DC3 safety ControlPanel
    DSP_CLK_PLL_LVDS4, //DC4 safety Entertainment
    DSP_CLK_PLL_LVDS4  //DC5 ecockpit HUD
#endif
};

static unsigned int res_count = 0;

#ifdef _SYSTEM_CONFIG
static struct display_resource *g_display_res = NULL;

static int get_cfg_info(void)
{
    addr_t addr_base = 0;
    addr_t *addr_ptr = NULL;
    static int cfg_get = 0;

    if (cfg_get)
        return 0;

    get_config_info(MODULE_DISPLAY_SAF_CFG, &addr_base);
    if (!addr_base) {
        LOGE("get cfg info failed\n");
        return -1;
    }
    addr_ptr = (addr_t *)addr_base;
    res_count = *addr_ptr;
    g_display_res = (struct display_resource *)((int*)addr_ptr + 1);
    cfg_get = 1;

    return 0;
}
#endif

int get_resource_from_table(int table_item, const enum_res_to_idx *table,
                            int *num_out, mod_res_t **out)
{
    const enum_res_to_idx *item;
    mod_res_t *r = NULL;
    int res_num = 0;
    int i;
    // size_t table_size = ARRAY_SIZE(table);

    if (table_item == -1) {
        goto FAIL_DOMAIN;
    }
    item = NULL;
    for (i = 0;table[i].enum_res_id != -1; i++) {
        if (table[i].enum_res_id == table_item) {
            item = &table[i];
            break;
        }
    }
    if (item == NULL) {
        goto FAIL_DOMAIN;
    }

    for (i = 0, res_num = 0;; i++)
    {
        if (item->resources[i] == 0)
            break;
        res_num++;
    }
    r = malloc(sizeof(mod_res_t) * res_num);

    for (i = 0; i < res_num; i++)
    {
        int ret;
        r[i].res_id = item->resources[i];
        ret = res_get_info_by_id(r[i].res_id, &r[i].base, &r[i].index);
        if (ret) {
            LOGE("Error get info by id: %d,  (%d)",i, ret);
            goto FAIL_DOMAIN;
        }
    }
    *num_out = res_num;
    *out = r;
    return 0;

FAIL_DOMAIN:
    if (r)
        free(r);
    *out = NULL;
    return -1;
}

int install_resource(struct display_resource *res, struct sdm_domain_res **out)
{
    struct sdm_domain_res *dm = NULL;
    int ret = 0;

    dm = (struct sdm_domain_res *)malloc(sizeof(struct sdm_domain_res));
    if (!dm)
    {
        LOGE("malloc dm failed\n");
        goto DM_FAIL;
    }
    memset(dm, 0, sizeof(struct sdm_domain_res));

#if 0
//dp
    ret = get_resource_from_table(res->dp_res, table_dp_res, &dm->num_dp, &dm->dp);
    if (ret)
    {
        LOGE("get dp failed\n");
        //goto DM_FAIL;
    }
#endif

//dc
    ret = get_resource_from_table(res->dc_res, table_dc_res, &dm->num_dc, &dm->dc);
    if (ret)
    {
        LOGE("get dc failed\n");
        goto DM_FAIL;
    }

//if-res
    switch (res->if_res)
    {
    case IF_RES_MIPI_DSI1:
    case IF_RES_MIPI_DSI2:
    case IF_RES_DSI1_DSI2:
        ret = get_resource_from_table(res->if_res, table_if_res, &dm->num_intf, &dm->intf);
        if (ret)
        {
            LOGE("get dsi failed\n");
            goto DM_FAIL;
        }
        break;
    case IF_RES_LVDS1 ... IF_RES_LVDS4:
    case IF_RES_LVDS1_LVDS2:
    case IF_RES_LVDS3_LVDS4:
      //  if (LVDS_RES_NUM) {
            ret = get_resource_from_table(res->if_res, table_if_res, &dm->num_intf, &dm->intf);
            if (ret)
            {
                LOGE("get lvds failed\n");
                goto DM_FAIL;
            }
      //  }
        break;
    default:
        break;
    }

	*out = dm;
    return 0;

DM_FAIL:
    if (dm->dp)
        free(dm->dp);
    if (dm->dc)
        free(dm->dc);
    if (dm->intf)
        free(dm->intf);
    if (dm)
        free(dm);
    return ret;
}

static int get_wrap_id(display_handle *handle)
{
    int dc_id;

    switch (handle->res->dc_res) {
        case DC_RES_DC1:
        case DC_RES_DC1_DC2:
            dc_id = 0; break;
        case DC_RES_DC2:
            dc_id = 1; break;
        case DC_RES_DC3:
        case DC_RES_DC3_DC4:
            dc_id = 2; break;
        case DC_RES_DC4:
            dc_id = 3; break;
        case DC_RES_DC5:
            dc_id = 4; break;
        default:
            dc_id = 0; break;
    }

    return dc_pll[dc_id];
}


void hal_sdm_set_pll_clk(display_handle *handle, int clk)
{
#if WITH_HAL_MODULE_HELPER_HAL
    struct {
        int idx;
        int clk_id;
        int clean_ref1;
        int clean_ref2;
    } idx_to_clkid[] = {
        {DSP_CLK_PLL_LVDS1, CLK_ID_PLL_LVDS1_DIVA, CLK_ID_PLL_LVDS1_ROOT, CLK_ID_PLL_LVDS1_DIVB},
        {DSP_CLK_PLL_LVDS2, CLK_ID_PLL_LVDS2_DIVA, CLK_ID_PLL_LVDS2_ROOT, CLK_ID_PLL_LVDS2_DIVB},
        {DSP_CLK_PLL_LVDS3, CLK_ID_PLL_LVDS3_DIVA, CLK_ID_PLL_LVDS3_ROOT, CLK_ID_PLL_LVDS3_DIVB},
        {DSP_CLK_PLL_LVDS4, CLK_ID_PLL_LVDS4_DIVA, CLK_ID_PLL_LVDS4_ROOT, CLK_ID_PLL_LVDS4_DIVB},
        {-1, 0, 0, 0},
    };

    int ret = 0;
    int pll_id = 0;
    uint8_t j = 0;

    if (!handle->res && (handle->res->dc_res < 0))
        return;

    for (j = 0; idx_to_clkid[j].idx != -1; j++) {
        pll_id = get_wrap_id(handle);

        if (pll_id  == idx_to_clkid[j].idx)
            break;
    }

    if (idx_to_clkid[j].idx == -1) {
        dprintf(CRITICAL, "disp pll cfg -- get clk_id by idx failed.\n");
        return;
    }

    ret = res_clk_request(PER_ID_DISP, idx_to_clkid[j].clean_ref1, 0);
    if (ret) {
        dprintf(CRITICAL, "disp pll cfg -- clean ref1 failed.\n");
        return;
    }

    ret = res_clk_request(PER_ID_DISP, idx_to_clkid[j].clean_ref2, 0);
    if (ret) {
        dprintf(CRITICAL, "disp pll cfg -- clean ref2 failed.\n");
        return;
    }

    ret = res_clk_request(PER_ID_DISP, idx_to_clkid[j].clk_id, clk);
    if (ret) {
        dprintf(CRITICAL, "disp pll cfg -- set clk failed.\n");
        return;
    }
#endif
}

int hal_sdm_panel_connect(display_handle *handle, int sub_id, struct sdm_panel *panel)
{
    return disp_panel_connect(handle, sub_id, panel) == true? 0: -1;
}

int hal_sdm_panel_disconnect(display_handle *handle, int sub_id, struct sdm_panel *panel)
{
    return disp_panel_disconnect(handle, sub_id, panel);
}
int hal_display_connect_all()
{
    unsigned int i;
    int count = 0;

#ifdef _SYSTEM_CONFIG
    if (get_cfg_info())
        return -1;
#else
    res_count = ARRAY_SIZE(g_display_res);
#endif

    disp_link_init(dc_pll, g_display_res, res_count);

    /*init display_num to zero*/
    g_display_num = 0;
    for (i = 0; i < res_count; i++)
    {
        struct display_resource *res = g_display_res + i;
        if (hal_display_handle_install_resource(res))
        {
            LOGE("Error: probe display %d-%d failed\n", i, res->type);
            continue;
        }
        count++;
    }
    g_display_num = count;

    return 0;
}

int hal_display_num()
{
    return g_display_num;
}

display_handle *hal_get_display_handle(enum DISPLAY_TYPE display)
{
	int i;
	for (i = 0; i < DISPLAY_TYPE_MAX;i++) {
		display_handle *handle = &g_instances[i];
		if (handle->display_id == display)
			return handle;
	}
    // return display < DISPLAY_TYPE_MAX ? &g_instances[display] : NULL;
    return NULL;
}

display_handle *hal_get_display_handle_by_index(uint32_t index)
{
    if (index < DISPLAY_TYPE_MAX)
        return &g_instances[index];

    return NULL;
}

int hal_sdm_callback_vsync_register(display_handle *handle, CALLBACK_EVENT_T func)
{
    if (handle->display_id != -1) {
        handle->vsync = func;
    }
    return 0;
}

int hal_display_handle_install_resource(struct display_resource *res)
{
    static uint8_t index = 0;
    if (res->type > DISPLAY_TYPE_MAX || res->type < DISPLAY_TYPE_INVALID)
    {
        LOGE("Error: invalid display type: %d", res->type);
        return -1;
    }

    if (res->type == DISPLAY_TYPE_INVALID)
    {
        LOGD("display may not choosed");
        return -2;
    }

    display_handle *handle = &g_instances[index++];
    handle->res = res;
    handle->display_id = res->type;//billy need change name.
    handle->name = res->name;
    handle->hotplug = NULL;
    handle->vsync = NULL;
    handle->inited = false;
    handle->panel_post_end_done = false;
    return install_resource(res, &handle->dm);
}

int hal_display_create_handle(display_handle **handle, enum DISPLAY_TYPE display)
{
    if (hal_display_num() == 0)
    {
        hal_display_connect_all();
    }
    if (hal_display_num())
        *handle = hal_get_display_handle(display);
    else
    {
        LOGE("can not found any display\n");
        *handle = NULL;
        return -1;
    }

    return 0;
}

void hal_display_release_handle(display_handle *handle)
{
    LOGD("%d:%s released\n", handle->display_id, handle->name);
}

int hal_sdm_init(display_handle *handle)
{
    return disp_init(handle);
}

int hal_sdm_deinit(display_handle *handle)
{
    return disp_uninit(handle);
}

int hal_sdm_clear_layers(display_handle *handle, u8 mask, u8 zorder)
{
    return disp_clear_layers(handle, mask, zorder);
}

int hal_sdm_post_config(display_handle *handle, struct sdm_post_config *post)
{
    /*For sis funciton*/
    if (handle->panels[handle->found_panel]->rtos_screen) {
        for (size_t i = 0; i < post->n_bufs; i++)
            if (post->bufs[i].layer_en) {
                post->bufs[i].dst.x += handle->panels[handle->found_panel]->rtos_screen->pos_x;
                post->bufs[i].dst.y += handle->panels[handle->found_panel]->rtos_screen->pos_y;
            }
    }

    return disp_post_config(handle, post);
}

int hal_sdm_pq_config(display_handle *handle, struct sdm_pq_params *pq)
{
    return disp_pq_config(handle, pq);
}
