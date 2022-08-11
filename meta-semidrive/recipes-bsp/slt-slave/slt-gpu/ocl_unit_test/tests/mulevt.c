/*************************************************************************/ /*!
@File			mulevt.c
@Copyright		Copyright (c) Imagination Technologies Ltd. All Rights Reserved
@License		Strictly Confidential.
*/ /**************************************************************************/

#define TEST_MULEVT_MAX_ITERATIONS      1
#define TEST_MULEVT_MAX_INSTANCES       32

#define TEST_MULEVT_MAX_QUEUE           32

typedef struct _MulEvtKernelData_
{
	/* CL Objects */
	cl_context       psContext;
	cl_program       psProgram;
	cl_device_id     psDeviceID;
	cl_platform_id   psPlatformID;
	cl_command_queue psCommandQueues[TEST_MULEVT_MAX_QUEUE];
	cl_kernel        psKernels[TEST_MULEVT_MAX_QUEUE];
	cl_mem           psBufferAs[TEST_MULEVT_MAX_QUEUE];
	cl_mem           psBufferBs[TEST_MULEVT_MAX_QUEUE];
	cl_event         psEvent[TEST_MULEVT_MAX_QUEUE];
	int              psHasAnEvent[TEST_MULEVT_MAX_QUEUE];

	/* Host Object */
	unsigned int *pui32InputBuffer;
	unsigned int *pui32OutputBuffer;
	size_t        puGlobalWorkSize[3];
	size_t*       puLocalWorkSize;

} MulEvtKernelData;

cl_int Init_MulEvtKernel(OCLTestInstance *psInstance);
cl_int Verify_MulEvtKernel(OCLTestInstance *psInstance);

static char *g_pszMulEvtKernelSource =
{
	"__kernel void MemoryCopy(__global unsigned int* a, __global unsigned int* b)\n"
	"{\n"
	"\tint ith = get_global_id(0);\n"
	"\ta[ith] = b[ith];\n"
	"}"
};

