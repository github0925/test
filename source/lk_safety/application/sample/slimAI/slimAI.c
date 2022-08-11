#include <app.h>
#include <lk_wrapper.h>
#include <console.h>
#include <thread.h>
#include <event.h>
#include "container.h"
#include <sdrpc.h>
#include <string.h>
#include <malloc.h>
#include <heap.h>
#include "res_loader.h"

#include "xrp_config.h"
#include "xrp_api.h"
#include "xnnc.h"

#define VDSP_ELF_PATH   "sample/slimai.elf"
#define INPUT_PATH      "sample/input1280x720.rgb"
void vdsp_rst_stall(uint32_t vector_base);
void vdsp_go(void);
int  vdsp_load_firmware(uint32_t elf_base,uint32_t elf_len, void* dummy_binary_base);
#define VDSP_FW_RUNTIME_MEM (ap2p(VDSP_MEMBASE))

struct xrp_device *xrp_dev = NULL;
struct xrp_queue  *xrp_que = NULL;

void slimai_test(void)
{
    enum xrp_status xrpStatus;
    xrp_dev = xrp_open_device(0, &xrpStatus);
    if (xrpStatus != XRP_STATUS_SUCCESS) {
        printf("xrp driver failed: %s:%d, err = %d\n", __FILE__, __LINE__, xrpStatus);
        return;
    } else {
        printf("xrp device open OK\n");
    }

    // Initialize XRP command queue
    unsigned char XRP_XNNC_NSID[] = XRP_XNNC_NSID_INITIALIZER;
    xrp_que = xrp_create_ns_queue(xrp_dev, &XRP_XNNC_NSID, &xrpStatus);
    if (xrpStatus != XRP_STATUS_SUCCESS) {
        printf("xrp driver failed: %s:%d, err = %d\n", __FILE__, __LINE__, xrpStatus);
        return;
    } else {
         printf("xrp queue create OK\n");
    }

    if (!checkXtensaXnncName(xrp_que)) {
        printf("xrp NN name mismatch!\n");
        return;
    }

    printf("start to feed preprocessed input\n");

    /* test the first face detect net */
    /* create buffer group for 1 input & 1 output buffer */
    struct xrp_buffer_group *xrpBufferGroup = xrp_create_buffer_group(&xrpStatus);
    if (xrpStatus != XRP_STATUS_SUCCESS) {
        printf("create xrp buffer group failed, err = %d\n", xrpStatus);
        return;
    }

    /* create input node */
    size_t inputSize = 1280 * 720 * 3; // only test rgb888
    void* einput = memalign(32, inputSize);
    printf("input @ %p size:0x%x\n", einput, inputSize);
    ASSERT(einput);

    res_load(INPUT_PATH, einput, inputSize, 0);

    struct xrp_buffer *inputXrpBuf = xrp_create_buffer(xrp_dev, inputSize, einput, &xrpStatus);
    if (xrpStatus != XRP_STATUS_SUCCESS) {
        printf("create input buffer failed, err = %d\n", xrpStatus);
        return;
    }

    /* Add input XRP buffer to the buffer group */
    size_t inputGrpIdx;
    inputGrpIdx = xrp_add_buffer_to_group(xrpBufferGroup, inputXrpBuf, XRP_READ, &xrpStatus);
    if (xrpStatus != XRP_STATUS_SUCCESS) {
        printf("add input buffer to group failed, err = %d\n", xrpStatus);
        return;
    }

    /* Create & add output buffers to the buffer group */
    size_t outputSize = (1+1+4)*100*4; // F32 x 100 class:1, confidence:1, bbox:4(l,t,r,b)
    struct xrp_buffer *outputXrpBuf = xrp_create_buffer(xrp_dev, outputSize, NULL, &xrpStatus);
    if (xrpStatus != XRP_STATUS_SUCCESS) {
        printf("create input buffer failed, err = %d\n", xrpStatus);
        return;
    }
    size_t outputGrpIdx;
    outputGrpIdx = xrp_add_buffer_to_group(xrpBufferGroup, outputXrpBuf, XRP_READ_WRITE, &xrpStatus);
    if (xrpStatus != XRP_STATUS_SUCCESS) {
        printf("add input buffer to group failed, err = %d\n", xrpStatus);
        return;
    }

    struct XtensaOperation *cmd = (struct XtensaOperation *)malloc(sizeof(struct XtensaOperation));
    if (cmd == NULL) {
        printf("malloc cmd failed\n");
        return;
    }

    /* Setup the command's XRP buffer group indices for its inputs and outputs */
    cmd->opType = XFL_START_INF; // start one ssd frame inf

    struct XtensaParams *p_params = (struct XtensaParams *)(&(cmd->params));
    xi_rect *rect_crop = &(p_params->crop_rect); // crop the input fullimage
    xi_size *size_data = &(p_params->data_sz);   // input fullimage size
    int32_t       *fmt = &(p_params->input_fmt);
    rect_crop->x       = 160; // crop the center
    rect_crop->y       = 0;
    rect_crop->width   = 960;
    rect_crop->height  = 720;
    size_data->width   = 1280;
    size_data->height  = 720;
    *fmt               = XFL_INPUT_RGB888;

    memset(cmd->inputIndexes, 0xff, XTENSA_OPERATION_MAX_NUM_INPUTS*sizeof(uint16_t));
    cmd->inputIndexes[0]  = inputGrpIdx;
    memset(cmd->outputIndexes, 0xff, XTENSA_OPERATION_MAX_NUM_OUTPUTS*sizeof(uint16_t));
    cmd->outputIndexes[0] = outputGrpIdx;

    int result;
    xrp_run_command_sync(xrp_que, cmd,
                         1 * sizeof(struct XtensaOperation),
                         &result, sizeof(result),
                         xrpBufferGroup, &xrpStatus);

    if (xrpStatus != XRP_STATUS_SUCCESS) {
        printf("Command run failed\n");
        return;
    }

    /* postprocess detection ouput */
    /* output: 1.000000 1 6 100 1 F32 eg:
       class(face) confidence score   bbox left  top       right     bottom
       1.000000    0.99               0.178135   0.125764  0.378135  0.398944
     */
    float *outputBuf = (float *)xrp_map_buffer(outputXrpBuf, 0, outputSize, XRP_READ_WRITE, &xrpStatus);
    if (xrpStatus != XRP_STATUS_SUCCESS) {
        printf("map output buffer to userspace failed, err = %d\n", xrpStatus);
        return;
    }
    printf("outputBuf map to %p\n", outputBuf);

    float  *output_pos = outputBuf;
    int16_t boxes      = 100;
    int16_t faces_nums = 0;
    float   class_idx, score;
    float   bbox_y1, bbox_x1, bbox_y2, bbox_x2;
    for (int j = 0; j < boxes; j++) {
        class_idx = *(output_pos + 0);
        score     = *(output_pos + 1);
        bbox_x1   = *(output_pos + 2);
        bbox_y1   = *(output_pos + 3);
        bbox_x2   = *(output_pos + 4);
        bbox_y2   = *(output_pos + 5);

        if (class_idx > 0.1) { // 1.0 is face, others is 0.0
            float py1 = bbox_y1 * 720.0f;
            float px1 = bbox_x1 * 960.0f + 160.0f;
            float py2 = bbox_y2 * 720.0f;
            float px2 = bbox_x2 * 960.0f + 160.0f;

            int32_t left = (int32_t)px1, top = (int32_t)py1, right = (int32_t)px2, bottom = (int32_t)py2;
            faces_nums++;
            printf("class:%f, score:%f, bbox x1:%d, y1:%d, x2:%d, y2:%d\n",
                    class_idx, score, left, top, right, bottom);
        } else {
            break;
        }
        output_pos += 6;
    }
    printf("%d faces detected\n", faces_nums);

    xrp_unmap_buffer(outputXrpBuf, outputBuf, &xrpStatus);

    xrp_release_buffer_group(xrpBufferGroup);
    xrp_release_buffer(outputXrpBuf);
    xrp_release_buffer(inputXrpBuf);

    free(cmd);
}


