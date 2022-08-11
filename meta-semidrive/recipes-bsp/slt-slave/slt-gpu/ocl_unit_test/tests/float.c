/*************************************************************************/ /*!
@File           float.c
@Copyright      Copyright (c) Imagination Technologies Ltd. All Rights Reserved
@License        Strictly Confidential.
*/ /**************************************************************************/

#include <unistd.h>
#include <sys/time.h>

typedef struct _FloatOpsData_
{
	/* CL Objects */
	cl_context       psContext;
	cl_program       psProgram;
	cl_device_id     psDeviceID;
	cl_platform_id   psPlatformID;
	cl_command_queue psCommandQueue;

	cl_kernel        psAddKernel;
	cl_kernel        psMulKernel;
	cl_kernel        psMadKernel;
	cl_kernel        psDivKernel;

	cl_mem           psBufferA;

	cl_event         psAddCompleteEvent;
	cl_event         psMulCompleteEvent;
	cl_event         psMadCompleteEvent;
	cl_event         psDivCompleteEvent;

	/* Host Object */
	unsigned int *pui32BufferA;
	size_t        puGlobalWorkSize[3];
	size_t*       puLocalWorkSize;
	int           computemode;



} FloatOpsData;

#define TOSTR(x) #x
#define STR(x) TOSTR(x)
#ifdef CUT_DOWN_UNIT_TEST
#define TEST_MAD_MAX_INSTANCES  (1024)
#define TEST_MAD_NUM_ITERATIONS 30
#else
#define TEST_MAD_MAX_INSTANCES  (262144)
#define TEST_MAD_NUM_ITERATIONS 150
#endif

