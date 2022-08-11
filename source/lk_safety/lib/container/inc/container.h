#ifndef _CONTAINER_CHAIN_H_
#define _CONTAINER_CHAIN_H_
#include <stdint.h>
#include <stdbool.h>

#define CC_TRACE_ENABLE    0

#if (CC_TRACE_ENABLE == 1)
#define CC_DBG(...) do{ \
    vTaskSuspendAll(); \
    printf(__VA_ARGS__); \
    xTaskResumeAll(); \
}while(0)
#else
#define CC_DBG(...) do{} while(0)
#endif

typedef void*       container_handle_t;
typedef void*       token_handle_t;
typedef uint32_t    reference_handle_t;


typedef bool (*ctx_convert_t)(void* ctx_src, void* ctx_dst);
typedef void (*token_expired_callback_t)(token_handle_t token,void* para);




// token rules:
// 1. token was passed from supervisor, tokener should not create/destroy it.
// 2. tokener should be held into only one thread-entity.
// 3. tokener could create/take/give container with its token to mark the container state.


// container rules:
// 1. container owner should be in charge of free.
// 2. tokener could take or create container from/by another tokener/itself.
// 3. modification of context in container have no effect after 'give'
// 4. dereference should be executed by the last referring tokener. In another
// word, only the tokener who DOES NOT 'give' container to another one or the
// given container have no memory-dependency ctx with privous referred container's
// ctx was allowed to dereference.

#define INVALID_REFERENCE_ID     (reference_handle_t)0xFFFFFFFF
#define TOKEN_DEFAULT   0
#define TOKEN_FIN   1
#define TOKEN_ABNORMAL  2
token_handle_t token_create(const char* name,ctx_convert_t conv_proc, uint32_t keep_alive, token_expired_callback_t dead_callback_t,void* para);
void token_destroy(token_handle_t token);

/*
status: 0 normal,1 fin, 2 abnormal
*/
void token_setstatus(void* token,uint32_t status);

/*
return: 0 token default; 1 token fin; 2 token abnormal
*/
uint32_t token_getstatus(void* token);



void token_serialization(uint32_t token_num,...);
void ls_token_info(token_handle_t token);


container_handle_t container_create(token_handle_t token_owner,bool reference,reference_handle_t* reference_id);
void container_destroy(token_handle_t token,container_handle_t container);

bool container_take(token_handle_t token,container_handle_t* container, bool wait);
bool container_give(token_handle_t token,container_handle_t container, bool wait);

void container_carryon(container_handle_t container,void* ctx, uint32_t len);
void container_carrydown(container_handle_t container,void* ctx, uint32_t len);

reference_handle_t container_wait_dereferenced(token_handle_t token, bool wait);
void container_dereferenced(container_handle_t container);
bool is_container_referenced(container_handle_t container);


#endif