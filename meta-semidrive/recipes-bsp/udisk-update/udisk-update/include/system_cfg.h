#ifndef _SYSTEM_CFG_H_
#define _SYSTEM_CFG_H_

//#define RUN_IN_ANDROID

#ifdef RUN_IN_ANDROID
#include <log/log.h>
#define PRINTF_CRITICAL  ALOGE
#define PRINTF_INFO      ALOGD
#define MMC_PATH(x)         "/dev/block/"#x""
#define MTD_PATH(x)         "/dev/block/"#x""
#define USE_HAND_OVER    0
#else
#define PRINTF_CRITICAL(format, ...) fprintf(stdout, format, ##__VA_ARGS__)
#define PRINTF_INFO(format, ...)     fprintf(stdout, format, ##__VA_ARGS__)
#define MMC_PATH(x)         "/dev/"#x""
#define MTD_PATH(x)         "/dev/"#x""
#define USE_HAND_OVER    0

#endif //RUN_IN_ANDROID

#endif //_SYSTEM_CFG_H_
