#include <string.h>
#include <gst/gst.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>

//gst-launch-1.0 filesrc location=/mnt/sda1/1280x720.yuv blocksizek=1382400 ! video/x-raw,format=I420, framerate=10/1, width=1280,height=720 ! omxh264enc  control-rate=1 target-bitrate=2097152 ! filesink location=/tmp/720p.h264
//gst-launch-1.0  filesrc location=/mnt/sda1/1280x720.yuv blocksize=1382400 ! video/x-raw,format=I420, framerate=30/1, width=1280,height=720 ! omxh264enc  control-rate=1 target-bitrate=2097152  ! video/x-h264, stream-format=byte-stream ! h264parse config-interval=1 ! mpegtsmux ! filesink location=/tmp/720p.ts
//gst-launch-1.0  filesrc location=/mnt/sda1/1280x720.yuv blocksize=1382400 ! video/x-raw,format=I420, framerate=30/1, width=1280,height=720! videoparse width=1280 height=720 framerate=30/1 format=GST_VIDEO_FORMAT_I420 ! omxh264enc  control-rate=1 target-bitrate=2097152  ! video/x-h264, stream-format=byte-stream ! h264parse config-interval=1 ! mpegtsmux ! filesink location=/tmp/720p.ts

#define MAX_PIPELINE 4   // Create 4 pipeline, yuv Convert to h264 in the same time

typedef struct _GstDataStruct
{
    GstElement *pipeline;
    GstElement *filesrc;
    GstElement *videoparse;
    GstElement *omxh264enc;
    GstElement *h264parse;
    GstElement *mpegtsmux;
    GstElement *filesink;

    GstBus *bus;
    guint sourceid;        /* To control the GSource */
    guint pipeline_id;
    GstMessage *msg;
    guint frame_width;
    guint frame_height;
    GstDebugLevel level;
} GstDataStruct;

char* getElementName(char* e_str, char *str, int id)
{
    sprintf(e_str, "%s%d", str, id);
    //printf("name:%s \n", e_str);

    return e_str;
}

