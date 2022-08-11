/*************************************************************************/ /*!
@File           char.c
@Copyright      Copyright (c) Imagination Technologies Ltd. All Rights Reserved
@License        Strictly Confidential.
*/ /**************************************************************************/

#include <unistd.h>
#include <sys/time.h>

#define TOSTR(x) #x
#define STR(x) TOSTR(x)
#ifdef CUT_DOWN_UNIT_TEST
#define TEST_CHAR_MAX_INSTANCES  (1024)
#define TEST_CHAR_NUM_ITERATIONS 10
#else
#define TEST_CHAR_MAX_INSTANCES  (32768)
#define TEST_CHAR_NUM_ITERATIONS 200
#endif

#define TEST_CHAR_NUM_TYPE_SIZES	  4

typedef struct _CharData_
{
	/* CL Objects */
	cl_context       psContext;
	cl_program       psProgram[TEST_CHAR_NUM_TYPE_SIZES];
	cl_device_id     psDeviceID;
	cl_platform_id   psPlatformID;
	cl_command_queue psCommandQueue;

	cl_kernel        psAddKernel[TEST_CHAR_NUM_TYPE_SIZES];
	cl_kernel        psMulKernel[TEST_CHAR_NUM_TYPE_SIZES];
	cl_kernel        psMadKernel[TEST_CHAR_NUM_TYPE_SIZES];
	cl_kernel        psDivKernel[TEST_CHAR_NUM_TYPE_SIZES];

	cl_mem           psInputBuffer[TEST_CHAR_NUM_TYPE_SIZES];

	cl_event         psAddCompleteEvent[TEST_CHAR_NUM_TYPE_SIZES];
	cl_event         psMulCompleteEvent[TEST_CHAR_NUM_TYPE_SIZES];
	cl_event         psMadCompleteEvent[TEST_CHAR_NUM_TYPE_SIZES];
	cl_event         psDivCompleteEvent[TEST_CHAR_NUM_TYPE_SIZES];

	/* Host Object */
	unsigned char *pui32BufferA;
	size_t        puGlobalWorkSize[3][TEST_CHAR_NUM_TYPE_SIZES];
	size_t*       puLocalWorkSize;
} CharData;

