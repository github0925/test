
#include "debug.h"
#include "G2dConverter.h"
#include "sdrv_g2d_cfg.h"
#include <unistd.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <cstring>

#define G2D_DEVICE_NAME "/dev/g2d0"

G2dConverter::G2dConverter() {

    struct g2d_capability cap;
    g2dFd = open(G2D_DEVICE_NAME, O_RDWR);

    if (g2dFd < 0)
    {
        LOGE("g2d_open ion_open failed\n");
    }

    int ret = ioctl(g2dFd, G2D_IOCTL_GET_CAPABILITIES, &cap);
    if (ret) {
        LOGE("g2d_open G2D_IOCTL_GET_CAPABILITIES failed\n");
    }
}

G2dConverter::~G2dConverter() {

}

int get_g2d_rotation(int hw_rotation)
{
    switch (hw_rotation) {
        case HW_ROTATION_TYPE_NONE:
        return ROTATION_TYPE_NONE;
        case HW_ROTATION_TYPE_ROT_90:
        return ROTATION_TYPE_ROT_90;
        case HW_ROTATION_TYPE_ROT_180:
        return ROTATION_TYPE_ROT_180;
        case HW_ROTATION_TYPE_ROT_270:
        return ROTATION_TYPE_ROT_270;
        case HW_ROTATION_TYPE_VF_90:
        return ROTATION_TYPE_VF_90;
        case HW_ROTATION_TYPE_HF_90:
        return ROTATION_TYPE_HF_90;
    };
    return 0;
}

int G2dConverter::BlitSingle(const HwBuffer *input, const HwBuffer *output)
{
    struct g2d_input post;
    int ret = 0;

    if (!input || !output) {
        return -1;
    }

    memset(&post, 0, sizeof(struct g2d_input));
    int index = 2;
    ret |= addInputHwBuffer(&post, input, index);
    post.layer_num = 1;
    ret = setOutputHwBuffer(&post, output, true);

    if (ret) {
        LOGE("set HwBuffer parameters failed\n");
        return ret;
    }
    Lock();
    ret = ioctl(g2dFd, G2D_IOCTL_POST_CONFIG, &post);
    if (ret) {
        LOGE("ioctl G2D_IOCTL_POST_CONFIG: ERROR\n");
        return ret;
    }
    unLock();
    return 0;
}

static int g2d_get_format_nplanes(int format)
{
    switch (format) {
        case DRM_FORMAT_NV12:
        case DRM_FORMAT_NV21:
            return 2;
        case DRM_FORMAT_YVU420:
        case DRM_FORMAT_YUV420:
            return 3;
        default:
            return 1;
    }
    return 1;
}

int G2dConverter::FastCopy(const int src_fd, const int dst_fd, size_t data_size)
{
    struct g2d_input input;
    int ret;
    int width, height, stride;
    width = 32;
    stride = width * 4;
    height = (data_size / stride) + ((data_size % stride) ? 1 : 0);
    LOGD("data_size, width, stride, height : (%d, %d, %d, %d) \n",data_size, width, stride, height);
    memset(&input,0x0,sizeof(struct g2d_input));

    input.bg_layer.en = 1;
    input.bg_layer.width = width;
    input.bg_layer.height = height;
    input.bg_layer.astride = stride;
    input.bg_layer.abufs.fd = src_fd;

    input.output.bufs[0].fd = dst_fd;
    input.output.width = width;
    input.output.height = height;
    input.output.stride[0] = stride;

    Lock();
    ret = ioctl(g2dFd, G2D_IOCTL_FAST_COPY, &input);
    if (ret) {
        LOGE("err :G2D_IOCTL_POST_CONFIG  failed \n");
    }
    unLock();
    return ret;
}

int G2dConverter::FillColor(const HwBuffer *hwbuf, uint32_t color_10bit, uint32_t g_alpha)
{
    int ret=0;
    struct g2d_input input;
    memset(&input,0x0,sizeof(struct g2d_input));

    /*alph config*/
    input.bg_layer.en = 1;
    input.bg_layer.color = color_10bit;
    input.bg_layer.g_alpha = g_alpha;

    /*output config*/
    input.output.nplanes = hwbuf->handle.n_planes;
    input.output.width = hwbuf->handle.width;
    input.output.height = hwbuf->handle.height;
    input.output.fmt = hwbuf->handle.format;

    int Bpp = DrmFrame::getBppByFormat(input.output.fmt) / 8;
    for (size_t i = 0; i < input.output.nplanes; i++) {
        input.output.stride[i] = hwbuf->handle.strides[i];
        input.output.offsets[i] = hwbuf->handle.offsets[i];
        input.output.offsets[i] += hwbuf->display.left * Bpp + hwbuf->display.top * input.output.stride[i];
        input.output.bufs[i].fd = hwbuf->handle.fds[i];
    }
    Lock();

    ret = ioctl(g2dFd, G2D_IOCTL_FILL_RECT, &input);
    if (ret) {
        LOGE("err :G2D_IOCTL_FILL_RECT  failed \n");
    }
    unLock();

    return ret;

}

