#ifndef LK_WRAPPER_H
#define LK_WRAPPER_H

#include <assert.h>
#include <err.h>
#include <stdlib.h>
/* Include FreeRTOS prototype declaration */
/* include LK function prototype declaration */
#include <kernel/vm.h>
#include <kernel/thread.h>
#include <kernel/mutex.h>
#include <kernel/event.h>
#include <kernel/semaphore.h>
#include <kernel/timer.h>
#include <kernel/spinlock.h>
#include <kernel/msgq.h>


#if LK_WRAPPER_DEBUG

#define RES_STUB(m,r) do{\
    if(!r) {printf("--- %s test FAIL ---\n",#m);while(1);}\
    else printf("*** %s test PASS ---\n",#m);\
    }while(0)

#define API_TRACE(a,id) do{\
    printf("%s - %s() API exec\n",#id,#a);\
    }while(0)
#else

#define RES_STUB(m,r) do {}while(0)
#define API_TRACE(a,id) do {}while(0)

#endif



#define WRAPPER_FUNCTION

/* convert freertos tick to ms*/
#define FRTS_TICK_TO_LKMS(t)    ((lk_time_t)((( TickType_t )(t)) * portTICK_PERIOD_MS))
/*convert ms to freertos tick*/
#define LKMS_TO_FRTS_TICK(s)     (pdMS_TO_TICKS((lk_time_t)(s)))


// Assert helper to check invalid resched
#include <debug.h>
#define RESCHED_ASSERT() do {\
    if(arch_ints_disabled()) { panic("Resched denied at spin-locked state.\n%s -> %d\n",__FILE__,__LINE__); for(;;); }\
    }while(0)

/* Pending resched in ISR from checking interrupt depth to
 * indicate if the caller is in an ISR. */
extern uint32_t ulPortInterruptNesting;
#define ISR_ASSERT() do {\
    if(ulPortInterruptNesting > 0) { panic("Invalid call in ISR context.\n%s -> %d\n",__FILE__,__LINE__); for(;;); }\
    }while(0)
#endif