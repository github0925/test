/*************************************************************************/ /*!
@File           add.c
@Copyright      Copyright (c) Imagination Technologies Ltd. All Rights Reserved
@License        Strictly Confidential.
*/ /**************************************************************************/

typedef struct _NOPData_
{
	/* CL Objects */
	cl_context       	psContext;
	cl_program       	psProgram;
	cl_device_id     	psDeviceID;
	cl_platform_id   	psPlatformID;
	cl_command_queue	psCommandQueue;
	cl_kernel        	psKernel;
	size_t        	 	puGlobalWorkSize[3];
	size_t*       		puLocalWorkSize;

} NOPData;

cl_int Init_NOP   	(OCLTestInstance *psInstance);
cl_int Compute_NOP	(OCLTestInstance *psInstance);
cl_int Verify_NOP	(OCLTestInstance *psInstance);

#ifdef CUT_DOWN_UNIT_TEST
#define TEST_NOP_MAX_INSTANCES 4
#else
#define TEST_NOP_MAX_INSTANCES 1024
#endif

static char *g_pszNOPSource =
{
	"__kernel void NOPKernel()\n"
	"{\n"
	"}"
};

/***********************************************************************************
 Function Name      : Init_NOP
 Inputs             : None
 Outputs            : None
 Returns            : None
 Description        : Initialises input data
************************************************************************************/
cl_int Init_NOP(OCLTestInstance *psInstance)
{
	size_t auLengths[2] = {0,0};
	char *ppszSource[2] = { 0 };

	cl_int eResult;

	NOPData *psData = (NOPData*)calloc(1, sizeof(NOPData));

	if (!psData)
		return CL_OUT_OF_RESOURCES;

	/* Initialise data */
	psInstance->pvPrivateData = (void*)psData;

	eResult = clGetPlatformIDs(1,&psData->psPlatformID,NULL);
	CheckAndReportError(psInstance, "clGetPlatformIDs", eResult, init_NOP_cleanup);

	eResult = clGetDeviceIDs(psData->psPlatformID, CL_DEVICE_TYPE_GPU, 1, &psData->psDeviceID, NULL);
	CheckAndReportError(psInstance, "clGetDeviceIDs", eResult, init_NOP_cleanup);

	psData->psContext = clCreateContext(NULL, 1, &psData->psDeviceID, NULL, NULL, &eResult);
	CheckAndReportError(psInstance, "clCreateContext", eResult, init_NOP_cleanup);

	auLengths[0]  = strlen(g_pszNOPSource);
	ppszSource[0] = g_pszNOPSource;

	/*
	 * Build the Program
	 */
	psData->psProgram = clCreateProgramWithSource(psData->psContext, 1, (const char**)ppszSource, auLengths, &eResult);
	CheckAndReportError(psInstance, "clCreateProgramWithSource", eResult, init_NOP_cleanup);

	eResult = clBuildProgram(psData->psProgram,1,&psData->psDeviceID,"",NULL,NULL);
	CheckAndReportError(psInstance, "clBuildProgram", eResult, init_NOP_cleanup);

	psData->psCommandQueue = clCreateCommandQueue(psData->psContext, psData->psDeviceID, 0, &eResult);
	CheckAndReportError(psInstance, "clCreateCommandQueue", eResult, init_NOP_cleanup);

	psData->psKernel = clCreateKernel(psData->psProgram, "NOPKernel", &eResult);
	CheckAndReportError(psInstance, "clCreateKernel", eResult, init_NOP_cleanup);

	/* Setup the size of the kernel run */
	psData->puGlobalWorkSize[0] = TEST_NOP_MAX_INSTANCES;
	psData->puLocalWorkSize = NULL;

	return CL_SUCCESS;

init_NOP_cleanup:
	return eResult;
}

/***********************************************************************************
 Function Name      : Compute_NOP
 Inputs             : psInstance - test instance data
 Returns            : 0 or 1
 Description        : Main compute function
************************************************************************************/
cl_int Compute_NOP(OCLTestInstance *psInstance)
{
	cl_int eResult;
	NOPData *psData =(NOPData*) psInstance->pvPrivateData;

	OCLTestLog("%s: Online compilation test with %zu instances running source:\n<source>\n%s\n</source>\n",
			   __func__,
			   psData->puGlobalWorkSize[0],
			   g_pszNOPSource);

	eResult = clEnqueueNDRangeKernel(psData->psCommandQueue,
									 psData->psKernel,
									 1,
									 NULL,
									 psData->puGlobalWorkSize,
									 psData->puLocalWorkSize,
									 0,
									 NULL,
									 NULL);
	CheckAndReportError(psInstance, "clEnqueueNDRangeKernel", eResult, compute_NOP_cleanup);

	clFinish(psData->psCommandQueue);

	return CL_SUCCESS;

compute_NOP_cleanup:
	return eResult;
}

/***********************************************************************************
 Function Name      : Verify_NOP
 Inputs             : None
 Outputs            : None
 Returns            : None
 Description        : Just cleanup and return success
************************************************************************************/
cl_int Verify_NOP(OCLTestInstance *psInstance)
{
	NOPData	*psData	= (NOPData*)psInstance->pvPrivateData;
	cl_int	eResult	= CL_SUCCESS;

	/* Cleanup */
	eResult = clReleaseKernel(psData->psKernel);
	CheckAndReportError(psInstance, "clReleaseKernel", eResult, verify_NOP_cleanup);

	eResult = clReleaseCommandQueue(psData->psCommandQueue);
	CheckAndReportError(psInstance, "clReleaseCommandQueue", eResult, verify_NOP_cleanup);

	eResult = clReleaseProgram(psData->psProgram);
	CheckAndReportError(psInstance, "clReleaseProgram", eResult, verify_NOP_cleanup);

	eResult = clReleaseContext(psData->psContext);
	CheckAndReportError(psInstance, "clReleaseContext", eResult, verify_NOP_cleanup);

verify_NOP_cleanup:
	return eResult;
}
/******************************************************************************
 End of file (add.c)
******************************************************************************/
