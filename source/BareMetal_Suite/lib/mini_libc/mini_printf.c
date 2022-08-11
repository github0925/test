/********************************************************
 *  Copyright(c) 2018,2020   Semidrive                  *
 *******************************************************/

/**
 * @file    min_printf.c
 * @brief   a light-weight version of printf
 */

#include <stdarg.h>
#include <common_hdr.h>
#include "arch/spinlock.h"
#include "common/compiler.h"

/* Implemented by SOC */
extern void send_char(U8);
extern U32 usb_send(U8 *from, U32 sz);

static const char assic[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
                             'a', 'b', 'c', 'd', 'e', 'f'
                            };

static const unsigned int dec_base[] = {
    1UL,
    10UL,
    100UL,
    1000UL,
    10000UL,
    100000UL,
    1000000UL,
    10000000UL,
    100000000UL,
    1000000000UL,
};

spin_lock_t lock = SPIN_LOCK_INITIAL_VALUE;

/*
 * Format tag prototype is "%[flags][width][.precision][length]specifier"
 *      flags: -, +, space, #, O
 *      .precision: .number
 *      length: h, I, L
 *      width: number, *
 */
#define INNER_PRINTF(f) \
    va_list args;\
    int len = 0;\
    const char *p = NULL, *str = NULL;\
    int ival = 0, i = 0;\
    long long lval = 0;\
    unsigned int digit = 0, msb_processed = 0, index = 0, width = 8, d_width = 0;\
    va_start(args, fmt);\
    arch_spin_lock(&lock);\
    for (p = fmt; *p; p++) {\
        if (*p != '%') {\
            if (*p == '\n') {\
                U8 c = '\r';\
                f(c);\
                len++;\
            }\
            f(*p);\
            len++;\
            continue;\
        }\
        p++;\
        while (*p != 'c' && *p != 'd'\
               && *p != 'X' && *p != 'x'\
               && *p != 'p' && *p != 'u'\
               && *p != 's') {\
            if (*p >= '0' && *p <= '9' && *(p - 1) != '.' && *(p - 2) != '.') {\
                width = *p - '0';\
                if (width < 1) {\
                    width = 1;\
                }\
                d_width = width;\
            }\
            p++;\
        }\
        switch (*p) {\
        case 'd':\
            ival = va_arg(args, int);\
            if (ival < 0) {\
                f('-');\
                len++;\
                if(d_width>0)\
                    d_width--;\
                ival = 0 - ival;\
            }\
            msb_processed = 0;\
            for (i = 9; i >= 0; i--) {\
                digit = ival / dec_base[i];\
                ival = ival % dec_base[i];\
                if ((digit > 0) || (msb_processed == 1)) {\
                    f(digit + '0');\
                    len++;\
                    msb_processed = 1;\
                } else if ((d_width>i) && (i>0)) {\
                    f(' ');\
                    len++;\
                }\
            }\
            /* all digits are zeros */\
            if (!msb_processed) {\
                f('0');\
                len++;\
            }\
            d_width=0;\
            break;\
        case 'x':\
        case 'X':\
        case 'u':\
            ival = va_arg(args, int);\
            for (i = width - 1; i >= 0; i--) {\
                index = (ival >> (i) * 4) & 0xFUL;\
                f(assic[index]);\
                len++;\
            }\
            width = 8;\
            break;\
        case 'p':\
            width = sizeof(uintptr_t) * 2;\
            if (width == 16) {\
                lval = va_arg(args, long long);\
                for (i = width - 1; i >= 0; i--) {\
                    index = (lval >> (i) * 4) & 0xFUL;\
                    f(assic[index]);\
                    len++;\
                }\
            } else {\
                ival = va_arg(args, int);\
                for (i = width - 1; i >= 0; i--) {\
                    index = (ival >> (i) * 4) & 0xFUL;\
                    f(assic[index]);\
                    len++;\
                }\
            }\
            break;\
        case 's':\
            for (str = va_arg(args, char *); *str; str++) {\
                f(*str);\
                len++;\
            }\
            break;\
        case 'c':\
            f(va_arg(args, int));\
            len++;\
            break;\
        default:\
            break;\
        }\
    }\
    arch_spin_unlock(&lock);\
    va_end(args);\

int mini_printf(const char *fmt, ...)
{
    INNER_PRINTF(send_char);
    return 0;
}

int mini_sprintf(char *ss, const char *fmt, ...)
{
#define BUF_CHAR(c) {*ss = c; ss++;}
    INNER_PRINTF(BUF_CHAR);
    *ss = '\0';
    return len;
}

static U8 send_buf[128] __ALIGNED(CACHE_LINE);
static U8 local_index;

void write_char(U8 c)
{
    send_buf[local_index++] = c;
}
int usb_printf(const char *fmt, ...)
{
    local_index = 0;
    INNER_PRINTF(write_char);
    send_buf[local_index] = '\0';
    usb_send((U8*)send_buf,(U32)len);
    return len;
}
