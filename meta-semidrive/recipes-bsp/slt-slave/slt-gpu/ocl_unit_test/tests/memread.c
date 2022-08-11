/*************************************************************************/ /*!
@File           memread.c
@Copyright      Copyright (c) Imagination Technologies Ltd. All Rights Reserved
@License        Strictly Confidential.
*/ /**************************************************************************/
#define TEST_MEMREAD_MAX_ITERATIONS      4
#define TEST_MEMREAD_MEMORY_IN_KILOBYTES 64
#define TEST_MEMREAD_MAX_INSTANCES       TEST_MEMREAD_MEMORY_IN_KILOBYTES * 1024

/* Should be multiple of 4, check how ui32ExpectedOutput is calculated. */
#define TEST_MEMREAD_ITERATIONS_PER_INSTANCE (32)
/* One read per a buffer a,b,c and d */
#define TEST_MEMREAD_READS_PER_ITERATION	 (4)
#define TEST_MEMREAD_NUM_TYPE_SIZES			 7
#ifdef CUT_DOWN_UNIT_TEST
#define BUFFER_SIZE							 (1<<12)
#else
#define BUFFER_SIZE							 (1<<20)
#endif
#define BUFFER_SIZE_BYTES					 (BUFFER_SIZE * sizeof(unsigned int))

typedef struct _MemReadKernelData_
{
	/* CL Objects */
	cl_context       psContext;
	cl_program       psProgram[TEST_MEMREAD_NUM_TYPE_SIZES];
	cl_device_id     psDeviceID;
	cl_platform_id   psPlatformID;
	cl_command_queue psCommandQueue;
	cl_kernel        psKernel[TEST_MEMREAD_NUM_TYPE_SIZES];
	cl_mem           psBufferA;
	cl_mem           psBufferB;
	cl_mem           psBufferC;
	cl_mem           psBufferD;
	cl_mem           psBufferOut;
	cl_event         psEvent;

	/* Host Object */
	unsigned int *pui32Buffer;
	size_t        puGlobalWorkSize[3];
	size_t       puLocalWorkSize[3];

} MemReadKernelData;

static char* memreadTypeSizeNames[TEST_MEMREAD_NUM_TYPE_SIZES] = {"uchar", "ushort", "uint", "uint2", "uint4", "uint8", "uint16"};
cl_int Init_MemReadKernel(OCLTestInstance *psInstance);
cl_int Compute_MemReadKernel(OCLTestInstance *psInstance);
cl_int Verify_MemReadKernel(OCLTestInstance *psInstance);

/*
 * Memory Read Kernel
 *
 * Test kernel that attempts to perform a large number of reads to determine
 * maximum read bandwidth.
 */
static char *g_pszMemReadKernelSource =
{
	"__kernel void MemoryRead_%s(__global %s* a, __global %s* b, __global %s* c, __global %s* d, __global %s* out, unsigned int n)\n"
	"{\n"
		"unsigned int i;\n"
		"unsigned int offset = get_global_id(0) * n;\n"
		"%s x = 0;\n"
		"for(i = 0; i < n; i++)\n"
		"{\n"
			"x += a[offset+i] + b[offset+i] + c[offset+i] + d[offset+i];\n"
		"}\n"
		"out[get_global_id(0)] = x;\n"
	"}\n"
};

