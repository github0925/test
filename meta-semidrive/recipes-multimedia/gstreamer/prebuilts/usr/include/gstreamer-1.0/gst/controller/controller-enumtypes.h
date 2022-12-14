


#pragma once

#include <glib-object.h>
#include <gst/controller/controller-prelude.h>

G_BEGIN_DECLS

/* enumerations from "gstinterpolationcontrolsource.h" */

GST_CONTROLLER_API
GType gst_interpolation_mode_get_type (void);
#define GST_TYPE_INTERPOLATION_MODE (gst_interpolation_mode_get_type())

/* enumerations from "gstlfocontrolsource.h" */

GST_CONTROLLER_API
GType gst_lfo_waveform_get_type (void);
#define GST_TYPE_LFO_WAVEFORM (gst_lfo_waveform_get_type())

G_END_DECLS



