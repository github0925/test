/******************************************************************************
 *
 *  This is the main implementation file for the test case manager.
 *
 ******************************************************************************/
#pragma once

#include <assert.h>
#include <glib.h>
#include <gst/gst.h>
#include <gtest/gtest.h>
#include <string.h>

#include <string>

#include "gstreamer_test_cmd_parser.h"
#include "gstreamer_test_player_utils.h"

using namespace std;
#define GSTREAMER_TEST_RESOURCE "/data/gstreamer_test/resource/"

gboolean gst_video_create(void *param);

gboolean gst_video_play(void *param);

gboolean gst_video_pause(void *param);

gboolean gst_video_next(void *param);

gboolean gst_video_prev(void *param);

gboolean gst_video_seek(void *param);

gboolean gst_video_stop(void *param);

gboolean gst_video_wait_eos(void *param);

gboolean gst_video_delay(void *param);

gboolean gst_video_destory(void *param);

gboolean gst_video_speed(void *param);

gboolean gst_video_preroll(void *param);