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
#include "tqueue.h"

// the second encoder instance test.
//#define INTANCE_TWO
typedef struct
{
    struct EncContext *encoder;
    pthread_t encThread;
    pthread_t srcThread;

    FILE *yuvFile;
    FILE *encodedFile;
    TQueue encodingTQ;
    EncOutputBuffer outputBuffer;
    EncInputFrame inputFrame;
    int bitrate;
    int framerate;
    bool exitFlag;
    int frameSize;

#ifdef INTANCE_TWO
    struct EncContext *secEncoder;
    pthread_t encodingthread2;
    FILE *yuvFile2;
    FILE *encodedFile2;
#endif
} TestContext;

int saveBitstream(FILE *, uint8_t *, uint32_t);
void *encodingTask(void *);
void getHeadInfo(void *, void *);
void encodingFrame(void *, void *);
void *srcReadTask(void *);
void quit(TestContext *);
void freeSrcFrameBuf(void *buf);
unsigned char *allocateSrcFrameBuf(unsigned int bufSize);
void changeFramerate(void *pContext, void *ft);
void changeBitrate(void *pContext, void *bt);
void generateIframe(void *pContext, void *param);
#ifdef INTANCE_TWO
void *secEncodingTask(void *context);
#endif
