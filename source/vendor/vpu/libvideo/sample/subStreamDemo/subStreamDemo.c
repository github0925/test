#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "subStreamDemo.h"

#define MAIN_WIDTH 1920
#define MAIN_HEIGHT 1080
#define SUB_WIDTH 1280
#define SUB_HEIGHT 720

#define MAX_FRAME_SIZE 1000
FILE *yuvfp = NULL;

static int gExitFlag = 0;
g2d_dev_ops* g2dDevOps = NULL;

void freeIonFrameBuf(void *buf, int framesize, int ion_fd)
{
    if (buf)
        ionMemoryRelease(buf, framesize, ion_fd);
}

int initEncParam(TestContext *pContext, EncBaseConfig *pEncBaseCfg, char *enc_file)
{
    char out_file[64] = {0};
    int ret;

    pContext->encodedFile = fopen(enc_file, "wb");
    if (!pContext->encodedFile)
    {
        printf("-o %s file not exist \n", optarg);
        return -1;
    }
    pContext->frameSize = pEncBaseCfg->picHeight * pEncBaseCfg->picWidth * 3 / 2;

    pContext->encoder = encInit(CODEC_AVC, pEncBaseCfg);
    if (pContext->encoder == NULL)
    {
        printf("encoder initialization failed \n");
        goto ERR_QUIT_ENCINIT;
    }

    ret = ionMemoryAlloc(pContext->frameSize, &pContext->ion_fd);
    if(ret < 0)
    {
        printf("ionMemoryAlloc failed\n");
        goto ERR_QUIT_IONALLOC;
    }

    pContext->outputBuffer.buf = NULL;
    pContext->outputBuffer.buf = malloc(pContext->frameSize);
    if(pContext->outputBuffer.buf == NULL)
    {
        printf("malloc outputBuffer buf failed\n");
        goto ERR_QUIT_OUTPUT;
    }
    pContext->outputBuffer.bufSize = pContext->frameSize;

    return 0;

ERR_QUIT_OUTPUT:
    ionMemoryRelease(ionGetMemoryViraddr(pContext->frameSize, pContext->ion_fd), pContext->frameSize, pContext->ion_fd);
ERR_QUIT_IONALLOC:
    encUninit(pContext->encoder);
ERR_QUIT_ENCINIT:
    fclose(pContext->encodedFile);
    return -1;
}

