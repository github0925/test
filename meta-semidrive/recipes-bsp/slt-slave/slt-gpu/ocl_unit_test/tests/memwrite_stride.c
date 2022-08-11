/*************************************************************************/ /*!
@File           memwrite_stride.c
@Copyright      Copyright (c) Imagination Technologies Ltd. All Rights Reserved
@License        Strictly Confidential.
*/ /**************************************************************************/

#define TEST_MEMSTRIDEWRITE_ITERATIONS_PER_INSTANCE (32)
#define TEST_MEMSTRIDEWRITE_WRITES_PER_ITERATION   (4)
#define TEST_MEMSTRIDEWRITE_NUM_TYPE_SIZES          7
#ifdef CUT_DOWN_UNIT_TEST
#define BUFFER_SIZE						  (1<<12)
#else
#define BUFFER_SIZE						  (1<<20)
#endif
#define BUFFER_SIZE_BYTES				  (BUFFER_SIZE * sizeof(unsigned int))

typedef struct _MemStrideWriteKernelData_
{
	/* CL Objects */
	cl_context       psContext;
	cl_program       psProgram[TEST_MEMSTRIDEWRITE_NUM_TYPE_SIZES];
	cl_device_id     psDeviceID;
	cl_platform_id   psPlatformID;
	cl_command_queue psCommandQueue;
	cl_kernel        psKernel[TEST_MEMSTRIDEWRITE_NUM_TYPE_SIZES];
	cl_mem           psBufferA;
	cl_mem           psBufferB;
	cl_mem           psBufferC;
	cl_mem           psBufferD;
	cl_event         psEvent ;

	/* Host Object */
	unsigned int *pui32Buffer;
	size_t        puGlobalWorkSize[3];
	size_t        puLocalWorkSize[3];

} MemStrideWriteKernelData;

static char* memstridewriteTypeSizeNames[TEST_MEMSTRIDEWRITE_NUM_TYPE_SIZES] = {"uchar", "ushort", "uint", "uint2", "uint4", "uint8", "uint16"};
cl_int Init_MemStrideWriteKernel(OCLTestInstance *psInstance);
cl_int Compute_MemStrideWriteKernel(OCLTestInstance *psInstance);
cl_int Verify_MemStrideWriteKernel(OCLTestInstance *psInstance);

/*
 * Memory Stride Write Kernel
 *
 * Test kernel that attempts to perform a large number of write to determine maximum
 * write bandwidth. Each instance of kernel writes n locations in memory using stride.
 */
static char *g_pszMemStrideWriteKernelSourceSmall =
{
	"__kernel void MemoryStrideWrite(__global %s* a, __global %s* b, __global %s* c, __global %s *d, uint n)\n"
	"{\n"
	"	unsigned int i,j;\n"
	"	unsigned short seq = %d;\n"
	"	unsigned int lid = get_local_id(0)*seq;\n"
	"	unsigned int lsize = get_local_size(0);\n"
	"	unsigned int gid = get_group_id(0);\n"
	"	unsigned int offset = gid * lsize * n;\n"
	"\n"
	"	for(i=0; i < n; i+=seq)\n"
	"	{\n"
		"for(j=0; j < seq; j++)\n"
		"{\n"
	"		a[i*lsize+offset+lid+j] = n;\n"
	"		b[i*lsize+offset+lid+j] = n;\n"
	"		c[i*lsize+offset+lid+j] = n;\n"
	"		d[i*lsize+offset+lid+j] = n;\n"
		"}\n"
	"	}\n"
	"}\n"
};
static char *g_pszMemStrideWriteKernelSourceLarge =
{
	"__kernel void MemoryStrideWrite(__global %s* a, __global %s* b, __global %s* c, __global %s *d, uint n)\n"
	"{\n"
		"	unsigned int i;\n"
		"	unsigned int lid = get_local_id(0);\n"
		"	unsigned int lsize = get_local_size(0);\n"
		"	unsigned int gid = get_group_id(0);\n"
		"	unsigned int offset = gid * lsize * n;\n"
		"\n"
		"	for(i=0; i < n; i++)\n"
		"	{\n"
			"		a[i*lsize+offset+lid] = n;\n"
			"		b[i*lsize+offset+lid] = n;\n"
			"		c[i*lsize+offset+lid] = n;\n"
			"		d[i*lsize+offset+lid] = n;\n"
			"	}\n"
			"}\n"
};

