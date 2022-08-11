/*
* v4l2.h
*
* Copyright (c) 2018 Semidrive Semiconductor.
* All rights reserved.
*
* v4l2 struct header
*
* Revision History:
* -----------------
* 0.1, 12/21/2018 init version
*/
#pragma once
#include <stdio.h>
#include <sys/types.h>
#include <platform.h>
#include "videodev2.h"


/**
 * enum v4l2_mbus_type - media bus type
 * @V4L2_MBUS_PARALLEL: parallel interface with hsync and vsync
 * @V4L2_MBUS_BT656:   parallel interface with embedded synchronisation, can
 *             also be used for BT.1120
 * @V4L2_MBUS_CSI1:  MIPI CSI-1 serial interface
 * @V4L2_MBUS_CCP2: CCP2 (Compact Camera Port 2)
 * @V4L2_MBUS_CSI2:    MIPI CSI-2 serial interface
 */
enum v4l2_mbus_type {
    V4L2_MBUS_PARALLEL,
    V4L2_MBUS_BT656,
    V4L2_MBUS_CSI1,
    V4L2_MBUS_CCP2,
    V4L2_MBUS_CSI2,
    V4L2_MBUS_DC_PARALLEL,
    V4L2_MBUS_DC_PARALLEL2,
    V4L2_MBUS_PARALLEL2,
};

struct v4l2_fwnode_endpoint {
    enum v4l2_mbus_type bus_type;
    unsigned int flags;
};

struct v4l2_fract {
    uint32_t numerator;
    uint32_t denominator;
};


struct ex_info {
    uint32_t pclk;  //MHz
    uint32_t lanes;
    uint32_t hsa;
    uint32_t hbp;
    uint32_t hsd;
    bool sync;
    uint32_t vcn;
    uint8_t vc;
    uint32_t mask_en;
};

/**
 * struct v4l2_subdev_frame_interval - Pad-level frame rate
 * @pad: pad number, as reported by the media API
 * @interval: frame interval in seconds
 */
struct v4l2_frame_interval {
    uint32_t pad;
    struct v4l2_fract interval;
    uint32_t reserved[9];
};

/**
 * struct v4l2_mbus_framefmt - frame format on the media bus
 * @width:  image width
 * @height: image height
 * @code:   data format code (from enum v4l2_mbus_pixelcode)
 * @field:  used interlacing type (from enum v4l2_field)
 * @colorspace: colorspace of the data (from enum v4l2_colorspace)
 */
struct v4l2_mbus_framefmt {
    uint32_t            width;
    uint32_t            height;
    uint32_t            code;
    uint32_t            field;
    uint32_t            colorspace;
    uint16_t            reserved[11];
};

/**
 * struct v4l2_frame_interval_enum - Frame interval enumeration
 * @pad: pad number, as reported by the media API
 * @index: frame interval index during enumeration
 * @code: format code (MEDIA_BUS_FMT_ definitions)
 * @width: frame width in pixels
 * @height: frame height in pixels
 * @interval: frame interval in seconds
 */
struct v4l2_frame_interval_enum {
    uint32_t index;
    uint32_t pad;
    uint32_t code;
    uint32_t width;
    uint32_t height;
    struct v4l2_fract interval;
};

/*
 *  F O R M A T   E N U M E R A T I O N
 */
struct v4l2_fmtdesc {
    uint32_t index;             /* Format number      */
    uint32_t pixelformat;       /* Format fourcc      */
};

/**
 * struct v4l2_subdev_frame_size_enum - Media bus format enumeration
 * @pad: pad number, as reported by the media API
 * @index: format index during enumeration
 * @code: format code (MEDIA_BUS_FMT_ definitions)
 */
struct v4l2_frame_size_enum {
    uint32_t index;
    uint32_t code;
    uint32_t min_width;
    uint32_t max_width;
    uint32_t min_height;
    uint32_t max_height;
};

struct v4l2_ctrl {
    uint32_t id;
    uint32_t val;
};

struct v4l2_device;
struct v4l2_dev_ops {
    int (*s_power)(struct v4l2_device *vdev, int on);
    int (*g_frame_interval)(struct v4l2_device *vdev,
                            struct v4l2_fract *fi);
    int (*s_frame_interval)(struct v4l2_device *vdev,
                            struct v4l2_fract fi);
    int (*s_stream)(struct v4l2_device *vdev, int enable);
    int (*enum_format)(struct v4l2_device *vdev, struct v4l2_fmtdesc *fe);
    int (*get_fmt)(struct v4l2_device *vdev,
                   struct v4l2_mbus_framefmt *fmt);
    int (*set_fmt)(struct v4l2_device *vdev,
                   struct v4l2_mbus_framefmt fmt);
    int (*get_interface)(struct v4l2_device *vdev,
                         struct v4l2_fwnode_endpoint *ep);
    int (*set_interface)(struct v4l2_device *vdev,
                         struct v4l2_fwnode_endpoint ep);
    int (*enum_frame_size)(struct v4l2_frame_size_enum *fse);
    int (*enum_frame_interval)(struct v4l2_device *vdev,
                               struct v4l2_frame_interval_enum *fie);
    int (*g_volatile_ctrl)(struct v4l2_device *vdev,
                           struct v4l2_ctrl *ctrl);
    int (*s_ctrl)(struct v4l2_device *vdev,
                  struct v4l2_ctrl *ctrl);
    int (*sync_enable)(struct v4l2_device *vdev, bool en);
    int (*vc_enable)(struct v4l2_device *vdev,
                  bool en, uint8_t ch);
    int (*close)(struct v4l2_device *vdev);
};

struct v4l2_device {
    struct v4l2_fwnode_endpoint ep;
    struct v4l2_mbus_framefmt fmt;
    struct v4l2_fract frame_interval;
    struct v4l2_dev_ops ops;
    struct ex_info ex_info;
};

/**
 * v4l2_find_nearest_size - Find the nearest size among a discrete
 *  set of resolutions contained in an array of a driver specific struct.
 *
 * @array: a driver specific array of image sizes
 * @array_size: the length of the driver specific array of image sizes
 * @width_field: the name of the width field in the driver specific struct
 * @height_field: the name of the height field in the driver specific struct
 * @width: desired width.
 * @height: desired height.
 *
 * Finds the closest resolution to minimize the width and height differences
 * between what requested and the supported resolutions. The size of the width
 * and height fields in the driver specific must equal to that of u32, i.e. four
 * bytes.
 *
 * Returns the best match or NULL if the length of the array is zero.
 */

#define v4l2_find_nearest_size(type, array, array_size, width_field, height_field, \
                           width, height)               \
    ({                              \
           __v4l2_find_nearest_size(        \
                    (array), array_size, sizeof(*(array)),      \
                    offsetof(type, width_field),  \
                    offsetof(type, height_field), \
                    width, height);                 \
    })

const void *
__v4l2_find_nearest_size(const void *array, size_t array_size,
                         size_t entry_size, size_t width_offset,
                         size_t height_offset, s32 width, s32 height);

//extern lk_bigtime_t current_time(void);

static inline void mdelay(uint32_t ms)
{
    lk_bigtime_t start = current_time();

    while ((current_time() - start) < ms);
}

#define udelay spin

