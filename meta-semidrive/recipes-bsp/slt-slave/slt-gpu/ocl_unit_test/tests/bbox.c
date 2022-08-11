/*************************************************************************/ /*!
@File           bbox.c
@Copyright      Copyright (c) Imagination Technologies Ltd. All Rights Reserved
@License        Strictly Confidential.
*/ /**************************************************************************/

#define TOSTR(x) #x
#define STR(x) TOSTR(x)
#define TEST_BBOX_GLOBAL_WORK_SIZE	(512)
#define TEST_BBOX_REQUIRED_WORKGROUP_SIZE	(32)
#define TEST_BBOX_INPUT_SIZE						((TEST_BBOX_GLOBAL_WORK_SIZE*3 * 30 ) + 99)
#define TEST_BBOX_BUFFER_SIZE						 (TEST_BBOX_INPUT_SIZE * 3)
/* Number of objects defines number of independent kernels to spawn. */
#ifdef CUT_DOWN_UNIT_TEST
#define TEST_BBOX_OBJECT_COUNT 10
#else
#define TEST_BBOX_OBJECT_COUNT 25
#endif
#include <float.h>
#include <stdlib.h>

typedef struct
{
	/* CL Objects */
	cl_context       psContext;
	cl_program       psProgram;
	cl_device_id     psDeviceID;
	cl_platform_id   psPlatformID;
	cl_command_queue psCommandQueue;
	cl_kernel        psKernel;
	cl_kernel        psKernelCollect;
	cl_mem           psInputIndexBuffer[TEST_BBOX_OBJECT_COUNT];
	cl_mem           psInputDataBuffer[TEST_BBOX_OBJECT_COUNT];
	cl_mem           psOutputMinBuffer[TEST_BBOX_OBJECT_COUNT];
	cl_mem           psOutputMaxBuffer[TEST_BBOX_OBJECT_COUNT];
	cl_mem           psOutputGlobalMinBuffer[TEST_BBOX_OBJECT_COUNT];
	cl_mem           psOutputGlobalMaxBuffer[TEST_BBOX_OBJECT_COUNT];
	cl_event         psEvent[TEST_BBOX_OBJECT_COUNT];
	cl_event         psEventCollect[TEST_BBOX_OBJECT_COUNT];

	/* Host Object */
	unsigned short *pui32IndexesBuffer[TEST_BBOX_OBJECT_COUNT];
	float *pf32VerticesBuffer[TEST_BBOX_OBJECT_COUNT];
	size_t        puGlobalWorkSize[3];
	size_t        puLocalWorkSize[3];
	float psKnownMins[TEST_BBOX_OBJECT_COUNT][3];
	float psKnownMaxes[TEST_BBOX_OBJECT_COUNT][3];
} BBoxKernelData;

cl_int Init_BBoxKernel(OCLTestInstance *psInstance);
cl_int Verify_BBoxKernel(OCLTestInstance *psInstance);