/***********************************************************************************
 Function Name      : Init_MemStrideWriteKernel
 Inputs             : None
 Outputs            : None
 Returns            : None
 Description        : Initialises input data
************************************************************************************/
cl_int Init_MemStrideWriteKernel(OCLTestInstance *psInstance)
{
	size_t auLengths[2] = {0,0};

	cl_int eResult;
	int typeSizeLog2;
	int typeSize;
	char programSource[1024];
	const char* programSourcePtrs[1] = {programSource};

	MemStrideWriteKernelData *psData = (MemStrideWriteKernelData*)calloc(1, sizeof(MemStrideWriteKernelData));

	if(!psData)
	{
		return CL_OUT_OF_RESOURCES;
	}

	/* Initialise data */
	psInstance->pvPrivateData = (void*)psData;

	eResult = clGetPlatformIDs(1,&psData->psPlatformID,NULL);
	CheckAndReportError(psInstance, "clGetPlatformIDs", eResult, init_memstridewritekernel_cleanup);

	eResult = clGetDeviceIDs(psData->psPlatformID,CL_DEVICE_TYPE_GPU,1,&psData->psDeviceID,NULL);
	CheckAndReportError(psInstance, "clGetDeviceIDs", eResult, init_memstridewritekernel_cleanup);

	psData->psContext = clCreateContext(NULL,1,&psData->psDeviceID,NULL,NULL,&eResult);
	CheckAndReportError(psInstance, "clCreateContext", eResult, init_memstridewritekernel_cleanup);

	psData->psCommandQueue = clCreateCommandQueue(psData->psContext, psData->psDeviceID, CL_QUEUE_PROFILING_ENABLE, &eResult);
	CheckAndReportError(psInstance, "clCreateCommandQueue", eResult, init_memstridewritekernel_cleanup);

	psData->psBufferA = clCreateBuffer(psData->psContext, CL_MEM_WRITE_ONLY, BUFFER_SIZE_BYTES, NULL, &eResult);
	CheckAndReportError(psInstance, "clCreateBuffer", eResult, init_memstridewritekernel_cleanup);

	psData->psBufferB = clCreateBuffer(psData->psContext, CL_MEM_WRITE_ONLY, BUFFER_SIZE_BYTES, NULL, &eResult);
	CheckAndReportError(psInstance, "clCreateBuffer", eResult, init_memstridewritekernel_cleanup);

	psData->psBufferC = clCreateBuffer(psData->psContext, CL_MEM_WRITE_ONLY, BUFFER_SIZE_BYTES, NULL, &eResult);
	CheckAndReportError(psInstance, "clCreateBuffer", eResult, init_memstridewritekernel_cleanup);

	psData->psBufferD = clCreateBuffer(psData->psContext, CL_MEM_WRITE_ONLY, BUFFER_SIZE_BYTES, NULL, &eResult);
	CheckAndReportError(psInstance, "clCreateBuffer", eResult, init_memstridewritekernel_cleanup);


	for (typeSizeLog2 = 0; typeSizeLog2 != TEST_MEMSTRIDEWRITE_NUM_TYPE_SIZES; ++typeSizeLog2)
	{
		typeSize = 1 << typeSizeLog2;

		programSource[0] = 0;

		/* Based on size of the type, pick correct kernel. Stride patterns for uchar and ushort benefit from some sequential reading. */
		if (typeSizeLog2 < 3 )
		{
			snprintf(programSource, 1024, g_pszMemStrideWriteKernelSourceSmall, memstridewriteTypeSizeNames[typeSizeLog2],
					memstridewriteTypeSizeNames[typeSizeLog2], memstridewriteTypeSizeNames[typeSizeLog2],
					memstridewriteTypeSizeNames[typeSizeLog2], 16 >> typeSizeLog2);
		}
		else
		{
			snprintf(programSource, 1024, g_pszMemStrideWriteKernelSourceLarge, memstridewriteTypeSizeNames[typeSizeLog2],
					memstridewriteTypeSizeNames[typeSizeLog2], memstridewriteTypeSizeNames[typeSizeLog2],
					memstridewriteTypeSizeNames[typeSizeLog2]);
		}

		/* Use the global memwrite program */
		auLengths[0] = strlen(programSource);

		/*
		 * Build the Program
		 */
		psData->psProgram[typeSizeLog2] = clCreateProgramWithSource(psData->psContext, 1, programSourcePtrs, auLengths, &eResult);
		CheckAndReportError(psInstance, "clCreateProgramWithSource", eResult, init_memstridewritekernel_cleanup);

		eResult = clBuildProgram(psData->psProgram[typeSizeLog2],1,&psData->psDeviceID,"",NULL,NULL);
		CheckAndReportError(psInstance, "clBuildProgram", eResult, init_memstridewritekernel_cleanup);

		psData->psKernel[typeSizeLog2] = clCreateKernel(psData->psProgram[typeSizeLog2], "MemoryStrideWrite", &eResult);
		CheckAndReportError(psInstance, "clCreateKernel", eResult, init_memstridewritekernel_cleanup);

		/*
		 * Setup the Arguments
		 */
		eResult = clSetKernelArg(psData->psKernel[typeSizeLog2], 0, sizeof(cl_mem), (void*) &psData->psBufferA );
		CheckAndReportError(psInstance, "clSetKernelArg", eResult, init_memstridewritekernel_cleanup);

		eResult = clSetKernelArg(psData->psKernel[typeSizeLog2], 1, sizeof(cl_mem), (void*) &psData->psBufferB );
		CheckAndReportError(psInstance, "clSetKernelArg", eResult, init_memstridewritekernel_cleanup);

		eResult = clSetKernelArg(psData->psKernel[typeSizeLog2], 2, sizeof(cl_mem), (void*) &psData->psBufferC );
		CheckAndReportError(psInstance, "clSetKernelArg", eResult, init_memstridewritekernel_cleanup);

		eResult = clSetKernelArg(psData->psKernel[typeSizeLog2], 3, sizeof(cl_mem), (void*) &psData->psBufferD );
		CheckAndReportError(psInstance, "clSetKernelArg", eResult, init_memstridewritekernel_cleanup);
	}

	return CL_SUCCESS;

init_memstridewritekernel_cleanup:
	return eResult;
}

