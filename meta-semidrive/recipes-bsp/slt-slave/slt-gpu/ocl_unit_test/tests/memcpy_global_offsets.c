/*************************************************************************/ /*!
@File
@Copyright      Copyright (c) Imagination Technologies Ltd. All Rights Reserved
@License        Strictly Confidential.
*/ /**************************************************************************/

// TEST_MEMCPY_GLOBAL_OFFSETS_MAX_DIMENSION_SIZE % TEST_MEMCPY_GLOBAL_OFFSETS_VAL_OFFSET == 0
#define TEST_MEMCPY_GLOBAL_OFFSETS_MAX_DIMENSION_SIZE	16
#define TEST_MEMCPY_GLOBAL_OFFSETS_VAL_OFFSET			4

typedef struct _MemcpyGlobalOffsetsData_
{
	/* CL Objects */
	cl_context       psContext;
	cl_program       psProgram;
	cl_device_id     psDeviceID;
	cl_platform_id   psPlatformID;
	cl_command_queue psCommandQueue;
	cl_kernel        psKernel;
	cl_mem           psBufferA;
	cl_mem           psBufferB;

	/* Host Object */
	unsigned int *pui32Buffer;
	unsigned int *pui32Output;
	unsigned int uiTotalElements;
	unsigned int uiMaxDimSize;

} MemcpyGlobalOffsetsData;

cl_int Init_MemcpyGlobalOffsets(OCLTestInstance *psInstance);
cl_int Compute_MemcpyGlobalOffsets(OCLTestInstance *psInstance);
cl_int Verify_MemcpyGlobalOffsets(OCLTestInstance *psInstance);

static char *g_pszMemcpyGlobalOffsetsSource =
{
	"__kernel void MemoryCopyOffsets(__global unsigned int* a, __global unsigned int* b, unsigned int d)\n"
	"{\n"
	"\tunsigned int global_id_x = get_global_offset(0) + (get_group_id(0) * get_local_size(0)) + get_local_id(0);\n"
	"\tunsigned int global_id_y = get_global_offset(1) + (get_group_id(1) * get_local_size(1)) + get_local_id(1);\n"
	"\tunsigned int global_id_z = get_global_offset(2) + (get_group_id(2) * get_local_size(2)) + get_local_id(2);\n"
	"\tint ith = (global_id_z * d * d) + (global_id_y * d) + global_id_x;\n"
	"\ta[ith] = b[ith];\n"
	"}"
};