static char *g_pszBBoxKernelSource =
{
	"#define TEST_BBOX_WG_SIZE " STR(TEST_BBOX_REQUIRED_WORKGROUP_SIZE) \
	"\n"
	"#define TEST_BBOX_GLOBAL_SIZE " STR(TEST_BBOX_GLOBAL_WORK_SIZE) \
	"\n"
	"\n"
	"__kernel  __attribute__((reqd_work_group_size(TEST_BBOX_WG_SIZE,1,1)))\n"
	"void BoundingBox(__global ushort2* indexes, __global float* vertices, __global float* mins,\n"
	"					__global float* maxes, const uint numberOfTriangles, const uint stride)\n"
	"{\n"
	"	unsigned int i;\n"
	"\n"
	"	unsigned int offset = get_global_size(0);\n"
	"	unsigned int gid = get_global_id(0);\n"
	"	unsigned int ind = gid;\n"
	"\n"
	"	ushort2 uIndex1;\n"
	"	ushort2 uIndex2;\n"
	"	ushort2 uIndex3;\n"
	"	float3 f3V[3];\n"
	"\n"
	"	float3 min1 = (float3)(MAXFLOAT), min2 = (float3)(MAXFLOAT);\n"
	"	float3 max1 = (float3)(-MAXFLOAT), max2 = (float3)(-MAXFLOAT);\n"
	"\n"
	"	unsigned int size = numberOfTriangles >> 1;\n"
	"   /* Bravely discarding last triangle. */\n"
	"\n"
	"	for (; ind < size; ind += offset)\n"
	"	{\n"
	"		uIndex1 = indexes[3*ind];\n"
	"		uIndex2 = indexes[3*ind+1];\n"
	"		uIndex3 = indexes[3*ind+2];\n"
	"\n"
	"		if (uIndex1.x != uIndex1.y && uIndex1.y != uIndex2.x && uIndex1.x != uIndex2.x)\n"
	"		{\n"
	"\n"
	"			f3V[0] = vload3(uIndex1.x * stride, vertices);\n"
	"			f3V[1] = vload3(uIndex1.y * stride, vertices);\n"
	"			f3V[2] = vload3(uIndex2.x * stride, vertices);\n"
	"\n"
	"			min1 = fmin(fmin(min1, f3V[0]), fmin(f3V[1], f3V[2]));\n"
	"			max1 = fmax(fmax(max1, f3V[0]), fmax(f3V[1], f3V[2]));\n"
	"		}\n"
	"		if (uIndex2.y != uIndex3.x && uIndex3.x != uIndex3.y && uIndex2.y != uIndex3.y)\n"
	"		{\n"
	"\n"
	"			f3V[0] = vload3(uIndex2.y * stride, vertices);\n"
	"			f3V[1] = vload3(uIndex3.x * stride, vertices);\n"
	"			f3V[2] = vload3(uIndex3.y * stride, vertices);\n"
	"\n"
	"			min2 = fmin(fmin(min2, f3V[0]), fmin(f3V[1], f3V[2]));\n"
	"			max2 = fmax(fmax(max2, f3V[0]), fmax(f3V[1], f3V[2]));\n"
	"		}\n"
	"	}\n"
	"\n"
	"	vstore3(fmin(min1, min2), gid, mins);\n"
	"	vstore3(fmax(max1, max2), gid, maxes);\n"
	"}"
	"\n"
	"\n"
	"__kernel  __attribute__((reqd_work_group_size(1,1,1)))\n"
	"void BoundingBoxCollect(__global float* mins, __global float* maxes, __global float3* resultmin, __global float3* resultmax)\n"
	"{\n"
	"	unsigned int i;\n"
	"	float3 lmin1 = vload3(0, mins);\n"
	"	float3 lmax1 = vload3(0, maxes);\n"
	"	float3 lmin2 = vload3(1, mins);\n"
	"	float3 lmax2 = vload3(1, maxes);\n"
	"\n"
	"	for (i = 2; i < TEST_BBOX_GLOBAL_SIZE; i+=2)\n"
	"	{\n"
	"		lmin1 = fmin(lmin1, vload3(i, mins));\n"
	"		lmax1 = fmax(lmax1, vload3(i, maxes));\n"
	"		lmin2 = fmin(lmin2, vload3(i+1, mins));\n"
	"		lmax2 = fmax(lmax2, vload3(i+1, maxes));\n"
	"	}\n"
	"	resultmin[0] = fmin(lmin1, lmin2);\n"
	"	resultmax[0] = fmax(lmax1, lmax2);\n"
	"}"
};

