/******************************************************************************
 *
 *  This is the main implementation file for the test case manager.
 *
 ******************************************************************************/
#include "gstreamer_test_case_api.h"

#include <assert.h>
#include <string.h>

#include "gstreamer_test_player.h"

static const char *playing_file;
gboolean gst_video_create(void *param) {
  playing_file = (char *)param;
  if (on_prepared(playing_file) != 0) return FALSE;
  ;
  return TRUE;
}

gboolean gst_video_play(void *param) {
  if (on_start(playing_file) != 0) return FALSE;
  return TRUE;
}

gboolean gst_video_pause(void *param) {
  if (on_pause() != 0) return FALSE;
  return TRUE;
}

gboolean gst_video_next(void *param) {
  playing_file = (char *)param;
  if (on_destory() != 0) {
    slog_info("gst_video_next on_destory false");
    return FALSE;
  }
  if (on_prepared(playing_file) != 0) {
    slog_info("gst_video_next on_prepared false");
    return FALSE;
  }
  if (on_start(playing_file) != 0) {
    slog_info("gst_video_next on_start false");
    return FALSE;
  }
  slog_info("gst_video_next true");
  return TRUE;
}

gboolean gst_video_seek(void *param) {
  gint64 pos = *(gint64 *)param;
  if (on_seek(pos) != 0) return FALSE;
  return TRUE;
}

gboolean gst_video_stop(void *param) {
  if (on_stop() != 0) return FALSE;
  return TRUE;
}

gboolean gst_video_wait_eos(void *param) {
  if (on_wait_eos() != 0) return FALSE;
  return TRUE;
}

gboolean gst_video_delay(void *param) {
  gint64 time = *(gint64 *)param;
  if (on_delay(time / 1000) != 0) return FALSE;
  return TRUE;
}

gboolean gst_video_destory(void *param) {
  if (on_destory() != 0) return FALSE;
  return TRUE;
}

gboolean gst_video_speed(void *param) {
  char c = *(char *)param;
  if (on_speed_player(c) != 0) return FALSE;
  return TRUE;
}

gboolean gst_video_preroll(void *param) {
  int c = *(int *)param;
  if (on_preroll_frame(c) != 0) return FALSE;
  return TRUE;
}
