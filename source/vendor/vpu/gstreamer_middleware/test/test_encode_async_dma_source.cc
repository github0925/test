#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "test_encode_async_dma_source.h"

// #define DUMP_APPSINK_BUFFER
#define QUEUE_MAX_ELEMENT_SIZE 24

GstFlowReturn async_source_get_encoder_test_buffer(GstElement *appSink, void *userData)
{
    TestEncodeAsyncDMASource *source = (TestEncodeAsyncDMASource *)userData;

    return source->handle_encoder_output_buffer(appSink);
}

GstFlowReturn TestEncodeAsyncDMASource::handle_encoder_output_buffer(GstElement *appSink)
{
    g_signal_emit_by_name(appSink, "pull-sample", &this->sample);

    if (this->sample) {
        TestSourceBuffer buffer;
        static unsigned int output_buffer_count = 0;

        pthread_mutex_lock(&this->appsink_mutex);
        if (this->output_buffer_pool.empty()) {
            pthread_mutex_unlock(&this->appsink_mutex);
            // WARN("reach the max value of the buffer queue, drop the latest data");
            goto done;
        }
        pthread_mutex_unlock(&this->appsink_mutex);

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

        if (!gst_buffer_map(this->appsink_buffer, &this->appsink_info, GST_MAP_READ)) {
            ERROR("gst_buffer_map failed.");
            goto done;
        }

        output_buffer_count++;

        pthread_mutex_lock(&this->appsink_mutex);
        buffer = this->output_buffer_pool.front();
        this->output_buffer_pool.pop();
        pthread_mutex_unlock(&this->appsink_mutex);

        if (!buffer.bo || !buffer.data) {
            ERROR("invalid output buffer");
            goto unmap_buffer;
        }

        memcpy(buffer.data, this->appsink_info.data, this->appsink_info.size);

        buffer.size = this->appsink_info.size;
        buffer.pts = GST_BUFFER_PTS(this->appsink_buffer) / 1000;

        if (output_buffer_count == 1) {
            buffer.dts = -1;
            buffer.duration = -1;
        } else {
            buffer.dts = GST_BUFFER_PTS(this->appsink_buffer) / 1000;
            buffer.duration = GST_BUFFER_DURATION(this->appsink_buffer) / 1000;
        }

        DEBUG("this->appsink_buffer:%p buffer.data:%p buffer.size:%d this:%p", this->appsink_buffer,
              buffer.data, buffer.size, this);

        pthread_mutex_lock(&this->appsink_mutex);
        this->output_buffer_queue.push(buffer);
        pthread_mutex_unlock(&this->appsink_mutex);
    } else {
        WARN("pull-sample return NULL.");
        return GST_FLOW_OK;
    }

unmap_buffer:
    gst_buffer_unmap(this->appsink_buffer, &this->appsink_info);
    this->appsink_buffer = NULL;
done:
    gst_sample_unref(this->sample);
    this->sample = NULL;
    return GST_FLOW_OK;
}

int TestEncodeAsyncDMASource::init_output_buffer_pool()
{
    for (int i = 0; i < QUEUE_MAX_ELEMENT_SIZE; i++) {
        TestSourceBuffer buffer;
        memset(&buffer, 0x0, sizeof(buffer));
        buffer.bo = hw_buffer_create(this->width, this->height,
                                     gst_drm_format_from_video(this->output_format));
        if (!buffer.bo) {
            ERROR("dm_drm_bo_create failed");
            return -1;
        }

        hw_buffer_map(buffer.bo);
        buffer.data = buffer.bo->mapped_vaddrs[0];
        buffer.fd = buffer.bo->fds[0];

        this->output_buffer_pool.push(buffer);
    }

    return 0;
}

int TestEncodeAsyncDMASource::get_test_encode_buffer(TestSourceBuffer *buffer)
{
    DEBUG_FUNC();

    if (this->mode != GSTREAMER_MIDDLEWARE_CODEC_MODE_ENCODE) {
        ERROR("only for encode mode");
        return -1;
    }

    pthread_mutex_lock(&this->appsink_mutex);
    if (this->output_buffer_queue.empty()) {
        ERROR("no output buffer available");
        pthread_mutex_unlock(&this->appsink_mutex);
        return -1;
    }

    TestSourceBuffer output_buf = this->output_buffer_queue.front();
    *buffer = output_buf;

    this->output_buffer_queue.pop();
    pthread_mutex_unlock(&this->appsink_mutex);
    return 0;
}

