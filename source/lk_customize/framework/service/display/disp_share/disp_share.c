#include <stdlib.h>
#include <g2dlite_api.h>
#include <res.h>
#include <res_loader.h>

#include "sdm_display.h"
#include "disp_data_type.h"
#include "display_service.h"
#include "display_share.h"

#define MASK_PATH "early_app/dc_sharing/dc_share_mask.bin"

double sqrt(double number)
{
    if(number<=0)return 0;

    double i = 1;
    double j = number/i;
    while((i<j?j-i:i-j)>1e-9)
    {
        i = (i+j)/2;
        j = number/i;
    }
    return i;
}

double pow(double base , unsigned int exponent)
{
    if (exponent == 0) {
        return 1;
    } else if (exponent == 1) {
        return base;
    }

    double result = pow(base, exponent >> 1);
    result *= result;
    if ((exponent & 0x1) == 1) {
        result *= base;
    }
    return result;
}

int generateAlphaTemplate(unsigned char* data, int w, int h, int c, int mode) {
    if (NULL == data) {
        return -1;
    }
    //reset all 0xff
    memset(data, 0xff, w*h*c);
    if (mode == 0) {//rectangle (720,120,480,480)
        int a_x = 700;
        int a_y = 100;
        int a_w = 520;
        int a_h = 520;
        unsigned int gradient_px = 50;

        //alpha bit
        for (int j=0; j<h; j++) {
            for (int i=0; i<w; i++) {
                if (j < a_y
                    || j > (a_y+a_h)
                    || i < a_x
                    || i > (a_x+a_w)
                    ) {
                    data[j*w*c + i*c + 3] = 0x00;
                } else {
                    unsigned int dx0 = abs(i-a_x);
                    unsigned int dx1 = abs(i-(a_x + a_w));
                    unsigned int dy0 = abs(j-a_y);
                    unsigned int dy1 = abs(j-(a_y + a_h));

                    if (dx0 < gradient_px
                        || dx1 < gradient_px
                        || dy0 < gradient_px
                        || dy1 < gradient_px) {

                        unsigned int min_g = MIN(dx0, dx1);
                        min_g = MIN(min_g, dy0);
                        min_g = MIN(min_g, dy1);

                        int da = (i+j)%2 == 0 ? 5 : -5;
                        if (min_g == 0) {
                            da = 0;
                        }
                        data[j*w*c + i*c + 3] = abs(min_g*5 + da);
                    }
                }
            }
        }
    }
    else if (mode == 1) {//full screen except two meter
        int cx1 = 404;
        int cy1 = 375;
        int cx2 = 1516;
        int cy2 = 375;
        int radis = 300;
        int gap = 10;

        //alpha bit
        for (int j=0; j<h; j++) {
            for (int i=0; i<w; i++) {
                if (j < (cy1-radis-gap)
                    || j > (cy1+radis+gap)
                    || i < (cx1-radis-gap)
                    || i > (cx2+radis+gap)
                    ) {
                    data[j*w*c + i*c + 3] = 0xff;
                } else {
                    double r = 0;
                    if (i < w/2) {
                        r = sqrt(pow(i-cx1, 2)+pow(j-cy1, 2));
                    } else {
                        r = sqrt(pow(i-cx2, 2)+pow(j-cy2, 2));
                    }

                    if (r < radis) {
                        data[j*w*c + i*c + 3] = 0x00;
                    } else if (r >=radis && r <= radis+gap) {
                        data[j*w*c + i*c + 3] = 255.0f*(r-radis)/(gap+1);
                    } else {
                        data[j*w*c + i*c + 3] = 0xff;
                    }
                }
            }
        }
    }
    else if (mode == 2) {//full screen
        int cx1 = 400;
        int cy1 = 360;
        int cx2 = 1520;
        int cy2 = 360;
        int radis = 260;

        //alpha bit
        for (int j=0; j<h; j++) {
            for (int i=0; i<w; i++) {
                if (j >= cy1-radis
                    && j <= cy1+radis
                    && i >= cx1
                    && i <= cx2) {
                    data[j*w*c + i*c + 3] = 0xff;
                }
                else if (j < cy1-radis
                    && i >= cx1
                    && i <= cx2) {
                    data[j*w*c + i*c + 3] = 0xff*j/(cy1-radis);
                }
                else if (j > cy1+radis
                    && i >= cx1
                    && i <= cx2) {
                    data[j*w*c + i*c + 3] = 0xff*(h-j)/(h-(cy1+radis));
                }
                else {
                    double r = 0;
                    if (i < cx1) {
                        r = sqrt(pow(i-cx1, 2)+pow(j-cy1, 2));
                    } else {
                        r = sqrt(pow(i-cx2, 2)+pow(j-cy2, 2));
                    }

                    if (r <= radis) {
                        data[j*w*c + i*c + 3] = 0xff;
                    }
                    else if ((r-radis) > (cy1-radis)) {
                        data[j*w*c + i*c + 3] = 0x00;
                    }
                    else {
                        data[j*w*c + i*c + 3] = 255.0f*((cy1-radis)-(r-radis))/(cy1-radis);
                    }
                }
            }
        }
    }
    return 0;
}

