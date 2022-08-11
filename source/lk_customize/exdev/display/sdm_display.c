#include "sdm_display.h"
#include "disp_data_type.h"
#include <list.h>
#include <stdlib.h>
#ifdef SUPPORT_LVGL_GUI
#include "lvgl_gui.h"
#endif
#include <debug.h>
#ifdef SUPPORT_BACKLIGHT_SVC
#include <bl_conf.h>
#endif

static int is_inited = false;

struct list_node g_disp_list;
struct list_node g_group_disp_list;

extern void backlight_enable(void);

void sdm_display_change_clk(display_handle *handle, int fps)
{
    uint32_t h_total = 0;
    uint32_t v_total = 0;
    uint8_t  dual_mode = 0;

    struct sdm_panel *panel = handle->panels[handle->found_panel];

    dual_mode = (handle->res->lvds_mode == LVDS_MODE_DUAL) ? 2 : 1;

    h_total = panel->display_timing->hactive + panel->display_timing->hback_porch +
              panel->display_timing->hfront_porch + panel->display_timing->hsync_len;
    v_total = panel->display_timing->vactive + panel->display_timing->vback_porch +
              panel->display_timing->vfront_porch + panel->display_timing->vsync_len;
    handle->res->clk = h_total * v_total * fps * 7 / dual_mode;

    hal_sdm_set_pll_clk(handle, handle->res->clk);

    if (panel->if_type == IF_TYPE_DSI) {
        panel->mipi->phy_freq = h_total * v_total * fps * panel->mipi->video_bus_width * 1.1 /
                                (panel->mipi->lane_number * 1000);

        dprintf(0, "clk:%d phy_freq:%d\n", handle->res->clk, panel->mipi->phy_freq);
    }
    else
        dprintf(0, "clk:%d\n", handle->res->clk);
}

int sdm_display_handle_init(enum DISPLAY_TYPE display, struct sdm_panel *panels[], int n_panels) {
    int ret;
    int found_panel = -1;
    int num_panel = 0;
    int i;

    display_handle *handle;
    hal_display_create_handle(&handle, display);
    if (!handle) {
        LOGE("create %d handle failed\n", display);
        return -1;
    }

    //LOGD("check: display  %d handle get success\n", display);
    num_panel = sdm_check_display_panel_num(handle);

    for (i = 0; i < num_panel; i++) {
        found_panel = sdm_panel_probe(handle, i, panels, n_panels);
        if (found_panel < 0) {
            LOGE("get panel failed\n");
            ret = -2;
            goto PANEL_FAIL;
        }

        handle->panels[i] = panels[found_panel];
        handle->found_panel = found_panel;
        handle->info.width = handle->panels[i]->display_timing->hactive;
        handle->info.height = handle->panels[i]->display_timing->vactive;
        handle->info.format = 0;
        handle->info.vrefresh = 60;
        if (handle->panels[i]->rtos_screen) {
            handle->info.rtos_screen.height = handle->panels[i]->rtos_screen->height;
            handle->info.rtos_screen.width = handle->panels[i]->rtos_screen->width;
            handle->info.rtos_screen.pos_x = handle->panels[i]->rtos_screen->pos_x;
            handle->info.rtos_screen.pos_y = handle->panels[i]->rtos_screen->pos_y;
        } else {
            handle->info.rtos_screen.height =  handle->info.height;
            handle->info.rtos_screen.width =  handle->info.width;
            handle->info.rtos_screen.pos_x =  0;
            handle->info.rtos_screen.pos_y =  0;
        }
    }

    if (panels[found_panel]->panel_post_begin)
         panels[found_panel]->panel_post_begin();

    sdm_display_change_clk(handle, panels[found_panel]->fps);

    ret = hal_sdm_panel_connect(handle, i ,panels[found_panel]);
    if (ret < 0) {
        LOGE("hal_sdm_panel_connect failed\n");
        goto PANEL_FAIL;
    }

#if (defined(V_DEFAULT) || defined(V9TS_DEFAULT))
    handle->is_need_register_irq = false;
#elif defined(X9U_B)
        handle->is_need_register_irq = true;
#elif defined(X9U_A)
	// hud screen
	if(handle->display_id == 4)
		handle->is_need_register_irq = false;
	else
		handle->is_need_register_irq = true;
#else
    handle->is_need_register_irq = true;
#endif

    if (hal_sdm_init(handle)) {
        LOGE("hal_sdm_init failed\n");
        goto PANEL_FAIL;
    }

    sdm_display_t *d = (sdm_display_t *)malloc(sizeof(sdm_display_t));
    d->id = display;
    d->handle = handle;
    list_add_tail(&g_disp_list, &d->node);
    return 0;

PANEL_FAIL:
    LOGE("TODO: we need uninit panel here\n");
HANDLE_FAIL:
    hal_display_release_handle(handle);
    return ret;
}

