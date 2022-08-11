/*************************************************************************/ /*!
@File           conversions.c
@Title          OpenCL unit test for data conversions
@Copyright      Copyright (c) Imagination Technologies Ltd. All Rights Reserved
@Description    USC Backend common Builtin header.
@License        Strictly Confidential.
*/ /**************************************************************************/

/* Number of values tested per type combination */
#ifdef CUT_DOWN_UNIT_TEST
#define TEST_CONVERSIONS_NUM_VALUES_PER_TYPE 16
#else
#define TEST_CONVERSIONS_NUM_VALUES_PER_TYPE 32
#endif

#define MAX_INT_32 ((int)0x7FFFFFFF)
#define MIN_INT_32 ((int)0x80000000)
#define MAX_INT_16 ((short)0x7FFF)
#define MIN_INT_16 ((short)0x8000)
#define MAX_INT_8 ((signed char)0x7F)
#define MIN_INT_8 ((signed char)0x80)

#define MAX_UINT_32 0xFFFFFFFFU
#define MAX_UINT_16 0xFFFFU
#define MAX_UINT_8 0xFFU

#if !defined(NO_HARDWARE)
#define VERIFY_DATA() \
	for (i = 0; i < TEST_CONVERSIONS_NUM_VALUES_PER_TYPE; ++i) \
	{ \
		if (puCalc[i] != 0xFFFFFFFF) \
		{ \
			OCLTestLog("%s: Verification failure at %d, expected %08x got %08x.\n", \
				       __func__, i, 0xFFFFFFFF, puCalc[i]); \
			return CL_INVALID_VALUE; \
		} \
	}
#else
#define VERIFY_DATA()
#endif

#include <fenv.h>
#include <float.h>
float nearbyintf(float x);

/*
   Set the pragma to say that we may use the non-default rounding mode.
   Not setting this and then using fesetround results in undefined behaviour.
*/
#pragma STDC FENV_ACCESS ON

typedef struct _ConversionsData_
{
	/* CL Objects */
	cl_context       psContext;
	cl_device_id     psDeviceID;
	cl_platform_id   psPlatformID;
	cl_command_queue psCommandQueue;
	cl_mem           psInBuffer;
	cl_mem           psRefBuffer;
	cl_mem           psOutBuffer;

	/* Kernel run counter */
	unsigned uCount;
} ConversionData;


/* Prototypes to silence warnings */
cl_int Init_Conversions(OCLTestInstance *psInstance);
cl_int Verify_Conversions(OCLTestInstance *psInstance);

static char *g_psConversionsTemplate =
{
	"__kernel void ConversionTest(__global %s * in, __global unsigned int * out, __global %s * ref)\n"
	"{\n"
	"\tint ith = get_global_id(0);\n"
	"\t%s conv = %s(in[ith]);\n"
	"\tout[ith] = (conv == ref[ith]) ? 0xFFFFFFFF : 0;\n"
	"}\n\n"
};

static char *g_psVectorConversionsTemplate =
{
	"__kernel void ConversionTest(__global %s * in, __global unsigned int * out, __global %s * ref)\n"
	"{\n"
	"\tint ith = get_global_id(0);\n"
	"\t%s conv = %s(in[ith]);\n"
	"\tint element;\n"
	"\tint vecSize = %u;\n"
	"\tfor (element = 0; element < vecSize; ++element)\n"
	"\t\tout[ith * vecSize + element] = ((conv == ref[ith])[element]) ? 0xFFFFFFFF : 0;\n"
	"}\n\n"
};

#if !defined(PVR_ANDROID_HAS_WORKING_FESETROUND) && \
    defined(__arm__) && defined(SUPPORT_ANDROID_PLATFORM)

/* Set rounding mode for android only because standard fesetround is not implemented on android  */
static __inline int arm_fesetround(int __round) {
	__uint32_t _fpscr;
	__asm__ __volatile__ ("vmrs %0,fpscr" : "=r" (_fpscr));
	_fpscr &= ~(0x3 << 22);
	switch (__round) {
		case FE_TONEAREST:  _fpscr |= 0x000000; break;
		case FE_UPWARD:     _fpscr |= 0x400000; break;
		case FE_DOWNWARD:   _fpscr |= 0x800000; break;
		case FE_TOWARDZERO: _fpscr |= 0xc00000; break;
		default: return -1;
	}
	__asm__ __volatile__ ("vmsr fpscr,%0" : :"ri" (_fpscr));
	return 0;
}

#define FESETROUND(X) arm_fesetround(X)

#else

#define FESETROUND(X) fesetround(X)

#endif

/******************** CONVERSION FUNCTIONS **********************/

/* From floats */
static cl_int convertFloatToInt_Sat(cl_float f)
{
	if (f > MAX_INT_32)
		return MAX_INT_32;
	else if (f < MIN_INT_32)
		return MIN_INT_32;
	else
		return (cl_int)nearbyintf(f);
}
static cl_uint convertFloatToUInt_Sat(cl_float f)
{
	if (f > MAX_UINT_32)
		return MAX_UINT_32;
	else if (f < 0.0f)
		return 0;
	else
		return (cl_uint)nearbyintf(f);
}

static cl_short convertFloatToShort_Sat(cl_float f)
{
	if (f > MAX_INT_16)
		return MAX_INT_16;
	else if (f < MIN_INT_16)
		return MIN_INT_16;
	else
		return (cl_short)nearbyintf(f);
}
static cl_ushort convertFloatToUShort_Sat(cl_float f)
{
	if (f > MAX_UINT_16)
		return MAX_UINT_16;
	else if (f < 0.0f)
		return 0;
	else
		return (cl_ushort)nearbyintf(f);
}

static cl_char convertFloatToChar_Sat(cl_float f)
{
	if (f > MAX_INT_8)
		return MAX_INT_8;
	else if (f < MIN_INT_8)
		return MIN_INT_8;
	else
		return (cl_char)nearbyintf(f);
}
static cl_uchar convertFloatToUChar_Sat(cl_float f)
{
	if (f > MAX_UINT_8)
		return MAX_UINT_8;
	else if (f < 0.0f)
		return 0;
	else
		return (cl_uchar)nearbyintf(f);
}


/* From signed integers */
static cl_uint convertIntToUInt_Sat(cl_int i)
{
	if (i < 0)
		return 0;
	else
		return (cl_uint)i;
}

static cl_short convertIntToShort_Sat(cl_int i)
{
	if (i > MAX_INT_16)
		return MAX_INT_16;
	else if (i < MIN_INT_16)
		return MIN_INT_16;
	else
		return (cl_short)i;
}
static cl_ushort convertIntToUShort_Sat(cl_int i)
{
	if (i < 0)
		return 0;
	else if (i > (cl_int)MAX_UINT_16)
		return MAX_UINT_16;
	else
		return (cl_ushort)i;
}

static cl_char convertIntToChar_Sat(cl_int i)
{
	if (i > MAX_INT_8)
		return MAX_INT_8;
	else if (i < MIN_INT_8)
		return MIN_INT_8;
	else
		return (cl_char)i;
}
static cl_uchar convertIntToUChar_Sat(cl_int i)
{
	if (i < 0)
		return 0;
	else if (i > (cl_int)MAX_UINT_8)
		return MAX_UINT_8;
	else
		return (cl_uchar)i;
}

static cl_ushort convertShortToUShort_Sat(cl_short i)
{
	if (i < 0)
		return 0;
	else
		return (cl_ushort)i;
}

/****************************************************************/

