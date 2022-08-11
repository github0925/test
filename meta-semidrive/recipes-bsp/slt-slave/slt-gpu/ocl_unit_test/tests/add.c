/*************************************************************************/ /*!
@File           add.c
@Copyright      Copyright (c) Imagination Technologies Ltd. All Rights Reserved
@License        Strictly Confidential.
*/ /**************************************************************************/

typedef struct _AddData_
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
	unsigned int *pui32BufferA;
	unsigned int *pui32BufferB;
	size_t        puGlobalWorkSize[3];
	size_t*       puLocalWorkSize;

} AddData;

cl_int Init_Add(OCLTestInstance *psInstance);
cl_int Compute_Add(OCLTestInstance *psInstance);
cl_int Verify_Add(OCLTestInstance *psInstance);

#define TEST_ADD_MAX_INSTANCES 4096

static char *g_pszAddSource =
{
	"__kernel void AdditionKernel(__global int* a, __global int* b)\n"
	"{\n"
	"\tint ith = get_global_id(0);\n"
	"\ta[ith] = a[ith] + b[ith];\n"
	"}"
};

/***********************************************************************************
 Function Name      : Init_Add
 Inputs             : None
 Outputs            : None
 Returns            : None
 Description        : Initialises input data
************************************************************************************/
cl_int Init_Add(OCLTestInstance *psInstance)
{
	size_t auLengths[2] = {0,0};
	char *ppszSource[2] = { 0 };

	cl_int eResult;

	AddData *psData = (AddData*)calloc(1, sizeof(AddData));

	if(!psData)
		return CL_OUT_OF_RESOURCES;

	/* Initialise data */
	psInstance->pvPrivateData = (void*)psData;

	/* Initialise host side objects */

	/* Allocate first host buffer */
	psData->pui32BufferA = (unsigned int*)malloc(sizeof(unsigned int)*TEST_ADD_MAX_INSTANCES);

	if(!psData->pui32BufferA)
	{
		free(psData);
		return CL_OUT_OF_RESOURCES;
	}

	/* Allocate second host buffer */
	psData->pui32BufferB = (unsigned int*)malloc(sizeof(unsigned int)*TEST_ADD_MAX_INSTANCES);

	if(!psData->pui32BufferB)
	{
		free(psData->pui32BufferA);
		free(psData);
		return CL_OUT_OF_RESOURCES;
	}

	/* Initialise with test values */
	{
		unsigned int i;

		for(i=0; i < TEST_ADD_MAX_INSTANCES; i++)
		{
			psData->pui32BufferA[i] = rand();
			psData->pui32BufferB[i] = rand();
		}
	}

	eResult = clGetPlatformIDs(1,&psData->psPlatformID,NULL);
	CheckAndReportError(psInstance, "clGetPlatformIDs", eResult, init_add_cleanup);

	eResult = clGetDeviceIDs(psData->psPlatformID,CL_DEVICE_TYPE_GPU | CL_DEVICE_TYPE_ACCELERATOR,1,&psData->psDeviceID,NULL);
	CheckAndReportError(psInstance, "clGetDeviceIDs", eResult, init_add_cleanup);

	psData->psContext = clCreateContext(NULL,1,&psData->psDeviceID,NULL,NULL,&eResult);
	CheckAndReportError(psInstance, "clCreateContext", eResult, init_add_cleanup);

	/* Use the global add program */
	auLengths[0]  = strlen(g_pszAddSource);
	ppszSource[0] = g_pszAddSource;

	/*
	 * Build the Program
	 */

	psData->psProgram = clCreateProgramWithSource(psData->psContext, 1, (const char**)ppszSource, auLengths, &eResult);
	CheckAndReportError(psInstance, "clCreateProgramWithSource", eResult, init_add_cleanup);

	eResult = clBuildProgram(psData->psProgram,1,&psData->psDeviceID,"",NULL,NULL);
	CheckAndReportError(psInstance, "clBuildProgram", eResult, init_add_cleanup);

	psData->psCommandQueue = clCreateCommandQueue(psData->psContext, psData->psDeviceID, 0, &eResult);
	CheckAndReportError(psInstance, "clCreateCommandQueue", eResult, init_add_cleanup);

	/*
	 * Setup the Arguments
	 */

	psData->psBufferA = clCreateBuffer(psData->psContext, CL_MEM_READ_WRITE, sizeof(cl_uint) * TEST_ADD_MAX_INSTANCES, NULL, &eResult);
	CheckAndReportError(psInstance, "clCreateBuffer", eResult, init_add_cleanup);

	psData->psBufferB = clCreateBuffer(psData->psContext, CL_MEM_COPY_HOST_PTR, sizeof(cl_uint) * TEST_ADD_MAX_INSTANCES, psData->pui32BufferB, &eResult);
	CheckAndReportError(psInstance, "clCreateBuffer", eResult, init_add_cleanup);

	psData->psKernel = clCreateKernel(psData->psProgram, "AdditionKernel", &eResult);
	CheckAndReportError(psInstance, "clCreateKernel", eResult, init_add_cleanup);

	eResult = clSetKernelArg(psData->psKernel, 0, sizeof(cl_mem), (void*) &psData->psBufferA );
	CheckAndReportError(psInstance, "clSetKernelArg", eResult, init_add_cleanup);

	eResult = clSetKernelArg(psData->psKernel, 1, sizeof(cl_mem), (void*) &psData->psBufferB );
	CheckAndReportError(psInstance, "clSetKernelArg", eResult, init_add_cleanup);

	/* Setup the size of the kernel run */
	psData->puGlobalWorkSize[0] = TEST_ADD_MAX_INSTANCES;
	psData->puLocalWorkSize = NULL;

	return CL_SUCCESS;

init_add_cleanup:
	return eResult;
}