static char* charTypeSizeNames[TEST_MEMCPY_NUM_TYPE_SIZES] = {"char", "char2", "char3", "char4", "char8", "char16"};
static int charTypeDividers[TEST_MEMCPY_NUM_TYPE_SIZES] = {1, 2, 3, 4, 8, 16};
static char *g_pszCharSource =
{
"#define TEST_CHAR_NUM_ITERATIONS " STR(TEST_CHAR_NUM_ITERATIONS) \
"\n"
"#define CHAR_VARIANT %s\n"
"__kernel void computeCharAdd(char in,\n"
"				  __global CHAR_VARIANT *output)\n"
"{\n"
"	// 5 char ops\n"
"	CHAR_VARIANT a = 1 + in;\n"
"	CHAR_VARIANT b = 2 + in;\n"
"	CHAR_VARIANT c = 3 + in;\n"
"	CHAR_VARIANT d = 4 + in;\n"
"	CHAR_VARIANT e = 5 + in;\n"
"\n"
"	for (unsigned int i = 0; i < TEST_CHAR_NUM_ITERATIONS; ++i)\n"
"	{\n"
"		a = b + e;\n"
"		b = c + e;\n"
"		c = c + a + d;\n"
"		d = c + b + e;\n"
"		e = c + a + e;\n"
"		a = c + b + d;\n"
"		b = c + a + d;\n"
"		c = c + b + e;\n"
"		d = c + a + e;\n"
"		e = c + b + d;\n"
"		a = c + a + d;\n"
"		b = c + b + e;\n"
"		c = c + a + e;\n"
"		d = c + b + d;\n"
"		e = c + a + d;\n"
"		a = c + b + e;\n"
"		b = c + a + e;\n"
"		c = c + b + d;\n"
"		d = c + a + e;\n"
"		e = c + b + d;\n"
"	}\n"
"\n"
"	*output = a + b;\n"
"}\n"
"\n"
"__kernel void computeCharMul(char in,\n"
"				  __global CHAR_VARIANT *output)\n"
"{\n"
"	// 3 char operations\n"
"	CHAR_VARIANT a = 3 * in;\n"
"	CHAR_VARIANT b = 5 * in;\n"
"	CHAR_VARIANT c = 7 * in;\n"
"\n"
"	for (unsigned int i = 0; i < TEST_CHAR_NUM_ITERATIONS; ++i)\n"
"	{\n"
"		a = c * a;\n"
"		b = c * b;\n"
"		a = c * a * a;\n"
"		b = c * b * a;\n"
"		a = c * a * b;\n"
"		b = c * b * b;\n"
"		a = c * a * a;\n"
"		b = c * b * b;\n"
"		a = c * a * a;\n"
"		b = c * b * b;\n"
"		a = c * a * a;\n"
"		b = c * b * a;\n"
"		a = c * a * b;\n"
"		b = c * b * b;\n"
"	}\n"
"\n"
"	*output = a * b;\n"
"}\n"
"\n"
"__kernel void computeCharMad(char in,\n"
"                 __global CHAR_VARIANT *output)\n"
"{\n"
"   // 3 char operations\n"
"   CHAR_VARIANT a = 3 * in;\n"
"   CHAR_VARIANT b = 5 * in;\n"
"   CHAR_VARIANT c = 7 * in;\n"
"\n"
"   for (unsigned int i = 0; i < TEST_CHAR_NUM_ITERATIONS; ++i)\n"
"   {\n"
"       a = c * a + b;\n"
"       b = c * b + a;\n"
"       a = c * a + a;\n"
"       b = c * b + b;\n"
"       a = c * a + a;\n"
"       b = c * b + b;\n"
"       a = c * a + a;\n"
"       b = c * b + b;\n"
"       a = c * a + a;\n"
"       b = c * b + b;\n"
"       a = c * a + a;\n"
"       b = c * b + b;\n"
"       a = c * a + a;\n"
"       b = c * b + b;\n"
"       a = c * a + a;\n"
"       b = c * b + b;\n"
"       a = c * a + a;\n"
"       b = c * b + b;\n"
"       a = c * a + a;\n"
"       b = c * b + b;\n"
"       a = c * a + b;\n"
"       b = c * b + a;\n"
"       a = c * a + a;\n"
"       b = c * b + b;\n"
"       a = c * a + a;\n"
"       b = c * b + b;\n"
"       a = c * a + a;\n"
"       b = c * b + b;\n"
"       a = c * a + a;\n"
"       b = c * b + b;\n"
"       a = c * a + a;\n"
"       b = c * b + b;\n"
"       a = c * a + a;\n"
"       b = c * b + b;\n"
"       a = c * a + a;\n"
"       b = c * b + b;\n"
"       a = c * a + a;\n"
"       b = c * b + b;\n"
"       a = c * a + a;\n"
"       b = c * b + b;\n"
"   }\n"
"	// 4 + (TEST_CHAR_NUM_ITERATIONS * 98) integer operations\n"
"	*output = a * b + c;\n"
"}\n"
"\n"
"__kernel void computeCharDiv(char in,\n"
"				  __global CHAR_VARIANT *output)\n"
"{\n"
"	// 3 integer operations\n"
"	CHAR_VARIANT a = 127 / in;\n"
"	CHAR_VARIANT b = 125 / in;\n"
"	CHAR_VARIANT c = 123 / in;\n"
"\n"
"	for (unsigned int i = 0; i < TEST_CHAR_NUM_ITERATIONS; ++i)\n"
"	{\n"
"		a = c / a;\n"
"		b = c / b;\n"
"		a = c / a / a;\n"
"		b = c / b / a;\n"
"		a = c / a / b;\n"
"		b = c / b / b;\n"
"		a = c / a / a;\n"
"		b = c / b / b;\n"
"		a = c / a / a;\n"
"		b = c / b / b;\n"
"		a = c / a / a;\n"
"		b = c / b / a;\n"
"		a = c / a / b;\n"
"		b = c / b / b;\n"
"		a = c / a / a;\n"
"		b = c / b / b;\n"
"		a = c / a / a;\n"
"		b = c / b / b;\n"
"		a = c / a / a;\n"
"		b = c / b / a;\n"
"	}\n"
"\n"
"	*output = a / b;\n"
"}\n"

};

