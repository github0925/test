/*************************************************************************/ /*!
@File           async.c
@Copyright      Copyright (c) Imagination Technologies Ltd. All Rights Reserved
@License        Strictly Confidential.
*/ /**************************************************************************/

#define TEST_MEMCPY_NUM_TYPE_SIZES	  7


//#define COMPILE_TIME_KNOWN_BURST_LENGTH

typedef struct _AsyncMemCopyKernelData_
{
	/* CL Objects */
	cl_context       psContext;
	cl_device_id     psDeviceID;
	cl_platform_id   psPlatformID;
	cl_command_queue psCommandQueue;
	cl_kernel        psCopyToKernel[TEST_MEMCPY_NUM_TYPE_SIZES];
	cl_kernel        psCopyFromKernel[TEST_MEMCPY_NUM_TYPE_SIZES];
	size_t           auCopyLocalSize[TEST_MEMCPY_NUM_TYPE_SIZES];
	cl_kernel        psVerifyKernel[TEST_MEMCPY_NUM_TYPE_SIZES];
	cl_mem           psBufferA;
	cl_mem           psBufferB;
	cl_mem           psBufferC;
	cl_event         psEvent;

	/* Host Object */
	unsigned int *pui32Buffer;
	size_t        puGlobalWorkSize[3];
	cl_uint ASYNC_BUFFER_SIZE;
	cl_uint ASYNC_BUFFER_SIZE_BYTES;
	cl_uint WORK_GROUPS_PER_BUFFER;
	cl_uint ASYNC_BUFFER_SIZE_BYTES_PER_WG;

} AsyncMemCopyKernelData;

static char* TypeSizeNames[TEST_MEMCPY_NUM_TYPE_SIZES] = {"char", "short", "int", "int2", "int4", "int8", "int16"};
cl_int Init_AsyncMemCopyKernel(OCLTestInstance *psInstance);
cl_int Compute_AsycMemCopyKernel(OCLTestInstance *psInstance);
cl_int Verify_AsyncMemCopyKernel(OCLTestInstance *psInstance);

static char *g_pszAsyncMemCopyToLocalKernelSource =
{
	"__kernel __attribute__((reqd_work_group_size(%d, 1, 1))) void AsyncMemoryCopyTo(__global %s* a, __global %s* b, uint n)\n"
	"{\n"
	"	local %s temp[%d];\n"
	"	const int copies_per_item = %d;\n"
	"	async_work_group_copy(temp, b + get_group_id(0) * get_local_size(0) * copies_per_item, %s, 0 /* event */);\n"
	"	barrier(CLK_LOCAL_MEM_FENCE);\n"
	"	for (int i = 0; i < copies_per_item; i++)\n"
	"	{\n"
	"		a[get_global_id(0) * copies_per_item + i] = temp[get_local_id(0) * copies_per_item + i];\n"
	"	}\n"
	"}"
};

static char *g_pszAsyncMemCopyFromLocalKernelSource =
{
	"__kernel __attribute__((reqd_work_group_size(%d, 1, 1))) void AsyncMemoryCopyFrom(__global %s* a, __global %s* b, uint n)\n"
	"{\n"
	"	local %s temp[%d];\n"
	"	const int copies_per_item = %d;\n"
	"	for (int i = 0; i < copies_per_item; i++)\n"
	"	{\n"
	"		temp[get_local_id(0) * copies_per_item + i] = b[get_global_id(0) * copies_per_item + i];\n"
	"	}\n"
	"	barrier(CLK_LOCAL_MEM_FENCE);\n"
	"	async_work_group_copy(a + get_group_id(0) * get_local_size(0) * copies_per_item, temp, %s, 0 /* event */);\n"
	"}"
};

static char *g_pszVerifyAsyncMemCopyKernelSource =
{
	"int __attribute__((overloadable)) myeq(int a, int b)\n"
	"{\n"
	"	return a==b;\n"
	"}\n"
	"int __attribute__((overloadable)) myeq(int2 a, int2 b)\n"
	"{\n"
	"	return all(a==b);\n"
	"}\n"
	"int __attribute__((overloadable)) myeq(int4 a, int4 b)\n"
	"{\n"
	"	return all(a==b);\n"
	"}\n"
	"int __attribute__((overloadable)) myeq(int8 a, int8 b)\n"
	"{\n"
	"	return all(a==b);\n"
	"}\n"
	"int __attribute__((overloadable)) myeq(int16 a, int16 b)\n"
	"{\n"
	"	return all(a==b);\n"
	"}\n"
	"__kernel void AsyncMemoryCopyVerify(__global %s* a, __global %s* b, uint n, __global int* c)\n"
	"{\n"
	"	int i;\n"
	"	int res = 1;\n"
	"	for (i = 0; i < n; i++)\n"
	"	{\n"
	"	if (!myeq(a[i], b[i]))\n"
	"		{\n"
	"			res = 0;\n"
	"			break;\n"
	"		}\n"
	"	}\n"
	"	*c = res;\n"
	"}"
};


