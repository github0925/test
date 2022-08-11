#include "gstreamer_middleware_api.h"
#include "gstreamer_middleware_decode.h"
#include "gstreamer_middleware_encode.h"
#include "gstreamer_middleware_pool.h"
#include "gstreamer_middleware_utils.h"

#include <gst/allocators/gstdmabuf.h>
#include <gst/video/video.h>

// #define DUMP_APPSINK_BUFFER

#define MIN_INPUT_BUFFER_NUM 3
#define DEFALUT_INPUT_BUFFER_SIZE 0x400000 // 4M

GstFlowReturn get_output_buffer(GstElement *appSink, void *userData)
{
    DEBUG_FUNC();
    static unsigned int output_buffer_count = 0;
    gstreamer_middleware_output_buffer_t buffer;
    gstreamer_middleware_t *instance = (gstreamer_middleware_t *)userData;

    GstSample *sample = NULL;
    g_signal_emit_by_name(appSink, "pull-sample", &sample);
    if (sample) {
        GstMapInfo info;
        GstBuffer *gst_buf = gst_sample_get_buffer(sample);
        GstMemory *mem = gst_buffer_peek_memory(gst_buf, 0);
        if (gst_is_dmabuf_memory(mem)) {
            buffer.fd = gst_dmabuf_memory_get_fd(mem);
        } else {
            buffer.fd = -1;
        }

        if (!gst_buffer_map(gst_buf, &info, GST_MAP_READ)) {
            ERROR("gst_buffer_map failed.");
            return GST_FLOW_OK;
        }

#ifdef DUMP_APPSINK_BUFFER
        {
            FILE *fp = fopen("appsink_buffer.data", "ab");
            fwrite(info.data, 1, info.size, fp);
            fclose(fp);
        }
#endif

        output_buffer_count++;

        buffer.data = info.data;
        buffer.size = info.size;
        buffer.pts = GST_BUFFER_PTS(gst_buf) / 1000;
        if (output_buffer_count == 1) {
            buffer.dts = -1;
            buffer.duration = -1;
        } else {
            buffer.dts = GST_BUFFER_PTS(gst_buf) / 1000;
            buffer.duration = GST_BUFFER_DURATION(gst_buf) / 1000;
        }

        instance->config.handle_output_buffer_callbake(instance->private_data, &buffer);

        gst_buffer_unmap(gst_buf, &info);
        gst_sample_unref(sample);
    } else {
        ERROR("pull-sample return NULL.");
    }

    return GST_FLOW_OK;
}

static GstBufferPool *gstreamer_middleware_setup_pool(GstCaps *caps, gsize size, guint num_buf,
                                                      gboolean is_dma_buffer)
{
    GstBufferPool *pool = NULL;
    GstStructure *structure = NULL;
    guint buf_cnt = num_buf;

    DEBUG("size:%d num_buf:%d is_dma_buffer:%d", size, num_buf, is_dma_buffer);
    pool = gstreamer_middleware_pool_new(is_dma_buffer);

    structure = gst_buffer_pool_get_config(pool);
    /*We don't support dynamically allocating buffers, so set the max buffer
       count to be the same as the min buffer count */
    gst_buffer_pool_config_set_params(structure, caps, size, buf_cnt, buf_cnt);
    if (!gst_buffer_pool_set_config(pool, structure)) {
        ERROR("gstreamer middleware pool set config fail");
        gst_object_unref(pool);
        return NULL;
    }

    return pool;
}

