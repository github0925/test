/*
* sd_csi.h
*
* Copyright (c) 2018 Semidrive Semiconductor.
* All rights reserved.
*
* csi interface struct header
*
* Revision History:
* -----------------
* 0.1, 12/21/2018 init version
*/

#pragma once

#include "v4l2.h"
#include "csi_reg.h"
#include <kernel/event.h>
#include <kernel/thread.h>
#include <list.h>

#if DC_FPGA_SUPPORT_CAM
#include <crc32.h>
#include <dc_def.h>
#endif
#if WITH_ON_ZONE
#define CAMERA_MAX_BUF          2
#else
#define CAMERA_MAX_BUF          3
#endif
#define CAMERA_BUF_POS(n) ((n-1) > 0 ? ((n) - 2) : (CAMERA_MAX_BUF + (n) - 2))
//#define CAMERA_BUF_POS(n) ((n) > 0 ? ((n) - 1) : (CAMERA_MAX_BUF + (n) - 1))
#define CSI_INC_BUF_POS(i) (((i) + 1) < CAMERA_MAX_BUF ? (i +1) : 0)

struct csi_device;

struct csi_image {
    bool enable;
    bool initialized;
    uint8_t id;
    struct csi_device *csi;
    event_t completion;
    uint8_t buf_pos;
    uint8_t buf_free;
    addr_t rgby_baddr[CAMERA_MAX_BUF];
    addr_t u_baddr[CAMERA_MAX_BUF];
    addr_t v_baddr[CAMERA_MAX_BUF];
    uint32_t rgby_stride;
    uint32_t u_stride;
    uint32_t v_stride;
    uint32_t height;
    bool crop_enable;
    uint32_t crop_pos;
    uint32_t crop_len;
    thread_t *stream_thread;
    struct list_node buf_list;
    uint32_t q_flag;
};

struct csi_buffer {
    int index;
    struct list_node node;
};

struct csi_device_ops {
    int (*init)(struct csi_device *dev);
    int (*start)(struct csi_device *dev, uint32_t mask);
    int (*stop)(struct csi_device *dev, uint32_t mask);
    int (*cfg_mem)(struct csi_device *dev, uint32_t mask);
    int (*cfg_interface)(struct csi_device *dev,
                         struct v4l2_fwnode_endpoint endpoint);
    struct csi_image *(*get_image)(struct csi_device *dev, uint8_t image_id);
    int (*set_image)(struct csi_device *dev, struct csi_image *image);
    int (*qbuf)(struct csi_device *dev, int img, struct csi_buffer *b);
};

struct csi_device {
    uint8_t id;
    bool streaming;
    uint32_t mask_en;
    addr_t regs;
    uint32_t irq;
    uint32_t interface;
    spin_lock_t lock;
    struct csi_device_ops ops;
    struct v4l2_device *vdev;
    struct csi_image csi_image[IMG_COUNT];
//    thread_t *t[IMG_COUNT];
    enum handler_return (*int_handler)(void *arg);
    event_t err_completion;
    uint32_t stream_cnt;
    uint32_t err_cnt;
    uint32_t err_stat0;
    uint32_t err_stat1;
    bool err_thread_done;
    thread_t *err_thread;
    uint32_t err_bt_cof;
    uint32_t err_bt_fatal;
    uint32_t err_bt_ecc;
    uint32_t err_bus;
    uint32_t err_overflow;
    uint32_t err_pixel;
    uint32_t err_crop;
    uint32_t frm_cnt;
    void *priv;
    //struct list_node buf_list;
    bool using_queue;
    uint32_t sdw_cnt;
    bool sync;
    uint32_t vcn;
    bool initialized;
};

struct csi_device *csi_host_init(uint32_t interface, addr_t addr,
                                 uint32_t irq);
int csi_exit(struct csi_device *dev);
