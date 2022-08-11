/*************************************************************************/ /*!
@File           imgcpy.c
@Copyright      Copyright (c) Imagination Technologies Ltd. All Rights Reserved
@License        Strictly Confidential.
*/ /**************************************************************************/

/* Defines the maximum amount of data copied in the final instance */
#ifdef CUT_DOWN_UNIT_TEST
#define TEST_IMGCPY_MAX_TRANSFER_SIZE_KB  (512)
#else
#define TEST_IMGCPY_MAX_TRANSFER_SIZE_KB  (256*1024)
#endif

/* Defines the starting transfer size, increase if the test complains*/
/* Note that each step is n=n*2 i.e log2 */
#ifdef CUT_DOWN_UNIT_TEST
#define TEST_IMGCPY_START_TRANSFER_SIZE_KB  (128)
#else
#define TEST_IMGCPY_START_TRANSFER_SIZE_KB  (2028)
#endif

/* Function macro to convert KB to Bytes */
#define TEST_IMGCPY_KB_TO_BYTES(X) (X*1024)

/* The width and height of the two different images */
#define TEST_IMGCPY_IMG_WIDTH_RGBA (64)
#define TEST_IMGCPY_IMG_HEIGHT_RGBA (256)

#define TEST_IMGCPY_IMG_WIDTH_R (TEST_IMGCPY_IMG_WIDTH_RGBA * 4)
#define TEST_IMGCPY_IMG_HEIGHT_R (TEST_IMGCPY_IMG_HEIGHT_RGBA)

typedef struct _ImgCopyKernelData_
{
	/* CL Objects */
	cl_context       psContext;
	cl_program       psProgram;
	cl_device_id     psDeviceID;
	cl_platform_id   psPlatformID;
	cl_command_queue psCommandQueue;
	cl_kernel        psKernelRGBA;
	cl_kernel        psKernelR;
	cl_mem           psImageARGBA;
	cl_mem           psImageBRGBA;
	cl_mem           psImageAR;
	cl_mem           psImageBR;

	/* Test Dimensions */
	cl_uint ui32ImageWidthRGBA;
	cl_uint ui32ImageHeightRGBA;
	cl_uint ui32ImageWidthR;
	cl_uint ui32ImageHeightR;

	/* Performance Counters*/
	double pfdMBCopied;
	unsigned long int pfulTicks;

	size_t*        puLocalWorkSize;

} ImgCopyKernelData;

static char *g_pszImgCopyKernelSource =
{
	"__kernel void ImageCopy(__read_only image2d_t srcImage,\n"
	"                        __write_only image2d_t dstImage)\n"
	"{\n"
	"	const sampler_t sSampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_NONE | CLK_FILTER_NEAREST;\n"
	"	int2 coords = (int2)(get_global_id(0), get_global_id(1));\n"
	"	float4 colour = read_imagef(srcImage, sSampler, coords);\n"
	"	write_imagef(dstImage, coords, colour);\n"
	"}\n"
};