static gboolean gst_bus_call_back(GstBus *bus, GstMessage *msg, gpointer user_data)
{
    gstreamer_middleware_t *instance = (gstreamer_middleware_t *)user_data;
    DEBUG("message:%d", GST_MESSAGE_TYPE(msg));
    switch (GST_MESSAGE_TYPE(msg)) {
    case GST_MESSAGE_EOS:
        instance->config.event_callback(instance->private_data,
                                        GSTREAMER_MIDDLEWARE_CODEC_EVNET_EOS, NULL);
        break;
    case GST_MESSAGE_ERROR: {
        GError *error;

        if (instance->contrl.pipeline_state >= GST_STATE_NULL) {
            GstElement *filesrc = NULL;
            gst_message_parse_error(msg, &error, NULL);
            ERROR("Error: %s\n", error->message);
            g_error_free(error);

            instance->config.event_callback(instance->private_data,
                                            GSTREAMER_MIDDLEWARE_CODEC_EVNET_ERROR, NULL);
        }
        break;
    }
    case GST_MESSAGE_STATE_CHANGED:
        /* Only pay attention to messages coming from the pipeline, not its children */
        if (GST_MESSAGE_SRC(msg) == GST_OBJECT(instance->gst_element.pipeline)) {
            GstState old_state;
            GstState new_state;
            GstState pending_state;
            gstreamer_middleware_codec_state_t state;
            gst_message_parse_state_changed(msg, &old_state, &new_state, &pending_state);
            DEBUG("handle_state_change in state: %d to %d\n", old_state, new_state);

            instance->contrl.pipeline_state = new_state;
            switch (new_state) {
            case GST_STATE_PLAYING: {
                state = GSTREAMER_MIDDLEWARE_CODEC_STATE_PLAYING;
                instance->config.event_callback(
                    instance->private_data, GSTREAMER_MIDDLEWARE_CODEC_EVNET_STATE_CHANGED, &state);
                break;
            }
            case GST_STATE_PAUSED: {
                state = GSTREAMER_MIDDLEWARE_CODEC_STATE_PAUSED;
                instance->config.event_callback(
                    instance->private_data, GSTREAMER_MIDDLEWARE_CODEC_EVNET_STATE_CHANGED, &state);
                break;
            }
            case GST_STATE_READY: {
                // from pause to ready, player is stopped
                if (GST_STATE_PAUSED == old_state) {
                    state = GSTREAMER_MIDDLEWARE_CODEC_STATE_STOPPED;
                    instance->config.event_callback(instance->private_data,
                                                    GSTREAMER_MIDDLEWARE_CODEC_EVNET_STATE_CHANGED,
                                                    &state);
                }
                break;
            }
            }
        }
        break;
    case GST_MESSAGE_DURATION:
        /* Called when the duration of the media changes.
          Just mark it as unknown, so we re-query it in the next refresh. */
        instance->contrl.duration = GST_CLOCK_TIME_NONE;
        instance->config.event_callback(instance->private_data,
                                        GSTREAMER_MIDDLEWARE_CODEC_EVNET_DURATION, NULL);
        break;
    case GST_MESSAGE_ASYNC_DONE:
        // seek finish
        instance->config.event_callback(instance->private_data,
                                        GSTREAMER_MIDDLEWARE_CODEC_EVNET_ASYNC_DONE, NULL);
        break;
    default:
        break;
    }
    return TRUE;
}

static void *event_thread(void *user_data)
{
    GstBus *bus;
    gstreamer_middleware_t *instance = (gstreamer_middleware_t *)user_data;
    instance->contrl.context = g_main_context_new();
    if (!instance->contrl.context) {
        ERROR("create context failed");
        pthread_detach(pthread_self());
        sem_post(&instance->contrl.sem);
        return NULL;
    }

    g_main_context_push_thread_default(instance->contrl.context);
    bus = gst_pipeline_get_bus(GST_PIPELINE(instance->gst_element.pipeline));
    gst_bus_add_watch(bus, gst_bus_call_back, instance);
    gst_object_unref(bus);

    instance->contrl.main_loop = g_main_loop_new(instance->contrl.context, FALSE);
    sem_post(&instance->contrl.sem);

    DEBUG("running main loop");
    g_main_loop_run(instance->contrl.main_loop);
    DEBUG("exiting main loop");
    g_main_loop_unref(instance->contrl.main_loop);
    g_main_context_pop_thread_default(instance->contrl.context);
    g_main_context_unref(instance->contrl.context);

    instance->contrl.main_loop = NULL;
    instance->contrl.context = NULL;
    return NULL;
}

void more_input_buffer(GstElement *appsrc, guint unused_size, gpointer user_data)
{
    DEBUG_FUNC();
    gstreamer_middleware_t *instance = (gstreamer_middleware_t *)user_data;
    instance->config.need_more_data_callback(instance->private_data, TRUE);
}

void enough_input_buffer(GstElement *appsrc, gpointer user_data)
{
    DEBUG_FUNC();
    gstreamer_middleware_t *instance = (gstreamer_middleware_t *)user_data;
    instance->config.need_more_data_callback(instance->private_data, FALSE);
}

