/*************************************************************************/ /*!
@File           halfvec.c
@Copyright      Copyright (c) Imagination Technologies Ltd. All Rights Reserved
@License        Strictly Confidential.
*/ /**************************************************************************/

#include <unistd.h>
#include <sys/time.h>

#define TOSTR(x) #x
#define STR(x) TOSTR(x)
#ifdef CUT_DOWN_UNIT_TEST
#define TEST_HALFOPS_MAX_INSTANCES  (1024)
#define TEST_HALFOPS_NUM_ITERATIONS 10
#else
#define TEST_HALFOPS_MAX_INSTANCES  (512*1024)
#define TEST_HALFOPS_NUM_ITERATIONS 50
#endif
#define TEST_HALFOPS_NUM_TYPE_SIZES	  6

typedef struct _HalfopsData_
{
	/* CL Objects */
	cl_context       psContext;
	cl_program       psProgram[TEST_HALFOPS_NUM_TYPE_SIZES];
	cl_device_id     psDeviceID;
	cl_platform_id   psPlatformID;
	cl_command_queue psCommandQueue;

	cl_kernel        psAddKernel[TEST_HALFOPS_NUM_TYPE_SIZES];
	cl_kernel        psMulKernel[TEST_HALFOPS_NUM_TYPE_SIZES];
	cl_kernel        psMadKernel[TEST_HALFOPS_NUM_TYPE_SIZES];
	cl_kernel        psDivKernel[TEST_HALFOPS_NUM_TYPE_SIZES];
	cl_kernel        psSOPKernel[TEST_HALFOPS_NUM_TYPE_SIZES];

	cl_mem           psBufferA[TEST_HALFOPS_NUM_TYPE_SIZES][5];

	cl_event         psAddCompleteEvent[TEST_HALFOPS_NUM_TYPE_SIZES];
	cl_event         psMulCompleteEvent[TEST_HALFOPS_NUM_TYPE_SIZES];
	cl_event         psMadCompleteEvent[TEST_HALFOPS_NUM_TYPE_SIZES];
	cl_event         psDivCompleteEvent[TEST_HALFOPS_NUM_TYPE_SIZES];
	cl_event         psSOPCompleteEvent[TEST_HALFOPS_NUM_TYPE_SIZES];

	/* Host Object */
	float		*pfBuffer;
	size_t		puGlobalWorkSize[3][TEST_HALFOPS_NUM_TYPE_SIZES];
	size_t*		puLocalWorkSize;
} HalfopsData;

