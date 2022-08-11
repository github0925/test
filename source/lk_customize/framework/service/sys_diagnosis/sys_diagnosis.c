#include <kernel/thread.h>
#include <kernel/event.h>
#include <kernel/mutex.h>
#include <platform/interrupts.h>
#include <irq.h>
#include <assert.h>
#include <sys_priority.h>
#include "sys_diagnosis.h"


void* malloc(size_t size);

#define MAX_SYSD_SIGNALS    319
#define SYSD_WORKER_STACK_SZ (4 * 1024)

typedef struct sysd_handler_t
{
    struct sysd_handler_t* next;
    struct sysd_handler_t* prev;
    sys_event_cb_t evt_cb;
    void* args;
}sysd_handler_t;


typedef struct
{
    event_t evt;
    mutex_t lock;
    enum sem sem;
    uint16_t irq;
}sem_handle_t;

typedef struct sysd_list_t
{
    int chain_len;
    sysd_handler_t* head;
    sysd_handler_t* tail;
}sysd_list_t;

static sysd_list_t total_signals[MAX_SYSD_SIGNALS];


static sem_handle_t sem_handle;


static void sysd_signal_walkthrough(int signal)
{

    ASSERT(signal < MAX_SYSD_SIGNALS);

    sysd_handler_t* sysd = total_signals[signal].head;

    while(sysd)
    {
        if(sysd->evt_cb)
        {
            sysd->evt_cb(signal,sysd->args);
        }

        sysd = sysd->next;
    }
}

static void sysd_add_handler(int signal, sysd_handler_t* handler, int tail)
{

    ASSERT(signal < MAX_SYSD_SIGNALS);

    sysd_list_t* list = (sysd_list_t*)&total_signals[signal];

    if(list->head == NULL && list->tail == NULL)
    {
        //first added
        list->head = list->tail = handler;
        handler->prev = handler->next = NULL;
    }
    else
    {
        if(!tail)
        {
            handler->prev = NULL;
            handler->next = list->head;
            list->head->prev = handler;
            list->head = handler;
        }
        else
        {
            handler->next = NULL;
            handler->prev = list->tail;
            list->tail->next = handler;
            list->tail = handler;
        }
    }

    list->chain_len++;

}

int sysd_register_handler(sys_event_cb_t cb,void* args,uint32_t n,...)
{
    va_list ap;
    int signal = NULL_SIGNAL;
    va_start(ap,n);

    sysd_handler_t* phandler = NULL;

    mutex_acquire(&sem_handle.lock);

    for(int i=0;i<n;i++)
    {
        signal = va_arg(ap,int);

        if(signal == NULL_SIGNAL)
        {
            continue;
        }
        else
        {

            phandler = malloc(sizeof(sysd_handler_t));
            if(!phandler)
            {

                va_end(ap);
                mutex_release(&sem_handle.lock);
                return -1;
            }
            else
            {
                phandler->args = args;
                phandler->evt_cb = cb;
                sysd_add_handler(signal,phandler,0);
                sem_map_signal(sem_handle.sem, signal, SEM_INTR_CPU, true);
            }
        }
    }

    va_end(ap);

    mutex_release(&sem_handle.lock);

    return 0;
}

int sysd_register_daemon_handler(sys_event_cb_t cb,void* args,int signal)
{

    sysd_handler_t* phandler = NULL;

    mutex_acquire(&sem_handle.lock);

    phandler = malloc(sizeof(sysd_handler_t));
    if(!phandler)
    {
        mutex_release(&sem_handle.lock);
        return -1;
    }
    else
    {
        phandler->args = args;
        phandler->evt_cb = cb;
        sysd_add_handler(signal,phandler,1);
        sem_map_signal(sem_handle.sem, signal, SEM_INTR_CPU, true);
        mutex_release(&sem_handle.lock);
        return 0;
    }
}

static enum handler_return sysd_sem_handler(void *arg)
{

    sem_enable_intr(sem_handle.sem, SEM_INTR_CPU, false);
    event_signal(&sem_handle.evt, false);

    return INT_RESCHEDULE;
}

static void sysd_dispatcher(void)
{
    for(int s=0; s<MAX_SYSD_SIGNALS;s++)
    {
        if (sem_signal_status(sem_handle.sem, s))
        {
            sysd_signal_walkthrough(s);
        }
    }
}

static void sysd_worker(enum sem sem)
{

    sem_handle.sem = sem;
    sem_handle.irq = ( (sem == SEM1) ? SEM1_O_SEM_INT_CPU_NUM : SEM2_O_SEM_INT_CPU_NUM) ;

    event_init(&sem_handle.evt,false,1);
    mutex_init(&sem_handle.lock);

    mutex_acquire(&sem_handle.lock);

    for(int s=0; s<MAX_SYSD_SIGNALS;s++)
    {
        sem_map_signal(sem_handle.sem, s,SEM_INTR_CPU,false);
    }

    register_int_handler(sem_handle.irq, sysd_sem_handler, NULL);
    unmask_interrupt(sem_handle.irq);


    sem_enable_intr(sem_handle.sem, SEM_INTR_CPU, true);

    mutex_release(&sem_handle.lock);

    while(1)
    {
        event_wait(&sem_handle.evt);

        mutex_acquire(&sem_handle.lock);

        sysd_dispatcher();
        sem_enable_intr(sem_handle.sem, SEM_INTR_CPU, true);

        mutex_release(&sem_handle.lock);
    }
}

void sysd_start(enum sem sem)
{
    thread_t* sysd = thread_create("sysd",(thread_start_routine)sysd_worker,(void*)sem,THREAD_PRI_SYSDIAG,SYSD_WORKER_STACK_SZ);
    thread_detach_and_resume(sysd);
}
