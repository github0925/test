/*************************************************************************/ /*!
@File
@Copyright      Copyright (c) Imagination Technologies Ltd. All Rights Reserved
@License        Strictly Confidential.
*/ /**************************************************************************/
#define TEST_MEMCPY_WORKGROUP_MAX_ITERATIONS      4
#ifdef CUT_DOWN_UNIT_TEST
#define TEST_MEMCPY_WORKGROUP_MAX_LOCAL_SIZE	  4
#define TEST_MEMCPY_WORKGROUP_MAX_NUM_WORKGROUPS  4
#else
#define TEST_MEMCPY_WORKGROUP_MAX_LOCAL_SIZE	  8
#define TEST_MEMCPY_WORKGROUP_MAX_NUM_WORKGROUPS  16 
#endif

typedef struct _MemcpyWorkgroupData_
{
	/* CL Objects */
	cl_context       psContext;
	cl_program       psProgram;
	cl_device_id     psDeviceID;
	cl_platform_id   psPlatformID;
	cl_command_queue psCommandQueue;
	cl_kernel        psKernel;
	cl_mem           psBufferA[TEST_MEMCPY_WORKGROUP_MAX_ITERATIONS];
	cl_mem           psBufferB[TEST_MEMCPY_WORKGROUP_MAX_ITERATIONS];
	cl_event         psEvent[TEST_MEMCPY_WORKGROUP_MAX_ITERATIONS];

	/* Host Object */
	unsigned int *pui32Buffer[TEST_MEMCPY_WORKGROUP_MAX_ITERATIONS];
	size_t        apuGlobalWorkSize[TEST_MEMCPY_WORKGROUP_MAX_ITERATIONS][3];
	size_t        apuLocalWorkSize[TEST_MEMCPY_WORKGROUP_MAX_ITERATIONS][3];
	size_t        auBufferSize[TEST_MEMCPY_WORKGROUP_MAX_ITERATIONS];

} MemcpyWorkgroupData;

cl_int Init_MemcpyWorkgroup(OCLTestInstance *psInstance);
cl_int Compute_MemcpyWorkgroup(OCLTestInstance *psInstance);
cl_int Verify_MemcpyWorkgroup(OCLTestInstance *psInstance);