gstreamer_middleware_t *gstreamer_middleware_init(gstreamer_middleware_codec_mode_t mode)
{
    gstreamer_middleware_t *instance = calloc(1, sizeof(gstreamer_middleware_t));
    instance->mode = mode;

    DEBUG("instance:%p", instance);
    gst_init(NULL, NULL);
    sem_init(&instance->contrl.sem, 0, 0);

    return instance;
}

int gstreamer_middleware_config(gstreamer_middleware_t *instance,
                                gstreamer_middleware_config_t config)
{
    DEBUG_FUNC();
    if (!instance) {
        ERROR("invaild instanve");
        return -1;
    }

    if (!config.event_callback || !config.handle_output_buffer_callbake) {
        ERROR("null callback ptr");
        return -1;
    }

    if (instance->mode == GSTREAMER_MIDDLEWARE_CODEC_MODE_ENCODE &&
        (!config.need_more_data_callback || !config.release_input_buffer_callback)) {
        ERROR("null callback ptr for encode mode");
        return -1;
    }

    instance->config = config;

    return 0;
}

int gstreamer_middleware_clear_contrl_resource(gstreamer_middleware_t *instance)
{
    DEBUG_FUNC();
    if (!instance->gst_element.pipeline) {
        ERROR("invaild pipeline or main loop");
        return -1;
    }

    if (!instance->contrl.main_loop) {
        ERROR("invaild pipeline or main loop");
        return -1;
    }

    gstreamer_middleware_set_state(instance, GSTREAMER_MIDDLEWARE_CODEC_STATE_STOPPED);

    // wait for the main loop to be created successfully
    while (!g_main_loop_is_running(instance->contrl.main_loop)) {
        DEBUG("wait main loop runing");
        usleep(1000);
    }

    g_main_loop_quit(instance->contrl.main_loop);
    pthread_join(instance->contrl.event_thread, NULL);
}

int gstreamer_middleware_clear_gst_element_resource(gstreamer_middleware_t *instance)
{
    DEBUG_FUNC();
    if (!instance->gst_element.pipeline) {
        ERROR("invaild pipeline");
        return -1;
    }

    gstreamer_middleware_set_state(instance, GSTREAMER_MIDDLEWARE_CODEC_STATE_STOPPED);

    if (instance->gst_element.input_pool) {
        gst_buffer_pool_set_active(instance->gst_element.input_pool, FALSE);
        g_clear_object(&instance->gst_element.input_pool);
        instance->gst_element.input_pool = NULL;
    }

    if (instance->gst_element.input_caps) {
        gst_caps_unref(instance->gst_element.input_caps);
        instance->gst_element.input_caps = NULL;
    }

    if (instance->gst_element.appsink) {
        gst_object_unref(instance->gst_element.appsink);
        instance->gst_element.appsink = NULL;
    }

    if (instance->gst_element.encodec) {
        gst_object_unref(instance->gst_element.encodec);
        instance->gst_element.encodec = NULL;
    }

    if (instance->gst_element.appsrc) {
        gst_object_unref(instance->gst_element.appsrc);
        instance->gst_element.appsrc = NULL;
    }

    if (instance->gst_element.pipeline) {
        gst_object_unref(instance->gst_element.pipeline);
        instance->gst_element.pipeline = NULL;
    }

    return 0;
}

