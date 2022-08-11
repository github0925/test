#ifndef _EARLY_APP_CFG_H_
#define _EARLY_APP_CFG_H_
#include <lk_wrapper.h>
#include <worker.h>

#define USDBG_ENABLE    0

#if (USDBG_ENABLE == 1)
#include <stdio.h>
#define USDBG(...) printf(__VA_ARGS__)
#else
#define USDBG(...) do{} while(0)
#endif


#define MAX_USO_NUM      5
#define MAX_DEPS_PER_USO  (MAX_USO_NUM - 1)

typedef void (*uso_func_t)(void* token);
typedef void (*uso_callback_t)(void* pargs, int uargs, int signal);

typedef enum usmState
{
    ussNone,
    ussInit,
    ussReady,
    ussRun,
    ussTerminated,
    ussRepeat,
    ussMAX,
}usmState;


typedef enum usmType
{
    ustDaemon,
    ustOneShot,

}usmType;

struct uso_t;

typedef struct uso_cbk_block_t
{
    uso_callback_t callback;
    void* pargs;
    int uargs;
}uso_cbk_block_t;



typedef struct uso_deps_meta_t
{
    struct uso_t* parent;
    usmState toggle_state;
}uso_deps_meta_t;

typedef struct uso_deps_t
{
    int init_depc;
    int entry_depc;
    uso_deps_meta_t init[MAX_DEPS_PER_USO];
    uso_deps_meta_t entry[MAX_DEPS_PER_USO];
}uso_deps_t;



typedef struct uso_t
{

    const char* name;
    queue_t* q;
    uso_func_t init;
    uso_func_t entry;
    void* container;
    usmType type;

    uso_deps_t deps;
    usmState state;
    int property_id;

    struct uso_t* child[MAX_DEPS_PER_USO];
    int children;

    queue_t cbk_channel[ussMAX];


}uso_t;


void unified_service_publish(void* token, usmState state);

void unified_service_subscribe(
    int id,
    usmState state,
    uso_callback_t cb,
    void* pargs,
    int uargs);

int unified_servcie_deps_chain_append(
    uso_t* uso,
    uso_t* dep_uso,
    usmState toggle_state,
    int phase);



static inline void** unified_service_get_container_pointer(void* token)
{
    return &((uso_t*)token)->container;
}

static inline int unified_service_get_property_id(void* token)
{
    return ((uso_t*)token)->property_id;
}

static inline usmState unified_service_get_current_state(void* token)
{
    return ((uso_t*)token)->state;
}

#endif
