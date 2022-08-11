/*************************************************************************/ /*!
@File           atomics.c
@Title          OpenCL unit test for atomic operations
@Copyright      Copyright (c) Imagination Technologies Ltd. All Rights Reserved
@Description    USC Backend common Builtin header.
@License        Strictly Confidential.
*/ /**************************************************************************/

#define ATOMIC_WORKGROUP_SIZE_AND 32
#define ATOMIC_WG_SIZE_AND_STR "32"

#define ATOMIC_WORKGROUP_COUNT 1
#define ATOMIC_WG_COUNT_STR "1"

#define ATOMIC_INSTANCE_COUNT_AND (ATOMIC_WORKGROUP_SIZE_AND * ATOMIC_WORKGROUP_COUNT)
#define ATOMIC_BUFFER_SIZE_AND (ATOMIC_INSTANCE_COUNT_AND * sizeof(cl_int))


#if defined(NO_HARDWARE)
static char* g_pszVerifiers =
{
	"#define NUM_INSTANCES (%d * " ATOMIC_WG_COUNT_STR ")\n"
	"\n"
	"bool ValueInBuffer(global int* pvBuff, int value, int size)\n"
	"{\n"
	"\tint i;\n"
	"\tfor (i = 0; i < size / 16; ++i)\n"
	"\t{\n"
	"\t\tint16 temp = vload16(i, pvBuff);\n"
	"\t\tif (temp.s0 == value) return true;\n"
	"\t\tif (temp.s1 == value) return true;\n"
	"\t\tif (temp.s2 == value) return true;\n"
	"\t\tif (temp.s3 == value) return true;\n"
	"\t\tif (temp.s4 == value) return true;\n"
	"\t\tif (temp.s5 == value) return true;\n"
	"\t\tif (temp.s6 == value) return true;\n"
	"\t\tif (temp.s7 == value) return true;\n"
	"\t\tif (temp.s8 == value) return true;\n"
	"\t\tif (temp.s9 == value) return true;\n"
	"\t\tif (temp.sA == value) return true;\n"
	"\t\tif (temp.sB == value) return true;\n"
	"\t\tif (temp.sC == value) return true;\n"
	"\t\tif (temp.sD == value) return true;\n"
	"\t\tif (temp.sE == value) return true;\n"
	"\t\tif (temp.sF == value) return true;\n"
	"\t}\n"
	"\tfor (i = i*16; i < size; ++i)\n"
	"\t{\n"
	"\t\tif (pvBuff[i] == value) return true;\n"
	"\t}\n"
	"\n"
	"\treturn false;\n"
	"}\n"
	"\n"
	"bool ValueDuplicated(global int* pvBuff, int value, int size)"
	"{\n"
	"\tbool bFound = false;\n"
	"\tint i;\n"
	"\tif (ValueInBuffer(pvBuff, value, size))\n"
	"\t{\n"
	"\t\tif (bFound) return 1;\n"
	"\t\tbFound = 1;\n"
	"\t}\n"
	"\n"
	"\treturn 0;\n"
	"}\n"
	"\n"
	/* Here we run 1 instance per buffer element to verify in parallel */
	"kernel void VerifyGlobalAdd(global int* in, global int* out)\n"
	"{\n"
	"\tint i;\n"
	"\tout[get_global_id(0)] = 0xFFFFFFFF;\n"
	"\tif (!ValueInBuffer(in, get_global_id(0) * 2, get_global_size(0)))\n"
	"\t{\n"
	"\t\tout[get_global_id(0)] = 0;\n"
	"\t\treturn;\n"
	"\t}\n"
	"}\n"
	"\n"
	/* Here we run 1 instance per work-group item, to verify each work-group in parallel */
	"kernel void VerifyLocalAdd(global int* in, global int* out)\n"
	"{\n"
	"\tint i;\n"
	"\tout[get_global_id(0)] = 0xFFFFFFFF;\n"
	"\tfor (i = 0; i < " ATOMIC_WG_COUNT_STR "; i++)\n"
	"\t{\n"
	"\t\tif (!ValueInBuffer(&in[i * %d], get_global_id(0) * 2, %d))\n"
	"\t\t{\n"
	"\t\t\tout[get_global_id(0)] = 0;\n"
	"\t\t\treturn;\n"
	"\t\t}\n"
	"\t}\n"
	"}\n"
	"\n"
	"__attribute__((reqd_work_group_size(1, 1, 1)))\n"
	"kernel void VerifyGlobalAnd(global int* in, global int* out)\n"
	"{\n"
	"\tint i;\n"
	"\tfor (i = 0; i < " ATOMIC_WG_COUNT_STR "; i++)"
	"\t{\n"
	"\t\tif (in[i] != 0)\n"
	"\t\t{\n"
	"\t\t\tout[0] = 0;\n"
	"\t\t\treturn;\n"
	"\t\t}\n"
	"\t}\n"
	"\tout[0] = 0xFFFFFFFF;\n"
	"}\n"
	"\n"
	/* Here we run 1 instance per buffer element to verify in parallel */
	"kernel void VerifyGlobalXchg(global int* in, global int* out)\n"
	"{\n"
	"\tint i;\n"
	"\tout[get_global_id(0)] = 0xFFFFFFFF;\n"
	"\tif (ValueDuplicated(in, get_global_id(0), get_global_size(0)))\n"
	"\t{\n"
	"\t\tout[get_global_id(0)] = 0;\n"
	"\t\treturn;\n"
	"\t}\n"
	"}\n"
	"\n"
	/* Here we run 1 instance per work-group item to verify each work-group in parallel */
	"kernel void VerifyLocalXchg(global int* in, global int* out)\n"
	"{\n"
	"\tint i;\n"
	"\tout[get_global_id(0)] = 0xFFFFFFFF;\n"
	"\tfor (i = 0; i < " ATOMIC_WG_COUNT_STR "; i++)\n"
	"\t{\n"
	"\t\tif (ValueDuplicated(&in[i * %d], get_global_id(0), %d))\n"
	"\t\t{\n"
	"\t\t\tout[get_global_id(0)] = 0;\n"
	"\t\t\treturn;\n"
	"\t\t}\n"
	"\t}\n"
	"}\n"
	"\n"
};
#endif