/***********************************************************************************
 Function Name      : Init_Char
 Inputs             : None
 Outputs            : None
 Returns            : None
 Description        : Initialises input data
************************************************************************************/
static cl_int
Init_Char(OCLTestInstance *psInstance)
{
	size_t auLengths[2] = {0,0};
	char *ppszSource[2] = { 0 };
	char programSource[8192];
	char iValue = 1;
	int i32TypeIndex = 0;
	cl_int eResult;

	CharData *psData = (CharData*)calloc(1, sizeof(CharData));
	if (!psData)
	{
		return CL_OUT_OF_RESOURCES;
	}

	/* Initialise data */
	psInstance->pvPrivateData = (void*)psData;

	/* Allocate second host buffer */
	psData->pui32BufferA = (unsigned char*)malloc(sizeof(unsigned char));

	if(!psData->pui32BufferA)
	{
		free(psData);
		return CL_OUT_OF_RESOURCES;
	}

	/* Initialise with test values */
	psData->pui32BufferA[0] = 0;

	eResult = clGetPlatformIDs(1,&psData->psPlatformID,NULL);
	CheckAndReportError(psInstance, "clGetPlatformIDs", eResult, init_char_cleanup);

	eResult = clGetDeviceIDs(psData->psPlatformID,CL_DEVICE_TYPE_GPU,1,&psData->psDeviceID,NULL);
	CheckAndReportError(psInstance, "clGetDeviceIDs", eResult, init_char_cleanup);

	psData->psContext = clCreateContext(NULL,1,&psData->psDeviceID,NULL,NULL,&eResult);
	CheckAndReportError(psInstance, "clCreateContext", eResult, init_char_cleanup);

	psData->psCommandQueue = clCreateCommandQueue(psData->psContext, psData->psDeviceID, CL_QUEUE_PROFILING_ENABLE, &eResult);
	CheckAndReportError(psInstance, "clCreateCommandQueue", eResult, init_char_cleanup);

	for (i32TypeIndex = 0; i32TypeIndex < TEST_CHAR_NUM_TYPE_SIZES; i32TypeIndex++)
	{
		/* Use the global char program */
		programSource[0] = 0;
		snprintf(programSource, 8192, g_pszCharSource, charTypeSizeNames[i32TypeIndex]);
		auLengths[0]  = strlen(programSource);
		ppszSource[0] = programSource;
		/*
		* Build the Program
		*/

		psData->psProgram[i32TypeIndex] = clCreateProgramWithSource(psData->psContext, 1, (const char**)ppszSource, auLengths, &eResult);
		CheckAndReportError(psInstance, "clCreateProgramWithSource", eResult, init_char_cleanup);

		eResult = clBuildProgram(psData->psProgram[i32TypeIndex],1,&psData->psDeviceID,"-cl-fast-relaxed-math",NULL,NULL);

		if(eResult != CL_SUCCESS)
		{
			char         aszBuildLog[512];
			size_t       uBuildLogSize;

			eResult = clGetProgramBuildInfo(psData->psProgram[i32TypeIndex], psData->psDeviceID, CL_PROGRAM_BUILD_LOG, 512, aszBuildLog, &uBuildLogSize);
			CheckAndReportError(psInstance, "clGetProgramBuildInfo", eResult, init_char_cleanup);

			aszBuildLog[uBuildLogSize-1] = '\0';
			OCLTestLog("[%zu] CL_PROGRAM_BUILD_LOG:\n%s\n",uBuildLogSize,aszBuildLog);
		}
		CheckAndReportError(psInstance, "clBuildProgram", eResult, init_char_cleanup);

		/*
		* Setup the Arguments
		*/
		psData->psInputBuffer[i32TypeIndex] = clCreateBuffer(psData->psContext, CL_MEM_COPY_HOST_PTR, sizeof(cl_char), psData->pui32BufferA, &eResult);
		CheckAndReportError(psInstance, "clCreateBuffer", eResult, init_char_cleanup);

		/* Init Add kernel */
		OCLTestLog("Compiling Add %s kernel...\n", charTypeSizeNames[i32TypeIndex]);
		psData->psAddKernel[i32TypeIndex] = clCreateKernel(psData->psProgram[i32TypeIndex], "computeCharAdd", &eResult);
		CheckAndReportError(psInstance, "clCreateKernel", eResult, init_char_cleanup);

		eResult = clSetKernelArg(psData->psAddKernel[i32TypeIndex], 0, sizeof(cl_char), (void*) &iValue );
		CheckAndReportError(psInstance, "clSetKernelArg", eResult, init_char_cleanup);

		eResult = clSetKernelArg(psData->psAddKernel[i32TypeIndex], 1, sizeof(cl_mem), (void*) &psData->psInputBuffer[i32TypeIndex] );
		CheckAndReportError(psInstance, "clSetKernelArg", eResult, init_char_cleanup);

		/* Init Mul kernel */
		OCLTestLog("Compiling Mul %s kernel...\n", charTypeSizeNames[i32TypeIndex]);
		psData->psMulKernel[i32TypeIndex] = clCreateKernel(psData->psProgram[i32TypeIndex], "computeCharMul", &eResult);
		CheckAndReportError(psInstance, "clCreateKernel", eResult, init_char_cleanup);

		eResult = clSetKernelArg(psData->psMulKernel[i32TypeIndex], 0, sizeof(cl_char), (void*) &iValue );
		CheckAndReportError(psInstance, "clSetKernelArg", eResult, init_char_cleanup);

		eResult = clSetKernelArg(psData->psMulKernel[i32TypeIndex], 1, sizeof(cl_mem), (void*) &psData->psInputBuffer[i32TypeIndex] );
		CheckAndReportError(psInstance, "clSetKernelArg", eResult, init_char_cleanup);

		/* Init Mad kernel */
		OCLTestLog("Compiling Mad %s kernel...\n", charTypeSizeNames[i32TypeIndex]);
		psData->psMadKernel[i32TypeIndex] = clCreateKernel(psData->psProgram[i32TypeIndex], "computeCharMad", &eResult);
		CheckAndReportError(psInstance, "clCreateKernel", eResult, init_char_cleanup);

		eResult = clSetKernelArg(psData->psMadKernel[i32TypeIndex], 0, sizeof(cl_char), (void*) &iValue );
		CheckAndReportError(psInstance, "clSetKernelArg", eResult, init_char_cleanup);

		eResult = clSetKernelArg(psData->psMadKernel[i32TypeIndex], 1, sizeof(cl_mem), (void*) &psData->psInputBuffer[i32TypeIndex] );
		CheckAndReportError(psInstance, "clSetKernelArg", eResult, init_char_cleanup);

		/* Init Div kernel */
		OCLTestLog("Compiling Div %s kernel...\n", charTypeSizeNames[i32TypeIndex]);
		psData->psDivKernel[i32TypeIndex] = clCreateKernel(psData->psProgram[i32TypeIndex], "computeCharDiv", &eResult);
		CheckAndReportError(psInstance, "clCreateKernel", eResult, init_char_cleanup);

		eResult = clSetKernelArg(psData->psDivKernel[i32TypeIndex], 0, sizeof(cl_char), (void*) &iValue );
		CheckAndReportError(psInstance, "clSetKernelArg", eResult, init_char_cleanup);

		eResult = clSetKernelArg(psData->psDivKernel[i32TypeIndex], 1, sizeof(cl_mem), (void*) &psData->psInputBuffer[i32TypeIndex] );
		CheckAndReportError(psInstance, "clSetKernelArg", eResult, init_char_cleanup);

		/* Setup the size of the kernel run */
		psData->puGlobalWorkSize[0][i32TypeIndex] = TEST_CHAR_MAX_INSTANCES;// / charTypeDividers[i32TypeIndex];
		psData->puLocalWorkSize = NULL;
	}

	return CL_SUCCESS;

init_char_cleanup:
	return eResult;
}

