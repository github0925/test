//------------------------------------------------------------------------------
// File: config.h
//
// Copyright (c) 2006, Chips & Media.  All rights reserved.
// This file should be modified by some developers of C&M according to product version.
//------------------------------------------------------------------------------

#ifndef __CONFIG_H__
#define __CONFIG_H__

#if defined(linux) || defined(__linux) || defined(ANDROID)
	#define PLATFORM_LINUX
#else
	#define PLATFORM_NON_OS
#endif

#define API_VERSION 0x127

#define SUPPORT_PADDING_UNALIGNED_YUV
#endif	/* __CONFIG_H__ */