/* specify "__attribute__((reqd_work_group_size(" ATOMIC_WG_SIZE_STR ", 1, 1)))"
 because of the BRN where the kernel enqueues extra instances. */
static char* g_pszAtomicBinaryTemplate =
{
	"__attribute__((reqd_work_group_size(%d, 1, 1)))\n"
	"kernel void GlobalAtomicTest(global int* in, global int* out)\n"
	"{\n"
	"\tint gid = get_global_id(0);\n"
	"\tout[gid] = atomic_%s(in, 2);\n"
	"}\n"
	"\n"
	"__attribute__((reqd_work_group_size(%d, 1, 1)))\n"
	"kernel void LocalAtomicTest(global int* in, global int* out)\n"
	"{\n"
	"\tlocal int loc;\n"
	"\tint gid = get_global_id(0);\n"
	"\tif (get_local_id(0) == 0) loc = 0;\n"
	"\tbarrier(CLK_LOCAL_MEM_FENCE);\n"
	"\tout[gid] = atomic_%s(&loc, 2);\n"
	"}\n"
	"\n"
};

static char* g_pszAtomicAND =
{
	"__attribute__((reqd_work_group_size(" ATOMIC_WG_SIZE_AND_STR ", 1, 1)))\n"
	"kernel void GlobalAtomicTest(global int* in)\n"
	"{\n"
	"\tint gid = get_global_id(0);\n"
	"\tuint mask = ~ (1 << get_local_id(0));\n"
	"\tatomic_and(in + get_group_id(0), mask);\n"
	"}\n"
	"\n"
	"__attribute__((reqd_work_group_size(" ATOMIC_WG_SIZE_AND_STR ", 1, 1)))\n"
	"kernel void LocalAtomicTest(global int* in, global int* out)\n"
	"{\n"
	"\tlocal int loc;\n"
	"\tint gid = get_global_id(0);\n"
	"\tuint mask = ~ (1 << get_local_id(0));\n"
	"\tif (get_local_id(0) == 0) loc = 0xFFFFFFFF;\n"
	"\tbarrier(CLK_LOCAL_MEM_FENCE);\n"
	"\tatomic_and(&loc, mask);\n"
	"\tbarrier(CLK_LOCAL_MEM_FENCE);\n"
	"\tif (get_local_id(0) == 0) out[get_group_id(0)] = ~loc;\n"
	"}\n"
	"\n"
};

static char* g_pszAtomicXCHG =
{
	"__attribute__((reqd_work_group_size(%d, 1, 1)))\n"
	"kernel void GlobalAtomicTest(global int* in, global int* out)\n"
	"{\n"
	"\tint gid = get_global_id(0);\n"
	"\tout[gid] = atomic_xchg(in, gid);\n"
	"}\n"
	"\n"
	"__attribute__((reqd_work_group_size(%d, 1, 1)))\n"
	"kernel void LocalAtomicTest(global int* in, global int* out)\n"
	"{\n"
	"\tlocal int loc;"
	"\tint lid = get_local_id(0);\n"
	"\tint gid = get_global_id(0);\n"
	"\tloc = 0xFFFFFFFF;\n"
	"\tbarrier(CLK_LOCAL_MEM_FENCE);\n"
	"\tout[gid] = atomic_xchg(&loc, lid);\n"
	"}\n"
	"\n"

};

typedef struct _AtomicsData_
{
	cl_context       psContext;
	cl_device_id     psDeviceID;
	cl_platform_id   psPlatformID;
	cl_command_queue psCommandQueue;
	cl_mem           psInBuffer;
	cl_mem           psOutBuffer;
#if defined(NO_HARDWARE)
	cl_program       psVerifiers;
#endif
} AtomicsData;

cl_int Init_Atomics(OCLTestInstance* psInstance);
cl_int Verify_Atomics(OCLTestInstance* psInstance);

#if !defined(NO_HARDWARE)
static cl_bool ValueInBuffer(int* piBuff, int size, int value);
static cl_bool ValueDuplicated(int* piBuff, int size, int value);
#endif /* !defined(NO_HARDWARE) */