/***********************************************************************************
 Function Name      : Init_MulEvtKernel
 Inputs             : None
 Outputs            : None
 Returns            : None
 Description        : Initialises input data
************************************************************************************/
cl_int Init_MulEvtKernel(OCLTestInstance *psInstance)
{
	size_t auLengths[2] = {0,0};
	char *ppszSource[2] = { 0 };
	unsigned int i;
	unsigned int j;

	cl_int eResult = CL_SUCCESS;

	MulEvtKernelData *psData = (MulEvtKernelData*)calloc(1, sizeof(MulEvtKernelData));

	if(!psData)
		return CL_OUT_OF_RESOURCES;

	/* Initialise data */
	psInstance->pvPrivateData = (void*)psData;

	/* Initialise host side objects */

	/* Allocate first host buffer */
	psData->pui32InputBuffer  = (unsigned int*)malloc(sizeof(unsigned int)*TEST_MULEVT_MAX_INSTANCES*TEST_MULEVT_MAX_QUEUE);
	psData->pui32OutputBuffer = (unsigned int*)malloc(sizeof(unsigned int)*TEST_MULEVT_MAX_INSTANCES*TEST_MULEVT_MAX_QUEUE);

	if(!psData->pui32InputBuffer || !psData->pui32OutputBuffer)
	{
		eResult = CL_OUT_OF_RESOURCES;
		goto init_mulevtkernel_cleanup;
	}

	/* Initialise with test values */
	for(i=0; i < TEST_MULEVT_MAX_INSTANCES; i++)
	{
		for(j=0; j < TEST_MULEVT_MAX_QUEUE; j++)
		{
			psData->pui32InputBuffer[j*TEST_MULEVT_MAX_INSTANCES + i] = 0xFFFFFFFF;
			psData->pui32OutputBuffer[j*TEST_MULEVT_MAX_INSTANCES + i] = 0x0;
		}
	}

	eResult = clGetPlatformIDs(1,&psData->psPlatformID,NULL);
	CheckAndReportError(psInstance, "clGetPlatformIDs", eResult, init_mulevtkernel_cleanup);

	eResult = clGetDeviceIDs(psData->psPlatformID,CL_DEVICE_TYPE_GPU,1,&psData->psDeviceID,NULL);
	CheckAndReportError(psInstance, "clGetDeviceIDs", eResult, init_mulevtkernel_cleanup);

	psData->psContext = clCreateContext(NULL,1,&psData->psDeviceID,NULL,NULL,&eResult);
	CheckAndReportError(psInstance, "clCreateContext", eResult, init_mulevtkernel_cleanup);

	/* Use the global memcpy program */
	auLengths[0]  = strlen(g_pszMulEvtKernelSource);
	ppszSource[0] = g_pszMulEvtKernelSource;

	/*
	 * Build the Program
	 */
	psData->psProgram = clCreateProgramWithSource(psData->psContext, 1, (const char**)ppszSource, auLengths, &eResult);
	CheckAndReportError(psInstance, "clCreateProgramWithSource", eResult, init_mulevtkernel_cleanup);

	eResult = clBuildProgram(psData->psProgram,1,&psData->psDeviceID,"",NULL,NULL);
	CheckAndReportError(psInstance, "clBuildProgram", eResult, init_mulevtkernel_cleanup);

	for(j=0; j < TEST_MULEVT_MAX_QUEUE; j++)
	{
		psData->psCommandQueues[j] = clCreateCommandQueue(psData->psContext, psData->psDeviceID, CL_QUEUE_PROFILING_ENABLE, &eResult);
		CheckAndReportError(psInstance, "clCreateCommandQueue", eResult, init_mulevtkernel_cleanup);
	}

	/*
	 * Setup the Arguments
	 */
	for(j=0; j < TEST_MULEVT_MAX_QUEUE; j++)
	{
		psData->psBufferAs[j] = clCreateBuffer(psData->psContext,
											   CL_MEM_COPY_HOST_PTR,
											   sizeof(cl_uint) * TEST_MULEVT_MAX_INSTANCES,
											   &psData->pui32OutputBuffer[TEST_MULEVT_MAX_INSTANCES*j],
											   &eResult);

		CheckAndReportError(psInstance, "clCreateBuffer", eResult, init_mulevtkernel_cleanup);

		psData->psBufferBs[j] = clCreateBuffer(psData->psContext,
			   								   CL_MEM_COPY_HOST_PTR,
											   sizeof(cl_uint) * TEST_MULEVT_MAX_INSTANCES,
											   &psData->pui32InputBuffer[TEST_MULEVT_MAX_INSTANCES*j],
											   &eResult);

		CheckAndReportError(psInstance, "clCreateBuffer", eResult, init_mulevtkernel_cleanup);

		psData->psKernels[j] = clCreateKernel(psData->psProgram, "MemoryCopy", &eResult);
		CheckAndReportError(psInstance, "clCreateKernel", eResult, init_mulevtkernel_cleanup);

		eResult = clSetKernelArg(psData->psKernels[j], 0, sizeof(cl_mem), (void*) &psData->psBufferAs[j] );
		CheckAndReportError(psInstance, "clSetKernelArg", eResult, init_mulevtkernel_cleanup);

		eResult = clSetKernelArg(psData->psKernels[j], 1, sizeof(cl_mem), (void*) &psData->psBufferBs[j] );
		CheckAndReportError(psInstance, "clSetKernelArg", eResult, init_mulevtkernel_cleanup);
	}

	/* Setup the size of the kernel run */
	psData->puGlobalWorkSize[0] = TEST_MULEVT_MAX_INSTANCES;
	psData->puLocalWorkSize = NULL;

init_mulevtkernel_cleanup:
	/* Free the initial host buffer as no longer required, has been copied into device */
	free(psData->pui32InputBuffer);
	free(psData->pui32OutputBuffer);

	return eResult;
}

