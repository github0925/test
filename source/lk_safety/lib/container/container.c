#include <string.h>

#include <FreeRTOS.h>
#include <queue.h>
#include <timers.h>
#include <lk_wrapper.h>

#include <container.h>


#define TOKEN_THROUGHPUT  10

#define TK_TYPE(x)   ((token_t)x)
#define TK_PTYPE(x)  ((token_t*)x)

#define CC_TYPE(x) ((container_t)x)
#define CC_PTYPE(x) ((container_t*)x)

#define INVALID_UID (uid_t)INVALID_REFERENCE_ID
#define TOKEN_CHKCNT    10

#define TOKEN_MAGIC   0x544F4BE  //'TOKN'
#define CONTAINER_MAGIC 0x434F4E54  //'CONT'

struct token_t;

typedef uint32_t    uid_t;

typedef struct container_t
{
    uint32_t            magic;
    void*               ctx;
    struct token_t*     owner;
    struct token_t*     holder;
    uid_t               uid;
    QueueHandle_t*      dereference;
}container_t;

typedef struct token_t
{
    uint32_t            magic;
    struct token_t*     supplier;
    struct token_t*     consumer;
    container_t         recv_container;
    QueueHandle_t       issue;
    QueueHandle_t       dereference;
    uid_t               uids;
    uint32_t            throughput;
    ctx_convert_t       conv;
    token_expired_callback_t     dead_cb;
    uint32_t            keep_alive;
    bool                dead;
    TimerHandle_t       supervisor;
    void*               para;
    uint32_t            status;
    const char*         name;
}token_t;


static void default_dead_cb(void* token,void* para)
{
    ASSERT(TK_PTYPE(token)->magic == TOKEN_MAGIC);
    CC_DBG("token %s dead.\n",TK_PTYPE(token)->name);
}

static void token_general_timer_callback( TimerHandle_t xTimer )
{
    token_t* token = pvTimerGetTimerID(xTimer);
    ASSERT(token->magic == TOKEN_MAGIC);
    if(token->dead)
    {
        token->dead_cb(token,token->para);
    }
    else
    {
        token->dead = true;
        xTimerReset(xTimer,portMAX_DELAY);
    }

}

void* token_create(const char* name,ctx_convert_t conv_proc, uint32_t keep_alive, token_expired_callback_t dead_cb,void* para)
{
    token_t* token = malloc(sizeof(token_t));
    ASSERT(token);
    token->magic = TOKEN_MAGIC;
    token->throughput = TOKEN_THROUGHPUT;
    token->uids = 0;
    token->issue = xQueueCreate(token->throughput, sizeof(container_t));
    token->dereference = xQueueCreate(token->throughput, sizeof(uid_t));
    token->name = name;
    token->consumer = NULL;
    token->supplier = NULL;
    token->conv = conv_proc;
    token->keep_alive = keep_alive;
    token->dead = false;
    token->para = para;
    token->status = 0;
    if(!dead_cb)
    {
        token->dead_cb = default_dead_cb;
    }
    else
    {
        token->dead_cb = dead_cb;
    }

    if(token->keep_alive)
    {
        token->supervisor = xTimerCreate(name,token->keep_alive,pdFALSE,token,token_general_timer_callback);
        xTimerStart(token->supervisor,portMAX_DELAY);
    }
    else
    {
        token->supervisor = NULL;
    }

    return (void*)token;
}

void token_destroy(void* token)
{
    ASSERT(TK_PTYPE(token)->magic == TOKEN_MAGIC);

    uint32_t chkcnt = 0;
    while(TK_PTYPE(token)->status == TOKEN_ABNORMAL){
        thread_sleep(150);
        if(chkcnt++ >= TOKEN_CHKCNT)
        {
            printf("%s_%s!\r\n",TK_PTYPE(token)->name,__func__);
            break;
        }
    }

    CC_DBG("token %s is to be destroyed\n",TK_PTYPE(token)->name);

    vQueueDelete(TK_PTYPE(token)->issue);
    vQueueDelete(TK_PTYPE(token)->dereference);
    CC_DBG("token %s issue/dereference queue deleted\n",TK_PTYPE(token)->name);

    if(TK_PTYPE(token)->supervisor)
    {
        xTimerStop(TK_PTYPE(token)->supervisor,portMAX_DELAY);
        xTimerDelete(TK_PTYPE(token)->supervisor,portMAX_DELAY);
        CC_DBG("token %s stop & delete supervisor timer\n",TK_PTYPE(token)->name);
    }

    free(token);
    CC_DBG("token %s destroyed\n",TK_PTYPE(token)->name);
}

