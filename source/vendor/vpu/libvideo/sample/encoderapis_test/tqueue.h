/*******************************************************************************
 *           Copyright (C) 2020 Semidrive Technology Ltd. All rights reserved.
 ******************************************************************************/

/*******************************************************************************
 * FileName : tqueue.h
 * Version  : 1.0.0
 * Purpose  : sample app header file
 * Authors  : wei.fan
 * Date     : 2021-06-25
 * Notes    :
 ******************************************************************************/

#ifndef THREAD_QUEUE_
#define THREAD_QUEUE_

#include <pthread.h>

typedef void(*TaskRun)(void *, void*);
typedef enum {
   GENARATE_INFO,
   ENCODING,
   REQUEST_I,
   CHANGE_BR,
   CHANGE_FR,
   EOS
}TaskType;

typedef struct {
   void* param1;
   void* param2;
   TaskRun callback;
   TaskType type;
} Task;


typedef struct {
    Task* pTaskArray;
    int qHead;
    int qTail;
    int qMax;
    pthread_mutex_t mutex;
} TQueue;

int tqueue_init(TQueue * pQueue,int eNum);
void tqueue_deinit(TQueue *pQueue);
int tqueue_enQueue(TQueue *pQueue, TaskType type, TaskRun func, void* param, void* param2);
int tqueue_dequeue(TQueue *pQueue, Task* pTask);
#endif