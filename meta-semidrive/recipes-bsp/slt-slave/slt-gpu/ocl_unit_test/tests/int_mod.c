/*************************************************************************/ /*!
@File           int_mod.c
@Copyright      Copyright (c) Imagination Technologies Ltd. All Rights Reserved
@License        Strictly Confidential.
*/ /**************************************************************************/

#include <unistd.h>
#include <sys/time.h>

#define TOSTR(x) #x
#define STR(x) TOSTR(x)
#ifdef CUT_DOWN_UNIT_TEST
#define TEST_INTOPS_MOD_MAX_INSTANCES  (1024)
#define TEST_INTOPS_MOD_NUM_ITERATIONS 10
#else
#define TEST_INTOPS_MOD_MAX_INSTANCES  (32768)
#define TEST_INTOPS_MOD_NUM_ITERATIONS 200
#endif
#define TEST_INTOPS_MOD_NUM_TYPE_SIZES	  4

typedef struct _IntopsDataMod_
{
	/* CL Objects */
	cl_context       psContext;
	cl_program       psProgram[TEST_INTOPS_MOD_NUM_TYPE_SIZES];
	cl_device_id     psDeviceID;
	cl_platform_id   psPlatformID;
	cl_command_queue psCommandQueue;

	cl_kernel		 psDivKernel[TEST_INTOPS_MOD_NUM_TYPE_SIZES];
	cl_kernel		 psDivKernelFast[TEST_INTOPS_MOD_NUM_TYPE_SIZES];

	cl_mem           psBufferA[TEST_INTOPS_MOD_NUM_TYPE_SIZES];

	cl_event		 psDivCompleteEvent[TEST_INTOPS_MOD_NUM_TYPE_SIZES];
	cl_event		 psDivCompleteEventFast[TEST_INTOPS_MOD_NUM_TYPE_SIZES];

	/* Host Object */
	unsigned int *pui32BufferA;
	size_t        puGlobalWorkSize[3][TEST_INTOPS_MOD_NUM_TYPE_SIZES];
	size_t*       puLocalWorkSize;
} IntopsDataMod;

static char* intTypeSizeNamesMod[TEST_MEMCPY_NUM_TYPE_SIZES] = {"uint", "uint2", "uint3", "uint4", "uint8", "uint16"};
static int intTypeDividersMod[TEST_MEMCPY_NUM_TYPE_SIZES] = {1, 2, 3, 4, 8, 16};
static char *g_pszIntopsSourceMod =
{
"#define TEST_INTOPS_MOD_NUM_ITERATIONS " STR(TEST_INTOPS_MOD_NUM_ITERATIONS) \
"\n"
"#define INTOPS_VARIANT %s\n"
"__kernel void computeIntegerDiv(uint in,\n"
"				 uint mod,\n"
"				  __global INTOPS_VARIANT *output)\n"
"{\n"
"	// 3 integer operations\n"
"	INTOPS_VARIANT a = INT_MAX / in;\n"
"	INTOPS_VARIANT b = (INT_MAX-3) / in;\n"
"	INTOPS_VARIANT c = (INT_MAX-7) / in;\n"
"\n"
"	for (unsigned int i = 0; i < TEST_INTOPS_MOD_NUM_ITERATIONS; ++i)\n"
"	{\n"
"		a = (c %% mod) + a;\n"
"		b = (c %% mod) + b;\n"
"		a = (c %% mod) + (a %% mod);\n"
"		b = (c %% mod) + (b %% mod);\n"
"		a = (c %% mod) + (a %% mod);\n"
"		b = (c %% mod) + (b %% mod);\n"
"		a = (c %% mod) + (a %% mod);\n"
"		b = (c %% mod) + (b %% mod);\n"
"		a = (c %% mod) + (a %% mod);\n"
"		b = (c %% mod) + (b %% mod);\n"
"		a = (c %% mod) + (a %% mod);\n"
"		b = (c %% mod) + (b %% mod);\n"
"		a = (c %% mod) + (a %% mod);\n"
"		b = (c %% mod) + (b %% mod);\n"
"		a = (c %% mod) + (a %% mod);\n"
"		b = (c %% mod) + (b %% mod);\n"
"		a = (c %% mod) + (a %% mod);\n"
"		b = (c %% mod) + (b %% mod);\n"
"		a = (c %% mod) + (a %% mod);\n"
"		b = (c %% mod) + (b %% mod);\n"
"	}\n"
"\n"
"	*output = a / b;\n"
"}\n"
"\n"
"__kernel void computeIntegerDivFast(uint in,\n"
"				 uint mod,\n"
"				  __global INTOPS_VARIANT *output)\n"
"{\n"
"	// 3 integer operations\n"
"	INTOPS_VARIANT a = INT_MAX / in;\n"
"	INTOPS_VARIANT b = (INT_MAX-3) / in;\n"
"	INTOPS_VARIANT c = (INT_MAX-7) / in;\n"
"\n"
"	for (unsigned int i = 0; i < TEST_INTOPS_MOD_NUM_ITERATIONS; ++i)\n"
"	{\n"
"		a = (c %% mod) + a;\n"
"		b = (c %% mod) + b;\n"
"		a = (c %% mod) + (a %% mod);\n"
"		b = (c %% mod) + (b %% mod);\n"
"		a = (c %% mod) + (a %% mod);\n"
"		b = (c %% mod) + (b %% mod);\n"
"		a = (c %% mod) + (a %% mod);\n"
"		b = (c %% mod) + (b %% mod);\n"
"		a = (c %% mod) + (a %% mod);\n"
"		b = (c %% mod) + (b %% mod);\n"
"		a = (c %% mod) + (a %% mod);\n"
"		b = (c %% mod) + (b %% mod);\n"
"		a = (c %% mod) + (a %% mod);\n"
"		b = (c %% mod) + (b %% mod);\n"
"		a = (c %% mod) + (a %% mod);\n"
"		b = (c %% mod) + (b %% mod);\n"
"		a = (c %% mod) + (a %% mod);\n"
"		b = (c %% mod) + (b %% mod);\n"
"		a = (c %% mod) + (a %% mod);\n"
"		b = (c %% mod) + (b %% mod);\n"
"	}\n"
"\n"
"	*output = a / b;\n"
"}\n"
"\n"

};