static int get_format(int drm_format) {
    switch (drm_format) {
        case DRM_FORMAT_XRGB8888:
        case DRM_FORMAT_ARGB8888:
            return COLOR_ABGR8888;
        case DRM_FORMAT_XBGR8888:
        case DRM_FORMAT_ABGR8888:
            return COLOR_ARGB8888;
        // case DRM_FORMAT_RGBX8888:
        // case DRM_FORMAT_RGBA8888:
        //  return COLOR_RGBA8888;
        case DRM_FORMAT_BGRX8888:
        case DRM_FORMAT_BGRA8888:
            return COLOR_BGRA8888;
        default:
            return COLOR_ARGB8888;
    }
}

static int fill_frame_info(struct sdm_buffer *buf, struct disp_frame_info *fi)
{

    buf->addr[0] = ap2p(fi->addr_l);
    buf->alpha = 0xff;
    buf->alpha_en = 1;
    buf->ckey = 0;
    buf->ckey_en = 0;
    buf->fmt = get_format(fi->format);
    buf->layer_en = 1;
    buf->src.x = 0;
    buf->src.y = 0;
    buf->src.w = fi->width;
    buf->src.h = fi->height;

    buf->start.x = 0;
    buf->start.y = 0;
    buf->start.w = buf->src.w;
    buf->start.h = buf->src.h;
    buf->src_stride[0] = fi->pitch;

    // dc do not support scaling.
    buf->dst.x = fi->pos_x;
    buf->dst.y = fi->pos_y;
    buf->dst.w = fi->width;
    buf->dst.h = fi->height;
    buf->z_order = 0;
    buf->layer = 0;
    buf->layer_en = 1;

    return 0;
}

/* display ioc callback implement */
int disp_ioctl_set_frameinfo(struct dc_share *dc_s, void *arg)
{
    int i;
    int ret = 0;

    struct sdm_buffer *buf;
    struct sdm_post_config *post = NULL;
    struct display_server *disp = &dc_s->display_server;
    struct disp_frame_info *fi = (struct disp_frame_info *) arg;

    /*swap buffer*/
    i = (++disp->on_screen) % MAX_BUFFERS;
    disp->on_screen = i;
    post = &dc_s->post_configs[i];

    fill_frame_info(dc_s->post_configs[i].bufs, fi);

    return ret;
}

int sdm_post_init(struct dc_share *dc_s)
{
    int i;

    dc_s->display_server.on_screen = 0;

    for (i = 0; i < MAX_BUFFERS; i++) {
        memset(&dc_s->post_configs[i], 0, sizeof(struct sdm_post_config));
        memset(&dc_s->bufs[i], 0, sizeof(struct sdm_buffer));

        dc_s->post_configs[i].n_bufs = 1;
        dc_s->post_configs[i].bufs = &dc_s->bufs[i];
    }
    return 0;
}

