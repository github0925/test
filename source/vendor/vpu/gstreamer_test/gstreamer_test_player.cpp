#include "gstreamer_test_player.h"

#include <glib.h>
#include <gst/gst.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h>
#include <unistd.h>

#include <algorithm>
#include <cstdio>
#include <string>

#include "gstreamer_test_elements.h"
#include "gstreamer_test_player_utils.h"

#define MAX_LENGTH_STRING 256

static int target_state;
int wait_target_state(int target);

struct decoder_element_table {
  const char *codec_format;
  bool support;
  const char *decoder_element;
  const char *parse_element;
};
// TODO
/*
** load hw supported decoder from vendor xml file, and instead of the
*hw_decoder_list.
*/
static struct decoder_element_table hw_decoder_list[] = {
    {"H263", true, "omxh263dec", "h263parse"},
    {"H264", true, "omxh264dec",
     "h264parse ! video/x-h264,stream-format=byte-stream"},
    {"H265", true, "omxh265dec",
     "h265parse ! video/x-h265,stream-format=byte-stream"},
    {"MPEG2", true, "omxmpeg2dec", "mpegvideoparse"},
    {"MPEG4", true, "omxmpeg4dec", "mpeg4videoparse"},
    {"VP8", true, "omxvp8dec", NULL},
    {"VP9", true, "omxvp9dec", NULL},
    {"DIVX", true, "omxdivxvideodec", NULL},
    {"DIVX311", true, "omxdivx311videodec", NULL},
    {"WMV3", true, "omxwmvdec", NULL},
    {"MJPEG", true, "omxmjpegdec", "jpegparse"},
    {0}
};

static string sink_name = "waylandsink";
static string convert_name;

static bool restore_sink_name(string name);
static bool restore_convert_name(string name);

static void print_decoder_name() {
  struct decoder_element_table *handler;

  for (handler = hw_decoder_list; handler->codec_format; handler++)
    slog_info("\t format:%s, supported:%d, element_name:%s, parse_name:%s",
              handler->codec_format, handler->support, handler->decoder_element,
              handler->parse_element);
}

// file format(demuxer)--->codec format(decoder name)--->sink
static bool restore_decoder_name(const char *format, string element_name) {
  struct decoder_element_table *handler;

  for (handler = hw_decoder_list; handler->codec_format; handler++) {
    if (!strncmp(handler->codec_format, format,
                 strlen(handler->codec_format))) {
      if (element_name.length() == 0) {
        handler->support = false;
        break;
      } else {
        handler->decoder_element = strdup(element_name.c_str());
        handler->support = true;
        return true;
      }
    }
  }

  return false;
}

// TODO load from .json configure.
void load_hw_decoder_elements() {
  auto elements = GstElements::shared();
  if (!elements) return;

  std::string decoder, caps, h265_name, h264_name, h263_name, mpeg2_name,
      mpeg4_name, vp8_name;

  elements->getVideoDecoder("video/avc", h264_name, caps);
  elements->getVideoDecoder("video/hevc", h265_name, caps);
  elements->getVideoDecoder("video/x-vnd.on2.vp8", vp8_name, caps);
  elements->getVideoDecoder("video/mpegvideo", mpeg2_name, caps);
  elements->getVideoDecoder("video/mpeg4v-es", mpeg4_name, caps);
  elements->getVideoDecoder("video/h263", h263_name, caps);
  // auto parser = elements->videoParser(mime);
  auto convert = elements->videoConverter("");
  string sink = elements->videoRenderer("");

  restore_decoder_name("H265", h265_name);
  restore_decoder_name("H264", h264_name);
  restore_decoder_name("H263", h263_name);
  restore_decoder_name("VP8", vp8_name);
  // restore_decoder_name("VP9", vp9_name);
  restore_decoder_name("MPEG2", mpeg2_name);
  restore_decoder_name("MPEG4", mpeg4_name);
  restore_sink_name(sink);
  restore_convert_name(convert);

  print_decoder_name();
}

