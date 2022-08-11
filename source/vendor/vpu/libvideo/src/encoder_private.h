/*******************************************************************************
 *           Copyright (C) 2020 Semidrive Technology Ltd. All rights reserved.
 ******************************************************************************/

/*******************************************************************************
 * FileName : encoder_private.h
 * Version  : 1.0.0
 * Purpose  : encoder internal header file
 * Authors  : wei.fan
 * Date     : 2021-06-25
 * Notes    :
 ******************************************************************************/
#include "vpuapi.h"
#include "encoder.h"

#define STREAM_BUF_SIZE 0x700000 // max bitstream size
#define MAX_REG_BUFFER 2         // max bitstream size
#define ENC_SRC_BUFFER_NUM 2     // max bitstream size
#define VBV_BUFFER_SIZE 0
typedef enum
{
    VPU_NONE,
    VPU_OPEN,
    VPU_SEQ_INIT,
    VPU_TERMINAL,
    VPU_MAX
} VPU_STATUS;

typedef struct
{
    EncParam encParam;
    EncOpenParam openParam;
    EncHandle encHandle;
    vpu_buffer_t vbBitstream;
    vpu_buffer_t vbRefFrameBuffer[MAX_REG_BUFFER];
    vpu_buffer_t vbSourceFrameBuffer[ENC_SRC_BUFFER_NUM];
    FrameBuffer srcFrameBuffer[ENC_SRC_BUFFER_NUM];
    int srcFrameIndex;
    TiledMapType mapType;
    int regFramebufferCount;
    int rotAngle;
    int mirDir;
    int bitRate;
    int frameRate;
    int profile;
    int level;
    int picHeight;
    int picWidth;
    RCMode rcMode;
    int delayMs;
    int productId;
    bool exitFlag;
    bool forceIframe;
    int coreIdx;
    int sourceDMAEnable;
    int frameCnt;
    YUVFormat frameFmt;
    int intialDelay;  //it affects the encoding time of first frame
    int secondaryAxi; // should match srammode, (for encoder,secondary AXI not used)
    VPU_STATUS vpuStatus;
#ifdef __DUMP_SRC_DATA__
    osal_file_t *dumpfile;
#endif
} EncContext;
