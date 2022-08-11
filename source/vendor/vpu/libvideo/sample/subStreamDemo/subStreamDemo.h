/*******************************************************************************
 *           Copyright (C) 2020 Semidrive Technology Ltd. All rights reserved.
 ******************************************************************************/

/*******************************************************************************
 * FileName : encoderapis_test.h
 * Version  : 1.0.0
 * Purpose  : sample app header file
 * Authors  : wei.fan
 * Date     : 2021-06-25
 * Notes    :
 ******************************************************************************/

#include "encoder.h"
#include <pthread.h>
#include <stdio.h>
#include "ion_memorymanager.h"
#include "g2dapi.h"

// the second encoder instance test.
typedef struct
{
    struct EncContext *encoder;
    pthread_t encThread;

    FILE *encodedFile;
    FILE *yuvout;
    EncOutputBuffer outputBuffer;
    EncInputFrame inputFrame;
    int bitrate;
    int framerate;
    bool exitFlag;
    int frameSize;
    int ion_fd;
} TestContext;

int saveBitstream(FILE *, uint8_t *, uint32_t);
void getHeadInfo(void *, void *);
int encodingFrame(void *, void *);
void quit(TestContext *);
void freeIonFrameBuf(void *buf, int framesize, int ion_fd);
void changeFramerate(void *pContext, void *ft);
void changeBitrate(void *pContext, void *bt);
void generateIframe(void *pContext, void *param);

