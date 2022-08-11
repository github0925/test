/*
 * Copyright (c) 2019, Semidrive, Inc. All rights reserved
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
#ifndef _SLT_MESSAGE_H_
#define _SLT_MESSAGE_H_

#include <slt_main.h>
#include <mbox_hal.h>

#define SLT_MESSAGE_IPC_MSG_LEN_MAX 384

//slt_ipc_msg size is smaller than
typedef struct slt_ipc_msg {
    uint32_t cmd_type;
    uint32_t msg_len;
//slt msg
    char msg[SLT_MESSAGE_IPC_MSG_LEN_MAX];

} slt_ipc_msg_t;

int slt_ipc_init(slt_app_context_t* pcontext);
int slt_ipc_send_msg(slt_app_context_t* pcontext, slt_ipc_msg_t* msg, struct ipcc_channel* chan);
int slt_uart_send_msg(slt_app_context_t* pcontext, void* msg);
#endif  /* _SLT_MESSAGE_H_ */