static bool get_decoder_name(string file_name, char *decoder_name,
                             char *parse_name) {
  struct decoder_element_table *handler;

  slog_info("loading file: %s ", file_name.c_str());
  size_t index = file_name.find_last_of('_');
  string file_format = file_name.substr(0, index);
  transform(file_format.begin(), file_format.end(), file_format.begin(),
            ::toupper);
  slog_info("file format: %s ", file_format.c_str());
  for (handler = hw_decoder_list; handler->codec_format; handler++) {
    if (!strncmp(handler->codec_format, file_format.c_str(),
                 strlen(handler->codec_format)) &&
        handler->support) {
      strcpy(decoder_name, handler->decoder_element);
      if (handler->parse_element) strcpy(parse_name, handler->parse_element);
      return true;
    }
  }

  return false;
}

struct container_handler_table {
  const char *container_format;
  bool support;
  const char *demuxer_name;
  // int (*function) (const char *at_cmd);
};

static struct container_handler_table container_handlers[] = {
    {".MKV", true, "matroskademux"},
    {".MP4", true, "qtdemux"},
    {".AVI", true, "avidemux"},
    {".3GP", true, "qtdemux"},
    {".MOV", true, "qtdemux"},
    {".TS", true, "tsdemux"},
    {".M4V", true, "qtdemux"},
    {".MPG", true, ""},
    {".WEBM", true, "matroskademux"},
    {".WMV", true, "asfdemux"},
    {0}};

static bool restore_sink_name(string name) {
  sink_name = name;
  return true;
}

static bool restore_convert_name(string name) {
  convert_name = name;
  return true;
}

static bool get_sink_name(string file_name, char *name) {
  if (name == nullptr)
    return false;
  strcpy(name, sink_name.c_str());
  return true;
}

static bool get_convert_name(string file_name, char *name) {
  if (name == nullptr)
    return false;
  strcpy(name, convert_name.c_str());
  return true;
}

void sem_timed_wait(sem_t *psem, int timeout) {
  struct timespec ts;

  if (clock_gettime(CLOCK_REALTIME, &ts) < 0) return;

  ts.tv_sec += timeout;
  ts.tv_nsec += 0;

  sem_timedwait(psem, &ts);
}

// Get demuxer name and create element depends on file format
// return element name.
static bool get_demuxer_name(string file_name, char *demuxer_name) {
  struct container_handler_table *handler;

  slog_info("loading file: %s ", file_name.c_str());
  size_t end = file_name.find_last_of('.');
  string file_format = file_name.substr(end);
  transform(file_format.begin(), file_format.end(), file_format.begin(),
            ::toupper);
  for (handler = container_handlers; handler->container_format; handler++)
    if (!strncmp(handler->container_format, file_format.c_str(),
                 strlen(handler->container_format)) &&
        handler->support) {
      strcpy(demuxer_name, handler->demuxer_name);
      return true;
    }

  return false;
}

typedef struct {
  GstElement *main_pipeline;
  GstElement *sink;
  gboolean playing;
  gboolean terminate;
  gboolean seek_enabled;
  gboolean preroll_done;
  gint64 duration;
  gdouble speed_rate;
  sem_t waiteos_sem_;
  sem_t state_sem_;
  sem_t command_sem_;
  GstState current_state;
} PlayerData;

static PlayerData *p_player_data;
int wait_state_change(int sec);
int PLAYER_ELEMENT_SET_STATE(GstElement *x_element, int x_state) {
  GstStateChangeReturn ret;
  if (p_player_data->current_state == x_state) return 0;
  slog_info("setting state [%d] to [%s]\n", x_state,
            GST_ELEMENT_NAME(x_element));
  if (x_state == GST_STATE_NULL)
    target_state = GST_STATE_READY;
  else
    target_state = x_state;

  if ((ret = gst_element_set_state(x_element, (GstState)x_state)) ==
      GST_STATE_CHANGE_FAILURE) {
    slog_info("failed to set state %d to %s\n", x_state,
              GST_ELEMENT_NAME(x_element));
    return -1;
  }
  return wait_target_state(x_state);
}