int gstreamer_middleware_set_state(gstreamer_middleware_t *instance,
                                   gstreamer_middleware_codec_state_t state)
{
    DEBUG_FUNC();
    int ret = 0;
    int buffer_size = 0;

    if (!instance) {
        ERROR("invaild instance");
        return -1;
    }

    if (state >= GSTREAMER_MIDDLEWARE_CODEC_STATE_MAX) {
        ERROR("invaild state");
        return -1;
    }

    if (!instance->gst_element.pipeline && state != GSTREAMER_MIDDLEWARE_CODEC_STATE_STOPPED) {
        if (instance->mode == GSTREAMER_MIDDLEWARE_CODEC_MODE_DECODE) {
            ret = create_decode_pipeline(instance);
            if (ret < 0) {
                ERROR("create_decode_pipeline failed");
                goto clear_gst_element_resource;
            }
        } else {
            ret = create_encode_pipeline(instance);
            if (ret < 0) {
                ERROR("create_encode_pipeline failed");
                goto clear_gst_element_resource;
            }

            if (instance->gst_element.input_caps) {
                GstVideoInfo vinfo;
                if (!gst_video_info_from_caps(&vinfo, instance->gst_element.input_caps)) {
                    ERROR("invaild caps");
                    goto clear_gst_element_resource;
                }
                buffer_size = vinfo.size;
            } else {
                buffer_size = DEFALUT_INPUT_BUFFER_SIZE;
            }

            if (instance->config.input_buffer_count < MIN_INPUT_BUFFER_NUM)
                instance->config.input_buffer_count = MIN_INPUT_BUFFER_NUM;

            instance->gst_element.input_pool = gstreamer_middleware_setup_pool(
                instance->gst_element.input_caps, buffer_size, instance->config.input_buffer_count,
                instance->config.is_input_dma_buffer);

            if (!instance->gst_element.input_pool) {
                ERROR("create input pool failed");
                goto clear_gst_element_resource;
            }

            gst_buffer_pool_set_active(instance->gst_element.input_pool, TRUE);

            g_signal_connect(instance->gst_element.appsrc, "need-data",
                             G_CALLBACK(more_input_buffer), instance);
            g_signal_connect(instance->gst_element.appsrc, "enough-data",
                             G_CALLBACK(enough_input_buffer), instance);
        }

        if (instance->gst_element.appsink)
            g_signal_connect(instance->gst_element.appsink, "new-sample",
                             G_CALLBACK(get_output_buffer), instance);

        ret = pthread_create(&instance->contrl.event_thread, NULL, &event_thread, instance);
        if (ret < 0) {
            ERROR("create event thread failed");
            goto clear_contrl_resource;
        }

        sem_wait(&instance->contrl.sem);

        if (!instance->contrl.main_loop) {
            ERROR("create event thread error");
            goto clear_contrl_resource;
        }
    }

    switch (state) {
    case GSTREAMER_MIDDLEWARE_CODEC_STATE_STOPPED:
        gst_element_set_state(instance->gst_element.pipeline, GST_STATE_NULL);
        break;
    case GSTREAMER_MIDDLEWARE_CODEC_STATE_PAUSED:
        gst_element_set_state(instance->gst_element.pipeline, GST_STATE_PAUSED);
        break;
    case GSTREAMER_MIDDLEWARE_CODEC_STATE_PLAYING:
        gst_element_set_state(instance->gst_element.pipeline, GST_STATE_PLAYING);
        break;
    default:
        break;
    }

    return 0;

clear_contrl_resource:
    gstreamer_middleware_clear_contrl_resource(instance);
clear_gst_element_resource:
    gstreamer_middleware_clear_gst_element_resource(instance);
    return -1;
}

gstreamer_middleware_codec_state_t gstreamer_middleware_get_state(gstreamer_middleware_t *instance)
{
    GstState current = GST_STATE_NULL;
    gstreamer_middleware_codec_state_t ret = GSTREAMER_MIDDLEWARE_CODEC_STATE_STOPPED;

    if (!instance->gst_element.pipeline) {
        ERROR("invaild pipeline");
        return ret;
    }

    gst_element_get_state(instance->gst_element.pipeline, &current, NULL, 1000 * 1000);

    switch (current) {
    case GST_STATE_PLAYING:
        ret = GSTREAMER_MIDDLEWARE_CODEC_STATE_PLAYING;
        break;
    case GST_STATE_PAUSED:
        ret = GSTREAMER_MIDDLEWARE_CODEC_STATE_PAUSED;
        break;
    default:
        break;
    }

    return ret;
}

void release_input_buffer_callback(void *private_ptr, GstreamerMiddlewarePoolBuffer *pool_buffer)
{
    gstreamer_middleware_t *instance = (gstreamer_middleware_t *)private_ptr;
    DEBUG("release_input_buffer, buf:%p", pool_buffer);

    gstreamer_middleware_input_buffer_t buffer;

    buffer.buf_size = pool_buffer->buf_size;
    if (pool_buffer->bo)
        buffer.fd = pool_buffer->bo->fds[0];
    else
        buffer.fd = 0;
    buffer.is_dma_buffer = pool_buffer->is_dma_buffer;
    buffer.pool_buffer = pool_buffer;
    buffer.user_ptr = pool_buffer->user_ptr;

    instance->config.release_input_buffer_callback(instance->private_data, &buffer);
}