int TestEncodeAsyncDMASource::release_test_encode_buffer(TestSourceBuffer *buffer)
{
    DEBUG_FUNC();

    if (this->mode != GSTREAMER_MIDDLEWARE_CODEC_MODE_ENCODE) {
        ERROR("only for encode mode");
        return -1;
    }

    pthread_mutex_lock(&this->appsink_mutex);
    this->output_buffer_pool.push(*buffer);
    pthread_mutex_unlock(&this->appsink_mutex);

    return 0;
}

int TestEncodeAsyncDMASource::release_source_pipeline()
{
    DEBUG_FUNC();

    if (this->mode == GSTREAMER_MIDDLEWARE_CODEC_MODE_ENCODE) {
        if (this->pipeline == NULL) {
            ERROR("invaild pipeline");
            return -1;
        }

        int buffer_queue_size;
        TestSourceBuffer buffer;

        gst_element_set_state(this->pipeline, GST_STATE_NULL);
        // wait pipeline state change to NULL
        gst_element_get_state(this->pipeline, NULL, NULL, GST_CLOCK_TIME_NONE);

        pthread_mutex_destroy(&this->appsink_mutex);

        buffer_queue_size = this->output_buffer_queue.size();

        for (int i = 0; i < buffer_queue_size; i++) {
            buffer = this->output_buffer_queue.front();
            this->release_test_encode_buffer(&buffer);
            this->output_buffer_queue.pop();
        }

        buffer_queue_size = this->output_buffer_pool.size();

        if (buffer_queue_size != QUEUE_MAX_ELEMENT_SIZE)
            WARN("output buffer is not all returned, there may be a memory leak");

        for (int i = 0; i < buffer_queue_size; i++) {
            buffer = this->output_buffer_pool.front();
            if (buffer.bo) {
                if (buffer.data)
                    hw_buffer_unmap(buffer.bo);

                hw_buffer_destroy(buffer.bo);
            }

            this->output_buffer_pool.pop();
        }

        gst_object_unref(this->appsink);
        this->appsink = NULL;
        gst_object_unref(this->pipeline);
        this->pipeline = NULL;
    }

    DEBUG_FUNC();
    return 0;
}

TestEncodeAsyncDMASource::TestEncodeAsyncDMASource(
    gstreamer_middleware_codec_mode_t _mode, int _width, int _height,
    gstreamer_middleware_codec_encode_format_t _format)
    : mode(_mode), width(_width), height(_height), pipeline(nullptr), appsink(nullptr),
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

TestEncodeAsyncDMASource::~TestEncodeAsyncDMASource() { this->release_source_pipeline(); }

/* for encoder, the pipeline is 'videotestsrc ! video/x-raw,width=$width,height=$height,format=RGB !
 * appsink' to get RGB input buffer for test encoder
 */

int TestEncodeAsyncDMASource::create_source_pipeline()
{
    gst_init(NULL, NULL);

    gchar *pipeline_description = NULL;

    this->mode = mode;
    if (mode == GSTREAMER_MIDDLEWARE_CODEC_MODE_DECODE) {
        ERROR("don't support GSTREAMER_MIDDLEWARE_CODEC_MODE_DECODE mode");
        return -1;
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
        goto CREATE_TEST_ENCODE_ASYNC_DMA_SOURCE_ERROR;
    }

    g_signal_connect(this->appsink, "new-sample", G_CALLBACK(async_source_get_encoder_test_buffer),
                     this);

    if (init_output_buffer_pool() < 0) {
        ERROR("init_output_buffer_pool");
        goto CREATE_TEST_ENCODE_ASYNC_DMA_SOURCE_ERROR;
    }

    pthread_mutex_init(&this->appsink_mutex, NULL);

    gst_element_set_state(this->pipeline, GST_STATE_PLAYING);

    return 0;

CREATE_TEST_ENCODE_ASYNC_DMA_SOURCE_ERROR:
    if (this->pipeline) {
        gst_object_unref(this->pipeline);
        this->pipeline = NULL;
    }

    return -1;
}