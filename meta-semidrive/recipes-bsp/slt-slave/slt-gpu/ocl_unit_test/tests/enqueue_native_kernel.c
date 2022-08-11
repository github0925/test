/*************************************************************************/ /*!
@File           enqueue_native_kernel.c
@Title          Unit tests for the clEnqueueNativeKernel function
@Copyright      Copyright (c) Imagination Technologies Ltd. All Rights Reserved
@Description    Contains tests for clEnqueueNativeKernel and
                OCL_FlushNativeKernel that enqueue NativeKernel on a command
                queue with buffer objects as arguments, and then runs it with
                clFinish - verifying that NativeKernel can interact with data
                stored in buffer objects and any changes are visible on
                completion of the function.
@License        Strictly Confidential.
*/ /**************************************************************************/

#define ARG_COUNT          (100)
#define BUFFERED_OBJ_COUNT (2)

/* This is the structure of arguments that the NativeKernel function expects */
typedef struct _NATIVE_KERNEL_ARGS_
{
	uint32_t *pui32Src;
	uint32_t *pui32Dst;
	uint32_t ui32SrcCnt;
} NATIVE_KERNEL_ARGS, *PNATIVE_KERNEL_ARGS;

typedef struct _ENQUEUE_NATIVE_KERNEL_DATA_
{
	/* CL Objects */
	cl_context       psContext;
	cl_device_id     psDeviceID;
	cl_platform_id   psPlatformID;
	cl_command_queue psCommandQueue;
	cl_event         *ppsEvent;

	/* clEnqueueNativeKernel arguments */
	void             (*pfnUserFunc)(void*);
	void             *pvArgs;
	size_t           uiCbArgs;
	cl_uint          uiNumMemObjs;
	cl_mem           *ppsMemList;
	void             **ppvArgsMemLoc;
	cl_uint          uiNumEventsInWaitList;
	cl_event         *ppsEventWaitList;

	/* Copy of the random data passed through args to allow for verification */
	uint32_t         *pui32OriginalSrc;
} ENQUEUE_NATIVE_KERNEL_DATA, *PENQUEUE_NATIVE_KERNEL_DATA;

/*
	This struct defines the structure of arguments to be passed through
	clEnqueueNativeKernel. It's the responsibility of clEnqueueNativeKernel
	to unbox the buffer objects and ultimately pass arguments in the form of
	NATIVE_KERNEL_ARGS
*/
typedef struct _NATIVE_KERNEL_BUFFERED_ARGS_
{
	cl_mem   psSrcBuffer;
	cl_mem   psDstBuffer;
	uint32_t ui32SrcCnt;
} *PNATIVE_KERNEL_BUFFERED_ARGS;

static void CL_CALLBACK NativeKernel(void *pvArgs);

static cl_int Init_EnqueueNativeKernel(OCLTestInstance *psInstance);
static cl_int Compute_EnqueueNativeKernel(OCLTestInstance *psInstance);
static cl_int Verify_EnqueueNativeKernel(OCLTestInstance *psInstance);

/*************************************************************************/ /*!
 @Function		NativeKernel
 @Description	This function is used to read some cl_uints from a source and
 				write these to a specified destination.
 @Input			pvArgs	Contains the source and dest locations and number of
 						values to write in the format of a TestFunctionArgs.
 @Return		void
*/ /**************************************************************************/
static void CL_CALLBACK NativeKernel(void *pvArgs)
{
	PNATIVE_KERNEL_ARGS psCastedArgs = (PNATIVE_KERNEL_ARGS) pvArgs;

	if (!(psCastedArgs && psCastedArgs->pui32Src && psCastedArgs->pui32Dst))
	{
		OCLTestLog("%s: Null pointer passed to NativeKernel\n", __func__);
		return;
	}

	for (uint32_t i = 0; i < psCastedArgs->ui32SrcCnt; i++)
	{
		psCastedArgs->pui32Dst[i] = psCastedArgs->pui32Src[i];
	}
}