static char *g_pszMadSource =
{
"#define TEST_MAD_NUM_ITERATIONS " STR(TEST_MAD_NUM_ITERATIONS) \
"\n"
"__kernel void computeFloatAdd(float in,\n"
"				  __global float *output)\n"
"{\n"
"	// 3 floating point ops\n"
"	float a = 1.0f + in;\n"
"	float b = 2.0f + in;\n"
"	float c = 3.0f + in;\n"
"	float d = 4.0f + in;\n"
"	float e = 5.0f + in;\n"
"\n"
"	for (unsigned short i = 0; i < TEST_MAD_NUM_ITERATIONS; ++i)\n"
"	{\n"
"		// 5 + (TEST_MAD_NUM_ITERATIONS * 18) floating point ops\n"
"		a = b + a;\n"
"		c = c + e;\n"
"		a = c + a + d;\n"
"		b = c + b + e;\n"
"		a = c + a + e;\n"
"		b = c + b + d;\n"
"		a = c + a + d;\n"
"		b = c + b + e;\n"
"		a = c + a + e;\n"
"		b = c + b + d;\n"
"		// 5 + (TEST_MAD_NUM_ITERATIONS * 38) floating point ops\n"
"		a = c + a + d;\n"
"		b = c + b + e;\n"
"		a = c + a + e;\n"
"		b = c + b + d;\n"
"		a = c + a + d;\n"
"		b = c + b + e;\n"
"		a = c + a + e;\n"
"		b = c + b + d;\n"
"		a = c + a + d;\n"
"		b = c + b + e;\n"
"		// 5 + (TEST_MAD_NUM_ITERATIONS * 58) floating point ops\n"
"		a = c + a + e;\n"
"		b = c + b + d;\n"
"		a = c + a + d;\n"
"		b = c + b + e;\n"
"		a = c + a + e;\n"
"		b = c + b + d;\n"
"		a = c + a + d;\n"
"		b = c + b + e;\n"
"		a = c + a + e;\n"
"		b = c + b + d;\n"
"		// 5 + (TEST_MAD_NUM_ITERATIONS * 78) floating point ops\n"
"		a = c + a + d;\n"
"		b = c + b + e;\n"
"		a = c + a + e;\n"
"		b = c + b + d;\n"
"		a = c + a + d;\n"
"		b = c + b + e;\n"
"		a = c + a + e;\n"
"		b = c + b + d;\n"
"		a = c + a + d;\n"
"		b = c + b + e;\n"
"		// 5 + (TEST_MAD_NUM_ITERATIONS * 98) floating point ops\n"
"		a = c + a + e;\n"
"		b = c + b + d;\n"
"		a = c + a + d;\n"
"		b = c + b + e;\n"
"		a = c + a + e;\n"
"		b = c + b + d;\n"
"		a = c + a + d;\n"
"		b = c + b + e;\n"
"		a = c + a + e;\n"
"		b = c + b + d;\n"
"	}\n"

"\n"
"	// 6 + (TEST_MAD_NUM_ITERATIONS * 98) floating point ops\n"
"	*output = a + b;\n"
"}\n"
"\n"
"__kernel void computeFloatMul(float in,\n"
"				  __global float *output)\n"
"{\n"
"	// 3 floating point ops\n"
"	float a = 2.0f * in;\n"
"	float b = 3.0f * in;\n"
"	float c = 4.0f * in;\n"
"\n"
"	for (unsigned short i = 0; i < TEST_MAD_NUM_ITERATIONS; ++i)\n"
"	{\n"
"		// 3 * (TEST_MAD_NUM_ITERATIONS * 18) floating point ops\n"
"		a = c * a;\n"
"		b = c * b;\n"
"		a = c * a * a;\n"
"		b = c * b * b;\n"
"		a = c * a * a;\n"
"		b = c * b * b;\n"
"		a = c * a * a;\n"
"		b = c * b * b;\n"
"		a = c * a * a;\n"
"		b = c * b * b;\n"
"		// 3 + (TEST_MAD_NUM_ITERATIONS * 38) floating point ops\n"
"		a = c * a * a;\n"
"		b = c * b * b;\n"
"		a = c * a * a;\n"
"		b = c * b * b;\n"
"		a = c * a * a;\n"
"		b = c * b * b;\n"
"		a = c * a * a;\n"
"		b = c * b * b;\n"
"		a = c * a * a;\n"
"		b = c * b * b;\n"
"		// 3 + (TEST_MAD_NUM_ITERATIONS * 58) floating point ops\n"
"		a = c * a * a;\n"
"		b = c * b * b;\n"
"		a = c * a * a;\n"
"		b = c * b * b;\n"
"		a = c * a * a;\n"
"		b = c * b * b;\n"
"		a = c * a * a;\n"
"		b = c * b * b;\n"
"		a = c * a * a;\n"
"		b = c * b * b;\n"
"		// 3 + (TEST_MAD_NUM_ITERATIONS * 78) floating point ops\n"
"		a = c * a * a;\n"
"		b = c * b * b;\n"
"		a = c * a * a;\n"
"		b = c * b * b;\n"
"		a = c * a * a;\n"
"		b = c * b * b;\n"
"		a = c * a * a;\n"
"		b = c * b * b;\n"
"		a = c * a * a;\n"
"		b = c * b * b;\n"
"		// 3 + (TEST_MAD_NUM_ITERATIONS * 98) floating point ops\n"
"		a = c * a * a;\n"
"		b = c * b * b;\n"
"		a = c * a * a;\n"
"		b = c * b * b;\n"
"		a = c * a * a;\n"
"		b = c * b * b;\n"
"		a = c * a * a;\n"
"		b = c * b * b;\n"
"		a = c * a * a;\n"
"		b = c * b * b;\n"
"	}\n"
"\n"
"	// 4 + (TEST_MAD_NUM_ITERATIONS * 98) floating point ops\n"
"	*output = a * b;\n"
"}\n"
"\n"
"__kernel void computeFloatMad(float in,\n"
"				  __global float *output)\n"
"{\n"
"	// 3 floating point ops\n"
"	float a = 1.0f * in;\n"
"	float b = 2.0f * in;\n"
"	float c = 3.0f * in;\n"
"\n"
"	for (unsigned short i = 0; i < TEST_MAD_NUM_ITERATIONS; ++i)\n"
"	{\n"
"		// 3 + (TEST_MAD_NUM_ITERATIONS * 18) floating point ops\n"
"		a = c * a + b;\n"
"		b = c * b + a;\n"
"		a = c * a + a;\n"
"		b = c * b + b;\n"
"		a = c * a + a;\n"
"		b = c * b + b;\n"
"		a = c * a + a;\n"
"		b = c * b + b;\n"
"		a = c * a + a;\n"
"		b = c * b + b;\n"
"		// 3 + (TEST_MAD_NUM_ITERATIONS * 38) floating point ops\n"
"		a = c * a + a;\n"
"		b = c * b + b;\n"
"		a = c * a + a;\n"
"		b = c * b + b;\n"
"		a = c * a + a;\n"
"		b = c * b + b;\n"
"		a = c * a + a;\n"
"		b = c * b + b;\n"
"		a = c * a + a;\n"
"		b = c * b + b;\n"
"		// 3 + (TEST_MAD_NUM_ITERATIONS * 58) floating point ops\n"
"		a = c * a + a;\n"
"		b = c * b + b;\n"
"		a = c * a + a;\n"
"		b = c * b + b;\n"
"		a = c * a + a;\n"
"		b = c * b + b;\n"
"		a = c * a + a;\n"
"		b = c * b + b;\n"
"		a = c * a + a;\n"
"		b = c * b + b;\n"
"		// 3 + (TEST_MAD_NUM_ITERATIONS * 78) floating point ops\n"
"		a = c * a + a;\n"
"		b = c * b + b;\n"
"		a = c * a + a;\n"
"		b = c * b + b;\n"
"		a = c * a + a;\n"
"		b = c * b + b;\n"
"		a = c * a + a;\n"
"		b = c * b + b;\n"
"		a = c * a + a;\n"
"		b = c * b + b;\n"
"		// 3 + (TEST_MAD_NUM_ITERATIONS * 98) floating point ops\n"
"		a = c * a + a;\n"
"		b = c * b + b;\n"
"		a = c * a + a;\n"
"		b = c * b + b;\n"
"		a = c * a + a;\n"
"		b = c * b + b;\n"
"		a = c * a + a;\n"
"		b = c * b + b;\n"
"		a = c * a + a;\n"
"		b = c * b + b;\n"
"		a = c * a + a;\n"
"		b = c * b + b;\n"
"		a = c * a + a;\n"
"		b = c * b + b;\n"
"		a = c * a + a;\n"
"		b = c * b + b;\n"
"		a = c * a + a;\n"
"		b = c * b + b;\n"
"		// 3 + (TEST_MAD_NUM_ITERATIONS * 38) floating point ops\n"
"		a = c * a + a;\n"
"		b = c * b + b;\n"
"		a = c * a + a;\n"
"		b = c * b + b;\n"
"		a = c * a + a;\n"
"		b = c * b + b;\n"
"		a = c * a + a;\n"
"		b = c * b + b;\n"
"		a = c * a + a;\n"
"		b = c * b + b;\n"
"		// 3 + (TEST_MAD_NUM_ITERATIONS * 58) floating point ops\n"
"		a = c * a + a;\n"
"		b = c * b + b;\n"
"		a = c * a + a;\n"
"		b = c * b + b;\n"
"		a = c * a + a;\n"
"		b = c * b + b;\n"
"		a = c * a + a;\n"
"		b = c * b + b;\n"
"		a = c * a + a;\n"
"		b = c * b + b;\n"
"		// 3 + (TEST_MAD_NUM_ITERATIONS * 78) floating point ops\n"
"		a = c * a + a;\n"
"		b = c * b + b;\n"
"		a = c * a + a;\n"
"		b = c * b + b;\n"
"		a = c * a + a;\n"
"		b = c * b + b;\n"
"		a = c * a + a;\n"
"		b = c * b + b;\n"
"		a = c * a + a;\n"
"		b = c * b + b;\n"
"		// 3 + (TEST_MAD_NUM_ITERATIONS * 98) floating point ops\n"
"		a = c * a + a;\n"
"		b = c * b + b;\n"
"		a = c * a + a;\n"
"		b = c * b + b;\n"
"		a = c * a + a;\n"
"		b = c * b + b;\n"
"		a = c * a + a;\n"
"		b = c * b + b;\n"
"		a = c * a + a;\n"
"		b = c * b + b;\n"
"	}\n"
"\n"
"	// 4 + (TEST_MAD_NUM_ITERATIONS * 98) floating point ops\n"
"	*output = a + b;\n"
"}\n"
"\n"
"__kernel void computeFloatDiv(float in,\n"
"				  __global float *output)\n"
"{\n"
"	// 3 floating point ops\n"
"	float a = 1.0f / in;\n"
"	float b = 2.0f / in;\n"
"	float c = 3.0f / in;\n"
"\n"
"	for (unsigned short i = 0; i < TEST_MAD_NUM_ITERATIONS; ++i)\n"
"	{\n"
"		// 3 * (TEST_MAD_NUM_ITERATIONS * 18) floating point ops\n"
"		a = c / a;\n"
"		b = c / b;\n"
"		a = c / a / a;\n"
"		b = c / b / b;\n"
"		a = c / a / a;\n"
"		b = c / b / b;\n"
"		a = c / a / a;\n"
"		b = c / b / b;\n"
"		a = c / a / a;\n"
"		b = c / b / b;\n"
"		// 3 + (TEST_MAD_NUM_ITERATIONS * 38) floating point ops\n"
"		a = c / a / a;\n"
"		b = c / b / b;\n"
"		a = c / a / a;\n"
"		b = c / b / b;\n"
"		a = c / a / a;\n"
"		b = c / b / b;\n"
"		a = c / a / a;\n"
"		b = c / b / b;\n"
"		a = c / a / a;\n"
"		b = c / b / b;\n"
"		// 3 + (TEST_MAD_NUM_ITERATIONS * 58) floating point ops\n"
"		a = c / a / a;\n"
"		b = c / b / b;\n"
"		a = c / a / a;\n"
"		b = c / b / b;\n"
"		a = c / a / a;\n"
"		b = c / b / b;\n"
"		a = c / a / a;\n"
"		b = c / b / b;\n"
"		a = c / a / a;\n"
"		b = c / b / b;\n"
"		// 3 + (TEST_MAD_NUM_ITERATIONS * 78) floating point ops\n"
"		a = c / a / a;\n"
"		b = c / b / b;\n"
"		a = c / a / a;\n"
"		b = c / b / b;\n"
"		a = c / a / a;\n"
"		b = c / b / b;\n"
"		a = c / a / a;\n"
"		b = c / b / b;\n"
"		a = c / a / a;\n"
"		b = c / b / b;\n"
"		// 3 + (TEST_MAD_NUM_ITERATIONS * 98) floating point ops\n"
"		a = c / a / a;\n"
"		b = c / b / b;\n"
"		a = c / a / a;\n"
"		b = c / b / b;\n"
"		a = c / a / a;\n"
"		b = c / b / b;\n"
"		a = c / a / a;\n"
"		b = c / b / b;\n"
"		a = c / a / a;\n"
"		b = c / b / b;\n"
"	}\n"
"\n"
"	// 4 + (TEST_MAD_NUM_ITERATIONS * 98) floating point ops\n"
"	*output = a / b;\n"
"}\n"
};