/***********************************************************************************
 Function Name      : Compute_Add
 Inputs             : psInstance - test instance data
 Returns            : 0 or 1
 Description        : Main compute func
************************************************************************************/
cl_int Compute_Add(OCLTestInstance *psInstance)
{
	cl_int eResult;
	cl_event writeBufferEvent;
	AddData *psData =(AddData*) psInstance->pvPrivateData;

	OCLTestLog("%s: Online compilation test with %zu instances running source:\n<source>\n%s\n</source>\n",
	       __func__,psData->puGlobalWorkSize[0],g_pszAddSource);

	eResult = clEnqueueWriteBuffer(psData->psCommandQueue, psData->psBufferA, CL_FALSE, 0, sizeof(cl_uint) * TEST_ADD_MAX_INSTANCES, psData->pui32BufferA, 0, NULL, &writeBufferEvent);
	CheckAndReportError(psInstance, "clEnqueueWriteBuffer", eResult, compute_add_cleanup);

	eResult = clEnqueueNDRangeKernel(psData->psCommandQueue, psData->psKernel, 1, NULL, psData->puGlobalWorkSize, psData->puLocalWorkSize , 1, &writeBufferEvent, NULL);
	CheckAndReportError(psInstance, "clEnqueueNDRangeKernel", eResult, compute_add_cleanup);

	clFinish(psData->psCommandQueue);

	clReleaseEvent(writeBufferEvent);

	return CL_SUCCESS;

compute_add_cleanup:
	return eResult;
}

/***********************************************************************************
 Function Name      : Verify_Add
 Inputs             : None
 Outputs            : None
 Returns            : None
 Description        : Verifies output data
************************************************************************************/

cl_int Verify_Add(OCLTestInstance *psInstance)
{
#if !defined(NO_HARDWARE)
	unsigned int t;
#endif /* !defined(NO_HARDWARE) */

	unsigned int *pui32Results = NULL;
	cl_int eResult = CL_SUCCESS;

	AddData *psData =(AddData*) psInstance->pvPrivateData;
	psData = psData;

	/* Allocate temporary buffer for reading back results */
	pui32Results = (unsigned int*)malloc(sizeof(unsigned int)*TEST_ADD_MAX_INSTANCES);

	if(!pui32Results)
	{
		free(psInstance->pvPrivateData);
		psInstance->pvPrivateData = strdup("Results buffer allocation failure");
		return CL_OUT_OF_HOST_MEMORY;
	}

	/* Read back the results */
	eResult = clEnqueueReadBuffer(psData->psCommandQueue, psData->psBufferA, CL_TRUE, 0, sizeof(cl_uint) * TEST_ADD_MAX_INSTANCES, pui32Results, 0, NULL, NULL);
	CheckAndReportError(psInstance, "clEnqueueReadBuffer", eResult, verify_add_cleanup);

#if !defined(NO_HARDWARE)
	/* Verify the written back data with what we would expect */
	for(t=0; t < TEST_ADD_MAX_INSTANCES; t++)
	{
		if( pui32Results[t] != (psData->pui32BufferA[t] + psData->pui32BufferB[t]))
		{
			OCLTestLog("%s: Verification failure at %d, expected (%d,%d)=%d got %d.\n",
			       __func__,
			       t,
			       psData->pui32BufferA[t],
			       psData->pui32BufferB[t],
			       psData->pui32BufferA[t]+psData->pui32BufferB[t],
			       pui32Results[t]);

			/* Verification Error */
			free(psInstance->pvPrivateData);
			psInstance->pvPrivateData = strdup("Verification failure");
			return CL_INVALID_VALUE;
		}
	}

	OCLTestLog("%s: Verification OK\n",__func__);
#endif /* !defined(NO_HARDWARE) */

	/* Cleanup */
	clReleaseKernel      ( psData->psKernel       );
	clReleaseMemObject   ( psData->psBufferA      );
	clReleaseMemObject   ( psData->psBufferB      );
	clReleaseCommandQueue( psData->psCommandQueue );
	clReleaseProgram     ( psData->psProgram      );
	clReleaseContext     ( psData->psContext      );

verify_add_cleanup:
	free(psData->pui32BufferA);
	free(psData->pui32BufferB);
	free(pui32Results);

	free(psInstance->pvPrivateData);
	psInstance->pvPrivateData = NULL;
	return eResult;
}

/******************************************************************************
 End of file (add.c)
******************************************************************************/
