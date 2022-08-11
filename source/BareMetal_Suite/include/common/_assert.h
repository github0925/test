/********************************************************
 *  Copyright(c) 2019   Semidrive       *
 *  All Right Reserved.
 *******************************************************/

#ifndef __ASSERT_H__
#define __ASSERT_H__

#include "debug.h"

#define assert(expr)    \
    do {\
        if (!(expr)) {\
            DBG("Opps, assertion failed in %s at line %d.\n", __FILE__, __LINE__);\
            while(1);\
        }\
    } while (0)

#endif  /* __ASSERT_H__ */