/***********************************************************************************
 Function Name      : Init_ImgCopyKernel
 Inputs             : None
 Outputs            : None
 Returns            : None
 Description        : Initialises input data
************************************************************************************/
static cl_int
Init_ImgCopyKernel(OCLTestInstance *psInstance)
{
	cl_int           eResult;
	cl_image_format  sFormat;
	cl_image_desc    sDesc;
	size_t auLengths[2] = {0,0};
	char *ppszSource[2] = { 0 };

	ImgCopyKernelData *psData = calloc(1, sizeof(ImgCopyKernelData));

#if !defined(NO_HARDWARE)
	OCLTestLog("%s: Hardware detected, verification will be run.\n",__func__);
#endif


	if(!psData)
	{
		return CL_OUT_OF_RESOURCES;
	}

	/* Initialise data */
	psInstance->pvPrivateData = (void*)psData;

	/* Initialise host side objects */
	psData->ui32ImageWidthRGBA  = TEST_IMGCPY_IMG_WIDTH_RGBA;
	psData->ui32ImageHeightRGBA = TEST_IMGCPY_IMG_HEIGHT_RGBA;
	psData->ui32ImageWidthR  = TEST_IMGCPY_IMG_WIDTH_R;
	psData->ui32ImageHeightR = TEST_IMGCPY_IMG_HEIGHT_R;

	/* Initialise opencl context and device*/
	eResult = clGetPlatformIDs(1,&psData->psPlatformID,NULL);
	CheckAndReportError(psInstance, "clGetPlatformIDs", eResult, init_imgcpy_cleanup);

	eResult = clGetDeviceIDs(psData->psPlatformID,CL_DEVICE_TYPE_GPU,1,&psData->psDeviceID,NULL);
	CheckAndReportError(psInstance, "clGetDeviceIDs", eResult, init_imgcpy_cleanup);

	psData->psContext = clCreateContext(NULL,1,&psData->psDeviceID,NULL,NULL,&eResult);
	CheckAndReportError(psInstance, "clCreateContext", eResult, init_imgcpy_cleanup);

	psData->psCommandQueue = clCreateCommandQueue(psData->psContext, psData->psDeviceID, CL_QUEUE_PROFILING_ENABLE, &eResult);
	CheckAndReportError(psInstance, "clCreateCommandQueue", eResult, init_imgcpy_cleanup);

	/* Section where the images are set up */
	/* Setup the RGBA-channel image arguments for the kernel */
	sFormat.image_channel_order     = CL_RGBA;
	sFormat.image_channel_data_type = CL_UNORM_INT8;

	/* Setup the image description */
	sDesc.image_type = CL_MEM_OBJECT_IMAGE2D;
	sDesc.image_width = psData->ui32ImageWidthRGBA;
	sDesc.image_height = psData->ui32ImageHeightRGBA;
	sDesc.image_depth = 0;
	sDesc.image_array_size = 0;
	sDesc.image_row_pitch = 0;
	sDesc.image_slice_pitch = 0;
	sDesc.num_mip_levels = 0;
	sDesc.num_samples = 0;
	sDesc.buffer = NULL;

	psData->psImageARGBA = clCreateImage(psData->psContext, CL_MEM_READ_ONLY, &sFormat, &sDesc, NULL, &eResult);
	CheckAndReportError(psInstance, "clCreateImage", eResult, init_imgcpy_cleanup);

	psData->psImageBRGBA = clCreateImage(psData->psContext, CL_MEM_WRITE_ONLY, &sFormat, &sDesc, NULL, &eResult);
	CheckAndReportError(psInstance, "clCreateImage", eResult, init_imgcpy_cleanup);

	/* Setup the R-channel image arguments for the kernel */
	sFormat.image_channel_order     = CL_R;
	sFormat.image_channel_data_type = CL_UNORM_INT8;

	/* Setup the R-channel image description */
	sDesc.image_width = psData->ui32ImageWidthR;
	sDesc.image_height = psData->ui32ImageHeightR;

	psData->psImageAR = clCreateImage(psData->psContext, CL_MEM_READ_ONLY, &sFormat, &sDesc, NULL, &eResult);
	CheckAndReportError(psInstance, "clCreateImage", eResult, init_imgcpy_cleanup);

	psData->psImageBR = clCreateImage(psData->psContext, CL_MEM_WRITE_ONLY, &sFormat, &sDesc, NULL, &eResult);
	CheckAndReportError(psInstance, "clCreateImage", eResult, init_imgcpy_cleanup);


	/* Section where the kernel is compiled */
	/* Use the global imgcpy program */
	auLengths[0]  = strlen(g_pszImgCopyKernelSource);
	ppszSource[0] = g_pszImgCopyKernelSource;

	psData->psProgram = clCreateProgramWithSource(psData->psContext, 1, (const char**)ppszSource, auLengths, &eResult);
	CheckAndReportError(psInstance, "clCreateProgramWithSource", eResult, init_imgcpy_cleanup);

	eResult = clBuildProgram(psData->psProgram, 1, &psData->psDeviceID, "", NULL, NULL);
	if (eResult != CL_SUCCESS)
	{
		char aszBuildLog[512]={0};
		size_t uBuildLogSize;

		eResult = clGetProgramBuildInfo(psData->psProgram, psData->psDeviceID, CL_PROGRAM_BUILD_LOG, 512, aszBuildLog, &uBuildLogSize);
		CheckAndReportError(psInstance, "clGetProgramBuildInfo", eResult, init_imgcpy_cleanup);
		aszBuildLog[uBuildLogSize-1] = '\0';

		OCLTestLog("%s:\n"
			   "*** Build Log ***\n"
			   "%s",
			   psInstance->pszTestID,
			   aszBuildLog);
	}

	CheckAndReportError(psInstance, "clBuildProgram", eResult, init_imgcpy_cleanup);

	/* Create the kernels from the program */
	psData->psKernelRGBA = clCreateKernel(psData->psProgram, "ImageCopy", &eResult);
	CheckAndReportError(psInstance, "clCreateKernel", eResult, init_imgcpy_cleanup);

	psData->psKernelR = clCreateKernel(psData->psProgram, "ImageCopy", &eResult);
	CheckAndReportError(psInstance, "clCreateKernel", eResult, init_imgcpy_cleanup);

	/* Assign the input-parameters for each kernel*/
	eResult = clSetKernelArg(psData->psKernelRGBA, 0, sizeof(cl_mem), &psData->psImageARGBA);
	CheckAndReportError(psInstance, "clSetKernelArg", eResult, init_imgcpy_cleanup);

	eResult = clSetKernelArg(psData->psKernelRGBA, 1, sizeof(cl_mem), &psData->psImageBRGBA);
	CheckAndReportError(psInstance, "clSetKernelArg", eResult, init_imgcpy_cleanup);

	eResult = clSetKernelArg(psData->psKernelR, 0, sizeof(cl_mem), &psData->psImageAR);
	CheckAndReportError(psInstance, "clSetKernelArg", eResult, init_imgcpy_cleanup);

	eResult = clSetKernelArg(psData->psKernelR, 1, sizeof(cl_mem), &psData->psImageBR);
	CheckAndReportError(psInstance, "clSetKernelArg", eResult, init_imgcpy_cleanup);


	/* Allow driver to set local size */
	psData->puLocalWorkSize = NULL;

	return CL_SUCCESS;

init_imgcpy_cleanup:
	free(psData);
	return eResult;

}


