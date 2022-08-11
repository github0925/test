/*
* File : memorymanager.h
*/

#ifndef _MEMORY_MANAGER_H_
#define _MEMORY_MANAGER_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _MEMORY_CONTEXT
{
    int  ionFd;
    int refCnt;
}MEMORY_CONTEXT;

int ionMemoryOpen();
int ionMemoryClose();
int ionMemoryAlloc(int size,int *handleFd);
void* ionGetMemoryViraddr(int size,int handleFd);
uint64_t ionGetMemoryPhyaddr(int size ,int handleFd);
int ionMemoryRelease(void* virAddr,int size,int handleFd);
#ifdef __cplusplus
}
#endif

#endif//  _MEMORY_MANAGER_H_