void token_serialization(uint32_t token_num,...)
{

    if(token_num < 2)
    {
        CC_DBG("serialization tokens must more than 1\n");
    }
    va_list ap;
    token_t *prev,*next;
    va_start(ap,token_num);
    prev = va_arg(ap,token_t*);
    token_num --;

    for(uint32_t i=0;i<token_num;i++)
    {

        next = va_arg(ap,token_t*);

        ASSERT(next->magic == TOKEN_MAGIC);
        ASSERT(prev->magic == TOKEN_MAGIC);

        next->supplier = prev;
        prev->consumer = next;

        prev = next;

        CC_DBG("token %s join after %s\n",
                next->name,
                next->supplier->name);
    }

    va_end(ap);

}

void ls_token_info(void* token)
{

    ASSERT(TK_PTYPE(token)->magic == TOKEN_MAGIC);

    CC_DBG("[token %10s: | supplier:%10s | consumer:%10s | keep alive:%6d | curr referred containers: %4d | activated:%5s]\n",
            TK_PTYPE(token)->name,
            TK_PTYPE(token)->supplier->name,
            TK_PTYPE(token)->consumer->name,
            TK_PTYPE(token)->keep_alive,
            TK_PTYPE(token)->uids,
            TK_PTYPE(token)->dead ? "Alive" : "Dead");
}

bool container_take(void* token,void** container, bool wait)
{
    ASSERT(TK_PTYPE(token)->magic == TOKEN_MAGIC);
    if(TK_PTYPE(token)->status == TOKEN_ABNORMAL)
        wait = false;

    if(!TK_PTYPE(token)->supplier->issue || !container)
    {
        CC_DBG("Invalid arguments:Supplier %s have no issue Q | container recv p:%p\n",
                TK_PTYPE(token)->supplier->name,
                container);
        return false;
    }

    TK_PTYPE(token)->dead = false;

    uint32_t timeout = wait ? portMAX_DELAY : 0;

    if(pdTRUE != xQueueReceive(TK_PTYPE(token)->supplier->issue,&TK_PTYPE(token)->recv_container,timeout))
    {
        CC_DBG("Take from %s(%p) timeout.\n",TK_PTYPE(token)->supplier->name,TK_PTYPE(token)->supplier->issue);
        *container = NULL;
        return false;
    }
    else
    {
        *container = &(TK_PTYPE(token)->recv_container);
        return true;
    }

}

void container_carryon(void* container,void* ctx, uint32_t len)
{
    ASSERT(CC_PTYPE(container)->magic == CONTAINER_MAGIC);

    CC_PTYPE(container)->ctx = malloc(len);
    ASSERT(CC_PTYPE(container)->ctx);
    memcpy(CC_PTYPE(container)->ctx,ctx,len);
}

void container_carrydown(void* container,void* ctx, uint32_t len)
{
    ASSERT(CC_PTYPE(container)->magic == CONTAINER_MAGIC);

    if(CC_PTYPE(container)->holder->conv)
    {
        CC_DBG("Use supplier %s private convert proc %p.\n",
        CC_PTYPE(container)->holder->name,
        CC_PTYPE(container)->holder->conv);

        CC_PTYPE(container)->holder->conv(CC_PTYPE(container)->ctx,ctx);
    }
    else
    {
        memcpy(ctx,CC_PTYPE(container)->ctx,len);
    }

    free(CC_PTYPE(container)->ctx);
    CC_PTYPE(container)->ctx = NULL;
}