int main(int argc, char **argv)
{
    //the picture name , width, height could be gotten from arg
    //ex: vpuencoder -w1080 -h720 -swaterfall_cif.yuv -b2000 -f24
    int opt, ret, loop, err;
    int yuv_size;
    extern char *optarg;
    EncBaseConfig encBaseCfg_main = {0};
    EncBaseConfig encBaseCfg_sub = {0};
    TestContext tContext_main = {0};
    TestContext tContext_sub = {0};
    G2DInput *g2dinput = NULL;
    unsigned char *frameBuf_main = NULL, *frameBuf_sub = NULL;

    loop = 0;

    encBaseCfg_main.rcMode = 1;
    encBaseCfg_main.picWidth = MAIN_WIDTH;
    encBaseCfg_main.picHeight = MAIN_HEIGHT;

    encBaseCfg_main.bitRate = 4000;
    encBaseCfg_main.frameRate = 20;
    encBaseCfg_main.profile = -1;
    encBaseCfg_main.level = -1;
    encBaseCfg_main.srcFormat = YUV420Planar;

    encBaseCfg_sub.rcMode = 1;
    encBaseCfg_sub.picWidth = SUB_WIDTH;
    encBaseCfg_sub.picHeight = SUB_HEIGHT;

    encBaseCfg_sub.bitRate = 4000;
    encBaseCfg_sub.frameRate = 20;
    encBaseCfg_sub.profile = -1;
    encBaseCfg_sub.level = -1;
    encBaseCfg_sub.srcFormat = YUV420Planar;

    while ((opt = getopt(argc, argv, "mw:mh:sw:sh:mn:sn:i")) != -1)
    {
        switch (opt)
        {
        case 'mw':
            printf("%s , \n", optarg);
            encBaseCfg_main.picWidth = atoi(optarg);
            break;
        case 'mh':
            printf("%s , \n", optarg);
            encBaseCfg_main.frameRate = atoi(optarg);
            break;
        case 'sw':
            printf("%s , \n", optarg);
            encBaseCfg_sub.picWidth = atoi(optarg);
            break;
        case 'sh':
            printf("%s , \n", optarg);
            encBaseCfg_sub.picHeight = atoi(optarg);
            break;
        case 'mn':
            printf("%s , \n", optarg);
            encBaseCfg_main.srcFormat = atoi(optarg);
            break;
        case 'sn':
            printf("%s , \n", optarg);
            encBaseCfg_sub.srcFormat = atoi(optarg);
            break;
        case 'i':
            yuvfp = fopen(optarg, "r");
            if(yuvfp == NULL)
            {
                printf("yuvFile:%s not exist\n", optarg);
                exit(EXIT_FAILURE);
            }
        default:
            fprintf(stderr, "Usage: %s [-wfbhrsno] \n", argv[0]);
        }
    }

    yuv_size = encBaseCfg_main.picWidth * encBaseCfg_main.picHeight * 3 / 2;

    if(yuvfp == NULL)
    {
        yuvfp = fopen("/sdcard/1920x1080.yuv", "r");
        if(!yuvfp)
        {
            printf("yuvFile not exist\n");
            return -1;
        }
    }

    ret = ionMemoryOpen();
    if(ret < 0)
    {
        printf("CameraMemoryOpen failed\n");
        goto ERR_QUIT_ION;
    }

    ret = initEncParam(&tContext_main, &encBaseCfg_main, "/sdcard/main_stream.h264");
    if(ret < 0)
    {
        printf("initEncParam failed\n");
        goto ERR_QUIT_ENCINIT1;
    }

    ret = initEncParam(&tContext_sub, &encBaseCfg_sub, "/sdcard/sub_stream.h264");
    if(ret < 0)
    {
        printf("initEncParam failed\n");
        goto ERR_QUIT_ENCINIT2;
    }

    g2dDevOps = GetG2dDevOps();
    ret = g2dDevOps->g2dOpen();
    if(ret < 0)
    {
        printf( "g2dOpen failed\n");
        goto ERR_QUIT_G2D;
    }

    g2dinput = (G2DInput*)malloc(sizeof(G2DInput));
    memset((void *)g2dinput, 0, sizeof(G2DInput));

    g2dinput->layer_num = 1;
    g2dinput->layer[0].layer_index = 0;
    g2dinput->layer[0].src_w = g2dinput->layer[0].src_crop_w = encBaseCfg_main.picWidth;
    g2dinput->layer[0].src_h = g2dinput->layer[0].src_crop_h = encBaseCfg_main.picHeight;
    g2dinput->layer[0].src_format = DRM_FORMAT_YUV420;
    g2dinput->layer[0].alpha = 0xff;
    g2dinput->layer[0].blend_mode = BLEND_PIXEL_NONE;
    g2dinput->layer[0].zpos = 0;

    g2dinput->dst_w = g2dinput->dst_crop_w = encBaseCfg_sub.picWidth;
    g2dinput->dst_h = g2dinput->dst_crop_h = encBaseCfg_sub.picHeight;
    g2dinput->dst_format = DRM_FORMAT_YUV420;

    frameBuf_main = (unsigned char *)ionGetMemoryViraddr(tContext_main.frameSize, tContext_main.ion_fd);
    if (!frameBuf_main)
    {   //no space ,wait for a while and try it again
        printf("allocating 1080p buffer failed. \n");
        goto ERR_QUIT_GETMEMORY1;
    }

    frameBuf_sub = (unsigned char *)ionGetMemoryViraddr(tContext_sub.frameSize, tContext_sub.ion_fd);
    if (!frameBuf_sub)
    {   //no space ,wait for a while and try it again
        printf("allocating 720p buffer failed. \n");
        goto ERR_QUIT_GETMEMORY1;
    }

    printf("into file read\n");
    getHeadInfo((void*)&tContext_main, NULL);
    getHeadInfo((void*)&tContext_sub, NULL);

    while (!gExitFlag)
    {
        g2dinput->layer[0].src_fd = tContext_main.ion_fd;
        g2dinput->dst_fd = tContext_sub.ion_fd;

        ret = fread(frameBuf_main, 1, yuv_size, yuvfp);
        printf("%d , \033[0;36msize is readed out from src file, request %d  bytes \033[0m\n", ret, yuv_size);
        if(ret <= 0)
        {
            printf("file is read over\n");
            gExitFlag = 1;
            break;
        }

        if (ret == yuv_size && loop < MAX_FRAME_SIZE)
        {
            // 通过fd形式对图像进行缩放
            ret = g2dDevOps->g2dConvert(g2dinput);
            if(ret < 0)
            {
                printf("g2dConvert failed\n");
            }

            if(((loop++) % 15) == 0)
            {
                generateIframe((void *)&tContext_main, NULL);
                generateIframe((void *)&tContext_sub, NULL);
            }

            encodingFrame((void* )&tContext_main, (void*)frameBuf_main);
            encodingFrame((void* )&tContext_sub, (void*)frameBuf_sub);

            loop++;
        }
        usleep(1000 * 20);
    }

    quit(&tContext_main);
    quit(&tContext_sub);
    free(g2dinput);
    fclose(yuvfp);

    return 0;

ERR_QUIT_GETMEMORY1:
    g2dDevOps->g2dClose();
ERR_QUIT_G2D:
    free(tContext_sub.outputBuffer.buf);
    ionMemoryRelease(ionGetMemoryViraddr(tContext_sub.frameSize, tContext_sub.ion_fd), tContext_sub.frameSize, tContext_sub.ion_fd);
    encUninit(tContext_sub.encoder);
    fclose(tContext_sub.encodedFile);
ERR_QUIT_ENCINIT2:
    free(tContext_main.outputBuffer.buf);
    ionMemoryRelease(ionGetMemoryViraddr(tContext_main.frameSize, tContext_main.ion_fd), tContext_main.frameSize, tContext_main.ion_fd);
    encUninit(tContext_main.encoder);
    fclose(tContext_main.encodedFile);
ERR_QUIT_ENCINIT1:
    ionMemoryClose();
    fclose(yuvfp);
ERR_QUIT_ION:
    fclose(yuvfp);

    return -1;
}