void *createGstPipeline(void * args)
{
    char yuvstr[128] = {0};
    char h264str[128] = {0};
    char e_str[64] = {0};
    unsigned int frame_blockSize, frame_rate, frame_bps, num_buffers;
    GstCaps *caps_h264parse = NULL;

    GstDataStruct *GstData = (GstDataStruct *)args;

    //frame_width = 640;
    //frame_height = 480;
    frame_rate = 30;
    frame_bps = 2000000;
    frame_blockSize = GstData->frame_width * GstData->frame_height * 3 / 2;
    GstData->bus = NULL;
    GstData->pipeline = NULL;
    GstData->filesrc = NULL;
    GstData->videoparse = NULL;
    GstData->omxh264enc = NULL;
    GstData->h264parse = NULL;
    GstData->mpegtsmux = NULL;
    GstData->filesink = NULL;

    sprintf(yuvstr, "/mnt/sda1/%dx%d_%d.yuv", GstData->frame_width, GstData->frame_height, GstData->pipeline_id);
    sprintf(h264str, "/home/root/test_%d.ts", GstData->pipeline_id);

    //printf("sourceID:%u\n", GstData->sourceid);
    printf("width:%d, height:%d, rate:%d, bps:%d, filesrc:%s, fileh264:%s\n", GstData->frame_width, GstData->frame_height, frame_rate, frame_bps, yuvstr, h264str);

    gst_init (NULL, NULL);
    gst_debug_set_default_threshold(GstData->level);
    printf("=========== create pipeline ==============\n");

    GstData->pipeline           	= gst_pipeline_new (getElementName(e_str, "yuvTots", GstData->pipeline_id));
    GstData->filesrc        	   	= gst_element_factory_make ("filesrc", getElementName(e_str, "filesrc", GstData->pipeline_id));
    GstData->videoparse      		= gst_element_factory_make ("videoparse", getElementName(e_str, "videoparse", GstData->pipeline_id));
    GstData->omxh264enc      		= gst_element_factory_make ("omxh264enc", getElementName(e_str, "omxh264enc", GstData->pipeline_id));
    GstData->h264parse				= gst_element_factory_make ("h264parse", getElementName(e_str, "h264parse", GstData->pipeline_id));
    GstData->mpegtsmux	    		= gst_element_factory_make ("mpegtsmux", getElementName(e_str, "mpegtsmux", GstData->pipeline_id));
    GstData->filesink				= gst_element_factory_make ("filesink", getElementName(e_str, "filesink", GstData->pipeline_id));

    printf("gst_caps_new_simple some parameter\n");

    if (!GstData->pipeline || !GstData->filesrc || !GstData->omxh264enc || !GstData->filesink
    || !GstData->h264parse || !GstData->mpegtsmux || !GstData->videoparse)
    {
        g_printerr ("One element could not be created->.. Exit\n");
        return ;
    }

    caps_h264parse = gst_caps_new_simple("video/x-h264", "stream-format", G_TYPE_STRING, "byte-stream",
                    "alignment",G_TYPE_STRING, "au", NULL);

    printf("g_object_set some parameter\n");

    g_object_set(G_OBJECT(GstData->filesrc), "location", yuvstr, "blocksize", frame_blockSize, NULL);
    g_object_set(G_OBJECT(GstData->videoparse), "width", GstData->frame_width, "height", GstData->frame_height, "framerate", 30/1, NULL);
    g_object_set(G_OBJECT(GstData->omxh264enc), "control-rate", 1, "target-bitrate", 2097152, NULL);
    g_object_set(G_OBJECT(GstData->h264parse), "config-interval", 1, NULL);
    g_object_set(G_OBJECT(GstData->filesink), "location", h264str, NULL);

    gst_element_set_state (GstData->pipeline, GST_STATE_PAUSED);

    gst_bin_add_many(GST_BIN(GstData->pipeline), GstData->filesrc, GstData->videoparse,
    GstData->omxh264enc, GstData->h264parse, GstData->mpegtsmux, GstData->filesink, NULL);

    printf("link element\n");

    if(gst_element_link(GstData->filesrc, GstData->videoparse) != TRUE)
    {
        g_printerr ("GstData->filesrc could not link GstData->videoparse\n");
        gst_object_unref (GstData->pipeline);
        return ;
    }

    if(gst_element_link(GstData->videoparse, GstData->omxh264enc) != TRUE)
    {
        g_printerr ("GstData->filesrc could not link GstData->videoparse\n");
        gst_object_unref (GstData->pipeline);
        return ;
    }

    if(gst_element_link_filtered(GstData->omxh264enc, GstData->h264parse, caps_h264parse) != TRUE)
    {
        g_printerr ("GstData->h264parse could not link GstData->flvmux\n");
        gst_object_unref (GstData->pipeline);
        return ;
    }
    gst_caps_unref (caps_h264parse);

    if(gst_element_link(GstData->h264parse, GstData->mpegtsmux) != TRUE)
    {
        g_printerr ("GstData->avmux_mpegts could not link GstData->filesink\n");
        gst_object_unref (GstData->pipeline);
        return ;
    }

    if(gst_element_link(GstData->mpegtsmux, GstData->filesink) != TRUE)
    {
        g_printerr ("GstData->mpegtsmux could not link GstData->filesink\n");
        gst_object_unref (GstData->pipeline);
        return ;
    }

    printf("========= link filesrc -> videoparse -> omxh264enc -> h264parse -> mpegtsmux -> filesink pipeline running ==========\n");
    gst_element_set_state (GstData->pipeline, GST_STATE_PLAYING);

    GstData->bus = gst_pipeline_get_bus(GST_PIPELINE(GstData->pipeline));
    if(GstData->bus == NULL)
    {
        g_printerr ("gst_pipeline_get_bus get bus failed\n");
        gst_object_unref (GstData->pipeline);
        return ;
    }

    GstData->msg = gst_bus_timed_pop_filtered(GstData->bus, GST_CLOCK_TIME_NONE, GST_MESSAGE_ERROR | GST_MESSAGE_EOS);

    if (GstData->msg != NULL)
        gst_message_unref (GstData->msg);

    gst_object_unref(GstData->bus);
    gst_element_set_state (GstData->pipeline, GST_STATE_NULL);      // Stop pipeline to be released

    printf("Deleting pipeline\n");

    gst_object_unref (GstData->pipeline);                       // THis will also delete all pipeline elements

    pthread_exit(NULL);
    return ;
}


int main(int argc, char *argv[])
{
    int i;
    pthread_t thid[MAX_PIPELINE];
    GstDataStruct GstData[MAX_PIPELINE];

    if(argc < 3)
    {
        printf("Run Sample: ./gst_test 1280 720 4 3 \n \
            args(4):  Number of encode.             \n \
            args(3):  Gstreamer print level         \n \
            Before running gst_test, please run command as follow:      \n \
                \t export XDG_DATA_HOME=/etc/xdg        \n \
                \t mkdir /mnt/sda1/                     \n \
                \t mount /dev/mmcblk0p17 /mnt/sda1/     \n \
                \t cp 1280x720.yuv to /mnt/sda1/        \n \
                \t (1280x720.yuv stored in \\172.16.1.12\share\test-resource\multiMedia\YUV)    \n \
            After running, /home/root/test_%d.ts TS file will be generated.  \
        \n");
        return 0;
    }

    for (i=0; i<atoi(argv[3]); i++)
    {
        memset(&GstData[i], 0, sizeof(GstDataStruct));
        GstData[i].level = 0;
        GstData[i].pipeline_id = i;//atoi(argv[1]);
        GstData[i].frame_width = atoi(argv[1]);
        GstData[i].frame_height = atoi(argv[2]);

        if(argc > 4)
            GstData[i].level = (GstDebugLevel)atoi(argv[4]);

        if(pthread_create(&thid[i], NULL, (void *)createGstPipeline, &GstData[i]) != 0)
        {
            printf("pthread_create failed\n");
            return -1;
        }
    }

    printf("wait thread exit ...\n");

    for(i=0; i<atoi(argv[3]); i++)
        pthread_join(thid[i], NULL);

    printf("process exit\n");
    return 0;
}