/***********************************************************************************
 Function Name: RunConversionKernel
 Description  : Runs a generated conversion kernel
************************************************************************************/
static int RunConversionKernel(OCLTestInstance* psInstance, char* pszSrcType, char* pszDestType, char* pszFunc, unsigned uVecSize)
{
	size_t global, local;
	char* ppszSources[2] 	= {0, 0};
	char pszSource[512];
	cl_int eResult 			= CL_SUCCESS;
	cl_program psProgram 	= NULL;
	cl_kernel psKernel 		= NULL;

	ConversionData* psData = (ConversionData*) psInstance->pvPrivateData;

	/* Generate the test program */
	memset(pszSource, '\0', 512);
	if (uVecSize > 1)
	{
		/* Vector version */
		snprintf(pszSource, 512, g_psVectorConversionsTemplate, pszSrcType, pszDestType, pszDestType, pszFunc, uVecSize);
	}
	else
	{
		/* Scalar version */
		snprintf(pszSource, 512, g_psConversionsTemplate, pszSrcType, pszDestType, pszDestType, pszFunc);
	}
	ppszSources[0] = pszSource;
	ppszSources[1] = "\0";

	psProgram = clCreateProgramWithSource(psData->psContext, 1, (const char**)ppszSources, NULL, &eResult);
	CheckAndReportError(psInstance, "clCreateProgramWithSource", eResult, runconversionskernel_cleanup);

	eResult = clBuildProgram(psProgram, 0, NULL, NULL, NULL, NULL);
	if (eResult != CL_SUCCESS)
	{
		char pszLog[1024];
		size_t uLogSize;
		clGetProgramBuildInfo(psProgram, psData->psDeviceID, CL_PROGRAM_BUILD_LOG, 1024, pszLog, &uLogSize);
		pszLog[uLogSize] = '\0';
		printf("Failed to build test %s\nBuild log:\n%s\n*****\n", pszFunc, pszLog);
		CheckAndReportError(psInstance, "clGetProgramBuildInfo", eResult, runconversionskernel_cleanup);
	}

	psKernel = clCreateKernel(psProgram, "ConversionTest", &eResult);
	CheckAndReportError(psInstance, "clCreateKernel", eResult, runconversionskernel_cleanup);

	eResult = clSetKernelArg(psKernel, 0, sizeof(cl_mem), &psData->psInBuffer);
	CheckAndReportError(psInstance, "clSetKernelArg", eResult, runconversionskernel_cleanup);

	eResult = clSetKernelArg(psKernel, 1, sizeof(cl_mem), &psData->psOutBuffer);
	CheckAndReportError(psInstance, "clSetKernelArg", eResult, runconversionskernel_cleanup);

	eResult = clSetKernelArg(psKernel, 2, sizeof(cl_mem), &psData->psRefBuffer);
	CheckAndReportError(psInstance, "clSetKernelArg", eResult, runconversionskernel_cleanup);

	global = TEST_CONVERSIONS_NUM_VALUES_PER_TYPE / uVecSize;
	local = 1;
	eResult = clEnqueueNDRangeKernel(psData->psCommandQueue, psKernel, 1, NULL, &global, &local, 0, NULL, NULL);
	CheckAndReportError(psInstance, "clEnqueueNDRangeKernel", eResult, runconversionskernel_cleanup);

	clFinish(psData->psCommandQueue);

runconversionskernel_cleanup:
	clReleaseKernel(psKernel);
	clReleaseProgram(psProgram);

	return eResult;
}