static cl_int
ImgCpy_RunImgCpy(OCLTestInstance* psInstance, cl_mem psSrcImg, cl_mem psDstImg, cl_kernel psKernel)
{

	double fTimeInSeconds;
	double fMBPerSeconds;

	ImgCopyKernelData* psData = psInstance->pvPrivateData;
	unsigned t, i, j;
	float fMBCopied;
	size_t sizeImg;
	size_t origin[3] = {0, 0, 0};
	size_t region[3];
	size_t auGlobalSize[3];
	size_t uWidth;
	size_t uHeight;
	cl_int eResult;
	unsigned char* pui8Results = NULL;

	/* Get the memory size in bytes of the image */
	eResult =  clGetMemObjectInfo(psSrcImg,CL_MEM_SIZE,sizeof(size_t),&sizeImg,NULL);
	CheckAndReportError(psInstance,"clGetMemObjectInfo", eResult, run_imgcopykernel_cleanup);

	/* Get the height and width of the image */
	eResult =  clGetImageInfo(psSrcImg,CL_IMAGE_WIDTH,sizeof(size_t),&uWidth,NULL);
	CheckAndReportError(psInstance,"clGetImageInfo", eResult, run_imgcopykernel_cleanup);

	eResult =  clGetImageInfo(psSrcImg,CL_IMAGE_HEIGHT,sizeof(size_t),&uHeight,NULL);
	CheckAndReportError(psInstance,"clGetImageInfo", eResult, run_imgcopykernel_cleanup);

	/* Allocate the host-side buffer for the image data */
	pui8Results = malloc(sizeImg);

	if (!pui8Results)
	{
		free(psInstance->pvPrivateData);
		psInstance->pvPrivateData = strdup("Results buffer allocation failure");
		return CL_OUT_OF_HOST_MEMORY;
	}

	/* Set the values received to the region and auGlobalSize arrays */
	region[0] = uWidth;
	region[1] = uHeight;
	region[2] = 1;

	auGlobalSize[0] = uWidth;
	auGlobalSize[1] = uHeight;
	auGlobalSize[2] = 1;

	/* The main loop which handles all the imgcpy operations */
	for(t=TEST_IMGCPY_START_TRANSFER_SIZE_KB; t <= TEST_IMGCPY_MAX_TRANSFER_SIZE_KB; t *=2)
	{

		const unsigned int uNumIterations = (TEST_IMGCPY_KB_TO_BYTES(t)) /sizeImg;
		unsigned long lStart, lEnd, lTimeTaken;

		/* Ensure new data is copied each time (though not included in timing) */
		for (j=0; j < sizeImg; j++)
			pui8Results[j] = j % 255;


		/* Write to the device image psSrcImg from the host */
		eResult = clEnqueueWriteImage(psData->psCommandQueue,
					      psSrcImg, /* Write to */
					      CL_TRUE, /* blocking */
					      origin, region,
					      0, 0, /* row/slice pitch */
					      pui8Results, /* Data ptr */
					      0, NULL, NULL); /* Event data */

		CheckAndReportError(psInstance,"clEnqueueWriteImage", eResult, run_imgcopykernel_cleanup);

		lTimeTaken = 0;

		eResult = clFinish(psData->psCommandQueue);
		CheckAndReportError(psInstance, "clFinish", eResult, run_imgcopykernel_cleanup);

		/* This loop will copy t MB of image data in uNumIterations iterations*/
		for(i=0;i < uNumIterations; i++)
		{
			/* Starts the timer for the below for loop*/
			lStart = OCLGetTime();

			/* The kernel which does the image copy */
			eResult = clEnqueueNDRangeKernel(psData->psCommandQueue,
							 psKernel,
							 2, NULL,
							 auGlobalSize,
							 psData->puLocalWorkSize ,
							 0, NULL, NULL);

			CheckAndReportError(psInstance, "clEnqueueNDRangeKernel", eResult, run_imgcopykernel_cleanup);

			eResult = clFinish(psData->psCommandQueue);
			CheckAndReportError(psInstance, "clFinish", eResult, run_imgcopykernel_cleanup);

			lEnd = OCLGetTime();
			lTimeTaken += lEnd - lStart;
		}

		if(0 == lTimeTaken)
		{
			OCLTestLog("%s: Warning - time taken is ZERO, It is suggested to increase the macro %s .\n",
				   __func__,"TEST_IMGCPY_START_TRANSFER_SIZE_KB");
			lTimeTaken = 1;
		}

		/* How much data was copied, can be different than t if sizeImg does not divide t evenly */
		fMBCopied = uNumIterations*sizeImg;
		fMBCopied /= 1024*1024;

		fTimeInSeconds =  ((double) lTimeTaken) * 0.001;
		fMBPerSeconds = ((double) fMBCopied) / fTimeInSeconds;

		OCLTestLog("%s: Instances: %zux%zu, Iterations: %4d,  Copied %6.2f MBs, Time %fs, %5lu Ticks, %10fMB/s\n",
			   __func__,
			   auGlobalSize[0], auGlobalSize[1],
			   uNumIterations, fMBCopied,
			   fTimeInSeconds, lTimeTaken,
			   fMBPerSeconds);

		/* Record the data for the average speed calculation */
		psData->pfdMBCopied += fMBCopied;
		psData->pfulTicks += lTimeTaken;

		/* Reset Host side memory and read back from device to see if the copy was successful */
		memset(pui8Results, 0, sizeImg);
		eResult = clEnqueueReadImage(psData->psCommandQueue,
					     psDstImg, /* Read from */
					     CL_TRUE, /* Blocking*/
					     origin, region,
					     0, 0, /* row/slice pitch*/
					     pui8Results, /* Write to */
					     0, NULL, NULL);
		CheckAndReportError(psInstance, "clEnqueueReadImage", eResult, run_imgcopykernel_cleanup);


#if !defined(NO_HARDWARE)
		/* Verify the written back data with what we would expect */
		for (i=0; i < sizeImg; i++)
		{
			if (pui8Results[i] != i % 255)
			{
				OCLTestLog("%s: Verification failure at %d, expected %08x got %08x.\n",
					   __func__, i, i % 255,  pui8Results[i]);

				/* Verification Error */
				free(psInstance->pvPrivateData);
				psInstance->pvPrivateData = strdup("Verification failure");
				return CL_OUT_OF_RESOURCES;
			}
		}
#endif

	}

	free(pui8Results);
	return eResult;

run_imgcopykernel_cleanup:

	free(pui8Results);
	return eResult;


}

