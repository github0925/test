#ifndef __SEMIDRIVE_DRIVER_LOG_HPP
#define  __SEMIDRIVE_DRIVER_LOG_HPP

#ifdef ANDORID
#include <utils/Log.h>
#define TAG semidrivelink
#define LOGD(fmt,args...) ALOGD(fmt,##args)
#define LOGE(fmt,args...) ALOGE(fmt,##args)
#else 
#define LOGD(fmt,args...) printf(fmt,##args)
#define LOGE(fmt,args...) printf(fmt,##args)
#endif

#endif
