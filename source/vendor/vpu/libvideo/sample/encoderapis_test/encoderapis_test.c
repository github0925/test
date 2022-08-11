#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "encoderapis_test.h"

#define WIDTH 1920
#define HEIGHT 1080
#define MAX_FRAME_SIZE 1000
#define SRC_IMG_FILE "BigBuckBunny_30.yuv"
#define SRC_IMG_FILE_SEC "bus_cif.yuv"
#define BITSTREAM_FILE "avc.bin"
#define BITSTREAM_FILE_SEC "avc-2.bin"

int main(int argc, char **argv)
{
    //the picture name , width, height could be gotten from arg
    //ex: vpuencoder -w1080 -h720 -swaterfall_cif.yuv -b2000 -f24
    int opt;
    extern char *optarg;
    EncBaseConfig encBaseCfg = {0};
    TestContext tContext = {0};

    encBaseCfg.rcMode = 1;
    encBaseCfg.picHeight = HEIGHT;
    encBaseCfg.picWidth = WIDTH;

    encBaseCfg.bitRate = 4000;
    encBaseCfg.frameRate = 30;
    encBaseCfg.profile = -1;
    encBaseCfg.level = -1;
    encBaseCfg.srcFormat = YUV420Planar;

    while ((opt = getopt(argc, argv, "w:f:b:h:r:s:n:o:")) != -1)
    {
        switch (opt)
        {
        case 'w':
            printf("%s , \n", optarg);
            encBaseCfg.picWidth = atoi(optarg);
            break;
        case 'f':
            printf("%s , \n", optarg);
            encBaseCfg.frameRate = atoi(optarg);
            break;
        case 'b':
            printf("%s , \n", optarg);
            encBaseCfg.bitRate = atoi(optarg);
            break;
        case 'h':
            printf("%s , \n", optarg);
            encBaseCfg.picHeight = atoi(optarg);
            break;
        case 'r':
            printf("%s , \n", optarg);
            encBaseCfg.rcMode = atoi(optarg);
            break;
        case 's':
            printf("%s , \n", optarg);
            tContext.yuvFile = fopen(optarg, "r");
            if (!tContext.yuvFile)
            {
                printf("-s file not exist \n");
                return -1;
            }
            break;
        case 'o':
            printf("%s , \n", optarg);
            tContext.encodedFile = fopen(optarg, "wb");
            if (!tContext.encodedFile)
            {
                printf("-o %s file not exist \n", optarg);
                return -1;
            }
            break;
        case 'n':
            printf("%s , \n", optarg);
            int i = atoi(optarg);
            if (i == 0)
                encBaseCfg.srcFormat = NV21;
            else if (i == 1)
                encBaseCfg.srcFormat = NV12;
            break;
        default:
            fprintf(stderr, "Usage: %s [-wfbhrsno] \n", argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    tContext.frameSize = encBaseCfg.picHeight * encBaseCfg.picWidth * 3 / 2;

    if (tqueue_init(&tContext.encodingTQ, 100) == -1)
    {
        printf("thread task queue creating failed \n");
        goto ERR_QUIT_ENTRY;
    }

    if (!tContext.yuvFile)
        tContext.yuvFile = fopen(SRC_IMG_FILE, "r");
    if (!tContext.encodedFile)
        tContext.encodedFile = fopen(BITSTREAM_FILE, "wb");

    if (!tContext.yuvFile || !tContext.encodedFile)
    {
        printf("yuvFile not exist or bin file not created successfully \n");
        goto ERR_QUIT_ENTRY;
    }

    tContext.encoder = encInit(CODEC_AVC, &encBaseCfg);
    if (tContext.encoder == NULL)
    {
        printf("encoder initialization failed \n");
        goto ERR_QUIT_ENTRY;
    }

#ifdef INTANCE_TWO
    //the second encoder use the same encoding parameter .
    //so YUV format should be same as the first one
    tContext.secEncoder = encInit(CODEC_AVC, &encBaseCfg);
    if (!tContext.secEncoder)
    {
        printf("second encoder initialization failed \n");
        goto ERR_QUIT_ENTRY;
    }
    tContext.yuvFile2 = fopen(SRC_IMG_FILE_SEC, "r");
    if (!tContext.yuvFile2)
    {
        printf("yuvFile2 not created successfully \n");
        goto ERR_QUIT_ENTRY;
    }

    tContext.encodedFile2 = fopen(BITSTREAM_FILE_SEC, "wb");
    pthread_create(&tContext.encodingthread2, 0, secEncodingTask, &tContext);
#endif

    tContext.outputBuffer.buf = malloc(tContext.frameSize); //store encoded bitstream
    tContext.outputBuffer.bufSize = tContext.frameSize;
    pthread_create(&tContext.encThread, 0, encodingTask, &tContext);
    pthread_create(&tContext.srcThread, 0, srcReadTask, &tContext);
    tqueue_enQueue(&tContext.encodingTQ, GENARATE_INFO, getHeadInfo, &tContext, NULL);
    printf("post generating head info task, output buf: %p\n", tContext.outputBuffer.buf);
    pthread_join(tContext.encThread, NULL);
    pthread_join(tContext.srcThread, NULL);

#ifdef INTANCE_TWO
    pthread_join(tContext.encodingthread2, NULL);
    encUninit(tContext.secEncoder);
#endif

    quit(&tContext);
    return 0;
ERR_QUIT_ENTRY:
    quit(&tContext);
    return -1;
}

#ifdef INTANCE_TWO
void *secEncodingTask(void *context)
{
    TestContext *pTestContext = (TestContext *)context;
    EncInputFrame inputframe = {0};
    EncOutputBuffer outputbuf = {0};
    unsigned char *buf = malloc(pTestContext->frameSize);
    outputbuf.buf = malloc(pTestContext->frameSize);
    outputbuf.bufSize = pTestContext->frameSize;
    if (!buf || !outputbuf.buf)
        goto EXIT;

    inputframe.buf = buf;
    inputframe.dataSize = pTestContext->frameSize;
    bool success = true;
    EncHeaderData headInfo = {0};
    //since it is in the same thread with encoding, we reuse the buf
    headInfo.buf = outputbuf.buf;
    headInfo.bufSize = outputbuf.bufSize;
    int ret = getHeaderInfo(pTestContext->secEncoder, &headInfo);
    printf("head info bitstream is %d, \n", headInfo.bitstreamSize);
    if (ret == 0)
        saveBitstream(pTestContext->encodedFile2, headInfo.buf, headInfo.bitstreamSize);
    int maxSize = 0;
    do
    {
        ret = fread(buf, 1, pTestContext->frameSize, pTestContext->yuvFile2);
        if (ret == pTestContext->frameSize)
        {
            printf("\033[0;33m second instance encoding task \033[0m\n");
            int idx = addOneSrcFrame(pTestContext->secEncoder, &inputframe);
            if (0 == encOneFrame(pTestContext->secEncoder, idx))
            {
                getOneBitstreamFrame(pTestContext->secEncoder, &outputbuf);
                printf(" \033[0;34mencoded bitsize , %d , %dth frame encoded \033[0m\n", outputbuf.bitstreamSize, maxSize);
                maxSize++;
                saveBitstream(pTestContext->encodedFile2, outputbuf.buf, outputbuf.bitstreamSize);
            }
            else
            {
                success = false;
            }
        }
        else if (feof(pTestContext->yuvFile2))
            success = false;
    } while (success && maxSize < 1000);
EXIT:
    if (buf)
        free(buf);
    if (outputbuf.buf)
        free(outputbuf.buf);
    fclose(pTestContext->yuvFile2);
    fclose(pTestContext->encodedFile2);
    return NULL;
}
#endif

void quit(TestContext *tContext)
{
    //should free all the encoding task's framebuf in the taskqueue
    Task task = {0};
    while (-1 != tqueue_dequeue(&tContext->encodingTQ, &task))
    {
        if (task.type == ENCODING)
            freeSrcFrameBuf(task.param2);
    }
    tqueue_deinit(&tContext->encodingTQ);
    encUninit(tContext->encoder);
    if (tContext->yuvFile)
        fclose(tContext->yuvFile);
    if (tContext->encodedFile)
        fclose(tContext->encodedFile);
    if (tContext->outputBuffer.buf)
        free(tContext->outputBuffer.buf);
}

//the buffer allocation and deallocation would be managed by the app.
//it is better to avoid frequent allocation
unsigned char *allocateSrcFrameBuf(unsigned int bufSize)
{
    return (unsigned char *)malloc(bufSize);
}

void freeSrcFrameBuf(void *buf)
{
    if (buf)
        free(buf);
}

void *srcReadTask(void *context)
{
    int ret = 0;
    TestContext *pTestContext = (TestContext *)context;
    printf("src thread is started \n");
    int loop = 0;
    while (!pTestContext->exitFlag)
    {
        unsigned char *frameBuf = allocateSrcFrameBuf(pTestContext->frameSize);
        if (!frameBuf)
        {   //no space ,wait for a while and try it again
            printf("allocating buffer failed. \n");
            usleep(100);
            continue;
        }

        ret = fread(frameBuf, 1, pTestContext->frameSize, pTestContext->yuvFile);
        printf("%d , \033[0;36msize is readed out from src file, request %d  bytes \033[0m\n", ret, pTestContext->frameSize);
        if (ret == pTestContext->frameSize && loop < MAX_FRAME_SIZE)
        {
            if (loop == 10)
            {
                pTestContext->bitrate = 2000;
                tqueue_enQueue(&(pTestContext->encodingTQ), CHANGE_BR, changeBitrate, pTestContext, NULL);
                tqueue_enQueue(&(pTestContext->encodingTQ), REQUEST_I, generateIframe, pTestContext, NULL);
            }
        PUSH_AGAIN:
            // put task into encoding thread;
            if (-1 == tqueue_enQueue(&pTestContext->encodingTQ, ENCODING, encodingFrame, (void *)pTestContext, frameBuf))
            {
                //printf("src buf queue is full !!!!\n");
                usleep(3000);
                goto PUSH_AGAIN;
            }
            loop++;
        }
        else
        {
            freeSrcFrameBuf(frameBuf);
            if (loop >= MAX_FRAME_SIZE || feof(pTestContext->yuvFile))
            {
                printf("\033[47;31mpost exit task to encoding thread\033[0m\n");
                while (-1 == tqueue_enQueue(&pTestContext->encodingTQ, EOS, NULL, pTestContext, NULL))
                {
                    usleep(30);
                }
                break;
            }
            else
            {
                printf("\033[47;31mFailed in reading yuv data\033[0m from File \n");
            }
        }
    }
    return NULL;
}

// run in encoding thread
void encodingFrame(void *eContext, void *frameBuf)
{
    static int i = 0;
    printf("encoding Frame %d, \n", i);
    if (!frameBuf || !eContext)
    {
        printf("the %dth encoding failed because of null param \n", i);
        i++;
        return;
    }

    TestContext *pContext = (TestContext *)eContext;
    pContext->inputFrame.dataSize = pContext->frameSize;
    pContext->inputFrame.buf = frameBuf;
    int idx = addOneSrcFrame(pContext->encoder, &pContext->inputFrame);
    freeSrcFrameBuf(frameBuf);
    if (0 == encOneFrame(pContext->encoder, idx))
    {
        getOneBitstreamFrame(pContext->encoder, &pContext->outputBuffer);
        saveBitstream(pContext->encodedFile, pContext->outputBuffer.buf, pContext->outputBuffer.bitstreamSize);
        printf("succeed in encoding %dth Frame %d size \n", i, pContext->outputBuffer.bitstreamSize);
    }
    else
        printf("encoding failed in %dth frame\n", i);
    i++;
}

// run in encoding thread
void generateIframe(void *pContext, void *param)
{
    requestIFrame(((TestContext *)pContext)->encoder);
}

// run in encoding thread
void changeBitrate(void *pContext, void *param)
{
    changeBitRate(((TestContext *)pContext)->encoder, ((TestContext *)pContext)->bitrate);
}

// run in encoding thread
void changeFramerate(void *pContext, void *param)
{
    changeFrameRate(((TestContext *)pContext)->encoder, ((TestContext *)pContext)->framerate);
}

// run in encoding thread
void getHeadInfo(void *eContext, void *param)
{
    TestContext *pContext = (TestContext *)eContext;
    EncHeaderData headInfo = {0};
    //since it is in the same thread with encoding, we reuse the buf
    printf("get head info \n");
    headInfo.buf = pContext->outputBuffer.buf;
    headInfo.bufSize = pContext->outputBuffer.bufSize;
    int ret = getHeaderInfo(pContext->encoder, &headInfo);
    printf("head info bitstream is %d, \n", headInfo.bitstreamSize);
    if (ret == 0)
        saveBitstream(pContext->encodedFile, headInfo.buf, headInfo.bitstreamSize);
}

//getTask from taskQueue one by one. and call task.func
void *encodingTask(void *context)
{
    TestContext *tContext = (TestContext *)context;
    printf("encoding thread is started\n");
    while (!tContext->exitFlag)
    {
        Task task = {0};
        if (-1 != tqueue_dequeue(&tContext->encodingTQ, &task))
        {
            if (task.callback)
                task.callback(task.param1, task.param2);
            else
            {
                printf("encoding is completed, exit encoding thread \n");
                break;
            }
        }
        else
        {
            printf("encoding thread is in sleep\n");
            usleep(100);
        }
    }
    return NULL;
}

int saveBitstream(FILE *file, uint8_t *buf, uint32_t dataSize)
{
    if (!buf || !file)
    {
        printf("not saved into bitstream file %d \n", dataSize);
        return -1;
    }
    fwrite(buf, 1, dataSize, file);
    return 0;
}
