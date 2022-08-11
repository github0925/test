/*************************************************************************/ /*!
@File           binary.c
@Copyright      Copyright (c) Imagination Technologies Ltd. All Rights Reserved
@License        Strictly Confidential.
*/ /**************************************************************************/
#define TEST_BINARY_MAX_INSTANCES       4096

typedef struct _BinaryData_
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
	size_t        puGlobalWorkSize[3];
	size_t*       puLocalWorkSize;
	unsigned int  ui32NumBinaries;

} BinaryData;

cl_int Init_Binary(OCLTestInstance *psInstance);
cl_int Compute_Binary(OCLTestInstance *psInstance);
cl_int Verify_Binary(OCLTestInstance *psInstance);

static char *g_pszBinarySource =
{
	"__kernel void MemoryCopy(__global unsigned int* a, __global unsigned int* b)\n"
	"{\n"
	"\tint ith = get_global_id(0);\n"
	"\ta[ith] = b[ith];\n"
	"}"
};

/***********************************************************************************
 Function Name      : Init_Binary
 Inputs             : None
 Outputs            : None
 Returns            : None
 Description        : Initialises input data
************************************************************************************/
cl_int Init_Binary(OCLTestInstance *psInstance)
{
	size_t auLengths[2] = {0,0};
	char *ppszSource[2] = { 0 };
	unsigned int i;
	cl_int       eResult = CL_SUCCESS;
	char         aszBuildLog[512];
	size_t       uBuildLogSize;

	BinaryData *psData = (BinaryData*)calloc(1, sizeof(BinaryData));

	if(!psData)
	{
		return CL_OUT_OF_RESOURCES;
	}

	/* Initialise data */
	psInstance->pvPrivateData = (void*)psData;

	/* Allocate first host buffer */
	psData->pui32Buffer = (unsigned int*)malloc(sizeof(unsigned int)*TEST_BINARY_MAX_INSTANCES);

	if(!psData->pui32Buffer)
	{
		eResult = CL_OUT_OF_RESOURCES;
		goto init_binary_cleanup;
	}

	/* Initialise with sequential test values */
	for(i=0; i < TEST_BINARY_MAX_INSTANCES; i++)
	{
		psData->pui32Buffer[i] = i * 2;
	}

	eResult = clGetPlatformIDs(1,&psData->psPlatformID,NULL);
	CheckAndReportError(psInstance, "clGetPlatformIDs", eResult, init_binary_cleanup);

	eResult = clGetDeviceIDs(psData->psPlatformID,CL_DEVICE_TYPE_GPU,1,&psData->psDeviceID,NULL);
	CheckAndReportError(psInstance, "clGetDeviceIDs", eResult, init_binary_cleanup);

	psData->psContext = clCreateContext(NULL,1,&psData->psDeviceID,NULL,NULL,&eResult);
	CheckAndReportError(psInstance, "clCreateContext", eResult, init_binary_cleanup);

	/* Use the global source program */
	auLengths[0]  = strlen(g_pszBinarySource);
	ppszSource[0] = g_pszBinarySource;

	/*
	 * Build the Program
	 */

	psData->psProgram = clCreateProgramWithSource(psData->psContext, 1, (const char**)ppszSource, auLengths, &eResult);
	CheckAndReportError(psInstance, "clCreateProgramWithSource", eResult, init_binary_cleanup);

	eResult = clBuildProgram(psData->psProgram,1,&psData->psDeviceID,"",NULL,NULL);
	CheckAndReportError(psInstance, "clBuildProgram", eResult, init_binary_cleanup);

	eResult = clGetProgramBuildInfo(psData->psProgram, psData->psDeviceID, CL_PROGRAM_BUILD_LOG, 512, aszBuildLog, &uBuildLogSize);
	CheckAndReportError(psInstance, "clGetProgramBuildInfo", eResult, init_binary_cleanup);

	aszBuildLog[uBuildLogSize-1] = '\0';
	OCLTestLog("CL_PROGRAM_BUILD_LOG:\n%s\n",aszBuildLog);

	/*
	 * Output the compiled binaries to file
	 */
	if(!OCLOutputBinariesToFile(psData->psProgram,"binary_",&psData->ui32NumBinaries))
	{
		OCLTestLog("%s:%s:%d: unable to output binaries to file, binary test cannot continue\n",
		        psInstance->pszTestID ,__func__,__LINE__);
		eResult = CL_OUT_OF_RESOURCES;
		goto init_binary_cleanup;
	}

	OCLTestLog("%s: Wrote out %d binaries to the file system\n",__func__,psData->ui32NumBinaries);

	/* Free the context (and underlying objects) to ensure binaries are loaded free of any information */
	clReleaseProgram     ( psData->psProgram      );
	clReleaseContext     ( psData->psContext      );

	return CL_SUCCESS;

init_binary_cleanup:
	free(psData->pui32Buffer);
	free(psData);
	psInstance->pvPrivateData = NULL;

	return eResult;
}

