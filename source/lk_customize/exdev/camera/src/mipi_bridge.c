/*
* mipi_bridge.c
*
* Copyright (c) 2019 Semidrive Semiconductor.
* All rights reserved.
*
* mipi video bridge
*
* Revision History:
* -----------------
* 0.1, 2/18/2019 init version
*/

#include <sys/types.h>
#include <debug.h>
#include <compiler.h>
#include <bits.h>
#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <lib/page_alloc.h>

#include <platform.h>
#include <platform/interrupts.h>


#include "v4l2.h"
#include <kernel/thread.h>
#include "mipi_bridge.h"

#if WITH_KERNEL_VM
#include <kernel/vm.h>
#endif

struct mipi_bridge_dev {
    struct v4l2_device vdev;
    bool streaming;
    int power_count;
};

static inline struct mipi_bridge_dev *to_mipi_bridge(
    struct v4l2_device *vdev)
{
    return containerof(vdev, struct mipi_bridge_dev, vdev);
}

static int vbridge_s_power(struct v4l2_device *vdev, int on)
{
    int ret = 0;
    struct mipi_bridge_dev *bridge = to_mipi_bridge(vdev);

    bridge->power_count += on ? 1 : -1;

    return ret;
}

static int vbridge_g_frame_interval(struct v4l2_device *vdev,
                                    struct v4l2_fract *fi)
{
    struct mipi_bridge_dev *bridge = to_mipi_bridge(vdev);
    fi = &bridge->vdev.frame_interval;

    return 0;
}

static int vbridge_s_frame_interval(struct v4l2_device *vdev,
                                    struct v4l2_fract frame_interval)
{
    struct mipi_bridge_dev *bridge = to_mipi_bridge(vdev);

    if (bridge->streaming) {
        return -EBUSY;
    }

    bridge->vdev.frame_interval = frame_interval;

    return 0;
}

static int vbridge_s_stream(struct v4l2_device *vdev, int enable)
{
    struct mipi_bridge_dev *bridge = to_mipi_bridge(vdev);

    if (bridge->streaming == !enable)
        bridge->streaming = enable;

    return 0;
}

static int vbridge_get_fmt(struct v4l2_device *vdev,
                           struct v4l2_mbus_framefmt *fmt)
{
    struct mipi_bridge_dev *bridge = to_mipi_bridge(vdev);

    *fmt = bridge->vdev.fmt;

    return 0;
}

static int vbridge_set_fmt(struct v4l2_device *vdev,
                           struct v4l2_mbus_framefmt format)
{
    struct mipi_bridge_dev *bridge = to_mipi_bridge(vdev);

    if (bridge->streaming)
        return -EBUSY;

    bridge->vdev.fmt = format;
    return 0;
}

static int vbridge_set_interface(struct v4l2_device *vdev,
                                 struct v4l2_fwnode_endpoint ep)
{
    struct mipi_bridge_dev *bridge = to_mipi_bridge(vdev);

    if (bridge->streaming)
        return -EBUSY;

    if (ep.bus_type != V4L2_MBUS_CSI2)
        return -EINVAL;

    bridge->vdev.ep.bus_type = ep.bus_type;
    return 0;
}

static int vbridge_get_interface(struct v4l2_device *vdev,
                                 struct v4l2_fwnode_endpoint *ep)
{
    struct mipi_bridge_dev *bridge = to_mipi_bridge(vdev);

    ep = &bridge->vdev.ep;

    return 0;
}

static int vbridge_enum_frame_size(struct v4l2_frame_size_enum *fse)
{
    fse->min_width = 320;
    fse->max_width = 3840;
    fse->min_height = 240;
    fse->max_height = 1080;

    return 0;
}

static int vbridge_enum_frame_interval(struct v4l2_device *vdev,
                                       struct v4l2_frame_interval_enum *fie)
{
//  struct mipi_bridge_dev *bridge = to_mipi_bridge(vdev);
    struct v4l2_fract tpf;

    tpf.numerator = 1;
    tpf.denominator = 30;

    fie->interval = tpf;

    return 0;
}

static int vbridge_g_volatile_ctrl(struct v4l2_device *vdev,
                                   struct v4l2_ctrl *ctrl)
{
    return 0;
}

static int vbridge_s_ctrl(struct v4l2_device *vdev,
                          struct v4l2_ctrl *ctrl)
{
    return 0;
}

static const struct v4l2_dev_ops vbridge_vdev_ops = {
    .s_power = vbridge_s_power,
    .g_frame_interval = vbridge_g_frame_interval,
    .s_frame_interval = vbridge_s_frame_interval,
    .s_stream = vbridge_s_stream,
    .get_fmt = vbridge_get_fmt,
    .set_fmt = vbridge_set_fmt,
    .set_interface = vbridge_set_interface,
    .get_interface = vbridge_get_interface,
    .enum_frame_size = vbridge_enum_frame_size,
    .enum_frame_interval = vbridge_enum_frame_interval,
    .g_volatile_ctrl = vbridge_g_volatile_ctrl,
    .s_ctrl = vbridge_s_ctrl,
};

struct v4l2_device *mipi_bridge_init(int id)
{
    struct mipi_bridge_dev *bridge;
    struct v4l2_mbus_framefmt *fmt;

    bridge = malloc(sizeof(*bridge));

    if (!bridge)
        return NULL;

    memset(bridge, 0, sizeof(*bridge));

    fmt = &bridge->vdev.fmt;
    fmt->code = V4L2_PIX_FMT_UYVY;
    fmt->colorspace = V4L2_COLORSPACE_SRGB;
    fmt->width = 640;
    fmt->height = 480;
    fmt->field = V4L2_FIELD_NONE;
    bridge->vdev.frame_interval.numerator = 1;
    bridge->vdev.frame_interval.denominator = 30;

    bridge->vdev.ep.bus_type = V4L2_MBUS_CSI2;
    bridge->vdev.ops = vbridge_vdev_ops;

    return &bridge->vdev;
}

int mipi_bridge_exit(struct v4l2_device *vdev)
{
    struct mipi_bridge_dev *bridge = to_mipi_bridge(vdev);

    free(bridge);

    return 0;
}
