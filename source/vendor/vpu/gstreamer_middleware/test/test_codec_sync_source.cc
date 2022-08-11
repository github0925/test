#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "gstreamer_middleware_api.h"
#include "test_codec_sync_source.h"

// #define DUMP_APPSINK_BUFFER

GstFlowReturn get_encoder_test_buffer(GstElement *appSink, void *userData)
{
    DEBUG_FUNC();
    TestCodecSyncSouce *source = (TestCodecSyncSouce *)userData;

    return source->handle_encoder_output_buffer(appSink);
}

GstFlowReturn TestCodecSyncSouce::handle_encoder_output_buffer(GstElement *appSink)
{
    DEBUG_FUNC();
    pthread_cond_signal(&this->appsink_condition);

    pthread_mutex_lock(&this->appsink_mutex);

    while (this->sample) { // wait consumer read the output buffer
        if (this->is_exit) {
            pthread_mutex_unlock(&this->appsink_mutex);
            return GST_FLOW_OK;
        }
        pthread_cond_wait(&this->appsink_condition, &this->appsink_mutex);
    }

    g_signal_emit_by_name(appSink, "pull-sample", &this->sample);
    if (this->sample) {
        this->appsink_buffer = gst_sample_get_buffer(this->sample);
        DEBUG("this->appsink_buffer:%p this:%p this->sample:%p", this->appsink_buffer, this,
              this->sample);

#ifdef DUMP_APPSINK_BUFFER
        {
            GstMapInfo info;
            gst_buffer_map(this->appsink_buffer, &info, GST_MAP_READ);
            FILE *fp = fopen("appsink_buffer.data", "ab");
            fwrite(info.data, 1, info.size, fp);
            fclose(fp);
            gst_buffer_unmap(this->appsink_buffer, &info);
        }
#endif

    } else {
        WARN("pull-sample return NULL.");
    }

    pthread_mutex_unlock(&this->appsink_mutex);

    return GST_FLOW_OK;
}

int TestCodecSyncSouce::create_test_decoder_source()
{
    int ret = 0;
    GstBus *bus;
    GstMessage *message;
    gchar *pipeline_description = NULL;

    pipeline_description = g_strdup_printf(" videotestsrc num-buffers=1000 ! "
                                           " video/x-raw,width=%d,height=%d,format=I420 ! "
                                           " sdrvfilter ! "
                                           " omxh264enc ! "
                                           " h264parse ! "
                                           " qtmux ! "
                                           " filesink location=%s ",
                                           this->width, this->height, DECODE_TEST_FILE_NAME);

    this->pipeline = gst_parse_launch(pipeline_description, NULL);

    INFO("pipeline description =%s\n", pipeline_description);
    g_free(pipeline_description);

    if (this->pipeline == NULL) {
        ERROR("create decode pipeline error");
        return -1;
    }

    bus = gst_pipeline_get_bus(GST_PIPELINE(this->pipeline));

    gst_element_set_state(this->pipeline, GST_STATE_PLAYING);

    message = gst_bus_poll(bus, (GstMessageType)(GST_MESSAGE_ERROR | GST_MESSAGE_EOS), -1);

    if (message) {
        if (GST_MESSAGE_TYPE(message) == GST_MESSAGE_ERROR) {
            ERROR("create decode file failed");
            ret = -1;
        } else {
            INFO("create decode file success");
        }

        gst_message_unref(message);
    } else {
        ERROR("null message");
        ret = -1;
    }

    gst_object_unref(bus);
    gst_element_set_state(this->pipeline, GST_STATE_NULL);
    gst_object_unref(this->pipeline);
    this->pipeline = NULL;

    return ret;
}

int TestCodecSyncSouce::get_test_encode_buffer(TestSourceBuffer *buffer)
{
    DEBUG_FUNC();

    if (this->mode != GSTREAMER_MIDDLEWARE_CODEC_MODE_ENCODE) {
        ERROR("only for encode mode");
        return -1;
    }

    static unsigned int output_buffer_count = 0;

    pthread_mutex_lock(&this->appsink_mutex);
    while (this->appsink_buffer == NULL) {
        pthread_cond_wait(&this->appsink_condition, &this->appsink_mutex);
    }

    if (!gst_buffer_map(this->appsink_buffer, &this->appsink_info, GST_MAP_READ)) {
        ERROR("gst_buffer_map failed.");
        pthread_mutex_unlock(&this->appsink_mutex);
        return -1;
    }

    output_buffer_count++;

    memset(buffer, 0x0, sizeof(*buffer));
    buffer->data = this->appsink_info.data;
    buffer->size = this->appsink_info.size;
    buffer->pts = GST_BUFFER_PTS(this->appsink_buffer) / 1000;
    if (output_buffer_count == 1) {
        buffer->dts = -1;
        buffer->duration = -1;
    } else {
        buffer->dts = GST_BUFFER_PTS(this->appsink_buffer) / 1000;
        buffer->duration = GST_BUFFER_DURATION(this->appsink_buffer) / 1000;
    }
    pthread_mutex_unlock(&this->appsink_mutex);

    DEBUG("this->appsink_buffer:%p buffer->data:%p buffer->size:%d this:%p", this->appsink_buffer,
          buffer->data, buffer->size, this);

    return 0;
}