/***********************************************************************************
 Function Name      : Init_BBoxKernel
 Inputs             : None
 Outputs            : None
 Returns            : None
 Description        : Initialises input data
************************************************************************************/
cl_int Init_BBoxKernel(OCLTestInstance *psInstance)
{
	size_t auLengths[2] = {0,0};
	cl_int eResult;
	const char* programSourcePtrs[1] = {g_pszBBoxKernelSource};
	BBoxKernelData *psData = (BBoxKernelData*)calloc(1, sizeof(BBoxKernelData));
	if (!psData) return CL_OUT_OF_RESOURCES;

	/* Initialise data */
	psInstance->pvPrivateData = (void*)psData;

	/* Allocate host buffers */
	for (int i = 0; i < TEST_BBOX_OBJECT_COUNT; i++)
	{
		psData->pui32IndexesBuffer[i] = (unsigned short*)malloc(sizeof(unsigned short)*TEST_BBOX_INPUT_SIZE);

		if (!psData->pui32IndexesBuffer[i])
		{
			free(psData);
			return CL_OUT_OF_RESOURCES;
		}

		/* Initialise with random test values for indexes per object. */
		for (int j=0; j < TEST_BBOX_INPUT_SIZE; j++)
		{
			psData->pui32IndexesBuffer[i][j] = random() % TEST_BBOX_INPUT_SIZE;
		}
		psData->pf32VerticesBuffer[i] = (float*)malloc(TEST_BBOX_BUFFER_SIZE*sizeof(float));

		if (!psData->pf32VerticesBuffer[i])
		{
			free(psData);
			return CL_OUT_OF_RESOURCES;
		}

		/* Initialise with random test values for vertices per object. Should those be shared? */
		for (int j=0; j < TEST_BBOX_BUFFER_SIZE; j++)
		{
			psData->pf32VerticesBuffer[i][j] = (float)random() / (float)random();
		}
	}
	eResult = clGetPlatformIDs(1, &psData->psPlatformID, NULL);
	CheckAndReportError(psInstance, "clGetPlatformIDs", eResult, init_bboxkernel_cleanup);

	eResult = clGetDeviceIDs(psData->psPlatformID, CL_DEVICE_TYPE_GPU, 1, &psData->psDeviceID, NULL);
	CheckAndReportError(psInstance, "clGetDeviceIDs", eResult, init_bboxkernel_cleanup);

	psData->psContext = clCreateContext(NULL, 1, &psData->psDeviceID, NULL, NULL, &eResult);
	CheckAndReportError(psInstance, "clCreateContext", eResult, init_bboxkernel_cleanup);

	psData->psCommandQueue = clCreateCommandQueue(psData->psContext, psData->psDeviceID, CL_QUEUE_PROFILING_ENABLE|CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE, &eResult);
	CheckAndReportError(psInstance, "clCreateCommandQueue", eResult, init_bboxkernel_cleanup);

	/* Create separate buffers for each object. The Vertices buffer could be shared? */
	for (int i = 0; i < TEST_BBOX_OBJECT_COUNT; i++)
	{
		psData->psInputIndexBuffer[i] = clCreateBuffer(psData->psContext, CL_MEM_COPY_HOST_PTR|CL_MEM_READ_ONLY, sizeof(cl_ushort)*TEST_BBOX_INPUT_SIZE, psData->pui32IndexesBuffer[i], &eResult);
		CheckAndReportError(psInstance, "clCreateBuffer", eResult, init_bboxkernel_cleanup);

		psData->psInputDataBuffer[i] = clCreateBuffer(psData->psContext, CL_MEM_COPY_HOST_PTR|CL_MEM_READ_ONLY, sizeof(cl_float)*3*TEST_BBOX_INPUT_SIZE, psData->pf32VerticesBuffer[i], &eResult);
		CheckAndReportError(psInstance, "clCreateBuffer", eResult, init_bboxkernel_cleanup);

		psData->psOutputMinBuffer[i] = clCreateBuffer(psData->psContext, 0, TEST_BBOX_GLOBAL_WORK_SIZE*sizeof(cl_float)*3, NULL, &eResult);
		CheckAndReportError(psInstance, "clCreateBuffer", eResult, init_bboxkernel_cleanup);

		psData->psOutputMaxBuffer[i] = clCreateBuffer(psData->psContext, 0, TEST_BBOX_GLOBAL_WORK_SIZE*sizeof(cl_float)*3, NULL, &eResult);
		CheckAndReportError(psInstance, "clCreateBuffer", eResult, init_bboxkernel_cleanup);

		psData->psOutputGlobalMinBuffer[i] = clCreateBuffer(psData->psContext, 0, sizeof(cl_float)*3, NULL, &eResult);
		CheckAndReportError(psInstance, "clCreateBuffer", eResult, init_bboxkernel_cleanup);

		psData->psOutputGlobalMaxBuffer[i] = clCreateBuffer(psData->psContext, 0, sizeof(cl_float)*3, NULL, &eResult);
		CheckAndReportError(psInstance, "clCreateBuffer", eResult, init_bboxkernel_cleanup);

	}

	/* Use the global memwrite program */
	auLengths[0]  = strlen(g_pszBBoxKernelSource);

	/* Build the Program */
	psData->psProgram = clCreateProgramWithSource(psData->psContext, 1, programSourcePtrs, auLengths, &eResult);
	CheckAndReportError(psInstance, "clCreateProgramWithSource", eResult, init_bboxkernel_cleanup);

	eResult = clBuildProgram(psData->psProgram,1,&psData->psDeviceID,"-cl-fast-relaxed-math -cl-mad-enable",NULL,NULL);
	CheckAndReportError(psInstance, "clBuildProgram", eResult, init_bboxkernel_cleanup);

	/* Setup kernel arguments for each object. Compute minimums and maximums on host as well for later verification. */
	psData->psKernel = clCreateKernel(psData->psProgram, "BoundingBox", &eResult);
	CheckAndReportError(psInstance, "clCreateKernel", eResult, init_bboxkernel_cleanup);

	psData->psKernelCollect = clCreateKernel(psData->psProgram, "BoundingBoxCollect", &eResult);
	CheckAndReportError(psInstance, "clCreateKernel", eResult, init_bboxkernel_cleanup);
	for (int i = 0; i < TEST_BBOX_OBJECT_COUNT; i++)
	{
		/* Compute minimums and maximum on host to compare with result data from the device. */
		float minx=FLT_MAX;
		float miny=FLT_MAX;
		float minz=FLT_MAX;
		float maxx=FLT_MIN;
		float maxy=FLT_MIN;
		float maxz=FLT_MIN;
		for (int j=0; j < TEST_BBOX_INPUT_SIZE; j+=3)
		{
			unsigned int index[3];
			for (int k = 0; k < 3; k++)
			{
				index[k] = psData->pui32IndexesBuffer[i][j+k];
			}
			if (index[0] == index[1] || index[1] == index[2] || index[0] == index[2]) {
				continue;
			}
			for (int k = 0; k < 3; k++)
			{
				minx = (minx < psData->pf32VerticesBuffer[i][3*index[k]]) ? minx : psData->pf32VerticesBuffer[i][3*index[k]];
				miny = (miny < psData->pf32VerticesBuffer[i][3*index[k]+1]) ? miny : psData->pf32VerticesBuffer[i][3*index[k]+1];
				minz = (minz < psData->pf32VerticesBuffer[i][3*index[k]+2]) ? minz : psData->pf32VerticesBuffer[i][3*index[k]+2];
				maxx = (maxx > psData->pf32VerticesBuffer[i][3*index[k]]) ? maxx : psData->pf32VerticesBuffer[i][3*index[k]];
				maxy = (maxy > psData->pf32VerticesBuffer[i][3*index[k]+1]) ? maxy : psData->pf32VerticesBuffer[i][3*index[k]+1];
				maxz = (maxz > psData->pf32VerticesBuffer[i][3*index[k]+2]) ? maxz : psData->pf32VerticesBuffer[i][3*index[k]+2];
			}
		}
		psData->psKnownMins[i][0] = minx;
		psData->psKnownMins[i][1] = miny;
		psData->psKnownMins[i][2] = minz;
		psData->psKnownMaxes[i][0] = maxx;
		psData->psKnownMaxes[i][1] = maxy;
		psData->psKnownMaxes[i][2] = maxz;

		/* Free the initial host buffer as no longer required, has been copied into device */
		if (psData->pui32IndexesBuffer[i])
		{
			free(psData->pui32IndexesBuffer[i]);
			psData->pui32IndexesBuffer[i] = NULL;
		}
		if (psData->pf32VerticesBuffer[i])
		{
			free(psData->pf32VerticesBuffer[i]);
			psData->pf32VerticesBuffer[i] = NULL;
		}
	}
	return CL_SUCCESS;

init_bboxkernel_cleanup:
	return eResult;
}

