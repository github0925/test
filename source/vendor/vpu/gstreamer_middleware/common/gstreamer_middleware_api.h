#ifndef __GSTREAMER_MIDDLEWARE_API_H__
#define __GSTREAMER_MIDDLEWARE_API_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <glib.h>
#include <gst/gst.h>
#include <gst/video/video.h>

#include <semaphore.h>

#include "gstreamer_middleware_utils.h"
#include "hw_buffer_utils.h"

typedef enum {
    GSTREAMER_MIDDLEWARE_CODEC_MODE_DECODE,
    GSTREAMER_MIDDLEWARE_CODEC_MODE_ENCODE,
    GSTREAMER_MIDDLEWARE_CODEC_MODE_MAX,
} gstreamer_middleware_codec_mode_t;

typedef enum {
    GSTREAMER_MIDDLEWARE_CODEC_ENCODE_FORMAT_YUYV,
    GSTREAMER_MIDDLEWARE_CODEC_ENCODE_FORMAT_RGB,
    GSTREAMER_MIDDLEWARE_CODEC_ENCODE_FORMAT_I420,
    GSTREAMER_MIDDLEWARE_CODEC_ENCODE_FORMAT_MAX,
} gstreamer_middleware_codec_encode_format_t;

typedef enum {
    GSTREAMER_MIDDLEWARE_CODEC_EVNET_EOS,
    GSTREAMER_MIDDLEWARE_CODEC_EVNET_ERROR,
    GSTREAMER_MIDDLEWARE_CODEC_EVNET_STATE_CHANGED,
    GSTREAMER_MIDDLEWARE_CODEC_EVNET_DURATION,
    GSTREAMER_MIDDLEWARE_CODEC_EVNET_ASYNC_DONE,
    GSTREAMER_MIDDLEWARE_CODEC_EVENT_MAX,
} gstreamer_middleware_codec_event_t;

typedef enum {
    GSTREAMER_MIDDLEWARE_CODEC_STATE_STOPPED,
    GSTREAMER_MIDDLEWARE_CODEC_STATE_PAUSED,
    GSTREAMER_MIDDLEWARE_CODEC_STATE_PLAYING,
    GSTREAMER_MIDDLEWARE_CODEC_STATE_MAX,
} gstreamer_middleware_codec_state_t;

typedef struct {
    int fd;
    void *data;
    unsigned int size;
    unsigned int pts;
    unsigned int dts;
    unsigned int duration;
} gstreamer_middleware_output_buffer_t;

typedef struct {
    int fd;
    int is_dma_buffer;
    unsigned int buf_size;
    void *user_ptr;
    void *pool_buffer;
} gstreamer_middleware_input_buffer_t;

typedef void (*gstreamer_middleware_callbake_release_input_buffer)(
    void *private_ptr, gstreamer_middleware_input_buffer_t *buffer);
typedef void (*gstreamer_middleware_callbake_need_more_data_t)(void *private_ptr,
                                                               int is_need_more_data);
typedef int (*gstreamer_middleware_callbake_handle_output_buffer_t)(
    void *private_ptr, gstreamer_middleware_output_buffer_t *buffer);
typedef void (*gstreamer_middleware_callbake_event_t)(void *private_ptr,
                                                      gstreamer_middleware_codec_event_t event,
                                                      void *data);

typedef struct {
    int width;
    int height;
    int framerate;
    int is_input_dma_buffer;
    int encoder_bitrate;
    int input_buffer_count;
    unsigned char *decode_file_name;
    gstreamer_middleware_codec_encode_format_t encode_format;
    gstreamer_middleware_callbake_release_input_buffer release_input_buffer_callback;
    gstreamer_middleware_callbake_need_more_data_t need_more_data_callback;
    gstreamer_middleware_callbake_handle_output_buffer_t handle_output_buffer_callbake;
    gstreamer_middleware_callbake_event_t event_callback;
} gstreamer_middleware_config_t;

typedef struct {
    GstElement *pipeline;
    GstElement *appsink;
    GstElement *appsrc;
    GstElement *encodec;
    GstCaps *input_caps;
    GstBufferPool *input_pool;
} gstreamer_middleware_gst_element_t;

typedef struct {
    sem_t sem;
    pthread_t event_thread;
    GMainContext *context;
    GMainLoop *main_loop;

    gint64 duration;
    GstState pipeline_state;
} gstreamer_middleware_contrl_t;

typedef struct {
    gstreamer_middleware_codec_mode_t mode;
    gstreamer_middleware_config_t config;
    gstreamer_middleware_contrl_t contrl;
    gstreamer_middleware_gst_element_t gst_element;
    void *private_data; // for user
} gstreamer_middleware_t;

gstreamer_middleware_t *gstreamer_middleware_init(gstreamer_middleware_codec_mode_t mode);
int gstreamer_middleware_config(gstreamer_middleware_t *instance,
                                gstreamer_middleware_config_t config);
int gstreamer_middleware_set_state(gstreamer_middleware_t *instance,
                                   gstreamer_middleware_codec_state_t state);
gstreamer_middleware_codec_state_t gstreamer_middleware_get_state(gstreamer_middleware_t *instance);
int gstreamer_middleware_seek(gstreamer_middleware_t *instance, int64_t position);
int gstreamer_middleware_acquire_input_buffer(gstreamer_middleware_t *instance,
                                              gstreamer_middleware_input_buffer_t *buffer);
int gstreamer_middleware_release_input_buffer(gstreamer_middleware_t *instance,
                                              gstreamer_middleware_input_buffer_t *buffer);
int gstreamer_middleware_push_input_buffer(gstreamer_middleware_t *instance,
                                           gstreamer_middleware_input_buffer_t *buffer,
                                           unsigned int pts, unsigned duration);
int gstreamer_middleware_map_input_buffer(gstreamer_middleware_t *instance,
                                          gstreamer_middleware_input_buffer_t *buffer);
int gstreamer_middleware_unmap_input_buffer(gstreamer_middleware_t *instance,
                                            gstreamer_middleware_input_buffer_t *buffer);
int gstreamer_middleware_send_eos(gstreamer_middleware_t *instance);
int gstreamer_middleware_get_duration(gstreamer_middleware_t *instance);
int gstreamer_middleware_deinit(gstreamer_middleware_t *instance);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif //__GSTREAMER_MIDDLEWARE_API_H__