static char* halfTypeSizeNames[TEST_HALFOPS_NUM_TYPE_SIZES] = {"half", "half2", "half3", "half4", "half8", "half16"};
static int halfTypeDividers[TEST_HALFOPS_NUM_TYPE_SIZES] = {1, 2, 3, 4, 8, 16};
static char *g_pszHalfopsSource =
{
"#define TEST_HALFOPS_NUM_ITERATIONS " STR(TEST_HALFOPS_NUM_ITERATIONS) \
"\n"
"#define HALFOPS_VARIANT %s\n"
"#define CONVERT_FLOAT(id) convert_float%s(id)\n"
"#define CONVERT_HALF(id) convert_half%s%s(id)\n"
"#define CONVERT_UCHAR(id) convert_uchar%s(id)\n"
"#define UCHAR uchar%s\n"
"#define FLOAT float%s\n"
"#pragma OPENCL EXTENSION cl_khr_fp16 : enable\n"
"__kernel void computeHalfAdd(__global FLOAT* in,\n"
"				  __global FLOAT *output)\n"
"{\n"
"	// 5 float ops\n"
"	UCHAR temp = CONVERT_UCHAR(in[get_local_id(0)]*3);\n"
"	HALFOPS_VARIANT a = 1.0h + CONVERT_HALF(temp);\n"
"	HALFOPS_VARIANT b = 2.5h + CONVERT_HALF(temp);\n"
"   temp = CONVERT_UCHAR(a+b);\n"
"  if (get_global_id(0) == 0)\n"
"	*output = CONVERT_FLOAT(temp);\n"
"}\n"
"\n"
"__kernel void computeHalfMul(__global FLOAT* in,\n"
"				  __global FLOAT *output)\n"
"{\n"
"	// 3 float operations\n"
"	HALFOPS_VARIANT a = 3.0h * CONVERT_HALF(*in);\n"
"	HALFOPS_VARIANT b = 5.0h * CONVERT_HALF(*in);\n"
"	HALFOPS_VARIANT c = 7.0h * CONVERT_HALF(*in);\n"
"\n"
"	for (unsigned int i = 0; i < TEST_HALFOPS_NUM_ITERATIONS; ++i)\n"
"	{\n"
"		// 3 * (TEST_HALFOPS_NUM_ITERATIONS * 18) floating point ops\n"
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
"		// 3 + (TEST_HALFOPS_NUM_ITERATIONS * 38) floating point ops\n"
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
"		// 3 + (TEST_HALFOPS_NUM_ITERATIONS * 58) floating point ops\n"
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
"		// 3 + (TEST_HALFOPS_NUM_ITERATIONS * 78) floating point ops\n"
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
"		// 3 + (TEST_HALFOPS_NUM_ITERATIONS * 98) floating point ops\n"
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
"	// 4 + (TEST_HALFOPS_NUM_ITERATIONS * 98) floating point ops\n"
"  if (get_global_id(0) == 0)\n"
"	*output = CONVERT_FLOAT(a * b);\n"
"}\n"
"\n"
"__kernel void computeHalfMad(__global FLOAT* in,\n"
"                 __global FLOAT *output)\n"
"{\n"
"   // 3 float operations\n"
"   HALFOPS_VARIANT a = 3.0h * CONVERT_HALF(*in);\n"
"   HALFOPS_VARIANT b = 5.0h * CONVERT_HALF(*in);\n"
"   HALFOPS_VARIANT c = 7.0h * CONVERT_HALF(*in);\n"
"\n"
"   for (unsigned int i = 0; i < TEST_HALFOPS_NUM_ITERATIONS; ++i)\n"
"   {\n"
"		// 3 + (TEST_HALFOPS_NUM_ITERATIONS * 18) floating point ops\n"
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
"		// 3 + (TEST_HALFOPS_NUM_ITERATIONS * 38) floating point ops\n"
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
"		// 3 + (TEST_HALFOPS_NUM_ITERATIONS * 58) floating point ops\n"
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
"		// 3 + (TEST_HALFOPS_NUM_ITERATIONS * 78) floating point ops\n"
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
"		// 3 + (TEST_HALFOPS_NUM_ITERATIONS * 98) floating point ops\n"
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
"		// 3 + (TEST_HALFOPS_NUM_ITERATIONS * 38) floating point ops\n"
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
"		// 3 + (TEST_HALFOPS_NUM_ITERATIONS * 58) floating point ops\n"
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
"		// 3 + (TEST_HALFOPS_NUM_ITERATIONS * 78) floating point ops\n"
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
"		// 3 + (TEST_HALFOPS_NUM_ITERATIONS * 98) floating point ops\n"
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
"	// 4 + (TEST_HALFOPS_NUM_ITERATIONS * 98) floating point ops\n"
"  if (get_global_id(0) == 0)\n"
"	*output = CONVERT_FLOAT(a * b + c);\n"
"}\n"
"\n"
"__kernel void computeHalfDiv(__global FLOAT* in,\n"
"				  __global FLOAT *output)\n"
"{\n"
"	// 3 float operations\n"
"	HALFOPS_VARIANT a = 1.0h / CONVERT_HALF(*in);\n"
"	HALFOPS_VARIANT b = 2.0h / CONVERT_HALF(*in);\n"
"	HALFOPS_VARIANT c = 3.0h / CONVERT_HALF(*in);\n"
"\n"
"	for (unsigned int i = 0; i < TEST_HALFOPS_NUM_ITERATIONS; ++i)\n"
"	{\n"
"		// 3 * (TEST_HALFOPS_NUM_ITERATIONS * 18) floating point ops\n"
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
"		// 3 + (TEST_HALFOPS_NUM_ITERATIONS * 38) floating point ops\n"
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
"		// 3 + (TEST_HALFOPS_NUM_ITERATIONS * 58) floating point ops\n"
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
"		// 3 + (TEST_HALFOPS_NUM_ITERATIONS * 78) floating point ops\n"
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
"		// 3 + (TEST_HALFOPS_NUM_ITERATIONS * 98) floating point ops\n"
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
"	// 4 + (TEST_HALFOPS_NUM_ITERATIONS * 98) floating point ops\n"
"  if (get_global_id(0) == 0)\n"
"	*output = CONVERT_FLOAT(a / b);\n"
"}\n"
"\n"
"__kernel void computeHalfSOP(__global FLOAT* in,\n"
"				  __global FLOAT *output)\n"
"{\n"
"	// 3 float operations\n"
"	HALFOPS_VARIANT a = 1.0h / CONVERT_HALF(*(in+1));\n"
"	HALFOPS_VARIANT b = 2.0h / CONVERT_HALF(*(in+2));\n"
"	HALFOPS_VARIANT c = 3.0h / CONVERT_HALF(*(in+3));\n"
"	HALFOPS_VARIANT d = 3.0h / CONVERT_HALF(*(in+4));\n"
"\n"
"	for (unsigned int i = 0; i < TEST_HALFOPS_NUM_ITERATIONS; ++i)\n"
"	{\n"
"		// 1 SOP * (TEST_HALFOPS_NUM_ITERATIONS * 10) floating point ops\n"
"		a = c * a + b * d;\n"
"		b = d * a + c * b;\n"
"		a = c * a + b * d;\n"
"		b = d * a + c * b;\n"
"		a = c * a + b * d;\n"
"		b = d * a + c * b;\n"
"		a = c * a + b * d;\n"
"		b = d * a + c * b;\n"
"		a = c * a + b * d;\n"
"		b = d * a + c * b;\n"
"		// 1 SOP * (TEST_HALFOPS_NUM_ITERATIONS * 20) floating point ops\n"
"		a = c * a + b * d;\n"
"		b = d * a + c * b;\n"
"		a = c * a + b * d;\n"
"		b = d * a + c * b;\n"
"		a = c * a + b * d;\n"
"		b = d * a + c * b;\n"
"		a = c * a + b * d;\n"
"		b = d * a + c * b;\n"
"		a = c * a + b * d;\n"
"		b = d * a + c * b;\n"
"		// 1 SOP * (TEST_HALFOPS_NUM_ITERATIONS * 30) floating point ops\n"
"		a = c * a + b * d;\n"
"		b = d * a + c * b;\n"
"		a = c * a + b * d;\n"
"		b = d * a + c * b;\n"
"		a = c * a + b * d;\n"
"		b = d * a + c * b;\n"
"		a = c * a + b * d;\n"
"		b = d * a + c * b;\n"
"		a = c * a + b * d;\n"
"		b = d * a + c * b;\n"
"		// 1 SOP * (TEST_HALFOPS_NUM_ITERATIONS * 40) floating point ops\n"
"		a = c * a + b * d;\n"
"		b = d * a + c * b;\n"
"		a = c * a + b * d;\n"
"		b = d * a + c * b;\n"
"		a = c * a + b * d;\n"
"		b = d * a + c * b;\n"
"		a = c * a + b * d;\n"
"		b = d * a + c * b;\n"
"		a = c * a + b * d;\n"
"		b = d * a + c * b;\n"
"		// 1 SOP * (TEST_HALFOPS_NUM_ITERATIONS * 50) floating point ops\n"
"		a = c * a + b * d;\n"
"		b = d * a + c * b;\n"
"		a = c * a + b * d;\n"
"		b = d * a + c * b;\n"
"		a = c * a + b * d;\n"
"		b = d * a + c * b;\n"
"		a = c * a + b * d;\n"
"		b = d * a + c * b;\n"
"		a = c * a + b * d;\n"
"		b = d * a + c * b;\n"
"	}\n"
"\n"
"		// 1 SOP * (TEST_HALFOPS_NUM_ITERATIONS * 51) floating point ops\n"
"  if (get_global_id(0) == 0)\n"
"	*output = CONVERT_FLOAT(a * d + c * b);\n"
"}\n"
};

