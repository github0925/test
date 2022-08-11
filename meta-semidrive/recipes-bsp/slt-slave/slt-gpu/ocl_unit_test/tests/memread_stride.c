/*************************************************************************/ /*!
@File           memread_stride.c
@Copyright      Copyright (c) Imagination Technologies Ltd. All Rights Reserved
@License        Strictly Confidential.
*/ /**************************************************************************/
#define TEST_MEMSTRIDEREAD_MAX_ITERATIONS      4
#define TEST_MEMSTRIDEREAD_MEMORY_IN_KILOBYTES 64
#define TEST_MEMSTRIDEREAD_MAX_INSTANCES       TEST_MEMSTRIDEREAD_MEMORY_IN_KILOBYTES * 64


#define TEST_MEMSTRIDEREAD_ITERATIONS_PER_INSTANCE (32)
#define TEST_MEMSTRIDEREAD_READS_PER_ITERATION	 (4)
#define TEST_MEMSTRIDEREAD_NUM_TYPE_SIZES			 7
#ifdef CUT_DOWN_UNIT_TEST
#define BUFFER_SIZE							 (1<<12)
#else
#define BUFFER_SIZE							 (1<<20)
#endif
#define BUFFER_SIZE_BYTES					 (BUFFER_SIZE * sizeof(unsigned int))

#include <stdio.h>
typedef struct _MemStrideReadKernelData_
{
	/* CL Objects */
	cl_context       psContext;
	cl_program       psProgram[TEST_MEMSTRIDEREAD_NUM_TYPE_SIZES];
	cl_device_id     psDeviceID;
	cl_platform_id   psPlatformID;
	cl_command_queue psCommandQueue;
	cl_kernel        psKernel[TEST_MEMSTRIDEREAD_NUM_TYPE_SIZES];
	cl_mem           psBufferA;
	cl_mem           psBufferB;
	cl_mem           psBufferC;
	cl_mem           psBufferD;
	cl_mem           psBufferOut;
	cl_event         psEvent;

	/* Host Object */
	unsigned int *pui32Buffer;
	size_t        puGlobalWorkSize[3];
	size_t        puLocalWorkSize[3];

} MemStrideReadKernelData;

static char* memstridereadTypeSizeNames[TEST_MEMSTRIDEREAD_NUM_TYPE_SIZES] = { "uchar", "ushort", "uint", "uint2", "uint4", "uint8", "uint16" };
cl_int Init_MemStrideReadKernel(OCLTestInstance *psInstance);
cl_int Compute_MemStrideReadKernel(OCLTestInstance *psInstance);
cl_int Verify_MemStrideReadKernel(OCLTestInstance *psInstance);

/*
 * Memory Stride Read Kernel
 *
 * Test kernels that attempts to perform a large number of reads to determine
 * maximum read bandwidth using stride pattern.
 */
static char *g_pszMemStrideReadKernelSourceSmall =
{
	"__kernel void MemoryStrideRead(__global %s* a, __global %s* b, __global %s* c, __global %s* d, __global %s* out, unsigned int n)\n"
	"{\n"
		"unsigned int i,j;\n"
		"%s x = 0;\n"
		"unsigned short seq = %d;\n"
		"unsigned int lid = get_local_id(0)*seq;\n"
		"unsigned int lsize = get_local_size(0);\n"
		"unsigned int gid = get_group_id(0);\n"
		"unsigned int offset = gid * lsize * n;\n"
		"for(i = 0; i < n; i+=seq)\n"
		"{\n"
			"for(j=0; j < seq; j++)\n"
			"x += a[i*lsize+offset+lid+j] + b[i*lsize+offset+lid+j] + c[i*lsize+offset+lid+j] + d[i*lsize+offset+lid+j];\n"
		"}\n"
		"out[get_global_id(0)] = x;\n"
	"}\n"
};
static char *g_pszMemStrideReadKernelSourceLarge =
{
	"__kernel void MemoryStrideRead(__global %s* a, __global %s* b, __global %s* c, __global %s* d, __global %s* out, unsigned int n)\n"
	"{\n"
		"unsigned int i;\n"
		"unsigned int lid = get_local_id(0);\n"
		"unsigned int lsize = get_local_size(0);\n"
		"unsigned int gid = get_group_id(0);\n"
		"unsigned int offset = gid * lsize * n;\n"
		"%s x = 0;\n"
		"for(i = 0; i < n; i++)\n"
		"{\n"
			"x += a[i*lsize+offset+lid] + b[i*lsize+offset+lid] + c[i*lsize+offset+lid] + d[i*lsize+offset+lid];\n"
		"}\n"
		"out[get_global_id(0)] = x;\n"
	"}\n"
};

