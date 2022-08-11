#ifndef __SAFE_TS_HEAD__
#define __SAFE_TS_HEAD__

#include <dcf.h>
#include <event.h>
#include "touch_device.h"

#define TS_ANRDOID_MAIN_MBOX_ADDR  (0x70)
#define TS_ANRDOID_AUX1_MBOX_ADDR  (0x71)
#define TS_ANRDOID_AUX2_MBOX_ADDR  (0x72)
#define TS_ANRDOID_AUX3_MBOX_ADDR  (0x73)

#define TS_COORD_METADATA_SIZE sizeof(struct touch_coord_data)

struct touch_coord_data {
    u8 id;
    u16 x;
    u16 y;
    u16 w;
};

struct touch_report_data {
    u8 key_value;
    u8 touch_num;
    struct touch_coord_data coord_data[10];
};

struct xy_range_info {
    u16 x_max;
    u16 y_max;
    u16 x_offset;
    u16 y_offset;
};

struct ts_vendor_info {
    const char *name;
    u16 version;
    u16 id;
    u16 vendor;
};

struct ts_config_info {
    u8 swapped_x_y;
    u8 inverted_x;
    u8 inverted_y;
    u8 max_touch_num;
};

struct safe_ts_device {
    /* service config, auto init, vendor should not fill*/
    mutex_t dev_lock;
    struct dcf_notifier *notifier[TS_DO_MAX];
    event_t event;
    u32 rproc;
    u16 mbox_addr;
    bool inited_flag;

    /* vendor config, vendor must fill */
    u16 instance;
    enum DISPLAY_SCREEN screen_id;
    struct ts_vendor_info vinfo;
    struct ts_config_info cinfo;
    void *vendor_priv;
    int (*set_inited)(void *vendor_priv);
};

/* Let these definition compatiable with another OS side */
enum sts_op_type {
    STS_OP_GET_VERSION,
    STS_OP_GET_CONFIG,
    STS_OP_SET_INITED,
};

/* Do not exceed 16 bytes so far */
struct sts_ioctl_cmd {
    u16 op;
    u16 instance;
    union {
        struct {
            u16 what;
            u16 why;
            u32 how;
        } s;
    } msg;
};

/* Do not exceed 16 bytes so far */
struct sts_ioctl_result {
    u16 op;
    u16 instance;
    union {
        /** used for get_version */
        struct {
            u16 version;
            u16 id;
            u16 vendor;
        } v;
        /** used for get_config */
        struct {
            u16 abs_x_max;
            u16 abs_y_max;
            u16 x_offset;
            u16 y_offset;
            u8 swapped_x_y;
            u8 inverted_x;
            u8 inverted_y;
            u8 max_touch_num;
        } cg;
        u8 data[12];
    } msg;
};

extern struct safe_ts_device *safe_ts_alloc_device(void);
extern void safe_ts_delete_device(struct safe_ts_device *dev);
extern int safe_ts_register_device(struct safe_ts_device *dev);
extern void safe_ts_report_data(struct safe_ts_device *ts, void *data,
                                u16 len);

#endif //__SAFE_TS_HEAD__
