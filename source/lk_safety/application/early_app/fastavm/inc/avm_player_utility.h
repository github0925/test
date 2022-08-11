#include <lk_wrapper.h>
#include <sdm_display.h>
#include <disp_data_type.h>

struct list_node *sdm_get_display_list(void);

void *memset(void *s, int c, size_t count);
void *memcpy (void *dest, const void *src, unsigned n);

#define DISPLAY_AVM_TEMPLATE { \
    0,/*layer*/\
    0,/*layer_dirty*/\
    1,/*layer_en*/\
    COLOR_RGB888,/*fmt*/\
    {0,0,0,0},/*x,y,w,h src */ \
    {0x0,0x0,0x0,0x0},/*y,u,v,a*/ \
    {0,0,0,0},/*stride*/ \
    {0,0,0,0},/*start*/ \
    {0,0,0,0},/*dest*/ \
    0,/*ckey_en*/\
    0,/*ckey*/\
    1,/*alpha_en*/\
    0xff,/*alpha*/\
    0,/*z-order*/ \
    0/*security*/\
}

#define DISPLAY_SINGLE_TEMPLATE { \
    1,/*layer*/\
    0,/*layer_dirty*/\
    1,/*layer_en*/\
    COLOR_YUYV,/*fmt*/\
    {0,0,0,0},/*x,y,w,h src */ \
    {0x0,0x0,0x0,0x0},/*y,u,v,a*/ \
    {0,0,0,0},/*stride*/ \
    {0,0,0,0},/*start*/ \
    {0,0,0,0},/*dest*/ \
    0,/*ckey_en*/\
    0,/*ckey*/\
    1,/*alpha_en*/\
    0xff,/*alpha*/\
    1,/*z-order*/ \
    0/*security*/\
}

void avm_get_screen_attr(sdm_display_t* display,uint32_t* width, uint32_t* height);
void avm_fill_sdm_buf(struct sdm_buffer* sdm_buffer,unsigned long bufY, int stride);
void avm_crop_sdm_buf(struct sdm_buffer* sdm_buffer,int px,int py, int width, int height);
void avm_map_sdm_buf(struct sdm_buffer* sdm_buffer,int px,int py, int width, int height);