int G2dConverter::Blit(const HwBuffer **inputs, const HwBuffer *output, int layer_cnt)
{
    struct g2d_input post;
    int ret = 0;
    memset(&post, 0, sizeof(struct g2d_input));
    post.layer_num = 0;
    for (int i = 0; i < layer_cnt; i++) {
        ret |= addInputHwBuffer(&post, inputs[i], post.layer_num);
        post.layer_num++;
    }
    ret = setOutputHwBuffer(&post, output, true);

    if (ret) {
        LOGE("set HwBuffer parameters failed\n");
        return ret;
    }
    Lock();
    ret = ioctl(g2dFd, G2D_IOCTL_POST_CONFIG, &post);
    if (ret) {
        LOGE("ioctl G2D_IOCTL_POST_CONFIG: ERROR\n");
        return ret;
    }
    unLock();
    return 0;
}

int G2dConverter::ConvertSingle(const HwBuffer *input, const HwBuffer *output)
{
    struct g2d_input post;
    int ret = 0;

    if (!input || !output) {
        return -1;
    }

    memset(&post, 0, sizeof(struct g2d_input));
    int index = 2;
    ret |= addInputHwBuffer(&post, input, index);
    post.layer_num = 1;

    ret = setOutputHwBuffer(&post, output, false);

    if (ret) {
        LOGE("set HwBuffer parameters failed\n");
        return ret;
    }
    Lock();
    ret = ioctl(g2dFd, G2D_IOCTL_POST_CONFIG, &post);
    if (ret) {
        LOGE("ioctl G2D_IOCTL_POST_CONFIG: ERROR\n");
        return ret;
    }
    unLock();
    return 0;
}

int G2dConverter::Convert(const HwBuffer **inputs, const HwBuffer *output, int layer_cnt)
{
    struct g2d_input post;
    int ret = 0;
    memset(&post, 0, sizeof(struct g2d_input));

    post.layer_num = 0;
    for (int i = 0; i < layer_cnt; i++) {
        ret |= addInputHwBuffer(&post, inputs[i], post.layer_num);
        post.layer_num++;
    }
    ret = setOutputHwBuffer(&post, output, false);

    if (ret) {
        LOGE("set HwBuffer parameters failed\n");
        return ret;
    }
    Lock();
    ret = ioctl(g2dFd, G2D_IOCTL_POST_CONFIG, &post);
    if (ret) {
        LOGE("ioctl G2D_IOCTL_POST_CONFIG: ERROR\n");
        return ret;
    }
    unLock();
    return 0;
}

int G2dConverter::setOutputHwBuffer(struct g2d_input *post, const HwBuffer *hwbuf, bool blit)
{
    struct g2d_output_cfg *out = &post->output;

    if(!post || !hwbuf) {
        LOGE("invalid post or hwcubf pointer");
        return -1;
    }

    if (blit) {
        out->width = hwbuf->display.getWidth();
        out->height = hwbuf->display.getHeight();
    } else {
        out->width = hwbuf->handle.width;
        out->height = hwbuf->handle.height;
    }

    out->fmt = hwbuf->handle.format;
    out->nplanes = hwbuf->handle.n_planes;
    out->rotation = get_g2d_rotation(hwbuf->rotation);
    int Bpp = DrmFrame::getBppByFormat(out->fmt) / 8;
    for (size_t i = 0; i < out->nplanes; i++) {
        out->stride[i] = hwbuf->handle.strides[i];
        out->offsets[i] = hwbuf->handle.offsets[i];
        out->offsets[i] += hwbuf->display.left * Bpp + hwbuf->display.top * out->stride[i];
        out->bufs[i].fd = hwbuf->handle.fds[i];
        DDBG(out->offsets[i]);
        DDBG(out->stride[i]);
    }
    DDBG(out->width);

    return 0;
}

int G2dConverter::addInputHwBuffer(struct g2d_input *post, const HwBuffer *hwbuf, int index)
{
    if (index > 5) {
        LOGE("index %d bigger than 5\n", index);
        return -1;
    }

    struct dpc_layer *layer = &post->layer[post->layer_num];
    layer->index = index;
    layer->enable = 1;
    layer->format = hwbuf->handle.format;
    layer->nplanes = hwbuf->handle.n_planes;
    LOGD("g2d_convert in format: 0x%x,nplanes:%d, layer index = %d\n",layer->format,layer->nplanes, layer->index);
    layer->alpha = hwbuf->alpha;
    layer->blend_mode = hwbuf->blend_mode;
    layer->rotation = get_g2d_rotation(hwbuf->rotation);
    layer->zpos = hwbuf->zorder? hwbuf->zorder: index;
    layer->xfbc = 0;
    layer->modifier = hwbuf->handle.modifiers[0];
    layer->width = hwbuf->handle.width;
    layer->height = hwbuf->handle.height;
    layer->src_x = hwbuf->source.left;
    layer->src_y = hwbuf->source.top;
    layer->src_w = hwbuf->source.getWidth();
    layer->src_h = hwbuf->source.getHeight();

    layer->dst_x = hwbuf->display.left;
    layer->dst_y = hwbuf->display.top;
    layer->dst_w = hwbuf->display.getWidth();
    layer->dst_h = hwbuf->display.getHeight();

    for (int i = 0; i < layer->nplanes; i++) {
        layer->pitch[i] = hwbuf->handle.strides[i];
        layer->offsets[i] = hwbuf->handle.offsets[i];
        layer->bufs[i].fd = hwbuf->handle.fds[i];
    }
    LOGD("source: <%d %d,%d %d> => <%d %d, %d %d>", layer->src_x, layer->src_y, layer->src_w, layer->src_h,
        layer->dst_x, layer->dst_y, layer->dst_w, layer->dst_h);

    return 0;
}
