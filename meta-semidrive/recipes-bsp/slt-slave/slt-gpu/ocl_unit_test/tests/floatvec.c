/*************************************************************************/ /*!
@File           floatvec.c
@Copyright      Copyright (c) Imagination Technologies Ltd. All Rights Reserved
@License        Strictly Confidential.
*/ /**************************************************************************/

#include <unistd.h>
#include <sys/time.h>

#define TOSTR(x) #x
#define STR(x) TOSTR(x)
#ifdef CUT_DOWN_UNIT_TEST
#define TEST_FLOATOPS_MAX_INSTANCES  (1024)
#define TEST_FLOATOPS_NUM_ITERATIONS 8
#else
#define TEST_FLOATOPS_MAX_INSTANCES  (32768)
#define TEST_FLOATOPS_NUM_ITERATIONS 200
#endif
#define TEST_FLOATOPS_NUM_TYPE_SIZES	  6

typedef struct _FloatopsData_
{
	/* CL Objects */
	cl_context       psContext;
	cl_program       psProgram[TEST_FLOATOPS_NUM_TYPE_SIZES];
	cl_device_id     psDeviceID;
	cl_platform_id   psPlatformID;
	cl_command_queue psCommandQueue;

	cl_kernel        psAddKernel[TEST_FLOATOPS_NUM_TYPE_SIZES];
	cl_kernel        psMulKernel[TEST_FLOATOPS_NUM_TYPE_SIZES];
	cl_kernel        psMadKernel[TEST_FLOATOPS_NUM_TYPE_SIZES];
	cl_kernel        psDivKernel[TEST_FLOATOPS_NUM_TYPE_SIZES];

	cl_mem           psBufferA[TEST_FLOATOPS_NUM_TYPE_SIZES];

	cl_event         psAddCompleteEvent[TEST_FLOATOPS_NUM_TYPE_SIZES];
	cl_event         psMulCompleteEvent[TEST_FLOATOPS_NUM_TYPE_SIZES];
	cl_event         psMadCompleteEvent[TEST_FLOATOPS_NUM_TYPE_SIZES];
	cl_event         psDivCompleteEvent[TEST_FLOATOPS_NUM_TYPE_SIZES];

	/* Host Object */
	float *pfBuffer;
	size_t        puGlobalWorkSize[3][TEST_FLOATOPS_NUM_TYPE_SIZES];
	size_t*       puLocalWorkSize;
} FloatopsData;