static
cl_int Async_CreateKernel(OCLTestInstance *psInstance, AsyncMemCopyKernelData *psData, char* pszProgramSource, char* pszKernelName, cl_kernel* ppsKernel)
{
	cl_int eResult;
	size_t auLengths[2] = {0,0};
	const char* programSourcePtrs[1] = {pszProgramSource};
	char*       aszBuildLog = NULL;

	/* Use the global memwrite program */
	auLengths[0] = strlen(pszProgramSource);

	/*
	 * Build the Program
	 */
	cl_program psProgram = clCreateProgramWithSource(psData->psContext, 1, programSourcePtrs, auLengths, &eResult);
	CheckAndReportError(psInstance, "clCreateProgramWithSource", eResult, async_createkernel_cleanup);

	eResult = clBuildProgram(psProgram,1,&psData->psDeviceID,"",NULL,NULL);
	if(eResult != CL_SUCCESS)
	{
		size_t uBuildLogSize;
		cl_int eSubResult;

		eSubResult = clGetProgramBuildInfo(psProgram, psData->psDeviceID, CL_PROGRAM_BUILD_LOG, 0 /* param_value_size */, NULL /* param_value */, &uBuildLogSize);
		CheckAndReportError(psInstance, "clGetProgramBuildInfo (for size)", eSubResult, async_createkernel_cleanup);

		aszBuildLog = malloc(uBuildLogSize);
		if (aszBuildLog == NULL)
		{
			eResult = CL_OUT_OF_RESOURCES;
			goto async_createkernel_cleanup;
		}

		eSubResult = clGetProgramBuildInfo(psProgram, psData->psDeviceID, CL_PROGRAM_BUILD_LOG, uBuildLogSize, aszBuildLog, &uBuildLogSize);
		CheckAndReportError(psInstance, "clGetProgramBuildInfo", eSubResult, async_createkernel_cleanup);

		aszBuildLog[uBuildLogSize-1] = '\0';
		OCLTestLog("Failed program was:\n%s\n", pszProgramSource);
		OCLTestLog("[%zu] CL_PROGRAM_BUILD_LOG:\n%s\n",uBuildLogSize,aszBuildLog);
	}
	CheckAndReportError(psInstance, "clBuildProgram", eResult, async_createkernel_cleanup);

	cl_kernel psKernel = clCreateKernel(psProgram, pszKernelName, &eResult);
	CheckAndReportError(psInstance, "clCreateKernel", eResult, async_createkernel_cleanup);

	/*
	 * Setup the Arguments
	 */
	eResult = clSetKernelArg(psKernel, 0, sizeof(cl_mem), (void*)&psData->psBufferA);
	CheckAndReportError(psInstance, "clSetKernelArg", eResult, async_createkernel_cleanup);

	eResult = clSetKernelArg(psKernel, 1, sizeof(cl_mem), (void*)&psData->psBufferB);
	CheckAndReportError(psInstance, "clSetKernelArg", eResult, async_createkernel_cleanup);

	clReleaseProgram(psProgram);

	*ppsKernel = psKernel;
	free(aszBuildLog);
	return CL_SUCCESS;

async_createkernel_cleanup:
	free(aszBuildLog);
	return eResult;
}

