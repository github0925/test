#ifndef _LVGL_LCD_CONFIG_H
#define _LVGL_LCD_CONFIG_H

/* lvgl include */
#include "lvgl_gui.h"
#include "sdm_display.h"

#ifdef __cplusplus
extern "C"
{
#endif

lv_disp_t *lvgl_lcd_display_init(sdm_display_t *hw);

#ifdef __cplusplus
}
#endif

#endif /* _LVGL_LCD_CONFIG_H */
