#include <kernel/thread.h>
#include <kernel/mutex.h>
#include <kernel/semaphore.h>
#include <sys_priority.h>
#include "worker.h"

void* malloc(size_t size);
void free(void* p);

typedef struct worker_t
{
    struct worker_t* next;
    struct worker_t* prev;
    worker_foo_t foo;
    void* pargs;
    uint32_t uargs;
}worker_t;

typedef struct worker_handle_t
{
    worker_t* head;
    worker_t* tail;
    mutex_t lock;
    semaphore_t sem;

}worker_handle_t;

static worker_handle_t spawn;

static inline void push_worker_with_locked(worker_t* worker)
{
    mutex_acquire(&spawn.lock);

    if(likely(spawn.head == NULL && spawn.tail == NULL))
    {
        worker->next = NULL;
        worker->prev = NULL;
        spawn.head = spawn.tail = worker;
    }
    else
    {

        worker->next = spawn.head;
        worker->prev = NULL;
        spawn.head->prev = worker;
        spawn.head = worker;
    }

    mutex_release(&spawn.lock);
}

static inline worker_t* pop_worker_with_locked(void)
{
    worker_t* worker = NULL;
    mutex_acquire(&spawn.lock);

    if(unlikely(spawn.head == NULL && spawn.tail == NULL))
    {
        ;//no worker in list
    }
    else
    {
        worker = spawn.tail;
        if(likely(spawn.head == spawn.tail))
        {
            //one node case
            spawn.head = spawn.tail = NULL;
        }
        else
        {
            worker->prev->next = NULL;
            spawn.tail = spawn.tail->prev;
        }
    }

    mutex_release(&spawn.lock);

    return worker;
}


static worker_t* create_worker(worker_foo_t foo, void* pargs, uint32_t uargs)
{
    worker_t* worker = malloc(sizeof(worker_t));
    if(!worker)
    {
        return NULL;
    }

    worker->foo = foo;
    worker->pargs = pargs;
    worker->uargs = uargs;

    return worker;

}

static inline void destroy_worker(worker_t* worker)
{
    free(worker);
}

static void worker_daemon(void)
{
    worker_t* worker = NULL;
    sem_init(&spawn.sem,0);
    mutex_init(&spawn.lock);
    spawn.tail = spawn.head = NULL;

    for(;;)
    {
        sem_wait(&spawn.sem);

        worker = pop_worker_with_locked();
        if(!worker || !worker->foo)
        {
            continue;
        }

        worker->foo(worker->pargs,worker->uargs);

        destroy_worker(worker);
    }
}


/* Interface */

void start_worker_service(void)
{
    thread_t* t = thread_create("worker_daemon",(thread_start_routine)worker_daemon,NULL,THREAD_PRI_WORKER,DEFAULT_STACK_SIZE);
    thread_detach_and_resume(t);
}

void call_worker(worker_foo_t foo, void* pargs, uint32_t uargs)
{
    worker_t* worker = create_worker(foo,pargs,uargs);

    if(!worker)
    {
        return;
    }

    push_worker_with_locked(worker);
    sem_post(&spawn.sem,false);

}
