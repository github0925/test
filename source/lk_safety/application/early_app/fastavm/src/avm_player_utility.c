#include "avm_player_utility.h"

void avm_get_screen_attr(sdm_display_t* display,uint32_t* width, uint32_t* height)
{
    if(display)
    {
        *width = display->handle->info.width;
        *height = display->handle->info.height;
    }
}

void avm_fill_sdm_buf(struct sdm_buffer* sdm_buffer,unsigned long bufY, int stride)
{
    sdm_buffer->addr[0] = bufY;
    sdm_buffer->src_stride[0] = stride;
}

void avm_crop_sdm_buf(struct sdm_buffer* sdm_buffer,int px,int py, int width, int height)
{
    sdm_buffer->start.x = px;
    sdm_buffer->start.y = py;
    sdm_buffer->start.w = width;
    sdm_buffer->start.h = height;
    sdm_buffer->src.x = px;
    sdm_buffer->src.y = py;
    sdm_buffer->src.w = width;
    sdm_buffer->src.h = height;
}

void avm_map_sdm_buf(struct sdm_buffer* sdm_buffer,int px,int py, int width, int height)
{
    sdm_buffer->dst.x = px;
    sdm_buffer->dst.y = py;
    sdm_buffer->dst.w = width;
    sdm_buffer->dst.h = height;
}
