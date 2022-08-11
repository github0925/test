#ifndef __SOC_USB_H_
#define __SOC_USB_H_

#include "common_hdr.h"
#include "arch.h"
#include "__regs_base.h"
#include "srv_timer/srv_timer.h"

enum handler_return {
    INT_NO_RESCHEDULE = 0,
    INT_RESCHEDULE
};
#if defined(TGT_ap)
#define clean_cache_range               arch_clean_cache_range
#define clean_invalidate_cache_range    arch_clean_invalidate_cache_range
#define invalidate_cache_range          arch_invalidate_cache_range
#endif

#define LTRACEF(a,...)
#if 0
#define USBDBG_L1           DBG
#define USBDBG_L2           DBG
#define USBDBG_L3           DBG
#define printf(...)            
#define dprintf(...) 
#else

#define USBDBG_L1(...)           
#define USBDBG_L2(...)           
#define USBDBG_L3(...)   
#define dprintf(...)  
#define printf(...)              
#endif

#ifndef DEBUG_ASSERT
#define DEBUG_ASSERT        assert
#endif

#ifndef ASSERT
#define ASSERT              assert
#endif

#ifndef spin
#define spin                udelay      
#endif

#define ERR_NO_MEMORY   (-1)
#define NO_ERROR        (0)
#define ETIMEOUT        (116)
#define ERR_GENERIC     (-1)
#define ERR_CANCELLED   (-2)

#define DCFG_DEVSPD_FS  1
#define DCFG_DEVSPD_HS  0
#define DCFG_DEVSPD_SS  4

#define PANIC_UNIMPLEMENTED   do{\
                                while(1);\
                            }while(0);

typedef struct {
    U32 (*open)(module_e dev_id,void* para);
    U32 (*read)(void* dev,U32 from,U8* to,U32 sz);
    U32 (*write)(void* dev,U8* from, U32 to,U32 sz);
    U32 (*close)(void* dev);
    U32 (*ioctl)(void* dev,U32 ctl,void* para);
} dev_ops_t;


typedef struct {
    module_e dev_id;
    U32 state;
    U32 attrobute;
    U32 blk_sz;
    dev_ops_t ops;
    U32 crc32;
}bt_dev_t;

typedef  module_e  bt_dev_id_e;

#endif

