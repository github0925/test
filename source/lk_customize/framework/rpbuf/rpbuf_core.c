//*****************************************************************************
//
// rpbuf.c - Driver for the rpbuffer shared among DCF layers
//
// Copyright (c) 2019-2029 SemiDrive Incorporated.  All rights reserved.
// Software License Agreement
//
//*****************************************************************************

#include <stdlib.h>
#include <trace.h>
#include <string.h>
#include <assert.h>
#include <err.h>
#include <platform.h>
#include <sys/types.h>
#include <pow2.h>
#include <platform/interrupts.h>
#include <platform/debug.h>
#include <kernel/thread.h>
#include <kernel/spinlock.h>
#include <kernel/event.h>

#include "rpbuf.h"

/* Module features */

sd_rpbuf_pool_t *sd_rpbuf_init_pool(unsigned long mem_base, unsigned int mem_size, unsigned int bufnum)
{
    unsigned int i;
    sd_rpbuf_pool_t *pool;
    sd_rpbuf_t *rpbuf;
    unsigned long buf_start;

    pool = (sd_rpbuf_pool_t *) mem_base;
    pool->buf_sz = mem_size - sizeof(sd_rpbuf_pool_t) - bufnum * sizeof(sd_rpbuf_t);
    pool->buf_sz /= bufnum;
    pool->buf_num = bufnum;
    pool->free_buf = bufnum;

    buf_start = mem_base + sizeof(sd_rpbuf_pool_t) + bufnum * sizeof(sd_rpbuf_t);

    spin_lock_init(&pool->pool_lock);

    for (i = 0;i < bufnum;i++) {
        rpbuf = &pool->buffers[i];
        rpbuf->buffer = (void *) (buf_start + i * pool->buf_sz);
        if ((unsigned long)rpbuf->buffer >= mem_base + mem_size) {
            dprintf(CRITICAL, "failed to initialize rpbuf pool\n");
            return NULL;
        }
        dprintf(1, "init buffer[%d]=%p\n", i, rpbuf->buffer);
        rpbuf->buf_size = pool->buf_sz;
        rpbuf->data_len = 0;
        rpbuf->ref_count = 0;
        rpbuf->wroff = 0;
    }

    return pool;
}

// indicate mem malloc internally
#define RPBUF_POOL_ALLOC  (1)

sd_rpbuf_pool_t *sd_rpbuf_create_pool(unsigned int bufsize, unsigned int bufnum)
{
    sd_rpbuf_pool_t *pool;
    unsigned int sz_align;
    unsigned int mem_size;

    sz_align = round_up_pow2_u32(bufsize);
    mem_size = sizeof(sd_rpbuf_pool_t) + bufnum *(sz_align + sizeof(sd_rpbuf_t));
    pool = malloc(mem_size);
    if (!pool) {
        dprintf(0, "No memory for rpbuf pool\n");
        return NULL;
    }
    pool->flags = RPBUF_POOL_ALLOC;
    dprintf(INFO, "new pool=%p size=%d bs=%d bn=%d\n", pool, mem_size, sz_align, bufnum);

    sd_rpbuf_init_pool((unsigned long )pool, mem_size, bufnum);

    return pool;
}

void sd_rpbuf_remove_pool(sd_rpbuf_pool_t *pool)
{
    sd_rpbuf_t *rpbuf;
    unsigned int i;

    if (pool->free_buf != pool->buf_num) {
        dprintf(0, "Risky to free buffer in use, checking buffers\n");

        for (i = 0;i < pool->buf_num;i++) {
            rpbuf = &pool->buffers[i];
            if (rpbuf->ref_count)
            dprintf(0, "to free buffer=%d in use refcnt=%d\n", i, rpbuf->ref_count);
        }
        return;
    } else {
        if (pool->flags & RPBUF_POOL_ALLOC)
            free(pool);
    }
}

sd_rpbuf_t *sd_rpbuf_alloc(sd_rpbuf_pool_t *pool, unsigned int flag)
{
    unsigned int i;
    sd_rpbuf_t *rpbuf;
    spin_lock_saved_state_t state;

    spin_lock_irqsave(&pool->pool_lock, state);
    for (i = 0;i < pool->buf_num; i++) {
        rpbuf = &pool->buffers[i];
        if (!rpbuf->ref_count) {
            rpbuf->ref_count++;
            rpbuf->flags = flag;
            rpbuf->buf_id = i;
            rpbuf->wroff = 0;
            pool->free_buf--;
            spin_unlock_irqrestore(&pool->pool_lock, state);

            dprintf(2, "alloc rpbuf[%d]=%p f=%d\n", i, rpbuf->buffer, rpbuf->flags);
            return rpbuf;
        }
    }
    spin_unlock_irqrestore(&pool->pool_lock, state);

    return NULL;
}

static int free_rpbuf_locked(sd_rpbuf_t *rpbuf)
{
    ASSERT(1 == rpbuf->ref_count);
    rpbuf->ref_count--;
    rpbuf->data_len = 0;
    rpbuf->wroff = 0;
    dprintf(2, "free rpbuf[%d]=%p f=%d\n", rpbuf->buf_id, rpbuf->buffer, rpbuf->flags);
    return 0;
}

int sd_rpbuf_free_ptr(sd_rpbuf_pool_t *pool, void *buf)
{
    unsigned int i;
    sd_rpbuf_t *rpbuf;
    spin_lock_saved_state_t state;

    spin_lock_irqsave(&pool->pool_lock, state);
    for (i = 0, rpbuf = &pool->buffers[0]; i < pool->buf_num; i++, rpbuf++) {
        if (buf == rpbuf->buffer) {
            free_rpbuf_locked(rpbuf);
            pool->free_buf++;
            spin_unlock_irqrestore(&pool->pool_lock, state);
            return 0;
        }
    }
    spin_unlock_irqrestore(&pool->pool_lock, state);
    dprintf(0, "fail to free rpbuf=%p\n", buf);

    return -1;
}