int gstreamer_middleware_acquire_input_buffer(gstreamer_middleware_t *instance,
                                              gstreamer_middleware_input_buffer_t *buffer)
{
    if (!instance) {
        ERROR("invaild instance");
        return -1;
    }

    if (instance->contrl.pipeline_state <= GST_STATE_NULL) {
        ERROR("invaild pipeline state");
        return -1;
    }

    if (instance->mode != GSTREAMER_MIDDLEWARE_CODEC_MODE_ENCODE) {
        ERROR("only for encode mode");
        return -1;
    }

    GstBuffer *gst_buf;
    GstreamerMiddlewarePoolBuffer *pool_buffer;
    GstFlowReturn ret = GST_FLOW_OK;

    DEBUG_FUNC();
    ret = gst_buffer_pool_acquire_buffer(instance->gst_element.input_pool, &gst_buf, NULL);
    if (ret != GST_FLOW_OK) {
        ERROR("could not allocate buffer from pool");
        return -1;
    }

    DEBUG("gst_buf:%p", gst_buf);
    pool_buffer = gstreamer_middleware_pool_buffer_get_buffer(gst_buf);

    if (!pool_buffer) {
        ERROR("gstreamer_middleware_pool_buffer_get_buffer failed");
        return -1;
    }

    if (!pool_buffer->release_callback) {
        pool_buffer->release_callback = release_input_buffer_callback;
        pool_buffer->private_ptr = instance;
    }

    buffer->buf_size = pool_buffer->buf_size;
    if (pool_buffer->bo)
        buffer->fd = pool_buffer->bo->fds[0];
    else
        buffer->fd = 0;
    buffer->is_dma_buffer = pool_buffer->is_dma_buffer;
    buffer->pool_buffer = pool_buffer;
    buffer->user_ptr = pool_buffer->user_ptr;

    return 0;
}

int gstreamer_middleware_release_input_buffer(gstreamer_middleware_t *instance,
                                              gstreamer_middleware_input_buffer_t *buffer)
{
    if (!instance || !buffer) {
        ERROR("invaild instance");
        return -1;
    }

    if (!buffer->pool_buffer) {
        ERROR("invaild gst buffer");
        return -1;
    }

    if (instance->mode != GSTREAMER_MIDDLEWARE_CODEC_MODE_ENCODE) {
        ERROR("only for encode mode");
        return -1;
    }

    DEBUG_FUNC();

    GstreamerMiddlewarePoolBuffer *pool_buffer =
        (GstreamerMiddlewarePoolBuffer *)buffer->pool_buffer;
    gstreamer_middleware_pool_buffer_release_buffer(pool_buffer);

    memset(buffer, 0x0, sizeof(*buffer));

    return 0;
}

int gstreamer_middleware_push_input_buffer(gstreamer_middleware_t *instance,
                                           gstreamer_middleware_input_buffer_t *buffer,
                                           unsigned int pts, unsigned duration)
{
    if (!instance || !buffer) {
        ERROR("invaild instance");
        return -1;
    }

    if (!buffer->pool_buffer) {
        ERROR("invaild gst buffer");
        return -1;
    }

    if (instance->contrl.pipeline_state <= GST_STATE_NULL) {
        ERROR("invaild pipeline state");
        return -1;
    }

    if (instance->mode != GSTREAMER_MIDDLEWARE_CODEC_MODE_ENCODE) {
        ERROR("only for encode mode");
        return -1;
    }

    INFO("push_input_buffer, buffer:%p, size:%d", buffer->pool_buffer, buffer->buf_size);

    GstreamerMiddlewarePoolBuffer *pool_buffer =
        (GstreamerMiddlewarePoolBuffer *)buffer->pool_buffer;
    GstFlowReturn ret = GST_FLOW_OK;

    GST_BUFFER_PTS(pool_buffer->buffer) = pts;
    GST_BUFFER_DURATION(pool_buffer->buffer) = duration;

    g_signal_emit_by_name(instance->gst_element.appsrc, "push-buffer", pool_buffer->buffer, &ret);

    if (ret != GST_FLOW_OK) {
        ERROR("push-buffer failed");
        return -1;
    }

    return 0;
}