static void handle_message(PlayerData *data, GstMessage *msg) {
  switch (GST_MESSAGE_TYPE(msg)) {
    case GST_MESSAGE_EOS: {
      slog_info("End of stream\n");
      data->playing = FALSE;
      sem_post(&data->waiteos_sem_);
    } break;
    case GST_MESSAGE_ERROR: {
      gchar *debug;
      GError *error;
      gst_message_parse_error(msg, &error, &debug);
      g_free(debug);
      slog_info("Error: %s\n", error->message);
      g_error_free(error);
      data->playing = FALSE;
      data->preroll_done = FALSE;
      fprintf(stderr, "Couldn't open file %s\n", strerror(errno));
      sem_post(&data->waiteos_sem_);
      break;
    }
    case GST_MESSAGE_DURATION: {
      slog_info("duration message\n");
      data->duration = GST_CLOCK_TIME_NONE;
    } break;
    case GST_MESSAGE_BUFFERING: {
      int percent;
      gst_message_parse_buffering(msg, &percent);
      slog_info("GST_MESSAGE_BUFFERING percent:%d", percent);
      if (percent == 100) {
      }
      break;
    }
    case GST_MESSAGE_STATE_CHANGED: {
      GstState old_state, new_state, pending_state;
      gst_message_parse_state_changed(msg, &old_state, &new_state,
                                      &pending_state);
      if (GST_MESSAGE_SRC(msg) != GST_OBJECT(data->main_pipeline)) {
        break;
      }
      if (old_state == new_state) break;
      data->playing = (new_state == GST_STATE_PLAYING);

      slog_info("state changed new_state:%s, target_state:%s\n",
                gst_element_state_get_name(new_state),
                gst_element_state_get_name((GstState)target_state));

      data->current_state = new_state;
      if (target_state == new_state) sem_post(&data->state_sem_);

      slog_info("Pipeline state changed from %s to %s\n",
                gst_element_state_get_name(old_state),
                gst_element_state_get_name(new_state));
      if (data->playing) {
        GstQuery *query;
        gint64 start, end;
        query = gst_query_new_seeking(GST_FORMAT_TIME);
        if (gst_element_query(data->main_pipeline, query)) {
          gst_query_parse_seeking(query, NULL, &data->seek_enabled, &start,
                                  &end);
          if (data->seek_enabled) {
            slog_info("Seeking is ENABLED from %" GST_TIME_FORMAT
                      " to %" GST_TIME_FORMAT "\n",
                      GST_TIME_ARGS(start), GST_TIME_ARGS(end));
          } else {
            slog_info("Seeking is DISABLED for this stream.\n");
          }
        } else {
          slog_info("Seeking query failed.");
        }
        gst_query_unref(query);
      }
    } break;
    case GST_MESSAGE_STEP_DONE: {
      GstFormat format;
      guint64 amount;
      gdouble rate;
      gboolean flush, intermediate;
      guint64 duration;
      gboolean eos;

      gst_message_parse_step_done(msg, &format, &amount, &rate, &flush,
                                  &intermediate, &duration, &eos);
      if (format == GST_FORMAT_DEFAULT) {
        slog_info("step done: %" GST_TIME_FORMAT
                  " skipped in %" G_GUINT64_FORMAT " frames",
                  GST_TIME_ARGS(duration), amount);
      } else {
        slog_info("step done: %" GST_TIME_FORMAT " skipped",
                  GST_TIME_ARGS(duration));
      }
      data->preroll_done = TRUE;
      sem_post(&data->waiteos_sem_);
    } break;
    default:
      break;
  }
  gst_message_unref(msg);
}