void g2d_mask_blending(struct dc_share *dc_s)
{
    bool ret;
    int i;
    struct g2dlite_input input;
    struct display_server *disp = &dc_s->display_server;
    struct sdm_buffer *buf = &dc_s->post_configs[disp->on_screen].bufs[0];
    static int current_index = 0;

    memset(&input, 0, sizeof(struct g2dlite_input));

    if (!dc_s->g2d || !dc_s->mask_base || !dc_s->outbufs[current_index]) {
        dprintf(CRITICAL, "%s g2d is null\n", __func__);
        return;
    }

    input.layer_num = 2;
    input.pd_info.en = 1;
    input.pd_info.zorder = 0;
    input.pd_info.mode = MULTIPLY;
    input.pd_info.alpha_need = 1;

    // fill frame
    //printf("displaysharing %d,%d,%d,%d,%d\n", buf->src.w, buf->src.h, buf->dst.w, buf->dst.h,buf->src_stride[0]);
    i = 0;
    input.layer[i].layer = 1;
    input.layer[i].layer_en = 1;

    input.layer[i].fmt = buf->fmt;
    input.layer[i].src.x = 0;
    input.layer[i].src.y = 0;
    input.layer[i].src.w = buf->src.w;
    input.layer[i].src.h = buf->src.h;

    input.layer[i].addr[0] = buf->addr[0];
    input.layer[i].src_stride[0] = buf->src_stride[0];
    input.layer[i].dst.x = 0;
    input.layer[i].dst.y = 0;
    input.layer[i].dst.w = buf->dst.w;
    input.layer[i].dst.h = buf->dst.h;
    input.layer[i].ckey.en = 0;
    input.layer[i].pd_type = PD_DST;
    input.layer[i].blend = BLEND_PIXEL_COVERAGE;
    // fill mask
    i = 1;

    input.layer[i].layer = 0;
    input.layer[i].layer_en = 1;

    input.layer[i].fmt = COLOR_ARGB8888;
    input.layer[i].src.x = 0;
    input.layer[i].src.y = 0;
    input.layer[i].src.w = SHARE_MASK_WIDTH;
    input.layer[i].src.h = SHARE_MASK_HEIGHT;

    input.layer[i].addr[0] = (unsigned long)dc_s->mask_base;
    input.layer[i].src_stride[0] = SHARE_MASK_WIDTH*4;
    input.layer[i].dst.x = 0;
    input.layer[i].dst.y = 0;
    input.layer[i].dst.w = buf->dst.w;
    input.layer[i].dst.h = buf->dst.h;
    input.layer[i].ckey.en = 0;
    input.layer[i].pd_type = PD_SRC;
    input.layer[i].blend = BLEND_PIXEL_COVERAGE;

    if (++current_index >= MAX_BUFFERS) {
        current_index = 0;
    }

    input.output.width = buf->dst.w;
    input.output.height = buf->dst.h;
    input.output.fmt = COLOR_ARGB8888;
    input.output.addr[0] = (unsigned long)dc_s->outbufs[current_index];
    input.output.stride[0] = buf->src_stride[0];
    input.output.rotation = 0;
    hal_g2dlite_blend(dc_s->g2d, &input);


    // change the disp post configuration.
    buf->fmt = COLOR_ABGR8888;
    buf->addr[0] = (unsigned )input.output.addr[0];
    buf->alpha_en = 0;
}

int g2d_handle_init(struct dc_share *dc_s)
{
    int ret = 0;
    int i = 0, j = 0;

    if (!dc_s->g2d) {
        ret = hal_g2dlite_creat_handle(&dc_s->g2d, RES_G2D_G2D2);
        if (!ret) {
            LOGD("g2dlite creat handle failed\n");
            ret = -1;
            goto get_handle_err;
        }

        hal_g2dlite_init(&dc_s->g2d);

        /*fill the mask*/
        dc_s->mask_base = memalign(4096, SHARE_MASK_WIDTH * SHARE_MASK_HEIGHT * 4);
        if (!dc_s->mask_base) {
            LOGD("alloc mask_base err!\n");
            ret = -1;
            goto get_handle_err;
        }

        for (i = 0; i < MAX_BUFFERS; i++) {
            dc_s->outbufs[i] = memalign(4096, SHARE_MASK_WIDTH * SHARE_MASK_HEIGHT * 4);
            if (!dc_s->outbufs[i]) {
                LOGD("alloc outbufs err!\n");
                ret = -1;
                goto alloc_err;
            }
        }
    }

    ret = res_load(MASK_PATH, dc_s->mask_base, SHARE_MASK_WIDTH * SHARE_MASK_HEIGHT * 4, 0);
    if (ret < 0) {
        LOGE("res load mask err! use CPU generate alpha.\n");
        generateAlphaTemplate((unsigned char *)dc_s->mask_base, SHARE_MASK_WIDTH, SHARE_MASK_HEIGHT, 4, 1);
    }

    return 0;

alloc_err:

    i--;

    while (i >= 0) {
        free(dc_s->outbufs[i--]);
    }

    free(dc_s->mask_base);
get_handle_err:
    return ret;
}

