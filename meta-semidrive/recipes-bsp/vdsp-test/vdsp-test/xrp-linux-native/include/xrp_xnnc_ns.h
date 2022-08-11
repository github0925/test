/*
 * Copyright (c) 2016 - 2017 Cadence Design Systems Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef _XRP_XNNC_NS_H
#define _XRP_XNNC_NS_H
#include "xrp_api.h"
#define XRP_XNNC_NSID_INITIALIZER \
  {0xde, 0x78, 0xdb, 0xbe, 0x4a, 0x99, 0x48, 0x89, \
   0x90, 0x83, 0xf0, 0x7b, 0xf8, 0x61, 0x09, 0x7a}

// Maximum number of input and output across all xtensa operations
#define XTENSA_OPERATION_MAX_NUM_INPUTS  7
#define XTENSA_OPERATION_MAX_NUM_OUTPUTS 8

// Define the max size of parameters in bytes of each xtensa operation
#define XTENSA_OPERATION_MAX_PARAMS_SIZE 16

// Define Xtensa NN name
#define XTENSA_XNNC_NAME "MOBLIENET_SSD_FACE"

#ifdef __cplusplus
extern "C" {
#endif

/* Type of operation corresponding to the NN op to be executed on the target DSP */
enum XtensaOperationType {
    XFL_START_INF     = 0,
    XFL_LOAD_COEF     = 1,
    XFL_START_VAN     = 2,
    XFL_START_AGE     = 3,
    XFL_START_GENDER  = 4,
    XFL_START_FACENET = 5,
    XFL_START_HAND    = 6,
    // All XNNC ops should come before this
    XFL_NUM_OPS,
    // Internal operations
    XFL_XNNC_NAME = XFL_NUM_OPS,
    XFL_NUM_ALL_OPS,
};

/* Same struct as DSP used for params, define how to crop the input */
typedef struct {
    int16_t x;
    int16_t y;
    uint16_t width;
    uint16_t height;
} xi_rect;

enum XtensaInputType {
    XFL_INPUT_RGB888  = 0,
    XFL_INPUT_NV12    = 1,
    XFL_INPUT_NV21    = 2,
    XFL_INPUT_I420    = 3,
    XFL_INPUT_YUYV    = 4,
};

typedef struct {
    uint16_t width;
    uint16_t height;
} xi_size;

struct XtensaParams {
    xi_rect crop_rect; /* crop position and size */
    xi_size data_sz;   /* input image size */
    int32_t input_fmt; /* input format, convert by vdsp to speed up */
};

/* Defines an operation in the XRP command buffer */
struct XtensaOperation {
    enum XtensaOperationType opType;
    uint8_t params[XTENSA_OPERATION_MAX_PARAMS_SIZE];
    uint16_t inputIndexes[XTENSA_OPERATION_MAX_NUM_INPUTS];
    uint16_t outputIndexes[XTENSA_OPERATION_MAX_NUM_OUTPUTS];
    uint8_t opt_control;
};

bool checkXtensaXnncName(xrp_queue* xrpQueue) {
    // Dispatch the version command synchronously to the target DSP
    struct XtensaXnncCmd {
        uint32_t result;
        char name[sizeof(XTENSA_XNNC_NAME)];
    } xtensaXnncCmd;
    xrp_status xrpStatus;

    XtensaOperation op = {.opType = XFL_XNNC_NAME};
    xrp_run_command_sync(xrpQueue, &op, sizeof(XtensaOperation),
                         &xtensaXnncCmd, sizeof(XtensaXnncCmd),
                         nullptr, &xrpStatus);
    if (xrpStatus != XRP_STATUS_SUCCESS) {
        fprintf(stderr, "xrp driver failed: %s:%d, err = %d\n",
                __FILE__, __LINE__, xrpStatus);
        return false;
    }

    if (strncmp(xtensaXnncCmd.name, XTENSA_XNNC_NAME, sizeof(XTENSA_XNNC_NAME))) {
        return false;
    }

    return true;
}

/* interfaces for app use */
/* input & output buffers */
struct command_args {
    uint8_t *input_frame;   /* input frame address, RGB888 720P */
    xi_rect *faces_list;    /* buffer to save all found faces rect(<100, 10 is enough) */
    int32_t *faces_nums;    /* save how many faces we found */
    int16_t *landmarks;     /* only need driver's landmark points, 68 * (x, y) */
};

/* vdsp xrp arg flows */
struct xrp_args {
    xrp_device       *device;      /* vdsp device */
    xrp_queue        *queue;       /* vdsp command queue */
    xrp_buffer_group *group1;      /* buffer group for net1(SSD) */
    xrp_buffer_group *group2;      /* buffer group for net2(Vanilla) */
    XtensaOperation  *cmd1;        /* command for net1, malloced in init, freeed in deinit */
    XtensaOperation  *cmd2;        /* command for vanilla net */
    xrp_buffer       *input;       /* same input for 2 nets, will be crop and resize inside vdsp */
    size_t            input_size;
    xrp_buffer       *output1;     /* output for net1, contains faces list  */
    size_t            output1_size;
    xrp_buffer       *output2;     /* output for net2, contains 68 landmark(x, y) */
    size_t            output2_size;
};