/***********************************************************************************
 Function Name      : Compute_Binary
 Inputs             : psInstance - test instance data
 Returns            : 0 or 1
 Description        : Main compute func
************************************************************************************/
cl_int Compute_Binary(OCLTestInstance *psInstance)
{
	size_t         uSize       = 0;
	unsigned char *pui32Binary = NULL;

	cl_int eResult;
	BinaryData *psData =(BinaryData*) psInstance->pvPrivateData;

	/* Recreate context and necessary buffers, using the binaries only */

	eResult = clGetPlatformIDs(1,&psData->psPlatformID,NULL);
	CheckAndReportError(psInstance, "clGetPlatformIDs", eResult, compute_binary_cleanup);

	eResult = clGetDeviceIDs(psData->psPlatformID,CL_DEVICE_TYPE_GPU,1,&psData->psDeviceID,NULL);
	CheckAndReportError(psInstance, "clGetDeviceIDs", eResult, compute_binary_cleanup);

	psData->psContext = clCreateContext(NULL,1,&psData->psDeviceID,NULL,NULL,&eResult);
	CheckAndReportError(psInstance, "clCreateContext", eResult, compute_binary_cleanup);

	psData->psCommandQueue = clCreateCommandQueue(psData->psContext, psData->psDeviceID, CL_QUEUE_PROFILING_ENABLE, &eResult);
	CheckAndReportError(psInstance, "clCreateCommandQueue", eResult, compute_binary_cleanup);

	/* Grab the binaries and create a program from them */
	/* We only allow binary to be tested */
	if(psData->ui32NumBinaries > 1 )
	{
		OCLTestLog("%s: Warning - currently will only test one binary, but there are %d available",
		       __func__,
		       psData->ui32NumBinaries);
	}

	pui32Binary = OCLLoadBinary("binary_0.bin",&uSize);

	if(!pui32Binary)
	{
		OCLTestLog("%s: Unable to read binary_0.bin into memory, cannot continue test\n",__func__);
		return CL_OUT_OF_HOST_MEMORY;
	}

	{
		const size_t lengths[2] = { uSize, 0 };
		const unsigned char* binaries[2] = { pui32Binary, NULL };

		psData->psProgram = clCreateProgramWithBinary(psData->psContext,1, &psData->psDeviceID,lengths,binaries,NULL,&eResult);
		CheckAndReportError(psInstance, "clCreateProgramWithBinary", eResult, compute_binary_cleanup);
	}

	OCLTestLog("%s: Successfully loaded back binary_0.bin from filesystem\n",__func__);

	/* Remove the file */
	unlink("binary_0.bin");

	eResult = clBuildProgram(psData->psProgram,1,&psData->psDeviceID,"",NULL,NULL);
	CheckAndReportError(psInstance, "clBuildProgram", eResult, compute_binary_cleanup);

	/* Free binary */
	free(pui32Binary);

	/*
	 * Setup the Arguments
	 */

	psData->psBufferA = clCreateBuffer(psData->psContext, 0, sizeof(cl_uint) * TEST_BINARY_MAX_INSTANCES, NULL, &eResult);
	CheckAndReportError(psInstance, "clCreateBuffer", eResult, compute_binary_cleanup);

	psData->psBufferB = clCreateBuffer(psData->psContext, CL_MEM_COPY_HOST_PTR, sizeof(cl_uint) * TEST_BINARY_MAX_INSTANCES, psData->pui32Buffer, &eResult);
	CheckAndReportError(psInstance, "clCreateBuffer", eResult, compute_binary_cleanup);

	psData->psKernel = clCreateKernel(psData->psProgram, "MemoryCopy", &eResult);
	CheckAndReportError(psInstance, "clCreateKernel", eResult, compute_binary_cleanup);

	eResult = clSetKernelArg(psData->psKernel, 0, sizeof(cl_mem), (void*) &psData->psBufferA );
	CheckAndReportError(psInstance, "clSetKernelArg", eResult, compute_binary_cleanup);

	eResult = clSetKernelArg(psData->psKernel, 1, sizeof(cl_mem), (void*) &psData->psBufferB );
	CheckAndReportError(psInstance, "clSetKernelArg", eResult, compute_binary_cleanup);

	/* Setup the size of the kernel run */
	psData->puGlobalWorkSize[0] = TEST_BINARY_MAX_INSTANCES;
	psData->puLocalWorkSize = NULL;

	return CL_SUCCESS;

compute_binary_cleanup:
	return eResult;
}

