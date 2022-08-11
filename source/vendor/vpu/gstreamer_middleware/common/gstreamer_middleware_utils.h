#ifndef __GSTREAMER_MIDDLEWARE_UTILS_H__
#define __GSTREAMER_MIDDLEWARE_UTILS_H__

#include <glib.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <unistd.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define LOG_NONE 0
#define LOG_STD 1

#define LOG_TYPE LOG_STD

#if LOG_TYPE == LOG_STD
// int getTimeString(char *buffer, int length);// thread is not safe and may need to be deleted
#define LOG_PRINT(level, format, args...)                                                          \
    do {                                                                                           \
        struct timeval current_time;                                                               \
        gettimeofday(&current_time, NULL);                                                         \
        double time_ms = current_time.tv_sec * 1000 + current_time.tv_usec / 1000;                 \
        printf("%lf %s %d:%u %s:%d " format "\n", time_ms, level, getpid(),                        \
               (uint32_t)syscall(__NR_gettid), __func__, __LINE__, ##args);                        \
    } while (0)
#define ERRNO(format, args...)                                                                     \
    LOG_PRINT("E", format ", errno %d %s", ##args, errno, strerror(errno))
#define ERROR(format, args...) LOG_PRINT("E", format, ##args)
#define WARN(format, args...) LOG_PRINT("W", format, ##args)
#define INFO(format, args...) LOG_PRINT("I", format, ##args)
#define DEBUG(format, args...) LOG_PRINT("D", format, ##args)
#define VERBOSE(format, args...) LOG_PRINT("V", format, ##args)
#elif LOG_TYPE == LOG_NONE
#define LOG_PRINT(level, format, args...)
#define ERROR(format, args...) LOG_PRINT("E", format, ##args)
#define WARN(format, args...) LOG_PRINT("W", format, ##args)
#define INFO(format, args...) LOG_PRINT("I", format, ##args)
#define DEBUG(format, args...) LOG_PRINT("D", format, ##args)
#define VERBOSE(format, args...) LOG_PRINT("V", format, ##args)
#endif

#define DEBUG_FUNC() DEBUG("")

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif //__GSTREAMER_MIDDLEWARE_UTILS_H__