/* pre-allocate xrp buffers & cmd buffers for speedup run, return 0 when success */
/* data_fmt: XFL_INPUT_RGB888, XFL_INPUT_NV12, XFL_INPUT_NV21, XFL_INPUT_I420 */
int dms_vdsp_init(struct xrp_args *args, int data_fmt);

/* do the work, fillin the pcmd with input and result buffer, return 0 when success */
int dms_vdsp_run(struct xrp_args *args, struct command_args *pcmd);

/* free pre-allocated buffers */
void dms_vdsp_deinit(struct xrp_args *args);

/* for the new landmarks & pose network */
/* input & output buffers */
struct command_args_pose {
    uint8_t  *input_frame;   /* input frame address, RGB888 720P */
    xi_rect  *faces_list;    /* buffer to save all found faces rect(<100, 10 is enough) */
    int32_t  *faces_nums;    /* save how many faces we found */
    int16_t  *landmarks;     /* only need driver's landmark points, 98 * (x, y) */
    float    *poses;         /* rad angle for yaw, pitch, roll */
    int32_t  *gender;        /* 1: male  0: female */
    int32_t  *age;           /* 0 - 101 */
    float    *features;      /* 512 features to calculate consine distance */
    xi_rect  *hands_list;    /* buffer to save all found hands rect(<100, 10 is enough) */
    int32_t  *hands_nums;    /* save how many hands we found */
};

/* for 98p landmarks and pose angles flow */
struct xrp_args_pose {
    xrp_device       *device;      /* vdsp device */
    xrp_queue        *queue;       /* vdsp command queue */
    xrp_buffer_group *group1;      /* buffer group for net1(SSD) */
    xrp_buffer_group *group2;      /* buffer group for net2(landmark) */
    xrp_buffer_group *group3;      /* buffer group for net3(gender) */
    xrp_buffer_group *group4;      /* buffer group for net4(age) */
    xrp_buffer_group *group5;      /* buffer group for net5(facenet) */
    xrp_buffer_group *group6;      /* buffer group for net6(hand) */
    XtensaOperation  *cmd1;        /* command for net1, malloced in init, freeed in deinit */
    XtensaOperation  *cmd2;        /* command for landmark pose net */
    XtensaOperation  *cmd3;        /* command for gender predict net */
    XtensaOperation  *cmd4;        /* command for age predict net */
    XtensaOperation  *cmd5;        /* command for face recog net */
    XtensaOperation  *cmd6;        /* command for hand detection net */
    xrp_buffer       *input;       /* same input for all nets, will be crop and resize inside vdsp */
    size_t            input_size;
    xrp_buffer       *output1;     /* output for net1, contains faces list  */
    size_t            output1_size;
    xrp_buffer       *output21;    /* output for net2, contains 98 landmarks(x, y) */
    size_t            output21_size;
    xrp_buffer       *output22;    /* output for net2, contains 3 Euler angle */
    size_t            output22_size;
    xrp_buffer       *output3;     /* output for net3, contains 2 gender prob */
    size_t            output3_size;
    xrp_buffer       *output4;     /* output for net4, contains age prob */
    size_t            output4_size;
    xrp_buffer       *output5;     /* output for net5, contains 512 face features */
    size_t            output5_size;
    xrp_buffer       *output61;    /* output for net6, contains 400 S16 boxes */
    size_t            output61_size;
    xrp_buffer       *output62;    /* output for net6, contains 512 face features */
    size_t            output62_size;
    xrp_buffer       *output63;    /* output for net6, contains 512 face features */
    size_t            output63_size;
};

/* pre-allocate xrp buffers & cmd buffers for speedup run, return 0 when success */
int dms_pose_vdsp_init(struct xrp_args_pose *args);

#define VDSP_DO_LANDMARK 0x1  /* if not set, pfpld will not run */
#define VDSP_DO_GENDER   0x2  /* if not set, gender predict will skip */
#define VDSP_DO_AGE      0x4  /* if not set, age predict will not run */
#define VDSP_DO_FACEID   0x8  /* if not set, face recognition will not run(need to load features.bin first */
#define VDSP_DO_HAND     0x10 /* if not set, hand detection will not run */
/* do the work, fillin the pcmd with input and result buffer, return 0 when success */
int dms_pose_vdsp_run(struct xrp_args_pose *args, struct command_args_pose *pcmd, uint32_t flow, bool use_filter);

/* free pre-allocated buffers */
void dms_pose_vdsp_deinit(struct xrp_args_pose *args);

#ifdef __cplusplus
}
#endif

#endif // _XRP_XNNC_NS_H