/***********************************************************************************
 Function Name      : Verify_MulEvtKernel
 Inputs             : None
 Outputs            : None
 Returns            : None
 Description        : Verifies output data
************************************************************************************/
cl_int Verify_MulEvtKernel(OCLTestInstance *psInstance)
{
	unsigned int failed = 0;
	unsigned int t;
	unsigned int j;
	unsigned int *pui32Results = NULL;
	cl_int eResult = CL_SUCCESS;

#if !defined(NO_HARDWARE)
	unsigned int i;

	cl_ulong lStart;
	cl_ulong lEnd;

	double fTimeInSeconds;
	double fMBPerSeconds;

	unsigned int ui32MBCopied;
#endif /* !defined(NO_HARDWARE) */

	MulEvtKernelData *psData =(MulEvtKernelData*) psInstance->pvPrivateData;
	psData = psData;



	/* Perform ever greater buffer copies */
	for(t=0; t < TEST_MULEVT_MAX_ITERATIONS; t++)
	{
		for(j=0; j < TEST_MULEVT_MAX_QUEUE; j++)
		{
			/* Enqueue a kernel to copy data from B --> A */
			if(rand()%2)
			{
				psData->psHasAnEvent[j] = 0;  // No event
				eResult = clEnqueueNDRangeKernel(psData->psCommandQueues[j], psData->psKernels[j], 1, NULL, psData->puGlobalWorkSize, psData->puLocalWorkSize, 0, NULL, NULL);
			}
			else
			{
				psData->psHasAnEvent[j] = 1; // Event
				eResult = clEnqueueNDRangeKernel(psData->psCommandQueues[j], psData->psKernels[j], 1, NULL, psData->puGlobalWorkSize, psData->puLocalWorkSize, 0, NULL, &psData->psEvent[j]);
			}

			CheckAndReportError(psInstance, "clEnqueueNDRangeKernel", eResult, verify_mulevtkernel_cleanup);
		}

		for(j=0; j < TEST_MULEVT_MAX_QUEUE; j++)
		{
			clFinish(psData->psCommandQueues[j]);
		}

		for(j=0; j < TEST_MULEVT_MAX_QUEUE; j++)
		{
			if(psData->psHasAnEvent[j])
			{
#if !defined(NO_HARDWARE)
				/* Print out timing information (command queue had profiling enabled) */
				eResult = clGetEventProfilingInfo(psData->psEvent[j],CL_PROFILING_COMMAND_START,sizeof(cl_ulong),&lStart,NULL);
				CheckAndReportError(psInstance, "clGetEventProfilingInfo", eResult, verify_mulevtkernel_cleanup);
				eResult = clGetEventProfilingInfo(psData->psEvent[j],CL_PROFILING_COMMAND_END  ,sizeof(cl_ulong),&lEnd,NULL);
				CheckAndReportError(psInstance, "clGetEventProfilingInfo", eResult, verify_mulevtkernel_cleanup);

				/* Convert bytes to megabytes, each load/store is 4 bytes (one uint32) */
				ui32MBCopied = (4 * TEST_MULEVT_MAX_INSTANCES) / (1024 * 1024);

				/* Calculate metrics */
				fTimeInSeconds = ((double)(lEnd-lStart)) / 1000000000.0;
				fMBPerSeconds  = (double)(ui32MBCopied)  / fTimeInSeconds;

				OCLTestLog("%s: Instances: %10d, Copied %4d MBs, Time %10fs %10fMB/s\n",__func__,TEST_MULEVT_MAX_INSTANCES,ui32MBCopied,fTimeInSeconds,fMBPerSeconds);
#endif
				clReleaseEvent(psData->psEvent[j]);
			}
		}

		/* Allocate temporary buffer for reading back results */
		pui32Results = (unsigned int*)malloc(sizeof(unsigned int)*TEST_MULEVT_MAX_INSTANCES);

		if(!pui32Results)
		{
			free(psInstance->pvPrivateData);
			psInstance->pvPrivateData = strdup("Results buffer allocation failure");
			return CL_OUT_OF_RESOURCES;
		}

		for(j=0; j < TEST_MULEVT_MAX_QUEUE; j++)
		{
			/* Read back the results */
			eResult = clEnqueueReadBuffer(psData->psCommandQueues[j], psData->psBufferAs[j], CL_TRUE, 0, sizeof(cl_uint) * TEST_MULEVT_MAX_INSTANCES, pui32Results, 0, NULL, NULL);
			CheckAndReportError(psInstance, "clEnqueueReadBuffer", eResult, verify_mulevtkernel_cleanup);

#if !defined(NO_HARDWARE)
			/* Verify the written back data with what we would expect */
			for(i=0; i < TEST_MULEVT_MAX_INSTANCES; i++)
			{
				if( pui32Results[i] != 0xFFFFFFFF)
				{
					OCLTestLog("%s: Verification failure at %d, expected %08x got %08x.\n",
						   __func__,
						   i,
						   0xFFFFFFFF,
						   pui32Results[i]);

					failed = 1;
				}
			}
#endif
		}

		if(failed)
		{
			/* Cleanup on failure */
			for(j=0; j < TEST_MULEVT_MAX_QUEUE; j++)
			{
				clReleaseKernel      ( psData->psKernels[j]		);
				clReleaseMemObject   ( psData->psBufferAs[j]	);
				clReleaseMemObject   ( psData->psBufferBs[j]	);
				clReleaseCommandQueue( psData->psCommandQueues[j]);
			}
			clReleaseProgram     ( psData->psProgram      );
			clReleaseContext     ( psData->psContext      );
			free(pui32Results);

			/* Verification Error */
			free(psInstance->pvPrivateData);
			psInstance->pvPrivateData = strdup("Verification failure");
			return CL_INVALID_VALUE;
		}
	}

	/* Cleanup */
	for(j=0; j < TEST_MULEVT_MAX_QUEUE; j++)
	{
		clReleaseKernel      ( psData->psKernels[j]		);
		clReleaseMemObject   ( psData->psBufferAs[j]	);
		clReleaseMemObject   ( psData->psBufferBs[j]	);
		clReleaseCommandQueue( psData->psCommandQueues[j]);
	}
	clReleaseProgram     ( psData->psProgram      );
	clReleaseContext     ( psData->psContext      );
	free(pui32Results);

	free(psInstance->pvPrivateData);
	psInstance->pvPrivateData = NULL;

	return eResult;

verify_mulevtkernel_cleanup:
	return eResult;
}

/******************************************************************************
 End of file (mulvt.c)
******************************************************************************/