static void no_more_pads(GstElement *elem, gpointer data) {
  GstElement *element = (GstElement *)data;

  slog_info("no more pads.");
}

static void on_pad_added(GstElement *elementFoo, GstPad *pad, gpointer data) {
  GstPad *sinkpad;
  GstElement *element = (GstElement *)data;

  slog_info("Dynamic pad created, linking element");
  sinkpad = gst_element_get_static_pad(element, "sink");
  slog_info("\t Linking successful\n");

  GstCaps *new_pad_caps = NULL;
  GstStructure *new_pad_struct = NULL;
  const gchar *new_pad_type = NULL;

  // new_pad_caps = gst_pad_get_caps (pad);
  new_pad_caps = gst_pad_get_current_caps(pad);
  new_pad_struct = gst_caps_get_structure(new_pad_caps, 0);
  new_pad_type = gst_structure_get_name(new_pad_struct);

  if (g_str_has_prefix(new_pad_type, "video/")) {
    gst_pad_link(pad, sinkpad);
  }

  slog_info("paded type:%s", new_pad_type);
  gst_object_unref(sinkpad);
}

static void get_gst_element_info(const char *file_path, char *demuxer_name,
                                 char *video_decode_name,
                                 char *video_parse_name, char *video_sink_name,
                                 char *video_convert_name) {
  string path = file_path;
  size_t index = path.find_last_of('/');
  string file = path.substr(index + 1);

  if (video_decode_name) video_decode_name[0] = '\0';
  if (demuxer_name) demuxer_name[0] = '\0';
  if (video_parse_name) video_parse_name[0] = '\0';
  if (video_sink_name) video_sink_name[0] = '\0';

  if (video_convert_name) video_convert_name[0] = '\0';
  if (get_demuxer_name(file, demuxer_name) == false)
    slog_info(
        "Failed to get demuxer element name. Maybe not support the container "
        "format.");
  if (get_decoder_name(file, video_decode_name, video_parse_name) == false)
    slog_info(
        "Failed to get decoder element name. Maybe not support the video "
        "codec.");
  if (get_sink_name(file, video_sink_name) == false)
    slog_info("Failed to get sink element name.");
  if (get_convert_name(file, video_convert_name) == false)
    slog_info("Failed to get convert element name.");
}