/***********************************************************************************
 Function Name      : Init_Memcpy
 Inputs             : None
 Outputs            : None
 Returns            : None
 Description        : Initialises input data
************************************************************************************/
cl_int Init_MemcpyGlobalOffsets(OCLTestInstance *psInstance)
{
	size_t auLengths[2] = {0,0};
	char *ppszSource[2] = { 0 };
	unsigned int i;

	cl_int eResult;

	MemcpyGlobalOffsetsData *psData = (MemcpyGlobalOffsetsData*)calloc(1, sizeof(MemcpyGlobalOffsetsData));

	if(!psData)
	{
		return CL_OUT_OF_RESOURCES;
	}

	/* Initialise data */
	psInstance->pvPrivateData = (void*)psData;

	eResult = clGetPlatformIDs(1,&psData->psPlatformID,NULL);
	CheckAndReportError(psInstance, "clGetPlatformIDs", eResult, init_memcpyglobaloffsets_cleanup);

	eResult = clGetDeviceIDs(psData->psPlatformID,CL_DEVICE_TYPE_GPU,1,&psData->psDeviceID,NULL);
	CheckAndReportError(psInstance, "clGetDeviceIDs", eResult, init_memcpyglobaloffsets_cleanup);

	psData->psContext = clCreateContext(NULL,1,&psData->psDeviceID,NULL,NULL,&eResult);
	CheckAndReportError(psInstance, "clCreateContext", eResult, init_memcpyglobaloffsets_cleanup);

	/* Use the global memcpy program */
	auLengths[0]  = strlen(g_pszMemcpyGlobalOffsetsSource);
	ppszSource[0] = g_pszMemcpyGlobalOffsetsSource;

	/*
	 * Build the Program
	 */
	psData->psProgram = clCreateProgramWithSource(psData->psContext, 1, (const char**)ppszSource, auLengths, &eResult);
	CheckAndReportError(psInstance, "clCreateProgramWithSource", eResult, init_memcpyglobaloffsets_cleanup);

	eResult = clBuildProgram(psData->psProgram,1,&psData->psDeviceID,"",NULL,NULL);
	CheckAndReportError(psInstance, "clBuildProgram", eResult, init_memcpyglobaloffsets_cleanup);

	psData->psCommandQueue = clCreateCommandQueue(psData->psContext, psData->psDeviceID, CL_QUEUE_PROFILING_ENABLE, &eResult);
	CheckAndReportError(psInstance, "clCreateCommandQueue", eResult, init_memcpyglobaloffsets_cleanup);

	/* Prepare the data for the test */
	i = TEST_MEMCPY_GLOBAL_OFFSETS_MAX_DIMENSION_SIZE;
	psData->uiTotalElements = i * i * i;
	psData->uiMaxDimSize = i;

	psData->pui32Buffer = (unsigned int*)malloc(sizeof(unsigned int) * psData->uiTotalElements);
	psData->pui32Output = (unsigned int*)malloc(sizeof(unsigned int) * psData->uiTotalElements);

	if(!psData->pui32Buffer || !psData->pui32Output)
	{
		eResult = CL_OUT_OF_RESOURCES;
		goto init_memcpyglobaloffsets_cleanup;
	}

	printf("Initialise %d elements..\n", psData->uiTotalElements);

	for(i = 0; i < psData->uiTotalElements; i++)
	{
		psData->pui32Buffer[i] = 0xFFFFFFFF;
		psData->pui32Output[i] = 0;
	}

	/*
	 * Setup the Arguments
	 */
	psData->psBufferA = clCreateBuffer(psData->psContext, CL_MEM_COPY_HOST_PTR, sizeof(cl_uint) * psData->uiTotalElements, psData->pui32Output, &eResult);
	CheckAndReportError(psInstance, "clCreateBuffer", eResult, init_memcpyglobaloffsets_cleanup);

	psData->psBufferB = clCreateBuffer(psData->psContext, CL_MEM_COPY_HOST_PTR, sizeof(cl_uint) * psData->uiTotalElements, psData->pui32Buffer, &eResult);
	CheckAndReportError(psInstance, "clCreateBuffer", eResult, init_memcpyglobaloffsets_cleanup);

	psData->psKernel = clCreateKernel(psData->psProgram, "MemoryCopyOffsets", &eResult);
	CheckAndReportError(psInstance, "clCreateKernel", eResult, init_memcpyglobaloffsets_cleanup);

	eResult = clSetKernelArg(psData->psKernel, 0, sizeof(cl_mem), (void*) &psData->psBufferA );
	CheckAndReportError(psInstance, "clSetKernelArg", eResult, init_memcpyglobaloffsets_cleanup);

	eResult = clSetKernelArg(psData->psKernel, 1, sizeof(cl_mem), (void*) &psData->psBufferB );
	CheckAndReportError(psInstance, "clSetKernelArg", eResult, init_memcpyglobaloffsets_cleanup);

	eResult = clSetKernelArg(psData->psKernel, 2, sizeof(cl_uint), (void*) &psData->uiMaxDimSize);
	CheckAndReportError(psInstance, "clSetKernelArg", eResult, init_memcpyglobaloffsets_cleanup);

	return CL_SUCCESS;

init_memcpyglobaloffsets_cleanup:
	return eResult;
}

