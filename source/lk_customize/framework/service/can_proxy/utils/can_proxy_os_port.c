/*
 * can proxy os port layer.
 * Copyright (c) 2020 Semidrive Semiconductor, Inc.
 * All rights reserved.
 *
 * User should placed appropriate os implementation
 * here to adapt different os.
 */

#include "lk_wrapper.h"
#include <message_buffer.h>


void* os_msg_buf_create(size_t buffer_size)
{
    return xMessageBufferCreate(buffer_size);
}

/* variable length msg post function. Should support interrupt context. */
size_t os_msg_buf_post(void* handle, void* data, size_t len)
{
    size_t size = 0;
    if(ulPortInterruptNesting > 0)
    {
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        size = xMessageBufferSendFromISR(handle,data,len,&xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
    else
    {
        size = xMessageBufferSend(handle,data,len,portMAX_DELAY);
    }

    return size;

}

size_t os_msg_buf_get(void* handle, void* data, size_t len)
{
    size_t size = 0;
    if(ulPortInterruptNesting > 0)
    {
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        size = xMessageBufferReceiveFromISR(handle,data,len,&xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
    else
    {
        size = xMessageBufferReceive(handle,data,len,portMAX_DELAY);
    }

    return size;

}