/***********************************************************************************
 Function Name      : Init_Float
 Inputs             : None
 Outputs            : None
 Returns            : None
 Description        : Initialises input data
************************************************************************************/
static cl_int
Init_Float(OCLTestInstance *psInstance)
{
	size_t auLengths[2] = {0,0};
	char *ppszSource[2] = { 0 };
	float fValue = 1.0f;

	cl_int eResult;

	FloatOpsData *psData = (FloatOpsData*)calloc(1, sizeof(FloatOpsData));
	if (!psData)
	{
		return CL_OUT_OF_RESOURCES;
	}

	/* Initialise data */
	psInstance->pvPrivateData = (void*)psData;

	/* Allocate second host buffer */
	psData->pui32BufferA = (unsigned int*)malloc(sizeof(unsigned int));

	if(!psData->pui32BufferA)
	{
		free(psData);
		return CL_OUT_OF_RESOURCES;
	}

	/* Initialise with test values */
	psData->pui32BufferA[0] = 0;

	eResult = clGetPlatformIDs(1,&psData->psPlatformID,NULL);
	CheckAndReportError(psInstance, "clGetPlatformIDs", eResult, init_float_cleanup);

	eResult = clGetDeviceIDs(psData->psPlatformID,CL_DEVICE_TYPE_GPU,1,&psData->psDeviceID,NULL);
	CheckAndReportError(psInstance, "clGetDeviceIDs", eResult, init_float_cleanup);

	psData->psContext = clCreateContext(NULL,1,&psData->psDeviceID,NULL,NULL,&eResult);
	CheckAndReportError(psInstance, "clCreateContext", eResult, init_float_cleanup);

	/* Use the global mad program */
	auLengths[0]  = strlen(g_pszMadSource);
	ppszSource[0] = g_pszMadSource;

	/*
	 * Build the Program
	 */

	psData->psProgram = clCreateProgramWithSource(psData->psContext, 1, (const char**)ppszSource, auLengths, &eResult);
	CheckAndReportError(psInstance, "clCreateProgramWithSource", eResult, init_float_cleanup);

	eResult = clBuildProgram(psData->psProgram,1,&psData->psDeviceID,"-cl-fast-relaxed-math -cl-mad-enable",NULL,NULL);

	if(eResult != CL_SUCCESS)
	{
		char         aszBuildLog[512];
		size_t       uBuildLogSize;

		eResult = clGetProgramBuildInfo(psData->psProgram, psData->psDeviceID, CL_PROGRAM_BUILD_LOG, 512, aszBuildLog, &uBuildLogSize);
		CheckAndReportError(psInstance, "clGetProgramBuildInfo", eResult, init_float_cleanup);

		aszBuildLog[uBuildLogSize-1] = '\0';
		OCLTestLog("[%zu] CL_PROGRAM_BUILD_LOG:\n%s\n",uBuildLogSize,aszBuildLog);
	}
	CheckAndReportError(psInstance, "clBuildProgram", eResult, init_float_cleanup);

	psData->psCommandQueue = clCreateCommandQueue(psData->psContext, psData->psDeviceID, CL_QUEUE_PROFILING_ENABLE|CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE, &eResult);
	CheckAndReportError(psInstance, "clCreateCommandQueue", eResult, init_float_cleanup);

	/*
	 * Setup the Arguments
	 */
	psData->psBufferA = clCreateBuffer(psData->psContext, CL_MEM_COPY_HOST_PTR, sizeof(cl_float), psData->pui32BufferA, &eResult);
	CheckAndReportError(psInstance, "clCreateBuffer", eResult, init_float_cleanup);

	/* Init Add kernel */
	OCLTestLog("Compiling Add kernel...\n");
	psData->psAddKernel = clCreateKernel(psData->psProgram, "computeFloatAdd", &eResult);
	CheckAndReportError(psInstance, "clCreateKernel", eResult, init_float_cleanup);

	eResult = clSetKernelArg(psData->psAddKernel, 0, sizeof(cl_float), (void*) &fValue );
	CheckAndReportError(psInstance, "clSetKernelArg", eResult, init_float_cleanup);

	eResult = clSetKernelArg(psData->psAddKernel, 1, sizeof(cl_mem), (void*) &psData->psBufferA );
	CheckAndReportError(psInstance, "clSetKernelArg", eResult, init_float_cleanup);

	/* Init Mul kernel */
	OCLTestLog("Compiling Mul kernel...\n");
	psData->psMulKernel = clCreateKernel(psData->psProgram, "computeFloatMul", &eResult);
	CheckAndReportError(psInstance, "clCreateKernel", eResult, init_float_cleanup);

	eResult = clSetKernelArg(psData->psMulKernel, 0, sizeof(cl_float), (void*) &fValue );
	CheckAndReportError(psInstance, "clSetKernelArg", eResult, init_float_cleanup);

	eResult = clSetKernelArg(psData->psMulKernel, 1, sizeof(cl_mem), (void*) &psData->psBufferA );
	CheckAndReportError(psInstance, "clSetKernelArg", eResult, init_float_cleanup);

	/* Init Mad kernel */
	OCLTestLog("Compiling Mad kernel...\n");
	psData->psMadKernel = clCreateKernel(psData->psProgram, "computeFloatMad", &eResult);
	CheckAndReportError(psInstance, "clCreateKernel", eResult, init_float_cleanup);

	eResult = clSetKernelArg(psData->psMadKernel, 0, sizeof(cl_float), (void*) &fValue );
	CheckAndReportError(psInstance, "clSetKernelArg", eResult, init_float_cleanup);

	eResult = clSetKernelArg(psData->psMadKernel, 1, sizeof(cl_mem), (void*) &psData->psBufferA );
	CheckAndReportError(psInstance, "clSetKernelArg", eResult, init_float_cleanup);

	/* Init Div kernel */
	OCLTestLog("Compiling Div kernel...\n");
	psData->psDivKernel = clCreateKernel(psData->psProgram, "computeFloatDiv", &eResult);
	CheckAndReportError(psInstance, "clCreateKernel", eResult, init_float_cleanup);

	eResult = clSetKernelArg(psData->psDivKernel, 0, sizeof(cl_float), (void*) &fValue );
	CheckAndReportError(psInstance, "clSetKernelArg", eResult, init_float_cleanup);

	eResult = clSetKernelArg(psData->psDivKernel, 1, sizeof(cl_mem), (void*) &psData->psBufferA );
	CheckAndReportError(psInstance, "clSetKernelArg", eResult, init_float_cleanup);

	/* Setup the size of the kernel run */
	psData->puGlobalWorkSize[0] = TEST_MAD_MAX_INSTANCES;
	psData->puLocalWorkSize = NULL;

	return CL_SUCCESS;

init_float_cleanup:
	return eResult;
}

