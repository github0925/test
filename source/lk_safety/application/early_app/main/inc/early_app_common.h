#ifndef _APP_COMMON_H
#define _APP_COMMON_H

#include <sdm_display.h>

#define MAX_DISPLAY_NUM    3

static inline void get_screen_attr(sdm_display_t* display,uint32_t* width, uint32_t* height)
{
    if(display)
    {
        *width = display->handle->info.width;
        *height = display->handle->info.height;
    }
}

static inline sdm_display_t* get_disp_handle(enum DISPLAY_SCREEN screen_id)
{
    struct list_node* disp_node = sdm_get_display_list();
    sdm_display_t* disp = NULL;
    disp_node = disp_node->next;

    for(int i=0;i<list_length(sdm_get_display_list());i++)
    {
        disp = containerof(disp_node,sdm_display_t,node);
        if(disp->handle->display_id == screen_id) return disp;
        disp_node = disp_node->next;
    }

    return NULL;
}


#endif
