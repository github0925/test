//------------------------------------------------------------------------------
// File: config.h
//
// Copyright (c) 2006, Chips & Media.  All rights reserved.
// This file should be modified by some developers of C&M according to product version.
//------------------------------------------------------------------------------


#ifndef __CONFIG_H__
#define __CONFIG_H__

#if 0
#undef __linux
#undef linux

#if defined(_WIN32) || defined(__WIN32__) || defined(_WIN64) || defined(WIN32) || defined(__MINGW32__)
#   define PLATFORM_WIN32
#elif defined(linux) || defined(__linux) || defined(ANDROID)
#   define PLATFORM_LINUX
#else
#   define PLATFORM_NON_OS
#endif

#if defined(CNM_FPGA_PLATFORM) || defined(CNM_SIM_PLATFORM)
#ifdef ANDROID
#else
#endif
#endif
#endif

#define PLATFORM_NON_OS
#define API_VERSION 0x124
#define SEMI_SW_TEST
#define SEMI_CODA_TEST
#define YUV_FILE_IN_DDR

//#define SEMI_VES_FILE_DDR_ADDR       0x81000000
#define SEMI_VES_FILE_SIZE           0x2000000
#define SEMI_VDB_VIDEO_SIZE          0x05000000
#define SEMI_YUV_FILE_DDR_ADDR       0x58000000
#define SEMI_YUV_FILE_DDR_SIZE       0x8000000
#define SEMI_OUTPUT_FILE_SIZE        0xc000000

//#define SEMI_VES_CFG_DDR_ADDR        0xf1000000
#endif  /* __CONFIG_H__ */

