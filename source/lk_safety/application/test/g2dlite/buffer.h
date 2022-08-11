#ifndef __UTIL_FORMAT_H
#define __UTIL_FORMAT_H

struct color_rgb24 {
    unsigned int value:24;
} __attribute__((__packed__));


struct util_color_component {
    unsigned int length;
    unsigned int offset;
};

struct util_rgb_info {
    struct util_color_component red;
    struct util_color_component green;
    struct util_color_component blue;
    struct util_color_component alpha;
};

struct util_format_info {
    uint32_t format;
    const char *name;
    const struct util_rgb_info rgb;
};

struct bo {
    int fmt;
    int src_width;
    int src_height;
    int dst_width;
    int dst_height;
    void * addr[4];
    int bpp;
    int stride;
};

const struct util_format_info *util_format_info_find(uint32_t format);

void fill_smpte(const struct util_format_info *info, void *planes[3],
               unsigned int width, unsigned int height,
               unsigned int stride);

int bo_init(struct bo *bo);
void bo_free(struct bo *bo);
void dshow_post(int display_id, struct bo *bo);

#endif //__UTIL_FORMAT_H