/***********************************************************************************
 Function Name      : Compute_Float
 Inputs             : psInstance - test instance data
 Returns            : 0 or 1
 Description        : Main compute func executes kernels in order
************************************************************************************/
static cl_int
Compute_Float(OCLTestInstance *psInstance)
{
	cl_int eResult;
	size_t global_dim[1];
	FloatOpsData *psData =(FloatOpsData*) psInstance->pvPrivateData;

	OCLTestLog("%s: Online compilation test with %zu instances, each with %d iterations\n",
	       __func__,psData->puGlobalWorkSize[0],TEST_MAD_NUM_ITERATIONS);

	global_dim[0] = psData->puGlobalWorkSize[0];

	// Enqueue and complete Add kernel
	OCLTestLog("Running Add kernel...\n");
	eResult = clEnqueueNDRangeKernel(psData->psCommandQueue, psData->psAddKernel, 1, NULL, global_dim, NULL , 0, NULL, &psData->psAddCompleteEvent);
	CheckAndReportError(psInstance, "clEnqueueNDRangeKernel", eResult, compute_float_cleanup);

	clFinish(psData->psCommandQueue);

	// Enqueue and complete Mul kernel
	OCLTestLog("Running Mul kernel...\n");
	eResult = clEnqueueNDRangeKernel(psData->psCommandQueue, psData->psMulKernel, 1, NULL, global_dim, NULL , 0, NULL, &psData->psMulCompleteEvent);
	CheckAndReportError(psInstance, "clEnqueueNDRangeKernel", eResult, compute_float_cleanup);

	clFinish(psData->psCommandQueue);

	// Enqueue and complete Mad kernel
	OCLTestLog("Running Mad kernel...\n");
	eResult = clEnqueueNDRangeKernel(psData->psCommandQueue, psData->psMadKernel, 1, NULL, global_dim, NULL , 0, NULL, &psData->psMadCompleteEvent);
	CheckAndReportError(psInstance, "clEnqueueNDRangeKernel", eResult, compute_float_cleanup);

	clFinish(psData->psCommandQueue);

	// Enqueue and complete Div kernel
	OCLTestLog("Running Div kernel...\n");
	eResult = clEnqueueNDRangeKernel(psData->psCommandQueue, psData->psDivKernel, 1, NULL, global_dim, NULL , 0, NULL, &psData->psDivCompleteEvent);
	CheckAndReportError(psInstance, "clEnqueueNDRangeKernel", eResult, compute_float_cleanup);

	clFinish(psData->psCommandQueue);

	return CL_SUCCESS;

compute_float_cleanup:
	return eResult;
}