/***********************************************************************************
 Function Name      : Compute_Char
 Inputs             : psInstance - test instance data
 Returns            : 0 or 1
 Description        : Main compute func
************************************************************************************/
static cl_int
Compute_Char(OCLTestInstance *psInstance)
{
	cl_int eResult;
	size_t global_dim[1];
	int i32TypeIndex = 0;
	CharData *psData =(CharData*) psInstance->pvPrivateData;

	for (i32TypeIndex = 0; i32TypeIndex < TEST_CHAR_NUM_TYPE_SIZES; i32TypeIndex++)
	{
		OCLTestLog("%s: Online compilation test with %s using %zu instances, each with %d iterations\n",
			__func__,charTypeSizeNames[i32TypeIndex], psData->puGlobalWorkSize[0][i32TypeIndex],TEST_CHAR_NUM_ITERATIONS);

		/* Reduce to match vec4 output size */
		global_dim[0] = psData->puGlobalWorkSize[0][i32TypeIndex];

		// Enqueue and complete Add kernel
		OCLTestLog("Running Add kernel for %s...\n", charTypeSizeNames[i32TypeIndex]);
		eResult = clEnqueueNDRangeKernel(psData->psCommandQueue, psData->psAddKernel[i32TypeIndex], 1, NULL, global_dim, psData->puLocalWorkSize , 0, NULL, &psData->psAddCompleteEvent[i32TypeIndex]);
		CheckAndReportError(psInstance, "clEnqueueNDRangeKernel", eResult, compute_char_cleanup);

		clFinish(psData->psCommandQueue);

		// Enqueue and complete Mul kernel
		OCLTestLog("Running Mul kernel for %s...\n", charTypeSizeNames[i32TypeIndex]);
		eResult = clEnqueueNDRangeKernel(psData->psCommandQueue, psData->psMulKernel[i32TypeIndex], 1, NULL, global_dim, psData->puLocalWorkSize , 0, NULL, &psData->psMulCompleteEvent[i32TypeIndex]);
		CheckAndReportError(psInstance, "clEnqueueNDRangeKernel", eResult, compute_char_cleanup);

		clFinish(psData->psCommandQueue);

		// Enqueue and complete Mad kernel
		OCLTestLog("Running Mad kernel for %s...\n",charTypeSizeNames[i32TypeIndex]);
		eResult = clEnqueueNDRangeKernel(psData->psCommandQueue, psData->psMadKernel[i32TypeIndex], 1, NULL, global_dim, psData->puLocalWorkSize , 0, NULL, &psData->psMadCompleteEvent[i32TypeIndex]);
		CheckAndReportError(psInstance, "clEnqueueNDRangeKernel", eResult, compute_char_cleanup);

		clFinish(psData->psCommandQueue);

		// Enqueue and complete Div kernel
		OCLTestLog("Running Div kernel for %s...\n", charTypeSizeNames[i32TypeIndex]);
		eResult = clEnqueueNDRangeKernel(psData->psCommandQueue, psData->psDivKernel[i32TypeIndex], 1, NULL, global_dim, psData->puLocalWorkSize , 0, NULL, &psData->psDivCompleteEvent[i32TypeIndex]);
		CheckAndReportError(psInstance, "clEnqueueNDRangeKernel", eResult, compute_char_cleanup);

		clFinish(psData->psCommandQueue);
	}

	return CL_SUCCESS;

compute_char_cleanup:
	return eResult;
}