int TestCodecSyncSouce::release_test_encode_buffer(TestSourceBuffer *buffer)
{
    DEBUG_FUNC();

    if (this->mode != GSTREAMER_MIDDLEWARE_CODEC_MODE_ENCODE) {
        ERROR("only for encode mode");
        return -1;
    }

    pthread_mutex_lock(&this->appsink_mutex);
    if (this->appsink_info.data) {
        DEBUG("this->appsink_buffer:%p this:%p", this->appsink_buffer, this);
        gst_buffer_unmap(this->appsink_buffer, &this->appsink_info);
        this->appsink_info.data = NULL;
        this->appsink_info.size = 0;
    }

    if (this->sample) {
        gst_sample_unref(this->sample);
        this->appsink_buffer = NULL;
        this->sample = NULL;
    }
    pthread_mutex_unlock(&this->appsink_mutex);

    pthread_cond_signal(&this->appsink_condition);

    if (buffer)
        memset(buffer, 0x0, sizeof(buffer));
    return 0;
}

int TestCodecSyncSouce::release_source_pipeline()
{
    DEBUG_FUNC();

    if (this->mode == GSTREAMER_MIDDLEWARE_CODEC_MODE_ENCODE) {

        if (this->pipeline == NULL) {
            ERROR("invaild pipeline");
            return -1;
        }

        this->is_exit = TRUE;
        pthread_cond_signal(&this->appsink_condition);
        gst_element_set_state(this->pipeline, GST_STATE_NULL);

        if (this->appsink_buffer)
            this->release_test_encode_buffer();

        pthread_mutex_destroy(&this->appsink_mutex);
        pthread_cond_destroy(&this->appsink_condition);

        gst_object_unref(this->appsink);
        this->appsink = NULL;
        gst_object_unref(this->pipeline);
        this->pipeline = NULL;
    }

    return 0;
}

TestCodecSyncSouce::TestCodecSyncSouce(gstreamer_middleware_codec_mode_t _mode, int _width,
                                       int _height,
                                       gstreamer_middleware_codec_encode_format_t _format)
    : mode(_mode), width(_width), height(_height), is_exit(0), pipeline(nullptr), appsink(nullptr),
      sample(nullptr), appsink_buffer(nullptr)
{
    switch (_format) {
    case GSTREAMER_MIDDLEWARE_CODEC_ENCODE_FORMAT_YUYV:
        this->output_format = GST_VIDEO_FORMAT_YUY2;
        break;
    case GSTREAMER_MIDDLEWARE_CODEC_ENCODE_FORMAT_RGB:
        this->output_format = GST_VIDEO_FORMAT_RGB;
        break;
    case GSTREAMER_MIDDLEWARE_CODEC_ENCODE_FORMAT_I420:
        this->output_format = GST_VIDEO_FORMAT_I420;
        break;
    default:
        ERROR("don't support format:%d", _format);
        throw "don't support format";
        break;
    }

    int ret = this->create_source_pipeline();
    if (ret < 0) {
        ERROR("create source pipeline error");
        throw "create source pipeline error";
    }
}

TestCodecSyncSouce::~TestCodecSyncSouce() { this->release_source_pipeline(); }

/* for encoder, the pipeline is 'videotestsrc ! video/x-raw,width=$width,height=$height,format=RGB !
 * appsink' to get RGB input buffer for test encoder
 *
 * for decoder, the pipeline is 'videotestsrc !
 * video/x-raw,width=$width,height=$height,format=I420 ! omxh264enc ! h264parse ! qtmux ! appsink'
 * to get mp4 input file for test decoder
 *
 */

int TestCodecSyncSouce::create_source_pipeline()
{
    gst_init(NULL, NULL);

    gchar *pipeline_description = NULL;

    this->mode = mode;
    if (mode == GSTREAMER_MIDDLEWARE_CODEC_MODE_DECODE) {

        int ret = 0;
        FILE *fp = fopen(DECODE_TEST_FILE_NAME, "r");

        if (fp == NULL) {
            ret = this->create_test_decoder_source();

            if (ret < 0) {
                ERROR("create decoder test file failed");
                return -1;
            }
        } else {
            fclose(fp);
        }

        return 0;
    } else {
        pipeline_description = g_strdup_printf(
            " videotestsrc ! "
            " video/x-raw,width=%d,height=%d,format=%s ! "
            " appsink name=appsink sync=false emit-signals=true ",
            this->width, this->height, gst_video_format_to_string(this->output_format));

        this->pipeline = gst_parse_launch(pipeline_description, NULL);
    }

    INFO("pipeline description =%s\n", pipeline_description);
    g_free(pipeline_description);

    if (this->pipeline == NULL) {
        ERROR("create encoder this pipeline error");
        return -1;
    }

    this->appsink = gst_bin_get_by_name(GST_BIN(this->pipeline), "appsink");
    if (this->appsink == NULL) {
        ERROR("get appsink element error");

        goto CRATE_TEST_CODEC_SYNC_SOURCE_FAILED;
    }

    g_signal_connect(this->appsink, "new-sample", G_CALLBACK(get_encoder_test_buffer), this);

    pthread_cond_init(&this->appsink_condition, NULL);
    pthread_mutex_init(&this->appsink_mutex, NULL);

    gst_element_set_state(this->pipeline, GST_STATE_PLAYING);

    return 0;

CRATE_TEST_CODEC_SYNC_SOURCE_FAILED:
    if (this->pipeline) {
        gst_object_unref(this->pipeline);
        this->pipeline = NULL;
    }

    return -1;
}
