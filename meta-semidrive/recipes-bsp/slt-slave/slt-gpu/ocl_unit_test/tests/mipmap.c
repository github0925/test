/*************************************************************************/ /*!
@File           mipmap.c
@Copyright      Copyright (c) Imagination Technologies Ltd. All Rights Reserved
@License        Strictly Confidential.
*/ /**************************************************************************/

#define TEST_MIPMAP_STEP_SIZE      1
#define TEST_MIPMAP_MAX_ITERATIONS 16
#define TEST_MIPMAP_IMG_WIDTH      (512/4)
#define TEST_MIPMAP_IMG_HEIGHT     512
#define TEST_MIPMAP_MIP_LEVEL      10

typedef struct _MipMapData_
{
	/* CL Objects */
	cl_context       psContext;
	cl_device_id     psDeviceID;
	cl_platform_id   psPlatformID;
	cl_command_queue psCommandQueue;
	cl_mem           psImage;

	/* Test Dimensions */
	cl_uint ui32ImageWidth;
	cl_uint ui32ImageHeight;

	/* Host Object */
	size_t        puGlobalWorkSize4[3];

	size_t*        puLocalWorkSize;

} MipMapData;

/***********************************************************************************
 Function Name      : Init_MipMap
 Inputs             : None
 Outputs            : None
 Returns            : None
 Description        : Initialises input data
************************************************************************************/
static cl_int
Init_MipMap(OCLTestInstance *psInstance)
{
	cl_int           eResult;
	cl_image_format  sFormat;
	cl_image_desc    sDesc;
	MipMapData* psData = calloc(1, sizeof(MipMapData));
	if (!psData)
	{
		return CL_OUT_OF_RESOURCES;
	}

	/* Initialise data */
	psInstance->pvPrivateData = (void*)psData;

	/* Initialise host side objects */
	psData->ui32ImageWidth  = TEST_MIPMAP_IMG_WIDTH;
	psData->ui32ImageHeight = TEST_MIPMAP_IMG_HEIGHT;

	eResult = clGetPlatformIDs(1, &psData->psPlatformID, NULL);
	CheckAndReportError(psInstance, "clGetPlatformIDs", eResult, init_mipmap_cleanup);

	eResult = clGetDeviceIDs(psData->psPlatformID,
							 CL_DEVICE_TYPE_GPU,
							 1,
							 &psData->psDeviceID,
							 NULL);
	CheckAndReportError(psInstance, "clGetDeviceIDs", eResult, init_mipmap_cleanup);

	psData->psContext = clCreateContext(NULL, 1, &psData->psDeviceID, NULL, NULL, &eResult);
	CheckAndReportError(psInstance, "clCreateContext", eResult, init_mipmap_cleanup);

	psData->psCommandQueue = clCreateCommandQueue(psData->psContext,
												  psData->psDeviceID,
												  CL_QUEUE_PROFILING_ENABLE,
												  &eResult);
	CheckAndReportError(psInstance, "clCreateCommandQueue", eResult, init_mipmap_cleanup);

	/* Setup the 4-channel image arguments for the kernel */
	sFormat.image_channel_order     = CL_RGBA;
	sFormat.image_channel_data_type = CL_UNORM_INT8;

	/* Setup the image description */
	sDesc.image_type = CL_MEM_OBJECT_IMAGE2D;
	sDesc.image_width = psData->ui32ImageWidth;
	sDesc.image_height = psData->ui32ImageHeight;
	sDesc.image_depth = 0;
	sDesc.image_array_size = 0;
	sDesc.image_row_pitch = 0;
	sDesc.image_slice_pitch = 0;
	sDesc.num_mip_levels = TEST_MIPMAP_MIP_LEVEL;
	sDesc.num_samples = 0;
	sDesc.buffer = NULL;

	psData->psImage = clCreateImage(psData->psContext,
									  CL_MEM_READ_ONLY,
									  &sFormat,
									  &sDesc,
									  NULL,
									  &eResult);
	CheckAndReportError(psInstance, "clCreateImage", eResult, init_mipmap_cleanup);

	/* Allow driver to set local size */
	psData->puLocalWorkSize  = NULL;

	return CL_SUCCESS;

init_mipmap_cleanup:
	return eResult;
}