/***********************************************************************************
 Function Name      : Verify_Char
 Inputs             : None
 Outputs            : None
 Returns            : None
 Description        : Verifies output data
************************************************************************************/
static cl_int
Verify_Char(OCLTestInstance *psInstance)
{
	cl_int eResult = CL_SUCCESS;
	int i = 0;
	int i32TypeIndex = 0;

	CharData *psData =(CharData*) psInstance->pvPrivateData;

	for (i32TypeIndex = 0; i32TypeIndex < TEST_CHAR_NUM_TYPE_SIZES; i32TypeIndex++)
	{
		cl_event apsProfilingEvents[] = { psData->psAddCompleteEvent[i32TypeIndex],
										  psData->psMulCompleteEvent[i32TypeIndex],
										  psData->psMadCompleteEvent[i32TypeIndex],
										  psData->psDivCompleteEvent[i32TypeIndex] };

		char* apszTestNames[] = { "Add", "Mul", "Mad", "Div" };
		unsigned int auOpsPerInstance[] = { 6+(TEST_CHAR_NUM_ITERATIONS*38),
											4+(TEST_CHAR_NUM_ITERATIONS*26),
											4+(TEST_CHAR_NUM_ITERATIONS*80/*mad 98+98*/),
											4+(TEST_CHAR_NUM_ITERATIONS*38) };

		/* Print out some metrics for each kernel */
		for (i = 0; i != 4; i++)
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
			CheckAndReportError(psInstance, "clGetEventProfilingInfo", eResult, verify_char_cleanup);
			eResult = clGetEventProfilingInfo(apsProfilingEvents[i],CL_PROFILING_COMMAND_END  ,sizeof(cl_ulong),&ulEnd,NULL);
			CheckAndReportError(psInstance, "clGetEventProfilingInfo", eResult, verify_char_cleanup);

			fTimeInSeconds = ((double)(ulEnd-ulStart)) / 1000000000.0;

			/* Calculate the total number of char operations */
			ullIO = auOpsPerInstance[i] * charTypeDividers[i32TypeIndex] /* operating on charX */;
			ullIO *= psData->puGlobalWorkSize[0][i32TypeIndex];

			/* Divide by the time to get the IOPS */
			fIOPS = ullIO / fTimeInSeconds;

			/* Convert to GIGA IOps */
			fGIOPS = fIOPS / 1000000000.0;

			fTotalGIOp = ullIO / 1000000000.0;

			OCLTestLog("---------------\n");
			OCLTestLog("%s %s:\n", charTypeSizeNames[i32TypeIndex], apszTestNames[i]);
			OCLTestLog("---------------\n");
			OCLTestLog("%s: Time                      %fs\n",__func__,fTimeInSeconds);
			OCLTestLog("%s: Iterations                %d\n",__func__,TEST_CHAR_NUM_ITERATIONS);
			OCLTestLog("%s: Instances                 %zu\n",__func__,psData->puGlobalWorkSize[0][i32TypeIndex]);
			OCLTestLog("%s: Char operations %llu\n",__func__,ullIO);
			OCLTestLog("%s: GIOp                     %f\n",__func__,fTotalGIOp);
			OCLTestLog("%s: GIOP/S                   %f\n",__func__,fGIOPS);
			//OCLTestLog("%s: IOPS                     %f\n",__func__,fIOPS);
			OCLMetricOutputDouble2(__func__,charTypeSizeNames[i32TypeIndex],apszTestNames[i],fGIOPS,GIOPSEC);
		}
		OCLTestLog("\n");
	}
	/* Cleanup */
	for (i32TypeIndex = 0; i32TypeIndex < TEST_CHAR_NUM_TYPE_SIZES; i32TypeIndex++)
	{
		clReleaseEvent       ( psData->psAddCompleteEvent[i32TypeIndex]);
		clReleaseEvent       ( psData->psMulCompleteEvent[i32TypeIndex]);
		clReleaseEvent       ( psData->psMadCompleteEvent[i32TypeIndex]);
		clReleaseEvent       ( psData->psDivCompleteEvent[i32TypeIndex]);
		clReleaseKernel      ( psData->psAddKernel[i32TypeIndex]);
		clReleaseKernel      ( psData->psMulKernel[i32TypeIndex]);
		clReleaseKernel      ( psData->psMadKernel[i32TypeIndex]);
		clReleaseKernel      ( psData->psDivKernel[i32TypeIndex]);
		clReleaseMemObject   ( psData->psInputBuffer[i32TypeIndex]);
	}
	clReleaseCommandQueue( psData->psCommandQueue );
	for (i32TypeIndex = 0; i32TypeIndex < TEST_CHAR_NUM_TYPE_SIZES; i32TypeIndex++)
	{
		clReleaseProgram     ( psData->psProgram[i32TypeIndex]);
	}
	clReleaseContext     ( psData->psContext      );

	free(psData->pui32BufferA);

	free(psInstance->pvPrivateData);
	psInstance->pvPrivateData = NULL;
	return eResult;

verify_char_cleanup:
	return eResult;
}

/******************************************************************************
 End of file (char.c)
******************************************************************************/
