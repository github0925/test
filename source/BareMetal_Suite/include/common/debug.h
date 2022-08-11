/********************************************************
 *          Copyright(c) 2018   Semidrive               *
 *******************************************************/

/**
 * @file    debug.h
 * @brief   DBG definitions.
 */

#ifndef __DBG_H__
#define __DBG_H__
#include <arch.h>

extern int mini_printf(const char *fmt, ...);
extern int usb_printf(const char *fmt, ...);
extern int usb_if_ok;
#if defined(VTEST)
#define DBG(fmt, args...)       \
        do {\
            char w8xyz_buf[512];\
            sprintf(w8xyz_buf, "vtest_trace: %s", fmt);\
            io_printf(w8xyz_buf, ##args);\
        } while (0)
#elif defined(DEBUG_ENABLE)
#if defined(CFG_TB_SYS_BOOT_MON_EN) && !defined(TC_z1) && !defined(TC_zebu) && !defined(TC_fpga)
#if defined(CFG_TB_BUFFERD_PRINTF)
#define DBG(fmt, args...) tb_bprintf(fmt, ##args)
#else
#define DBG(fmt, args...) tb_printf(fmt, ##args)
#endif  /* CFG_TB_BUFFERD_PRINTF */
#else
#if defined(NO_STDLIB) || defined(NO_STD_PRINTF)
#if defined(WITH_SMP)
#define DBG(fmt, args...) \
                    do {\
                        uint32_t core_id = arch_curr_cpu_num();\
                        if (core_id != 0u) {\
                            mini_printf("[c%d]"fmt, core_id, ##args);\
                        } else {\
                            mini_printf(fmt, ##args);\
                            if(2==usb_if_ok){\
                                usb_printf(fmt, ##args);\
                            }\
                        }\
                    } while(0)
#else
#define DBG(fmt, args...) do{\
                            mini_printf(fmt, ##args);\
                            if(2==usb_if_ok){\
                                usb_printf(fmt, ##args);\
                            }\
                        }\
                        while(0);
#endif
#else
#define DBG(fmt, args...) printf(fmt, ##args)
#endif
#endif
#else
#define DBG(fmt, args...)
#endif

#if defined(ATB_SIGNER)
#define xyz_DBG(fmt, args...)  printf(fmt, ##args)
#else
#define xyz_DBG(fmt, args...)  DBG(fmt, ##args)
#endif

#if defined(DEBUG_ENABLE) || defined(ATB_SIGNER)
#define DBG_ARRAY_DUMP(array, sz)    \
        do {\
            U8 tmp_xyz[8];\
            memclr(tmp_xyz, sizeof(tmp_xyz));\
            for (U32 i_xyz = 0; i_xyz < (sz); i_xyz++) {\
                tmp_xyz[i_xyz%8] = (array)[i_xyz];\
                if (((i_xyz % 8 ) == 7) || (i_xyz == ((sz) - 1))) {\
                    xyz_DBG("0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x,\n",\
                            tmp_xyz[0], tmp_xyz[1], tmp_xyz[2], tmp_xyz[3],\
                            tmp_xyz[4], tmp_xyz[5], tmp_xyz[6], tmp_xyz[7]);\
                }\
            }\
        }while(0)
#else
#define DBG_ARRAY_DUMP(array, sz)
#endif

#if defined(INFO_LEVEL)
#define INFO(fmt, args...) mini_printf(fmt, ##args)
#define WARN(fmt, args...) mini_printf(fmt, ##args)
#define FATAL(fmt, args...) mini_printf(fmt, ##args)
#else
#define INFO(fmt, args...)  DBG(fmt, ##args)
#define WARN(fmt, args...)  DBG(fmt, ##args)
#define FATAL(fmt, args...)  DBG(fmt, ##args)
#endif

#endif // __DBG_H__