/***********************************************************************************
 Function Name      : Verify_ImgCopyKernel
 Inputs             : None
 Outputs            : None
 Returns            : None
 Description        : Verifies output data
************************************************************************************/
static cl_int
Verify_ImgCopyKernel(OCLTestInstance *psInstance)
{
	double fAverageSpeed = 0.0;

	cl_int eResult = CL_SUCCESS;

	ImgCopyKernelData *psData = psInstance->pvPrivateData;

	/* Runs the RGBA kernel copy */
	eResult = ImgCpy_RunImgCpy(psInstance, psData->psImageARGBA, psData->psImageBRGBA, psData->psKernelRGBA);

	/* Exit early if the first test has alreay failed */
	CheckAndReportError(psInstance, "ImgCpy_RunImageCpy", eResult, verify_imgcopykernel_cleanup);

	/* Runs the R kernel copy */
	eResult = ImgCpy_RunImgCpy(psInstance, psData->psImageAR, psData->psImageBR,psData->psKernelR);

	CheckAndReportError(psInstance, "ImgCpy_RunImageCpy", eResult, verify_imgcopykernel_cleanup);

	/* Compute average */
	fAverageSpeed = psData->pfdMBCopied / (((double) psData->pfulTicks) * 0.001);

	OCLTestLog("%s: Average speed %.2f MB/s\n", __func__, fAverageSpeed);

	OCLMetricOutputDouble(__func__,"AvgSpeed",fAverageSpeed,MBSEC);
	/* Cleanup */
		clReleaseKernel      (psData->psKernelRGBA);
	clReleaseKernel      (psData->psKernelR);
	clReleaseMemObject   (psData->psImageARGBA);
	clReleaseMemObject   (psData->psImageBRGBA);
	clReleaseMemObject   (psData->psImageAR);
	clReleaseMemObject   (psData->psImageBR);
	clReleaseCommandQueue(psData->psCommandQueue);
	clReleaseProgram     (psData->psProgram);
	clReleaseContext     (psData->psContext);
	free(psData);
	return eResult;

verify_imgcopykernel_cleanup:
	return eResult;
}

/******************************************************************************
 End of file (imgcpy.c)
******************************************************************************/