/***********************************************************************************
 Function Name      : Verify_Binary
 Inputs             : None
 Outputs            : None
 Returns            : None
 Description        : Verifies output data
************************************************************************************/
cl_int Verify_Binary(OCLTestInstance *psInstance)
{
#if !defined(NO_HARDWARE)
	unsigned int i;
#endif /* !defined(NO_HARDWARE) */

	int eResult = CL_SUCCESS;
	unsigned int *pui32Results = NULL;

	BinaryData *psData =(BinaryData*) psInstance->pvPrivateData;
	//psData = psData;

	/* Allocate temporary buffer for reading back results */
	pui32Results = (unsigned int*)malloc(sizeof(unsigned int)*TEST_BINARY_MAX_INSTANCES);

	if(!pui32Results)
	{
		free(psInstance->pvPrivateData);
		psInstance->pvPrivateData = strdup("Results buffer allocation failure");
		return CL_OUT_OF_RESOURCES;
	}

	/* Enqueue a kernel to copy data from B --> A */
	eResult = clEnqueueNDRangeKernel(psData->psCommandQueue, psData->psKernel, 1, NULL, psData->puGlobalWorkSize, psData->puLocalWorkSize , 0, NULL, NULL);
	CheckAndReportError(psInstance, "clEnqueueNDRangeKernel", eResult, verify_binary_cleanup);

	clFinish(psData->psCommandQueue);

	/* Read back the results */
	eResult = clEnqueueReadBuffer(psData->psCommandQueue, psData->psBufferA, CL_TRUE, 0, sizeof(cl_uint) * psData->puGlobalWorkSize[0], pui32Results, 0, NULL, NULL);
	CheckAndReportError(psInstance, "clEnqueueReadBuffer", eResult, verify_binary_cleanup);

#if !defined(NO_HARDWARE)
	/* Verify the written back data with what we would expect */
	for(i=0; i < psData->puGlobalWorkSize[0]; i++)
	{
		if( pui32Results[i] != psData->pui32Buffer[i])
		{
			OCLTestLog("%s: Verification failure at %d, expected %08x got %08x.\n",
			       __func__,
			       i,
			       psData->pui32Buffer[i],
			       pui32Results[i]);

			/* Verification Error */
			free(psInstance->pvPrivateData);
			psInstance->pvPrivateData = strdup("Verification failure");
			return CL_INVALID_VALUE;
		}
	}

	OCLTestLog("%s: Binary file successfully executed kernel\n",__func__);
#endif /* defined(NO_HARDWARE) */

verify_binary_cleanup:
	/* Cleanup */
	if(psData->psKernel)		clReleaseKernel      ( psData->psKernel       );
	if(psData->psBufferA)		clReleaseMemObject   ( psData->psBufferA      );
	if(psData->psBufferB)		clReleaseMemObject   ( psData->psBufferB      );
	if(psData->psCommandQueue)	clReleaseCommandQueue( psData->psCommandQueue );
	if(psData->psProgram)		clReleaseProgram     ( psData->psProgram      );
	if(psData->psContext)		clReleaseContext     ( psData->psContext      );

	free(psData->pui32Buffer);
	free(pui32Results);

	free(psInstance->pvPrivateData);
	psInstance->pvPrivateData = NULL;

	return eResult;

	return eResult;
}

/******************************************************************************
 End of file (binary.c)
******************************************************************************/
