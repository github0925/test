#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "HwConverter.h"

#include "gstreamer_middleware_api.h"
#include "test_codec_sync_source.h"
#include "test_encode_async_dma_source.h"

//#define DUMP_TEST_SOURCE_BUFFER

typedef struct {
    int is_eos;
    int duration;
    TestSource *source;
    TestSourceBuffer output_buffer;
    gstreamer_middleware_t *instance;
    gstreamer_middleware_input_buffer_t input_buffer;
    gstreamer_middleware_config_t config;
    HwConverter *g2d;
} test_private_t;

int fastcopy_use_g2d(test_private_t *private_data, int input_fd, int output_fd, int size)
{
    int ret = 0;

    if (!private_data->g2d) {
        ERROR("invaild g2d instance");
        return -1;
    }

    ret = private_data->g2d->FastCopy(input_fd, output_fd, size);

    return ret;
}

void more_data(void *private_ptr, int is_need_more_data)
{
    DEBUG_FUNC();
    int ret = 0;
    test_private_t *private_data = (test_private_t *)private_ptr;

    if (is_need_more_data) {
        ret = private_data->source->get_test_encode_buffer(&private_data->output_buffer);
        if (ret < 0) {
            WARN("get_test_encode_buffer failed");
            return;
        }

        gstreamer_middleware_acquire_input_buffer(private_data->instance,
                                                  &private_data->input_buffer);

        if (private_data->output_buffer.size != private_data->input_buffer.buf_size) {
            ERROR("input buffer size not equal to output buffer size");
            goto release_buffer;
        }

        if (private_data->instance->config.is_input_dma_buffer) {
            // option 1, software copy
            // ret = gstreamer_middleware_map_input_buffer(private_data->instance,
            //                                             &private_data->input_buffer);
            // if (ret < 0) {
            //     ERROR("gstreamer_middleware_map_input_buffer failed");
            //     goto release_buffer;
            // }

            // memcpy(private_data->input_buffer.user_ptr, private_data->output_buffer.data,
            //        private_data->output_buffer.size);
            // gstreamer_middleware_unmap_input_buffer(private_data->instance,
            //                                         &private_data->input_buffer);

            // option 2, fast copy(use g2d)
            ret = fastcopy_use_g2d(private_data, private_data->output_buffer.fd,
                                   private_data->input_buffer.fd, private_data->output_buffer.size);

            if (ret < 0) {
                ERROR("fast copy failed");
                goto release_buffer;
            }
        } else {
            memcpy(private_data->input_buffer.user_ptr, private_data->output_buffer.data,
                   private_data->output_buffer.size);
        }

        gstreamer_middleware_push_input_buffer(private_data->instance, &private_data->input_buffer,
                                               private_data->output_buffer.pts * 1000,
                                               private_data->output_buffer.duration * 1000);

    release_buffer:
        gstreamer_middleware_release_input_buffer(private_data->instance,
                                                  &private_data->input_buffer);
        private_data->source->release_test_encode_buffer(&private_data->output_buffer);
    }

    return;
}

void release_input_buffer_done(void *private_ptr, gstreamer_middleware_input_buffer_t *buffer)
{
    test_private_t *private_data = (test_private_t *)private_ptr;
    DEBUG("private_data:%p buffer:%p", private_data, buffer);
}

void on_event(void *private_ptr, gstreamer_middleware_codec_event_t event, void *data)
{
    int ret = 0;
    test_private_t *private_data = (test_private_t *)private_ptr;

    DEBUG("event:%d", event);

    switch (event) {
    case GSTREAMER_MIDDLEWARE_CODEC_EVNET_EOS:
        DEBUG("EOS");
        private_data->is_eos = TRUE;
        break;
    case GSTREAMER_MIDDLEWARE_CODEC_EVNET_ERROR:
        /* code */
        break;
    case GSTREAMER_MIDDLEWARE_CODEC_EVNET_STATE_CHANGED: {
        gstreamer_middleware_codec_state_t *state = (gstreamer_middleware_codec_state_t *)data;
        DEBUG("current pipeline state:%d", *state);
        break;
    }
    case GSTREAMER_MIDDLEWARE_CODEC_EVNET_DURATION:
        // do nothing
        break;
    case GSTREAMER_MIDDLEWARE_CODEC_EVNET_ASYNC_DONE:
        /* code */
        break;
    default:
        break;
    }
}

