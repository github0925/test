#pragma once

#include <glib.h>
#include <gst/gst.h>
#include <gst/video/video.h>

#include "test_source.h"

class TestCodecSyncSouce : public TestSource
{
public:
    TestCodecSyncSouce(gstreamer_middleware_codec_mode_t mode, int width, int height,
                       gstreamer_middleware_codec_encode_format_t format =
                           GSTREAMER_MIDDLEWARE_CODEC_ENCODE_FORMAT_YUYV);
    ~TestCodecSyncSouce();

    int get_test_encode_buffer(TestSourceBuffer *buffer);
    int release_test_encode_buffer(TestSourceBuffer *buffer = 0);

    GstFlowReturn handle_encoder_output_buffer(GstElement *appSink);

private:
    int is_exit;
    int width;
    int height;
    GstElement *pipeline;
    GstElement *appsink;
    GstSample *sample;
    GstBuffer *appsink_buffer;
    GstMapInfo appsink_info;
    GstVideoFormat output_format;
    pthread_mutex_t appsink_mutex;
    pthread_cond_t appsink_condition;
    gstreamer_middleware_codec_mode_t mode;

    int create_source_pipeline();
    int release_source_pipeline();
    int create_test_decoder_source();
};