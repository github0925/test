/*************************************************************************/ /*!
@File           memcpy.c
@Copyright      Copyright (c) Imagination Technologies Ltd. All Rights Reserved
@License        Strictly Confidential.
*/ /**************************************************************************/

#define TEST_MEMCPY_ITERATIONS_PER_INSTANCE (32)
#define TEST_MEMCPY_COPIES_PER_ITERATION   (1)
#define TEST_MEMCPY_NUM_TYPE_SIZES          7
#ifdef CUT_DOWN_UNIT_TEST
#define BUFFER_SIZE						  (1<<12)
#else
#define BUFFER_SIZE						  (1<<20)
#endif
#define BUFFER_SIZE_BYTES				  (BUFFER_SIZE * sizeof(unsigned int))


typedef struct _MemCopyKernelData_
{
	/* CL Objects */
	cl_context       psContext;
	cl_program       psProgram[TEST_MEMCPY_NUM_TYPE_SIZES];
	cl_device_id     psDeviceID;
	cl_platform_id   psPlatformID;
	cl_command_queue psCommandQueue;
	cl_kernel        psKernel[TEST_MEMCPY_NUM_TYPE_SIZES];
	cl_mem           psBufferA;
	cl_mem           psBufferB;
	cl_event         psEvent;

	/* Host Object */
	unsigned int *pui32Buffer;
	size_t        puGlobalWorkSize[3];
	size_t       puLocalWorkSize[3];

} MemCopyKernelData;

static const char* const memcpyTypeSizeNames[TEST_MEMCPY_NUM_TYPE_SIZES] = {"uchar", "ushort", "uint", "uint2", "uint4", "uint8", "uint16"};
cl_int Init_MemCopyKernel(OCLTestInstance *psInstance);
cl_int Compute_MemCopyKernel(OCLTestInstance *psInstance);
cl_int Verify_MemCopyKernel(OCLTestInstance *psInstance);

static const char *g_pszMemCopyKernelSource =
{
	"__kernel void MemoryCopy(__global %s* a, __global %s* b, uint n)\n"
	"{\n"
	"	unsigned int i;\n"
	"	unsigned int offset = get_global_id(0) * n;\n"
	"\n"
	"	for(i=0; i < n; i++)\n"
	"	{\n"
	"		a[i+offset] = b[i+offset];\n"
	"	}\n"
	"}"
};

