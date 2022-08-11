/*************************************************************************/ /*!
@File
@Title          Interface for managing worker threads which are used to
                perform deferred work, such as freeing of resources.
@Copyright      Copyright (c) Imagination Technologies Ltd. All Rights Reserved
@Description    Functions to handle asynchronous freeing of device memory
@License        Strictly Confidential.
*/ /**************************************************************************/

#ifndef PVRSRV_TASK_H
#define PVRSRV_TASK_H

#include "img_types.h"
#include "img_defs.h"
#include "pvrsrv_error.h"
#include "services.h"

#define PVRSRV_TASK_FLAG_NONE          (0U)
/* flag is free to use                 (1U<<0) */
#define PVRSRV_TASK_FLAG_AUTO_CLEANUP  (1U<<1)
#define PVRSRV_TASK_FLAG_WAITABLE      (1U<<2)
#define PVRSRV_TASK_FLAG_PTHREAD       (1U<<3)
#if defined(GTRACE_TOOL)
#define PVRSRV_TASK_FLAG_PTHREAD_INDEX_CLR_MASK	   (0xFFEFFFFF)
#define PVRSRV_TASK_FLAG_PTHREAD_INDEX_SHIFT	   (20)
#endif

typedef enum PVRSRV_TaskStateTAG
{
	PVRSRV_TASK_STATE_UNQUEUED   = 0,	/*!< Not in task queue */
	PVRSRV_TASK_STATE_QUEUED,			/*!< In task queue */
	PVRSRV_TASK_STATE_RUNNING,			/*!< Not in task queue, owned by worker thread */
}PVRSRV_TaskState;


typedef enum PRVSRV_TaskPriorityTAG
{
	/*! The task doesn't have any priority requirements and the framework can do
	 * whatever fits best */
	PVRSRV_TASK_PRIORITY_AUTO       = 0,
	/*! The task is high priority and should be finished as fast as possible */
	PVRSRV_TASK_PRIORITY_REALTIME   = 1,
	/*! The task is low priority and can be scheduled after more important tasks */
	PVRSRV_TASK_PRIORITY_BACKGROUND = 2,

}PVRSRV_TaskPriority;

typedef PVRSRV_ERROR (*PVRSRV_TaskCallback)(void*);

/**************************************************************************/ /*!
@Function       PVRSRVCreateTaskContext
@Description    Create a task context, on which deferred tasks can be queued.
@Output         ppsContext     The created context.
@Return         PVRSRV_OK on success, or an error code on failure.
*/ /***************************************************************************/
IMG_EXPORT PVRSRV_ERROR
PVRSRVCreateTaskContext(PVRSRV_TASK_CONTEXT **ppsContext, PVRSRV_TASK_CONTEXT_SETUP *psSetup);

/**************************************************************************/ /*!
@Function       PVRSRVRefTaskContext
@Description    Adds a reference on the given task context, which was previously
                created by calling PVRSRVCreateTaskContext.
@Output         psContext      The context to add a reference on.
@Return         PVRSRV_OK on success, or an error code on failure.
*/ /***************************************************************************/
IMG_EXPORT PVRSRV_ERROR
PVRSRVRefTaskContext(PVRSRV_TASK_CONTEXT *psContext, PVRSRV_TASK_CONTEXT_SETUP *psSetup);

#define PVRSRV_TASK_FLUSH_TIMEOUT_FOREVER 0xFFFFFFFF
#define PVRSRV_TASK_FLUSH_TIMEOUT_DEFAULT 0x0

/**************************************************************************/ /*!
@Function       PVRSRVFlushTaskContext
@Description    Blocks until the given task context no longer has any pending
                tasks belonging to the given device connection.
@Input          psContext      The context to check
@Input          psConnection   The device connection to look for pending tasks
                               belonging to. Pass NULL to flush all tasks.
@Input          ui32Timeousus  Timeout (in microseconds) after which the flush is
                               abandoned an PVRSRV_ERROR_TIMEOUT is returned.
                               Pass PVRSRV_TASK_FLUSH_TIMEOUT_FOREVER to have no timeout,
                               or pass PVRSRV_TASK_FLUSH_TIMEOUT_DEFAULT to use a default
                               timeout.
@Return         PVRSRV_OK on success, or an error code on failure.
*/ /***************************************************************************/
IMG_EXPORT PVRSRV_ERROR
PVRSRVFlushTaskContext(PVRSRV_TASK_CONTEXT *psContext,
                                             PVRSRV_DEV_CONNECTION *psConnection,
                                             IMG_UINT32 ui32Timeoutus);

/**************************************************************************/ /*!
@Function       PVRSRVDestroyTaskContext
@Description    Destroy a task context. If the task context has multiple reference
                then the reference count will be decremented and PVRSRV_ERROR_OBJECT_STILL_REFERENCED
                will be returned.
@Input         psContext     The context to destroy or decrement the reference count on.
@Return         PVRSRV_OK if destroyed, PVRSRV_ERROR_OBJECT_STILL_REFERENCED if the reference count was
                successfully decremented and there are still references to the task,
                or an error code on failure.
*/ /***************************************************************************/
IMG_EXPORT PVRSRV_ERROR
PVRSRVDestroyTaskContext(PVRSRV_TASK_CONTEXT *psContext);