GstElement *create_pipeline(const char *file_path) {
  /*********************
   *   GSTREAMER INIT   *
   **********************/
  char demuxer_name[MAX_LENGTH_STRING];
  char video_decode_name[MAX_LENGTH_STRING];
  char video_parse_name[MAX_LENGTH_STRING];
  char video_sink_name[MAX_LENGTH_STRING];
  char caps_string[MAX_LENGTH_STRING];

  memset(demuxer_name, 0, MAX_LENGTH_STRING);
  memset(video_decode_name, 0, MAX_LENGTH_STRING);
  memset(video_parse_name, 0, MAX_LENGTH_STRING);
  memset(video_sink_name, 0, MAX_LENGTH_STRING);

  GstElement *pipeline, *sourceVid, *demuxer, *videoDecoder, *videoSink,
      *subParser, *typefind;

  get_gst_element_info(file_path, demuxer_name, video_decode_name,
                       video_parse_name, video_sink_name, NULL);
  slog_info(
      "Get Element info demuxer element name: %s, decoder element name: %s, "
      "video parse name: %s, viddo sink name:%s",
      demuxer_name, video_decode_name, video_parse_name, video_sink_name);

  gst_init(NULL, NULL);
  /* Initialization of the basic video player GstElements */
  pipeline = gst_pipeline_new("video-player");
  sourceVid = gst_element_factory_make("filesrc", "file-source");

  if (!pipeline) {
    g_printerr("Failed to create pipeline.\n");
    return NULL;
  }
  demuxer = gst_element_factory_make(demuxer_name, "video demuxer");
  videoDecoder = gst_element_factory_make(video_decode_name, "hw decoder");
  videoSink = gst_element_factory_make(video_sink_name, "video output");
  typefind = gst_element_factory_make("typefind", "typefind");

  if (strlen(video_parse_name) > 0)
    subParser = gst_element_factory_make(video_parse_name, "video parse");

  // verifying initialization
  if (!sourceVid || !demuxer || !videoDecoder || !videoSink) {
    g_printerr("One Basic element could not be created. Exiting.\n");
    gst_object_unref(pipeline);
    return NULL;
  }

  /* Setting up the pipeline */
  /* Use parameter given by the user as a video file */
  g_object_set(G_OBJECT(sourceVid), "location", file_path, NULL);

  /* Adding GstElements to the pipeline */
  if (strlen(video_parse_name) > 0)
    gst_bin_add_many(GST_BIN(pipeline), sourceVid, demuxer, subParser,
                     videoDecoder, videoSink, NULL);
  else
    gst_bin_add_many(GST_BIN(pipeline), sourceVid, demuxer, videoDecoder,
                     videoSink, NULL);

  /* Linking GstElements to each other */
  if (!gst_element_link(sourceVid, demuxer)) {
    slog_info("Elements could not be linked.");
    gst_object_unref(pipeline);
    return NULL;
  }
  if (strlen(video_parse_name) > 0) {
    gst_element_link_many(subParser, videoDecoder, videoSink, NULL);
    g_signal_connect(demuxer, "pad-added", G_CALLBACK(on_pad_added), subParser);
    g_signal_connect(demuxer, "on-more-pads", G_CALLBACK(no_more_pads),
                     subParser);
  } else {
    gst_element_link_many(videoDecoder, videoSink, NULL);
    g_signal_connect(demuxer, "pad-added", G_CALLBACK(on_pad_added),
                     videoDecoder);
    g_signal_connect(demuxer, "on-more-pads", G_CALLBACK(no_more_pads),
                     subParser);
  }

  return pipeline;
}

GstElement *create_launch_pipeline(const char *file_path) {
  string pipeline_command = "filesrc location=";
  char demuxer_name[MAX_LENGTH_STRING];
  char video_decode_name[MAX_LENGTH_STRING];
  char video_parse_name[MAX_LENGTH_STRING];
  char video_sink_name[MAX_LENGTH_STRING];
  char video_convert_name[MAX_LENGTH_STRING];
  char caps_string[MAX_LENGTH_STRING];

  memset(demuxer_name, 0, MAX_LENGTH_STRING);
  memset(video_decode_name, 0, MAX_LENGTH_STRING);
  memset(video_parse_name, 0, MAX_LENGTH_STRING);
  memset(video_sink_name, 0, MAX_LENGTH_STRING);
  memset(video_convert_name, 0, MAX_LENGTH_STRING);

  GstElement *pipeline, *sourceVid, *demuxer, *videoDecoder, *videoSink,
      *subParser, *typefind;

  // get the demux / parse / decode / convert / sink name from the file name
  get_gst_element_info(file_path, demuxer_name, video_decode_name,
                       video_parse_name, video_sink_name, video_convert_name);
  slog_info(
      "Get Element info demuxer element name: %s, decoder element name: %s, "
      "video parse name: %s, viddo sink name:%s, convert name:%s",
      demuxer_name, video_decode_name, video_parse_name, video_sink_name,
      video_convert_name);
  gst_init(NULL, NULL);

  // create gst pipeline
  pipeline_command += file_path;
  string dex = demuxer_name;
  if (strlen(demuxer_name) > 0) pipeline_command += " ! " + dex;
  string parse = video_parse_name;
  if (strlen(video_parse_name) > 0) pipeline_command += " ! " + parse;
  string decoder = video_decode_name;
  if (strlen(video_decode_name) > 0) pipeline_command += " ! " + decoder;
  string convert = video_convert_name;
  if (strlen(video_convert_name) > 0) pipeline_command += " ! " + convert;

  string sink = video_sink_name;
  if (strlen(video_sink_name) > 0)
    pipeline_command += " ! " + sink + " name=sink";

  slog_info("gst parse launch: %s", pipeline_command.c_str());
  pipeline = gst_parse_launch(pipeline_command.c_str(), NULL);
  if (!pipeline) {
    slog_info("Failed to create pipeline.\n");
    return NULL;
  }

  return pipeline;
}

