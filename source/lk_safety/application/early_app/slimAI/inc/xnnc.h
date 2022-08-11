#include <app.h>
#include <lk_wrapper.h>
#include <console.h>
#include <thread.h>
#include <event.h>
#include "container.h"
#include <sdrpc.h>
#include <sdm_display.h>
#include <disp_data_type.h>
#include <string.h>
#include <malloc.h>
#include <heap.h>

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

/* Type of operation corresponding to the NN op to be executed on the target DSP */
enum XtensaOperationType {
    XFL_START_INF     = 0,
    XFL_LOAD_COEF     = 1,
    XFL_START_VAN     = 2,
    XFL_START_AGE     = 3,
    XFL_START_GENDER  = 4,
    XFL_START_FACENET = 5,
    // All XNNC ops should come before this
    XFL_NUM_OPS,
    // Internal operations
    XFL_XNNC_NAME     = XFL_NUM_OPS,
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
    enum     XtensaOperationType opType;
    uint8_t  params[XTENSA_OPERATION_MAX_PARAMS_SIZE];
    uint16_t inputIndexes[XTENSA_OPERATION_MAX_NUM_INPUTS];
    uint16_t outputIndexes[XTENSA_OPERATION_MAX_NUM_OUTPUTS];
    uint8_t  opt_control;
};

bool checkXtensaXnncName(struct xrp_queue* xrpQueue) {
    // Dispatch the version command synchronously to the target DSP
    struct XtensaXnncCmd {
        uint32_t result;
        char     name[sizeof(XTENSA_XNNC_NAME)];
    } xtensaXnncCmd;
    enum xrp_status xrpStatus;

    struct XtensaOperation op = {.opType = XFL_XNNC_NAME};
    printf("run check name command\n");
    xrp_run_command_sync(xrpQueue, &op, sizeof(struct XtensaOperation),
                         &xtensaXnncCmd, sizeof(struct XtensaXnncCmd),
                         NULL, &xrpStatus);
    if (xrpStatus != XRP_STATUS_SUCCESS) {
        printf("xrp driver failed: %s:%d, err = %d\n", __FILE__, __LINE__, xrpStatus);
        return false;
    }

    printf("%s\n", xtensaXnncCmd.name);
    if (strncmp(xtensaXnncCmd.name, XTENSA_XNNC_NAME, sizeof(XTENSA_XNNC_NAME))) {
        return false;
    }

    return true;
}