static char* floatTypeSizeNames[TEST_MEMCPY_NUM_TYPE_SIZES] = {"float", "float2", "float3", "float4", "float8", "float16"};
static int floatTypeDividers[TEST_MEMCPY_NUM_TYPE_SIZES] = {1, 2, 3, 4, 8, 16};
static char *g_pszFloatopsSource =
{
"#define TEST_FLOATOPS_NUM_ITERATIONS " STR(TEST_FLOATOPS_NUM_ITERATIONS) \
"\n"
"#define FLOATOPS_VARIANT %s\n"
"__kernel void computeFloatAdd(float in,\n"
"				  __global FLOATOPS_VARIANT *output)\n"
"{\n"
"	// 5 float ops\n"
"	FLOATOPS_VARIANT a = 1.0f + in;\n"
"	FLOATOPS_VARIANT b = 2.0f + in;\n"
"	FLOATOPS_VARIANT c = 3.0f + in;\n"
"	FLOATOPS_VARIANT d = 4.0f + in;\n"
"	FLOATOPS_VARIANT e = 5.0f + in;\n"
"\n"
"	for (unsigned short i = 0; i < TEST_FLOATOPS_NUM_ITERATIONS; ++i)\n"
"	{\n"
"		// 5 + (TEST_FLOATOPS_NUM_ITERATIONS * 18) floating point ops\n"
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
"		// 5 + (TEST_FLOATOPS_NUM_ITERATIONS * 38) floating point ops\n"
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
"		// 5 + (TEST_FLOATOPS_NUM_ITERATIONS * 58) floating point ops\n"
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
"		// 5 + (TEST_FLOATOPS_NUM_ITERATIONS * 78) floating point ops\n"
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
"		// 5 + (TEST_FLOATOPS_NUM_ITERATIONS * 98) floating point ops\n"
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
"	// 6 + (TEST_FLOATOPS_NUM_ITERATIONS * 98) floating point ops\n"
"	*output = a + b;\n"
"}\n"
"\n"
"__kernel void computeFloatMul(float in,\n"
"				  __global FLOATOPS_VARIANT *output)\n"
"{\n"
"	// 3 float operations\n"
"	FLOATOPS_VARIANT a = 3.0f * in;\n"
"	FLOATOPS_VARIANT b = 5.0f * in;\n"
"	FLOATOPS_VARIANT c = 7.0f * in;\n"
"\n"
"	for (unsigned int i = 0; i < TEST_FLOATOPS_NUM_ITERATIONS; ++i)\n"
"	{\n"
"		// 3 * (TEST_FLOATOPS_NUM_ITERATIONS * 18) floating point ops\n"
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
"		// 3 + (TEST_FLOATOPS_NUM_ITERATIONS * 38) floating point ops\n"
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
"		// 3 + (TEST_FLOATOPS_NUM_ITERATIONS * 58) floating point ops\n"
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
"		// 3 + (TEST_FLOATOPS_NUM_ITERATIONS * 78) floating point ops\n"
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
"		// 3 + (TEST_FLOATOPS_NUM_ITERATIONS * 98) floating point ops\n"
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
"	// 4 + (TEST_FLOATOPS_NUM_ITERATIONS * 98) floating point ops\n"
"	*output = a * b;\n"
"}\n"
"\n"
"__kernel void computeFloatMad(float in,\n"
"                 __global FLOATOPS_VARIANT *output)\n"
"{\n"
"   // 3 float operations\n"
"   FLOATOPS_VARIANT a = 3.0f * in;\n"
"   FLOATOPS_VARIANT b = 5.0f * in;\n"
"   FLOATOPS_VARIANT c = 7.0f * in;\n"
"\n"
"   for (unsigned int i = 0; i < TEST_FLOATOPS_NUM_ITERATIONS; ++i)\n"
"   {\n"
"		// 3 + (TEST_FLOATOPS_NUM_ITERATIONS * 18) floating point ops\n"
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
"		// 3 + (TEST_FLOATOPS_NUM_ITERATIONS * 38) floating point ops\n"
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
"		// 3 + (TEST_FLOATOPS_NUM_ITERATIONS * 58) floating point ops\n"
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
"		// 3 + (TEST_FLOATOPS_NUM_ITERATIONS * 78) floating point ops\n"
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
"		// 3 + (TEST_FLOATOPS_NUM_ITERATIONS * 98) floating point ops\n"
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
"		// 3 + (TEST_FLOATOPS_NUM_ITERATIONS * 38) floating point ops\n"
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
"		// 3 + (TEST_FLOATOPS_NUM_ITERATIONS * 58) floating point ops\n"
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
"		// 3 + (TEST_FLOATOPS_NUM_ITERATIONS * 78) floating point ops\n"
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
"		// 3 + (TEST_FLOATOPS_NUM_ITERATIONS * 98) floating point ops\n"
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
"   }\n"
"	// 4 + (TEST_FLOATOPS_NUM_ITERATIONS * 98) floating point ops\n"
"	*output = a * b + c;\n"
"}\n"
"\n"
"__kernel void computeFloatDiv(float in,\n"
"				  __global FLOATOPS_VARIANT *output)\n"
"{\n"
"	// 3 float operations\n"
"	FLOATOPS_VARIANT a = 1.0f / in;\n"
"	FLOATOPS_VARIANT b = 2.0f / in;\n"
"	FLOATOPS_VARIANT c = 3.0f / in;\n"
"\n"
"	for (unsigned int i = 0; i < TEST_FLOATOPS_NUM_ITERATIONS; ++i)\n"
"	{\n"
"		// 3 * (TEST_FLOATOPS_NUM_ITERATIONS * 18) floating point ops\n"
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
"		// 3 + (TEST_FLOATOPS_NUM_ITERATIONS * 38) floating point ops\n"
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
"		// 3 + (TEST_FLOATOPS_NUM_ITERATIONS * 58) floating point ops\n"
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
"		// 3 + (TEST_FLOATOPS_NUM_ITERATIONS * 78) floating point ops\n"
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
"		// 3 + (TEST_FLOATOPS_NUM_ITERATIONS * 98) floating point ops\n"
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
"	// 4 + (TEST_FLOATOPS_NUM_ITERATIONS * 98) floating point ops\n"
"	*output = a / b;\n"
"}\n"

};

