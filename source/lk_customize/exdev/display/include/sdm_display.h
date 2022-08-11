/**
 * @file lv_port_disp_templ.h
 *
 */


#ifndef SDM_DISPLAY_H
#define SDM_DISPLAY_H

#ifdef __cplusplus
extern "C" {
#endif
#include "disp_hal.h"
#include "disp_panels.h"
#include <list.h>

typedef struct {
    struct list_node node;
    int id;
    display_handle *handle;
} sdm_display_t;
int sdm_check_display_panel_num(display_handle *handle);
int sdm_display_handle_init(enum DISPLAY_TYPE display, struct sdm_panel *panels[], int n_panels);
int sdm_init(void);

/**
 *@mask:It is a flag, intend to tell drv which layer we want to close.
 *CLEAR_SPIPE_LAYER, CLEAR_GPIPE_LAYER, CLEAR_ALL_LAYERS.
 *@zorder:The layer's current zorder.
 */
static inline int sdm_clear_layer(display_handle *handle, u8 mask, u8 zorder)
{
    return hal_sdm_clear_layers(handle, mask, zorder);
}

static inline int sdm_clear_display(display_handle *handle)
{
    return hal_sdm_clear_layers(handle, CLEAR_ALL_LAYERS, 0);
}

static inline int sdm_post(display_handle *handle, struct sdm_post_config *post)
{
    int ret;
    ret = hal_sdm_post_config(handle, post);

    if (handle->panels[handle->found_panel]->panel_post_end) {
        if (!handle->panel_post_end_done) {
            handle->panels[handle->found_panel]->panel_post_end();
            handle->panel_post_end_done = true;
        }
    }

    return ret;
}

static inline int sdm_callback_vsync_register(display_handle *handle, CALLBACK_EVENT_T func)
{
    return hal_sdm_callback_vsync_register(handle, func);
}

static inline int sdm_pq_set(display_handle *handle, struct sdm_pq_params *pq)
{
    return hal_sdm_pq_config(handle, pq);
}
struct list_node *sdm_get_display_list(void);

int sdm_group_display_init_default(void);
int sdm_group_display_register(sdm_display_t *sdm);
int sdm_group_display_post(struct sdm_post_config *post);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*SDM_DISPLAY_H*/