int handle_output_buffer(void *private_ptr, gstreamer_middleware_output_buffer_t *buffer)
{
    test_private_t *private_data = (test_private_t *)private_ptr;
    DEBUG("data:%p size:%d pts:%d", buffer->data, buffer->size, buffer->pts);

    if (private_data->instance->mode == GSTREAMER_MIDDLEWARE_CODEC_MODE_DECODE) {
        FILE *fp = fopen("test.yuv", "ab");
        fwrite(buffer->data, 1, buffer->size, fp);
        fclose(fp);

        if (private_data->duration <= 0) {
            private_data->duration = gstreamer_middleware_get_duration(private_data->instance);
            if (private_data->duration < 0) {
                WARN("get duration failed");
            } else {
                DEBUG("duration : %d ms", private_data->duration);
            }
        }

        if (private_data->duration > 0) {
            INFO("playback timestamp: %d / %d", buffer->pts / 1000000,
                 private_data->duration / 1000);
        }
    } else {
        FILE *fp = fopen("test.h264", "ab");
        fwrite(buffer->data, 1, buffer->size, fp);
        fclose(fp);
    }

    return 0;
}

int test_decode(int width, int height, int framerate, gstreamer_middleware_codec_mode_t mode)
{

    test_private_t private_data;
    memset(&private_data, 0x0, sizeof(private_data));

    try {
        private_data.source = new TestCodecSyncSouce(mode, width, height);
    } catch (unsigned char *e) {
        ERROR("create test source failed");
        delete private_data.source;
        return -1;
    }

    gstreamer_middleware_t *instance = gstreamer_middleware_init(mode);

    private_data.config.height = height;
    private_data.config.width = width;
    private_data.config.decode_file_name = (unsigned char *)DECODE_TEST_FILE_NAME;

    private_data.config.event_callback = on_event;
    private_data.config.handle_output_buffer_callbake = handle_output_buffer;

    private_data.instance = instance;
    instance->private_data = &private_data;

    DEBUG("private_data:%p", instance->private_data);
    gstreamer_middleware_config(instance, private_data.config);
    gstreamer_middleware_set_state(instance, GSTREAMER_MIDDLEWARE_CODEC_STATE_PLAYING);

    sleep(3);

    gstreamer_middleware_set_state(instance, GSTREAMER_MIDDLEWARE_CODEC_STATE_PAUSED);
    INFO("paused");
    INFO("please input any key to restart...");
    getchar();

    gstreamer_middleware_set_state(instance, GSTREAMER_MIDDLEWARE_CODEC_STATE_PLAYING);

    sleep(1);

    int64_t pts = 8000000;
    INFO("seek to %.3f s", pts / 1000000.0);
    gstreamer_middleware_seek(instance, pts);

    // wait for eos event
    while (!private_data.is_eos) {
        sleep(1);
    }

    gstreamer_middleware_set_state(instance, GSTREAMER_MIDDLEWARE_CODEC_STATE_STOPPED);

    gstreamer_middleware_deinit(instance);

    delete private_data.source;

    return 0;
}

