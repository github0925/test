/**
 * @file lv_gpu_sdrv_dma2d.h
 *
 */

#ifndef LV_GPU_SEMIDRIVE_DMA2D_H
#define LV_GPU_SEMIDRIVE_DMA2D_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "lvgl.h"
#include <kernel/thread.h>
#include <platform.h>

/*********************
 *      DEFINES
 *********************/

#define LV_DMA2D_ARGB8888 0
#define LV_DMA2D_RGB888 1
#define LV_DMA2D_RGB565 2
#define LV_DMA2D_ARGB1555 3
#define LV_DMA2D_ARGB4444 4

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * Turn on the peripheral and set output color mode, this only needs to be done once
 */
void lv_gpu_sdrv_dma2d_init(void);

/**
 * Fill an area in the buffer with a color
 * @param buf a buffer which should be filled
 * @param buf_w width of the buffer in pixels
 * @param color fill color
 * @param fill_w width to fill in pixels (<= buf_w)
 * @param fill_h height to fill in pixels
 * @note `buf_w - fill_w` is offset to the next line after fill
 */
void lv_gpu_sdrv_dma2d_fill(lv_color_t * buf, lv_coord_t buf_w, lv_color_t color,
                                lv_coord_t fill_w, lv_coord_t fill_h, lv_opa_t opa);

/**
 * Fill an area in the buffer with a color but take into account a mask which describes the opacity of each pixel
 * @param buf a buffer which should be filled using a mask
 * @param buf_w width of the buffer in pixels
 * @param color fill color
 * @param mask 0..255 values describing the opacity of the corresponding pixel. It's width is `fill_w`
 * @param opa overall opacity. 255 in `mask` should mean this opacity.
 * @param fill_w width to fill in pixels (<= buf_w)
 * @param fill_h height to fill in pixels
 * @note `buf_w - fill_w` is offset to the next line after fill
 */
void lv_gpu_sdrv_dma2d_fill_mask(lv_color_t * buf, lv_coord_t buf_w, lv_color_t color, const lv_opa_t * mask,
                                  lv_opa_t opa, lv_coord_t fill_w, lv_coord_t fill_h);

/**
 * Copy a map (typically RGB image) to a buffer
 * @param buf a buffer where map should be copied
 * @param buf_w width of the buffer in pixels
 * @param map an "image" to copy
 * @param map_w width of the map in pixels
 * @param copy_w width of the area to copy in pixels (<= buf_w)
 * @param copy_h height of the area to copy in pixels
 * @note `map_w - fill_w` is offset to the next line after copy
 */
int lv_gpu_sdrv_g2d_copy(lv_color_t * buf, lv_coord_t buf_w, const lv_color_t * map, lv_coord_t map_w,
                             lv_coord_t copy_w, lv_coord_t copy_h);
/**
 * Blend a map (e.g. ARGB image or RGB image with opacity) to a buffer
 * @param buf a buffer where `map` should be copied
 * @param buf_w width of the buffer in pixels
 * @param map an "image" to copy
 * @param opa opacity of `map`
 * @param map_w width of the map in pixels
 * @param copy_w width of the area to copy in pixels (<= buf_w)
 * @param copy_h height of the area to copy in pixels
 * @note `map_w - fill_w` is offset to the next line after copy
 */
int lv_gpu_sdrv_g2d_blend(const lv_area_t * disp_area, lv_color_t * disp_buf,
                            const lv_area_t * draw_area,
                            const lv_area_t * map_area, const lv_color_t * map_buf, lv_opa_t opa,
                            const lv_opa_t * mask, lv_draw_mask_res_t mask_res);

int lv_gpu_sdrv_g2d_blend_nocache(lv_color_t * dst_buf, uint16_t dst_stride,
                            const lv_color_t * fg_buf, uint16_t fg_stride,
                            const lv_color_t * bg_buf, uint16_t bg_stride,
                            const lv_area_t * dst_area,
                            const lv_area_t * fg_src_area, const lv_area_t * fg_dst_area,
                            const lv_area_t * bg_src_area, const lv_area_t * bg_dst_area);
int lv_gpu_sdrv_g2d_blend_sharing(lv_color_t * dst_buf, uint16_t dst_stride,
                            const lv_color_t * fg_buf, uint16_t fg_stride,
                            const lv_color_t * bg_buf, uint16_t bg_stride,
                            const lv_area_t * dst_area,
                            const lv_area_t * fg_src_area, const lv_area_t * fg_dst_area,
                            const lv_area_t * bg_src_area, const lv_area_t * bg_dst_area);
int lv_gpu_sdrv_g2d_blend_scale(lv_color_t * dst_buf, uint16_t dst_stride,
                            const lv_color_t * fg_buf, uint16_t fg_stride,
                            const lv_color_t * bg_buf, uint16_t bg_stride,
                            const lv_area_t * dst_area,
                            const lv_area_t * fg_src_area, const lv_area_t * fg_dst_area,
                            const lv_area_t * bg_src_area, const lv_area_t * bg_dst_area);
int lv_gpu_sdrv_g2d_blend_sharing_scale(lv_color_t * dst_buf, uint16_t dst_stride,
                            const lv_color_t * fg_buf, uint16_t fg_stride,
                            const lv_color_t * bg_buf, uint16_t bg_stride,
                            const lv_area_t * dst_area,
                            const lv_area_t * fg_src_area, const lv_area_t * fg_dst_area,
                            const lv_area_t * bg_src_area, const lv_area_t * bg_dst_area);
/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*lv_gpu_sdrv_DMA2D_H*/
