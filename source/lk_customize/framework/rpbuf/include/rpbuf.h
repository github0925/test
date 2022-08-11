//*****************************************************************************
//
// rpbuf.h - Prototypes for the rpbuf hal
//
// Copyright (c) 2019-2029 Semidrive Incorporated.  All rights reserved.
// Software License Agreement
//
//
//*****************************************************************************

#ifndef __SD_RPBUF_H__
#define __SD_RPBUF_H__
//*****************************************************************************
//
// If building with a C++ compiler, make all of the definitions in this header
// have a C binding.
//
//*****************************************************************************
#ifdef __cplusplus
extern "C"
{
#endif

#include <stdlib.h>
#include <trace.h>
#include <string.h>
#include <assert.h>
#include <err.h>
#include <platform.h>
#include <sys/types.h>
#include <kernel/spinlock.h>
#include <platform/interrupts.h>
#include <platform/debug.h>

#define SD_RPBUF_F_RX       (1<<0)
#define SD_RPBUF_F_TX       (1<<1)

typedef struct sd_rpbuf {
    struct list_node buf_node;
    unsigned char *buffer;
    u16 buf_size;
    u16 data_len;
    u16 wroff;
    u16 buf_id;
    u16 ref_count;
    u16 flags;
} __PACKED __ALIGNED(CACHE_LINE) sd_rpbuf_t;

typedef struct sd_rpbuf_pool {
    u32 buf_num;
    u32 buf_sz;
    u32 free_buf;
    u32 flags;
    spin_lock_t pool_lock;
    sd_rpbuf_t buffers[0];
} __ALIGNED(CACHE_LINE) sd_rpbuf_pool_t;

typedef struct sd_rpbuf_queue {
    struct list_node queue_head;
    uint32_t buf_num;
    spin_lock_t queue_lock;

} sd_rpbuf_queue_t;

/* rpbuf buffer pool interface */
sd_rpbuf_pool_t *sd_rpbuf_init_pool(unsigned long mem_base, unsigned int mem_size, unsigned int bufnum);
sd_rpbuf_pool_t *sd_rpbuf_create_pool(unsigned int bufsize, unsigned int bufnum);
void sd_rpbuf_remove_pool(sd_rpbuf_pool_t *pool);

/* rpbuf buffer node interface */
int sd_rpbuf_ref(sd_rpbuf_pool_t *pool, sd_rpbuf_t *rpbuf);
int sd_rpbuf_unref(sd_rpbuf_pool_t *pool, sd_rpbuf_t *rpbuf);
sd_rpbuf_t *sd_rpbuf_alloc(sd_rpbuf_pool_t *pool, unsigned int flag);
int sd_rpbuf_free(sd_rpbuf_t *rpbuf);
int sd_rpbuf_free_id(sd_rpbuf_pool_t *pool, unsigned int buf_id);
int sd_rpbuf_free_ptr(sd_rpbuf_pool_t *pool, void *buf);
sd_rpbuf_t *sd_rpbuf_find_handle(sd_rpbuf_pool_t *pool, void *buf);
int sd_rpbuf_push_data(sd_rpbuf_t *rpbuf, void *data, u16 len);
void *sd_rpbuf_pop_data(sd_rpbuf_t *rpbuf, void *buf, u16 len);

/* rpbuf buffer queue interface */
int sd_rpbuf_init_queue(sd_rpbuf_queue_t *rq);
int sd_rpbuf_remove_queue(sd_rpbuf_queue_t *rq);
void sd_rpbuf_enqueue(sd_rpbuf_queue_t *rq, sd_rpbuf_t *rpbuf);
sd_rpbuf_t *sd_rpbuf_dequeue(sd_rpbuf_queue_t *rq);
bool sd_rpbuf_queue_empty(sd_rpbuf_queue_t *rq);

/* other helper interface */
inline static void sd_rpbuf_reserve(sd_rpbuf_t *rpbuf, u16 len)
{
    rpbuf->wroff += len;
}

#ifdef __cplusplus
}
#endif
#endif // __SD_RPBUF_H__