/***********************************************************************************
 Function Name      : Init_MemStrideReadKernel
 Inputs             : None
 Outputs            : None
 Returns            : None
 Description        : Initialises input data
************************************************************************************/
cl_int Init_MemStrideReadKernel(OCLTestInstance *psInstance)
{
	size_t auLengths[2] = {0,0};

	cl_int eResult;

	int typeSizeLog2;
	int typeSize;
	char programSource[1024];
	const char* programSourcePtrs[1] = {programSource};

	MemStrideReadKernelData *psData = (MemStrideReadKernelData*)calloc(1, sizeof(MemStrideReadKernelData));

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
	{
		unsigned int i;

		for(i=0; i < BUFFER_SIZE; i++)
		{
			psData->pui32Buffer[i] = 0x00000001;
		}
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
	for (typeSizeLog2 = 0; typeSizeLog2 != TEST_MEMSTRIDEREAD_NUM_TYPE_SIZES; ++typeSizeLog2)
	{
		typeSize = 1 << typeSizeLog2;

		programSource[0] = 0;
		/* Based on size of the type, pick correct kernel. Stride patterns for uchar, ushort and unint benefit from some sequential reading. */
		if (typeSizeLog2 < 3 )
		{
			snprintf(programSource, 1024, g_pszMemStrideReadKernelSourceSmall, memstridereadTypeSizeNames[typeSizeLog2],
					memstridereadTypeSizeNames[typeSizeLog2], memstridereadTypeSizeNames[typeSizeLog2],
					memstridereadTypeSizeNames[typeSizeLog2], memstridereadTypeSizeNames[typeSizeLog2],
					memstridereadTypeSizeNames[typeSizeLog2], 16 >> typeSizeLog2);
		}
		else
		{
			snprintf(programSource, 1024, g_pszMemStrideReadKernelSourceLarge, memstridereadTypeSizeNames[typeSizeLog2],
					memstridereadTypeSizeNames[typeSizeLog2], memstridereadTypeSizeNames[typeSizeLog2],
					memstridereadTypeSizeNames[typeSizeLog2], memstridereadTypeSizeNames[typeSizeLog2],
					memstridereadTypeSizeNames[typeSizeLog2]);
		}

		/* Use the global memwrite program */
		auLengths[0] = strlen(programSource);

		/*
		 * Build the Program
		 */
		psData->psProgram[typeSizeLog2] = clCreateProgramWithSource(psData->psContext, 1, programSourcePtrs, auLengths, &eResult);
		CheckAndReportError(psInstance, "clCreateProgramWithSource", eResult, init_memreadkernel_cleanup);

		eResult = clBuildProgram(psData->psProgram[typeSizeLog2],1,&psData->psDeviceID,"",NULL,NULL);
		CheckAndReportError(psInstance, "clBuildProgram", eResult, init_memreadkernel_cleanup);

		psData->psKernel[typeSizeLog2] = clCreateKernel(psData->psProgram[typeSizeLog2], "MemoryStrideRead", &eResult);
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
 Function Name      : Verify_MemStrideReadKernel
 Inputs             : None
 Outputs            : None
 Returns            : None
 Description        : Verifies output data
************************************************************************************/
cl_int Verify_MemStrideReadKernel(OCLTestInstance *psInstance)
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

	MemStrideReadKernelData *psData =(MemStrideReadKernelData*) psInstance->pvPrivateData;
	psData = psData;

	/* Perform ever greater buffer copies */
	for(typeSizeLog2 = 0; typeSizeLog2 != TEST_MEMSTRIDEREAD_NUM_TYPE_SIZES; typeSizeLog2++)
	{
		unsigned int ui32NumPerKernel;
		unsigned int ui32ExpectedOutput;
		size_t global_size;

		typeSize = 1 << typeSizeLog2;

		psData->puGlobalWorkSize[0] = BUFFER_SIZE_BYTES / (typeSize * TEST_MEMSTRIDEREAD_ITERATIONS_PER_INSTANCE);
		/* We have 4 cache lines, 64 bytes wide, so set up the workgroup size to maximize throughput */
		psData->puLocalWorkSize[0]  = 4*64/typeSize;//32;

		global_size = psData->puGlobalWorkSize[0];

		/* Set the number of reads to perform per kernel (always a power of two) */
		ui32NumPerKernel = TEST_MEMSTRIDEREAD_ITERATIONS_PER_INSTANCE;

		OCLTestLog("Performing %d memory reads per kernel instance, totalling %zu memory loads...\n",ui32NumPerKernel,ui32NumPerKernel*psData->puGlobalWorkSize[0]);

		eResult = clSetKernelArg(psData->psKernel[typeSizeLog2], 5, sizeof(cl_uint), (void*)&ui32NumPerKernel );
		CheckAndReportError(psInstance, "clSetKernelArg", eResult, verify_memreadkernel_cleanup);

		/* Enqueue a kernel to copy data from B --> A */
		eResult = clEnqueueNDRangeKernel(psData->psCommandQueue, psData->psKernel[typeSizeLog2], 1, NULL, &global_size, &psData->puLocalWorkSize[0] /*NULL*/, 0, NULL, &psData->psEvent);
		CheckAndReportError(psInstance, "clEnqueueNDRangeKernel", eResult, verify_memreadkernel_cleanup);

		clFinish(psData->psCommandQueue);

		/* Print out timing information (command queue had profiling enabled) */
		eResult = clGetEventProfilingInfo(psData->psEvent,CL_PROFILING_COMMAND_START,sizeof(cl_ulong),&lStart,NULL);
		CheckAndReportError(psInstance, "clGetEventProfilingInfo", eResult, verify_memreadkernel_cleanup);
		eResult = clGetEventProfilingInfo(psData->psEvent,CL_PROFILING_COMMAND_END  ,sizeof(cl_ulong),&lEnd,NULL);
		CheckAndReportError(psInstance, "clGetEventProfilingInfo", eResult, verify_memreadkernel_cleanup);

		/* Calculate number of bytes read by the kernel, and convert to megabytes  */
		ui32MBCopied = psData->puGlobalWorkSize[0] * TEST_MEMSTRIDEREAD_ITERATIONS_PER_INSTANCE *
			TEST_MEMSTRIDEREAD_READS_PER_ITERATION * typeSize;
		ui32MBCopied /= (1 << 20);

		/* Calculate metrics */
		fTimeInSeconds = ((double)(lEnd-lStart)) / 1000000000.0;
		fMBPerSeconds  = (double)(ui32MBCopied)  / fTimeInSeconds;

		OCLTestLog("  BufferType: %8s, Instances: %8zu, Reads per instance: %4d, WG size %zu, Read: %4d MBs, Time: %10fs, %10fMB/s\n",
				   memstridereadTypeSizeNames[typeSizeLog2],
		           psData->puGlobalWorkSize[0],
		           TEST_MEMSTRIDEREAD_ITERATIONS_PER_INSTANCE * TEST_MEMSTRIDEREAD_READS_PER_ITERATION,
				   psData->puLocalWorkSize[0],
		           ui32MBCopied,
		           fTimeInSeconds,
		           fMBPerSeconds);

		OCLMetricOutputDouble(__func__,memstridereadTypeSizeNames[typeSizeLog2],fMBPerSeconds,MBSEC);

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
		eResult = clEnqueueReadBuffer(psData->psCommandQueue, psData->psBufferOut, CL_TRUE, 0, global_size * typeSize, pui32Results, 0, NULL, NULL);
		CheckAndReportError(psInstance, "clEnqueueReadBuffer", eResult, verify_memreadkernel_cleanup);

		ui32ExpectedOutput = (TEST_MEMSTRIDEREAD_ITERATIONS_PER_INSTANCE * TEST_MEMSTRIDEREAD_READS_PER_ITERATION) / 4;

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
#endif
		/* Free the results to prevent leak */
		free(pui32Results);
	}

	/* Cleanup */
	for (typeSizeLog2 = 0; typeSizeLog2 != TEST_MEMSTRIDEREAD_NUM_TYPE_SIZES; typeSizeLog2++)
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

	free(psInstance->pvPrivateData);
	psInstance->pvPrivateData = NULL;

	return eResult;

verify_memreadkernel_cleanup:
	return eResult;
}

/******************************************************************************
 End of file (memread_stride.c)
******************************************************************************/