/***********************************************************************************
 Function Name      : Verify_MemStrideWriteKernel
 Inputs             : None
 Outputs            : None
 Returns            : None
 Description        : Verifies output data
************************************************************************************/
cl_int Verify_MemStrideWriteKernel(OCLTestInstance *psInstance)
{
	double fTimeInSeconds;
	double fMBPerSeconds;

	unsigned int ui32MBWritten;

#if !defined(NO_HARDWARE)
	unsigned int i;
#endif /* !defined(NO_HARDWARE) */

	unsigned int *pui32Results = NULL;
	cl_int eResult = CL_SUCCESS;
	cl_ulong lStart;
	cl_ulong lEnd;

	int typeSizeLog2;
	int typeSize;
	size_t global_size;

	MemStrideWriteKernelData *psData =(MemStrideWriteKernelData*) psInstance->pvPrivateData;
	psData = psData;

	for (typeSizeLog2 = 0; typeSizeLog2 != TEST_MEMSTRIDEWRITE_NUM_TYPE_SIZES; typeSizeLog2++)
	{
		unsigned int ui32NumPerKernel;
		unsigned int ui32ExpectedOutput;

		typeSize = 1 << typeSizeLog2;

		/* Setup the size of the kernel run */
		psData->puGlobalWorkSize[0] = BUFFER_SIZE_BYTES / (typeSize * TEST_MEMSTRIDEWRITE_ITERATIONS_PER_INSTANCE);
		/* We have 4 cache lines, 64 bytes wide, so set up the workgroup size to maximize throughput */
		psData->puLocalWorkSize[0]  = 4*64/typeSize;//32;

		global_size = psData->puGlobalWorkSize[0];

		/* Set the number of write to perform each instance */
		ui32NumPerKernel = TEST_MEMSTRIDEWRITE_ITERATIONS_PER_INSTANCE;

		if (typeSizeLog2 == 0) /* char */
			ui32ExpectedOutput = (ui32NumPerKernel << 24) | (ui32NumPerKernel << 16) | (ui32NumPerKernel << 8) | ui32NumPerKernel;
		else if (typeSizeLog2 == 1) /* short */
			ui32ExpectedOutput = (ui32NumPerKernel << 16) | ui32NumPerKernel;
		else /* intN */
			ui32ExpectedOutput = ui32NumPerKernel;

		eResult = clSetKernelArg(psData->psKernel[typeSizeLog2], 4, sizeof(cl_uint), (void*)&ui32NumPerKernel );
		CheckAndReportError(psInstance, "clSetKernelArg", eResult, verify_memstridewritekernel_cleanup);

		/* Enqueue Kernel */
		eResult = clEnqueueNDRangeKernel(psData->psCommandQueue, psData->psKernel[typeSizeLog2], 1, NULL, &global_size, &psData->puLocalWorkSize[0], 0, NULL, &psData->psEvent);
		CheckAndReportError(psInstance, "clEnqueueNDRangeKernel", eResult, verify_memstridewritekernel_cleanup);

		clFinish(psData->psCommandQueue);

		/* Print out timing information (command queue had profiling enabled) */
		eResult = clGetEventProfilingInfo(psData->psEvent,CL_PROFILING_COMMAND_START,sizeof(cl_ulong),&lStart,NULL);
		CheckAndReportError(psInstance, "clGetEventProfilingInfo", eResult, verify_memstridewritekernel_cleanup);
		eResult = clGetEventProfilingInfo(psData->psEvent,CL_PROFILING_COMMAND_END  ,sizeof(cl_ulong),&lEnd,NULL);
		CheckAndReportError(psInstance, "clGetEventProfilingInfo", eResult, verify_memstridewritekernel_cleanup);

		/* Calculate number of MB's written out in total */
		ui32MBWritten = psData->puGlobalWorkSize[0] * TEST_MEMSTRIDEWRITE_ITERATIONS_PER_INSTANCE *
			TEST_MEMSTRIDEWRITE_WRITES_PER_ITERATION * typeSize;
		ui32MBWritten /= (1 << 20);

		/* Calculate metrics */
		fTimeInSeconds = ((double)(lEnd-lStart)) / 1000000000.0;
		fMBPerSeconds  = (double)(ui32MBWritten)  / fTimeInSeconds;

		OCLTestLog("  BufferType: %8s, Instances: %8zu, Writes Per Instance: %4d, Written: %4d MBs, Time: %10fs, %10fMB/s\n",
				   memstridewriteTypeSizeNames[typeSizeLog2],
		           psData->puGlobalWorkSize[0],
		           TEST_MEMSTRIDEWRITE_ITERATIONS_PER_INSTANCE * TEST_MEMSTRIDEWRITE_WRITES_PER_ITERATION,
		           ui32MBWritten,
		           fTimeInSeconds,
		           fMBPerSeconds);

		OCLMetricOutputDouble(__func__,memstridewriteTypeSizeNames[typeSizeLog2],fMBPerSeconds,MBSEC);

		clReleaseEvent(psData->psEvent);

		/* Allocate temporary buffer for reading back results */
		pui32Results = (unsigned int*)malloc(BUFFER_SIZE_BYTES);

		if(!pui32Results)
		{
			free(psInstance->pvPrivateData);
			psInstance->pvPrivateData = strdup("Results buffer allocation failure");
			return CL_OUT_OF_RESOURCES;
		}

		/* Read back the results */
		eResult = clEnqueueReadBuffer(psData->psCommandQueue, psData->psBufferA, CL_TRUE, 0, BUFFER_SIZE_BYTES, pui32Results, 0, NULL, NULL);
		CheckAndReportError(psInstance, "clEnqueueReadBuffer", eResult, verify_memstridewritekernel_cleanup);
#if !defined(NO_HARDWARE)
		/* Verify the data properly */
		for(i=0; i < BUFFER_SIZE; i++)
		{
			if(pui32Results[i] != ui32ExpectedOutput)
			{
				OCLTestLog("Verification Error at %d, got %08x expected %08x!\n",i,pui32Results[i],ui32NumPerKernel);
				free(psInstance->pvPrivateData);
				psInstance->pvPrivateData = strdup("Verification failure");
				return CL_INVALID_VALUE;
			}
		}
#endif
		free(pui32Results);
	}

	/* Cleanup */
	for (typeSizeLog2 = 0; typeSizeLog2 != TEST_MEMSTRIDEWRITE_NUM_TYPE_SIZES; typeSizeLog2++)
	{
		clReleaseKernel( psData->psKernel[typeSizeLog2] );
		clReleaseProgram( psData->psProgram[typeSizeLog2] );
	}

	clReleaseMemObject   ( psData->psBufferA      );
	clReleaseMemObject   ( psData->psBufferB      );
	clReleaseMemObject   ( psData->psBufferC      );
	clReleaseMemObject   ( psData->psBufferD      );
	clReleaseCommandQueue( psData->psCommandQueue );
	clReleaseContext     ( psData->psContext      );

	free(psInstance->pvPrivateData);
	psInstance->pvPrivateData = NULL;

	return eResult;

verify_memstridewritekernel_cleanup:
	return eResult;
}

/******************************************************************************
 End of file (memwrite_stride.c)
******************************************************************************/