/*************************************************************************/ /*!
 @Function   	Init_EnqueueNativeKernel
 @Description	This function is used to set up the program, context, device ID
 				etc necessary before the native kernel(s) are enqueued. It also
 				randomly sets the src values that will be passed to
				NativeKernel.
 @Output	 	psInstance			The function sets the necessary private
 									data to run the native kernel(s), such as
									the device IDs, pointers to the native
									kernel(s) to run, the arguments to call the
									native kernel(s) with, etc.
 @Return     	CL_OUT_OF_RESOURCES if there is not enough memory to allocate
 									the necessary data.
             	CL_SUCCESS 			otherwise.
*/ /**************************************************************************/
static cl_int Init_EnqueueNativeKernel(OCLTestInstance *psInstance)
{
	cl_int iResult = CL_SUCCESS;

	PENQUEUE_NATIVE_KERNEL_DATA psData = (PENQUEUE_NATIVE_KERNEL_DATA) malloc(sizeof(*psData));

	iResult = clGetPlatformIDs(1, &psData->psPlatformID, NULL);
	CheckAndReportError(psInstance,
						"clGetPlatformIDs",
						iResult,
						init_enqueue_native_kernel_cleanup);

	iResult = clGetDeviceIDs(psData->psPlatformID,
							 CL_DEVICE_TYPE_GPU | CL_DEVICE_TYPE_ACCELERATOR,
							 1,
							 &psData->psDeviceID,
							 NULL);
	CheckAndReportError(psInstance,
						"clGetDeviceIDs",
						iResult,
						init_enqueue_native_kernel_cleanup);

	psData->psContext = clCreateContext(NULL,
										1,
										&psData->psDeviceID,
										NULL,
										NULL,
										&iResult);
										CheckAndReportError(psInstance,
										"clCreateContext",
										iResult,
										init_enqueue_native_kernel_cleanup);

	psData->psCommandQueue = clCreateCommandQueue(psData->psContext,
												  psData->psDeviceID,
												  0,
												  &iResult);
												  CheckAndReportError(psInstance,
												  "clCreateCommandQueue",
												  iResult,
												  init_enqueue_native_kernel_cleanup);

	psData->pfnUserFunc = NativeKernel;

	uint32_t sourceValues[ARG_COUNT];
	uint32_t destValues[ARG_COUNT];
	psData->pui32OriginalSrc = calloc(ARG_COUNT, sizeof(sourceValues[0]));

	/* Set the random seed with which to set the source values */
	srand(time(NULL));

	for (cl_uint i = 0; i < ARG_COUNT; i++)
	{
		uint32_t ui32Rand = rand() % CL_UINT_MAX;
		sourceValues[i] = ui32Rand;
		psData->pui32OriginalSrc[i] = ui32Rand;
	}

	PNATIVE_KERNEL_BUFFERED_ARGS psBufferedArgs = malloc(sizeof(*psBufferedArgs));

	psBufferedArgs->psSrcBuffer = clCreateBuffer(psData->psContext,
												 CL_MEM_COPY_HOST_PTR,
												 ARG_COUNT * sizeof(sourceValues[0]),
												 sourceValues,
												 &iResult);
	CheckAndReportError(psInstance,
						"clEnqueueNativeKernel",
						iResult,
						init_enqueue_native_kernel_cleanup);

	psBufferedArgs->psDstBuffer = clCreateBuffer(psData->psContext,
												 CL_MEM_COPY_HOST_PTR,
												 ARG_COUNT * sizeof(destValues[0]),
												 destValues,
												 &iResult);
	CheckAndReportError(psInstance,
						"clEnqueueNativeKernel",
						iResult,
						init_enqueue_native_kernel_cleanup);

	psBufferedArgs->ui32SrcCnt = ARG_COUNT;

	psData->pvArgs = (void *) psBufferedArgs;
	psData->uiCbArgs = sizeof(*psBufferedArgs);
	psData->uiNumMemObjs = BUFFERED_OBJ_COUNT;

	psData->ppsMemList = calloc(BUFFERED_OBJ_COUNT, sizeof(*psData->ppvArgsMemLoc));
	psData->ppsMemList[0] = psBufferedArgs->psSrcBuffer;
	psData->ppsMemList[1] = psBufferedArgs->psDstBuffer;

	psData->ppvArgsMemLoc = calloc(BUFFERED_OBJ_COUNT,
								  sizeof(*psData->ppvArgsMemLoc));
	psData->ppvArgsMemLoc[0] = &psBufferedArgs->psSrcBuffer;
	psData->ppvArgsMemLoc[1] = &psBufferedArgs->psDstBuffer;

	psData->uiNumEventsInWaitList = 0;
	psData->ppsEventWaitList = NULL;
	psData->ppsEvent = NULL;

	psInstance->pvPrivateData = (void *) psData;

init_enqueue_native_kernel_cleanup:
	return iResult;
}