/***********************************************************************************
 Function Name      : Init_Halfops
 Inputs             : None
 Outputs            : None
 Returns            : None
 Description        : Initialises input data
************************************************************************************/
static cl_int
Init_Halfops(OCLTestInstance *psInstance)
{
	size_t auLengths[2] = {0,0};
	char *ppszSource[2] = { 0 };
	char programSource[16384];
	/* Enough space for a maximum work group of 1024 with vec16 values */
	float fValue[16*1024] = {1.0,2.0,3.0,4.0};
	int i32TypeIndex = 0;
	cl_int eResult;

	HalfopsData *psData = (HalfopsData*)calloc(1, sizeof(HalfopsData));

	/* Initialise data */
	psInstance->pvPrivateData = (void*)psData;

	/* Allocate second host buffer */
	psData->pfBuffer = (float*)malloc(sizeof(float)*halfTypeDividers[TEST_HALFOPS_NUM_TYPE_SIZES-1]*4);

	if(!psData->pfBuffer)
	{
		free(psData);
		return CL_OUT_OF_RESOURCES;
	}
	for (i32TypeIndex = 0; i32TypeIndex < halfTypeDividers[TEST_HALFOPS_NUM_TYPE_SIZES-1]*4; i32TypeIndex++)
	{
	/* Initialise with test values */
	psData->pfBuffer[i32TypeIndex] = -1.0;

	}
	eResult = clGetPlatformIDs(1,&psData->psPlatformID,NULL);
	CheckAndReportError(psInstance, "clGetPlatformIDs", eResult, init_halfops_cleanup);

	eResult = clGetDeviceIDs(psData->psPlatformID,CL_DEVICE_TYPE_GPU,1,&psData->psDeviceID,NULL);
	CheckAndReportError(psInstance, "clGetDeviceIDs", eResult, init_halfops_cleanup);

	psData->psContext = clCreateContext(NULL,1,&psData->psDeviceID,NULL,NULL,&eResult);
	CheckAndReportError(psInstance, "clCreateContext", eResult, init_halfops_cleanup);

	psData->psCommandQueue = clCreateCommandQueue(psData->psContext, psData->psDeviceID, CL_QUEUE_PROFILING_ENABLE, &eResult);
	CheckAndReportError(psInstance, "clCreateCommandQueue", eResult, init_halfops_cleanup);

	/* Enough space for a maximum work group of 1024 with vec16 values */
	cl_mem inputBuffer = clCreateBuffer(psData->psContext, CL_MEM_COPY_HOST_PTR, sizeof(cl_float)* 16 * 1024, &fValue, &eResult);
	CheckAndReportError(psInstance, "clCreateBuffer", eResult, init_halfops_cleanup);

	for (i32TypeIndex = 0; i32TypeIndex < TEST_HALFOPS_NUM_TYPE_SIZES; i32TypeIndex++)
	{
		/* Use the global int program */
		programSource[0] = 0;
		if (i32TypeIndex == 0)
		{
			sprintf(programSource, g_pszHalfopsSource, halfTypeSizeNames[i32TypeIndex], "", "","", "","","");
		}
		else
		{
			char tmp[3];
			sprintf(tmp,"%d", halfTypeDividers[i32TypeIndex]);
			sprintf(programSource, g_pszHalfopsSource, halfTypeSizeNames[i32TypeIndex], tmp,tmp,"", tmp,tmp,tmp);
		}
		auLengths[0]  = strlen(programSource);
		ppszSource[0] = programSource;
		/*
		* Build the Program
		*/

		psData->psProgram[i32TypeIndex] = clCreateProgramWithSource(psData->psContext, 1, (const char**)ppszSource, auLengths, &eResult);
		CheckAndReportError(psInstance, "clCreateProgramWithSource", eResult, init_halfops_cleanup);

		eResult = clBuildProgram(psData->psProgram[i32TypeIndex],1,&psData->psDeviceID,"-cl-fast-relaxed-math",NULL,NULL);

		if(eResult != CL_SUCCESS)
		{
			char         aszBuildLog[512];
			size_t       uBuildLogSize;

			eResult = clGetProgramBuildInfo(psData->psProgram[i32TypeIndex], psData->psDeviceID, CL_PROGRAM_BUILD_LOG, 512, aszBuildLog, &uBuildLogSize);
			aszBuildLog[uBuildLogSize-1] = '\0';
			OCLTestLog("[%zu] CL_PROGRAM_BUILD_LOG:\n%s\n",uBuildLogSize,aszBuildLog);
			CheckAndReportError(psInstance, "clGetProgramBuildInfo", eResult, init_halfops_cleanup);

			aszBuildLog[uBuildLogSize-1] = '\0';
			OCLTestLog("[%zu] CL_PROGRAM_BUILD_LOG:\n%s\n",uBuildLogSize,aszBuildLog);
		}
		CheckAndReportError(psInstance, "clBuildProgram", eResult, init_halfops_cleanup);

		/*
		* Setup the Arguments
		*/
	        psData->psBufferA[i32TypeIndex][0] = clCreateBuffer(psData->psContext, CL_MEM_COPY_HOST_PTR, sizeof(cl_float)* halfTypeDividers[i32TypeIndex]*4, psData->pfBuffer, &eResult);
		CheckAndReportError(psInstance, "clCreateBuffer", eResult, init_halfops_cleanup);

	        psData->psBufferA[i32TypeIndex][1] = clCreateBuffer(psData->psContext, CL_MEM_COPY_HOST_PTR, sizeof(cl_float)* halfTypeDividers[i32TypeIndex]*4, psData->pfBuffer, &eResult);
	        CheckAndReportError(psInstance, "clCreateBuffer", eResult, init_halfops_cleanup);

	        psData->psBufferA[i32TypeIndex][2] = clCreateBuffer(psData->psContext, CL_MEM_COPY_HOST_PTR, sizeof(cl_float)* halfTypeDividers[i32TypeIndex]*4, psData->pfBuffer, &eResult);
	        CheckAndReportError(psInstance, "clCreateBuffer", eResult, init_halfops_cleanup);

	        psData->psBufferA[i32TypeIndex][3] = clCreateBuffer(psData->psContext, CL_MEM_COPY_HOST_PTR, sizeof(cl_float)* halfTypeDividers[i32TypeIndex]*4, psData->pfBuffer, &eResult);
	        CheckAndReportError(psInstance, "clCreateBuffer", eResult, init_halfops_cleanup);

	        psData->psBufferA[i32TypeIndex][4] = clCreateBuffer(psData->psContext, CL_MEM_COPY_HOST_PTR, sizeof(cl_float)* halfTypeDividers[i32TypeIndex]*4, psData->pfBuffer, &eResult);
	        CheckAndReportError(psInstance, "clCreateBuffer", eResult, init_halfops_cleanup);

		/* Init Add kernel */
		OCLTestLog("Compiling Add %s kernel...\n", halfTypeSizeNames[i32TypeIndex]);
		psData->psAddKernel[i32TypeIndex] = clCreateKernel(psData->psProgram[i32TypeIndex], "computeHalfAdd", &eResult);
		CheckAndReportError(psInstance, "clCreateKernel", eResult, init_halfops_cleanup);
		eResult = clSetKernelArg(psData->psAddKernel[i32TypeIndex], 0, sizeof(cl_mem), &inputBuffer );
		CheckAndReportError(psInstance, "clSetKernelArg", eResult, init_halfops_cleanup);
		eResult = clSetKernelArg(psData->psAddKernel[i32TypeIndex], 1, sizeof(cl_mem), (void*) &psData->psBufferA[i32TypeIndex][0] );
		CheckAndReportError(psInstance, "clSetKernelArg", eResult, init_halfops_cleanup);

		/* Init Mul kernel */
		OCLTestLog("Compiling Mul %s kernel...\n", halfTypeSizeNames[i32TypeIndex]);
		psData->psMulKernel[i32TypeIndex] = clCreateKernel(psData->psProgram[i32TypeIndex], "computeHalfMul", &eResult);
		CheckAndReportError(psInstance, "clCreateKernel", eResult, init_halfops_cleanup);
		eResult = clSetKernelArg(psData->psMulKernel[i32TypeIndex], 0, sizeof(cl_mem), &inputBuffer );
		CheckAndReportError(psInstance, "clSetKernelArg", eResult, init_halfops_cleanup);
		eResult = clSetKernelArg(psData->psMulKernel[i32TypeIndex], 1, sizeof(cl_mem), (void*) &psData->psBufferA[i32TypeIndex][1] );
		CheckAndReportError(psInstance, "clSetKernelArg", eResult, init_halfops_cleanup);

		/* Init Mad kernel */
		OCLTestLog("Compiling Mad %s kernel...\n", halfTypeSizeNames[i32TypeIndex]);
		psData->psMadKernel[i32TypeIndex] = clCreateKernel(psData->psProgram[i32TypeIndex], "computeHalfMad", &eResult);
		CheckAndReportError(psInstance, "clCreateKernel", eResult, init_halfops_cleanup);
		eResult = clSetKernelArg(psData->psMadKernel[i32TypeIndex], 0, sizeof(cl_mem), &inputBuffer );
		CheckAndReportError(psInstance, "clSetKernelArg", eResult, init_halfops_cleanup);

		eResult = clSetKernelArg(psData->psMadKernel[i32TypeIndex], 1, sizeof(cl_mem), (void*) &psData->psBufferA[i32TypeIndex][2] );
		CheckAndReportError(psInstance, "clSetKernelArg", eResult, init_halfops_cleanup);

		/* Init Div kernel */
		OCLTestLog("Compiling Div %s kernel...\n", halfTypeSizeNames[i32TypeIndex]);
		psData->psDivKernel[i32TypeIndex] = clCreateKernel(psData->psProgram[i32TypeIndex], "computeHalfDiv", &eResult);
		CheckAndReportError(psInstance, "clCreateKernel", eResult, init_halfops_cleanup);
		eResult = clSetKernelArg(psData->psDivKernel[i32TypeIndex], 0, sizeof(cl_mem), &inputBuffer );
		CheckAndReportError(psInstance, "clSetKernelArg", eResult, init_halfops_cleanup);
		eResult = clSetKernelArg(psData->psDivKernel[i32TypeIndex], 1, sizeof(cl_mem), (void*) &psData->psBufferA[i32TypeIndex][3] );
		CheckAndReportError(psInstance, "clSetKernelArg", eResult, init_halfops_cleanup);

		/* Init SOP kernel */
		OCLTestLog("Compiling SOP %s kernel...\n", halfTypeSizeNames[i32TypeIndex]);
		psData->psSOPKernel[i32TypeIndex] = clCreateKernel(psData->psProgram[i32TypeIndex], "computeHalfSOP", &eResult);
		CheckAndReportError(psInstance, "clCreateKernel", eResult, init_halfops_cleanup);
		eResult = clSetKernelArg(psData->psSOPKernel[i32TypeIndex], 0, sizeof(cl_mem), &inputBuffer );
		CheckAndReportError(psInstance, "clSetKernelArg", eResult, init_halfops_cleanup);
		eResult = clSetKernelArg(psData->psSOPKernel[i32TypeIndex], 1, sizeof(cl_mem), (void*) &psData->psBufferA[i32TypeIndex][4] );
		CheckAndReportError(psInstance, "clSetKernelArg", eResult, init_halfops_cleanup);
		/* Setup the size of the kernel run */
		psData->puGlobalWorkSize[0][i32TypeIndex] = TEST_HALFOPS_MAX_INSTANCES;// / halfTypeDividers[i32TypeIndex];
		psData->puLocalWorkSize = NULL;

		OCLTestLog("%s: Reducing from %d to %zu due to use of %s in kernel.\n",__func__,TEST_HALFOPS_MAX_INSTANCES,
				   psData->puGlobalWorkSize[0][i32TypeIndex], halfTypeSizeNames[i32TypeIndex]);
	}

	return CL_SUCCESS;

init_halfops_cleanup:
	return eResult;
}