void slimai_loadfm(void)
{
    if (xrp_que) {
        xrp_release_queue(xrp_que);
        xrp_que = NULL;
    }

    if (xrp_dev) {
        xrp_release_device(xrp_dev);
        xrp_dev = NULL;
    }

    int elf_size = ROUNDUP(res_size(VDSP_ELF_PATH),32);

    void* elf = memalign(32, elf_size);
    printf("elf file @ %p size:0x%x\n", elf, elf_size);
    ASSERT(elf);

    res_load(VDSP_ELF_PATH, elf, elf_size, 0);
    xrp_load_firmware((uint32_t)elf, res_size(VDSP_ELF_PATH), XRP_COMM_BASE);

    vdsp_rst_stall(p2ap((paddr_t)VDSP_FW_RUNTIME_MEM));
    vdsp_load_firmware((uint32_t)elf, elf_size, NULL);
    printf("vdsp load done\n");
    vdsp_go();
    free(elf);
}

#if defined(WITH_LIB_CONSOLE)
STATIC_COMMAND_START
STATIC_COMMAND("slimai_test",   "test vdsp AI functions", (console_cmd)&slimai_test)
STATIC_COMMAND("slimai_loadfm", "load vdsp firmware", (console_cmd)&slimai_loadfm)
STATIC_COMMAND_END(cv_sets);
#endif
