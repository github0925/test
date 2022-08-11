#include "gstreamer_test_lib.h"

#include <assert.h>
#include <signal.h>
#include <string.h>
#include <time.h>

#include "glib.h"
#include "gstreamer_test_case_api.h"
#include "gstreamer_test_cmd_parser.h"
#include "gstreamer_test_elements.h"
#include "gstreamer_test_player.h"
#include "gstreamer_test_utils.h"

#define GSTREAMER_TEST_CMD_CONFIG "/data/gstreamer_test/gstreamer_cmds.cfg"
#define GSTREAMER_TEST_lOG_PATH "/data/gstreamer_test/gstreamer_test.log"

static vector<string> s_file_lists;
static vector<media_command_t> s_player_commands;

/* type for action functions */
typedef gboolean (*tPLAYER_ACTION)(void *params);

const tPLAYER_ACTION player_actions[] = {
    NULL,
    gst_video_delay,
    gst_video_play,
    gst_video_pause,
    gst_video_seek,
    NULL,
    gst_video_wait_eos,
    gst_video_next,
    gst_video_speed,
    gst_video_preroll,
    NULL,
    gst_video_stop,
};

const char *player_actions_2_str[] = {
    "None",    "Delay", "Play",        "Pause",   "Seek", "WaitLoad",
    "WaitEos", "Next",  "ChangeSpeed", "Preroll", "Loop", "Stop",
};

gboolean gstreamer_test_case_execute(void *param) {
  gboolean ret = TRUE;
  int num = *(int *)param;
  int file_count = s_file_lists.size();
  string str = s_file_lists[num % file_count];
  const char *file_name = str.c_str();
  int cmd_index = num / file_count;

  // slog_info("sleep 10s to debug last test error");
  // sleep(3);

  // create action function
  slog_info("current test case index:%d, file:%s", num, file_name);

  FILE *fp = fopen(GSTREAMER_TEST_lOG_PATH, "a+");
  time_t now;
  struct tm *tm_now;
  time(&now);
  tm_now = localtime(&now);
  char szTime[15];
  sprintf(szTime, "%02d-%02d %02d:%02d:%02d", tm_now->tm_mon + 1,
          tm_now->tm_mday, tm_now->tm_hour, tm_now->tm_min, tm_now->tm_sec);
  stringstream rpt;
  rpt << szTime << "Case #" << (num + 1) << endl;
  rpt << "File: " << file_name << endl;
  string report = rpt.str();
  fputs(report.c_str(), fp);

  ret = gst_video_create((void *)file_name);  // create

  if (ret) {  // action
    tPLAYER_ACTION handler;
    media_command_t commands;
    commands = s_player_commands[cmd_index];
    for (unsigned int i = 0; i < commands.size(); i++) {
      slog_info("Excute player Action:%d, %s", commands[i].action,
                player_actions_2_str[commands[i].action]);

      handler = player_actions[commands[i].action];
      if (handler) {
        if (commands[i].action == 7) {  // NEXT

          string next_str = s_file_lists[(num + 1) % file_count];
          const char *next_file = next_str.c_str();

          ret &= (*handler)((void *)next_file);

        } else
          ret &= (*handler)((void *)&(commands[i].data));
      }
      slog_info("excute result:%d", ret);

      // Output act and result
      if (fp) {
        rpt.str("");  // reset string stream
        rpt << "Action " << player_actions_2_str[commands[i].action]
            << ", result: " << ret << endl;
        report = rpt.str();
        fputs(report.c_str(), fp);
      }

      if (ret == 0) break;
    }
  }

  if (fp) {
    if (ret)
      fputs("====== Passed\n\n", fp);
    else
      fputs("======= Failed\n\n", fp);
    fclose(fp);
  }

  slog_info("destory video player....ret:%d", ret);
  if (ret) gst_video_destory(NULL);  // result

  return ret;
}

static int getFilesCount() {
  int cases_count = 0;
  slog_info("get files count");
  load_hw_decoder_elements();  // parse gst elements config
  string file_path = GSTREAMER_TEST_RESOURCE;
  auto elements = GstElements::shared();
  if (!elements) return 0;

  string path_second = elements->videoTests();
  GetFilesFromDirectory(s_file_lists,
                        file_path);  // get all video test file name

  s_player_commands =
      parseMediaCommands(GSTREAMER_TEST_CMD_CONFIG);  // parse all test cmds
  cases_count = s_player_commands.size();
  return s_file_lists.size() * cases_count;
}

TEST_P(GstPlayerTest, HandleTrueReturn) {  // P:parameterized, use diffenert
                                           // args to test the same test
  int n = GetParam();

  ASSERT_EXIT(exit(gstreamer_test_case_execute(&n) ? 0 : 1),
              ::testing::ExitedWithCode(0), ".*");
}

INSTANTIATE_TEST_CASE_P(
    TrueReturn, GstPlayerTest,
    testing::Range(
        0, getFilesCount(),
        1));  // according the file count to instantiate the test, the rang of 0
              // to getFilesCount() is the parame send to the test