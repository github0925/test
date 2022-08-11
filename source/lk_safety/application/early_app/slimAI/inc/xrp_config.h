#ifndef _XRP_CONFIG_H
#define _XRP_CONFIG_H

#include <image_cfg.h>

#define CV_DBG_ENABLE    0

#if (CV_DBG_ENABLE == 1)
#define APP_DBG(...) printf(__VA_ARGS__)
#else
#define APP_DBG(...) do{} while(0)
#endif

/* config will be send to vdsp when sync */
#define XRP_COMM_BASE   (VDSP_SHARE_MEMBASE)
#define XRP_SHARED_BASE (XRP_COMM_BASE + 0x4000)
#define XRP_SHARED_SIZE (VDSP_SHARE_MEMSIZE - 0x4000)
#define XRP_IO_BASE     (0x34040000)

/* 1 level mode, 0 polling mode */
#define XRP_IRQ_MODE    (1)
#define XRP_IRQ_NUM     (23)

#endif
