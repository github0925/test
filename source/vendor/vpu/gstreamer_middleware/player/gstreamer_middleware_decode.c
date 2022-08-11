#include "gstreamer_middleware_decode.h"

#ifndef ARRAYSIZE
#define ARRAYSIZE(x) (sizeof(x) / sizeof(x[0]))
#endif

int change_omx_decode_element_rank()
{
    GstRegistry *registry = gst_registry_get();

    if (!registry) {
        ERROR("Failed to get gstreamer registry");
        return -1;
    }

    char *omx_decode_element_list[] = {"omxh264dec", "omxh263dec", "omxmpeg4dec", "omxmpeg2dec",
                                       "omxvp8dec",  "omxh265dec", "omxwmvdec",   "omxmjpegdec"};

    for (int i = 0; i < ARRAYSIZE(omx_decode_element_list); i++) {
        GstPluginFeature *feature =
            gst_registry_lookup_feature(registry, omx_decode_element_list[i]);
        if (!feature) {
            WARN("Featuer does not exist: %s", omx_decode_element_list[i]);
            continue;
        }

        gst_plugin_feature_set_rank(feature, GST_RANK_PRIMARY + 1);
        gst_registry_add_feature(registry, feature);
        gst_object_unref(feature);
    }

    return 0;
}

int create_decode_pipeline(gstreamer_middleware_t *instance)
{
    DEBUG_FUNC();

    gchar *pipeline_description = NULL;

    change_omx_decode_element_rank();

    pipeline_description = g_strdup_printf(
        " filesrc location=%s ! "
        " decodebin3 name=decodebin ! "
        " sdrvfilter width=%d height=%d ! "
        " appsink name=decode_appsink sync=true emit-signals=true ",
        instance->config.decode_file_name, instance->config.width, instance->config.height);

    instance->gst_element.pipeline = gst_parse_launch(pipeline_description, NULL);

    INFO("pipeline description =%s\n", pipeline_description);
    g_free(pipeline_description);

    if (instance->gst_element.pipeline == NULL) {
        ERROR("create encoder source pipeline error");
        return -1;
    }

    instance->gst_element.appsink =
        gst_bin_get_by_name(GST_BIN(instance->gst_element.pipeline), "decode_appsink");

    if (!instance->gst_element.appsink) {
        ERROR("get gst element error");
        goto CREATE_DECODER_PIPIELINE_FAILED;
    }

    return 0;

CREATE_DECODER_PIPIELINE_FAILED:
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