/***********************************************************************************
 Function Name      : Init_Floatops
 Inputs             : None
 Outputs            : None
 Returns            : None
 Description        : Initialises input data
************************************************************************************/
static cl_int
Init_Floatops(OCLTestInstance *psInstance)
{
	size_t auLengths[2] = {0,0};
	char *ppszSource[2] = { 0 };
	char programSource[8192];
	float fValue = 1.0;
	int i32TypeIndex = 0;
	cl_int eResult;

	FloatopsData *psData = (FloatopsData*)calloc(1, sizeof(FloatopsData));
	if (!psData)
	{
		return CL_OUT_OF_RESOURCES;
	}

	/* Initialise data */
	psInstance->pvPrivateData = (void*)psData;

	/* Allocate second host buffer */
	psData->pfBuffer = (float*)malloc(sizeof(float));

	if(!psData->pfBuffer)
	{
		free(psData);
		return CL_OUT_OF_RESOURCES;
	}

	/* Initialise with test values */
	psData->pfBuffer[0] = 0.0;

	eResult = clGetPlatformIDs(1,&psData->psPlatformID,NULL);
	CheckAndReportError(psInstance, "clGetPlatformIDs", eResult, init_floatops_cleanup);

	eResult = clGetDeviceIDs(psData->psPlatformID,CL_DEVICE_TYPE_GPU,1,&psData->psDeviceID,NULL);
	CheckAndReportError(psInstance, "clGetDeviceIDs", eResult, init_floatops_cleanup);

	psData->psContext = clCreateContext(NULL,1,&psData->psDeviceID,NULL,NULL,&eResult);
	CheckAndReportError(psInstance, "clCreateContext", eResult, init_floatops_cleanup);

	psData->psCommandQueue = clCreateCommandQueue(psData->psContext, psData->psDeviceID, CL_QUEUE_PROFILING_ENABLE, &eResult);
	CheckAndReportError(psInstance, "clCreateCommandQueue", eResult, init_floatops_cleanup);

	for (i32TypeIndex = 0; i32TypeIndex < TEST_FLOATOPS_NUM_TYPE_SIZES; i32TypeIndex++)
	{
		/* Use the global int program */
		programSource[0] = 0;
		snprintf(programSource, 8192, g_pszFloatopsSource, floatTypeSizeNames[i32TypeIndex]);
		auLengths[0]  = strlen(programSource);
		ppszSource[0] = programSource;
		/*
		* Build the Program
		*/

		psData->psProgram[i32TypeIndex] = clCreateProgramWithSource(psData->psContext, 1, (const char**)ppszSource, auLengths, &eResult);
		CheckAndReportError(psInstance, "clCreateProgramWithSource", eResult, init_floatops_cleanup);

		eResult = clBuildProgram(psData->psProgram[i32TypeIndex],1,&psData->psDeviceID,"-cl-fast-relaxed-math",NULL,NULL);

		if(eResult != CL_SUCCESS)
		{
			char         aszBuildLog[512];
			size_t       uBuildLogSize;

			eResult = clGetProgramBuildInfo(psData->psProgram[i32TypeIndex], psData->psDeviceID, CL_PROGRAM_BUILD_LOG, 512, aszBuildLog, &uBuildLogSize);
			CheckAndReportError(psInstance, "clGetProgramBuildInfo", eResult, init_floatops_cleanup);

			aszBuildLog[uBuildLogSize-1] = '\0';
			OCLTestLog("[%zu] CL_PROGRAM_BUILD_LOG:\n%s\n",uBuildLogSize,aszBuildLog);
		}
		CheckAndReportError(psInstance, "clBuildProgram", eResult, init_floatops_cleanup);

		/*
		* Setup the Arguments
		*/
		psData->psBufferA[i32TypeIndex] = clCreateBuffer(psData->psContext, CL_MEM_COPY_HOST_PTR, sizeof(cl_float), psData->pfBuffer, &eResult);
		CheckAndReportError(psInstance, "clCreateBuffer", eResult, init_floatops_cleanup);

		/* Init Add kernel */
		OCLTestLog("Compiling Add %s kernel...\n", floatTypeSizeNames[i32TypeIndex]);
		psData->psAddKernel[i32TypeIndex] = clCreateKernel(psData->psProgram[i32TypeIndex], "computeFloatAdd", &eResult);
		CheckAndReportError(psInstance, "clCreateKernel", eResult, init_floatops_cleanup);

		{
			eResult = clSetKernelArg(psData->psAddKernel[i32TypeIndex], 0, sizeof(cl_float), (void*) &fValue );
			CheckAndReportError(psInstance, "clSetKernelArg", eResult, init_floatops_cleanup);
		}

		eResult = clSetKernelArg(psData->psAddKernel[i32TypeIndex], 1, sizeof(cl_mem), (void*) &psData->psBufferA[i32TypeIndex] );
		CheckAndReportError(psInstance, "clSetKernelArg", eResult, init_floatops_cleanup);

		/* Init Mul kernel */
		OCLTestLog("Compiling Mul %s kernel...\n", floatTypeSizeNames[i32TypeIndex]);
		psData->psMulKernel[i32TypeIndex] = clCreateKernel(psData->psProgram[i32TypeIndex], "computeFloatMul", &eResult);
		CheckAndReportError(psInstance, "clCreateKernel", eResult, init_floatops_cleanup);

		{
			eResult = clSetKernelArg(psData->psMulKernel[i32TypeIndex], 0, sizeof(cl_float), (void*) &fValue );
			CheckAndReportError(psInstance, "clSetKernelArg", eResult, init_floatops_cleanup);
		}

		eResult = clSetKernelArg(psData->psMulKernel[i32TypeIndex], 1, sizeof(cl_mem), (void*) &psData->psBufferA[i32TypeIndex] );
		CheckAndReportError(psInstance, "clSetKernelArg", eResult, init_floatops_cleanup);

		/* Init Mad kernel */
		OCLTestLog("Compiling Mad %s kernel...\n", floatTypeSizeNames[i32TypeIndex]);
		psData->psMadKernel[i32TypeIndex] = clCreateKernel(psData->psProgram[i32TypeIndex], "computeFloatMad", &eResult);
		CheckAndReportError(psInstance, "clCreateKernel", eResult, init_floatops_cleanup);

		{
			eResult = clSetKernelArg(psData->psMadKernel[i32TypeIndex], 0, sizeof(cl_float), (void*) &fValue );
			CheckAndReportError(psInstance, "clSetKernelArg", eResult, init_floatops_cleanup);
		}

		eResult = clSetKernelArg(psData->psMadKernel[i32TypeIndex], 1, sizeof(cl_mem), (void*) &psData->psBufferA[i32TypeIndex] );
		CheckAndReportError(psInstance, "clSetKernelArg", eResult, init_floatops_cleanup);

		/* Init Div kernel */
		OCLTestLog("Compiling Div %s kernel...\n", floatTypeSizeNames[i32TypeIndex]);
		psData->psDivKernel[i32TypeIndex] = clCreateKernel(psData->psProgram[i32TypeIndex], "computeFloatDiv", &eResult);
		CheckAndReportError(psInstance, "clCreateKernel", eResult, init_floatops_cleanup);

		{
			eResult = clSetKernelArg(psData->psDivKernel[i32TypeIndex], 0, sizeof(cl_float), (void*) &fValue );
			CheckAndReportError(psInstance, "clSetKernelArg", eResult, init_floatops_cleanup);
		}

		eResult = clSetKernelArg(psData->psDivKernel[i32TypeIndex], 1, sizeof(cl_mem), (void*) &psData->psBufferA[i32TypeIndex] );
		CheckAndReportError(psInstance, "clSetKernelArg", eResult, init_floatops_cleanup);

		/* Setup the size of the kernel run */
		psData->puGlobalWorkSize[0][i32TypeIndex] = TEST_FLOATOPS_MAX_INSTANCES;// / floatTypeDividers[i32TypeIndex];
		psData->puLocalWorkSize = NULL;

		OCLTestLog("%s: Reducing from %d to %zu due to use of %s in kernel.\n",__func__,TEST_FLOATOPS_MAX_INSTANCES,
				   psData->puGlobalWorkSize[0][i32TypeIndex], floatTypeSizeNames[i32TypeIndex]);
	}

	return CL_SUCCESS;

init_floatops_cleanup:
	return eResult;
}