/***********************************************************************************
 Function Name      : Verify_BBoxKernel
 Inputs             : None
 Outputs            : None
 Returns            : None
 Description        : Verifies output data and computes time consumed
************************************************************************************/
cl_int Verify_BBoxKernel(OCLTestInstance *psInstance)
{
	double fTimeInMiliSeconds;
	cl_int eResult = CL_SUCCESS;
	cl_ulong lStarts[TEST_BBOX_OBJECT_COUNT];
	cl_ulong lEnds[TEST_BBOX_OBJECT_COUNT];
	cl_ulong lMinStart;
	cl_ulong lMaxEnd;
	cl_event psEventList[TEST_BBOX_OBJECT_COUNT] = {0};

	BBoxKernelData *psData =(BBoxKernelData*) psInstance->pvPrivateData;
	psData = psData;
	psData->puGlobalWorkSize[0] = TEST_BBOX_GLOBAL_WORK_SIZE;
	psData->puLocalWorkSize[0]  = TEST_BBOX_REQUIRED_WORKGROUP_SIZE;

	const size_t global_size = psData->puGlobalWorkSize[0];
	const size_t local_size = psData->puLocalWorkSize[0];
	/* Enqueue one kernel for each of the objects. Kernels are independent of each other. */
	for (int i = 0; i < TEST_BBOX_OBJECT_COUNT; i++)
	{
		/* Setup the Arguments */
		eResult = clSetKernelArg(psData->psKernel, 0, sizeof(cl_mem), &psData->psInputIndexBuffer[i]);
		CheckAndReportError(psInstance, "clSetKernelArg", eResult, verify_bboxkernel_cleanup);

		eResult = clSetKernelArg(psData->psKernel, 1, sizeof(cl_mem), &psData->psInputDataBuffer[i]);
		CheckAndReportError(psInstance, "clSetKernelArg", eResult, verify_bboxkernel_cleanup);

		eResult = clSetKernelArg(psData->psKernel, 2, sizeof(cl_mem), &psData->psOutputMinBuffer[i]);
		CheckAndReportError(psInstance, "clSetKernelArg", eResult, verify_bboxkernel_cleanup);

		eResult = clSetKernelArg(psData->psKernel, 3, sizeof(cl_mem), &psData->psOutputMaxBuffer[i]);
		CheckAndReportError(psInstance, "clSetKernelArg", eResult, verify_bboxkernel_cleanup);
		{
			/* Pass size of the input as a number of triangles. */
			cl_uint ui32NumerOfTriangles = TEST_BBOX_INPUT_SIZE/3;
			eResult = clSetKernelArg(psData->psKernel, 4, sizeof(cl_uint), &ui32NumerOfTriangles);
			CheckAndReportError(psInstance, "clSetKernelArg", eResult, verify_bboxkernel_cleanup);
		}
		{
			/* Pass stride. */
			cl_uint ui32Stride = 1;
			eResult = clSetKernelArg(psData->psKernel, 5, sizeof(cl_uint), &ui32Stride);
			CheckAndReportError(psInstance, "clSetKernelArg", eResult, verify_bboxkernel_cleanup);
		}

		/* Setup the size of the kernel run */
		eResult = clEnqueueNDRangeKernel(psData->psCommandQueue, psData->psKernel, 1, NULL, &global_size, &local_size, 0, NULL, &psData->psEvent[i]);
		CheckAndReportError(psInstance, "clEnqueueNDRangeKernel", eResult, verify_bboxkernel_cleanup);
	}

	/* Enqueue one kernel for each object that processes output of the previous kernel execution. Workspace size must be 1. */
	const size_t collect_size = 1;
	/* Each kernel waits on event produced by it's corresponding computing kernel above. */
	for (int i = 0; i < TEST_BBOX_OBJECT_COUNT; i++)
	{
		/* Setup the Arguments */
		eResult = clSetKernelArg(psData->psKernelCollect, 0, sizeof(cl_mem), &psData->psOutputMinBuffer[i]);
		CheckAndReportError(psInstance, "clSetKernelArg", eResult, verify_bboxkernel_cleanup);

		eResult = clSetKernelArg(psData->psKernelCollect, 1, sizeof(cl_mem), &psData->psOutputMaxBuffer[i]);
		CheckAndReportError(psInstance, "clSetKernelArg", eResult, verify_bboxkernel_cleanup);

		eResult = clSetKernelArg(psData->psKernelCollect, 2, sizeof(cl_mem), &psData->psOutputGlobalMinBuffer[i]);
		CheckAndReportError(psInstance, "clSetKernelArg", eResult, verify_bboxkernel_cleanup);

		eResult = clSetKernelArg(psData->psKernelCollect, 3, sizeof(cl_mem), &psData->psOutputGlobalMaxBuffer[i]);
		CheckAndReportError(psInstance, "clSetKernelArg", eResult, verify_bboxkernel_cleanup);

		psEventList[i] = psData->psEvent[i];
		eResult = clEnqueueNDRangeKernel(psData->psCommandQueue, psData->psKernelCollect, 1, NULL, &collect_size, &collect_size, 1, &psEventList[i], &psData->psEventCollect[i]);
		CheckAndReportError(psInstance, "clEnqueueNDRangeKernel", eResult, verify_bboxkernel_cleanup);
	}

	clFinish(psData->psCommandQueue);

	/* Print out timing information (command queue had profiling enabled) */
	for (int i = 0; i < TEST_BBOX_OBJECT_COUNT; i++)
	{
		eResult = clGetEventProfilingInfo(psData->psEvent[i],CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &lStarts[i], NULL);
		CheckAndReportError(psInstance, "clGetEventProfilingInfo", eResult, verify_bboxkernel_cleanup);

		eResult = clGetEventProfilingInfo(psData->psEventCollect[i], CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &lEnds[i], NULL);
		CheckAndReportError(psInstance, "clGetEventProfilingInfo", eResult, verify_bboxkernel_cleanup);
	}
	/* Find earliest start and latest end of all the enqueued kernels. */
	lMinStart = lStarts[0];
	lMaxEnd = lEnds[0];

	for (int i = 1; i < TEST_BBOX_OBJECT_COUNT; i++)
	{
		lMinStart = (lMinStart < lStarts[i]) ? lMinStart : lStarts[i];
		lMaxEnd = (lMaxEnd > lEnds[i]) ? lMaxEnd : lEnds[i];
	}
	fTimeInMiliSeconds = ((double)(lMaxEnd-lMinStart)) / 1000000.0;

	OCLTestLog("  Objects: %8d, # triangles per object: %8d, Computing instances per object: %4zu, Time - start - stop: %10fms\n",
		TEST_BBOX_OBJECT_COUNT,
		TEST_BBOX_INPUT_SIZE/3,
		psData->puGlobalWorkSize[0],
		fTimeInMiliSeconds);

	OCLMetricOutputDouble(__func__, "speed", fTimeInMiliSeconds, MS);

	for (int i = 0; i < TEST_BBOX_OBJECT_COUNT; i++)
	{
		cl_float pf32GlobalMinResults[3];
		cl_float pf32GlobalMaxResults[3];

		/* Read back the results */
		eResult = clEnqueueReadBuffer(psData->psCommandQueue, psData->psOutputGlobalMinBuffer[i], CL_FALSE, 0, sizeof(cl_float)*3, pf32GlobalMinResults, 0, NULL, NULL);
		CheckAndReportError(psInstance, "clEnqueueReadBuffer", eResult, verify_bboxkernel_cleanup);
		eResult = clEnqueueReadBuffer(psData->psCommandQueue, psData->psOutputGlobalMaxBuffer[i], CL_FALSE, 0, sizeof(cl_float)*3, pf32GlobalMaxResults, 0, NULL, NULL);
		CheckAndReportError(psInstance, "clEnqueueReadBuffer", eResult, verify_bboxkernel_cleanup);

		clFinish(psData->psCommandQueue);
#if !defined(NO_HARDWARE)
		if (pf32GlobalMinResults[0] != psData->psKnownMins[i][0] ||
			pf32GlobalMinResults[1] != psData->psKnownMins[i][1] ||
			pf32GlobalMinResults[2] != psData->psKnownMins[i][2] ||
			pf32GlobalMaxResults[0] != psData->psKnownMaxes[i][0] ||
			pf32GlobalMaxResults[1] != psData->psKnownMaxes[i][1] ||
			pf32GlobalMaxResults[2] != psData->psKnownMaxes[i][2])
		{
			OCLTestLog("Verification error2: %s: Object instance %d, Input size %d vertices, Found mins (%f, %f, %f) <> expected mins(%f, %f, %f), "\
			"Found max(%f, %f, %f) <>  expected max(%f, %f, %f).\n",
						__func__, i, TEST_BBOX_INPUT_SIZE,
			  pf32GlobalMinResults[0], pf32GlobalMinResults[1], pf32GlobalMinResults[2],
			  psData->psKnownMins[i][0],psData->psKnownMins[i][1], psData->psKnownMins[i][2],
			  pf32GlobalMaxResults[0], pf32GlobalMaxResults[1], pf32GlobalMaxResults[2],
			  psData->psKnownMaxes[i][0], psData->psKnownMaxes[i][1], psData->psKnownMaxes[i][2]);
		}
#endif
	}

verify_bboxkernel_cleanup:
	for (int i = 0; i < TEST_BBOX_OBJECT_COUNT; i++)
	{
		if (psData->psInputDataBuffer[i]) clReleaseMemObject(psData->psInputDataBuffer[i]);
		if (psData->psInputIndexBuffer[i]) clReleaseMemObject(psData->psInputIndexBuffer[i]);
		if (psData->psOutputMaxBuffer[i]) clReleaseMemObject(psData->psOutputMaxBuffer[i]);
		if (psData->psOutputMinBuffer[i]) clReleaseMemObject(psData->psOutputMinBuffer[i]);
		if (psData->psOutputGlobalMaxBuffer[i]) clReleaseMemObject(psData->psOutputGlobalMaxBuffer[i]);
		if (psData->psOutputGlobalMinBuffer[i]) clReleaseMemObject(psData->psOutputGlobalMinBuffer[i]);
		if (psData->psEvent[i]) clReleaseEvent(psData->psEvent[i]);
		if (psData->psEventCollect[i]) clReleaseEvent(psData->psEventCollect[i]);
	}

	if (psData->psKernel) clReleaseKernel(psData->psKernel);
	if (psData->psKernelCollect) clReleaseKernel(psData->psKernelCollect);
	if (psData->psProgram) clReleaseProgram(psData->psProgram);
	if (psData->psCommandQueue)	clReleaseCommandQueue(psData->psCommandQueue);
	if (psData->psContext) clReleaseContext(psData->psContext);

	free(psInstance->pvPrivateData);
	psInstance->pvPrivateData = strdup("Results buffer allocation failure");
	return eResult;
}

/******************************************************************************
 End of file (bbox.c)
******************************************************************************/