GstElement *create_auto_pipeline(const char *file_path) {
  string pipeline_command = "filesrc location=";

  gst_init(NULL, NULL);
  GstElement *pipeline;
  pipeline_command += file_path;

  pipeline_command += " ! decodebin ! waylandsink";

  slog_info("gst parse launch: %s", pipeline_command.c_str());
  pipeline = gst_parse_launch(pipeline_command.c_str(), NULL);
  if (!pipeline) {
    slog_info("Failed to create pipeline.\n");
    return NULL;
  }
  return pipeline;
}

static pthread_t player_thread;

static void *player_loop_thread(void *param) {
  PlayerData *data = (PlayerData *)param;
  GstBus *bus;
  GstMessage *msg;
  bus = gst_pipeline_get_bus(GST_PIPELINE(data->main_pipeline));

  do {
    msg = gst_bus_timed_pop_filtered(bus, 100 * GST_MSECOND, GST_MESSAGE_ANY);
    // GST_MESSAGE_STATE_CHANGED | GST_MESSAGE_ERROR | GST_MESSAGE_EOS |
    // GST_MESSAGE_DURATION | GST_MESSAGE_STEP_DONE | GST_MESSAGE_BUFFERING);
    if (msg != NULL) {
      handle_message(data, msg);
    } else {
      GstFormat fmt = GST_FORMAT_TIME;
      gint64 current;
      /* We got no message, this means the timeout expired */
      if (data->playing) {
        current = -1;

        /* Query the current position of the stream */
        if (!gst_element_query_position(data->main_pipeline, fmt, &current)) {
          slog_info("Could not query current position.\n");
        }

        if (!GST_CLOCK_TIME_IS_VALID(data->duration)) {
          if (!gst_element_query_duration(data->main_pipeline, fmt,
                                          &data->duration)) {
            slog_info("Could not query current duration.\n");
          }
        }
        /* Print current position and total duration */
        printf("Position %" GST_TIME_FORMAT " / %" GST_TIME_FORMAT "\r",
               GST_TIME_ARGS(current), GST_TIME_ARGS(data->duration));
      }
    }
  } while (!data->terminate);
  slog_info("exit player thread!!!");
  gst_object_unref(bus);
}

gboolean on_prepared(const char *test_file) {
  GstElement *pipeline;
  p_player_data = g_new0(PlayerData, 1);
  pipeline = create_launch_pipeline(
      test_file);  // get the gst pipeline from the test_file

  if (!pipeline) return -1;

  p_player_data->main_pipeline = pipeline;
  sem_init(&p_player_data->waiteos_sem_, 0, 0);
  sem_init(&p_player_data->command_sem_, 0, 0);
  sem_init(&p_player_data->state_sem_, 0, 0);
  p_player_data->terminate = FALSE;
  p_player_data->playing = FALSE;
  p_player_data->speed_rate = 1.0;
  p_player_data->duration = GST_CLOCK_TIME_NONE;
  pthread_create(&player_thread, NULL, player_loop_thread, p_player_data);
  return 0;
}