/***********************************************************************************
 Function Name      : Compute_Floatops
 Inputs             : psInstance - test instance data
 Returns            : 0 or 1
 Description        : Main compute func
************************************************************************************/
static cl_int
Compute_Floatops(OCLTestInstance *psInstance)
{
	cl_int eResult;
	size_t global_dim[1];
	int i32TypeIndex = 0;
	FloatopsData *psData =(FloatopsData*) psInstance->pvPrivateData;

	for (i32TypeIndex = 0; i32TypeIndex < TEST_FLOATOPS_NUM_TYPE_SIZES; i32TypeIndex++)
	{
		OCLTestLog("%s: Online compilation test with %s using %zu instances, each with %d iterations\n",
			__func__,floatTypeSizeNames[i32TypeIndex], psData->puGlobalWorkSize[0][i32TypeIndex],TEST_FLOATOPS_NUM_ITERATIONS);

		/* Reduce to match vec4 output size */
		global_dim[0] = psData->puGlobalWorkSize[0][i32TypeIndex];

		// Enqueue and complete Add kernel
		OCLTestLog("Running Add kernel for %s...\n", floatTypeSizeNames[i32TypeIndex]);
		eResult = clEnqueueNDRangeKernel(psData->psCommandQueue, psData->psAddKernel[i32TypeIndex], 1, NULL, global_dim, psData->puLocalWorkSize , 0, NULL, &psData->psAddCompleteEvent[i32TypeIndex]);
		CheckAndReportError(psInstance, "clEnqueueNDRangeKernel", eResult, compute_floatops_cleanup);

		clFinish(psData->psCommandQueue);

		// Enqueue and complete Mul kernel
		OCLTestLog("Running Mul kernel for %s...\n", floatTypeSizeNames[i32TypeIndex]);
		eResult = clEnqueueNDRangeKernel(psData->psCommandQueue, psData->psMulKernel[i32TypeIndex], 1, NULL, global_dim, psData->puLocalWorkSize , 0, NULL, &psData->psMulCompleteEvent[i32TypeIndex]);
		CheckAndReportError(psInstance, "clEnqueueNDRangeKernel", eResult, compute_floatops_cleanup);

		clFinish(psData->psCommandQueue);

		// Enqueue and complete Mad kernel
		OCLTestLog("Running Mad kernel for %s...\n",floatTypeSizeNames[i32TypeIndex]);
		eResult = clEnqueueNDRangeKernel(psData->psCommandQueue, psData->psMadKernel[i32TypeIndex], 1, NULL, global_dim, psData->puLocalWorkSize , 0, NULL, &psData->psMadCompleteEvent[i32TypeIndex]);
		CheckAndReportError(psInstance, "clEnqueueNDRangeKernel", eResult, compute_floatops_cleanup);

		clFinish(psData->psCommandQueue);

		// Enqueue and complete Div kernel
		OCLTestLog("Running Div kernel for %s...\n", floatTypeSizeNames[i32TypeIndex]);
		eResult = clEnqueueNDRangeKernel(psData->psCommandQueue, psData->psDivKernel[i32TypeIndex], 1, NULL, global_dim, psData->puLocalWorkSize , 0, NULL, &psData->psDivCompleteEvent[i32TypeIndex]);
		CheckAndReportError(psInstance, "clEnqueueNDRangeKernel", eResult, compute_floatops_cleanup);

		clFinish(psData->psCommandQueue);
	}

	return CL_SUCCESS;

compute_floatops_cleanup:
	return eResult;
}