/*************************************************************************/ /*!
 @Function    Compute_EnqueueNativeKernel
 @Description This function enqueues the native kernel(s) and then calls
              clFinish to call the kernel(s).
 @Input       psInstance Contains the necessary arguments with which to queue
                         and flush the native kernel.
 @Return      eResult    CL_SUCCESS if the native kernel is correctly enqueued
                         and flushed, and a relevant error code otherwise.
*/ /**************************************************************************/
static cl_int Compute_EnqueueNativeKernel(OCLTestInstance *psInstance)
{
	PENQUEUE_NATIVE_KERNEL_DATA psData = (PENQUEUE_NATIVE_KERNEL_DATA) psInstance->pvPrivateData;
	cl_int iResult = CL_SUCCESS;

	iResult = clEnqueueNativeKernel(psData->psCommandQueue,
									psData->pfnUserFunc,
									psData->pvArgs,
									psData->uiCbArgs,
									psData->uiNumMemObjs,
									(const cl_mem *) psData->ppsMemList,
									(const void **) psData->ppvArgsMemLoc,
									psData->uiNumEventsInWaitList,
									psData->ppsEventWaitList,
									psData->ppsEvent);
	CheckAndReportError(psInstance,
						"clEnqueueNativeKernel",
						iResult,
						compute_enqueue_native_kernel_cleanup);

	clWaitForEvents(1, psData->ppsEvent);
	clFinish(psData->psCommandQueue);

compute_enqueue_native_kernel_cleanup:
	return iResult;
}

/*************************************************************************/ /*!
 @Function    Verify_EnqueueNativeKernel
 @Description This function checks that the NativeKernel function has correctly
              read from the source pointer and written it to the destination
              pointer.
 @Input       psInstance Contains the necessary data to read from the buffers
                         in order to verify they contain the same values.
 @Return      eResult    CL_SUCCESS if the source and destination values are
                         equal, and CL_INVALID_VALUE otherwise.
*/ /**************************************************************************/
static cl_int Verify_EnqueueNativeKernel(OCLTestInstance *psInstance)
{
	PENQUEUE_NATIVE_KERNEL_DATA psData = (PENQUEUE_NATIVE_KERNEL_DATA) psInstance->pvPrivateData;
	cl_int iResult;

	uint32_t aui32SrcValues[ARG_COUNT];
	uint32_t aui32DstValues[ARG_COUNT];

	iResult = clEnqueueReadBuffer(psData->psCommandQueue,
								  psData->ppsMemList[0],
								  CL_TRUE,
								  0,
								  ARG_COUNT * sizeof(aui32SrcValues[0]),
								  aui32SrcValues,
								  0,
								  NULL,
								  NULL);
	CheckAndReportError(psInstance,
						"clEnqueueNativeKernel",
						iResult,
						verify_enqueue_native_kernel_cleanup);

	iResult = clEnqueueReadBuffer(psData->psCommandQueue,
								  psData->ppsMemList[1],
								  CL_TRUE,
								  0,
								  ARG_COUNT * sizeof(aui32DstValues[0]),
								  aui32DstValues,
								  0,
								  NULL,
								  NULL);
	CheckAndReportError(psInstance,
						"clEnqueueNativeKernel",
						iResult,
						verify_enqueue_native_kernel_cleanup);

	for (uint32_t i = 0; i < ARG_COUNT; i++)
	{
		if (aui32SrcValues[i] != aui32DstValues[i])
		{
			OCLTestLog("%s: Failure: aui32SrcValues[%d] = %d, aui32DstValues[%d] = %d should be equal\n",
					   __func__,
					   i,
					   aui32SrcValues[i],
					   i,
					   aui32DstValues[i]);
			iResult = CL_INVALID_VALUE;
		}
		if (aui32SrcValues[i] != psData->pui32OriginalSrc[i])
		{
			OCLTestLog("%s: Failure: original source %d = %d, but after calling the kernel, source argument %d is %d. They should be equal.\n",
					   __func__,
					   i,
					   psData->pui32OriginalSrc[i],
					   i,
					   aui32SrcValues[i]);
			iResult = CL_INVALID_VALUE;
		}
	}

verify_enqueue_native_kernel_cleanup:

	for (cl_uint i = 0; i < psData->uiNumMemObjs; i++)
	{
		clReleaseMemObject(psData->ppsMemList[i]);
	}
	for (cl_uint i = 0; i < psData->uiNumEventsInWaitList; i++)
	{
		clReleaseEvent(psData->ppsEventWaitList[i]);
	}
	clReleaseCommandQueue(psData->psCommandQueue);
	clReleaseContext     (psData->psContext);
	clReleaseDevice      (psData->psDeviceID);

	free(psData->pvArgs);
	free(psData->ppsMemList);
	free(psData->ppvArgsMemLoc);

	return iResult;
}

/******************************************************************************
 End of file (enqueue_native_kernel.c)
******************************************************************************/