bool container_give(void* token,void* container, bool wait)
{
    ASSERT(TK_PTYPE(token)->magic == TOKEN_MAGIC);
    ASSERT(CC_PTYPE(container)->magic == CONTAINER_MAGIC);

    if(TK_PTYPE(token)->status == TOKEN_ABNORMAL)
        wait = false;

    TK_PTYPE(token)->dead = false;

    uint32_t timeout = wait ? portMAX_DELAY : 0;

    if(!TK_PTYPE(token)->issue || !container)
    {
        CC_DBG("Invalid arguments:token %s have no issue Q | container:%p\n",
                TK_PTYPE(token)->name,
                container);
        return false;
    }

    CC_PTYPE(container)->holder = TK_PTYPE(token);

    if(pdTRUE != xQueueSend(TK_PTYPE(token)->issue,container,timeout))
    {
        CC_DBG("Give to %s(%p) timeout.\n",TK_PTYPE(token)->name,TK_PTYPE(token)->issue);
        return false;
    }
    else
    {
        return true;
    }

}
void token_setstatus(void* token,uint32_t status)
{
    ASSERT(TK_PTYPE(token)->magic == TOKEN_MAGIC);
    TK_PTYPE(token)->status = status;
}
uint32_t token_getstatus(void* token)
{
    return TK_PTYPE(token)->status;
}
void* container_create(void* token_owner,bool reference,uid_t* reference_id)
{
    ASSERT(TK_PTYPE(token_owner)->magic == TOKEN_MAGIC);

    container_t* container = malloc(sizeof(container_t));
    ASSERT(container);

    TK_PTYPE(token_owner)->dead = false;

    container->magic = CONTAINER_MAGIC;
    container->ctx = NULL;
    container->owner = TK_PTYPE(token_owner);
    container->holder = NULL;

    if(reference)
    {
        ASSERT(reference_id);
        container->uid = TK_PTYPE(token_owner)->uids;
        TK_PTYPE(token_owner)->uids++;
        container->dereference = &TK_PTYPE(token_owner)->dereference;
        *reference_id = container->uid;
    }
    else
    {
        container->uid = INVALID_UID;
        container->dereference = NULL;
    }


    return (void*)container;
}

uid_t container_wait_dereferenced(void* token, bool wait)
{

    ASSERT(TK_PTYPE(token)->magic == TOKEN_MAGIC);

    uid_t uid = INVALID_UID;
    if(TK_PTYPE(token)->status == TOKEN_ABNORMAL)
        wait = false;

    uint32_t timeout = wait ? portMAX_DELAY : 0;


    CC_DBG("Wait on container dereference Q %s(%p)\n",
            TK_PTYPE(token)->name,
            TK_PTYPE(token)->dereference);

    TK_PTYPE(token)->dead = false;

    if(pdTRUE != xQueueReceive(TK_PTYPE(token)->dereference,&uid,timeout))
    {
        CC_DBG("Wait on dereference Q %s(%p) timeout.\n",
                TK_PTYPE(token)->name,
                TK_PTYPE(token)->dereference);
        return INVALID_UID;
    }
    else
    {
        return uid;
    }
}

void container_dereferenced(void* container)
{
    ASSERT(CC_PTYPE(container)->magic == CONTAINER_MAGIC);
    ASSERT(CC_PTYPE(container)->dereference);

    token_t* token = containerof(CC_PTYPE(container)->dereference,token_t,dereference);

    ASSERT(token->magic == TOKEN_MAGIC);

    CC_DBG("Container dereference to %s\n",token->name);

    xQueueSend(token->dereference,&CC_PTYPE(container)->uid,0);

}

bool is_container_referenced(void* container)
{
    ASSERT(CC_PTYPE(container)->magic == CONTAINER_MAGIC);

    return CC_PTYPE(container)->dereference ? true : false;
}

void container_destroy(void* token,void* container)
{
    ASSERT(CC_PTYPE(container)->magic == CONTAINER_MAGIC);
    ASSERT(CC_PTYPE(container)->owner == token);

    free(container);
}