static char *g_pszMemcpyWorkgroupSource =
{
	"__kernel void MemoryCopyWorkgroup(__global unsigned int* a, __global unsigned int* b)\n"
	"{\n"
	"\tunsigned int global_id_x = get_group_id(0) * get_local_size(0) + get_local_id(0);\n"
	"\tunsigned int global_id_y = get_group_id(1) * get_local_size(1) + get_local_id(1);\n"
	"\tunsigned int global_id_z = get_group_id(2) * get_local_size(2) + get_local_id(2);\n"
	"\tint ith = (global_id_z * get_global_size(0) * get_global_size(1)) + (global_id_y * get_global_size(0)) + global_id_x;\n"
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
cl_int Init_MemcpyWorkgroup(OCLTestInstance *psInstance)
{
	size_t auLengths[2] = {0,0};
	char *ppszSource[2] = { 0 };
	unsigned int i;

	cl_int eResult;

	MemcpyWorkgroupData *psData = (MemcpyWorkgroupData*)calloc(1, sizeof(MemcpyWorkgroupData));

	if(!psData)
	{
		return CL_OUT_OF_RESOURCES;
	}

	/* Initialise data */
	psInstance->pvPrivateData = (void*)psData;

	eResult = clGetPlatformIDs(1,&psData->psPlatformID,NULL);
	CheckAndReportError(psInstance, "clGetPlatformIDs", eResult, init_memcpyworkgroup_cleanup);

	eResult = clGetDeviceIDs(psData->psPlatformID,CL_DEVICE_TYPE_GPU,1,&psData->psDeviceID,NULL);
	CheckAndReportError(psInstance, "clGetDeviceIDs", eResult, init_memcpyworkgroup_cleanup);

	psData->psContext = clCreateContext(NULL,1,&psData->psDeviceID,NULL,NULL,&eResult);
	CheckAndReportError(psInstance, "clCreateContext", eResult, init_memcpyworkgroup_cleanup);

	/* Use the global memcpy program */
	auLengths[0]  = strlen(g_pszMemcpyWorkgroupSource);
	ppszSource[0] = g_pszMemcpyWorkgroupSource;

	/*
	 * Build the Program
	 */
	psData->psProgram = clCreateProgramWithSource(psData->psContext, 1, (const char**)ppszSource, auLengths, &eResult);
	CheckAndReportError(psInstance, "clCreateProgramWithSource", eResult, init_memcpyworkgroup_cleanup);

	eResult = clBuildProgram(psData->psProgram,1,&psData->psDeviceID,"",NULL,NULL);
	CheckAndReportError(psInstance, "clBuildProgram", eResult, init_memcpyworkgroup_cleanup);

	psData->psCommandQueue = clCreateCommandQueue(psData->psContext, psData->psDeviceID, CL_QUEUE_PROFILING_ENABLE, &eResult);
	CheckAndReportError(psInstance, "clCreateCommandQueue", eResult, init_memcpyworkgroup_cleanup);

	/* Prepare the data for each iteration of the test */
	for(i = 0; i < TEST_MEMCPY_WORKGROUP_MAX_ITERATIONS; i++)
	{
		/* Pick the dimensions for the test */
		unsigned int j;
		unsigned int *puiTempBuffer = NULL;

		for(j = 0; j < 3; j++)
		{
			psData->apuLocalWorkSize[i][j] = (rand() % TEST_MEMCPY_WORKGROUP_MAX_LOCAL_SIZE) + 1;
			psData->apuGlobalWorkSize[i][j] = psData->apuLocalWorkSize[i][j] * ((rand() % TEST_MEMCPY_WORKGROUP_MAX_NUM_WORKGROUPS) + 1);
		}

		psData->auBufferSize[i] = psData->apuGlobalWorkSize[i][0] * psData->apuGlobalWorkSize[i][1] * psData->apuGlobalWorkSize[i][2] * sizeof(unsigned int);

		/* Initialise host side objects */

		/* Allocate first host buffer */
		psData->pui32Buffer[i] = (unsigned int*)calloc(1, psData->auBufferSize[i]);

		if(!psData->pui32Buffer[i])
		{
			/* Free all the previous buffers */
			for(j = 0; j < i; j++)
				free(psData->pui32Buffer[j]);

			free(psData);
			return CL_OUT_OF_RESOURCES;
		}

		puiTempBuffer = (unsigned int*)calloc(1, psData->auBufferSize[i]);

		if(!puiTempBuffer)
		{
			/* Free all the previous buffers */
			for(j = 0; j < i; j++)
				free(psData->pui32Buffer[j]);

			free(psData);
			return CL_OUT_OF_RESOURCES;
		}

		/* Initialise with test values */
		for(j=0; j < psData->auBufferSize[i] / sizeof(unsigned int); j++)
		{
			psData->pui32Buffer[i][j] = 0xFFFFFFFF;
			puiTempBuffer[j] = 0;
		}

		/*
		 * Setup the Arguments
		 */

		psData->psBufferA[i] = clCreateBuffer(psData->psContext, CL_MEM_COPY_HOST_PTR, psData->auBufferSize[i], puiTempBuffer, &eResult);
		CheckAndReportError(psInstance, "clCreateBuffer", eResult, init_memcpyworkgroup_cleanup);

		psData->psBufferB[i] = clCreateBuffer(psData->psContext, CL_MEM_COPY_HOST_PTR, psData->auBufferSize[i], psData->pui32Buffer[i], &eResult);
		CheckAndReportError(psInstance, "clCreateBuffer", eResult, init_memcpyworkgroup_cleanup);


		free(puiTempBuffer);
	}

	psData->psKernel = clCreateKernel(psData->psProgram, "MemoryCopyWorkgroup", &eResult);
	CheckAndReportError(psInstance, "clCreateKernel", eResult, init_memcpyworkgroup_cleanup);

	return CL_SUCCESS;

init_memcpyworkgroup_cleanup:
	return eResult;
}

/***********************************************************************************
 Function Name      : Verify_Memcpy
 Inputs             : None
 Outputs            : None
 Returns            : None
 Description        : Verifies output data
************************************************************************************/
cl_int Verify_MemcpyWorkgroup(OCLTestInstance *psInstance)
{
	unsigned int t;
	unsigned int *pui32Results = NULL;
	cl_int eResult = CL_SUCCESS;

#if !defined(NO_HARDWARE)
	unsigned int i;

	cl_ulong lStart;
	cl_ulong lEnd;

	double fTimeInSeconds;
	double fMBPerSeconds;

	double fMBCopied;
#endif /* !defined(NO_HARDWARE) */

	MemcpyWorkgroupData *psData =(MemcpyWorkgroupData*) psInstance->pvPrivateData;

	/* Perform ever greater buffer copies */
	for(t=0; t < TEST_MEMCPY_WORKGROUP_MAX_ITERATIONS; t++)
	{
		/* Allocate temporary buffer for reading back results */
		pui32Results = (unsigned int*)malloc(psData->auBufferSize[t]);

		if(!pui32Results)
		{
			free(psInstance->pvPrivateData);
			psInstance->pvPrivateData = strdup("Results buffer allocation failure");
			return CL_OUT_OF_RESOURCES;
		}

		eResult = clSetKernelArg(psData->psKernel, 0, sizeof(cl_mem), (void*) &psData->psBufferA[t] );
		CheckAndReportError(psInstance, "clSetKernelArg", eResult, verify_memcpyworkgroup_cleanup);

		eResult = clSetKernelArg(psData->psKernel, 1, sizeof(cl_mem), (void*) &psData->psBufferB[t] );
		CheckAndReportError(psInstance, "clSetKernelArg", eResult, verify_memcpyworkgroup_cleanup);

		/* Enqueue a kernel to copy data from B --> A */
		eResult = clEnqueueNDRangeKernel(psData->psCommandQueue, psData->psKernel, 3, NULL, psData->apuGlobalWorkSize[t], psData->apuLocalWorkSize[t] , 0, NULL, &psData->psEvent[t]);
		CheckAndReportError(psInstance, "clEnqueueNDRangeKernel", eResult, verify_memcpyworkgroup_cleanup);

		/* Read back the results */
		eResult = clEnqueueReadBuffer(psData->psCommandQueue, psData->psBufferA[t], CL_TRUE, 0, psData->auBufferSize[t], pui32Results, 0, NULL, NULL);
		CheckAndReportError(psInstance, "clEnqueueReadBuffer", eResult, verify_memcpyworkgroup_cleanup);

#if !defined(NO_HARDWARE)
		/* Print out timing information (command queue had profiling enabled) */
		eResult = clGetEventProfilingInfo(psData->psEvent[t],CL_PROFILING_COMMAND_START,sizeof(cl_ulong),&lStart,NULL);
		CheckAndReportError(psInstance, "clGetEventProfilingInfo", eResult, verify_memcpyworkgroup_cleanup);
		eResult = clGetEventProfilingInfo(psData->psEvent[t],CL_PROFILING_COMMAND_END  ,sizeof(cl_ulong),&lEnd,NULL);
		CheckAndReportError(psInstance, "clGetEventProfilingInfo", eResult, verify_memcpyworkgroup_cleanup);

		/* Calculate metrics */
		fTimeInSeconds = ((double)(lEnd-lStart)) / 1000000000.0;

		/* Convert bytes to megabytes, each load/store is 4 bytes (one uint32) */
		fMBCopied = (double) (4 * psData->apuGlobalWorkSize[t][0]*psData->apuGlobalWorkSize[t][1]*psData->apuGlobalWorkSize[t][2]) / (double) (1024 * 1024);

		fMBPerSeconds = fMBCopied / fTimeInSeconds;

		OCLTestLog("%s: Instances: %10zu, Copied %10f MBs, Time %10fs %10fMB/s\n",
					__func__,
					psData->apuGlobalWorkSize[t][0]*psData->apuGlobalWorkSize[t][1]*psData->apuGlobalWorkSize[t][2],
					fMBCopied,
					fTimeInSeconds,
					fMBPerSeconds);
		OCLMetricOutputDouble(__func__,"",fMBPerSeconds,MBSEC);
#endif /* defined(NO_HARDWARE) */

		clReleaseEvent(psData->psEvent[t]);
#if !defined(NO_HARDWARE)
		/* Verify the written back data with what we would expect */
		for(i=0; i < psData->apuGlobalWorkSize[t][0]; i++)
		{
			if( pui32Results[i] != psData->pui32Buffer[t][i])
			{
				printf("%s: Verification failure at %d, expected %08x got %08x.\n",
				       __func__,
				       i,
				       psData->pui32Buffer[t][i],
				       pui32Results[i]);

				/* Verification Error */
				free(psInstance->pvPrivateData);
				psInstance->pvPrivateData = strdup("Verification failure");
				return CL_INVALID_VALUE;
			}
		}
#endif

		free(pui32Results);
		pui32Results = NULL;
	}

	clFinish(psData->psCommandQueue);

verify_memcpyworkgroup_cleanup:
	/* Cleanup */

	for(t = 0; t < TEST_MEMCPY_WORKGROUP_MAX_ITERATIONS; t++)
	{
		free(psData->pui32Buffer[t]);
		if(psData->psBufferA[t])	clReleaseMemObject   (psData->psBufferA[t]);
		if(psData->psBufferB[t])	clReleaseMemObject   (psData->psBufferB[t]);
	}

	if(psData->psKernel)		clReleaseKernel      ( psData->psKernel       );
	if(psData->psCommandQueue)	clReleaseCommandQueue( psData->psCommandQueue );
	if(psData->psProgram)		clReleaseProgram     ( psData->psProgram      );
	if(psData->psContext)		clReleaseContext     ( psData->psContext      );

	free(pui32Results);

	free(psInstance->pvPrivateData);
	psInstance->pvPrivateData = NULL;

	return eResult;
}

/******************************************************************************
 End of file (memcpy.c)
******************************************************************************/