/***********************************************************************************
 Function Name      : Init_AsyncMemCopyKernel
 Inputs             : None
 Outputs            : None
 Returns            : None
 Description        : Initialises input data
************************************************************************************/
cl_int Init_AsyncMemCopyKernel(OCLTestInstance *psInstance)
{
	cl_int eResult;
	int typeSizeLog2;
	int typeSize;
	char programSource[2048];
	unsigned int i;

	AsyncMemCopyKernelData* psData = (AsyncMemCopyKernelData*)calloc(1, sizeof(AsyncMemCopyKernelData));

	if(!psData)
		return CL_OUT_OF_RESOURCES;

	/* Initialise data */
	psInstance->pvPrivateData = (void*)psData;

	/* Initialise host side objects */

	eResult = clGetPlatformIDs(1,&psData->psPlatformID,NULL);
	CheckAndReportError(psInstance, "clGetPlatformIDs", eResult, init_memcopykernel_cleanup);

	eResult = clGetDeviceIDs(psData->psPlatformID,CL_DEVICE_TYPE_GPU,1,&psData->psDeviceID,NULL);
	CheckAndReportError(psInstance, "clGetDeviceIDs", eResult, init_memcopykernel_cleanup);

	psData->psContext = clCreateContext(NULL,1,&psData->psDeviceID,NULL,NULL,&eResult);
	CheckAndReportError(psInstance, "clCreateContext", eResult, init_memcopykernel_cleanup);

	psData->psCommandQueue = clCreateCommandQueue(psData->psContext, psData->psDeviceID, CL_QUEUE_PROFILING_ENABLE, &eResult);
	CheckAndReportError(psInstance, "clCreateCommandQueue", eResult, init_memcopykernel_cleanup);

	cl_ulong ulSize;
	eResult = clGetDeviceInfo(psData->psDeviceID, CL_DEVICE_LOCAL_MEM_SIZE, sizeof(ulSize), &ulSize, NULL);
	CheckAndReportError(psInstance, "clGetDeviceInfo", eResult, init_memcopykernel_cleanup);

	psData->ASYNC_BUFFER_SIZE = MIN(ulSize, 6144) / sizeof(unsigned int);
	psData->ASYNC_BUFFER_SIZE_BYTES = psData->ASYNC_BUFFER_SIZE * sizeof(unsigned int);
	psData->WORK_GROUPS_PER_BUFFER = 4;
	psData->ASYNC_BUFFER_SIZE_BYTES_PER_WG = psData->ASYNC_BUFFER_SIZE_BYTES / psData->WORK_GROUPS_PER_BUFFER;

	/* Allocate first host buffer */
	psData->pui32Buffer = (unsigned int*)malloc(psData->ASYNC_BUFFER_SIZE_BYTES);

	if(!psData->pui32Buffer)
	{
		free(psData);
		return CL_OUT_OF_RESOURCES;
	}

	/* Initialise with test values */
	for(i=0; i < psData->ASYNC_BUFFER_SIZE; i++)
	{
		psData->pui32Buffer[i] = rand();
	}

	psData->psBufferA = clCreateBuffer(psData->psContext, 0, psData->ASYNC_BUFFER_SIZE_BYTES, NULL, &eResult);
	CheckAndReportError(psInstance, "clCreateBuffer", eResult, init_memcopykernel_cleanup);

	psData->psBufferB = clCreateBuffer(psData->psContext, CL_MEM_COPY_HOST_PTR, psData->ASYNC_BUFFER_SIZE_BYTES, psData->pui32Buffer, &eResult);
	CheckAndReportError(psInstance, "clCreateBuffer", eResult, init_memcopykernel_cleanup);

	psData->psBufferC = clCreateBuffer(psData->psContext, 0, sizeof(cl_int), NULL, &eResult);
	CheckAndReportError(psInstance, "clCreateBuffer", eResult, init_memcopykernel_cleanup);

	size_t auMaximumWGSize[3];
	eResult = clGetDeviceInfo(psData->psDeviceID, CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(auMaximumWGSize), auMaximumWGSize, NULL);
	CheckAndReportError(psInstance, "clGetDeviceInfo", eResult, init_memcopykernel_cleanup);

	for (typeSizeLog2 = 0; typeSizeLog2 != TEST_MEMCPY_NUM_TYPE_SIZES; ++typeSizeLog2)
	{
		typeSize = 1 << typeSizeLog2;

		IMG_UINT32 ui32ElemsPerWG = psData->ASYNC_BUFFER_SIZE_BYTES_PER_WG / typeSize;

		char aszASyncCopySize[256];
#if defined(COMPILE_TIME_KNOWN_BURST_LENGTH)
		sprintf(aszASyncCopySize, "%d", ui32ElemsPerWG);
#else /* COMPILE_TIME_KNOWN_BURST_LENGTH */
		strcpy(aszASyncCopySize, "n");
#endif /* COMPILE_TIME_KNOWN_BURST_LENGTH */

		IMG_UINT32 ui32ElemsPerItem, ui32WGSizeX;
		if (ui32ElemsPerWG < auMaximumWGSize[0])
		{
			ui32ElemsPerItem = 1;
			ui32WGSizeX = ui32ElemsPerWG;
		}
		else
		{
			ui32WGSizeX = auMaximumWGSize[0];
			while ((ui32ElemsPerWG % ui32WGSizeX) != 0)
			{
				ui32WGSizeX--;
			}

			ui32ElemsPerItem = ui32ElemsPerWG / ui32WGSizeX;
		}
		psData->auCopyLocalSize[typeSizeLog2] = ui32WGSizeX;

		/*
		  Create the memory copy kernel; using async_work_group_copy to local memory.
		 */
		sprintf(programSource,
				g_pszAsyncMemCopyToLocalKernelSource,
				ui32WGSizeX,
				TypeSizeNames[typeSizeLog2],
				TypeSizeNames[typeSizeLog2],
				TypeSizeNames[typeSizeLog2],
				ui32ElemsPerWG,
				ui32ElemsPerItem,
				aszASyncCopySize);
		eResult = Async_CreateKernel(psInstance, psData, programSource, "AsyncMemoryCopyTo", &psData->psCopyToKernel[typeSizeLog2]);
		CheckAndReportError(psInstance, "CreateKernel(CopyTo)", eResult, init_memcopykernel_cleanup);

		/*
		  Create the memory copy kernel; using async_work_group_copy from local memory.
		 */
		sprintf(programSource,
				g_pszAsyncMemCopyFromLocalKernelSource,
				ui32WGSizeX,
				TypeSizeNames[typeSizeLog2],
				TypeSizeNames[typeSizeLog2],
				TypeSizeNames[typeSizeLog2],
				ui32ElemsPerWG,
				ui32ElemsPerItem,
				aszASyncCopySize);
		eResult = Async_CreateKernel(psInstance, psData, programSource, "AsyncMemoryCopyFrom", &psData->psCopyFromKernel[typeSizeLog2]);
		CheckAndReportError(psInstance, "CreateKernel(CopyFrom)", eResult, init_memcopykernel_cleanup);

		/*
		  Create the verify kernel.
		 */
		sprintf(programSource, g_pszVerifyAsyncMemCopyKernelSource, TypeSizeNames[typeSizeLog2], TypeSizeNames[typeSizeLog2]);
		eResult = Async_CreateKernel(psInstance, psData, programSource, "AsyncMemoryCopyVerify", &psData->psVerifyKernel[typeSizeLog2]);
		CheckAndReportError(psInstance, "CreateKernel(Verify)", eResult, init_memcopykernel_cleanup);
	}

	/* Free the initial host buffer as no longer required, has been copied into device */
	free(psData->pui32Buffer);
	psData->pui32Buffer = NULL;

	return CL_SUCCESS;

init_memcopykernel_cleanup:
	return eResult;
}