/***********************************************************************************
 Function Name: TestConvertFloatToX
 Description  : Tests conversions from float to other types
************************************************************************************/
static int TestConvertFloatToX(OCLTestInstance* psInstance, cl_float* pfBufferIn, void* pvReference, cl_uint* puCalc, char* pszRound)
{
	ConversionData* psData = (ConversionData*) psInstance->pvPrivateData;

	int i;
	cl_int eResult = CL_SUCCESS;

	char pszFunction[24];
	memset(pszFunction, '\0', 24);

	/***** Run the tests *****/

	/* Float -> UInt */
	/* Generate random float data */
	for (i = 0; i < TEST_CONVERSIONS_NUM_VALUES_PER_TYPE; ++i)
	{
		pfBufferIn[i] = rand();
		if (rand() % 2)
		{
			pfBufferIn[i] = -(pfBufferIn[i]);
		}
	}
	/* Request write-out of input data */
	eResult = clEnqueueWriteBuffer(psData->psCommandQueue, psData->psInBuffer, CL_FALSE, 0,
		sizeof(cl_float) * TEST_CONVERSIONS_NUM_VALUES_PER_TYPE, pfBufferIn, 0, NULL, NULL);
	CheckAndReportError(psInstance, "clEnqueueWriteBuffer", eResult, testconvertfloattox_cleanup);

	/* Generate reference results */
	for (i = 0; i < TEST_CONVERSIONS_NUM_VALUES_PER_TYPE; ++i)
	{
		((cl_int*)pvReference)[i] = convertFloatToUInt_Sat(pfBufferIn[i]);
	}

	/* Write reference data */
	eResult = clEnqueueWriteBuffer(psData->psCommandQueue, psData->psRefBuffer, CL_FALSE, 0,
		sizeof(cl_int) * TEST_CONVERSIONS_NUM_VALUES_PER_TYPE, pvReference, 0, NULL, NULL);
	CheckAndReportError(psInstance, "clEnqueueWriteBuffer", eResult, testconvertfloattox_cleanup);

	/* Run the test kernel */
	snprintf(pszFunction, 24, "convert_uint_sat_%s", pszRound);
	printf("Kick %u: Running float -> uint (%s)...\n", psData->uCount, pszFunction);
	eResult = RunConversionKernel(psInstance, "float", "uint", pszFunction, 1);
	CheckAndReportError(psInstance, "RunConversionsKernel", eResult, testconvertfloattox_cleanup);

	psData->uCount++;
	eResult = clEnqueueReadBuffer(psData->psCommandQueue, psData->psOutBuffer, CL_TRUE, 0,
		sizeof(cl_uint) * TEST_CONVERSIONS_NUM_VALUES_PER_TYPE, puCalc, 0, NULL, NULL);
	CheckAndReportError(psInstance, "clEnqueueReadBuffer", eResult, testconvertfloattox_cleanup);

	/* Check the data we've read back */
	VERIFY_DATA();

	/* Float -> Int */
	/* Generate reference results */
	for (i = 0; i < TEST_CONVERSIONS_NUM_VALUES_PER_TYPE; ++i)
	{
		((cl_int*)pvReference)[i] = convertFloatToInt_Sat(pfBufferIn[i]);
	}

	/* Write reference data */
	eResult = clEnqueueWriteBuffer(psData->psCommandQueue, psData->psRefBuffer, CL_FALSE, 0,
		sizeof(cl_int) * TEST_CONVERSIONS_NUM_VALUES_PER_TYPE, pvReference, 0, NULL, NULL);
	CheckAndReportError(psInstance, "clEnqueueWriteBuffer", eResult, testconvertfloattox_cleanup);

	/* Run the test kernel */
	snprintf(pszFunction, 24, "convert_int_sat_%s", pszRound);
	printf("Kick %u: Running float -> int (%s)...\n", psData->uCount, pszFunction);
	eResult = RunConversionKernel(psInstance, "float", "int", pszFunction, 1);
	CheckAndReportError(psInstance, "RunConversionsKernel", eResult, testconvertfloattox_cleanup);

	psData->uCount++;
	eResult = clEnqueueReadBuffer(psData->psCommandQueue, psData->psOutBuffer, CL_TRUE, 0,
		sizeof(cl_uint) * TEST_CONVERSIONS_NUM_VALUES_PER_TYPE, puCalc, 0, NULL, NULL);
	CheckAndReportError(psInstance, "clEnqueueReadBuffer", eResult, testconvertfloattox_cleanup);

	/* Check the data we've read back */
	VERIFY_DATA();

	/* Float -> UShort */
	/* Generate random float data */
	for (i = 0; i < TEST_CONVERSIONS_NUM_VALUES_PER_TYPE; ++i)
	{
		/* Shift random number by 15 to reduce probability of saturating cases */
		pfBufferIn[i] = rand() >> 15;
		if (rand() % 2)
		{
			pfBufferIn[i] = -(pfBufferIn[i]);
		}
	}

	/* Request write-out of input data */
	eResult = clEnqueueWriteBuffer(psData->psCommandQueue, psData->psInBuffer, CL_FALSE, 0,
		sizeof(cl_float) * TEST_CONVERSIONS_NUM_VALUES_PER_TYPE, pfBufferIn, 0, NULL, NULL);
	CheckAndReportError(psInstance, "clEnqueueWriteBuffer", eResult, testconvertfloattox_cleanup);

	/* Generate reference results */
	for (i = 0; i < TEST_CONVERSIONS_NUM_VALUES_PER_TYPE; ++i)
	{
		((cl_ushort*)pvReference)[i] = convertFloatToUShort_Sat(pfBufferIn[i]);
	}

	/* Write reference data */
	eResult = clEnqueueWriteBuffer(psData->psCommandQueue, psData->psRefBuffer, CL_FALSE, 0,
		sizeof(cl_ushort) * TEST_CONVERSIONS_NUM_VALUES_PER_TYPE, pvReference, 0, NULL, NULL);
	CheckAndReportError(psInstance, "clEnqueueWriteBuffer", eResult, testconvertfloattox_cleanup);

	/* Run the test kernel */
	snprintf(pszFunction, 24, "convert_ushort_sat_%s", pszRound);
	printf("Kick %u: Running float -> ushort (%s)...\n", psData->uCount, pszFunction);
	eResult = RunConversionKernel(psInstance, "float", "ushort", pszFunction, 1);
	CheckAndReportError(psInstance, "RunConversionsKernel", eResult, testconvertfloattox_cleanup);

	psData->uCount++;
	eResult = clEnqueueReadBuffer(psData->psCommandQueue, psData->psOutBuffer, CL_TRUE, 0,
		sizeof(cl_uint) * TEST_CONVERSIONS_NUM_VALUES_PER_TYPE, puCalc, 0, NULL, NULL);
	CheckAndReportError(psInstance, "clEnqueueReadBuffer", eResult, testconvertfloattox_cleanup);

	/* Check the data we've read back */
	VERIFY_DATA();

	/* Float -> Short */
	/* Generate reference results */
	for (i = 0; i < TEST_CONVERSIONS_NUM_VALUES_PER_TYPE; ++i)
	{
		((cl_short*)pvReference)[i] = convertFloatToShort_Sat(pfBufferIn[i]);
	}

	/* Write reference data */
	eResult = clEnqueueWriteBuffer(psData->psCommandQueue, psData->psRefBuffer, CL_FALSE, 0,
		sizeof(cl_short) * TEST_CONVERSIONS_NUM_VALUES_PER_TYPE, pvReference, 0, NULL, NULL);
	CheckAndReportError(psInstance, "clEnqueueWriteBuffer", eResult, testconvertfloattox_cleanup);

	/* Run the test kernel */
	snprintf(pszFunction, 24, "convert_short_sat_%s", pszRound);
	printf("Kick %u: Running float -> short (%s)...\n", psData->uCount, pszFunction);
	eResult = RunConversionKernel(psInstance, "float", "short", pszFunction, 1);
	CheckAndReportError(psInstance, "RunConversionsKernel", eResult, testconvertfloattox_cleanup);

	psData->uCount++;
	eResult = clEnqueueReadBuffer(psData->psCommandQueue, psData->psOutBuffer, CL_TRUE, 0,
		sizeof(cl_uint) * TEST_CONVERSIONS_NUM_VALUES_PER_TYPE, puCalc, 0, NULL, NULL);
	CheckAndReportError(psInstance, "clEnqueueReadBuffer", eResult, testconvertfloattox_cleanup);

	/* Check the data we've read back */
	VERIFY_DATA();

	/* Float -> UChar */
	/* Generate random float data */
	for (i = 0; i < TEST_CONVERSIONS_NUM_VALUES_PER_TYPE; ++i)
	{
		/* Shift random number by 23 to reduce probability of saturating cases */
		pfBufferIn[i] = rand() >> 23;
		if (rand() % 2)
		{
			pfBufferIn[i] = -(pfBufferIn[i]);
		}
	}

	/* Request write-out of input data */
	eResult = clEnqueueWriteBuffer(psData->psCommandQueue, psData->psInBuffer, CL_FALSE, 0,
		sizeof(cl_float) * TEST_CONVERSIONS_NUM_VALUES_PER_TYPE, pfBufferIn, 0, NULL, NULL);
	CheckAndReportError(psInstance, "clEnqueueWriteBuffer", eResult, testconvertfloattox_cleanup);

	/* Generate reference results */
	for (i = 0; i < TEST_CONVERSIONS_NUM_VALUES_PER_TYPE; ++i)
	{
		((cl_uchar*)pvReference)[i] = convertFloatToUChar_Sat(pfBufferIn[i]);
	}

	/* Write reference data */
	eResult = clEnqueueWriteBuffer(psData->psCommandQueue, psData->psRefBuffer, CL_FALSE, 0,
		sizeof(cl_uchar) * TEST_CONVERSIONS_NUM_VALUES_PER_TYPE, pvReference, 0, NULL, NULL);
	CheckAndReportError(psInstance, "clEnqueueWriteBuffer", eResult, testconvertfloattox_cleanup);

	/* Run the test kernel */
	snprintf(pszFunction, 24, "convert_uchar_sat_%s", pszRound);
	printf("Kick %u: Running float -> uchar (%s)...\n", psData->uCount, pszFunction);
	eResult = RunConversionKernel(psInstance, "float", "uchar", pszFunction, 1);
	CheckAndReportError(psInstance, "RunConversionsKernel", eResult, testconvertfloattox_cleanup);

	psData->uCount++;
	eResult = clEnqueueReadBuffer(psData->psCommandQueue, psData->psOutBuffer, CL_TRUE, 0,
		sizeof(cl_uint) * TEST_CONVERSIONS_NUM_VALUES_PER_TYPE, puCalc, 0, NULL, NULL);
	CheckAndReportError(psInstance, "clEnqueueReadBuffer", eResult, testconvertfloattox_cleanup);

	/* Check the data we've read back */
	VERIFY_DATA();

	/* Float -> Char */
	/* Generate reference results */
	for (i = 0; i < TEST_CONVERSIONS_NUM_VALUES_PER_TYPE; ++i)
	{
		((cl_char*)pvReference)[i] = convertFloatToChar_Sat(pfBufferIn[i]);
	}

	/* Write reference data */
	eResult = clEnqueueWriteBuffer(psData->psCommandQueue, psData->psRefBuffer, CL_FALSE, 0,
		sizeof(cl_char) * TEST_CONVERSIONS_NUM_VALUES_PER_TYPE, pvReference, 0, NULL, NULL);
	CheckAndReportError(psInstance, "clEnqueueWriteBuffer", eResult, testconvertfloattox_cleanup);

	/* Run the test kernel */
	snprintf(pszFunction, 24, "convert_char_sat_%s", pszRound);
	printf("Kick %u: Running float -> char (%s)...\n", psData->uCount, pszFunction);
	eResult = RunConversionKernel(psInstance, "float", "char", pszFunction, 1);
	CheckAndReportError(psInstance, "RunConversionsKernel", eResult, testconvertfloattox_cleanup);

	psData->uCount++;
	eResult = clEnqueueReadBuffer(psData->psCommandQueue, psData->psOutBuffer, CL_TRUE, 0,
		sizeof(cl_uint) * TEST_CONVERSIONS_NUM_VALUES_PER_TYPE, puCalc, 0, NULL, NULL);
	CheckAndReportError(psInstance, "clEnqueueReadBuffer", eResult, testconvertfloattox_cleanup);

	/* Check the data we've read back */
	VERIFY_DATA();

	/* End of Float to Integer tests */
	return eResult;

testconvertfloattox_cleanup:
	return eResult;
}