/***********************************************************************************
 Function Name      : Verify_Floatops
 Inputs             : None
 Outputs            : None
 Returns            : None
 Description        : Verifies output data
************************************************************************************/
static cl_int
Verify_Floatops(OCLTestInstance *psInstance)
{
	cl_int eResult = CL_SUCCESS;
	int i = 0;
	int i32TypeIndex = 0;

	FloatopsData *psData =(FloatopsData*) psInstance->pvPrivateData;

	for (i32TypeIndex = 0; i32TypeIndex < TEST_FLOATOPS_NUM_TYPE_SIZES; i32TypeIndex++)
	{
			cl_event apsProfilingEvents[] = { psData->psAddCompleteEvent[i32TypeIndex], psData->psMulCompleteEvent[i32TypeIndex],
											psData->psMadCompleteEvent[i32TypeIndex], psData->psDivCompleteEvent[i32TypeIndex] };
		char* apszTestNames[] = { "Add", "Mul", "Mad", "Div" };
		unsigned int auOpsPerInstance[] = { 6+(TEST_FLOATOPS_NUM_ITERATIONS*98), 4+(TEST_FLOATOPS_NUM_ITERATIONS*98), 4+(TEST_FLOATOPS_NUM_ITERATIONS*196/*mad 98+98*/), 4+(TEST_FLOATOPS_NUM_ITERATIONS*98) };

		/* Print out some metrics for each kernel */
		for (i = 0; i != 4; i++)
		{
			cl_ulong ulStart;
			cl_ulong ulEnd;
			unsigned long long ullIO;
			double fTimeInSeconds;
			double fFLOPS;
			double fGFLOPS;
			double fTotalGIOp;

			/* Print out timing information (command queue had profiling enabled) */
			eResult = clGetEventProfilingInfo(apsProfilingEvents[i],CL_PROFILING_COMMAND_START,sizeof(cl_ulong),&ulStart,NULL);
			CheckAndReportError(psInstance, "clGetEventProfilingInfo", eResult, verify_floatops_cleanup);
			eResult = clGetEventProfilingInfo(apsProfilingEvents[i],CL_PROFILING_COMMAND_END  ,sizeof(cl_ulong),&ulEnd,NULL);
			CheckAndReportError(psInstance, "clGetEventProfilingInfo", eResult, verify_floatops_cleanup);

			fTimeInSeconds = ((double)(ulEnd-ulStart)) / 1000000000.0;

			/* Calculate the total number of float operations */
			ullIO = auOpsPerInstance[i] * floatTypeDividers[i32TypeIndex] /* operating on floatX */;
			ullIO *= psData->puGlobalWorkSize[0][i32TypeIndex];

			/* Divide by the time to get the FLOPS */
			fFLOPS = ullIO / fTimeInSeconds;

			/* Convert to GIGA FLOPs */
			fGFLOPS = fFLOPS / 1000000000.0;

			fTotalGIOp = ullIO / 1000000000.0;

			OCLTestLog("---------------\n");
			OCLTestLog("%s %s:\n", floatTypeSizeNames[i32TypeIndex], apszTestNames[i]);
			OCLTestLog("---------------\n");
			OCLTestLog("%s: Time                      %fs\n",__func__,fTimeInSeconds);
			OCLTestLog("%s: Iterations                %d\n",__func__,TEST_FLOATOPS_NUM_ITERATIONS);
			OCLTestLog("%s: Instances                 %zu\n",__func__,psData->puGlobalWorkSize[0][i32TypeIndex]);
			OCLTestLog("%s: Float operations %llu\n",__func__,ullIO);
			OCLTestLog("%s: GFLOP                     %f\n",__func__,fTotalGIOp);
			OCLTestLog("%s: GFLOP/S                   %f\n",__func__,fGFLOPS);
			//OCLTestLog("%s: IOPS                     %f\n",__func__,fIOPS);

			OCLMetricOutputDouble2(__func__,floatTypeSizeNames[i32TypeIndex],apszTestNames[i],fGFLOPS,GFLOPSEC);
		}
		OCLTestLog("\n");
	}

	/* Cleanup */
	for (i32TypeIndex = 0; i32TypeIndex < TEST_FLOATOPS_NUM_TYPE_SIZES; i32TypeIndex++)
	{
		clReleaseEvent       ( psData->psAddCompleteEvent[i32TypeIndex]);
		clReleaseEvent       ( psData->psMulCompleteEvent[i32TypeIndex]);
		clReleaseEvent       ( psData->psMadCompleteEvent[i32TypeIndex]);
		clReleaseEvent       ( psData->psDivCompleteEvent[i32TypeIndex]);
		clReleaseKernel      ( psData->psAddKernel[i32TypeIndex]);
		clReleaseKernel      ( psData->psMulKernel[i32TypeIndex]);
		clReleaseKernel      ( psData->psMadKernel[i32TypeIndex]);
		clReleaseKernel      ( psData->psDivKernel[i32TypeIndex]);
		clReleaseMemObject   ( psData->psBufferA[i32TypeIndex]);
	}


	clReleaseCommandQueue( psData->psCommandQueue );
	for (i32TypeIndex = 0; i32TypeIndex < TEST_FLOATOPS_NUM_TYPE_SIZES; i32TypeIndex++)
	{
		clReleaseProgram     ( psData->psProgram[i32TypeIndex]);
	}
	clReleaseContext     ( psData->psContext      );

	free(psData->pfBuffer);

	free(psInstance->pvPrivateData);
	psInstance->pvPrivateData = NULL;
	return eResult;

verify_floatops_cleanup:
	return eResult;
}

/******************************************************************************
 End of file (floatvec.c)
******************************************************************************/
