#include <dlfcn.h>
#include <errno.h>
#include <fcntl.h>
#include <glib.h>
#include <gtest/gtest.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include <cstdio>
#include <cstring>
#include <vector>

#include "gstreamer_test_utils.h"

#define GSTREAMER_TEST_MODULE "gstreamer_test_module"
#define MAX_ARGV_COUNT 8

int main(int argc, char* argv[]) {
  int ret = -1;
  const char* execv_str[MAX_ARGV_COUNT] = {NULL};
  result_info_t result_info;

  slog_info(
      "%s:============================ GSTREAMER TEST BEGIN "
      "============================",
      __FUNCTION__);

  if (access(GSTREAMER_RESULT_FIFO_NAME, F_OK)) {
    ret = mkfifo(GSTREAMER_RESULT_FIFO_NAME, 0666);
    if (ret < 0) {
      slog_err("mkfifo failed, path:%s\n", GSTREAMER_RESULT_FIFO_NAME);
      return -1;
    }
  }

  int fd = open(GSTREAMER_RESULT_FIFO_NAME, O_RDONLY | O_NONBLOCK);
  if (fd < 0) {
    slog_err("open fifo RD failed, path:%s, error:%s\n",
             GSTREAMER_RESULT_FIFO_NAME, strerror(errno));
    return -1;
  }

  // vector<string> argv_temp;
  // argv_temp.push_back(GSTREAMER_TEST_MODULE);
  execv_str[0] = GSTREAMER_TEST_MODULE;
  execv_str[1] = NULL;

  pid_t fpid = fork();
  if (0 == fpid) {
    // child process run the real test
    if (execvp(GSTREAMER_TEST_MODULE, (char* const*)execv_str) < 0) {
      slog_err("execv error, errno:%s, exit!!!", strerror(errno));
      exit(-1);
    }
  } else if (fpid > 0) {
    int status;
    wait(&status);

    if (WIFEXITED(status)) {
      if (0 == WEXITSTATUS(status)) {
        slog_info("gstreamer module test done\n");
      } else {
        slog_err(
            "gstreamer module test exit, but test run end with in abnormal "
            "status:%d",
            WEXITSTATUS(status));
      }
    } else {
      slog_err("gstreamer module test exit abnormally, errno:%s",
               strerror(errno));
    }

    memset(&result_info, 0, sizeof(result_info_t));
    int count = read(fd, &result_info, sizeof(result_info_t));
    if (count != sizeof(result_info_t)) {
      slog_err("no valid result found after run gstreamer test");
    }
  } else {
    slog_err("fork error, errno:%s", strerror(errno));
  }

  close(fd);
  ret = unlink(GSTREAMER_RESULT_FIFO_NAME);
  if (ret < 0) {
    slog_err("unlink fifo fail, path:%s", GSTREAMER_RESULT_FIFO_NAME);
  }

  slog_info("\n**** TEST RESULT PRINT BEGIN: \n");

  uint32_t totalCount = 0;
  uint32_t totalPass = 0;
  slog_info(
      "**** module %32s:\t pass %4d, failed %4d, total %4d, Pass "
      "Rate:%.1f%%\n",
      result_info.module, result_info.pass,
      (result_info.total - result_info.pass), result_info.total,
      (float)result_info.pass / result_info.total * 100);
  totalCount += result_info.total;
  totalPass += result_info.pass;

  if (totalCount > 0) {
    slog_info(
        "\n**** gstreamer test modules result:\t pass %4d, failed %4d, total "
        "%4d, Pass Rate:%.1f%%\n",
        totalPass, (totalCount - totalPass), totalCount,
        (float)totalPass / totalCount * 100);
  }

  slog_info("**** TEST RESULT PRINT END: \n\n");
  slog_info(
      "%s: ============================ GSTREAMER TEST END "
      "============================ \n\n",
      __FUNCTION__);

  return ret;
}