/***********************************************************************************
 Function Name      : Init_MemReadKernel
 Inputs             : None
 Outputs            : None
 Returns            : None
 Description        : Initialises input data
************************************************************************************/
cl_int Init_MemReadKernel(OCLTestInstance *psInstance)
{
	size_t auLengths[2] = {0,0};

	cl_int eResult;

	int typeSizeLog2;
	int typeSize;
	char programSource[512];
	const char* programSourcePtrs[1] = {programSource};
	unsigned int i;

	MemReadKernelData *psData = (MemReadKernelData*)calloc(1, sizeof(MemReadKernelData));

	if(!psData)
	{
		return CL_OUT_OF_RESOURCES;
	}

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
		psData->pui32Buffer[i] = 0x00000001;
	}

	eResult = clGetPlatformIDs(1,&psData->psPlatformID,NULL);
	CheckAndReportError(psInstance, "clGetPlatformIDs", eResult, init_memreadkernel_cleanup);

	eResult = clGetDeviceIDs(psData->psPlatformID,CL_DEVICE_TYPE_GPU,1,&psData->psDeviceID,NULL);
	CheckAndReportError(psInstance, "clGetDeviceIDs", eResult, init_memreadkernel_cleanup);

	psData->psContext = clCreateContext(NULL,1,&psData->psDeviceID,NULL,NULL,&eResult);
	CheckAndReportError(psInstance, "clCreateContext", eResult, init_memreadkernel_cleanup);

	psData->psBufferA = clCreateBuffer(psData->psContext, CL_MEM_COPY_HOST_PTR, BUFFER_SIZE_BYTES, psData->pui32Buffer, &eResult);
	CheckAndReportError(psInstance, "clCreateBuffer", eResult, init_memreadkernel_cleanup);

	psData->psBufferB = clCreateBuffer(psData->psContext, CL_MEM_COPY_HOST_PTR, BUFFER_SIZE_BYTES, psData->pui32Buffer, &eResult);
	CheckAndReportError(psInstance, "clCreateBuffer", eResult, init_memreadkernel_cleanup);

	psData->psBufferC = clCreateBuffer(psData->psContext, CL_MEM_COPY_HOST_PTR, BUFFER_SIZE_BYTES, psData->pui32Buffer, &eResult);
	CheckAndReportError(psInstance, "clCreateBuffer", eResult, init_memreadkernel_cleanup);

	psData->psBufferD = clCreateBuffer(psData->psContext, CL_MEM_COPY_HOST_PTR, BUFFER_SIZE_BYTES, psData->pui32Buffer, &eResult);
	CheckAndReportError(psInstance, "clCreateBuffer", eResult, init_memreadkernel_cleanup);

	psData->psBufferOut = clCreateBuffer(psData->psContext, CL_MEM_WRITE_ONLY, BUFFER_SIZE_BYTES, NULL, &eResult);
	CheckAndReportError(psInstance, "clCreateBuffer", eResult, init_memreadkernel_cleanup);


	psData->psCommandQueue = clCreateCommandQueue(psData->psContext, psData->psDeviceID, CL_QUEUE_PROFILING_ENABLE, &eResult);
	CheckAndReportError(psInstance, "clCreateCommandQueue", eResult, init_memreadkernel_cleanup);

	/*
	 * Setup the Arguments
	 */

	for (typeSizeLog2 = 0; typeSizeLog2 != TEST_MEMREAD_NUM_TYPE_SIZES; ++typeSizeLog2)
	{
		char kernelName[100];
		typeSize = 1 << typeSizeLog2;

		programSource[0] = 0;

		snprintf(programSource, 512, g_pszMemReadKernelSource, memreadTypeSizeNames[typeSizeLog2], memreadTypeSizeNames[typeSizeLog2], memreadTypeSizeNames[typeSizeLog2], memreadTypeSizeNames[typeSizeLog2], memreadTypeSizeNames[typeSizeLog2], memreadTypeSizeNames[typeSizeLog2], memreadTypeSizeNames[typeSizeLog2]);

		/* Use the global memwrite program */
		auLengths[0] = strlen(programSource);

		/*
		 * Build the Program
		 */

		psData->psProgram[typeSizeLog2] = clCreateProgramWithSource(psData->psContext, 1, programSourcePtrs, auLengths, &eResult);
		CheckAndReportError(psInstance, "clCreateProgramWithSource", eResult, init_memreadkernel_cleanup);

		eResult = clBuildProgram(psData->psProgram[typeSizeLog2],1,&psData->psDeviceID,"",NULL,NULL);
		CheckAndReportError(psInstance, "clBuildProgram", eResult, init_memreadkernel_cleanup);

		strcpy(kernelName, "MemoryRead_");
		strcat(kernelName, memreadTypeSizeNames[typeSizeLog2]);
		psData->psKernel[typeSizeLog2] = clCreateKernel(psData->psProgram[typeSizeLog2], kernelName , &eResult);
		CheckAndReportError(psInstance, "clCreateKernel", eResult, init_memreadkernel_cleanup);

		eResult = clSetKernelArg(psData->psKernel[typeSizeLog2], 0, sizeof(cl_mem), (void*) &psData->psBufferA );
		CheckAndReportError(psInstance, "clSetKernelArg", eResult, init_memreadkernel_cleanup);

		eResult = clSetKernelArg(psData->psKernel[typeSizeLog2], 1, sizeof(cl_mem), (void*) &psData->psBufferB );
		CheckAndReportError(psInstance, "clSetKernelArg", eResult, init_memreadkernel_cleanup);

		eResult = clSetKernelArg(psData->psKernel[typeSizeLog2], 2, sizeof(cl_mem), (void*) &psData->psBufferC );
		CheckAndReportError(psInstance, "clSetKernelArg", eResult, init_memreadkernel_cleanup);

		eResult = clSetKernelArg(psData->psKernel[typeSizeLog2], 3, sizeof(cl_mem), (void*) &psData->psBufferD );
		CheckAndReportError(psInstance, "clSetKernelArg", eResult, init_memreadkernel_cleanup);

		eResult = clSetKernelArg(psData->psKernel[typeSizeLog2], 4, sizeof(cl_mem), (void*) &psData->psBufferOut );
		CheckAndReportError(psInstance, "clSetKernelArg", eResult, init_memreadkernel_cleanup);
	}

	/* Free the initial host buffer as no longer required, has been copied into device */
	free(psData->pui32Buffer);
	psData->pui32Buffer = NULL;

	return CL_SUCCESS;