static int TestConvertVectorFloatToX(OCLTestInstance* psInstance, cl_float* pfBufferIn, void* pvReference, cl_uint* puCalc, char* pszRound)
{
	ConversionData* psData = (ConversionData*) psInstance->pvPrivateData;

	int i;
	cl_int eResult = CL_SUCCESS;
	char pszFunction[24];
	memset(pszFunction, '\0', 24);

	/***** Run the tests *****/

	/* Float -> UInt */
	/* Generate random float data */
	for (i = 0; i < TEST_CONVERSIONS_NUM_VALUES_PER_TYPE; ++i)
	{
		pfBufferIn[i] = rand();
		if (rand() % 2)
		{
			pfBufferIn[i] = -(pfBufferIn[i]);
		}
	}
	/* Request write-out of input data */
	eResult = clEnqueueWriteBuffer(psData->psCommandQueue, psData->psInBuffer, CL_FALSE, 0,
		sizeof(cl_float) * TEST_CONVERSIONS_NUM_VALUES_PER_TYPE, pfBufferIn, 0, NULL, NULL);
	CheckAndReportError(psInstance, "clEnqueueWriteBuffer", eResult, testconvertvectorfloattox_cleanup);

	/* Generate reference results */
	for (i = 0; i < TEST_CONVERSIONS_NUM_VALUES_PER_TYPE; ++i)
	{
		((cl_int*)pvReference)[i] = convertFloatToUInt_Sat(pfBufferIn[i]);
	}

	/* Write reference data */
	eResult = clEnqueueWriteBuffer(psData->psCommandQueue, psData->psRefBuffer, CL_FALSE, 0,
		sizeof(cl_int) * TEST_CONVERSIONS_NUM_VALUES_PER_TYPE, pvReference, 0, NULL, NULL);
	CheckAndReportError(psInstance, "clEnqueueWriteBuffer", eResult, testconvertvectorfloattox_cleanup);

	/* Run the test kernel */
	snprintf(pszFunction, 24, "convert_uint4_sat_%s", pszRound);
	printf("Kick %u: Running float4 -> uint4 (%s)...\n", psData->uCount, pszFunction);
	eResult = RunConversionKernel(psInstance, "float4", "uint4", pszFunction, 4);
	CheckAndReportError(psInstance, "RunConversionsKernel", eResult, testconvertvectorfloattox_cleanup);

	psData->uCount++;
	eResult = clEnqueueReadBuffer(psData->psCommandQueue, psData->psOutBuffer, CL_TRUE, 0,
		sizeof(cl_uint) * TEST_CONVERSIONS_NUM_VALUES_PER_TYPE, puCalc, 0, NULL, NULL);
	CheckAndReportError(psInstance, "clEnqueueReadBuffer", eResult, testconvertvectorfloattox_cleanup);

	/* Check the data we've read back */
	VERIFY_DATA();

	/* Float -> Int */
	/* Generate reference results */
	for (i = 0; i < TEST_CONVERSIONS_NUM_VALUES_PER_TYPE; ++i)
	{
		((cl_int*)pvReference)[i] = convertFloatToInt_Sat(pfBufferIn[i]);
	}

	/* Write reference data */
	eResult = clEnqueueWriteBuffer(psData->psCommandQueue, psData->psRefBuffer, CL_FALSE, 0,
		sizeof(cl_int) * TEST_CONVERSIONS_NUM_VALUES_PER_TYPE, pvReference, 0, NULL, NULL);
	CheckAndReportError(psInstance, "clEnqueueWriteBuffer", eResult, testconvertvectorfloattox_cleanup);

	/* Run the test kernel */
	snprintf(pszFunction, 24, "convert_int4_sat_%s", pszRound);
	printf("Kick %u: Running float4 -> int4 (%s)...\n", psData->uCount, pszFunction);
	eResult = RunConversionKernel(psInstance, "float4", "int4", pszFunction, 4);
	CheckAndReportError(psInstance, "RunConversionsKernel", eResult, testconvertvectorfloattox_cleanup);

	psData->uCount++;
	eResult = clEnqueueReadBuffer(psData->psCommandQueue, psData->psOutBuffer, CL_TRUE, 0,
		sizeof(cl_uint) * TEST_CONVERSIONS_NUM_VALUES_PER_TYPE, puCalc, 0, NULL, NULL);
	CheckAndReportError(psInstance, "clEnqueueReadBuffer", eResult, testconvertvectorfloattox_cleanup);

	/* Check the data we've read back */
	VERIFY_DATA();

	/* Float -> UShort */
	/* Generate random float data */
	for (i = 0; i < TEST_CONVERSIONS_NUM_VALUES_PER_TYPE; ++i)
	{
		/* Shift random number by 15 to reduce probability of saturating cases */
		pfBufferIn[i] = rand() >> 15;
		if (rand() % 2)
		{
			pfBufferIn[i] = -(pfBufferIn[i]);
		}
	}

	/* Request write-out of input data */
	eResult = clEnqueueWriteBuffer(psData->psCommandQueue, psData->psInBuffer, CL_FALSE, 0,
		sizeof(cl_float) * TEST_CONVERSIONS_NUM_VALUES_PER_TYPE, pfBufferIn, 0, NULL, NULL);
	CheckAndReportError(psInstance, "clEnqueueWriteBuffer", eResult, testconvertvectorfloattox_cleanup);

	/* Generate reference results */
	for (i = 0; i < TEST_CONVERSIONS_NUM_VALUES_PER_TYPE; ++i)
	{
		((cl_ushort*)pvReference)[i] = convertFloatToUShort_Sat(pfBufferIn[i]);
	}

	/* Write reference data */
	eResult = clEnqueueWriteBuffer(psData->psCommandQueue, psData->psRefBuffer, CL_FALSE, 0,
		sizeof(cl_ushort) * TEST_CONVERSIONS_NUM_VALUES_PER_TYPE, pvReference, 0, NULL, NULL);
	CheckAndReportError(psInstance, "clEnqueueWriteBuffer", eResult, testconvertvectorfloattox_cleanup);

	/* Run the test kernel */
	snprintf(pszFunction, 24, "convert_ushort4_sat_%s", pszRound);
	printf("Kick %u: Running float4 -> ushort4 (%s)...\n", psData->uCount, pszFunction);
	eResult = RunConversionKernel(psInstance, "float4", "ushort4", pszFunction, 4);
	CheckAndReportError(psInstance, "RunConversionsKernel", eResult, testconvertvectorfloattox_cleanup);

	psData->uCount++;
	eResult = clEnqueueReadBuffer(psData->psCommandQueue, psData->psOutBuffer, CL_TRUE, 0,
		sizeof(cl_uint) * TEST_CONVERSIONS_NUM_VALUES_PER_TYPE, puCalc, 0, NULL, NULL);
	CheckAndReportError(psInstance, "clEnqueueReadBuffer", eResult, testconvertvectorfloattox_cleanup);

	/* Check the data we've read back */
	VERIFY_DATA();

	/* Float -> Short */
	/* Generate reference results */
	for (i = 0; i < TEST_CONVERSIONS_NUM_VALUES_PER_TYPE; ++i)
	{
		((cl_short*)pvReference)[i] = convertFloatToShort_Sat(pfBufferIn[i]);
	}

	/* Write reference data */
	eResult = clEnqueueWriteBuffer(psData->psCommandQueue, psData->psRefBuffer, CL_FALSE, 0,
		sizeof(cl_short) * TEST_CONVERSIONS_NUM_VALUES_PER_TYPE, pvReference, 0, NULL, NULL);
	CheckAndReportError(psInstance, "clEnqueueWriteBuffer", eResult, testconvertvectorfloattox_cleanup);

	/* Run the test kernel */
	snprintf(pszFunction, 24, "convert_short4_sat_%s", pszRound);
	printf("Kick %u: Running float4 -> short4 (%s)...\n", psData->uCount, pszFunction);
	eResult = RunConversionKernel(psInstance, "float4", "short4", pszFunction, 4);
	CheckAndReportError(psInstance, "RunConversionsKernel", eResult, testconvertvectorfloattox_cleanup);

	psData->uCount++;
	eResult = clEnqueueReadBuffer(psData->psCommandQueue, psData->psOutBuffer, CL_TRUE, 0,
		sizeof(cl_uint) * TEST_CONVERSIONS_NUM_VALUES_PER_TYPE, puCalc, 0, NULL, NULL);
	CheckAndReportError(psInstance, "clEnqueueReadBuffer", eResult, testconvertvectorfloattox_cleanup);

	/* Check the data we've read back */
	VERIFY_DATA();

	/* Float -> UChar */
	/* Generate random float data */
	for (i = 0; i < TEST_CONVERSIONS_NUM_VALUES_PER_TYPE; ++i)
	{
		/* Shift random number by 23 to reduce probability of saturating cases */
		pfBufferIn[i] = rand() >> 23;
		if (rand() % 2)
		{
			pfBufferIn[i] = -(pfBufferIn[i]);
		}
	}

	/* Request write-out of input data */
	eResult = clEnqueueWriteBuffer(psData->psCommandQueue, psData->psInBuffer, CL_FALSE, 0,
		sizeof(cl_float) * TEST_CONVERSIONS_NUM_VALUES_PER_TYPE, pfBufferIn, 0, NULL, NULL);
	CheckAndReportError(psInstance, "clEnqueueWriteBuffer", eResult, testconvertvectorfloattox_cleanup);

	/* Generate reference results */
	for (i = 0; i < TEST_CONVERSIONS_NUM_VALUES_PER_TYPE; ++i)
	{
		((cl_uchar*)pvReference)[i] = convertFloatToUChar_Sat(pfBufferIn[i]);
	}

	/* Write reference data */
	eResult = clEnqueueWriteBuffer(psData->psCommandQueue, psData->psRefBuffer, CL_FALSE, 0,
		sizeof(cl_uchar) * TEST_CONVERSIONS_NUM_VALUES_PER_TYPE, pvReference, 0, NULL, NULL);
	CheckAndReportError(psInstance, "clEnqueueWriteBuffer", eResult, testconvertvectorfloattox_cleanup);

	/* Run the test kernel */
	snprintf(pszFunction, 24, "convert_uchar4_sat_%s", pszRound);
	printf("Kick %u: Running float4 -> uchar4 (%s)...\n", psData->uCount, pszFunction);
	eResult = RunConversionKernel(psInstance, "float4", "uchar4", pszFunction, 4);
	CheckAndReportError(psInstance, "RunConversionsKernel", eResult, testconvertvectorfloattox_cleanup);

	psData->uCount++;
	eResult = clEnqueueReadBuffer(psData->psCommandQueue, psData->psOutBuffer, CL_TRUE, 0,
		sizeof(cl_uint) * TEST_CONVERSIONS_NUM_VALUES_PER_TYPE, puCalc, 0, NULL, NULL);
	CheckAndReportError(psInstance, "clEnqueueReadBuffer", eResult, testconvertvectorfloattox_cleanup);

	/* Check the data we've read back */
	VERIFY_DATA();

	/* Float -> Char */
	/* Generate reference results */
	for (i = 0; i < TEST_CONVERSIONS_NUM_VALUES_PER_TYPE; ++i)
	{
		((cl_char*)pvReference)[i] = convertFloatToChar_Sat(pfBufferIn[i]);
	}

	/* Write reference data */
	eResult = clEnqueueWriteBuffer(psData->psCommandQueue, psData->psRefBuffer, CL_FALSE, 0,
		sizeof(cl_char) * TEST_CONVERSIONS_NUM_VALUES_PER_TYPE, pvReference, 0, NULL, NULL);
	CheckAndReportError(psInstance, "clEnqueueWriteBuffer", eResult, testconvertvectorfloattox_cleanup);

	/* Run the test kernel */
	snprintf(pszFunction, 24, "convert_char4_sat_%s", pszRound);
	printf("Kick %u: Running float4 -> char4 (%s)...\n", psData->uCount, pszFunction);
	eResult = RunConversionKernel(psInstance, "float4", "char4", pszFunction, 4);
	CheckAndReportError(psInstance, "RunConversionsKernel", eResult, testconvertvectorfloattox_cleanup);

	psData->uCount++;
	eResult = clEnqueueReadBuffer(psData->psCommandQueue, psData->psOutBuffer, CL_TRUE, 0,
		sizeof(cl_uint) * TEST_CONVERSIONS_NUM_VALUES_PER_TYPE, puCalc, 0, NULL, NULL);
	CheckAndReportError(psInstance, "clEnqueueReadBuffer", eResult, testconvertvectorfloattox_cleanup);

	/* Check the data we've read back */
	VERIFY_DATA();

	/* End of Float to Integer tests */
testconvertvectorfloattox_cleanup:
	return eResult;
}

/***********************************************************************************
 Function Name: TestConvertXToFloat
 Description  : Tests conversions from all types to float
************************************************************************************/
static int TestConvertXToFloat(OCLTestInstance* psInstance, void* pvBufferIn, cl_float* pfReference, cl_uint* puCalc, char* pszRound)
{
	ConversionData* psData = (ConversionData*) psInstance->pvPrivateData;

	int i;
	cl_int eResult = CL_SUCCESS;
	char pszFunction[24];
	memset(pszFunction, '\0', 24);

	/* Generate random float data */
	for (i = 0; i < TEST_CONVERSIONS_NUM_VALUES_PER_TYPE; ++i)
	{
		((cl_int*)pvBufferIn)[i] = rand();
		if (rand() % 2)
		{
			((cl_int*)pvBufferIn)[i] = -(((cl_int*)pvBufferIn)[i]);
		}
	}

	/* Request write-out of input data */
	eResult = clEnqueueWriteBuffer(psData->psCommandQueue, psData->psInBuffer, CL_FALSE, 0,
		sizeof(cl_uint) * TEST_CONVERSIONS_NUM_VALUES_PER_TYPE, pvBufferIn, 0, NULL, NULL);
	CheckAndReportError(psInstance, "clEnqueueWriteBuffer", eResult, testconvertxtofloat_cleanup);

	/***** Run the tests *****/

	/* UInt -> Float */
	/* Generate reference results */
	for (i = 0; i < TEST_CONVERSIONS_NUM_VALUES_PER_TYPE; ++i)
	{
#if defined(__arm__) && defined(SUPPORT_ANDROID_PLATFORM)
		/*
		   Work around a toolchain bug on Android ARM where the conversion from
		   integer to float doesn't respect the current rounding mode.
		   Discovered as part of RDI8247.
		*/
		double fTemp = ((cl_uint*)pvBufferIn)[i];
		pfReference[i] = (float) fTemp;
#else
		pfReference[i] = ((cl_uint*)pvBufferIn)[i];
#endif
	}

	/* Write reference data */
	eResult = clEnqueueWriteBuffer(psData->psCommandQueue, psData->psRefBuffer, CL_FALSE, 0,
		sizeof(cl_float) * TEST_CONVERSIONS_NUM_VALUES_PER_TYPE, pfReference, 0, NULL, NULL);
	CheckAndReportError(psInstance, "clEnqueueWriteBuffer", eResult, testconvertxtofloat_cleanup);

	/* Run the test kernel */
	snprintf(pszFunction, 24, "convert_float_%s", pszRound);
	printf("Kick %u: Running uint -> float (%s)...\n", psData->uCount, pszFunction);
	eResult = RunConversionKernel(psInstance, "uint", "float", pszFunction, 1);
	CheckAndReportError(psInstance, "RunConversionsKernel", eResult, testconvertxtofloat_cleanup);

	psData->uCount++;
	eResult = clEnqueueReadBuffer(psData->psCommandQueue, psData->psOutBuffer, CL_TRUE, 0,
		sizeof(cl_uint) * TEST_CONVERSIONS_NUM_VALUES_PER_TYPE, puCalc, 0, NULL, NULL);
	CheckAndReportError(psInstance, "clEnqueueReadBuffer", eResult, testconvertxtofloat_cleanup);

	/* Check the data we've read back */
	VERIFY_DATA();

	/* Int -> Float */
	/* Generate reference results */
	for (i = 0; i < TEST_CONVERSIONS_NUM_VALUES_PER_TYPE; ++i)
	{
#if defined(__arm__) && defined(SUPPORT_ANDROID_PLATFORM)
		/*
		   Work around a toolchain bug on Android ARM where the conversion from
		   integer to float doesn't respect the current rounding mode.
		   Discovered as part of RDI8247.
		*/
		double fTemp = ((cl_int*)pvBufferIn)[i];
		pfReference[i] = (float) fTemp;
#else
		pfReference[i] = ((cl_int*)pvBufferIn)[i];
#endif
	}

	/* Write reference data */
	eResult = clEnqueueWriteBuffer(psData->psCommandQueue, psData->psRefBuffer, CL_FALSE, 0,
		sizeof(cl_float) * TEST_CONVERSIONS_NUM_VALUES_PER_TYPE, pfReference, 0, NULL, NULL);
	CheckAndReportError(psInstance, "clEnqueueWriteBuffer", eResult, testconvertxtofloat_cleanup);

	/* Run the test kernel */
	snprintf(pszFunction, 24, "convert_float_%s", pszRound);
	printf("Kick %u: Running int -> float (%s)...\n", psData->uCount, pszFunction);
	eResult = RunConversionKernel(psInstance, "int", "float", pszFunction, 1);
	CheckAndReportError(psInstance, "RunConversionsKernel", eResult, testconvertxtofloat_cleanup);

	psData->uCount++;
	eResult = clEnqueueReadBuffer(psData->psCommandQueue, psData->psOutBuffer, CL_TRUE, 0,
		sizeof(cl_uint) * TEST_CONVERSIONS_NUM_VALUES_PER_TYPE, puCalc, 0, NULL, NULL);
	CheckAndReportError(psInstance, "clEnqueueReadBuffer", eResult, testconvertxtofloat_cleanup);

	/* Check the data we've read back */
	VERIFY_DATA();

	/* UShort -> Float */
	/* Generate reference results */
	for (i = 0; i < TEST_CONVERSIONS_NUM_VALUES_PER_TYPE; ++i)
	{
		pfReference[i] = (((cl_ushort*)pvBufferIn)[i]);
	}

	/* Write reference data */
	eResult = clEnqueueWriteBuffer(psData->psCommandQueue, psData->psRefBuffer, CL_FALSE, 0,
		sizeof(cl_float) * TEST_CONVERSIONS_NUM_VALUES_PER_TYPE, pfReference, 0, NULL, NULL);
	CheckAndReportError(psInstance, "clEnqueueWriteBuffer", eResult, testconvertxtofloat_cleanup);

	/* Run the test kernel */
	snprintf(pszFunction, 24, "convert_float_%s", pszRound);
	printf("Kick %u: Running ushort -> float (%s)...\n", psData->uCount, pszFunction);
	eResult = RunConversionKernel(psInstance, "ushort", "float", pszFunction, 1);
	CheckAndReportError(psInstance, "RunConversionsKernel", eResult, testconvertxtofloat_cleanup);

	psData->uCount++;
	eResult = clEnqueueReadBuffer(psData->psCommandQueue, psData->psOutBuffer, CL_TRUE, 0,
		sizeof(cl_uint) * TEST_CONVERSIONS_NUM_VALUES_PER_TYPE, puCalc, 0, NULL, NULL);
	CheckAndReportError(psInstance, "clEnqueueReadBuffer", eResult, testconvertxtofloat_cleanup);

	/* Check the data we've read back */
	VERIFY_DATA();

	/* Short -> Float */
	/* Generate reference results */
	for (i = 0; i < TEST_CONVERSIONS_NUM_VALUES_PER_TYPE; ++i)
	{
		pfReference[i] = (((cl_short*)pvBufferIn)[i]);
	}

	/* Write reference data */
	eResult = clEnqueueWriteBuffer(psData->psCommandQueue, psData->psRefBuffer, CL_FALSE, 0,
		sizeof(cl_float) * TEST_CONVERSIONS_NUM_VALUES_PER_TYPE, pfReference, 0, NULL, NULL);
	CheckAndReportError(psInstance, "clEnqueueWriteBuffer", eResult, testconvertxtofloat_cleanup);

	/* Run the test kernel */
	snprintf(pszFunction, 24, "convert_float_%s", pszRound);
	printf("Kick %u: Running short -> float (%s)...\n", psData->uCount, pszFunction);
	eResult = RunConversionKernel(psInstance, "short", "float", pszFunction, 1);
	CheckAndReportError(psInstance, "RunConversionsKernel", eResult, testconvertxtofloat_cleanup);

	psData->uCount++;
	eResult = clEnqueueReadBuffer(psData->psCommandQueue, psData->psOutBuffer, CL_TRUE, 0,
		sizeof(cl_uint) * TEST_CONVERSIONS_NUM_VALUES_PER_TYPE, puCalc, 0, NULL, NULL);
	CheckAndReportError(psInstance, "clEnqueueReadBuffer", eResult, testconvertxtofloat_cleanup);

	/* Check the data we've read back */
	VERIFY_DATA();

	/* UChar -> Float */
	/* Generate reference results */
	for (i = 0; i < TEST_CONVERSIONS_NUM_VALUES_PER_TYPE; ++i)
	{
		pfReference[i] = (((cl_uchar*)pvBufferIn)[i]);
	}

	/* Write reference data */
	eResult = clEnqueueWriteBuffer(psData->psCommandQueue, psData->psRefBuffer, CL_FALSE, 0,
		sizeof(cl_float) * TEST_CONVERSIONS_NUM_VALUES_PER_TYPE, pfReference, 0, NULL, NULL);
	CheckAndReportError(psInstance, "clEnqueueWriteBuffer", eResult, testconvertxtofloat_cleanup);

	/* Run the test kernel */
	snprintf(pszFunction, 24, "convert_float_%s", pszRound);
	printf("Kick %u: Running uchar -> float (%s)...\n", psData->uCount, pszFunction);
	eResult = RunConversionKernel(psInstance, "uchar", "float", pszFunction, 1);
	CheckAndReportError(psInstance, "RunConversionsKernel", eResult, testconvertxtofloat_cleanup);

	psData->uCount++;
	eResult = clEnqueueReadBuffer(psData->psCommandQueue, psData->psOutBuffer, CL_TRUE, 0,
		sizeof(cl_uint) * TEST_CONVERSIONS_NUM_VALUES_PER_TYPE, puCalc, 0, NULL, NULL);
	CheckAndReportError(psInstance, "clEnqueueReadBuffer", eResult, testconvertxtofloat_cleanup);

	/* Check the data we've read back */
	VERIFY_DATA();

	/* Char -> Float */
	/* Generate reference results */
	for (i = 0; i < TEST_CONVERSIONS_NUM_VALUES_PER_TYPE; ++i)
	{
		pfReference[i] = (((cl_char*)pvBufferIn)[i]);
	}

	/* Write reference data */
	eResult = clEnqueueWriteBuffer(psData->psCommandQueue, psData->psRefBuffer, CL_FALSE, 0,
		sizeof(cl_float) * TEST_CONVERSIONS_NUM_VALUES_PER_TYPE, pfReference, 0, NULL, NULL);
	CheckAndReportError(psInstance, "clEnqueueWriteBuffer", eResult, testconvertxtofloat_cleanup);

	/* Run the test kernel */
	snprintf(pszFunction, 24, "convert_float_%s", pszRound);
	printf("Kick %u: Running char -> float (%s)...\n", psData->uCount, pszFunction);
	eResult = RunConversionKernel(psInstance, "char", "float", pszFunction, 1);
	CheckAndReportError(psInstance, "RunConversionsKernel", eResult, testconvertxtofloat_cleanup);

	psData->uCount++;
	eResult = clEnqueueReadBuffer(psData->psCommandQueue, psData->psOutBuffer, CL_TRUE, 0,
		sizeof(cl_uint) * TEST_CONVERSIONS_NUM_VALUES_PER_TYPE, puCalc, 0, NULL, NULL);
	CheckAndReportError(psInstance, "clEnqueueReadBuffer", eResult, testconvertxtofloat_cleanup);

	/* Check the data we've read back */
	VERIFY_DATA();

	/* End of Integer to Float tests */
testconvertxtofloat_cleanup:
	return eResult;
}

/***********************************************************************************
 Function Name: TestConvertInt32ToX
 Description  : Tests conversions from Integers32 to other types
************************************************************************************/
static int TestConvertInt32ToX(OCLTestInstance* psInstance, cl_int* pnBufferIn, void* pvReference, cl_uint* puCalc)
{
	ConversionData* psData = (ConversionData*) psInstance->pvPrivateData;

	int i;
	cl_int eResult = CL_SUCCESS;

	/***** Run the tests *****/

	/* Generate random data */
	for (i = 0; i < TEST_CONVERSIONS_NUM_VALUES_PER_TYPE; ++i)
	{
		pnBufferIn[i] = rand();
		if (rand() % 2)
		{
			pnBufferIn[i] = -(pnBufferIn[i]);
		}
	}

	/* Request write-out of input data */
	eResult = clEnqueueWriteBuffer(psData->psCommandQueue, psData->psInBuffer, CL_FALSE, 0,
		sizeof(cl_int) * TEST_CONVERSIONS_NUM_VALUES_PER_TYPE, pnBufferIn, 0, NULL, NULL);
	CheckAndReportError(psInstance, "clEnqueueWriteBuffer", eResult, testconvertint32tox_cleanup);

	/* Int -> uint */
	/* Generate reference results */
	for (i = 0; i < TEST_CONVERSIONS_NUM_VALUES_PER_TYPE; ++i)
	{
		((cl_uint*)pvReference)[i] = convertIntToUInt_Sat(pnBufferIn[i]);
	}

	/* Write reference data */
	eResult = clEnqueueWriteBuffer(psData->psCommandQueue, psData->psRefBuffer, CL_FALSE, 0,
		sizeof(cl_uint) * TEST_CONVERSIONS_NUM_VALUES_PER_TYPE, pvReference, 0, NULL, NULL);
	CheckAndReportError(psInstance, "clEnqueueWriteBuffer", eResult, testconvertint32tox_cleanup);

	/* Run the test kernel */
	printf("Kick %u: Running int -> uint...\n", psData->uCount);
	eResult = RunConversionKernel(psInstance, "int", "uint", "convert_uint_sat", 1);
	CheckAndReportError(psInstance, "RunConversionsKernel", eResult, testconvertint32tox_cleanup);

	psData->uCount++;
	eResult = clEnqueueReadBuffer(psData->psCommandQueue, psData->psOutBuffer, CL_TRUE, 0,
		sizeof(cl_uint) * TEST_CONVERSIONS_NUM_VALUES_PER_TYPE, puCalc, 0, NULL, NULL);
	CheckAndReportError(psInstance, "clEnqueueReadBuffer", eResult, testconvertint32tox_cleanup);

	/* Check the data we've read back */
	VERIFY_DATA();

	/* Shift random data to reduce probability of saturations */
	for (i = 0; i < TEST_CONVERSIONS_NUM_VALUES_PER_TYPE; ++i)
	{
		pnBufferIn[i] >>= 15;
	}

	/* Request write-out of input data */
	eResult = clEnqueueWriteBuffer(psData->psCommandQueue, psData->psInBuffer, CL_FALSE, 0,
		sizeof(cl_int) * TEST_CONVERSIONS_NUM_VALUES_PER_TYPE, pnBufferIn, 0, NULL, NULL);
	CheckAndReportError(psInstance, "clEnqueueWriteBuffer", eResult, testconvertint32tox_cleanup);

	/* Int -> ushort */
	/* Generate reference results */
	for (i = 0; i < TEST_CONVERSIONS_NUM_VALUES_PER_TYPE; ++i)
	{
		((cl_ushort*)pvReference)[i] = convertIntToUShort_Sat(pnBufferIn[i]);
	}

	/* Write reference data */
	eResult = clEnqueueWriteBuffer(psData->psCommandQueue, psData->psRefBuffer, CL_FALSE, 0,
		sizeof(cl_ushort) * TEST_CONVERSIONS_NUM_VALUES_PER_TYPE, pvReference, 0, NULL, NULL);
	CheckAndReportError(psInstance, "clEnqueueWriteBuffer", eResult, testconvertint32tox_cleanup);

	/* Run the test kernel */
	printf("Kick %u: Running int -> ushort...\n", psData->uCount);
	eResult = RunConversionKernel(psInstance, "int", "ushort", "convert_ushort_sat", 1);
	CheckAndReportError(psInstance, "RunConversionsKernel", eResult, testconvertint32tox_cleanup);

	psData->uCount++;
	eResult = clEnqueueReadBuffer(psData->psCommandQueue, psData->psOutBuffer, CL_TRUE, 0,
		sizeof(cl_uint) * TEST_CONVERSIONS_NUM_VALUES_PER_TYPE, puCalc, 0, NULL, NULL);
	CheckAndReportError(psInstance, "clEnqueueReadBuffer", eResult, testconvertint32tox_cleanup);

	/* Check the data we've read back */
	VERIFY_DATA();

	/* Int -> short */
	/* Generate reference results */
	for (i = 0; i < TEST_CONVERSIONS_NUM_VALUES_PER_TYPE; ++i)
	{
		((cl_short*)pvReference)[i] = convertIntToShort_Sat(pnBufferIn[i]);
	}

	/* Write reference data */
	eResult = clEnqueueWriteBuffer(psData->psCommandQueue, psData->psRefBuffer, CL_FALSE, 0,
		sizeof(cl_short) * TEST_CONVERSIONS_NUM_VALUES_PER_TYPE, pvReference, 0, NULL, NULL);
	CheckAndReportError(psInstance, "clEnqueueWriteBuffer", eResult, testconvertint32tox_cleanup);

	/* Run the test kernel */
	printf("Kick %u: Running int -> short...\n", psData->uCount);
	eResult = RunConversionKernel(psInstance, "int", "short", "convert_short_sat", 1);
	CheckAndReportError(psInstance, "RunConversionsKernel", eResult, testconvertint32tox_cleanup);

	psData->uCount++;
	eResult = clEnqueueReadBuffer(psData->psCommandQueue, psData->psOutBuffer, CL_TRUE, 0,
		sizeof(cl_uint) * TEST_CONVERSIONS_NUM_VALUES_PER_TYPE, puCalc, 0, NULL, NULL);
	CheckAndReportError(psInstance, "clEnqueueReadBuffer", eResult, testconvertint32tox_cleanup);

	/* Check the data we've read back */
	VERIFY_DATA();

	/* Shift input data to reduce probability of saturations */
	for (i = 0; i < TEST_CONVERSIONS_NUM_VALUES_PER_TYPE; ++i)
	{
		pnBufferIn[i] >>= 8;
	}

	/* Request write-out of input data */
	eResult = clEnqueueWriteBuffer(psData->psCommandQueue, psData->psInBuffer, CL_FALSE, 0,
		sizeof(cl_int) * TEST_CONVERSIONS_NUM_VALUES_PER_TYPE, pnBufferIn, 0, NULL, NULL);
	CheckAndReportError(psInstance, "clEnqueueWriteBuffer", eResult, testconvertint32tox_cleanup);

	/* Int -> uchar */
	/* Generate reference results */
	for (i = 0; i < TEST_CONVERSIONS_NUM_VALUES_PER_TYPE; ++i)
	{
		((cl_uchar*)pvReference)[i] = convertIntToUChar_Sat(pnBufferIn[i]);
	}

	/* Write reference data */
	eResult = clEnqueueWriteBuffer(psData->psCommandQueue, psData->psRefBuffer, CL_FALSE, 0,
		sizeof(cl_uchar) * TEST_CONVERSIONS_NUM_VALUES_PER_TYPE, pvReference, 0, NULL, NULL);
	CheckAndReportError(psInstance, "clEnqueueWriteBuffer", eResult, testconvertint32tox_cleanup);

	/* Run the test kernel */
	printf("Kick %u: Running int -> uchar...\n", psData->uCount);
	eResult = RunConversionKernel(psInstance, "int", "uchar", "convert_uchar_sat", 1);
	CheckAndReportError(psInstance, "RunConversionsKernel", eResult, testconvertint32tox_cleanup);

	psData->uCount++;
	eResult = clEnqueueReadBuffer(psData->psCommandQueue, psData->psOutBuffer, CL_TRUE, 0,
		sizeof(cl_uint) * TEST_CONVERSIONS_NUM_VALUES_PER_TYPE, puCalc, 0, NULL, NULL);
	CheckAndReportError(psInstance, "clEnqueueReadBuffer", eResult, testconvertint32tox_cleanup);

	/* Check the data we've read back */
	VERIFY_DATA();

	/* Generate reference results */
	for (i = 0; i < TEST_CONVERSIONS_NUM_VALUES_PER_TYPE; ++i)
	{
		((cl_char*)pvReference)[i] = convertIntToChar_Sat(pnBufferIn[i]);
	}

	/* Write reference data */
	eResult = clEnqueueWriteBuffer(psData->psCommandQueue, psData->psRefBuffer, CL_FALSE, 0,
		sizeof(cl_char) * TEST_CONVERSIONS_NUM_VALUES_PER_TYPE, pvReference, 0, NULL, NULL);
	CheckAndReportError(psInstance, "clEnqueueWriteBuffer", eResult, testconvertint32tox_cleanup);

	/* Run the test kernel */
	printf("Kick %u: Running int -> char...\n", psData->uCount);
	eResult = RunConversionKernel(psInstance, "int", "char", "convert_char_sat", 1);
	CheckAndReportError(psInstance, "RunConversionsKernel", eResult, testconvertint32tox_cleanup);

	psData->uCount++;
	eResult = clEnqueueReadBuffer(psData->psCommandQueue, psData->psOutBuffer, CL_TRUE, 0,
		sizeof(cl_uint) * TEST_CONVERSIONS_NUM_VALUES_PER_TYPE, puCalc, 0, NULL, NULL);
	CheckAndReportError(psInstance, "clEnqueueReadBuffer", eResult, testconvertint32tox_cleanup);

	/* Check the data we've read back */
	VERIFY_DATA();

testconvertint32tox_cleanup:
	return eResult;
}

/***********************************************************************************
 Function Name: TestConvertInt16ToX
 Description  : Tests conversions from Integer16 to other types
************************************************************************************/
static int TestConvertInt16ToX(OCLTestInstance* psInstance, cl_short* pnBufferIn, void* pvReference, cl_uint* puCalc)
{
	ConversionData* psData = (ConversionData*) psInstance->pvPrivateData;

	int i;
	cl_int eResult = CL_SUCCESS;

	/***** Run the tests *****/

	/* Generate random data */
	for (i = 0; i < TEST_CONVERSIONS_NUM_VALUES_PER_TYPE; ++i)
	{
		pnBufferIn[i] = (rand() >> 16);
		if (rand() % 2)
		{
			pnBufferIn[i] = -(pnBufferIn[i]);
		}
	}

	/* Request write-out of input data */
	eResult = clEnqueueWriteBuffer(psData->psCommandQueue, psData->psInBuffer, CL_FALSE, 0,
		sizeof(cl_short) * TEST_CONVERSIONS_NUM_VALUES_PER_TYPE, pnBufferIn, 0, NULL, NULL);
	CheckAndReportError(psInstance, "clEnqueueWriteBuffer", eResult, testconvertint16tox_cleanup);

	/* Short -> ushort */
	/* Generate reference results */
	for (i = 0; i < TEST_CONVERSIONS_NUM_VALUES_PER_TYPE; ++i)
	{
		((cl_ushort*)pvReference)[i] = convertShortToUShort_Sat(pnBufferIn[i]);
	}

	/* Write reference data */
	eResult = clEnqueueWriteBuffer(psData->psCommandQueue, psData->psRefBuffer, CL_FALSE, 0,
		sizeof(cl_ushort) * TEST_CONVERSIONS_NUM_VALUES_PER_TYPE, pvReference, 0, NULL, NULL);
	CheckAndReportError(psInstance, "clEnqueueWriteBuffer", eResult, testconvertint16tox_cleanup);

	/* Run the test kernel */
	printf("Kick %u: Running short -> ushort...\n", psData->uCount);
	eResult = RunConversionKernel(psInstance, "short", "ushort", "convert_ushort_sat", 1);
	if (eResult != CL_SUCCESS) return eResult;
	CheckAndReportError(psInstance, "RunConversions", eResult, testconvertint16tox_cleanup);

	psData->uCount++;
	eResult = clEnqueueReadBuffer(psData->psCommandQueue, psData->psOutBuffer, CL_TRUE, 0,
		sizeof(cl_uint) * TEST_CONVERSIONS_NUM_VALUES_PER_TYPE, puCalc, 0, NULL, NULL);
	CheckAndReportError(psInstance, "clEnqueueWriteBuffer", eResult, testconvertint16tox_cleanup);

	/* Check the data we've read back */
	VERIFY_DATA();

testconvertint16tox_cleanup:
	return eResult;
}

/***********************************************************************************
 Function Name: Init_Conversions
 Description  : Initialises input data
************************************************************************************/
cl_int Init_Conversions(OCLTestInstance *psInstance)
{
	cl_int eResult = CL_SUCCESS;
	ConversionData* psData;

	psInstance->pvPrivateData = malloc(sizeof(ConversionData));
	psData = (ConversionData*) psInstance->pvPrivateData;

	if(!psData)
	{
		return CL_OUT_OF_RESOURCES;
	}

	eResult = clGetPlatformIDs(1, &psData->psPlatformID, NULL);
	CheckAndReportError(psInstance, "clGetPlatformIDs", eResult, init_conversions_cleanup);

	eResult = clGetDeviceIDs(NULL, CL_DEVICE_TYPE_GPU, 1, &psData->psDeviceID, NULL);
	CheckAndReportError(psInstance, "clGetPlatformIDs", eResult, init_conversions_cleanup);

	psData->psContext = clCreateContext(NULL, 1, &psData->psDeviceID, NULL, NULL, &eResult);
	CheckAndReportError(psInstance, "clCreateContext", eResult, init_conversions_cleanup);

	psData->psCommandQueue = clCreateCommandQueue(psData->psContext, psData->psDeviceID, 0, &eResult);
	CheckAndReportError(psInstance, "clCreateCommandQueue", eResult, init_conversions_cleanup);

	psData->psInBuffer = clCreateBuffer(psData->psContext, CL_MEM_READ_ONLY, sizeof(cl_uint) * TEST_CONVERSIONS_NUM_VALUES_PER_TYPE, NULL, &eResult);
	CheckAndReportError(psInstance, "clCreateBuffer", eResult, init_conversions_cleanup);
	psData->psOutBuffer = clCreateBuffer(psData->psContext, CL_MEM_WRITE_ONLY, sizeof(cl_uint) * TEST_CONVERSIONS_NUM_VALUES_PER_TYPE, NULL, &eResult);
	CheckAndReportError(psInstance, "clCreateBuffer", eResult, init_conversions_cleanup);
	psData->psRefBuffer = clCreateBuffer(psData->psContext, CL_MEM_READ_ONLY, sizeof(cl_uint) * TEST_CONVERSIONS_NUM_VALUES_PER_TYPE, NULL, &eResult);
	CheckAndReportError(psInstance, "clCreateBuffer", eResult, init_conversions_cleanup);

	psData->uCount = 0;

init_conversions_cleanup:
	return eResult;
}

/***********************************************************************************
 Function Name: Verify_Conversions
 Description  : Verifies output data
************************************************************************************/
cl_int Verify_Conversions(OCLTestInstance *psInstance)
{
	cl_int eResult = CL_SUCCESS;
	ConversionData* psData = (ConversionData*) psInstance->pvPrivateData;
	void* pRandom = NULL;
	void* pReference = NULL;
	cl_uint* puCalc = NULL;

	if(!psData)
		return CL_OUT_OF_RESOURCES;

	pRandom = malloc(sizeof(cl_uint) * TEST_CONVERSIONS_NUM_VALUES_PER_TYPE);
	pReference = malloc(sizeof(cl_uint) * TEST_CONVERSIONS_NUM_VALUES_PER_TYPE);
	puCalc = malloc(sizeof(cl_uint) * TEST_CONVERSIONS_NUM_VALUES_PER_TYPE);

	if(!pRandom || !pReference || !puCalc)
		goto verify_conversions_cleanup;

	/* Set the RTZ round mode */
	FESETROUND(FE_TOWARDZERO);
	eResult = TestConvertFloatToX(psInstance, pRandom, pReference, puCalc, "rtz");
	CheckAndReportError(psInstance, "TestConvertFloatToX_rtz", eResult, verify_conversions_cleanup);
	eResult = TestConvertXToFloat(psInstance, pRandom, pReference, puCalc, "rtz");
	CheckAndReportError(psInstance, "TestConvertXToFloat_rtz", eResult, verify_conversions_cleanup);
	/* Vector conversions */
	eResult = TestConvertVectorFloatToX(psInstance, pRandom, pReference, puCalc, "rtz");
	CheckAndReportError(psInstance, "TestConvertVectorFloatToX_rtz", eResult, verify_conversions_cleanup);

	/* Set the RTE round mode */
	FESETROUND(FE_TONEAREST);
	eResult = TestConvertFloatToX(psInstance, pRandom, pReference, puCalc, "rte");
	CheckAndReportError(psInstance, "TestConvertFloatToX_rte", eResult, verify_conversions_cleanup);
	eResult = TestConvertXToFloat(psInstance, pRandom, pReference, puCalc, "rte");
	CheckAndReportError(psInstance, "TestConvertXToFloat_rte", eResult, verify_conversions_cleanup);
	/* Vector conversions */
	eResult = TestConvertVectorFloatToX(psInstance, pRandom, pReference, puCalc, "rte");
	CheckAndReportError(psInstance, "TestConvertVectorFloatToX_rte", eResult, verify_conversions_cleanup);

	/* Set the RTP round mode */
	FESETROUND(FE_UPWARD);
	eResult = TestConvertFloatToX(psInstance, pRandom, pReference, puCalc, "rtp");
	CheckAndReportError(psInstance, "TestConvertFloatToX_rtp", eResult, verify_conversions_cleanup);
	eResult = TestConvertXToFloat(psInstance, pRandom, pReference, puCalc, "rtp");
	CheckAndReportError(psInstance, "TestConvertXToFloat_rtp", eResult, verify_conversions_cleanup);
	/* Vector conversions */
	eResult = TestConvertVectorFloatToX(psInstance, pRandom, pReference, puCalc, "rtp");
	CheckAndReportError(psInstance, "TestConvertVectorFloatToX_rtp", eResult, verify_conversions_cleanup);

	/* Set the RTN round mode */
	FESETROUND(FE_DOWNWARD);
	eResult = TestConvertFloatToX(psInstance, pRandom, pReference, puCalc, "rtn");
	CheckAndReportError(psInstance, "TestConvertFloatToX_rtn", eResult, verify_conversions_cleanup);
	eResult = TestConvertXToFloat(psInstance, pRandom, pReference, puCalc, "rtn");
	CheckAndReportError(psInstance, "TestConvertXToFloat_rtn", eResult, verify_conversions_cleanup);
	/* Vector conversions */
	eResult = TestConvertVectorFloatToX(psInstance, pRandom, pReference, puCalc, "rtn");
	CheckAndReportError(psInstance, "TestConvertVectorFloatToX_rtn", eResult, verify_conversions_cleanup);

	/* Non-trivial conversions from integer to integer formats */
	eResult = TestConvertInt32ToX(psInstance, pRandom, pReference, puCalc);
	CheckAndReportError(psInstance, "TestConvertIntToX", eResult, verify_conversions_cleanup);
	eResult = TestConvertInt16ToX(psInstance, pRandom, pReference, puCalc);
	CheckAndReportError(psInstance, "TestConvertShortToX", eResult, verify_conversions_cleanup);

verify_conversions_cleanup:
	free(pRandom);
	free(pReference);
	free(puCalc);

	clReleaseMemObject(psData->psInBuffer);
	clReleaseMemObject(psData->psOutBuffer);
	clReleaseMemObject(psData->psRefBuffer);
	clReleaseDevice(psData->psDeviceID);
	clReleaseCommandQueue(psData->psCommandQueue);
	clReleaseContext(psData->psContext);

	free(psInstance->pvPrivateData);
	psInstance->pvPrivateData = NULL;

	return eResult;
}

#undef VERIFY_DATA

/******************************************************************************
 End of file (conversions.c)
******************************************************************************/
