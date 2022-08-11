#ifndef __GSTREAMER_TEST_UTILS_H__
#define __GSTREAMER_TEST_UTILS_H__

#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

//****log verbose
#define DEBUG_VERBOSE true

#if DEBUG_VERBOSE
#define slog_verbose(...)                                                    \
  do {                                                                       \
    printf("<%s %s> pid:%d %s %s:%d VERBOSE:", __DATE__, __TIME__, getpid(), \
           __FUNCTION__, __FILE__, __LINE__) &&                              \
        printf(__VA_ARGS__) && printf("\n");                                 \
  } while (0)
#else

#define slog_verbose(...) \
  do {                    \
  } while (0)

#endif

#define slog_info(...)                                                    \
  do {                                                                    \
    printf("<%s %s> pid:%d %s %s:%d INFO:", __DATE__, __TIME__, getpid(), \
           __FUNCTION__, __FILE__, __LINE__) &&                           \
        printf(__VA_ARGS__) && printf("\n");                              \
  } while (0)

#define slog_war(...)                                                    \
  do {                                                                   \
    printf("<%s %s> pid:%d %s %s:%d WAR:", __DATE__, __TIME__, getpid(), \
           __FUNCTION__, __FILE__, __LINE__) &&                          \
        printf(__VA_ARGS__) && printf("\n");                             \
  } while (0)

#define slog_err(...)                                                    \
  do {                                                                   \
    printf("<%s %s> pid:%d %s %s:%d ERR:", __DATE__, __TIME__, getpid(), \
           __FUNCTION__, __FILE__, __LINE__) &&                          \
        printf(__VA_ARGS__) && printf("\n");                             \
  } while (0)

#define debug_fmt(a)                                          \
  do {                                                        \
    slog_info("format : %c%c%c%c", a & 0xff, (a >> 8) & 0xff, \
              (a >> 16) & 0xff, (a >> 24) & 0xff);            \
  } while (0)

#ifndef fourcc_code
#define fourcc_code(a, b, c, d) \
  ((__u32)(a) | ((__u32)(b) << 8) | ((__u32)(c) << 16) | ((__u32)(d) << 24))
#endif

#define GSTREAMER_RESULT_FIFO_NAME "/data/gstreamer_result_fifo"

typedef struct ResultInfo {
  char module[64];
  int32_t total;
  int32_t pass;
} result_info_t;

#endif  // __GSTREAMER_TEST_UTILS_H__