/***********************************************************************************
 Function Name      : Init_Int
 Inputs             : None
 Outputs            : None
 Returns            : None
 Description        : Initialises input data
************************************************************************************/
static cl_int
Init_IntMod(OCLTestInstance *psInstance)
{
	size_t auLengths[2] = {0,0};
	char *ppszSource[2] = { 0 };
	char programSource[8192];
	int iValue = 1;
	int i32TypeIndex = 0;
	cl_int eResult;
	unsigned int uiMod, uiModFast;
	IntopsDataMod *psData = (IntopsDataMod*)calloc(1, sizeof(IntopsDataMod));
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
	CheckAndReportError(psInstance, "clGetPlatformIDs", eResult, init_int_cleanup);

	eResult = clGetDeviceIDs(psData->psPlatformID,CL_DEVICE_TYPE_GPU,1,&psData->psDeviceID,NULL);
	CheckAndReportError(psInstance, "clGetDeviceIDs", eResult, init_int_cleanup);

	psData->psContext = clCreateContext(NULL,1,&psData->psDeviceID,NULL,NULL,&eResult);
	CheckAndReportError(psInstance, "clCreateContext", eResult, init_int_cleanup);

	psData->psCommandQueue = clCreateCommandQueue(psData->psContext, psData->psDeviceID, CL_QUEUE_PROFILING_ENABLE, &eResult);
	CheckAndReportError(psInstance, "clCreateCommandQueue", eResult, init_int_cleanup);

	for (i32TypeIndex = 0; i32TypeIndex < TEST_INTOPS_MOD_NUM_TYPE_SIZES; i32TypeIndex++)
	{
		/* Use the global int program */
		programSource[0] = 0;
		snprintf(programSource, 8192, g_pszIntopsSourceMod, intTypeSizeNamesMod[i32TypeIndex]);
		auLengths[0]  = strlen(programSource);
		ppszSource[0] = programSource;
		/*
		* Build the Program
		*/

		psData->psProgram[i32TypeIndex] = clCreateProgramWithSource(psData->psContext, 1, (const char**)ppszSource, auLengths, &eResult);
		CheckAndReportError(psInstance, "clCreateProgramWithSource", eResult, init_int_cleanup);

		eResult = clBuildProgram(psData->psProgram[i32TypeIndex],1,&psData->psDeviceID,"-cl-fast-relaxed-math",NULL,NULL);

		if(eResult != CL_SUCCESS)
		{
			char         aszBuildLog[512];
			size_t       uBuildLogSize;

			eResult = clGetProgramBuildInfo(psData->psProgram[i32TypeIndex], psData->psDeviceID, CL_PROGRAM_BUILD_LOG, 512, aszBuildLog, &uBuildLogSize);
			CheckAndReportError(psInstance, "clGetProgramBuildInfo", eResult, init_int_cleanup);

			aszBuildLog[uBuildLogSize-1] = '\0';
			OCLTestLog("[%zu] CL_PROGRAM_BUILD_LOG:\n%s\n",uBuildLogSize,aszBuildLog);
		}
		CheckAndReportError(psInstance, "clBuildProgram", eResult, init_int_cleanup);

		/*
		* Setup the Arguments
		*/
		psData->psBufferA[i32TypeIndex] = clCreateBuffer(psData->psContext, CL_MEM_COPY_HOST_PTR, sizeof(cl_int), psData->pui32BufferA, &eResult);
		CheckAndReportError(psInstance, "clCreateBuffer", eResult, init_int_cleanup);

		/* Init Div kernel */
		OCLTestLog("Compiling Div %s kernel...\n", intTypeSizeNamesMod[i32TypeIndex]);
		psData->psDivKernel[i32TypeIndex] = clCreateKernel(psData->psProgram[i32TypeIndex], "computeIntegerDiv", &eResult);
		CheckAndReportError(psInstance, "clCreateKernel", eResult, init_int_cleanup);

		eResult = clSetKernelArg(psData->psDivKernel[i32TypeIndex], 0, sizeof(cl_int), (void*) &iValue );
		CheckAndReportError(psInstance, "clSetKernelArg", eResult, init_int_cleanup);
		uiMod = 3;
		eResult = clSetKernelArg(psData->psDivKernel[i32TypeIndex], 1, sizeof(cl_uint), (void*) &uiMod);

		eResult = clSetKernelArg(psData->psDivKernel[i32TypeIndex], 2, sizeof(cl_mem), (void*) &psData->psBufferA[i32TypeIndex] );
		CheckAndReportError(psInstance, "clSetKernelArg", eResult, init_int_cleanup);

		/* Init Div kernel fast*/
		OCLTestLog("Compiling Div Fast %s kernel...\n", intTypeSizeNamesMod[i32TypeIndex]);
		psData->psDivKernelFast[i32TypeIndex] = clCreateKernel(psData->psProgram[i32TypeIndex], "computeIntegerDivFast", &eResult);
		CheckAndReportError(psInstance, "clCreateKernel", eResult, init_int_cleanup);

		eResult = clSetKernelArg(psData->psDivKernelFast[i32TypeIndex], 0, sizeof(cl_int), (void*) &iValue );
		CheckAndReportError(psInstance, "clSetKernelArg", eResult, init_int_cleanup);
		uiModFast = 2;
		eResult = clSetKernelArg(psData->psDivKernelFast[i32TypeIndex], 1, sizeof(cl_uint), (void*) &uiModFast);

		eResult = clSetKernelArg(psData->psDivKernelFast[i32TypeIndex], 2, sizeof(cl_mem), (void*) &psData->psBufferA[i32TypeIndex] );
		CheckAndReportError(psInstance, "clSetKernelArg", eResult, init_int_cleanup);

		/* Setup the size of the kernel run */
		psData->puGlobalWorkSize[0][i32TypeIndex] = TEST_INTOPS_MOD_MAX_INSTANCES;// / intTypeDividersMod[i32TypeIndex];
		psData->puLocalWorkSize = NULL;
	}

	return CL_SUCCESS;

init_int_cleanup:
	return eResult;
}