int gstreamer_middleware_map_input_buffer(gstreamer_middleware_t *instance,
                                          gstreamer_middleware_input_buffer_t *buffer)
{
    if (!instance || !buffer) {
        ERROR("invaild instance");
        return -1;
    }

    if (!buffer->pool_buffer) {
        ERROR("invaild gst buffer");
        return -1;
    }

    DEBUG_FUNC();

    int ret = 0;
    if (buffer->user_ptr)
        return 0;

    if (instance->config.is_input_dma_buffer) {
        ret = gstreamer_middleware_pool_buffer_map_buffer(buffer->pool_buffer);
        buffer->user_ptr = ((GstreamerMiddlewarePoolBuffer *)buffer->pool_buffer)->user_ptr;
        return ret;
    }

    return -1;
}

int gstreamer_middleware_unmap_input_buffer(gstreamer_middleware_t *instance,
                                            gstreamer_middleware_input_buffer_t *buffer)
{
    if (!instance || !buffer) {
        ERROR("invaild instance");
        return -1;
    }

    if (!buffer->pool_buffer) {
        ERROR("invaild gst buffer");
        return -1;
    }

    DEBUG_FUNC();

    if (instance->config.is_input_dma_buffer && buffer->user_ptr) {
        gstreamer_middleware_pool_buffer_unmap_buffer(buffer->pool_buffer);
        buffer->user_ptr = NULL;
    }

    return 0;
}

int gstreamer_middleware_seek(gstreamer_middleware_t *instance, int64_t position)
{
    if (!instance) {
        ERROR("invaild instance");
        return -1;
    }

    if (!instance->gst_element.pipeline) {
        ERROR("invaild pipeline");
        return -1;
    }

    if (instance->contrl.pipeline_state <= GST_STATE_NULL) {
        ERROR("invaild pipeline state");
        return -1;
    }

    if (instance->mode != GSTREAMER_MIDDLEWARE_CODEC_MODE_DECODE) {
        ERROR("only for decode mode");
        return -1;
    }

    INFO("seek to %f", position / 1000000.0);
    gst_element_seek(instance->gst_element.pipeline, 1.0, GST_FORMAT_TIME, GST_SEEK_FLAG_FLUSH,
                     GST_SEEK_TYPE_SET, position * GST_USECOND, GST_SEEK_TYPE_NONE,
                     GST_CLOCK_TIME_NONE);

    return 0;
}

/* send eos to appsrc, then, appsrc will not send need-data signal
 */
int gstreamer_middleware_send_eos(gstreamer_middleware_t *instance)
{
    if (!instance) {
        ERROR("invaild instanve");
        return -1;
    }

    if (!instance->gst_element.pipeline || !instance->gst_element.appsrc) {
        ERROR("invaild pipeline");
        return -1;
    }

    DEBUG_FUNC();
    GstFlowReturn ret = GST_FLOW_OK;
    g_signal_emit_by_name(instance->gst_element.appsrc, "end-of-stream", &ret);
    INFO("send eos to gst pipeline with ret: %d", ret);

    if (ret != GST_FLOW_OK) {
        ERROR("send end-of-stream event failed");
        return -1;
    }

    return 0;
}

int gstreamer_middleware_get_duration(gstreamer_middleware_t *instance)
{
    if (!instance) {
        ERROR("invaild instanve");
        return -1;
    }

    if (!instance->gst_element.pipeline) {
        ERROR("invaild pipeline");
        return -1;
    }

    if (instance->contrl.pipeline_state <= GST_STATE_READY) {
        ERROR("invaild pipeline state:%d to get duration", instance->contrl.pipeline_state);
        return -1;
    }

    if (instance->mode != GSTREAMER_MIDDLEWARE_CODEC_MODE_DECODE) {
        ERROR("only for decode mode");
        return -1;
    }

    DEBUG_FUNC();

    gint64 duration;

    if (!gst_element_query_duration(instance->gst_element.pipeline, GST_FORMAT_TIME, &duration)) {
        WARN("Could not query current duration\n");
        duration = -1;
    } else {
        duration = duration / GST_MSECOND;
    }

    return duration;
}

int gstreamer_middleware_deinit(gstreamer_middleware_t *instance)
{
    if (!instance) {
        ERROR("invaild instanve");
        return -1;
    }

    DEBUG_FUNC();

    gstreamer_middleware_clear_contrl_resource(instance);
    gstreamer_middleware_clear_gst_element_resource(instance);

    sem_destroy(&instance->contrl.sem);
    free(instance);
    instance = NULL;

    DEBUG_FUNC();
    return 0;
}