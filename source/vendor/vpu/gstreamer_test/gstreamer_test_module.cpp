#include <dlfcn.h>
#include <errno.h>
#include <fcntl.h>
#include <glib.h>
#include <gtest/gtest.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <cstdio>
#include <cstring>
#include <vector>

#include "gstreamer_test_utils.h"

#define GSTREAMER_DEFAULT_TIMEOUT_SECONDS (60 * 60 * 10)  // 10h
//#define GSTREAMER_MIN_TIMEOUT_SECONDS 5

#define GSTREAMER_XML_OUTPUT_PATH "/data/gstreamer_test/"
#define GSTREAMER_TEST_MODULE "gstreamer_test_module"
//#define GSTREAMER_TEST_MODULE_LIB "libgstreamer_test.so"

void sigAlarmHandler(int sign) {
  if (SIGALRM == sign) {
    slog_war("%s: test run timeout(over %u seconds), just term process\n",
             __FUNCTION__, GSTREAMER_DEFAULT_TIMEOUT_SECONDS);
    raise(SIGTERM);
  }
}

void beforeRun() {}

void afterRun() {}

int main(int argc, char* argv[]) {
  int ret = -1;

  // void* g_handler = dlopen(GSTREAMER_TEST_MODULE_LIB, RTLD_LAZY |
  // RTLD_GLOBAL); if (NULL == g_handler) {
  //   slog_info("open %s failed, error msg:%s\n", GSTREAMER_TEST_MODULE_LIB,
  //             dlerror());
  //   exit(-1);
  // }

  // set timeout alarm handler
  signal(SIGALRM, sigAlarmHandler);
  alarm(GSTREAMER_DEFAULT_TIMEOUT_SECONDS);

  // config output xml file
  if (access(GSTREAMER_XML_OUTPUT_PATH, F_OK) != 0) {
    slog_info("create xml output dir:%s\n", GSTREAMER_XML_OUTPUT_PATH);
    mkdir(GSTREAMER_XML_OUTPUT_PATH, 0777);
  }

  char gtest_flag[64] = {0};
  snprintf(gtest_flag, 64, "xml:%s/%s_result.xml", GSTREAMER_XML_OUTPUT_PATH,
           GSTREAMER_TEST_MODULE);
  testing::GTEST_FLAG(output) = gtest_flag;

  slog_info(
      "\n\nGStreamer test run module begin **************************\n\n");

  int fd = open(GSTREAMER_RESULT_FIFO_NAME, O_WRONLY | O_NONBLOCK);
  if (fd < 0) {
    slog_err("open fifo WR failed, path:%s, error:%s\n",
             GSTREAMER_RESULT_FIFO_NAME, strerror(errno));
    return -1;
  }

  slog_info("argc:%d", argc);
  ::testing::InitGoogleTest(&argc, argv);

  beforeRun();

  ret = RUN_ALL_TESTS();

  afterRun();

  int total = testing::UnitTest::GetInstance()->total_test_count();
  int fail = testing::UnitTest::GetInstance()->failed_test_count();
  int pass = total - fail;
  slog_info("Run:%d, Pass:%d, Fail:%d, Pass Rate:%.1f%%\n", total, pass, fail,
            (float)pass / total * 100);

  result_info_t result_info;
  memset(&result_info, 0, sizeof(result_info_t));

  strncpy(result_info.module, GSTREAMER_TEST_MODULE,
          strlen(GSTREAMER_TEST_MODULE) + 1);
  result_info.total = total;
  result_info.pass = pass;

  int count = write(fd, &result_info, sizeof(result_info_t));
  if (count != sizeof(result_info_t)) {
    slog_err("result write failed");
  }

  close(fd);

  slog_info("\n\nGStreamer test run module end **************************\n\n");
  return ret;
}
