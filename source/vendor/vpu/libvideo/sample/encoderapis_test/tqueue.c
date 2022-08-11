#include "tqueue.h"
#include <malloc.h>

int tqueue_init(TQueue *pQueue, int eNum)
{
    //allocate memory
    pQueue->pTaskArray = (Task *)malloc(eNum * sizeof(Task));
    if (pQueue->pTaskArray == NULL)
        return -1;

    pQueue->qHead = 0;
    pQueue->qTail = 0;
    pQueue->qMax = eNum;
    pthread_mutex_init(&pQueue->mutex, NULL);
    return 0;
}

void tqueue_deinit(TQueue *pQueue)
{
    if (pQueue->pTaskArray)
        free(pQueue->pTaskArray);
}

int tqueue_enQueue(TQueue *pQueue, TaskType type, TaskRun func, void *param, void *param2)
{
    if (!pQueue || !pQueue->pTaskArray)
        return -1;
    pthread_mutex_lock(&pQueue->mutex);
    int next = pQueue->qTail + 1;
    if (next % pQueue->qMax == pQueue->qHead)
    {
        pthread_mutex_unlock(&pQueue->mutex);
        printf("failed in enqueue, it is full\n");
        return -1;
    }

    pQueue->pTaskArray[pQueue->qTail].type = type;
    pQueue->pTaskArray[pQueue->qTail].callback = func;
    pQueue->pTaskArray[pQueue->qTail].param1 = param;
    pQueue->pTaskArray[pQueue->qTail].param2 = param2;
    pQueue->qTail = ++pQueue->qTail % pQueue->qMax;
    pthread_mutex_unlock(&pQueue->mutex);
    return 0;
}

int tqueue_dequeue(TQueue *pQueue, Task *pTask)
{
    if (!pQueue || !pQueue->pTaskArray)
        return -1;
    pthread_mutex_lock(&pQueue->mutex);
    if (pQueue->qTail == pQueue->qHead)
    {
        pthread_mutex_unlock(&pQueue->mutex);
        return -1;
    }
    pTask->type = pQueue->pTaskArray[pQueue->qHead].type;
    pTask->callback = pQueue->pTaskArray[pQueue->qHead].callback;
    pTask->param1 = pQueue->pTaskArray[pQueue->qHead].param1;
    pTask->param2 = pQueue->pTaskArray[pQueue->qHead].param2;

    pQueue->qHead = ++pQueue->qHead % pQueue->qMax;
    pthread_mutex_unlock(&pQueue->mutex);
    return 0;
}