/***********************************************************************************
 Function Name      : Init_MemCopyKernel
 Inputs             : None
 Outputs            : None
 Returns            : None
 Description        : Initialises input data
************************************************************************************/
cl_int Init_MemCopyKernel(OCLTestInstance *psInstance)
{
	size_t auLengths[2] = {0,0};

	cl_int eResult;
	int typeSizeLog2;
	int typeSize;
	char programSource[512];
	const char* programSourcePtrs[1] = {programSource};
	unsigned int i;

	MemCopyKernelData *psData = (MemCopyKernelData*)calloc(1, sizeof(MemCopyKernelData));

	if(!psData)
		return CL_OUT_OF_RESOURCES;

	/* Initialise data */
	psInstance->pvPrivateData = (void*)psData;

	/* Initialise host side objects */

	/* Allocate first host buffer */
	psData->pui32Buffer = (unsigned int*)malloc(BUFFER_SIZE_BYTES);

	if(!psData->pui32Buffer)
	{
		free(psData);
		return CL_OUT_OF_RESOURCES;
	}

	/* Initialise with test values */
	for(i=0; i < BUFFER_SIZE; i++)
	{
		psData->pui32Buffer[i] = i;
	}

	eResult = clGetPlatformIDs(1,&psData->psPlatformID,NULL);
	CheckAndReportError(psInstance, "clGetPlatformIDs", eResult, init_memcopykernel_cleanup);

	eResult = clGetDeviceIDs(psData->psPlatformID,CL_DEVICE_TYPE_GPU,1,&psData->psDeviceID,NULL);
	CheckAndReportError(psInstance, "clGetDeviceIDs", eResult, init_memcopykernel_cleanup);

	psData->psContext = clCreateContext(NULL,1,&psData->psDeviceID,NULL,NULL,&eResult);
	CheckAndReportError(psInstance, "clCreateContext", eResult, init_memcopykernel_cleanup);

	psData->psCommandQueue = clCreateCommandQueue(psData->psContext, psData->psDeviceID, CL_QUEUE_PROFILING_ENABLE, &eResult);
	CheckAndReportError(psInstance, "clCreateCommandQueue", eResult, init_memcopykernel_cleanup);

	psData->psBufferA = clCreateBuffer(psData->psContext, 0, BUFFER_SIZE_BYTES, NULL, &eResult);
	CheckAndReportError(psInstance, "clCreateBuffer", eResult, init_memcopykernel_cleanup);

	psData->psBufferB = clCreateBuffer(psData->psContext, CL_MEM_COPY_HOST_PTR, BUFFER_SIZE_BYTES, psData->pui32Buffer, &eResult);
	CheckAndReportError(psInstance, "clCreateBuffer", eResult, init_memcopykernel_cleanup);

	for (typeSizeLog2 = 0; typeSizeLog2 != TEST_MEMCPY_NUM_TYPE_SIZES; ++typeSizeLog2)
	{
		typeSize = 1 << typeSizeLog2;

		programSource[0] = 0;

		snprintf(programSource, 512, g_pszMemCopyKernelSource, memcpyTypeSizeNames[typeSizeLog2], memcpyTypeSizeNames[typeSizeLog2]);

		/* Use the global memwrite program */
		auLengths[0] = strlen(programSource);

		/*
		 * Build the Program
		 */
		psData->psProgram[typeSizeLog2] = clCreateProgramWithSource(psData->psContext, 1, programSourcePtrs, auLengths, &eResult);
		CheckAndReportError(psInstance, "clCreateProgramWithSource", eResult, init_memcopykernel_cleanup);

		eResult = clBuildProgram(psData->psProgram[typeSizeLog2],1,&psData->psDeviceID,"",NULL,NULL);
		CheckAndReportError(psInstance, "clBuildProgram", eResult, init_memcopykernel_cleanup);

		psData->psKernel[typeSizeLog2] = clCreateKernel(psData->psProgram[typeSizeLog2], "MemoryCopy", &eResult);
		CheckAndReportError(psInstance, "clCreateKernel", eResult, init_memcopykernel_cleanup);

		/*
		 * Setup the Arguments
		 */
		eResult = clSetKernelArg(psData->psKernel[typeSizeLog2], 0, sizeof(cl_mem), (void*) &psData->psBufferA );
		CheckAndReportError(psInstance, "clSetKernelArg", eResult, init_memcopykernel_cleanup);

		eResult = clSetKernelArg(psData->psKernel[typeSizeLog2], 1, sizeof(cl_mem), (void*) &psData->psBufferB );
		CheckAndReportError(psInstance, "clSetKernelArg", eResult, init_memcopykernel_cleanup);
	}

	/* Free the initial host buffer as no longer required, has been copied into device */
	free(psData->pui32Buffer);
	psData->pui32Buffer = NULL;

	return CL_SUCCESS;

init_memcopykernel_cleanup:
	return eResult;
}