/***********************************************************************************
 Function Name      : Verify_Memcpy
 Inputs             : None
 Outputs            : None
 Returns            : None
 Description        : Verifies output data
************************************************************************************/
cl_int Verify_MemcpyGlobalOffsets(OCLTestInstance *psInstance)
{
	unsigned int i,j,k;
	unsigned int *pui32Results = NULL;
	cl_int eResult = CL_SUCCESS;
	size_t global_worksize[3];
	size_t global_offset[3];

	MemcpyGlobalOffsetsData *psData =(MemcpyGlobalOffsetsData*) psInstance->pvPrivateData;

	/* Allocate temporary buffer for reading back results */
	pui32Results = (unsigned int*)malloc(sizeof(unsigned int) * psData->uiTotalElements);

	if(!pui32Results)
	{
		psInstance->pvPrivateData = strdup("Results buffer allocation failure");
		eResult = CL_OUT_OF_HOST_MEMORY;
		goto verify_memcpyglobaloffsets_cleanup;
	}

	global_worksize[0] = global_worksize[1] = global_worksize[2] = TEST_MEMCPY_GLOBAL_OFFSETS_VAL_OFFSET;
	global_offset[0] = global_offset[1] = global_offset[2] = 0;

	/* Perform memcopy computation with different offsets */
	for(k=0; k + global_worksize[2] <= psData->uiMaxDimSize; k+=TEST_MEMCPY_GLOBAL_OFFSETS_VAL_OFFSET)
	{
		global_offset[2] = k;
		for(j=0; j + global_worksize[1] <= psData->uiMaxDimSize; j+=TEST_MEMCPY_GLOBAL_OFFSETS_VAL_OFFSET)
		{
			global_offset[1] = j;
			for(i=0; i + global_worksize[0] <= psData->uiMaxDimSize; i+=TEST_MEMCPY_GLOBAL_OFFSETS_VAL_OFFSET)
			{
				global_offset[0] = i;

				/* Enqueue a kernel to copy data from B --> A */
				eResult = clEnqueueNDRangeKernel(psData->psCommandQueue, psData->psKernel, 3, global_offset, global_worksize, NULL, 0, NULL, NULL);
				CheckAndReportError(psInstance, "clEnqueueNDRangeKernel", eResult, verify_memcpyglobaloffsets_cleanup);

				clFinish(psData->psCommandQueue);
			}
		}
	}

	/* Read back the results */
	eResult = clEnqueueReadBuffer(psData->psCommandQueue, psData->psBufferA, CL_TRUE, 0, sizeof(cl_uint) * psData->uiTotalElements, pui32Results, 0, NULL, NULL);
	CheckAndReportError(psInstance, "clEnqueueReadBuffer", eResult, verify_memcpyglobaloffsets_cleanup);

#if !defined(NO_HARDWARE)
	/* Verify the written back data with what we would expect */
	// global_work_offset[2] is not used in clEnqueueNDRangeKernel
	//for(i=0; i<psData->uiMaxDimSize * psData->uiMaxDimSize * psData->uiMaxDimSize; ++i)
	for(i=0; i<psData->uiMaxDimSize * psData->uiMaxDimSize * TEST_MEMCPY_GLOBAL_OFFSETS_VAL_OFFSET; ++i)
	{
		if( pui32Results[i] != psData->pui32Buffer[i])
		{
			printf("%s: Verification failure at %d, expected %08x got %08x.\n",
				   __func__,
				   i,
				   psData->pui32Buffer[i],
				   pui32Results[i]);

			eResult = CL_INVALID_VALUE;
			goto verify_memcpyglobaloffsets_cleanup;
		}
	}

	printf(" %d elements checked\n",psData->uiMaxDimSize * psData->uiMaxDimSize * TEST_MEMCPY_GLOBAL_OFFSETS_VAL_OFFSET);
#endif

verify_memcpyglobaloffsets_cleanup:
	clReleaseKernel      ( psData->psKernel       );
	clReleaseMemObject   (psData->psBufferA);
	clReleaseMemObject   (psData->psBufferB);
	clReleaseCommandQueue( psData->psCommandQueue );
	clReleaseProgram     ( psData->psProgram      );
	clReleaseContext     ( psData->psContext      );

	free(psData->pui32Buffer);
	free(psData->pui32Output);

	/* Cleanup */
	free(pui32Results);

	free(psInstance->pvPrivateData);
	psInstance->pvPrivateData = NULL;

	return eResult;
}

/******************************************************************************
 End of file (memcpy_global_offsets.c)
******************************************************************************/
