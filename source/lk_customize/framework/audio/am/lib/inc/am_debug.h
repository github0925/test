/**
 * @file am_debug.h
 * @author shao yi
 * @brief
 * @version 0.1
 * @date 2021-01-15
 *
 * @copyright Copyright (c) 2021 Semidrive Semiconductor
 *
 */

#ifndef __AM_DEBUG_H__
#define __AM_DEBUG_H__
#define AM_DEBUG_PRT 1
#ifdef AM_DEBUG_PRT
#define _NO_IMPL_PRT_ printf("Func %s NON-IMPL yet.\n", __FUNCTION__);
#define _NO_COMP_PRT_ printf("Func %s NON-COMPLETED yet.\n", __FUNCTION__);
#define _FUNC_LINE_PRT_ printf("Func %s line %d.\n", __FUNCTION__,__LINE__);
#define _ERR_FUNC_LINE_PRT_ printf("Err on Func %s line %d.\n", __FUNCTION__, __LINE__);
#else
#define _NO_IMPL_PRT_
#define _NO_COMP_PRT_
#define _FUNC_LINE_PRT_
#define _ERR_FUNC_LINE_PRT_
#endif
#define _ERR_FUNC_LINE_ printf("Err occurred on Func %s line %d.\n", __FUNCTION__, __LINE__);
#define _CHECK_RET(x) if(x==false){printf("Err ret is false on Func %s line %d.\n", __FUNCTION__, __LINE__);return false;}
#define _CHECK_RET_AND_VAL(x, y, z)                                            \
    {                                                                          \
        if (x == false) {                                                      \
            printf("Err ret is false on Func %s line %d.\n", __FUNCTION__,     \
                   __LINE__);                                                  \
            return false;                                                      \
        }                                                                      \
        if (y != z) {                                                          \
            printf("Err val(%d) != %d on Func %s line %d.\n", y, z,            \
                   __FUNCTION__, __LINE__);                                    \
            return false;                                                      \
        }                                                                      \
    }                                                                          \

#endif