int wait_state_change(int sec) {
  GstState element_state = GST_STATE_VOID_PENDING;
  GstState pending_state = GST_STATE_VOID_PENDING;
  slog_info("wait_state_change timeout:%d", sec);
  GstStateChangeReturn res =
      gst_element_get_state(p_player_data->main_pipeline, &element_state,
                            &pending_state, sec * GST_SECOND);
  if (res == GST_STATE_CHANGE_FAILURE) {
    slog_info("Failed to get player state.\n");
    slog_info("[%s] state : %s  pending : %s \n",
              GST_ELEMENT_NAME(p_player_data->main_pipeline),
              gst_element_state_get_name(element_state),
              gst_element_state_get_name(pending_state));
    return 0;
  }
  slog_info("[%s] state change has changed: %s ",
            GST_ELEMENT_NAME(p_player_data->main_pipeline),
            gst_element_state_get_name(element_state));
  return 0;
}

int wait_target_state(int target) {
  slog_info("waiting target state:%d", target);
  int ret = 0;
  sem_timed_wait(&p_player_data->state_sem_, 10);

  if (target_state != p_player_data->current_state) {
    slog_info("Failed to change player state.");
    ret = -1;
  } else {
    slog_info("Success to change player state.");
    ret = 0;
  }

  return ret;
}

int on_start(const char *test_file) {
  if (!p_player_data || !p_player_data->main_pipeline) {
    on_prepared(test_file);
  }

  /* Let's handle pipeline's GST_MESSAGES
   * usefull to detect errors or end of stream among other things
   */
  int ret = 0;

  ret =
      PLAYER_ELEMENT_SET_STATE(p_player_data->main_pipeline, GST_STATE_PLAYING);

  return ret;
}

int on_pause() {
  if (!p_player_data || !p_player_data->main_pipeline) return 0;

  return PLAYER_ELEMENT_SET_STATE(p_player_data->main_pipeline,
                                  GST_STATE_PAUSED);
}

int on_resume() {
  if (!p_player_data || !p_player_data->main_pipeline) return 0;
  return PLAYER_ELEMENT_SET_STATE(p_player_data->main_pipeline,
                                  GST_STATE_PLAYING);
}

int on_stop() {
  GstState state;
  int ret;
  slog_info("on_stop");
  if (!p_player_data || !p_player_data->main_pipeline) return 0;
  if (p_player_data->current_state > GST_STATE_PAUSED) {
    ret = PLAYER_ELEMENT_SET_STATE(p_player_data->main_pipeline,
                                   GST_STATE_PAUSED);
    if (ret) return ret;
  }
  return 0;
}

int on_destory() {
  slog_info("on_destory");
  if (!p_player_data || !p_player_data->main_pipeline) return 0;

  slog_info("on_destory stop thread.");
  int ret =
      PLAYER_ELEMENT_SET_STATE(p_player_data->main_pipeline, GST_STATE_NULL);
  if (ret != 0) return -1;
  p_player_data->terminate = TRUE;
  pthread_join(player_thread, NULL);
  // PLAYER_ELEMENT_SET_STATE(p_player_data->main_pipeline, GST_STATE_NULL);
  gst_object_unref(p_player_data->main_pipeline);
  p_player_data->main_pipeline = NULL;
  sem_destroy(&p_player_data->waiteos_sem_);
  sem_destroy(&p_player_data->command_sem_);
  sem_destroy(&p_player_data->state_sem_);
  g_free(p_player_data);
  p_player_data = NULL;
  slog_info("on_destory exit....");
  return 0;
}

int on_flush() {
  int ret;
  if (!p_player_data || !p_player_data->main_pipeline) return 0;
  GstEvent *flush_start, *flush_stop;
  flush_start = gst_event_new_flush_start();
  ret = gst_element_send_event(GST_ELEMENT(p_player_data->main_pipeline),
                               flush_start);
  if (!ret) slog_info("failed to send flush-start event");
  flush_stop = gst_event_new_flush_stop(FALSE);
  ret = gst_element_send_event(GST_ELEMENT(p_player_data->main_pipeline),
                               flush_stop);
  if (!ret) slog_info("failed to send flush-start event");

  return 0;
}

int on_delay(int sec) {
  sleep(sec);
  return 0;
}

