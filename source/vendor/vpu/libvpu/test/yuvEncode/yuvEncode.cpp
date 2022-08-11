#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <binder/IPCThreadState.h>
#include <gui/Surface.h>
#include <media/openmax/OMX_IVCommon.h>
#include <media/stagefright/foundation/ABuffer.h>
#include <media/stagefright/foundation/AMessage.h>
#include <media/stagefright/MediaCodec.h>
#include <media/stagefright/MediaErrors.h>
#include <media/stagefright/MediaMuxer.h>
#include <media/stagefright/MediaCodecConstants.h>

#include <media/ICrypto.h>
#include <media/MediaCodecBuffer.h>

using namespace android;

typedef struct _h264EncParam{
    unsigned int width;
    unsigned int heigh;
    unsigned int fps;
    unsigned int bitrate;
    FILE         *yuvfp;
    FILE         *outfp;
    int          outfd;
    char         yuvfile[64];
    char         outfile[64];
}H264EncParam;

// Set by signal handler to stop recording.
static volatile bool gStopRequested = false;
static const char* kMimeTypeAvc = "video/avc";
static enum {
    FORMAT_MP4, FORMAT_H264, FORMAT_WEBM, FORMAT_3GPP, FORMAT_FRAMES, FORMAT_RAW_FRAMES
} gOutputFormat = FORMAT_MP4;


/* return current time in milliseconds */
long get_current_ms()
{
    struct timespec res;
    clock_gettime(CLOCK_REALTIME, &res);
    return 1000 * res.tv_sec + res.tv_nsec / 1e6;
}

static status_t initEncodeAndMuxer(H264EncParam *pH264enc, sp<MediaCodec> *pCodec, sp<MediaMuxer> *pMuxer)
{
    status_t err;
    sp<ProcessState> self;

    self = ProcessState::self();
    self->startThreadPool();

    // 1. 配置编码参数
    sp<AMessage> format = new AMessage;
    format->setInt32(KEY_WIDTH, pH264enc->width);     // 宽
    format->setInt32(KEY_HEIGHT, pH264enc->heigh);   // 高
    format->setString(KEY_MIME, kMimeTypeAvc);    // 编码名
    format->setInt32(KEY_COLOR_FORMAT, OMX_COLOR_FormatYUV420Planar); // 编码格式
    format->setInt32(KEY_BIT_RATE, pH264enc->bitrate); // 码率
    format->setFloat(KEY_FRAME_RATE, pH264enc->fps);  // 帧率
    format->setInt32(KEY_I_FRAME_INTERVAL, 3);   // I 帧

    // 2. 创建编码器
    sp<android::ALooper> looper = new android::ALooper;
    looper->setName("H264_Encoder");
    looper->start();

    sp<MediaCodec> codec;
    codec = MediaCodec::CreateByType(looper, kMimeTypeAvc, true);
    if(codec == NULL)
    {
       ALOGE("[native_camera][%s](%d) Create %s codec Failed !!!\n", __FUNCTION__, __LINE__, kMimeTypeAvc);
       return -1;
    }

    // 3. 配置编码器
    err = codec->configure(format, NULL, NULL, MediaCodec::CONFIGURE_FLAG_ENCODE);
    if (err != NO_ERROR)
    {
        ALOGE("[native_camera][%s](%d) configure %s codec at %dx%d (err=%d) Failed !!!\n", __FUNCTION__, __LINE__, kMimeTypeAvc, pH264enc->width, pH264enc->heigh, err);
        codec->release();
        codec = NULL;
        return err;
    }

    // 4. 启动编码器
    err = codec->start();
    if (err != NO_ERROR)
    {
        ALOGE("[native_camera][%s](%d) Start %s codec (err=%d) Failed !!!\n", __FUNCTION__, __LINE__, kMimeTypeAvc, err);
        codec->release();
        codec = NULL;
        return err;
    }

    AString name;
    codec->getName(&name);

    printf("Encode codec %s prepared\n", name.c_str());

    *pCodec = codec;

    // 3.3 ,读取文件内容
    pH264enc->yuvfp = fopen(pH264enc->yuvfile, "rb+");
    if(pH264enc->yuvfp == NULL)
    {
        printf("fopen %s failed\n", pH264enc->yuvfile);
        return -1;
    }

    if(gOutputFormat == FORMAT_MP4)
        strncat(pH264enc->outfile, ".mp4", 4);
    else if(gOutputFormat == FORMAT_WEBM)
        strncat(pH264enc->outfile, ".webm", 5);
    else if(gOutputFormat == FORMAT_3GPP)
        strncat(pH264enc->outfile, ".3gpp", 5);
    else
        strncat(pH264enc->outfile, ".h264", 5);

    sp<MediaMuxer> muxer = NULL;
    switch (gOutputFormat)
    {
        case FORMAT_MP4:
        case FORMAT_WEBM:
        case FORMAT_3GPP:
        {
            pH264enc->outfd = open(pH264enc->outfile, O_CREAT | O_LARGEFILE | O_TRUNC | O_RDWR, S_IRUSR | S_IWUSR);
            if(pH264enc->outfd == 0)
            {
                printf("fopen %s failed\n", pH264enc->outfile);
                return -1;
            }

            // Configure muxer.  We have to wait for the CSD blob from the encoder
            // before we can start it.
            if (gOutputFormat == FORMAT_MP4)
                muxer = new MediaMuxer(pH264enc->outfd, MediaMuxer::OUTPUT_FORMAT_MPEG_4);
            else if (gOutputFormat == FORMAT_WEBM)
                muxer = new MediaMuxer(pH264enc->outfd, MediaMuxer::OUTPUT_FORMAT_WEBM);
            else if(gOutputFormat == FORMAT_3GPP)
                muxer = new MediaMuxer(pH264enc->outfd, MediaMuxer::OUTPUT_FORMAT_THREE_GPP);
            else
                muxer = new MediaMuxer(pH264enc->outfd, MediaMuxer::OUTPUT_FORMAT_MPEG_4);

            break;
        }
        case FORMAT_H264:
        case FORMAT_FRAMES:
        case FORMAT_RAW_FRAMES:
            pH264enc->outfp = fopen(pH264enc->outfile, "wb+");
            if(pH264enc->outfp == NULL)
            {
                printf("fopen %s failed\n", pH264enc->outfile);
                return -1;
            }
        break;

        default:
        break;
    }

    *pMuxer = muxer;

    return  err;
}