/***********************************************************************************
 Function Name      : Compute_Float_Parallel
 Inputs             : psInstance - test instance data
 Returns            : 0 or 1
 Description        : Main compute func executes kernels in parallel
************************************************************************************/
static cl_int
Compute_Float_Parallel(OCLTestInstance *psInstance)
{
	cl_int eResult;
	size_t global_dim[1];
	FloatOpsData *psData =(FloatOpsData*) psInstance->pvPrivateData;
	/* Set computemode to one so the output metric knows which compute mode is running*/
	psData->computemode = 1;
	OCLTestLog("%s: Online compilation test with %zu instances, each with %d iterations\n",
	       __func__,psData->puGlobalWorkSize[0],TEST_MAD_NUM_ITERATIONS);

	global_dim[0] = psData->puGlobalWorkSize[0];

	// Enqueue and complete Add kernel
	OCLTestLog("Running Add kernel...\n");
	eResult = clEnqueueNDRangeKernel(psData->psCommandQueue, psData->psAddKernel, 1, NULL, global_dim, NULL , 0, NULL, &psData->psAddCompleteEvent);
	CheckAndReportError(psInstance, "clEnqueueNDRangeKernel", eResult, compute_float_parallel_cleanup);

	// Enqueue and complete Mul kernel
	OCLTestLog("Running Mul kernel...\n");
	eResult = clEnqueueNDRangeKernel(psData->psCommandQueue, psData->psMulKernel, 1, NULL, global_dim, NULL , 0, NULL, &psData->psMulCompleteEvent);
	CheckAndReportError(psInstance, "clEnqueueNDRangeKernel", eResult, compute_float_parallel_cleanup);

	// Enqueue and complete Mad kernel
	OCLTestLog("Running Mad kernel...\n");
	eResult = clEnqueueNDRangeKernel(psData->psCommandQueue, psData->psMadKernel, 1, NULL, global_dim, NULL , 0, NULL, &psData->psMadCompleteEvent);
	CheckAndReportError(psInstance, "clEnqueueNDRangeKernel", eResult, compute_float_parallel_cleanup);

	// Enqueue and complete Div kernel
	OCLTestLog("Running Div kernel...\n");
	eResult = clEnqueueNDRangeKernel(psData->psCommandQueue, psData->psDivKernel, 1, NULL, global_dim, NULL , 0, NULL, &psData->psDivCompleteEvent);
	CheckAndReportError(psInstance, "clEnqueueNDRangeKernel", eResult, compute_float_parallel_cleanup);

	clFinish(psData->psCommandQueue);

	return CL_SUCCESS;

compute_float_parallel_cleanup:
	return eResult;
}