int on_speed_player(char c) {
  slog_info("set player speed:%c", c);
  if (!p_player_data || !p_player_data->main_pipeline) return 0;

  gint64 streamPosition;
  GstFormat format = GST_FORMAT_TIME;
  GstEvent *seekEvent;

  GstElement *videoSink =
      gst_bin_get_by_name(GST_BIN(p_player_data->main_pipeline), "sink");
  slog_info("videoSink :%p, gst_element_get_name:%s", videoSink,
            gst_element_get_name(videoSink));

  if (c == '+')
    p_player_data->speed_rate = p_player_data->speed_rate * 2;
  else if (c == '-')
    p_player_data->speed_rate = p_player_data->speed_rate / 2;

  gst_element_query_position(p_player_data->main_pipeline, format,
                             &streamPosition);

  seekEvent = gst_event_new_seek(
      p_player_data->speed_rate, GST_FORMAT_TIME,
      (GstSeekFlags)(GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_ACCURATE),
      GST_SEEK_TYPE_SET, streamPosition, GST_SEEK_TYPE_NONE, 0);

  int ret = gst_element_send_event(videoSink, seekEvent);
  slog_info("speed player ret:%d", ret);

  if (ret)
    return 0;
  else
    return -1;
}

int on_wait_eos() {
  GstState state;
  if (!p_player_data || !p_player_data->main_pipeline) return 0;
  int res = gst_element_get_state(p_player_data->main_pipeline, &state, NULL,
                                  5 * GST_SECOND);
  if (res == GST_STATE_CHANGE_FAILURE) {
    g_print("Failed to get player state.\n");
    return -1;
  }

  sem_timed_wait(&p_player_data->waiteos_sem_, 30);
  on_stop();
  return 0;
}

int on_seek(gint64 pos) {
  GstFormat fmt = GST_FORMAT_TIME;
  gint64 current;
  int ret = 0;

  slog_info("on seek pos:%d", pos);
  current = -1;
  if (!p_player_data || !p_player_data->main_pipeline) return 0;

  if (!gst_element_query_position(p_player_data->main_pipeline, fmt,
                                  &current)) {
    g_printerr("Could not query current position.\n");
  }

  if (!GST_CLOCK_TIME_IS_VALID(p_player_data->duration)) {
    if (!gst_element_query_duration(p_player_data->main_pipeline, fmt,
                                    &p_player_data->duration)) {
      g_printerr("Could not query current duration.\n");
    }
  }

  slog_info("Seek Position %" GST_TIME_FORMAT " / %" GST_TIME_FORMAT "\r",
            GST_TIME_ARGS(current), GST_TIME_ARGS(p_player_data->duration));

  if (p_player_data->seek_enabled && pos < p_player_data->duration) {
    ret = !gst_element_seek_simple(
        p_player_data->main_pipeline, GST_FORMAT_TIME,
        (GstSeekFlags)(GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_KEY_UNIT),
        pos * GST_MSECOND);
    slog_info("seek ret:%d", ret);

  } else {
    ret = 0;
    slog_info("Failed to seek, return");
  }
  return ret;
}

int on_preroll_frame(int frame) {
  if (!p_player_data || !p_player_data->main_pipeline) return 0;

  slog_info("prerolling first frame");
  // int ret = PLAYER_ELEMENT_SET_STATE(p_player_data->main_pipeline,
  // GST_STATE_PLAYING); if (ret != 0)
  //    return -1;
  p_player_data->preroll_done = FALSE;
  slog_info("stepping frames");
  if (!gst_element_send_event(
          p_player_data->main_pipeline,
          gst_event_new_step(GST_FORMAT_BUFFERS, frame, 1.0, TRUE, FALSE))) {
    slog_info("Filed to send STEP event!");
    return -1;
  }

  sem_timed_wait(&p_player_data->waiteos_sem_, 10);
  if (p_player_data->preroll_done)
    return 0;
  else
    return -1;
}