int test_encode(int width, int height, int framerate, int is_input_dma_buffer,
                gstreamer_middleware_codec_mode_t mode,
                gstreamer_middleware_codec_encode_format_t format)
{

    test_private_t private_data;
    memset(&private_data, 0x0, sizeof(private_data));

    private_data.g2d = HwConverter::getHwConverter();
    if (!private_data.g2d) {
        ERROR("get g2d instance failed");
        return -1;
    }

    try {
        if (is_input_dma_buffer)
            private_data.source = new TestEncodeAsyncDMASource(mode, width, height, format);
        else
            private_data.source = new TestCodecSyncSouce(mode, width, height, format);
    } catch (unsigned char *e) {
        ERROR("create test source failed");
        delete private_data.source;
        return -1;
    }

#ifdef DUMP_TEST_SOURCE_BUFFER
    int ret = 0;
    TestSourceBuffer buffer;
    FILE *fp = NULL;
    if (mode == GSTREAMER_MIDDLEWARE_CODEC_MODE_ENCODE)
        fp = fopen("raw.yuv", "a+b");
    else
        fp = fopen("test.mp4", "a+b");

    for (int i = 0; i < 10; i++) {
        ret = private_data.source->get_test_encode_buffer(&buffer);
        if (ret < 0) {
            i--;
            continue;
        }

        fwrite(buffer.data, 1, buffer.size, fp);
        DEBUG("buffer.size:%d", buffer.size);
        private_data.source->release_test_encode_buffer(&buffer);
    }
    fclose(fp);

    delete private_data.source;
    return 0;
#endif

    gstreamer_middleware_t *instance = gstreamer_middleware_init(mode);

    private_data.config.height = height;
    private_data.config.width = width;
    private_data.config.framerate = framerate;
    private_data.config.encode_format = format;

    private_data.config.need_more_data_callback = more_data;
    private_data.config.release_input_buffer_callback = release_input_buffer_done;
    private_data.config.event_callback = on_event;
    private_data.config.handle_output_buffer_callbake = handle_output_buffer;
    private_data.config.is_input_dma_buffer = is_input_dma_buffer;

    private_data.instance = instance;
    instance->private_data = &private_data;

    DEBUG("private_data:%p", instance->private_data);
    gstreamer_middleware_config(instance, private_data.config);
    gstreamer_middleware_set_state(instance, GSTREAMER_MIDDLEWARE_CODEC_STATE_PLAYING);

    sleep(3);

    gstreamer_middleware_set_state(instance, GSTREAMER_MIDDLEWARE_CODEC_STATE_PAUSED);
    INFO("paused");
    INFO("please input any key to restart...");
    getchar();

    gstreamer_middleware_set_state(instance, GSTREAMER_MIDDLEWARE_CODEC_STATE_PLAYING);
    INFO("resumed");
    INFO("please input any key to stop...");
    getchar();

    gstreamer_middleware_send_eos(instance);
    // wait for eos event
    while (!private_data.is_eos) {
        sleep(1);
    }

    gstreamer_middleware_set_state(instance, GSTREAMER_MIDDLEWARE_CODEC_STATE_STOPPED);

    gstreamer_middleware_deinit(instance);
    delete private_data.source;

    return 0;
}

int main(int argc, char **argv)
{
    printf("[%s][%d]\n", __func__, __LINE__);
    if (argc < 3) {
        INFO("\nusage: %s widthxheight_fps [mode] [is_input_dma_buffer] [encode_format]\n"
             "\t mode: 0:decode 1:encode\n"
             "\t is_input_dma_buffer: for encode, 0:system buffer, 1:dma buffer\n"
             "\t encode_format: for encode, 0:YUYV, 1:RGB, 2:I420\n",
             argv[0]);
        return 0;
    }

    int width = 0, height = 0, framerate = 0, is_input_dma_buffer = 0;
    sscanf(argv[1], "%dx%d_%d", &width, &height, &framerate);

    gstreamer_middleware_codec_mode_t mode = GSTREAMER_MIDDLEWARE_CODEC_MODE_DECODE;
    mode = (gstreamer_middleware_codec_mode_t)atoi(argv[2]);

    if (mode == GSTREAMER_MIDDLEWARE_CODEC_MODE_ENCODE) {
        if (argc < 5) {
            INFO("\nusage: %s widthxheight_fps [mode] [is_input_dma_buffer] [encode_format]\n"
                 "\t mode: 0:decode 1:encode\n"
                 "\t is_input_dma_buffer: for encode, 0:system buffer, 1:dma buffer\n"
                 "\t encode_format: for encode, 0:YUYV, 1:RGB, 2:I420\n",
                 argv[0]);
            return 0;
        }

        gstreamer_middleware_codec_encode_format_t encode_format;

        is_input_dma_buffer = atoi(argv[3]);
        encode_format = (gstreamer_middleware_codec_encode_format_t)atoi(argv[4]);

        test_encode(width, height, framerate, is_input_dma_buffer, mode, encode_format);
    } else {
        test_decode(width, height, framerate, mode);
    }

    return 0;
}