/***********************************************************************************
 Function Name      : Compute_Float_Chain
 Inputs             : psInstance - test instance data
 Returns            : 0 or 1
 Description        : Main compute func executes kernels in events dependence order
************************************************************************************/
static cl_int
Compute_Float_Chain(OCLTestInstance *psInstance)
{
	cl_int eResult;
	size_t global_dim[1];
	cl_event eList[4];
	FloatOpsData *psData =(FloatOpsData*) psInstance->pvPrivateData;
	/* Set computemode to two so the output metric knows chain*/
	psData->computemode = 2;

	OCLTestLog("%s: Online compilation test with %zu instances, each with %d iterations\n",
	       __func__,psData->puGlobalWorkSize[0],TEST_MAD_NUM_ITERATIONS);

	global_dim[0] = psData->puGlobalWorkSize[0];

	// Enqueue and complete Add kernel
	OCLTestLog("Running Add kernel...\n");
	eResult = clEnqueueNDRangeKernel(psData->psCommandQueue, psData->psAddKernel, 1, NULL, global_dim, NULL , 0, NULL, &psData->psAddCompleteEvent);
	CheckAndReportError(psInstance, "clEnqueueNDRangeKernel", eResult, compute_float_chain_cleanup);

	// Enqueue and complete Mul kernel
	OCLTestLog("Running Mul kernel...\n");
	eList[0] = psData->psAddCompleteEvent;
	eResult = clEnqueueNDRangeKernel(psData->psCommandQueue, psData->psMulKernel, 1, NULL, global_dim, NULL , 1, eList, &psData->psMulCompleteEvent);
	CheckAndReportError(psInstance, "clEnqueueNDRangeKernel", eResult, compute_float_chain_cleanup);

	// Enqueue and complete Mad kernel
	OCLTestLog("Running Mad kernel...\n");
	eResult = clEnqueueNDRangeKernel(psData->psCommandQueue, psData->psMadKernel, 1, NULL, global_dim, NULL , 1, eList, &psData->psMadCompleteEvent);
	CheckAndReportError(psInstance, "clEnqueueNDRangeKernel", eResult, compute_float_chain_cleanup);

	// Enqueue and complete Div kernel
	OCLTestLog("Running Div kernel...\n");
	eList[0] = psData->psMulCompleteEvent;
	eList[1] = psData->psMadCompleteEvent;
	eResult = clEnqueueNDRangeKernel(psData->psCommandQueue, psData->psDivKernel, 1, NULL, global_dim, NULL , 2, eList, &psData->psDivCompleteEvent);
	CheckAndReportError(psInstance, "clEnqueueNDRangeKernel", eResult, compute_float_chain_cleanup);

	eList[0] = psData->psDivCompleteEvent;
	eList[1] = psData->psMulCompleteEvent;
	eList[2] = psData->psAddCompleteEvent;
	eList[3] = psData->psMadCompleteEvent;
	eResult = clWaitForEvents(4, eList);
	CheckAndReportError(psInstance, "clWaitForEvents", eResult, compute_float_chain_cleanup);

	return CL_SUCCESS;

compute_float_chain_cleanup:
	return eResult;
}

