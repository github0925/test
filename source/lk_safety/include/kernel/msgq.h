/*
 * Copyright (c) 2008-2014 Travis Geiselbrecht
 * Copyright (c) 2012 Shantanu Gupta
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#ifndef __KERNEL_MSGQ_H
#define __KERNEL_MSGQ_H

#include <compiler.h>
#include <debug.h>
#include <stdint.h>
#include <FreeRTOS.h>
#include <queue.h>

__BEGIN_CDECLS;


typedef QueueHandle_t queue_t;

/***************** API DECLARATION *****************/

/* Rules for queues:
 * - Forbidden to call queue API in interrupt context.
*/

static inline void queue_create(queue_t *q, uint32_t len, uint32_t item_size )
{
    *q =  xQueueCreate( len, item_size );
    ASSERT(*q);
}


static inline void queue_destroy(queue_t* q)
{
    vQueueDelete(*q);
}

static inline void queue_send(queue_t* q, void* item)
{
    xQueueSend(*q,item,portMAX_DELAY);
}

static inline void queue_recv(queue_t* q,void* buf)
{
    xQueueReceive(*q,buf,portMAX_DELAY);
}

static inline int queue_query(queue_t* q,void* buf)
{
    return ((xQueueReceive(*q,buf,0) == pdTRUE) ? 1 : 0);
}

static inline void queue_peek(queue_t* q, void* buf)
{
    xQueuePeek(*q,buf,portMAX_DELAY);
}


__END_CDECLS;
#endif