static status_t runEncodeAndMuxer(H264EncParam *pH264enc, sp<MediaCodec>& encoder, sp<MediaMuxer> &muxer)
{
    status_t err;
    ssize_t trackIdx = -1;
    unsigned int frmaeNum = 0;
    int64_t o_ptsUsec = 0;

    Vector<sp<MediaCodecBuffer>> minbuffers;
    Vector<sp<MediaCodecBuffer>> moutbuffers;

    err = encoder->getInputBuffers(&minbuffers);
    if (err != NO_ERROR)
    {
        ALOGI("[native_camera][%s](%d) getInputBuffers Failed (err=%d)\n", __FUNCTION__, __LINE__, err);
        return -1;
    }

    err = encoder->getOutputBuffers(&moutbuffers);
    if (err != NO_ERROR)
    {
        ALOGI("[native_camera][%s](%d) getOutputBuffers Failed (err=%d)\n", __FUNCTION__, __LINE__, err);
        return -1;
    }

    printf("got %zu input and %zu output buffers\n", minbuffers.size(), moutbuffers.size());

    while (!gStopRequested)
    {
        size_t i_index, i_size=0, o_index, o_offset, o_size;
        uint32_t o_flags;
        uint8_t *p_input = NULL;

        // 3.1 申请一个可用的buffer index
        err = encoder->dequeueInputBuffer(&i_index, 5000ll);
        if(err != 0)
        {
            printf("dequeueInputBuffer Failed (err=%u)\n", err);
            usleep(10000);	// 10ms
            continue;
        }

        // 3.2 填充数据到 buffer 中
        //printf("get i_index  (%zu)\n", i_index);
        p_input = minbuffers[i_index]->data();

        i_size = fread(p_input, 1, pH264enc->width * pH264enc->heigh * 3 / 2, pH264enc->yuvfp);
        if(i_size == 0)
        {
            printf("file %s already encode compelet\n", pH264enc->yuvfile);
            break;
        }

        //printf("read (%s), size (%zu)\n", pH264enc->yuvfile, i_size);
        //printf("queueInputBuffer size %zu, offset:%zu\n", minbuffers[i_index]->size(), minbuffers[i_index]->offset());

        // 3.5 将输入buffer 入队列，等待编码
        err = encoder->queueInputBuffer(i_index, minbuffers[i_index]->offset(), minbuffers[i_index]->size(), frmaeNum++, MediaCodec::BUFFER_FLAG_SYNCFRAME);
        if(err != NO_ERROR)
        {
            printf("queueInputBuffer failed (%d)\n", err);
            //continue;
        }

        // 3.6 查询是否有编码后的数据,超时时间 500us
        err = encoder->dequeueOutputBuffer(&o_index, &o_offset, &o_size, &o_ptsUsec, &o_flags, 5000ll);
        switch (err)
        {
            case NO_ERROR:
            {
                if (muxer == NULL)
                {
                    printf("Got data in buffer %zu, size=%zu, pts=%" PRId64"\n", o_index, o_size, o_ptsUsec);

                    // 将buffers 中的数据写入文件中
                    fwrite(moutbuffers[o_index]->data(), 1, o_size, pH264enc->outfp);
                    // Flush the data immediately in case we're streaming.
                    // We don't want to do this if all we've written is
                    // the SPS/PPS data because mplayer gets confused.
                    if ((o_flags & MediaCodec::BUFFER_FLAG_CODECCONFIG) == 0)
                    {
                        fflush(pH264enc->outfp);
                    }
                }
                else
                {
                    assert(trackIdx != -1);

                    //long getNowTimeLong = System.currentTimeMillis();
                    //SimpleDateFormat time = new SimpleDateFormat("yyyy-MM-dd-HH-mm-ss:SSS-E-F");
                    //string result = time.format(getNowTimeLong);

                    o_ptsUsec += 1000 * 1000 / o_ptsUsec;

                    printf("%ld: Got data in buffer %zu, size=%zu, pts=%" PRId64"\n", get_current_ms(), o_index, o_size, o_ptsUsec);

                    //startTimeRender = systemTime(SYSTEM_TIME_MONOTONIC) + 33000000 - (o_ptsUsec * 1000);
                    //o_ptsUsec = (o_ptsUsec * 1000) + startTimeRender;

                    // TODO
                    sp<ABuffer> buffer = new ABuffer(moutbuffers[o_index]->data(), moutbuffers[o_index]->size());
                    err = muxer->writeSampleData(buffer, trackIdx, o_ptsUsec, o_flags);
                    if (err != NO_ERROR)
                    {
                        fprintf(stderr, "Failed writing data to muxer (err=%d)\n", err);
                        return err;
                    }
                }

                err = encoder->releaseOutputBuffer(o_index);
                if (err != NO_ERROR)
                {
                    printf("Unable to release output buffer (err=%d)\n", err);
                    gStopRequested = true;
                }
                if ((o_flags & MediaCodec::BUFFER_FLAG_EOS) != 0)
                {
                    // Not expecting EOS from SurfaceFlinger.  Go with it.
                    printf("Received end-of-stream\n");
                    gStopRequested = true;
                }
            }
            break;
            case -EAGAIN:                       // INFO_TRY_AGAIN_LATER
                printf("Got -EAGAIN, looping\n");
            break;

            case android::INFO_FORMAT_CHANGED:	  // INFO_OUTPUT_FORMAT_CHANGED
            {
                // Format includes CSD, which we must provide to muxer.
                printf("Encoder format changed");
                sp<AMessage> newFormat;
                encoder->getOutputFormat(&newFormat);
                if (muxer != NULL)
                {
                    trackIdx = muxer->addTrack(newFormat);
                    printf("Starting muxer");
                    err = muxer->start();
                    if (err != NO_ERROR)
                    {
                        fprintf(stderr, "Unable to start muxer (err=%d)\n", err);
                        return err;
                    }
                }
            }
            break;

            case android::INFO_OUTPUT_BUFFERS_CHANGED:   // INFO_OUTPUT_BUFFERS_CHANGED
            {
                // Not expected for an encoder; handle it anyway.
                printf("Encoder buffers changed\n");
                err = encoder->getOutputBuffers(&moutbuffers);
                if (err != NO_ERROR)
                {
                    fprintf(stderr,"Unable to get new output buffers (err=%d)\n", err);
                    gStopRequested = true;
                }
            }
            break;

            case INVALID_OPERATION:
                printf("dequeueOutputBuffer returned INVALID_OPERATION");
                gStopRequested = true;
            break;
            default:
                printf("Got weird result %d from dequeueOutputBuffer\n", err);
            break;
        }
    }

    printf("H264 Encoder Complete, and output file to %s direct\n", pH264enc->outfile);
    return NO_ERROR;
}