static cl_int
RunMipMap(OCLTestInstance* psInstance,
		  cl_mem           psImg,
		  cl_uint          uWidth,
		  cl_uint          uHeight,
		  double*          pfAverageSpeed)
{
	//double fTimeInSeconds;
	//double fMBPerSeconds;

	MipMapData* psData = psInstance->pvPrivateData;
	unsigned t, i, j;
	//unsigned ui32MBCopied;
	size_t origin[]  = {0, 0, 0}; // 2D image, miplevel 2
	size_t origin3[] = {0, 0, 3}; // 2D image, miplevel 3
	size_t region[]  = {uWidth, uHeight, 1};
	size_t region3[] = {uWidth/8, uHeight/8, 1};
	//size_t auGlobalSize[] = {uWidth, uHeight, 1};
	cl_int eResult;
	unsigned long lStart, lEnd;
	unsigned char* pui32Results;
	unsigned char* pui32Results3;
	unsigned char* pui32ResultsReturn;
	unsigned char* pui32ResultsReturn3;
	/* Unused */
	pfAverageSpeed = pfAverageSpeed;

	pui32Results = malloc(sizeof(unsigned char) * uWidth * uHeight * 4 );
	if (!pui32Results)
	{
		free(psInstance->pvPrivateData);
		psInstance->pvPrivateData = strdup("Results buffer allocation failure");
		return CL_OUT_OF_HOST_MEMORY;
	}

	pui32Results3 = malloc((sizeof(unsigned char) * uWidth * uHeight * 4) / 8 );
	if (!pui32Results3)
	{
		free(psInstance->pvPrivateData);
		psInstance->pvPrivateData = strdup("Results buffer allocation failure");
		return CL_OUT_OF_HOST_MEMORY;
	}

	pui32ResultsReturn = malloc(sizeof(unsigned char) * uWidth * uHeight * 4 );
	if (!pui32ResultsReturn)
	{
		free(psInstance->pvPrivateData);
		psInstance->pvPrivateData = strdup("Results buffer allocation failure");
		return CL_OUT_OF_HOST_MEMORY;
	}

	pui32ResultsReturn3 = malloc((sizeof(unsigned char) * uWidth * uHeight * 4) / 8);
	if (!pui32ResultsReturn3)
	{
		free(psInstance->pvPrivateData);
		psInstance->pvPrivateData = strdup("Results buffer allocation failure");
		return CL_OUT_OF_HOST_MEMORY;
	}

	//for(t=0; t < TEST_MIPMAP_MAX_ITERATIONS; t++)
	t = 15;
	{
		unsigned long lTimeTaken = 0;
		unsigned int ui32NumIterations = (t + 1) * TEST_MIPMAP_STEP_SIZE;

		/* Enqueue a kernel to copy data from B --> A */
		for (i = 0; i < ui32NumIterations; i++)
		{
			/* Ensure new data is copied each time (though not included in timing) */
			for (j = 0; j < uWidth * uHeight * 4; j++)
			{
				/* RGBA order */
				/* Purple Colour */
				pui32Results[j++] = 114;
				pui32Results[j++] = 22;
				pui32Results[j++] = 107;
				pui32Results[j] = 255;
			}

			for (j = 0; j < (uWidth/8) * (uHeight/8) * 4; j++)
			{
				/* RGBA order */
				/* Pink Colour */
				pui32Results3[j++] = 183;
				pui32Results3[j++] = 26;
				pui32Results3[j++] = 139;
				pui32Results3[j] = 255;
			}

			lStart = OCLGetTime();

			eResult = clEnqueueWriteImage(psData->psCommandQueue,
			                              psImg,
			               /* blocking */ CL_TRUE,
			                              origin,
										  region,
			        /* row/slice pitch */ 0, 0,
			               /* Data ptr */ pui32Results,
			             /* Event data */ 0, NULL, NULL);
			CheckAndReportError(psInstance,"clEnqueueWriteImage", eResult, run_mipmap_cleanup);

			eResult = clEnqueueWriteImage(psData->psCommandQueue,
			                              psImg,
			               /* blocking */ CL_TRUE,
			                              origin3,
			                              region3,
			        /* row/slice pitch */ 0, 0,
			               /* Data ptr */ pui32Results3,
			             /* Event data */ 0, NULL, NULL);
			CheckAndReportError(psInstance,"clEnqueueWriteImage", eResult, run_mipmap_cleanup);

			clFinish(psData->psCommandQueue);

			lEnd = OCLGetTime();

			lTimeTaken += (lEnd-lStart);
		}

		memset(pui32ResultsReturn,  0, sizeof(unsigned char) *  uWidth   *   uHeight    * 4);
		memset(pui32ResultsReturn3, 0, sizeof(unsigned char) * (uWidth/8) * (uHeight/8) * 4);

		/* Read back the results */
		eResult = clEnqueueReadImage(psData->psCommandQueue,
									 psImg,
									 CL_TRUE,
									 origin,
									 region,
									 0,
									 0,
									 pui32ResultsReturn,
									 0,
									 NULL,
									 NULL);
		CheckAndReportError(psInstance, "clEnqueueReadImage", eResult, run_mipmap_cleanup);

		eResult = clEnqueueReadImage(psData->psCommandQueue,
									 psImg,
									 CL_TRUE,
									 origin3,
									 region3,
									 0,
									 0,
									 pui32ResultsReturn3,
									 0,
									 NULL,
									 NULL);
		CheckAndReportError(psInstance, "clEnqueueReadImage", eResult, run_mipmap_cleanup);

		/* Verify the written back data with what we would expect */
		for (i=0; i < uWidth * uHeight * 4; i++)
		{
			if (pui32Results[i] != pui32ResultsReturn[i])
			{
				OCLTestLog("%s: Verification failure at %d (mipmap level 0), expected 0x%08x got 0x%08x.\n",
				           __func__, i, pui32Results[i],  pui32ResultsReturn[i]);

				/* Verification Error */
				free(psInstance->pvPrivateData);
				psInstance->pvPrivateData = strdup("Verification failure");
				return CL_OUT_OF_RESOURCES;
			}
		}
		for (i=0; i < (uWidth/8) * (uHeight/8) * 4 / 8; i++)
		{
			if (pui32Results3[i] != pui32ResultsReturn3[i])
			{
				OCLTestLog("%s: Verification failure at %d (mipmap level 3), expected 0x%08x got 0x%08x.\n",
				           __func__, i, pui32Results3[i],  pui32ResultsReturn3[i]);

				/* Verification Error */
				free(psInstance->pvPrivateData);
				psInstance->pvPrivateData = strdup("Verification failure");
			    return CL_OUT_OF_RESOURCES;
			}
		}
	} /* for(t=0; t < TEST_MIPMAP_MAX_ITERATIONS; t++) */

	free(pui32Results);
	free(pui32Results3);
	free(pui32ResultsReturn);
	free(pui32ResultsReturn3);

run_mipmap_cleanup:
	return eResult;
}

/***********************************************************************************
 Function Name      : Verify_MipMap
 Inputs             : None
 Outputs            : None
 Returns            : None
 Description        : Verifies output data
************************************************************************************/
static cl_int
Verify_MipMap(OCLTestInstance *psInstance)
{
	double fAverageSpeed = 0.0;

	cl_int eResult = CL_SUCCESS;

	MipMapData* psData = psInstance->pvPrivateData;

	eResult = RunMipMap(psInstance,
						psData->psImage,
						psData->ui32ImageWidth,
						psData->ui32ImageHeight,
						&fAverageSpeed);
	CheckAndReportError(psInstance, "RunMipMap", eResult, verify_mipmap_cleanup);

	/* Compute average */
	fAverageSpeed = fAverageSpeed / (double)TEST_MIPMAP_MAX_ITERATIONS;

	OCLTestLog("%s: Average speed %.2f MB/s\n", __func__, fAverageSpeed);

	/* Cleanup */
	clReleaseMemObject   (psData->psImage);
	clReleaseCommandQueue(psData->psCommandQueue);
	clReleaseContext     (psData->psContext);

verify_mipmap_cleanup:
	return eResult;
}

/******************************************************************************
 End of file (mipmap.c)
******************************************************************************/