/***********************************************************************************
 Function Name      : Compute_Int
 Inputs             : psInstance - test instance data
 Returns            : 0 or 1
 Description        : Main compute func
************************************************************************************/
static cl_int
Compute_IntMod(OCLTestInstance *psInstance)
{
	cl_int eResult;
	size_t global_dim[1];
	int i32TypeIndex = 0;
	IntopsDataMod *psData =(IntopsDataMod*) psInstance->pvPrivateData;
	for (i32TypeIndex = 0; i32TypeIndex < TEST_INTOPS_MOD_NUM_TYPE_SIZES; i32TypeIndex++)
	{
		OCLTestLog("%s: Online compilation test with %s using %zu instances, each with %d iterations\n",
			__func__,intTypeSizeNamesMod[i32TypeIndex], psData->puGlobalWorkSize[0][i32TypeIndex],TEST_INTOPS_MOD_NUM_ITERATIONS);

		/* Reduce to match vec4 output size */
		global_dim[0] = psData->puGlobalWorkSize[0][i32TypeIndex];
		// Enqueue and complete Div kernel
		OCLTestLog("Running Div kernel for %s...\n", intTypeSizeNamesMod[i32TypeIndex]);
		eResult = clEnqueueNDRangeKernel(psData->psCommandQueue, psData->psDivKernel[i32TypeIndex], 1, NULL, global_dim, psData->puLocalWorkSize , 0, NULL, &psData->psDivCompleteEvent[i32TypeIndex]);
		CheckAndReportError(psInstance, "clEnqueueNDRangeKernel", eResult, compute_int_cleanup);

		clFinish(psData->psCommandQueue);
		// Enqueue and complete Div kernel
		OCLTestLog("Running Div kernel Fast for %s...\n", intTypeSizeNamesMod[i32TypeIndex]);
		eResult = clEnqueueNDRangeKernel(psData->psCommandQueue, psData->psDivKernelFast[i32TypeIndex], 1, NULL, global_dim, psData->puLocalWorkSize , 0, NULL, &psData->psDivCompleteEventFast[i32TypeIndex]);
		CheckAndReportError(psInstance, "clEnqueueNDRangeKernel", eResult, compute_int_cleanup);

		clFinish(psData->psCommandQueue);
	}
	return eResult;

compute_int_cleanup:
	return eResult;
}