int sd_rpbuf_free_id(sd_rpbuf_pool_t *pool, unsigned int buf_id)
{
    spin_lock_saved_state_t state;

    if (buf_id > pool->buf_num)
        return -1;

    spin_lock_irqsave(&pool->pool_lock, state);
    free_rpbuf_locked(&pool->buffers[buf_id]);
    pool->free_buf++;
    spin_unlock_irqrestore(&pool->pool_lock, state);

    return 0;
}

int sd_rpbuf_ref(sd_rpbuf_pool_t *pool, sd_rpbuf_t *rpbuf)
{
    spin_lock_saved_state_t state;

    spin_lock_irqsave(&pool->pool_lock, state);

    rpbuf->ref_count++;

    spin_unlock_irqrestore(&pool->pool_lock, state);

    return 0;
}

int sd_rpbuf_unref(sd_rpbuf_pool_t *pool, sd_rpbuf_t *rpbuf)
{
    spin_lock_saved_state_t state;

    spin_lock_irqsave(&pool->pool_lock, state);

    dprintf(0, "%s rpbuf=%d refcnt=%d\n", __func__, rpbuf->buf_id, rpbuf->ref_count);
    if (1 == rpbuf->ref_count) {
        free_rpbuf_locked(rpbuf);
        pool->free_buf++;
    } else
        rpbuf->ref_count--;

    spin_unlock_irqrestore(&pool->pool_lock, state);

    return 0;
}

sd_rpbuf_t *sd_rpbuf_find_handle(sd_rpbuf_pool_t *pool, void *buf)
{
    unsigned int i;
    sd_rpbuf_t *rpbuf;
    spin_lock_saved_state_t state;

    spin_lock_irqsave(&pool->pool_lock, state);
    for (i = 0, rpbuf = &pool->buffers[0]; i < pool->buf_num; i++, rpbuf++) {
        if (buf == rpbuf->buffer) {
            spin_unlock_irqrestore(&pool->pool_lock, state);
            return rpbuf;
        }
    }
    spin_unlock_irqrestore(&pool->pool_lock, state);

    return NULL;
}

int sd_rpbuf_push_data(sd_rpbuf_t *rpbuf, void *data, u16 len)
{
    if (len > rpbuf->buf_size - rpbuf->wroff) {
        dprintf(0, "Can't push len %u > buf %u, trim\n", len, rpbuf->buf_size);
        len = rpbuf->buf_size - - rpbuf->wroff;
    }

    memcpy(rpbuf->buffer + rpbuf->wroff, data, len);
    rpbuf->data_len += len;
    rpbuf->wroff += len;

    return 0;
}

void *sd_rpbuf_pop_data(sd_rpbuf_t *rpbuf, void *buf, u16 len)
{
    unsigned char *data = rpbuf->buffer + rpbuf->wroff;

    if (len > rpbuf->data_len) {
        dprintf(0, "Can't pop enough data len %d > actual %d\n", len, rpbuf->data_len);
        return NULL;
    }

    if (buf || len)
        memcpy(buf, data, len);

    return data;
}

int sd_rpbuf_init_queue(sd_rpbuf_queue_t *rq)
{
    spin_lock_init(&rq->queue_lock);
    list_initialize(&rq->queue_head);
    rq->buf_num = 0;

    return 0;
}

int sd_rpbuf_remove_queue(sd_rpbuf_queue_t *rq)
{
    sd_rpbuf_t *rpbuf = NULL;
    spin_lock_saved_state_t state;

    spin_lock_irqsave(&rq->queue_lock, state);
    while (!list_is_empty(&rq->queue_head)) {
        rpbuf = list_remove_head_type(&rq->queue_head, sd_rpbuf_t, buf_node);
        if (rpbuf) {
            dprintf(0, "remove pending rpbuf=%p, id=%d\n", rpbuf, rpbuf->buf_id);
        }
        rq->buf_num--;

    }
    spin_unlock_irqrestore(&rq->queue_lock, state);

    return 0;
}

void sd_rpbuf_enqueue(sd_rpbuf_queue_t *rq, sd_rpbuf_t *rpbuf)
{
    spin_lock_saved_state_t state;

    spin_lock_irqsave(&rq->queue_lock, state);
    list_add_tail(&rq->queue_head,
        (struct list_node *)(&rpbuf->buf_node));
    rq->buf_num++;
    spin_unlock_irqrestore(&rq->queue_lock, state);
}

sd_rpbuf_t *sd_rpbuf_dequeue(sd_rpbuf_queue_t *rq)
{
    spin_lock_saved_state_t state;
    sd_rpbuf_t *rpbuf = NULL;

    spin_lock_irqsave(&rq->queue_lock, state);
    rpbuf = list_remove_head_type(&rq->queue_head, sd_rpbuf_t, buf_node);
    rq->buf_num--;
    spin_unlock_irqrestore(&rq->queue_lock, state);

    return rpbuf;
}

bool sd_rpbuf_queue_empty(sd_rpbuf_queue_t *rq)
{
    spin_lock_saved_state_t state;
    bool ret;

    spin_lock_irqsave(&rq->queue_lock, state);
    ret = list_is_empty(&rq->queue_head);
    spin_unlock_irqrestore(&rq->queue_lock, state);

    return ret;
}

