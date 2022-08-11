#include <lk_wrapper.h>

struct cluster_fg {
    float fps;
    float kmph;
    float rpm;
    uint32_t * buf;
};

struct cluster_bg {
    uint32_t * buf;
};

struct cluster_res {
    uint8_t * needles_src;
    uint8_t * numbers_src;
    uint8_t * bg_src;
    uint8_t * nav_src;
};

struct cluster_ui {
    struct cluster_res res;
    struct cluster_fg fg[2];
    struct cluster_bg bg[3];
    struct cluster_fg * current_fg;
    struct cluster_fg * next_fg;
    struct cluster_bg * current_bg;
    struct cluster_bg * next_bg;
    struct cluster_bg * static_bg;
    bool nav_en;
};

void cluster_bg_init(struct cluster_ui * ui, uint32_t nav_en);
void cluster_bg_update(struct cluster_ui * ui, uint32_t nav_en, uint32_t * nav_source, uint32_t div, uint32_t index);
void cluster_fg_init(struct cluster_ui * ui, float fps, float kmph, float rpm);
void cluster_fg_update(struct cluster_ui * ui, float fps, float kmph, float rpm);
uint32_t * cluster_ui_get_fg(struct cluster_ui * ui);
uint32_t * cluster_ui_get_bg(struct cluster_ui * ui);