/***********************************************************************************
 Function Name      : Init_Atomics
 Description        : Initialises input data
************************************************************************************/
cl_int Init_Atomics(OCLTestInstance* psInstance)
{
	cl_int eResult = CL_SUCCESS;
	AtomicsData sData;
	size_t wg_size;

	eResult = clGetPlatformIDs(1, &sData.psPlatformID, NULL);
	CheckAndReportError(psInstance, "clGetPlatformIDs", eResult, init_atomics_cleanup);

	eResult = clGetDeviceIDs(NULL, CL_DEVICE_TYPE_GPU, 1, &sData.psDeviceID, NULL);
	CheckAndReportError(psInstance, "clGetPlatformIDs", eResult, init_atomics_cleanup);

	sData.psContext = clCreateContext(NULL, 1, &sData.psDeviceID, NULL, NULL, &eResult);
	CheckAndReportError(psInstance, "clCreateContext", eResult, init_atomics_cleanup);

	sData.psCommandQueue = clCreateCommandQueue(sData.psContext, sData.psDeviceID, 0, &eResult);
	CheckAndReportError(psInstance, "clCreateCommandQueue", eResult, init_atomics_cleanup);

	eResult = clGetDeviceInfo(sData.psDeviceID, CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(size_t), &wg_size, NULL);
	CheckAndReportError(psInstance, "clGetDeviceInfo", eResult, init_atomics_cleanup);

#if defined(NO_HARDWARE)
	{
		char pszSource[4096];
		char *ppszSources[2] = {0,0};

		snprintf(pszSource, 4096, g_pszVerifiers, wg_size, wg_size, wg_size, wg_size, wg_size, wg_size, wg_size);
		ppszSources[0] = pszSource;

		sData.psVerifiers = clCreateProgramWithSource(sData.psContext, 1, (const char**)ppszSources, NULL, &eResult);
		CheckAndReportError(psInstance, "clCreateProgramWithSource", eResult, init_atomics_cleanup);
		eResult = clBuildProgram(sData.psVerifiers, 0, NULL, NULL, NULL, NULL);
		CheckAndReportError(psInstance, "clBuildProgram", eResult, init_atomics_cleanup);
	}
#endif

	sData.psInBuffer = clCreateBuffer(sData.psContext, CL_MEM_READ_WRITE, wg_size * sizeof(cl_uint), NULL, &eResult);
	CheckAndReportError(psInstance, "clCreateBuffer", eResult, init_atomics_cleanup);

	sData.psOutBuffer = clCreateBuffer(sData.psContext, CL_MEM_WRITE_ONLY, wg_size * sizeof(cl_uint), NULL, &eResult);
	CheckAndReportError(psInstance, "clCreateBuffer", eResult, init_atomics_cleanup);

	psInstance->pvPrivateData = calloc(1, sizeof(AtomicsData));

	if(!psInstance->pvPrivateData)
		return CL_OUT_OF_RESOURCES;

	memcpy(psInstance->pvPrivateData, &sData, sizeof(AtomicsData));

init_atomics_cleanup:
	return eResult;
}

