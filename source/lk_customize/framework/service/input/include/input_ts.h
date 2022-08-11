#ifndef __INPUT_TS_HEAD__
#define __INPUT_TS_HEAD__

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

#define MAX_TOUCHSCREEN_NUM     (4)

/* Define common touchscreen data */
struct safe_ts_device {
    mutex_t dev_lock;

    /* communication level stuff */
    struct dcf_notifier *notifier;
    u32 rproc;
    u16 mbox_addr;
    bool virtual;
    u16 instance;
    bool enabled;
    event_t event;

    /* vendor touchscreen fill these data */
    u16 vendor_id;
    void *vendor_priv;
    u32 features;
    u16 (*get_version)(struct safe_ts_device *ts);
    u16 (*get_dev_id)(struct safe_ts_device *ts);
    u16 (*get_dev_type)(struct safe_ts_device *ts);
    int (*get_config)(struct safe_ts_device *ts, u8 *cfg, u16 maxlen);
    int (*set_config)(struct safe_ts_device *ts, u8 *cfg, u16 len);
    int (*get_point)(struct safe_ts_device *ts, u8 *data, u16 maxlen);
    int (*set_inited)(struct safe_ts_device *ts);
};

/* Let these definition compatiable with another OS side */
enum sts_op_type {
    STS_OP_GET_VERSION,
    STS_OP_GET_CONFIG,
    STS_OP_SET_CONFIG,
    STS_OP_RESET,
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
            u16 dev_type;
        } v;
        /** used for get_config */
        struct {
            u16 abs_x_max;
            u16 abs_y_max;
            u16 touch_num;
            u16 x_offset;
            u16 y_offset;
        } gc;
        u8 data[12];
    } msg;
};

#endif //__INPUT_TS_HEAD__