/**************************************************************************/ /*!
@Function       PVRSRVCreateDeferredTask
@Description    Creates a new deferred task.
                Deferred tasks or workqueues may be used to cleanup
                resources.
                This function is only used if DEFERRED_WORKER_THREAD is
                defined.
@Output         pHandle         Handle to the newly created task
@Input          psContext       The task context to associate the new task with
@Input          pfnCallback     Callback function
@Input          pvPrivData      Private data passed to callback
@Input          ePriority       Task priority
@Input          ui32Flags       Flags to configure the task
@Input          hEventObject    Event object to wait on before trying this task
                                Required for non-PTHREAD type tasks.
@Input          pszInfo         Debug string
@Return         PVRSRV_OK, or PVRSRV_ERROR_<something> if something goes wrong
*/ /***************************************************************************/
IMG_EXPORT PVRSRV_ERROR
PVRSRVCreateDeferredTask(IMG_HANDLE*         pHandle,
                         PVRSRV_TASK_CONTEXT *psContext,
                         PVRSRV_DEV_CONNECTION *psConnection,
                         PVRSRV_TaskCallback pfnCallback,
                         void*               pvPrivData,
                         PVRSRV_TaskPriority ePriority,
                         IMG_UINT32          ui32Flags,
                         IMG_HANDLE          hEventObject,
                         IMG_CHAR*           pszInfo);


/**************************************************************************/ /*!
@Function       PVRSRVDestroyDeferredTask
@Description    Destroys a deferred task.
                This function is only used if DEFERRED_WORKER_THREAD is
                defined.
@Input          hTask     The handle of the task to destroy
@Return         PVRSRV_OK, or PVRSRV_ERROR_<something> if something goes wrong
*/ /***************************************************************************/
IMG_EXPORT PVRSRV_ERROR
PVRSRVDestroyDeferredTask(IMG_HANDLE hTask);


/**************************************************************************/ /*!
@Function       PVRSRVQueueDeferredTask
@Description    Queues a task to run asynchronously.
                It is an error to queue a task that is PAUSED or QUEUED.
                This function is only used if DEFERRED_WORKER_THREAD is
                defined.
@Input          psDevConnection    Connection to the device
@Input          hTask              Task handle
@Return         PVRSRV_OK, or PVRSRV_ERROR_<something> if something goes wrong
*/ /***************************************************************************/
IMG_EXPORT PVRSRV_ERROR
PVRSRVQueueDeferredTask(PVRSRV_TASK_CONTEXT *psContext, IMG_HANDLE hTask);


/**************************************************************************/ /*!
@Function       PVRSRVQueueOneTimeDeferredTask
@Description    Queues a one time task to run asynchronously. This is equal
                to calling PVRSRVCreateDeferredTask with the
                PVRSRV_TASK_FLAG_AUTO_CLEANUP flag and running
                PVRSRVQueueDeferredTask afterwards.
                This function is only used if DEFERRED_WORKER_THREAD is
                defined.
@Input          psDevConnection   Connection to the device
@Input          pfnCallback       Callback function
@Input          pvPrivData        Private data passed to callback
@Input          ePriority         Task priority
@Input          ui32Flags         Flags to configure the task
@Input          pszInfo           Debug string
@Return         PVRSRV_OK, or PVRSRV_ERROR_<something> if something goes wrong
*/ /***************************************************************************/
IMG_EXPORT PVRSRV_ERROR
PVRSRVQueueOneTimeDeferredTask(PVRSRV_TASK_CONTEXT *psContext,
                               PVRSRV_DEV_CONNECTION *psConnection,
                               PVRSRV_TaskCallback     pfnCallback,
                               void*                   pvPrivData,
                               PVRSRV_TaskPriority     ePriority,
                               IMG_UINT32              ui32Flags,
                               IMG_HANDLE              hEventObject,
                               IMG_CHAR*               pszInfo);

/**************************************************************************/ /*!
@Function       PVRSRVRetrieveTaskErrorState
@Description    Query the error state a given task is currently in.
                This function is only used if DEFERRED_WORKER_THREAD is
                defined.
@Input          hTask            Task handle
@Return         PVRSRV_ERROR_INVALID_PARAMS if the handle is invalid
                or the last error number of the task
*/ /***************************************************************************/
IMG_EXPORT PVRSRV_ERROR
PVRSRVRetrieveTaskErrorState(IMG_HANDLE hTask);

/**************************************************************************/ /*!
@Function       PVRSRVWaitDeferredTask
@Description    Wait for a queued deferred task to complete
                This function is only used if DEFERRED_WORKER_THREAD is
                defined.
@Input          hTask            Task handle
@Return         PVRSRV_ERROR_INVALID_PARAMS if the handle is invalid
                or PVRSRV_OK if eveything is fine.
*/ /***************************************************************************/
IMG_EXPORT PVRSRV_ERROR
PVRSRVWaitDeferredTask(IMG_HANDLE hTask);

#if defined(GTRACE_TOOL)
/**************************************************************************/ /*!
@Function       PVRSRVRetrieveCurrentWorkerQueueGTrace
@Description    Get the GTrace for the current thread where this function is called.
                This function is only used if DEFERRED_WORKER_THREAD is
                defined and GTRACE_TOOL is defined.
@Input
@Return         Return pointer to GTrace.  If the function is not called in any of
                worker threads, return NULL.
*/ /***************************************************************************/
IMG_EXPORT struct GTraceTAG*
PVRSRVRetrieveCurrentWorkerQueueGTrace(PVRSRV_TASK_CONTEXT *psContext);
#endif

#endif /* PVRSRV_TASK_H */
