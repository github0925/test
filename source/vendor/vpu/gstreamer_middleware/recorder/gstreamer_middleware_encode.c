#include "gstreamer_middleware_encode.h"

int create_encode_pipeline(gstreamer_middleware_t *instance)
{
    DEBUG_FUNC();

    gchar *pipeline_description = NULL;
    GstVideoFormat format;

    switch (instance->config.encode_format) {
    case GSTREAMER_MIDDLEWARE_CODEC_ENCODE_FORMAT_YUYV:
        format = GST_VIDEO_FORMAT_YUY2;
        break;
    case GSTREAMER_MIDDLEWARE_CODEC_ENCODE_FORMAT_RGB:
        format = GST_VIDEO_FORMAT_RGB;
        break;
    case GSTREAMER_MIDDLEWARE_CODEC_ENCODE_FORMAT_I420:
        format = GST_VIDEO_FORMAT_I420;
        break;
    default:
        ERROR("don't support format:%d", instance->config.encode_format);
        return -1;
        break;
    }

    instance->gst_element.input_caps = gst_caps_new_simple(
        "video/x-raw",
        "format", G_TYPE_STRING, gst_video_format_to_string(format),
        "width", G_TYPE_INT, instance->config.width,
        "height", G_TYPE_INT, instance->config.height,
        "framerate",GST_TYPE_FRACTION, instance->config.framerate, 1,
        NULL);

    if (instance->config.is_input_dma_buffer) {
        pipeline_description = g_strdup_printf(
            " appsrc name=encode_appsrc ! "
            " sdrvfilter ! "
            " omxh264enc name=h264enc ! "
            " h264parse ! "
            " appsink name=encode_appsink sync=false emit-signals=true ");

        gst_caps_set_features_simple(instance->gst_element.input_caps,
                                     gst_caps_features_new("memory:DMABuf", NULL));
    } else {
        pipeline_description = g_strdup_printf(
            " appsrc name=encode_appsrc ! "
            " sdrvfilter ! "
            " omxh264enc name=h264enc ! "
            " h264parse ! "
            " appsink name=encode_appsink sync=false emit-signals=true ");
    }

    instance->gst_element.pipeline = gst_parse_launch(pipeline_description, NULL);

    INFO("pipeline description =%s\n", pipeline_description);
    g_free(pipeline_description);

    if (instance->gst_element.pipeline == NULL) {
        ERROR("create encoder source pipeline error");
        return -1;
    }

    instance->gst_element.appsrc =
        gst_bin_get_by_name(GST_BIN(instance->gst_element.pipeline), "encode_appsrc");
    instance->gst_element.appsink =
        gst_bin_get_by_name(GST_BIN(instance->gst_element.pipeline), "encode_appsink");
    instance->gst_element.encodec =
        gst_bin_get_by_name(GST_BIN(instance->gst_element.pipeline), "h264enc");

    if (!instance->gst_element.appsrc ||
        !instance->gst_element.encodec) {
        ERROR("get gst element error");
        goto CREATE_ENCODER_PIPIELINE_FAILED;
    }

    if (instance->config.encoder_bitrate)
        g_object_set(instance->gst_element.encodec, "target-bitrate",
                     instance->config.encoder_bitrate);

    g_object_set(G_OBJECT(instance->gst_element.appsrc), "caps", instance->gst_element.input_caps,
                 NULL);

    return 0;

CREATE_ENCODER_PIPIELINE_FAILED:
    if (instance->gst_element.input_caps) {
        gst_caps_unref(instance->gst_element.input_caps);
        instance->gst_element.input_caps = NULL;
    }

    if (instance->gst_element.encodec) {
        gst_object_unref(instance->gst_element.encodec);
        instance->gst_element.encodec = NULL;
    }

    if (instance->gst_element.appsrc) {
        gst_object_unref(instance->gst_element.appsrc);
        instance->gst_element.appsrc = NULL;
    }

    if (instance->gst_element.appsink) {
        gst_object_unref(instance->gst_element.appsink);
        instance->gst_element.appsink = NULL;
    }

    if (instance->gst_element.pipeline) {
        gst_object_unref(instance->gst_element.pipeline);
        instance->gst_element.pipeline = NULL;
    }

    return -1;
}