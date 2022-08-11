#ifndef __OMX_DUMP__
#define __OMX_DUMP__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define MAX_FN_LEN 100
#define DUMP_PATH "/data/semidrive/vpu/"
#define DEBUG_DEC_DUMP "debug.dec.dump"
#define DEBUG_ENC_DUMP "debug.enc.dump"
#define FILE_DEC_NAME "omx-dec-dump"
#define FILE_ENC_NAME "omx-enc-dump"
#define INPUT_DUMP_EXT "in"
#define OUTPUT_DUMP_EXT "out"

#define GENERATE_FILE_NAMES(pVpu, FILE_NAME)                                                       \
    {                                                                                              \
        struct timeval mTimeStart;                                                                 \
        gettimeofday(&mTimeStart, NULL);                                                           \
        strcpy(pVpu->dumpInputFn, "");                                                             \
        snprintf(pVpu->dumpInputFn, MAX_FN_LEN, "%s%s-%ld.%ld.%s", DUMP_PATH, FILE_NAME,           \
                mTimeStart.tv_sec, mTimeStart.tv_usec, INPUT_DUMP_EXT);                            \
        strcpy(pVpu->dumpOutputFn, "");                                                            \
        snprintf(pVpu->dumpOutputFn, MAX_FN_LEN, "%s%s-%ld.%ld.%s", DUMP_PATH, FILE_NAME,          \
                mTimeStart.tv_sec, mTimeStart.tv_usec, OUTPUT_DUMP_EXT);                           \
    }

enum {
        DUMP_NONE,
        DUMP_INPUT,
        DUMP_OUTPUT,
        DUMP_BOTH,
        DUMP_MAX
};

__attribute__((unused)) static void GetOmxDumpLevelFromProperty(const char *propertyName, long def, long *debug_dump_level)
{
#ifdef ANDROID
    *debug_dump_level = def;
    char value[PROPERTY_VALUE_MAX];
    if (property_get(propertyName, value, NULL))
    {
        char *end;
        *debug_dump_level = strtol(value, &end, 10);
    }
    printf("debug_dump_level , %ld\n", *debug_dump_level );
#elif defined GSTREAMER_LOG
    (void)(def);
    GST_DEBUG_CATEGORY_INIT(gst_omx_vpu_debug_category, propertyName, 0,
                            "debug category for gst-omx-vpu video codec base class");
#else
    *debug_dump_level = def;
    char *buffer = getenv(propertyName);
    if (buffer != NULL && *buffer != '\0')
    {
        char *end;
        *debug_dump_level = strtol(buffer, &end, 10);
        if (buffer == end)
            *debug_dump_level = def;
    }
#endif
}

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif
