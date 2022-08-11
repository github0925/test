
#ifndef __VK_ANDROID_NATIVE_BUFFER_H__
#define __VK_ANDROID_NATIVE_BUFFER_H__

#if defined(SUPPORT_ANDROID_PLATFORM)

#include  <android/api-level.h>

#if __ANDROID_API__ > 24
#define VK_ANDROID_NATIVE_BUFFER_V_H "vk_android_native_buffer/vk_android_native_buffer_O.h"
#else
#define VK_ANDROID_NATIVE_BUFFER_V_H "vk_android_native_buffer/vk_android_native_buffer_N.h"
#endif

#include VK_ANDROID_NATIVE_BUFFER_V_H

#endif //defined(SUPPORT_ANDROID_PLATFORM)
#endif // __VK_ANDROID_NATIVE_BUFFER_H__