/***********************************************************************************
 Function Name      : Verify_Int
 Inputs             : None
 Outputs            : None
 Returns            : None
 Description        : Verifies output data
************************************************************************************/
static cl_int
Verify_IntMod(OCLTestInstance *psInstance)
{
	cl_int eResult = CL_SUCCESS;
	int i = 0;
	int i32TypeIndex = 0;

	IntopsDataMod *psData =(IntopsDataMod*) psInstance->pvPrivateData;

	for (i32TypeIndex = 0; i32TypeIndex < TEST_INTOPS_MOD_NUM_TYPE_SIZES; i32TypeIndex++)
	{
	  cl_event apsProfilingEvents[] = {psData->psDivCompleteEvent[i32TypeIndex],psData->psDivCompleteEventFast[i32TypeIndex] };

	  char* apszTestNames[] = { "Div", "Div fast" };
	  unsigned int auOpsPerInstance[] = {4+(TEST_INTOPS_MOD_NUM_ITERATIONS*58), 4+(TEST_INTOPS_MOD_NUM_ITERATIONS*58)};

		/* Print out some metrics for each kernel */
		for (i = 0; i != 2; i++)
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
			CheckAndReportError(psInstance, "clGetEventProfilingInfo", eResult, verify_int_cleanup);
			eResult = clGetEventProfilingInfo(apsProfilingEvents[i],CL_PROFILING_COMMAND_END  ,sizeof(cl_ulong),&ulEnd,NULL);
			CheckAndReportError(psInstance, "clGetEventProfilingInfo", eResult, verify_int_cleanup);

			fTimeInSeconds = ((double)(ulEnd-ulStart)) / 1000000000.0;

			/* Calculate the total number of int operations */
			ullIO = auOpsPerInstance[i] * intTypeDividersMod[i32TypeIndex] /* operating on intX */;
			ullIO *= psData->puGlobalWorkSize[0][i32TypeIndex];

			/* Divide by the time to get the IOPS */
			fIOPS = ullIO / fTimeInSeconds;

			/* Convert to GIGA IOps */
			fGIOPS = fIOPS / 1000000000.0;

			fTotalGIOp = ullIO / 1000000000.0;

			OCLTestLog("---------------\n");
			OCLTestLog("%s %s:\n", intTypeSizeNamesMod[i32TypeIndex], apszTestNames[i]);
			OCLTestLog("---------------\n");
			OCLTestLog("%s: Time                      %fs\n",__func__,fTimeInSeconds);
			OCLTestLog("%s: Iterations                %d\n",__func__,TEST_INTOPS_MOD_NUM_ITERATIONS);
			OCLTestLog("%s: Instances                 %zu\n",__func__,psData->puGlobalWorkSize[0][i32TypeIndex]);
			OCLTestLog("%s: Integer operations %llu\n",__func__,ullIO);
			OCLTestLog("%s: GIOp                     %f\n",__func__,fTotalGIOp);
			OCLTestLog("%s: GIOP/S                   %f\n",__func__,fGIOPS);
			//OCLTestLog("%s: IOPS                     %f\n",__func__,fIOPS);
			OCLMetricOutputDouble2(__func__,intTypeSizeNamesMod[i32TypeIndex],apszTestNames[i],fGIOPS,GIOPSEC);
		}
		OCLTestLog("\n");
	}
	/* Cleanup */
	for (i32TypeIndex = 0; i32TypeIndex < TEST_INTOPS_MOD_NUM_TYPE_SIZES; i32TypeIndex++)
	{
		clReleaseEvent       ( psData->psDivCompleteEvent[i32TypeIndex]);
		clReleaseKernel      ( psData->psDivKernel[i32TypeIndex]);
		clReleaseEvent       ( psData->psDivCompleteEventFast[i32TypeIndex]);
		clReleaseKernel      ( psData->psDivKernelFast[i32TypeIndex]);
		clReleaseMemObject   ( psData->psBufferA[i32TypeIndex]);
	}

	clReleaseCommandQueue( psData->psCommandQueue );
	for (i32TypeIndex = 0; i32TypeIndex < TEST_INTOPS_MOD_NUM_TYPE_SIZES; i32TypeIndex++)
	{
		clReleaseProgram     ( psData->psProgram[i32TypeIndex]);
	}
	clReleaseContext     ( psData->psContext      );

	free(psData->pui32BufferA);

	free(psInstance->pvPrivateData);
	psInstance->pvPrivateData = NULL;
	return eResult;

verify_int_cleanup:
	return eResult;
}

/******************************************************************************
 End of file (int_mod.c)
******************************************************************************/