/***********************************************************************************
 Function Name      : Verify_MemCopyKernel
 Inputs             : None
 Outputs            : None
 Returns            : None
 Description        : Verifies output data
************************************************************************************/
cl_int Verify_MemCopyKernel(OCLTestInstance *psInstance)
{
	double fTimeInSeconds;
	double fMBPerSeconds;

	unsigned int ui32MBCopied;
	unsigned int *pui32Results = NULL;
	cl_int eResult = CL_SUCCESS;
	cl_ulong lStart;
	cl_ulong lEnd;

	int typeSizeLog2;
	int typeSize;
	size_t global_size;
#if !defined(NO_HARDWARE)
	unsigned int i;
#endif /* defined(NO_HARDWARE) */

	MemCopyKernelData *psData =(MemCopyKernelData*) psInstance->pvPrivateData;
	psData = psData;

	/* Allocate temporary buffer for reading back results */
	pui32Results = (unsigned int*)malloc(BUFFER_SIZE_BYTES);

	if(!pui32Results)
	{
		free(psInstance->pvPrivateData);
		psInstance->pvPrivateData = strdup("Results buffer allocation failure");
		return CL_OUT_OF_RESOURCES;
	}


	for (typeSizeLog2 = 0; typeSizeLog2 != TEST_MEMCPY_NUM_TYPE_SIZES; typeSizeLog2++)
	{
		unsigned int ui32NumPerKernel;
		unsigned int ui32ExpectedOutput;

		typeSize = 1 << typeSizeLog2;


		/* Setup the size of the kernel run */
		psData->puGlobalWorkSize[0] = BUFFER_SIZE_BYTES / (typeSize * TEST_MEMCPY_ITERATIONS_PER_INSTANCE);
		psData->puLocalWorkSize[0] = 4;

		global_size = psData->puGlobalWorkSize[0];

		/* Set the number of write to perform each instance */
		ui32NumPerKernel = TEST_MEMCPY_ITERATIONS_PER_INSTANCE;

		if (typeSizeLog2 == 0) /* char */
			ui32ExpectedOutput = (ui32NumPerKernel << 24) | (ui32NumPerKernel << 16) | (ui32NumPerKernel << 8) | ui32NumPerKernel;
		else if (typeSizeLog2 == 1) /* short */
			ui32ExpectedOutput = (ui32NumPerKernel << 16) | ui32NumPerKernel;
		else /* intN */
			ui32ExpectedOutput = ui32NumPerKernel;

		eResult = clSetKernelArg(psData->psKernel[typeSizeLog2], 2, sizeof(cl_uint), (void*)&ui32NumPerKernel );
		CheckAndReportError(psInstance, "clSetKernelArg", eResult, verify_memcopykernel_cleanup);

		/* Enqueue a kernel to copy data from B --> A */
		eResult = clEnqueueNDRangeKernel(psData->psCommandQueue, psData->psKernel[typeSizeLog2], 1, NULL, &global_size, psData->puLocalWorkSize , 0, NULL, &psData->psEvent);
		CheckAndReportError(psInstance, "clEnqueueNDRangeKernel", eResult, verify_memcopykernel_cleanup);

		clFinish(psData->psCommandQueue);

		/* Print out timing information (command queue had profiling enabled) */
		eResult = clGetEventProfilingInfo(psData->psEvent,CL_PROFILING_COMMAND_START,sizeof(cl_ulong),&lStart,NULL);
		CheckAndReportError(psInstance, "clGetEventProfilingInfo", eResult, verify_memcopykernel_cleanup);
		eResult = clGetEventProfilingInfo(psData->psEvent,CL_PROFILING_COMMAND_END  ,sizeof(cl_ulong),&lEnd,NULL);
		CheckAndReportError(psInstance, "clGetEventProfilingInfo", eResult, verify_memcopykernel_cleanup);

		/* Convert bytes to megabytes, each load/store is 4 bytes (one uint32) */
		ui32MBCopied = psData->puGlobalWorkSize[0] * TEST_MEMCPY_ITERATIONS_PER_INSTANCE *
			TEST_MEMCPY_COPIES_PER_ITERATION * typeSize;
		ui32MBCopied /= (1 << 20);

		/* Calculate metrics */
		fTimeInSeconds = ((double)(lEnd-lStart)) / 1000000000.0;
		fMBPerSeconds  = (double)(ui32MBCopied)  / fTimeInSeconds;

		OCLTestLog("  BufferType: %8s, Instances: %8zu, Copies Per Instance: %4d, Copied: %4d MBs, Time: %10fs, %10fMB/s\n",
		   memcpyTypeSizeNames[typeSizeLog2],
		   psData->puGlobalWorkSize[0],
		   TEST_MEMCPY_ITERATIONS_PER_INSTANCE * TEST_MEMCPY_COPIES_PER_ITERATION,
		   ui32MBCopied,
		   fTimeInSeconds,
		   fMBPerSeconds);

		OCLMetricOutputDouble(__func__,memcpyTypeSizeNames[typeSizeLog2],fMBPerSeconds,MBSEC);

		clReleaseEvent(psData->psEvent);

		/* Read back the results */
		eResult = clEnqueueReadBuffer(psData->psCommandQueue, psData->psBufferA, CL_TRUE, 0, BUFFER_SIZE_BYTES, pui32Results, 0, NULL, NULL);
		CheckAndReportError(psInstance, "clEnqueueReadBuffer", eResult, verify_memcopykernel_cleanup);

#if !defined(NO_HARDWARE)
		/* Verify the written back data with what we would expect */
		for (i=0; i < BUFFER_SIZE; i++)
		{
			if (pui32Results[i] != i)
			{
				OCLTestLog("%s: Verification failure at %d, expected %08x got %08x.\n",
				           __func__, i, i,  pui32Results[i]);
				break;
			}
		}
#endif
	}

verify_memcopykernel_cleanup:
	/* Cleanup */
	for (typeSizeLog2 = 0; typeSizeLog2 != TEST_MEMCPY_NUM_TYPE_SIZES; typeSizeLog2++)
	{
		if(psData->psKernel[typeSizeLog2]) 	clReleaseKernel( psData->psKernel[typeSizeLog2] );
		if(psData->psProgram[typeSizeLog2]) clReleaseProgram( psData->psProgram[typeSizeLog2] );
	}

	if(psData->psBufferA) 		clReleaseMemObject   ( psData->psBufferA      );
	if(psData->psBufferB) 		clReleaseMemObject   ( psData->psBufferB      );
	if(psData->psCommandQueue)	clReleaseCommandQueue( psData->psCommandQueue );
	if(psData->psContext)		clReleaseContext     ( psData->psContext      );

	free(pui32Results);

	free(psInstance->pvPrivateData);
	psInstance->pvPrivateData = NULL;

	return eResult;
}

/******************************************************************************
 End of file (memcpy.c)
******************************************************************************/
