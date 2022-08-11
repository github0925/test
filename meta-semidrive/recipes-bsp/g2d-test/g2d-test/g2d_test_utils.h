#ifndef _G2D_TEST_UTILS_H_
#define _G2D_TEST_UTILS_H_

#include <stdio.h>

//****log verbose
#define DEBUG_VERBOSE true

//****debug save input rawdata
#define DEBUG_INPUT true

//****debug save input args
#define DEBUG_INPUT_ARGS false

#define __FILENAME__ (strrchr(__FILE__, '/') ? (strrchr(__FILE__, '/') + 1):__FILE__)

#if DEBUG_VERBOSE
#define slog_verbose(...) do { \
                    printf("%s %s:%d verbose:", __FILENAME__,__FUNCTION__,__LINE__)&& \
                    printf(__VA_ARGS__)&& \
                    printf("\n"); \
                }while(0)
#else

#define slog_verbose(...) do { \
                }while(0)

#endif

#define slog_info(...) do { \
    printf("%s %s:%d INFO:", __FILENAME__,__FUNCTION__,__LINE__)&& \
    printf(__VA_ARGS__)&& \
    printf("\n"); \
}while(0)

#define slog_war(...) do { \
    printf("%s %s:%d WAR:", __FILENAME__,__FUNCTION__,__LINE__)&& \
    printf(__VA_ARGS__)&& \
    printf("\n"); \
}while(0)

#define slog_err(...) do { \
    printf("%s %s:%d ERR:", __FILENAME__,__FUNCTION__,__LINE__)&& \
    printf(__VA_ARGS__)&& \
    printf("\n"); \
}while(0)

#define debug_fmt(a) do { \
    slog_info("format : %c%c%c%c", a & 0xff, (a >> 8) & 0xff, \
        (a >> 16) & 0xff, (a >> 24) & 0xff); \
}while(0)

#define fmt_str(a) (a & 0xff, (a >> 8) & 0xff, (a >> 16) & 0xff, (a >> 24) & 0xff)

#ifndef fourcc_code
#define fourcc_code(a, b, c, d) ((__u32)(a) | ((__u32)(b) << 8) | \
				 ((__u32)(c) << 16) | ((__u32)(d) << 24))
#endif

#endif//  _G2D_TEST_UTILS_H_