/***********************************************************************************
 Function Name      : Compute_Halfops
 Inputs             : psInstance - test instance data
 Returns            : 0 or 1
 Description        : Main compute func
************************************************************************************/
static cl_int
Compute_Halfops(OCLTestInstance *psInstance)
{
	cl_int eResult;
	size_t global_dim[1];
	int i32TypeIndex = 0;
	HalfopsData *psData =(HalfopsData*) psInstance->pvPrivateData;

	for (i32TypeIndex = 0; i32TypeIndex < TEST_HALFOPS_NUM_TYPE_SIZES; i32TypeIndex++)
	{
		OCLTestLog("%s: Online compilation test with %s using %zu instances, each with %d iterations\n",
			__func__,halfTypeSizeNames[i32TypeIndex], psData->puGlobalWorkSize[0][i32TypeIndex],TEST_HALFOPS_NUM_ITERATIONS);

	        /* Reduce to match vec4 output size */
	        global_dim[0] = psData->puGlobalWorkSize[0][i32TypeIndex];

		// Enqueue and complete Add kernel
		OCLTestLog("Running Add kernel for %s...\n", halfTypeSizeNames[i32TypeIndex]);
		eResult = clEnqueueNDRangeKernel(psData->psCommandQueue, psData->psAddKernel[i32TypeIndex], 1, NULL, global_dim, psData->puLocalWorkSize , 0, NULL, &psData->psAddCompleteEvent[i32TypeIndex]);
		CheckAndReportError(psInstance, "clEnqueueNDRangeKernel", eResult, compute_halfops_cleanup);
		clFinish(psData->psCommandQueue);

		// Enqueue and complete Mul kernel
		OCLTestLog("Running Mul kernel for %s...\n", halfTypeSizeNames[i32TypeIndex]);
		eResult = clEnqueueNDRangeKernel(psData->psCommandQueue, psData->psMulKernel[i32TypeIndex], 1, NULL, global_dim, psData->puLocalWorkSize , 0, NULL, &psData->psMulCompleteEvent[i32TypeIndex]);
		CheckAndReportError(psInstance, "clEnqueueNDRangeKernel", eResult, compute_halfops_cleanup);
		clFinish(psData->psCommandQueue);

		// Enqueue and complete Mad kernel
		OCLTestLog("Running Mad kernel for %s...\n",halfTypeSizeNames[i32TypeIndex]);
		eResult = clEnqueueNDRangeKernel(psData->psCommandQueue, psData->psMadKernel[i32TypeIndex], 1, NULL, global_dim, psData->puLocalWorkSize , 0, NULL, &psData->psMadCompleteEvent[i32TypeIndex]);
		CheckAndReportError(psInstance, "clEnqueueNDRangeKernel", eResult, compute_halfops_cleanup);
		clFinish(psData->psCommandQueue);

		// Enqueue and complete Div kernel
		OCLTestLog("Running Div kernel for %s...\n", halfTypeSizeNames[i32TypeIndex]);
		eResult = clEnqueueNDRangeKernel(psData->psCommandQueue, psData->psDivKernel[i32TypeIndex], 1, NULL, global_dim, psData->puLocalWorkSize , 0, NULL, &psData->psDivCompleteEvent[i32TypeIndex]);
		CheckAndReportError(psInstance, "clEnqueueNDRangeKernel", eResult, compute_halfops_cleanup);
		clFinish(psData->psCommandQueue);

		// Enqueue and complete SOP kernel
		OCLTestLog("Running SOP kernel for %s...\n", halfTypeSizeNames[i32TypeIndex]);
		eResult = clEnqueueNDRangeKernel(psData->psCommandQueue, psData->psSOPKernel[i32TypeIndex], 1, NULL, global_dim, psData->puLocalWorkSize , 0, NULL, &psData->psSOPCompleteEvent[i32TypeIndex]);
		CheckAndReportError(psInstance, "clEnqueueNDRangeKernel", eResult, compute_halfops_cleanup);
		clFinish(psData->psCommandQueue);

	}

	return CL_SUCCESS;

compute_halfops_cleanup:
	return eResult;
}

