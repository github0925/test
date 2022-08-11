# LK to freeRTOS consistency regulation guide
---
  All the below APIs **MUST NOT** be called in interrupt context.
---
## event module
* event_init
  LK event module support 2 types of event: **EVENT_FLAG_AUTOUNSIGNAL(bool 1)** and **without EVENT_FLAG_AUTOUNSIGNAL(bool 0)**, **without EVENT_FLAG_AUTOUNSIGNAL** may cause all waiting threads wokeup once the event signaled. In this case, FreeRTOS didn't support multi-shooter - single receiver, which only one-shot model is supported. So that, use **EVENT_FLAG_AUTOUNSIGNAL** while calling *event_init* when porting from LK environment to FreeRTOS.
* event_destroy
  No regulation.
* event_wait_timeout
  No regulation.
* event_signal
  LK support **reschedule option** in this API, the scheduler may not pick and switch context at once if this option is **false**. In FreeRTOS environment, all synchronization API does resheduling at once. In this case, make sure **reschedule option** is **true** while calling *event_signal* on your porting.
* event_unsignal
  No regulation.
---
## mutex module
* mutex_init
  No regulation.
* mutex_destroy
  No regulation.
* mutex_acquire_timeout
  No regulation.
* mutex_release
  No regulation.

## semaphore module
* sem_init
  No regulation.
* sem_destroy
  No regulation.
* sem_post
  Like *event_signal*, make sure **reschedule option** is **true** while calling *sem_post* on your porting as FreeRTOS kernel does rescheduling at once in ALL synchronization API.
* sem_wait
  No regulation.
* sem_trywait
  No regulation.
* sem_timedwait
  No regulation.

## timer module
### difference
- Timers in LK envrionment excute its callback in **interrupt context**, however in FreeRTOS timers excute its callback in **task context**, as FreeRTOS support timer services by scheduler ticks counting and maintaining timer daemon task to excute/pass arguments to its callback.
- LK timers have resolution in dependency of hw timers accuracy, instead of the dependency of scheduler ticks accuracy in FreeRTOS.
- FreeRTOS uses command queue to notify timer daemon task from scheduler in its timers manipulation APIs , which means in this mechanism the timers API sets may **block** and **return bool type** as its unique implementation.
  1. **block**: Use `FRTS_TMR_OP_TICKOUT` macro to specific the blocking timeout. In FreeRTOS ticks unit, user can use `LKMS_TO_FRTS_TICK(ms)` to convert ms to FreeRTOS ticks.
  2. **return**: Successful manipulation as **true**,vice versa.

* timer_initialize
  No regulation.
* timer_set_oneshot
  No regulation.
* timer_set_periodic
  No regulation.
* timer_cancel
  No regulation.

  ## heap module
* malloc
  No regulation.
* calloc
  No regulation.
* memalign
  As FreeRTOS did not provide memalign function, this memalign function was implemented by semidrive. From functionality aspect, no regulation.
* free
  No regulation.

## thread module
Thread module only implemented limited API. Others' APIs were only stub in FreeRTOS, which may only be implemented if specific cases needed.
* thread_create
  For compitability reason, **never refer to member of *thread_t* structure!**
* thread_sleep
  No regulation.
* thread_yield
  No regulation.
* thread_resume
  No regulation.
* thread_set_priority
  No regulation.