/***********************************************************************************
 Function Name      : Verify_AsyncMemCopyKernel
 Inputs             : None
 Outputs            : None
 Returns            : None
 Description        : Verifies output data
************************************************************************************/
cl_int Verify_AsyncMemCopyKernel(OCLTestInstance *psInstance)
{
	unsigned int *pui32Results = NULL;
	cl_int eResult = CL_SUCCESS;

	int typeSizeLog2;
	int typeSize;

	AsyncMemCopyKernelData *psData =(AsyncMemCopyKernelData*) psInstance->pvPrivateData;
	//psData = psData;

	/* Allocate temporary buffer for reading back results */
	pui32Results = (unsigned int*)malloc(psData->ASYNC_BUFFER_SIZE_BYTES);

	if(!pui32Results)
	{
		free(psInstance->pvPrivateData);
		psInstance->pvPrivateData = strdup("Results buffer allocation failure");
		return CL_OUT_OF_RESOURCES;
	}

	/* Allocate temporary buffer for initializing B. */
	IMG_PUINT32 pui32AInit = malloc(psData->ASYNC_BUFFER_SIZE_BYTES);
	if(!pui32AInit)
	{
		eResult = CL_OUT_OF_RESOURCES;
		goto verify_memcopykernel_cleanup;
	}
	for (IMG_UINT32 i = 0; i < psData->ASYNC_BUFFER_SIZE; i++)
	{
		pui32AInit[i] = 0xdeadbeef;
	}

	for (typeSizeLog2 = 0; typeSizeLog2 != TEST_MEMCPY_NUM_TYPE_SIZES; typeSizeLog2++)
	{
		typeSize = 1 << typeSizeLog2;

		unsigned int ui32ElementsPerWG = psData->ASYNC_BUFFER_SIZE_BYTES_PER_WG / typeSize;
		size_t ui32GlobalSize = psData->auCopyLocalSize[typeSizeLog2] * psData->WORK_GROUPS_PER_BUFFER;

		for (IMG_UINT32 uDir = 0; uDir < 2; uDir++)
		{
			cl_kernel psCopyKernel = (uDir == 0) ? psData->psCopyToKernel[typeSizeLog2] : psData->psCopyFromKernel[typeSizeLog2];

			/* Set the number of write to perform each workgroup. */
			eResult = clSetKernelArg(psCopyKernel, 2 /* arg_index */, sizeof(cl_uint), (void*)&ui32ElementsPerWG);
			CheckAndReportError(psInstance, "clSetKernelArg", eResult, verify_memcopykernel_cleanup);

			/* Clear buffer A. */
			eResult = clEnqueueWriteBuffer(psData->psCommandQueue, psData->psBufferA, IMG_TRUE /* blocking_write */, 0 /* offset */, psData->ASYNC_BUFFER_SIZE_BYTES /* size */, pui32AInit, 0, NULL, &psData->psEvent);
			CheckAndReportError(psInstance, "clEnqueueWriteBuffer", eResult, verify_memcopykernel_cleanup);

			/* Enqueue a kernel to copy data from B --> A */
			eResult = clEnqueueNDRangeKernel(psData->psCommandQueue, psCopyKernel, 1, NULL, &ui32GlobalSize, &psData->auCopyLocalSize[typeSizeLog2], 0, NULL, &psData->psEvent);
			CheckAndReportError(psInstance, "clEnqueueNDRangeKernel", eResult, verify_memcopykernel_cleanup);

			IMG_UINT32 ui32BufferSizeInElements = psData->ASYNC_BUFFER_SIZE_BYTES / typeSize;
			eResult = clSetKernelArg(psData->psVerifyKernel[typeSizeLog2], 2 /* arg_index */, sizeof(cl_uint), &ui32BufferSizeInElements);
			CheckAndReportError(psInstance, "clSetKernelArg", eResult, verify_memcopykernel_cleanup);

			eResult = clSetKernelArg(psData->psVerifyKernel[typeSizeLog2], 3, sizeof(cl_mem), (void*) &psData->psBufferC);
			CheckAndReportError(psInstance, "clSetKernelArg", eResult, verify_memcopykernel_cleanup);

			/* Enqueue a kernel to check A. */
			size_t uOneInstance = 1;
			eResult = clEnqueueNDRangeKernel(psData->psCommandQueue, psData->psVerifyKernel[typeSizeLog2], 1, NULL, &uOneInstance, NULL /* local_work_size */ , 0, NULL, &psData->psEvent);
			CheckAndReportError(psInstance, "clEnqueueNDRangeKernel", eResult, verify_memcopykernel_cleanup);

			clFinish(psData->psCommandQueue);

			clReleaseEvent(psData->psEvent);

			/* Read back the results */
			eResult = clEnqueueReadBuffer(psData->psCommandQueue, psData->psBufferC, CL_TRUE, 0, sizeof(cl_int), pui32Results, 0, NULL, NULL);
			CheckAndReportError(psInstance, "clEnqueueReadBuffer", eResult, verify_memcopykernel_cleanup);

#if !defined(NO_HARDWARE)
			/* Verify the written back data with what we would expect */
			if (pui32Results[0] != 1)
			{
				OCLTestLog("%s: Verification failure\n", __func__);
				break;
			}
#endif
		}
	}

verify_memcopykernel_cleanup:
	/* Cleanup */
	for (typeSizeLog2 = 0; typeSizeLog2 != TEST_MEMCPY_NUM_TYPE_SIZES; typeSizeLog2++)
	{
		if(psData->psCopyToKernel[typeSizeLog2])	clReleaseKernel( psData->psCopyToKernel[typeSizeLog2] );
		if(psData->psCopyFromKernel[typeSizeLog2])	clReleaseKernel( psData->psCopyFromKernel[typeSizeLog2] );
		if(psData->psVerifyKernel[typeSizeLog2])	clReleaseKernel( psData->psVerifyKernel[typeSizeLog2] );
	}

	if(psData->psBufferA)		clReleaseMemObject   ( psData->psBufferA      );
	if(psData->psBufferB)		clReleaseMemObject   ( psData->psBufferB      );
	if(psData->psBufferC)		clReleaseMemObject   ( psData->psBufferC      );
	if(psData->psCommandQueue)	clReleaseCommandQueue( psData->psCommandQueue );
	if(psData->psContext)		clReleaseContext     ( psData->psContext      );

	free(pui32Results);

	free(pui32AInit);

	free(psInstance->pvPrivateData);
	psInstance->pvPrivateData = NULL;

	return eResult;
}

/******************************************************************************
 End of file (async.c)
******************************************************************************/