/***********************************************************************************
 Function Name      : Verify_Halfops
 Inputs             : None
 Outputs            : None
 Returns            : None
 Description        : Verifies output data
************************************************************************************/
static cl_int
Verify_Halfops(OCLTestInstance *psInstance)
{
	cl_int eResult = CL_SUCCESS;
	float* result =  (float*)malloc(sizeof(float)*5);
	int i = 0;
	int i32TypeIndex = 0;

	HalfopsData *psData =(HalfopsData*) psInstance->pvPrivateData;

	for (i32TypeIndex = 0; i32TypeIndex < TEST_HALFOPS_NUM_TYPE_SIZES; i32TypeIndex++)
	{
		cl_event apsProfilingEvents[] = { psData->psAddCompleteEvent[i32TypeIndex], psData->psMulCompleteEvent[i32TypeIndex],
						psData->psMadCompleteEvent[i32TypeIndex], psData->psDivCompleteEvent[i32TypeIndex],
						psData->psSOPCompleteEvent[i32TypeIndex]};
		char* apszTestNames[] = { "Add", "Mul", "Mad", "Div", "SOP" };
		unsigned int auOpsPerInstance[] = { 6+(TEST_HALFOPS_NUM_ITERATIONS*98), 4+(TEST_HALFOPS_NUM_ITERATIONS*98),
						4+(TEST_HALFOPS_NUM_ITERATIONS*196/*mad 98+98*/), 4+(TEST_HALFOPS_NUM_ITERATIONS*98),
						4+(TEST_HALFOPS_NUM_ITERATIONS*51)};

		/* Print out some metrics for each kernel */
		for (i = 0; i != 5; i++)
		{
			cl_ulong ulStart;
			cl_ulong ulEnd;
			unsigned long long ullIO;
			double fTimeInSeconds;
			double fIOPS;
			double fGIOPS;
			double fTotalGIOp;

			/* Print out timing information (command queue had profiling enabled) */
			eResult = clGetEventProfilingInfo(apsProfilingEvents[i],CL_PROFILING_COMMAND_START,sizeof(cl_ulong),&ulStart,NULL);
			CheckAndReportError(psInstance, "clGetEventProfilingInfo", eResult, verify_halfops_cleanup);
			eResult = clGetEventProfilingInfo(apsProfilingEvents[i],CL_PROFILING_COMMAND_END  ,sizeof(cl_ulong),&ulEnd,NULL);
			CheckAndReportError(psInstance, "clGetEventProfilingInfo", eResult, verify_halfops_cleanup);

			fTimeInSeconds = ((double)(ulEnd-ulStart)) / 1000000000.0;

			/* Calculate the total number of float operations */
			ullIO = auOpsPerInstance[i] * halfTypeDividers[i32TypeIndex] /* operating on floatX */;
			ullIO *= psData->puGlobalWorkSize[0][i32TypeIndex];

			/* Divide by the time to get the IOPS */
			fIOPS = ullIO / fTimeInSeconds;

			/* Convert to GIGA IOps */
			fGIOPS = fIOPS / 1000000000.0;

			fTotalGIOp = ullIO / 1000000000.0;

			OCLTestLog("---------------\n");
			OCLTestLog("%s %s:\n", halfTypeSizeNames[i32TypeIndex], apszTestNames[i]);
			OCLTestLog("---------------\n");
			OCLTestLog("%s: Time\t\t%fs\n",__func__,fTimeInSeconds);
			OCLTestLog("%s: Iterations\t%d\n",__func__,TEST_HALFOPS_NUM_ITERATIONS);
			OCLTestLog("%s: Instances\t%zu\n",__func__,psData->puGlobalWorkSize[0][i32TypeIndex]);
			if ( i != 4 )
			{
				OCLTestLog("%s: Float operations\t%llu\n",__func__,ullIO);
				OCLTestLog("%s: GFLOP/S\t\t%f\n",__func__,fGIOPS);
			}
			else
			{
				OCLTestLog("%s: SOP operations\t%llu\n",__func__,ullIO);
				OCLTestLog("%s: Float perations\t%llu\n",__func__,ullIO*3);
				OCLTestLog("%s: GSOP/S\t\t%f\n",__func__,fGIOPS);
				OCLTestLog("%s: GFLOP/S\t\t%f\n",__func__,fGIOPS*3);
			}
#if !defined(NO_HARDWARE)

		            {
		                result[0] = -0.5;
						result[1] = -0.5;
						result[2] = -0.5;
						result[3] = -0.5;
		            }

		            eResult = clEnqueueReadBuffer(psData->psCommandQueue, psData->psBufferA[i32TypeIndex][i], CL_TRUE, 0, sizeof(float)*4, result, 0, NULL, NULL);
		            CheckAndReportError(psInstance, "clEnqueueReadBuffer", eResult, verify_halfops_cleanup);
		            {
		                OCLTestLog("Res: %f\n",result[0]);
		                OCLTestLog("Res: %f\n",result[1]);
		                OCLTestLog("Res: %f\n",result[2]);
				OCLTestLog("Res: %f\n",result[3]);
		            }
#endif
		}
		OCLTestLog("\n");
	}

	/* Cleanup */
	for (i32TypeIndex = 0; i32TypeIndex < TEST_HALFOPS_NUM_TYPE_SIZES; i32TypeIndex++)
	{
		clReleaseEvent       ( psData->psAddCompleteEvent[i32TypeIndex]);
		clReleaseEvent       ( psData->psMulCompleteEvent[i32TypeIndex]);
		clReleaseEvent       ( psData->psMadCompleteEvent[i32TypeIndex]);
		clReleaseEvent       ( psData->psDivCompleteEvent[i32TypeIndex]);
		clReleaseEvent       ( psData->psSOPCompleteEvent[i32TypeIndex]);
		clReleaseKernel      ( psData->psAddKernel[i32TypeIndex]);
		clReleaseKernel      ( psData->psMulKernel[i32TypeIndex]);
		clReleaseKernel      ( psData->psMadKernel[i32TypeIndex]);
		clReleaseKernel      ( psData->psDivKernel[i32TypeIndex]);
		clReleaseKernel      ( psData->psSOPKernel[i32TypeIndex]);
	        clReleaseMemObject   ( psData->psBufferA[i32TypeIndex][0]);
	        clReleaseMemObject   ( psData->psBufferA[i32TypeIndex][1]);
	        clReleaseMemObject   ( psData->psBufferA[i32TypeIndex][2]);
	        clReleaseMemObject   ( psData->psBufferA[i32TypeIndex][3]);
	        clReleaseMemObject   ( psData->psBufferA[i32TypeIndex][4]);
	}


	clReleaseCommandQueue( psData->psCommandQueue );
	for (i32TypeIndex = 0; i32TypeIndex < TEST_HALFOPS_NUM_TYPE_SIZES; i32TypeIndex++)
	{
		clReleaseProgram     ( psData->psProgram[i32TypeIndex]);
	}
	clReleaseContext     ( psData->psContext      );

	free(psData->pfBuffer);
	free(result);

	free(psInstance->pvPrivateData);
	psInstance->pvPrivateData = NULL;
	return eResult;

verify_halfops_cleanup:
	return eResult;
}

/******************************************************************************
 End of file (halfvec.c)
******************************************************************************/