static status_t deinitEncodeAndMuxer(H264EncParam *pH264enc, sp<MediaCodec>& encoder, sp<MediaMuxer> &muxer)
{
    status_t err = NO_ERROR;

    if (encoder != NULL)
        encoder->stop();

    if (muxer != NULL)
    {
        err = muxer->stop();
        if(pH264enc->outfd != 0)
            close(pH264enc->outfd);
    }

    if(pH264enc->outfp != NULL)
    {
        fflush(pH264enc->outfp);
        fclose(pH264enc->outfp);
    }

    if(pH264enc->yuvfp != NULL)
        fclose(pH264enc->yuvfp);

    if(encoder != NULL)
        encoder->release();

    muxer = NULL;
    encoder = NULL;
    pH264enc->outfp = NULL;
    pH264enc->yuvfp = NULL;
    pH264enc->outfd = 0;

    return err;
}

int main(int argc, char **argv)
{
    H264EncParam h264enc;
    AString err_str;
    status_t err;

    // H264 Encoder
    sp<MediaCodec> encoder = NULL;
    sp<MediaMuxer> muxer = NULL;

    if(argc < 4)
    {
        printf("sample: ./yuvEncode 1280 720 mp4\n");
        printf("Copy 1280x720.yuv to /sdcard/ first, output /sdcard/1280.720.mp4 file,\n 	\
            The supported formats are as follows:mp4 h264 3gpp	\n \
            Before running yuvEncode, please run command as follow:  \n \
                \t cp 1280x720.yuv to /sdcard/		\n \
                \t (1280x720.yuv stored in \\172.16.1.12\\share\\test-resource\\multiMedia\\YUV)	\n \
            After running, /sdcard/1280x720.mp4 file will be generated. \n \
        \n");
        return 0;
    }

    // H264 video Format
    h264enc.width = atoi(argv[1]);
    h264enc.heigh = atoi(argv[2]);
    h264enc.fps = 25;
    h264enc.bitrate = 2000000;
    h264enc.yuvfp = NULL;
    h264enc.outfp = NULL;
    h264enc.outfd = 0;

    if(strncmp(argv[3], "h264", 4) == 0)
        gOutputFormat = FORMAT_H264;
    else if(strncmp(argv[3], "webm", 4) == 0)
        gOutputFormat = FORMAT_WEBM;
    else if(strncmp(argv[3], "mp4", 3) == 0)
        gOutputFormat = FORMAT_MP4;
    else if(strncmp(argv[3], "3gpp", 3) == 0)
        gOutputFormat = FORMAT_3GPP;
    else
        gOutputFormat = FORMAT_MP4;

    sprintf(h264enc.yuvfile, "/sdcard/%dx%d.yuv", h264enc.width, h264enc.heigh);
    sprintf(h264enc.outfile, "/sdcard/%dx%d", h264enc.width, h264enc.heigh);

    // 1. 配置 和 初始化 Codec
    err = initEncodeAndMuxer(&h264enc, &encoder, &muxer);
    if(err != NO_ERROR)
    {
        printf("initEncodeAndMuxer failed\n");
        return -1;
    }

    // 2. 运行编码器
    err = runEncodeAndMuxer(&h264enc, encoder, muxer);
    if(err != NO_ERROR)
    {
        printf("runEncodeAndMuxer failed\n");
        deinitEncodeAndMuxer(&h264enc, encoder, muxer);
        return -1;
    }

    // 3. release
    err = deinitEncodeAndMuxer(&h264enc, encoder, muxer);
    if(err != NO_ERROR)
    {
        printf("deinitEncodeAndMuxer failed\n");
        return -1;
    }

    return 0;
}