int sdm_check_display_panel_num(display_handle *handle) {
    return handle->res->lvds_mode == LVDS_MODE_DUPLICATE? 2: 1;
}

int sdm_init(void) {
    list_initialize(&g_disp_list);
    return 0;
}

struct list_node *sdm_get_display_list(void) {
    return &g_disp_list;
}

int sdm_group_display_init_default(void) {
    struct list_node *head = sdm_get_display_list();
    sdm_display_t *sdm;

    list_for_every_entry(head, sdm, sdm_display_t, node) {
        LOGD("disp->id, disp->handle->display_id (%d, %d)\n",
            sdm->id, sdm->handle->display_id);
        if (sdm->handle) {
            list_add_tail(&g_group_disp_list, &sdm->node);
        }
    }
    return 0;
}

int sdm_group_display_register(sdm_display_t *sdm) {
    if (!sdm->handle) {
        LOGE("register display hanlde is null\n");
        return -1;
    }
    list_add_tail(&g_group_disp_list, &sdm->node);
    return 0;
}

int sdm_group_display_post(struct sdm_post_config *post) {
    struct list_node *head = sdm_get_display_list();
    sdm_display_t *sdm;
    int index = 0;

    size_t len = list_length(&g_group_disp_list);
    if (post->n_bufs > len) {
        LOGE("have %d buffer to post, but only %d displays work\n", (int)post->n_bufs, (int)len);
        return -1;
    }

    index = 0;
    list_for_every_entry(head, sdm, sdm_display_t, node) {
        struct sdm_post_config p;
        p.bufs = post->bufs;
        p.n_bufs = 1;
        p.custom_data_size = post->custom_data_size;
        p.custom_data = post->custom_data;
        sdm_post(sdm->handle, &p);
        index++;
    }

    return 0;
}

void sdm_screen_clear(display_handle *handle)
{
    struct sdm_post_config p;
    struct sdm_buffer bufs[2] = {0};
    int width, height;
    int i;
    void *logo = NULL;

    width = handle->info.width;
    height = handle->info.height;

    p.bufs = bufs;
    p.n_bufs = 2;
    for (i = 0; i < 2;i++) {
        bufs[i].layer = i;
        bufs[i].layer_en = 0;
        bufs[i].fmt = COLOR_RGB565;
        bufs[i].alpha_en = 1;
        bufs[i].alpha = 0x0;
        bufs[i].ckey = 0;
        bufs[i].ckey_en = 0;
        bufs[i].src.x = 0;
        bufs[i].src.y = 0;
        bufs[i].src.w = width;
        bufs[i].src.h = height;

        bufs[i].start.x = 0;
        bufs[i].start.y = 0;
        bufs[i].start.w = width;
        bufs[i].start.h = height;
        bufs[i].addr[0] = (unsigned long)logo;
        bufs[i].src_stride[0] = width * 2;

        bufs[i].dst.x = 0;
        bufs[i].dst.y = 0;
        bufs[i].dst.w = width;
        bufs[i].dst.h = height;
        bufs[i].z_order = i;
    }
    sdm_post(handle, &p);
}

bool sdm_display_is_inited(void)
{
    return is_inited;
}

void sdm_display_init(void)
{
    int i;
    int num_display;

    // connect all display
    hal_display_connect_all();
    num_display = hal_display_num();

    sdm_init();

    LOGD("display num is %d\n", num_display);
    for(i = 0;i< num_display;i++) {
        display_handle *handle = hal_get_display_handle_by_index(i);
        if (handle->res->gui_enable == PANEL_NOT_NEED_INIT) {
            continue;
        }
        if (handle->dm && handle->dm->dc) {
            //LOGD("init panel for display [%d]-- index [%d]\n", handle->display_id, i);
            int n_panels = ARRAY_SIZE(registered_panels[handle->display_id]);
            sdm_display_handle_init(handle->display_id, registered_panels[handle->display_id], n_panels);
            sdm_screen_clear(handle);
#ifdef SUPPORT_BACKLIGHT_SVC
            //todo add backlight init
            backlight_service_init(handle->display_id);
#endif
        }
    }
    LOGD("found %u displays connected\n", list_length(sdm_get_display_list()));

#ifdef SUPPORT_LVGL_GUI
    lvgl_init();
#endif
    is_inited = true;
}