/***********************************************************************************
 Function Name      : Verify_Float
 Inputs             : None
 Outputs            : None
 Returns            : None
 Description        : Verifies output data
************************************************************************************/
static cl_int
Verify_Float(OCLTestInstance *psInstance)
{
	cl_int eResult = CL_SUCCESS;
	int i = 0;

	FloatOpsData *psData =(FloatOpsData*) psInstance->pvPrivateData;

	cl_event apsProfilingEvents[] = { psData->psAddCompleteEvent, psData->psMulCompleteEvent, psData->psMadCompleteEvent, psData->psDivCompleteEvent };
	char* apszTestNames[] = { "Float Add", "Float Mul", "Float Mad", "Float Div" };
	unsigned int auFlopsPerInstance[] = { 6+(TEST_MAD_NUM_ITERATIONS*98), 4+(TEST_MAD_NUM_ITERATIONS*98), 4+(TEST_MAD_NUM_ITERATIONS*196/*mad 98+98*/), 4+(TEST_MAD_NUM_ITERATIONS*98) };
	cl_ulong aulMinTime[4];
	cl_ulong aulMaxTime[4];
	cl_ulong ulMaxTime;
	cl_ulong ulMinTime;
	double fTimeInSeconds;

	/* Print out some metrics for each kernel */
	for (i = 0; i != 4; i++)
	{
		unsigned long long ullFLO;
		cl_ulong ulStart;
		cl_ulong ulEnd;

		double fFLOPS;
		double fGFLOPS;
		double fTotalGFlop;

		/* Print out timing information (command queue had profiling enabled) */
		eResult = clGetEventProfilingInfo(apsProfilingEvents[i],CL_PROFILING_COMMAND_START,sizeof(cl_ulong),&ulStart,NULL);
		CheckAndReportError(psInstance, "clGetEventProfilingInfo", eResult, verify_float_cleanup);
		eResult = clGetEventProfilingInfo(apsProfilingEvents[i],CL_PROFILING_COMMAND_END  ,sizeof(cl_ulong),&ulEnd,NULL);
		CheckAndReportError(psInstance, "clGetEventProfilingInfo", eResult, verify_float_cleanup);
		aulMinTime[i] = ulStart;
		aulMaxTime[i] = ulEnd;
		fTimeInSeconds = ((double)(ulEnd-ulStart)) / 1000000000.0;

		/* Calculate the total number of floating point operations */
		ullFLO = auFlopsPerInstance[i];
		ullFLO *= psData->puGlobalWorkSize[0];

		/* Divide by the time to get the FLOPS */
		fFLOPS = ullFLO / fTimeInSeconds;

		/* Convert to GIGA flops */
		fGFLOPS = fFLOPS / 1000000000.0;

		fTotalGFlop = ullFLO / 1000000000.0;

		OCLTestLog("---------------\n");
		OCLTestLog("%s:\n", apszTestNames[i]);
		OCLTestLog("---------------\n");
		OCLTestLog("%s: Time                      %fs\n",__func__,fTimeInSeconds);
		OCLTestLog("%s: Iterations                %d\n",__func__,TEST_MAD_NUM_ITERATIONS);
		OCLTestLog("%s: Instances                 %zu\n",__func__,psData->puGlobalWorkSize[0]);
		OCLTestLog("%s: Floating point operations %llu\n",__func__,ullFLO);
		OCLTestLog("%s: GFlop                     %f\n",__func__,fTotalGFlop);
		OCLTestLog("%s: GFLOP/S                   %f\n",__func__,fGFLOPS);
//		OCLTestLog("%s: FLOPS                     %f\n",__func__,fFLOPS);

		switch(psData->computemode) {
		case 0:
		  OCLMetricOutputDouble(__func__,apszTestNames[i],fGFLOPS,GFLOPSEC);
		  break;
		case 1:
		  OCLMetricOutputDouble2(__func__,apszTestNames[i],"PARALLEL",fGFLOPS,GFLOPSEC);
		  break;
		case 2:
		  OCLMetricOutputDouble2(__func__,apszTestNames[i],"CHAIN",fGFLOPS,GFLOPSEC);
		  break;
		default:
		  break;
		}
	}
	/* Print out timing information for whole set (command queue had profiling enabled) */
	ulMinTime = aulMinTime[0];
	ulMaxTime = aulMaxTime[0];
	for (i=1; i != 4; i++) {
	  if (ulMinTime > aulMinTime[i]) ulMinTime = aulMinTime[i];
	  if (ulMaxTime < aulMaxTime[i]) ulMaxTime = aulMaxTime[i];
	}
	fTimeInSeconds = ((double)(ulMaxTime-ulMinTime)) / 1000000000.0;
	OCLTestLog("---------------\n");
	OCLTestLog("%s: All tests time                      %fs\n",__func__,fTimeInSeconds);
	OCLTestLog("---------------\n");
	/* Cleanup */
	clReleaseEvent       ( psData->psAddCompleteEvent        );
	clReleaseEvent       ( psData->psMulCompleteEvent        );
	clReleaseEvent       ( psData->psMadCompleteEvent        );
	clReleaseEvent       ( psData->psDivCompleteEvent        );
	clReleaseMemObject   ( psData->psBufferA      );
	clReleaseKernel      ( psData->psAddKernel       );
	clReleaseKernel      ( psData->psMulKernel       );
	clReleaseKernel      ( psData->psMadKernel       );
	clReleaseKernel      ( psData->psDivKernel       );
	clReleaseCommandQueue( psData->psCommandQueue );
	clReleaseProgram     ( psData->psProgram      );
	clReleaseContext     ( psData->psContext      );

	free(psData->pui32BufferA);

	free(psInstance->pvPrivateData);
	psInstance->pvPrivateData = NULL;
	return eResult;

verify_float_cleanup:
	return eResult;
}

/******************************************************************************
 End of file (float.c)
******************************************************************************/