void quit(TestContext *tContext)
{
    g2dDevOps->g2dClose();
    ionMemoryRelease(ionGetMemoryViraddr(tContext->frameSize, tContext->ion_fd), tContext->frameSize, tContext->ion_fd);
    encUninit(tContext->encoder);
    fclose(tContext->encodedFile);
    ionMemoryClose();

    if (tContext->encodedFile)
        fclose(tContext->encodedFile);
    if (tContext->outputBuffer.buf)
        free(tContext->outputBuffer.buf);
}

// run in encoding thread
int encodingFrame(void *eContext, void *frameBuf)
{
    static int i = 0;
    TestContext *pContext = (TestContext *)eContext;

    printf("encoding Frame %d, framesize:%d\n", i, pContext->frameSize);
    if (!frameBuf || !eContext)
    {
        printf("the %dth encoding failed because of null param \n", i);
        i++;
        return -1;
    }

    pContext->inputFrame.dataSize = pContext->frameSize;
    pContext->inputFrame.buf = frameBuf;

    int idx = addOneSrcFrame(pContext->encoder, &pContext->inputFrame);
    if (0 == encOneFrame(pContext->encoder, idx))
    {
        getOneBitstreamFrame(pContext->encoder, &pContext->outputBuffer);
        saveBitstream(pContext->encodedFile, pContext->outputBuffer.buf, pContext->outputBuffer.bitstreamSize);
        printf("succeed in encoding %dth Frame %d size \n", i, pContext->outputBuffer.bitstreamSize);
    }
    else
        printf("encoding failed in %dth frame\n", i);
    i++;

     return 0;
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