/***********************************************************************************
 Function Name      : Verify_Atomics
 Description        : Verifies output data
************************************************************************************/
cl_int Verify_Atomics(OCLTestInstance* psInstance)
{
	AtomicsData* psData = psInstance->pvPrivateData;
	unsigned uFrame = 0;
	cl_int eResult = CL_SUCCESS;
	cl_program psProgram = NULL;
	cl_kernel psKernel = NULL;
	char pszSource[1024];
	char* ppszSources[2] = {pszSource, 0};
	size_t ATOMIC_WORKGROUP_SIZE, ATOMIC_INSTANCE_COUNT, ATOMIC_BUFFER_SIZE;
	void* pvZero = NULL;
	int* piBuffer = NULL;
	size_t global;
	size_t local;
#if defined(NO_HARDWARE)
	const size_t one = 1;
	cl_kernel psVer = NULL;
	cl_mem psVerOut = NULL;
#else
	unsigned int i;
#endif

	eResult = clGetDeviceInfo(psData->psDeviceID, CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(size_t), &ATOMIC_WORKGROUP_SIZE, NULL);
	CheckAndReportError(psInstance, "clGetDeviceInfo", eResult, verify_atomics_cleanup);

	ATOMIC_INSTANCE_COUNT = ATOMIC_WORKGROUP_SIZE * (ATOMIC_WORKGROUP_COUNT);
	ATOMIC_BUFFER_SIZE = ATOMIC_INSTANCE_COUNT * sizeof(cl_int);

	pvZero = calloc(ATOMIC_INSTANCE_COUNT, sizeof(cl_int));
	piBuffer = malloc(ATOMIC_BUFFER_SIZE);

	if(!piBuffer)
	{
		eResult = CL_OUT_OF_RESOURCES;
		goto verify_atomics_cleanup;
	}

	global = ATOMIC_INSTANCE_COUNT;

#if defined(NO_HARDWARE)
	psVerOut = clCreateBuffer(psData->psContext, CL_MEM_WRITE_ONLY, sizeof(cl_uint) * ATOMIC_INSTANCE_COUNT, NULL, &eResult);
	CheckAndReportError(psInstance, "clCreateBuffer", eResult, verify_atomics_cleanup);
#endif

	/**************/
	/* atomic_add */
	/**************/
	local = ATOMIC_WORKGROUP_SIZE;
	printf("Kick %u: Running global atomic_add...\n", uFrame);
	eResult = clEnqueueWriteBuffer(psData->psCommandQueue, psData->psInBuffer, CL_FALSE, 0,
		ATOMIC_BUFFER_SIZE, pvZero, 0, NULL, NULL);
	CheckAndReportError(psInstance, "clEnqueueWriteBuffer", eResult, verify_atomics_cleanup);
	snprintf(pszSource, 1024, g_pszAtomicBinaryTemplate, local, "add", local, "add");
	psProgram = clCreateProgramWithSource(psData->psContext, 1, (const char**)ppszSources, NULL, &eResult);
	CheckAndReportError(psInstance, "clCreateProgramWithSource", eResult, verify_atomics_cleanup);
	eResult = clBuildProgram(psProgram, 0, NULL, NULL, NULL, NULL);
	CheckAndReportError(psInstance, "clBuildProgram", eResult, verify_atomics_cleanup);
	psKernel = clCreateKernel(psProgram, "GlobalAtomicTest", &eResult);
	CheckAndReportError(psInstance, "clCreateKernel", eResult, verify_atomics_cleanup);
	eResult = clSetKernelArg(psKernel, 0, sizeof(cl_mem), &psData->psInBuffer);
	CheckAndReportError(psInstance, "clSetKernelArg", eResult, verify_atomics_cleanup);
	eResult = clSetKernelArg(psKernel, 1, sizeof(cl_mem), &psData->psOutBuffer);
	CheckAndReportError(psInstance, "clSetKernelArg", eResult, verify_atomics_cleanup);

	eResult = clEnqueueNDRangeKernel(psData->psCommandQueue, psKernel, 1, NULL, &global, &local, 0, NULL, NULL);
	CheckAndReportError(psInstance, "clEnqueueNDRangeKernel", eResult, verify_atomics_cleanup);
	clReleaseKernel(psKernel);
	psKernel = NULL;

#if defined(NO_HARDWARE)
	psVer = clCreateKernel(psData->psVerifiers, "VerifyGlobalAdd", &eResult);
	CheckAndReportError(psInstance, "clCreateKernel", eResult, verify_atomics_cleanup);
	eResult = clSetKernelArg(psVer, 0, sizeof(cl_mem), &psData->psOutBuffer);
	CheckAndReportError(psInstance, "clSetKernelArg", eResult, verify_atomics_cleanup);
	eResult = clSetKernelArg(psVer, 1, sizeof(cl_mem), &psVerOut);
	CheckAndReportError(psInstance, "clSetKernelArg", eResult, verify_atomics_cleanup);
	eResult = clEnqueueNDRangeKernel(psData->psCommandQueue, psVer, 1, NULL, &global, &local, 0, NULL, NULL);
	CheckAndReportError(psInstance, "clEnqueueNDRangeKernel", eResult, verify_atomics_cleanup);
	eResult = clEnqueueReadBuffer(psData->psCommandQueue, psVerOut, CL_TRUE, 0, sizeof(cl_uint) * ATOMIC_INSTANCE_COUNT, piBuffer, 0, NULL, NULL);
	CheckAndReportError(psInstance, "clEnqueueReadBuffer", eResult, verify_atomics_cleanup);
	clReleaseKernel(psVer);
	psVer = NULL;
#else
	eResult = clEnqueueReadBuffer(psData->psCommandQueue, psData->psOutBuffer, CL_TRUE, 0,
		ATOMIC_BUFFER_SIZE, piBuffer, 0, NULL, NULL);
	CheckAndReportError(psInstance, "clEnqueueReadBuffer", eResult, verify_atomics_cleanup);

	for (i = 0; i < ATOMIC_INSTANCE_COUNT; i++)
	{
		if (!ValueInBuffer(piBuffer, ATOMIC_INSTANCE_COUNT, i * 2))
		{
			OCLTestLog("%s: Verification failure: Value %d is not in output buffer.\n",
		       __func__,
			   i);

			eResult = CL_INVALID_VALUE;
			goto verify_atomics_cleanup;
		}
	}
#endif

	uFrame += 1;

	printf("Kick %u: Running local atomic_add...\n", uFrame);
	eResult = clEnqueueWriteBuffer(psData->psCommandQueue, psData->psInBuffer, CL_FALSE, 0,
		ATOMIC_BUFFER_SIZE, pvZero, 0, NULL, NULL);
	CheckAndReportError(psInstance, "clEnqueueWriteBuffer", eResult, verify_atomics_cleanup);
	psKernel = clCreateKernel(psProgram, "LocalAtomicTest", &eResult);
	CheckAndReportError(psInstance, "clCreateKernel", eResult, verify_atomics_cleanup);
	eResult = clSetKernelArg(psKernel, 0, sizeof(cl_mem), &psData->psInBuffer);
	CheckAndReportError(psInstance, "clSetKernelArg", eResult, verify_atomics_cleanup);
	eResult = clSetKernelArg(psKernel, 1, sizeof(cl_mem), &psData->psOutBuffer);
	CheckAndReportError(psInstance, "clSetKernelArg", eResult, verify_atomics_cleanup);

	eResult = clEnqueueNDRangeKernel(psData->psCommandQueue, psKernel, 1, NULL, &global, &local, 0, NULL, NULL);
	CheckAndReportError(psInstance, "clEnqueueNDRangeKernel", eResult, verify_atomics_cleanup);
	clReleaseKernel(psKernel);
	psKernel = NULL;

#if defined(NO_HARDWARE)
	psVer = clCreateKernel(psData->psVerifiers, "VerifyLocalAdd", &eResult);
	CheckAndReportError(psInstance, "clCreateKernel", eResult, verify_atomics_cleanup);
	eResult = clSetKernelArg(psVer, 0, sizeof(cl_mem), &psData->psOutBuffer);
	CheckAndReportError(psInstance, "clSetKernelArg", eResult, verify_atomics_cleanup);
	eResult = clSetKernelArg(psVer, 1, sizeof(cl_mem), &psVerOut);
	CheckAndReportError(psInstance, "clSetKernelArg", eResult, verify_atomics_cleanup);
	/* Here we verify using one instance per work-group */
	eResult = clEnqueueNDRangeKernel(psData->psCommandQueue, psVer, 1, NULL, &local, &local, 0, NULL, NULL);
	CheckAndReportError(psInstance, "clEnqueueNDRangeKernel", eResult, verify_atomics_cleanup);
	eResult = clEnqueueReadBuffer(psData->psCommandQueue, psVerOut, CL_TRUE, 0, sizeof(cl_uint) * ATOMIC_WORKGROUP_SIZE, piBuffer, 0, NULL, NULL);
	CheckAndReportError(psInstance, "clEnqueueReadBuffer", eResult, verify_atomics_cleanup);
	clReleaseKernel(psVer);
	psVer = NULL;
#else
	eResult = clEnqueueReadBuffer(psData->psCommandQueue, psData->psOutBuffer, CL_TRUE, 0,
		ATOMIC_BUFFER_SIZE, piBuffer, 0, NULL, NULL);
	CheckAndReportError(psInstance, "clEnqueueReadBuffer", eResult, verify_atomics_cleanup);

	for (i = 0; i < ATOMIC_INSTANCE_COUNT; i++)
	{
		if (!ValueInBuffer(piBuffer, ATOMIC_INSTANCE_COUNT, i * 2))
		{
			OCLTestLog("%s: Verification failure: Value %d is not in output buffer.\n",
			   __func__,
			   i);

			eResult = CL_INVALID_VALUE;
			goto verify_atomics_cleanup;
		}
	}
#endif

	clReleaseProgram(psProgram);
	psProgram = NULL;

	uFrame += 1;

	/**************/
	/* atomic_and */
	/**************/
	local = ATOMIC_WORKGROUP_SIZE_AND;
	printf("Kick %u: Running global atomic_and...\n", uFrame);
	memset(piBuffer, 0xFF, ATOMIC_WORKGROUP_COUNT * sizeof(cl_uint));
	eResult = clEnqueueWriteBuffer(psData->psCommandQueue, psData->psInBuffer, CL_FALSE, 0,
		ATOMIC_WORKGROUP_COUNT * sizeof(cl_uint), piBuffer, 0, NULL, NULL);
	CheckAndReportError(psInstance, "clEnqueueWriteBuffer", eResult, verify_atomics_cleanup);
	strncpy(pszSource, g_pszAtomicAND, 1024);
	psProgram = clCreateProgramWithSource(psData->psContext, 1, (const char**)ppszSources, NULL, &eResult);
	CheckAndReportError(psInstance, "clCreateProgramFromSource", eResult, verify_atomics_cleanup);
	eResult = clBuildProgram(psProgram, 0, NULL, NULL, NULL, NULL);
	CheckAndReportError(psInstance, "clBuildProgram", eResult, verify_atomics_cleanup);
	psKernel = clCreateKernel(psProgram, "GlobalAtomicTest", &eResult);
	CheckAndReportError(psInstance, "clCreateKernel", eResult, verify_atomics_cleanup);
	eResult = clSetKernelArg(psKernel, 0, sizeof(cl_mem), &psData->psInBuffer);
	CheckAndReportError(psInstance, "clSetKernelArg", eResult, verify_atomics_cleanup);

	eResult = clEnqueueNDRangeKernel(psData->psCommandQueue, psKernel, 1, NULL, &global, &local, 0, NULL, NULL);
	CheckAndReportError(psInstance, "clEnqueueNDRangeKernel", eResult, verify_atomics_cleanup);
	clReleaseKernel(psKernel);
	psKernel = NULL;

#if defined(NO_HARDWARE)
	{
		psVer = clCreateKernel(psData->psVerifiers, "VerifyGlobalAnd", &eResult);
		CheckAndReportError(psInstance, "clCreateKernel", eResult, verify_atomics_cleanup);
		eResult = clSetKernelArg(psVer, 0, sizeof(cl_mem), &psData->psInBuffer);
		CheckAndReportError(psInstance, "clSetKernelArg", eResult, verify_atomics_cleanup);
		eResult = clSetKernelArg(psVer, 1, sizeof(cl_mem), &psVerOut);
		CheckAndReportError(psInstance, "clSetKernelArg", eResult, verify_atomics_cleanup);
		eResult = clEnqueueNDRangeKernel(psData->psCommandQueue, psVer, 1, NULL, &one, &one, 0, NULL, NULL);
		CheckAndReportError(psInstance, "clEnqueueNDRangeKernel", eResult, verify_atomics_cleanup);
		eResult = clEnqueueReadBuffer(psData->psCommandQueue, psVerOut, CL_TRUE, 0, sizeof(cl_uint), piBuffer, 0, NULL, NULL);
		CheckAndReportError(psInstance, "clEnqueueReadBuffer", eResult, verify_atomics_cleanup);
		clReleaseKernel(psVer);
		psVer = NULL;
	}
#else
	{
		int i;

		eResult = clEnqueueReadBuffer(psData->psCommandQueue, psData->psInBuffer, CL_TRUE, 0,
		ATOMIC_WORKGROUP_COUNT * sizeof(cl_uint), piBuffer, 0, NULL, NULL);
		CheckAndReportError(psInstance, "clEnqueueReadBuffer", eResult, verify_atomics_cleanup);

		for (i = 0; i < ATOMIC_WORKGROUP_COUNT; i++)
		{
			if(piBuffer[i] != 0)
			{
				OCLTestLog("%s: Verification failure: Got %d but expected 0 at %d.\n",
			       __func__,
				   piBuffer[i],
				   i);

				eResult = CL_INVALID_VALUE;
				goto verify_atomics_cleanup;
			}
		}
	}
#endif

	uFrame += 1;

	printf("Kick %u: Running local atomic_and...\n", uFrame);
	eResult = clEnqueueWriteBuffer(psData->psCommandQueue, psData->psInBuffer, CL_FALSE, 0,
		ATOMIC_BUFFER_SIZE, pvZero, 0, NULL, NULL);
	CheckAndReportError(psInstance, "clEnqueueWriteBuffer", eResult, verify_atomics_cleanup);
	psKernel = clCreateKernel(psProgram, "LocalAtomicTest", &eResult);
	CheckAndReportError(psInstance, "clCreateKernel", eResult, verify_atomics_cleanup);
	eResult = clSetKernelArg(psKernel, 0, sizeof(cl_mem), &psData->psInBuffer);
	CheckAndReportError(psInstance, "clSetKernelArg", eResult, verify_atomics_cleanup);
	eResult = clSetKernelArg(psKernel, 1, sizeof(cl_mem), &psData->psOutBuffer);
	CheckAndReportError(psInstance, "clSetKernelArg", eResult, verify_atomics_cleanup);

	eResult = clEnqueueNDRangeKernel(psData->psCommandQueue, psKernel, 1, NULL, &global, &local, 0, NULL, NULL);
	CheckAndReportError(psInstance, "clEnqueueNDRangeKernel", eResult, verify_atomics_cleanup);
	clReleaseKernel(psKernel);
	psKernel = NULL;

#if defined(NO_HARDWARE)
	{
		const size_t one = 1;
		psVer = clCreateKernel(psData->psVerifiers, "VerifyGlobalAnd", &eResult);
		CheckAndReportError(psInstance, "clCreateKernel", eResult, verify_atomics_cleanup);
		eResult = clSetKernelArg(psVer, 0, sizeof(cl_mem), &psData->psInBuffer);
		CheckAndReportError(psInstance, "clSetKernelArg", eResult, verify_atomics_cleanup);
		eResult = clSetKernelArg(psVer, 1, sizeof(cl_mem), &psVerOut);
		CheckAndReportError(psInstance, "clSetKernelArg", eResult, verify_atomics_cleanup);
		eResult = clEnqueueNDRangeKernel(psData->psCommandQueue, psVer, 1, NULL, &one, &one, 0, NULL, NULL);
		CheckAndReportError(psInstance, "clEnqueueNDRangeKernel", eResult, verify_atomics_cleanup);
		eResult = clEnqueueReadBuffer(psData->psCommandQueue, psVerOut, CL_TRUE, 0, sizeof(cl_uint), piBuffer, 0, NULL, NULL);
		CheckAndReportError(psInstance, "clEnqueueReadBuffer", eResult, verify_atomics_cleanup);
		clReleaseKernel(psVer);
		psVer = NULL;
	}
#else
	{
		int i;

		eResult = clEnqueueReadBuffer(psData->psCommandQueue, psData->psInBuffer, CL_TRUE, 0,
		ATOMIC_WORKGROUP_COUNT * sizeof(cl_uint), piBuffer, 0, NULL, NULL);
		CheckAndReportError(psInstance, "clEnqueueReadBuffer", eResult, verify_atomics_cleanup);

		for (i = 0; i < ATOMIC_WORKGROUP_COUNT; i++)
		{
			if(piBuffer[i] != 0)
			{
				OCLTestLog("%s: Verification failure: Got %d but expected 0 at %d.\n",
			       __func__,
				   piBuffer[i],
				   i);

				eResult = CL_INVALID_VALUE;
				goto verify_atomics_cleanup;
			}
		}
	}
#endif

	clReleaseProgram(psProgram);
	psProgram = NULL;

	uFrame += 1;

	/***************/
	/* atomic_xchg */
	/***************/
	local = ATOMIC_WORKGROUP_SIZE;
	printf("Kick %u: Running global atomic_xhg...\n", uFrame);
	memset(piBuffer, 0xFF, ATOMIC_WORKGROUP_COUNT * sizeof(cl_uint));
	eResult = clEnqueueWriteBuffer(psData->psCommandQueue, psData->psInBuffer, CL_FALSE, 0,
		ATOMIC_WORKGROUP_COUNT * sizeof(cl_uint), piBuffer, 0, NULL, NULL);
	CheckAndReportError(psInstance, "clEnqueueWriteBuffer", eResult, verify_atomics_cleanup);
	snprintf(pszSource, 1024, g_pszAtomicXCHG, local, local);
	psProgram = clCreateProgramWithSource(psData->psContext, 1, (const char**)ppszSources, NULL, &eResult);
	CheckAndReportError(psInstance, "clCreateProgramWithSource", eResult, verify_atomics_cleanup);
	eResult = clBuildProgram(psProgram, 0, NULL, NULL, NULL, NULL);
	CheckAndReportError(psInstance, "clBuildProgram", eResult, verify_atomics_cleanup);
	psKernel = clCreateKernel(psProgram, "GlobalAtomicTest", &eResult);
	CheckAndReportError(psInstance, "clCreateKernel", eResult, verify_atomics_cleanup);
	eResult = clSetKernelArg(psKernel, 0, sizeof(cl_mem), &psData->psInBuffer);
	CheckAndReportError(psInstance, "clSetKernelArg", eResult, verify_atomics_cleanup);
	eResult = clSetKernelArg(psKernel, 1, sizeof(cl_mem), &psData->psOutBuffer);
	CheckAndReportError(psInstance, "clSetKernelArg", eResult, verify_atomics_cleanup);

	eResult = clEnqueueNDRangeKernel(psData->psCommandQueue, psKernel, 1, NULL, &global, &local, 0, NULL, NULL);
	CheckAndReportError(psInstance, "clEnqueueNDRangeKernel", eResult, verify_atomics_cleanup);
	clReleaseKernel(psKernel);
	psKernel = NULL;

#if defined(NO_HARDWARE)
	{
		psVer = clCreateKernel(psData->psVerifiers, "VerifyGlobalXchg", &eResult);
		CheckAndReportError(psInstance, "clCreateKernel", eResult, verify_atomics_cleanup);
		eResult = clSetKernelArg(psVer, 0, sizeof(cl_mem), &psData->psOutBuffer);
		CheckAndReportError(psInstance, "clSetKernelArg", eResult, verify_atomics_cleanup);
		eResult = clSetKernelArg(psVer, 1, sizeof(cl_mem), &psVerOut);
		CheckAndReportError(psInstance, "clSetKernelArg", eResult, verify_atomics_cleanup);
		eResult = clEnqueueNDRangeKernel(psData->psCommandQueue, psVer, 1, NULL, &global, &local, 0, NULL, NULL);
		CheckAndReportError(psInstance, "clEnqueueNDRangeKernel", eResult, verify_atomics_cleanup);
		eResult = clEnqueueReadBuffer(psData->psCommandQueue, psVerOut, CL_TRUE, 0, sizeof(cl_uint) * ATOMIC_INSTANCE_COUNT, piBuffer, 0, NULL, NULL);
		CheckAndReportError(psInstance, "clEnqueueReadBuffer", eResult, verify_atomics_cleanup);
		clReleaseKernel(psVer);
		psVer = NULL;
	}
#else
	{
		unsigned int i;

		eResult = clEnqueueReadBuffer(psData->psCommandQueue, psData->psOutBuffer, CL_TRUE, 0,
			ATOMIC_BUFFER_SIZE, piBuffer, 0, NULL, NULL);
		CheckAndReportError(psInstance, "clEnqueueReadBuffer", eResult, verify_atomics_cleanup);

		for (i = 0; i < ATOMIC_INSTANCE_COUNT; i++)
		{
			if (ValueDuplicated(piBuffer, ATOMIC_INSTANCE_COUNT, i))
			{
				OCLTestLog("%s: Verification failure: Duplicate value %d found.\n",
			       __func__,
				   i);

				eResult = CL_INVALID_VALUE;
				goto verify_atomics_cleanup;
			}
		}
	}
#endif

	uFrame += 1;

	printf("Kick %u: Running local atomic_xchg...\n", uFrame);
	memset(piBuffer, 0xFF, ATOMIC_WORKGROUP_COUNT * sizeof(cl_uint));
	eResult = clEnqueueWriteBuffer(psData->psCommandQueue, psData->psInBuffer, CL_FALSE, 0,
		ATOMIC_WORKGROUP_COUNT * sizeof(cl_uint), piBuffer, 0, NULL, NULL);
	CheckAndReportError(psInstance, "clEnqueueWriteBuffer", eResult, verify_atomics_cleanup);
	psKernel = clCreateKernel(psProgram, "LocalAtomicTest", &eResult);
	CheckAndReportError(psInstance, "clCreateKernel", eResult, verify_atomics_cleanup);
	eResult = clSetKernelArg(psKernel, 0, sizeof(cl_mem), &psData->psInBuffer);
	CheckAndReportError(psInstance, "clSetKernelArg", eResult, verify_atomics_cleanup);
	eResult = clSetKernelArg(psKernel, 1, sizeof(cl_mem), &psData->psOutBuffer);
	CheckAndReportError(psInstance, "clSetKernelArg", eResult, verify_atomics_cleanup);

	eResult = clEnqueueNDRangeKernel(psData->psCommandQueue, psKernel, 1, NULL, &global, &local, 0, NULL, NULL);
	CheckAndReportError(psInstance, "clEnqueueNDRangeKernel", eResult, verify_atomics_cleanup);
	clReleaseKernel(psKernel);
	psKernel = NULL;

#if defined(NO_HARDWARE)
	{
		psVer = clCreateKernel(psData->psVerifiers, "VerifyLocalXchg", &eResult);
		CheckAndReportError(psInstance, "clCreateKernel", eResult, verify_atomics_cleanup);
		eResult = clSetKernelArg(psVer, 0, sizeof(cl_mem), &psData->psInBuffer);
		CheckAndReportError(psInstance, "clSetKernelArg", eResult, verify_atomics_cleanup);
		eResult = clSetKernelArg(psVer, 1, sizeof(cl_mem), &psVerOut);
		CheckAndReportError(psInstance, "clSetKernelArg", eResult, verify_atomics_cleanup);
		eResult = clEnqueueNDRangeKernel(psData->psCommandQueue, psVer, 1, NULL, &local, &local, 0, NULL, NULL);
		CheckAndReportError(psInstance, "clEnqueueNDRangeKernel", eResult, verify_atomics_cleanup);
		eResult = clEnqueueReadBuffer(psData->psCommandQueue, psVerOut, CL_TRUE, 0, sizeof(cl_uint) * ATOMIC_WORKGROUP_SIZE, piBuffer, 0, NULL, NULL);
		CheckAndReportError(psInstance, "clEnqueueReadBuffer", eResult, verify_atomics_cleanup);
		clReleaseKernel(psVer);
		psVer = NULL;
	}
#else
	{
		unsigned int i;

		eResult = clEnqueueReadBuffer(psData->psCommandQueue, psData->psOutBuffer, CL_TRUE, 0,
			ATOMIC_BUFFER_SIZE, piBuffer, 0, NULL, NULL);
		CheckAndReportError(psInstance, "clEnqueueReadBuffer", eResult, verify_atomics_cleanup);

		for (i = 0; i < ATOMIC_INSTANCE_COUNT; i++)
		{
			if (ValueDuplicated(piBuffer, ATOMIC_INSTANCE_COUNT, i))
			{
				OCLTestLog("%s: Verification failure: Duplicate value %d found.\n",
			       __func__,
				   i);

				eResult = CL_INVALID_VALUE;
				goto verify_atomics_cleanup;
			}
		}
	}
#endif

	clReleaseProgram(psProgram);
	psProgram = NULL;

	uFrame += 1;

	clFinish(psData->psCommandQueue);

verify_atomics_cleanup:
	if (psKernel) clReleaseKernel(psKernel);
	if (psProgram) clReleaseProgram(psProgram);
#if defined(NO_HARDWARE)
	if (psVerOut) clReleaseMemObject(psVerOut);
	if (psVer) clReleaseKernel(psVer);
	if (psData->psVerifiers) clReleaseProgram(psData->psVerifiers);
#endif
	if(psData->psInBuffer) 		clReleaseMemObject(psData->psInBuffer);
	if(psData->psOutBuffer) 	clReleaseMemObject(psData->psOutBuffer);
	if(psData->psCommandQueue) 	clReleaseCommandQueue(psData->psCommandQueue);
	if(psData->psContext)		clReleaseContext(psData->psContext);
	free(pvZero);
	free(piBuffer);

	free(psInstance->pvPrivateData);
	psInstance->pvPrivateData = NULL;

	return eResult;
}

#if !defined(NO_HARDWARE)
static cl_bool ValueInBuffer(int* piBuff, int size, int value)
{
	int i;
	for (i = 0; i < size; ++i)
	{
		if (piBuff[i] == value)
		{
			return CL_TRUE;
		}
	}

	return CL_FALSE;
}

static cl_bool ValueDuplicated(int* piBuff, int size, int value)
{
	cl_bool bFound = CL_FALSE;
	int i;
	for (i = 0; i < size; ++i)
	{
		if (piBuff[i] == value)
		{
			if (bFound) return CL_TRUE;
			bFound = CL_TRUE;
		}
	}

	return CL_FALSE;
}
#endif /* !defined(NO_HARDWARE) */
