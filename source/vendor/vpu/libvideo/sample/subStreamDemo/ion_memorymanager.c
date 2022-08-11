/*
* File : memorymanager.c
*/

//#define LOG_NDEBUG 0
#define LOG_TAG "memorymanager"

#include <log/log.h>
#include <sys/ioctl.h>
#include <errno.h>
#include "ion_memorymanager.h"
#include "ion.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <time.h>
#include <sys/mman.h>
#include <pthread.h>
#include <asm-generic/ioctl.h>


MEMORY_CONTEXT    *gMemoryContext = NULL;
pthread_mutex_t      gMemoryMutex = PTHREAD_MUTEX_INITIALIZER;

MEMORY_CONTEXT *getInstance()
{
    if(gMemoryContext == NULL)
    {
        ionMemoryOpen();
    }
    return gMemoryContext;
}

int ionMemoryOpen()
{
    pthread_mutex_lock(&gMemoryMutex);
    if (gMemoryContext != NULL)
    {
        ALOGV("CameraMemoryOpen gMemoryContext has been inited");
        goto SUCCEED_OUT;
    }
    gMemoryContext = (MEMORY_CONTEXT*)malloc(sizeof(MEMORY_CONTEXT));
    if (gMemoryContext == NULL) {
        ALOGE("CameraMemoryOpen malloc failed");
        goto ERROR_OUT;
    }

    memset((void*)gMemoryContext, 0, sizeof(MEMORY_CONTEXT));
    gMemoryContext->ionFd = ion_open();

    if (gMemoryContext->ionFd < 0)
    {
        ALOGE("CameraMemoryOpen ion_open failed");
        goto ERROR_OUT;
    }
SUCCEED_OUT:
    gMemoryContext->refCnt++;
    ALOGD("CameraMemoryOpen succesfully refCnt:%d",gMemoryContext->refCnt);
    pthread_mutex_unlock(&gMemoryMutex);
    return 0;
ERROR_OUT:
    if (gMemoryContext != NULL
        && gMemoryContext->ionFd > 0)
    {
        ion_close(gMemoryContext->ionFd);
        gMemoryContext->ionFd = 0;
    }

    if (gMemoryContext != NULL)
    {
        free(gMemoryContext);
        gMemoryContext = NULL;
    }
    ALOGE("CameraMemoryOpen failed!!!!");
    pthread_mutex_unlock(&gMemoryMutex);
    return -1;
}

int ionMemoryClose()
{
    ALOGD("CameraMemoryClose ionFd:%d enter",gMemoryContext->ionFd);
    pthread_mutex_lock(&gMemoryMutex);
    if(gMemoryContext !=NULL)
    {
        ALOGD("CameraMemoryClose gMemoryContext->refCnt:%d",gMemoryContext->refCnt);
        gMemoryContext->refCnt--;
        if(gMemoryContext->refCnt <= 0)
        {
            if(gMemoryContext->ionFd > 0)
            {
                ion_close(gMemoryContext->ionFd);
            }
            gMemoryContext->ionFd = 0;
            free(gMemoryContext);
            gMemoryContext = NULL;
        }
    }
    pthread_mutex_unlock(&gMemoryMutex);
    return 0;
}

int ionMemoryAlloc(int size,int *handleFd)
{
    int ret;
    getInstance();
    if(gMemoryContext == NULL)
        return -1;

    #include "ion_4.12.h"
    int count = 0;
    int i = 0;
    unsigned int mask=ION_HEAP_TYPE_DMA;
    struct ion_heap_data *buffers = NULL;

    if (0 != ion_query_heap_cnt(gMemoryContext->ionFd, &count)) {
        ALOGD("CameraMemoryAlloc get heap count error \n");
        return -1;
    }

    buffers = (struct ion_heap_data *) malloc(sizeof(struct ion_heap_data) * count);
    if (NULL == buffers)
        return -1;

    /*get heap name & id & type */
    if (0 != ion_query_get_heaps(gMemoryContext->ionFd, count, buffers)) {
        free(buffers);
        buffers = NULL;
        return -1;
    }

    for (i = 0; i < count; i++) {
        ALOGD("CameraMemoryAlloc name=%s: id=0x%x: type=0x%x  \n", buffers[i].name,
            buffers[i].heap_id, buffers[i].type);
        if(strcmp(buffers[i].name, "sdrv,services")==0)
            mask = 1<<buffers[i].heap_id;
    }
    free(buffers);
    buffers = NULL;
    ALOGD("CameraMemoryAlloc mask=%u,ION_HEAP_TYPE_DMA:%d\n", mask,ION_HEAP_TYPE_DMA);

    ret = ion_alloc_fd(gMemoryContext->ionFd, size,  4096, mask, 1, handleFd);
    ALOGD("CameraMemoryAlloc gMemoryContext->ionFd:%d,ret:%d",gMemoryContext->ionFd,ret);
    return ret;
}

void* ionGetMemoryViraddr(int size,int handleFd)
{
    void* viraddr;
    getInstance();
    if(gMemoryContext == NULL)
        return NULL;
    viraddr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, handleFd, 0);
    ALOGV("CameraGetMemoryViraddr viraddr:%p",viraddr);
    return viraddr;
}

uint64_t ionGetMemoryPhyaddr(int size ,int handleFd)
{
    ALOGV("CameraGetMemoryPhyaddr size:%d,handleFd:%d",size,handleFd);
    getInstance();
    if(gMemoryContext == NULL)
        return -1;
    return 0;
}

int ionMemoryRelease(void* virAddr,int size,int handleFd)
{
    int ret;
    if(virAddr > 0)
    {
        if (munmap(virAddr, size) != 0)
        {
            ALOGE("CameraMemoryRelease munmap error");
            return -1;
        }
    }
    if(handleFd > 0)
    {
        ion_free(gMemoryContext->ionFd, handleFd);
        if (0 != (ret = ion_close(handleFd)))
        {
            ALOGE("CameraMemoryRelease close handleFd error:%d",ret);

        }
    }
    return 0;

}

