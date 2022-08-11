


#pragma once

#include <glib-object.h>
#include <gst/app/app-prelude.h>

G_BEGIN_DECLS

/* enumerations from "gstappsrc.h" */

GST_APP_API
GType gst_app_stream_type_get_type (void);
#define GST_TYPE_APP_STREAM_TYPE (gst_app_stream_type_get_type())

G_END_DECLS