init_memreadkernel_cleanup:
	return eResult;
}

/***********************************************************************************
 Function Name      : Verify_MemReadKernel
 Inputs             : None
 Outputs            : None
 Returns            : None
 Description        : Verifies output data
************************************************************************************/
cl_int Verify_MemReadKernel(OCLTestInstance *psInstance)
{
	double fTimeInSeconds;
	double fMBPerSeconds;
	unsigned int ui32MBCopied;
#if !defined(NO_HARDWARE)
	unsigned int i;
#endif /* !defined(NO_HARDWARE) */
	unsigned int *pui32Results = NULL;
	cl_int eResult = CL_SUCCESS;
	cl_ulong lStart;
	cl_ulong lEnd;

	int typeSizeLog2;
	int typeSize;

	MemReadKernelData *psData =(MemReadKernelData*)psInstance->pvPrivateData;
	psData = psData;

	/* Perform ever greater buffer copies */
	for(typeSizeLog2 = 0; typeSizeLog2 != TEST_MEMREAD_NUM_TYPE_SIZES; typeSizeLog2++)
	{
		unsigned int ui32NumPerKernel;
		unsigned int ui32ExpectedOutput;
		size_t global_size;

		typeSize = 1 << typeSizeLog2;

		psData->puGlobalWorkSize[0] = BUFFER_SIZE_BYTES / (typeSize * TEST_MEMREAD_ITERATIONS_PER_INSTANCE);
		psData->puLocalWorkSize[0] = 4;

		global_size = psData->puGlobalWorkSize[0];

		/* Set the number of reads to perform per kernel (always a power of two) */
		ui32NumPerKernel = TEST_MEMREAD_ITERATIONS_PER_INSTANCE;

		OCLTestLog("Performing %d memory reads per kernel instance, totalling %zu memory loads...\n",ui32NumPerKernel,ui32NumPerKernel*psData->puGlobalWorkSize[0]);

		eResult = clSetKernelArg(psData->psKernel[typeSizeLog2], 5, sizeof(cl_uint), (void*)&ui32NumPerKernel );
		CheckAndReportError(psInstance, "clSetKernelArg", eResult, verify_memreadkernel_cleanup);

		/* Enqueue a kernel to copy data from B --> A */
		eResult = clEnqueueNDRangeKernel(psData->psCommandQueue, psData->psKernel[typeSizeLog2], 1, NULL, &global_size, psData->puLocalWorkSize , 0, NULL, &psData->psEvent);
		CheckAndReportError(psInstance, "clEnqueueNDRangeKernel", eResult, verify_memreadkernel_cleanup);

		clFinish(psData->psCommandQueue);

		/* Print out timing information (command queue had profiling enabled) */
		eResult = clGetEventProfilingInfo(psData->psEvent,CL_PROFILING_COMMAND_START,sizeof(cl_ulong),&lStart,NULL);
		CheckAndReportError(psInstance, "clGetEventProfilingInfo", eResult, verify_memreadkernel_cleanup);
		eResult = clGetEventProfilingInfo(psData->psEvent,CL_PROFILING_COMMAND_END  ,sizeof(cl_ulong),&lEnd,NULL);
		CheckAndReportError(psInstance, "clGetEventProfilingInfo", eResult, verify_memreadkernel_cleanup);

		/* Calculate number of bytes read by the kernel, and convert to megabytes  */
		ui32MBCopied = psData->puGlobalWorkSize[0] * TEST_MEMREAD_ITERATIONS_PER_INSTANCE *
			TEST_MEMREAD_READS_PER_ITERATION * typeSize;
		ui32MBCopied /= (1 << 20);

		/* Calculate metrics */
		fTimeInSeconds = ((double)(lEnd-lStart)) / 1000000000.0;
		fMBPerSeconds  = (double)(ui32MBCopied)  / fTimeInSeconds;

		OCLTestLog("  BufferType: %8s, Instances: %8zu, Reads per instance: %4d, Read: %4d MBs, Time: %10fs, %10fMB/s\n",
				   memreadTypeSizeNames[typeSizeLog2],
		           psData->puGlobalWorkSize[0],
		           TEST_MEMREAD_ITERATIONS_PER_INSTANCE * TEST_MEMREAD_READS_PER_ITERATION,
		           ui32MBCopied,
		           fTimeInSeconds,
		           fMBPerSeconds);

		OCLMetricOutputDouble(__func__,memreadTypeSizeNames[typeSizeLog2],fMBPerSeconds,MBSEC);

		clReleaseEvent(psData->psEvent);

		if(!pui32Results)
		{
			/* Allocate temporary buffer for reading back results */
			pui32Results = (unsigned int*)malloc(BUFFER_SIZE_BYTES);

			if(!pui32Results)
			{
				free(psInstance->pvPrivateData);
				psInstance->pvPrivateData = strdup("Results buffer allocation failure");
				return CL_OUT_OF_RESOURCES;
			}
		}

		/* Read back the results */
		eResult = clEnqueueReadBuffer(psData->psCommandQueue, psData->psBufferOut, CL_TRUE, 0, global_size * typeSize, pui32Results, 0, NULL, NULL);
		CheckAndReportError(psInstance, "clEnqueueReadBuffer", eResult, verify_memreadkernel_cleanup);

		/* Host buffer is initialised with int, char sum will be increased 1 per 4 offsets, so divide by 4 */
		ui32ExpectedOutput = (TEST_MEMREAD_ITERATIONS_PER_INSTANCE * TEST_MEMREAD_READS_PER_ITERATION) / 4;

		if (typeSizeLog2 == 0) // char case
		{
		ui32ExpectedOutput |= (ui32ExpectedOutput << 24) | (ui32ExpectedOutput << 16) | (ui32ExpectedOutput << 8);
		}
		else if (typeSizeLog2 == 1) // short case
		{
			ui32ExpectedOutput *= 2;
			ui32ExpectedOutput |= ui32ExpectedOutput << 16;
		}
		else // intN cases
		{
			ui32ExpectedOutput *= 4;
		}

		/* Verify the data properly */
#if !defined(NO_HARDWARE)
		for(i=0; i < global_size * typeSize / 4; i++)
		{
			if(pui32Results[i] != ui32ExpectedOutput)
			{
				OCLTestLog("Verification Error at %d, got %08x expected %08x!\n",i,pui32Results[i],ui32ExpectedOutput);
				free(psInstance->pvPrivateData);
				psInstance->pvPrivateData = strdup("Verification failure");
				return CL_INVALID_VALUE;
			}
		}
#endif /* !defined(NO_HARDWARE) */
	}

	/* Cleanup */
	for (typeSizeLog2 = 0; typeSizeLog2 != TEST_MEMREAD_NUM_TYPE_SIZES; typeSizeLog2++)
	{
		clReleaseKernel( psData->psKernel[typeSizeLog2] );
		clReleaseProgram( psData->psProgram[typeSizeLog2] );
	}

	clReleaseMemObject   ( psData->psBufferA      );
	clReleaseMemObject   ( psData->psBufferB      );
	clReleaseMemObject   ( psData->psBufferC      );
	clReleaseMemObject   ( psData->psBufferD      );
	clReleaseMemObject   ( psData->psBufferOut      );
	clReleaseCommandQueue( psData->psCommandQueue );
	clReleaseContext     ( psData->psContext      );

	free(pui32Results);

verify_memreadkernel_cleanup:
	if(psInstance->pvPrivateData)
	{
		free(psInstance->pvPrivateData);
		psInstance->pvPrivateData = NULL;
	}

	return eResult;
}

/******************************************************************************
 End of file (memread.c)
******************************************************************************/
