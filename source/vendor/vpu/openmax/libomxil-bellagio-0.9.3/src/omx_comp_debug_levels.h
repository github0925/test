/**
  src/omx_comp_debug_levels.h

  Define the level of debug prints on standard err. The different levels can
  be composed with binary OR.
  The debug levels defined here belong to OpenMAX components and IL core

  Copyright (C) 2007-2011 STMicroelectronics
  Copyright (C) 2007-2009 Nokia Corporation and/or its subsidiary(-ies).

  This library is free software; you can redistribute it and/or modify it under
  the terms of the GNU Lesser General Public License as published by the Free
  Software Foundation; either version 2.1 of the License, or (at your option)
  any later version.

  This library is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
  FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
  details.

  You should have received a copy of the GNU Lesser General Public License
  along with this library; if not, write to the Free Software Foundation, Inc.,
  51 Franklin St, Fifth Floor, Boston, MA
  02110-1301  USA

*/

#ifndef __OMX_COMP_DEBUG_LEVELS_H__
#define __OMX_COMP_DEBUG_LEVELS_H__

#include <stdio.h>
#include <stdlib.h>

#ifdef ANDROID
#include <utils/Log.h>
#include <cutils/properties.h>
#define DEBUG_LOG_CORE "debug.log.vpu.core"
#define DEBUG_LOG_COMP "debug.log.vpu.comp"
#elif defined GSTREAMER_LOG
#include <gst/gst.h>
#include <gst/gstinfo.h>
#define DEBUG_LOG_CORE "gstomxvpucore"
#define DEBUG_LOG_COMP "gstomxvpucomp"
extern GstDebugCategory * gst_omx_vpu_debug_category;
#else
#define DEBUG_LOG_CORE "DEBUG_LOG_CORE"
#define DEBUG_LOG_COMP "DEBUG_LOG_COMP"
#endif

extern long s_debug_level;

/** Remove all debug output lines
 */
#define DEB_LEV_NO_OUTPUT  0

/** Messages explaining the reason of critical errors
 */
#define DEB_LEV_ERR        1

/** Messages showing values related to the test and the component/s used
 */
#define DEB_LEV_PARAMS     2

/** Messages representing steps in the execution. These are the simple messages, because
 * they avoid iterations
 */
#define DEB_LEV_SIMPLE_SEQ 4

/** Messages representing steps in the execution. All the steps are described,
 * also with iterations. With this level of output the performances are
 * seriously compromised
 */
#define DEB_LEV_FULL_SEQ   8

/** Messages that indicates the beginning and the end of a function.
 * It can be used to trace the execution
 */
#define DEB_LEV_FUNCTION_NAME 16

/** Messages that are the default test application output. These message should be
 * shown every time
 */
#define DEFAULT_MESSAGES 32

/** All the messages - max value
 */
#define DEB_ALL_MESS   255

#ifdef CONFIG_DEBUG_LEVEL
/** \def DEBUG_LEVEL is the current level do debug output on standard err */
#define DEBUG_LEVEL (DEB_LEV_ERR | CONFIG_DEBUG_LEVEL)
#else
#define DEBUG_LEVEL (DEB_LEV_ERR)
#endif

__attribute__((unused)) static void GetDebugLevelFromProperty(const char *propertyName, long def)
{
#ifdef ANDROID
    s_debug_level = def;
    char value[PROPERTY_VALUE_MAX];
    if (property_get(propertyName, value, NULL)) {
        char *end;
        s_debug_level = strtol(value, &end, 10);
    }
#elif defined GSTREAMER_LOG
    (void)(def);
    GST_DEBUG_CATEGORY_INIT (gst_omx_vpu_debug_category, propertyName, 0, \
        "debug category for gst-omx-vpu video codec base class");
#else
    s_debug_level = def;
    char *buffer = getenv(propertyName);
    if (buffer != NULL && *buffer != '\0')
    {
        char *end;
        s_debug_level = strtol(buffer, &end, 10);
        if (buffer == end)
            s_debug_level = def;
    }
#endif
}

#if DEBUG_LEVEL > 0
#ifdef ANDROID
#undef LOG_NDEBUG
#define LOG_NDEBUG 0
#undef LOG_TAG
#define LOG_TAG "VPUOMX"
#ifdef ALOGE
#define DEBUG(n, fmt, args...) do { if (s_debug_level & (n)){ALOGE("OMX-" fmt, ##args);} } while (0) // for android 4.1 later
#endif
#ifdef LOGE
#define DEBUG(n, fmt, args...) do { if (s_debug_level & (n)){LOGE("OMX-" fmt, ##args);} } while (0)
#endif
#else
#if defined(WIN32)
#define __func__ __FUNCTION__
#define DEBUG(n, fmt, ...) do { if (DEBUG_LEVEL & (n)){fprintf(stderr, "OMX-" fmt, __VA_ARGS__);} } while (0)
#elif defined GSTREAMER_LOG
#define DEBUG(n, fmt, args...)                                          \
    do                                                                  \
    {                                                                   \
        if (gst_omx_vpu_debug_category)                                 \
        {                                                               \
            switch (n)                                                  \
            {                                                           \
            case DEB_LEV_ERR:                                           \
                GST_CAT_ERROR(gst_omx_vpu_debug_category, fmt, ##args); \
                break;                                                  \
            case DEB_LEV_PARAMS:                                        \
            case DEB_LEV_SIMPLE_SEQ:                                    \
                GST_CAT_INFO(gst_omx_vpu_debug_category, fmt, ##args);  \
                break;                                                  \
            case DEB_LEV_FULL_SEQ:                                      \
                GST_CAT_DEBUG(gst_omx_vpu_debug_category, fmt, ##args); \
                break;                                                  \
            case DEB_LEV_FUNCTION_NAME:                                 \
            case DEFAULT_MESSAGES:                                      \
                GST_CAT_LOG(gst_omx_vpu_debug_category, fmt, ##args);   \
                break;                                                  \
            default:                                                    \
                GST_CAT_TRACE(gst_omx_vpu_debug_category, fmt, ##args); \
                break;                                                  \
            }                                                           \
        }                                                               \
        else                                                            \
        {                                                               \
            GST_WARNING(fmt, ##args);                                   \
        }                                                               \
                                                                        \
    } while (0)
#else
#define DEBUG(n, fmt, args...) do { if (s_debug_level & (n)){fprintf(stderr, "OMX-" fmt, ##args);} } while (0)
#endif
#endif
#else
#define DEBUG(n, fmt, args...) {}
#endif

#endif
