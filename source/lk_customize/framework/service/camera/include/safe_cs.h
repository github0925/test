#ifndef __CSI_SRV_HEAD__
#define __CSI_SRV_HEAD__

#include <stdlib.h>
#include <debug.h>
#include <stdio.h>
#include <err.h>
#include <sys/types.h>
#include <lib/console.h>
#include <lib/bytes.h>
#include <lib/reg.h>
#include <dcf.h>
#include <event.h>

#include "v4l2.h"



/*
 *  F R A M E   S I Z E   E N U M E R A T I O N
 */
enum v4l2_frmsizetypes {
    V4L2_FRMSIZE_TYPE_DISCRETE  = 1,
    V4L2_FRMSIZE_TYPE_CONTINUOUS    = 2,
    V4L2_FRMSIZE_TYPE_STEPWISE  = 3,
};
enum v4l2_buf_type {
    V4L2_BUF_TYPE_VIDEO_CAPTURE        = 1,
    V4L2_BUF_TYPE_VIDEO_OUTPUT         = 2,
    V4L2_BUF_TYPE_VIDEO_OVERLAY        = 3,
    V4L2_BUF_TYPE_VBI_CAPTURE          = 4,
    V4L2_BUF_TYPE_VBI_OUTPUT           = 5,
    V4L2_BUF_TYPE_SLICED_VBI_CAPTURE   = 6,
    V4L2_BUF_TYPE_SLICED_VBI_OUTPUT    = 7,
    V4L2_BUF_TYPE_VIDEO_OUTPUT_OVERLAY = 8,
    V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE = 9,
    V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE  = 10,
    V4L2_BUF_TYPE_SDR_CAPTURE          = 11,
    V4L2_BUF_TYPE_SDR_OUTPUT           = 12,
    V4L2_BUF_TYPE_META_CAPTURE         = 13,
    /* Deprecated, do not use */
    V4L2_BUF_TYPE_PRIVATE              = 0x80,
};


#define DDR_OFFSET 0x10000000

#define MAX_CAM_INST 9


#define AVM_SRV_LOG 4




struct csi_config {
    struct v4l2_mbus_framefmt fmt;
    struct v4l2_fract fi;
    struct v4l2_fwnode_endpoint ep;
    bool crop_enable;
};

/* Define common csi data */
struct cam_device {
    mutex_t dev_lock;

    bool binitialized;
    bool enabled;
    bool sync;
    uint32_t ip;
    uint32_t channel;

    void *csi_handle;
    struct csi_config csi_config;

    struct csi_buffer *cb;

    /* communication level stuff */
    struct dcf_notifier *notifier;
    u32 rproc;
    u16 mbox_addr;
    u16 instance;

    event_t event;
};


/* Let these definition compatiable with another OS side */
enum scs_op_type {
    SCS_OP_DEV_OPEN,
    SCS_OP_ENUM_FORMAT,
    SCS_OP_ENUM_FRAMESIZE,
    SCS_OP_GET_FORMAT,
    SCS_OP_SET_FORMAT,
    SCS_OP_QUEUE_SETUP,
    SCS_OP_GET_BUFINFO,
    SCS_OP_SET_BUFINFO,
    SCS_OP_QBUF,
    SCS_OP_STREAM_ON,
    SCS_OP_DQBUF,
    SCS_OP_STREAM_OFF,
    SCS_OP_DEV_CLOSE,
};

/* Do not exceed 16 bytes so far */
struct scs_ioctl_cmd {
    u16 op;
    u16 instance;
    union {
        struct {
            u32 fmt;
            u16 index;
        } fmt;
        struct {
            u16 type;
            u16 width;
            u16 height;
            u32 pixelformat;
        } s_fmt;
        struct {
            u16 type;
            u16 width;
            u16 height;
            u16 index;
        } fsz;
        struct {
            u16 index;
            u32 len;
            u32 addr;
        } s_bufinfo;
        u8 data[12];
    } msg;
};

/* Do not exceed 16 bytes so far */
struct scs_ioctl_result {
    u16 op;
    u16 instance;
    union {
        /** used for get_version */
        struct {
            u32 fmt;
            u16 index;
        } fmt;
        /** used for get_config */
        struct {
            u16 type;
            u16 width;
            u16 height;
            u16 index;
        } fsz;
        struct {
            u16 type;
            u16 width;
            u16 height;
            u32 pixelformat;
        } g_fmt;
        struct {
            u16 index;
            u32 len;
            u32 addr;
        } g_bufinfo;
        struct {
            u16 bufs;
            u16 planes;
            u32 size;
        } queue;
        u8 data[12];
    } msg;
};

int get_format_size(u32 format);


int cam_service_probe_device(struct cam_device *cam, int instance);

int cam_dev_init(struct cam_device *cam);

int cam_dev_enum_fmt(struct cam_device *cam,
                            struct v4l2_fmtdesc *fe);

int cam_dev_enum_framesize(struct cam_device *cam,
                                  struct v4l2_frame_size_enum *fse);

int cam_dev_config(struct cam_device *cam);

int cam_dev_get_bufinfo(void *handle, int index, uint8_t *pin);

bool cam_dev_init_buf(struct cam_device *cam, int index,
                             uint8_t *pin);

int cam_dev_qbuf(struct cam_device *cam, uint32_t index);

int cam_dev_start(struct cam_device *cam);

int cam_dev_stop(struct cam_device *cam);

int cam_dev_close(struct cam_device *cam);

#endif //__CSI_SRV_HEAD__
