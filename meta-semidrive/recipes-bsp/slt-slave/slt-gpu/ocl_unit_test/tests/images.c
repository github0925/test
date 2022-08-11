/*************************************************************************/ /*!
@File           images.c
@Title          OpenCL unit test for images
@Copyright      Copyright (c) Imagination Technologies Ltd. All Rights Reserved
@Description    USC Backend common Builtin header.
@License        Strictly Confidential.
*/ /**************************************************************************/

#define IMAGE_FILENAME "source.bmp"
#define MAX_IMAGE_WIDTH 16384
#define MAX_IMAGE_HEIGHT 16384

typedef struct _rgbaint8
{
	unsigned char r;
	unsigned char g;
	unsigned char b;
	unsigned char a;
} __attribute((packed)) rgbaint8;

typedef struct {
	char id[2];
	long filesize;
	int reserved[1];
	long headersize;
	long infoSize;
	long width;
	long depth;
	short biPlanes;
	short bits;
	long biCompression;
	long biSizeImage;
	long biXPelsPerMeter;
	long biYPelsPerMeter;
	long biClrUsed;
	long biClrImportant;
} __attribute((packed)) BMPHEAD;

typedef struct _ImagesData_
{
	/* CL Objects */
	cl_context       psContext;
	cl_program       psProgram;
	cl_kernel        ps3x3Kernel;
	cl_kernel        psCopyKernel;
	cl_device_id     psDeviceID;
	cl_platform_id   psPlatformID;
	cl_command_queue psCommandQueue;
	cl_mem           psImage;
	cl_mem           psOutput;
	cl_mem           psValues;
	cl_sampler       psSampler;
	rgbaint8        *psInTextureData;
	rgbaint8        *psOutTextureData;
	cl_uint          uImageWidth;
	cl_uint          uImageHeight;
} ImagesData;

cl_int Init_Images(OCLTestInstance *psInstance);
cl_int Compute_Images(OCLTestInstance *psInstance);
cl_int Verify_Images(OCLTestInstance *psInstance);


static char *g_psImageSources =
{
	/* Image copy kernel */
	"__kernel void copyImage(__read_only image2d_t srcImage, __write_only image2d_t dstImage,\n"
	"\tsampler_t sampler)\n"
	"{\n"
		"\tfloat2 coords = (float2)((float)get_global_id(0) / (float)get_image_width(srcImage), (float)get_global_id(1) / (float)get_image_height(srcImage));\n"
		"\tfloat4 colour = read_imagef(srcImage, sampler, coords);\n"
		"\twrite_imagef(dstImage, (int2)(get_global_id(0), get_global_id(1)), colour);\n"
	"}\n"

	"\n\n"

	/* 3x3 convolution kernel */
	"__kernel void convolve3x3(__read_only image2d_t srcImage,\n"
	"\t__write_only image2d_t dstImage,\n"
	"\tsampler_t sampler,\n"
	"\t__global float *kVals,\n"
	"\tfloat normalizationVal)\n"
	"{\n"
		"\tfloat2 coords = (float2)(get_global_id(0), get_global_id(1));\n"
		"\tfloat4 colour;\n"
		"\tfloat4 colours[9];\n"
		"\n"
		"\tcolours[0] = read_imagef(srcImage, sampler, ((float2)coords + (float2)(-1.0f, -1.0f))/(float2)(get_image_width(srcImage), get_image_height(srcImage)));\n"
		"\tcolours[1] = read_imagef(srcImage, sampler, ((float2)coords + (float2)( 0.0f, -1.0f))/(float2)(get_image_width(srcImage), get_image_height(srcImage)));\n"
		"\tcolours[2] = read_imagef(srcImage, sampler, ((float2)coords + (float2)( 1.0f, -1.0f))/(float2)(get_image_width(srcImage), get_image_height(srcImage)));\n"
		"\tcolours[3] = read_imagef(srcImage, sampler, ((float2)coords + (float2)(-1.0f,  0.0f))/(float2)(get_image_width(srcImage), get_image_height(srcImage)));\n"
		"\tcolours[4] = read_imagef(srcImage, sampler, ((float2)coords + (float2)( 0.0f,  0.0f))/(float2)(get_image_width(srcImage), get_image_height(srcImage)));\n"
		"\tcolours[5] = read_imagef(srcImage, sampler, ((float2)coords + (float2)( 1.0f,  0.0f))/(float2)(get_image_width(srcImage), get_image_height(srcImage)));\n"
		"\tcolours[6] = read_imagef(srcImage, sampler, ((float2)coords + (float2)(-1.0f,  1.0f))/(float2)(get_image_width(srcImage), get_image_height(srcImage)));\n"
		"\tcolours[7] = read_imagef(srcImage, sampler, ((float2)coords + (float2)( 0.0f,  1.0f))/(float2)(get_image_width(srcImage), get_image_height(srcImage)));\n"
		"\tcolours[8] = read_imagef(srcImage, sampler, ((float2)coords + (float2)( 1.0f,  1.0f))/(float2)(get_image_width(srcImage), get_image_height(srcImage)));\n"
		"\n"
		"\tcolour  = colours[0] * kVals[0] + colours[1] * kVals[1] + colours[2] * kVals[2];\n"
		"\tcolour += colours[3] * kVals[3] + colours[4] * kVals[4] + colours[5] * kVals[5];\n"
		"\tcolour += colours[6] * kVals[6] + colours[7] * kVals[7] + colours[8] * kVals[8];\n"
		"\n"
		"\tcolour /= normalizationVal;\n"
		"\n"
		"\twrite_imagef(dstImage, (int2)(get_global_id(0), get_global_id(1)), colour);\n"
	"}\n"
};

/* 3x3 matrices */
const float Sharpen3x3[] = {  0.0f, -1.0f,  0.0f,
							  -1.0f,  5.0f, -1.0f,
							   0.0f, -1.0f,  0.0f,
							   1.0f };

const float Blur3x3[] = {  1.0f,  1.0f,  1.0f,
						   1.0f,  1.0f,  1.0f,
						   1.0f,  1.0f,  1.0f,
						   1.0f };

const float BlurAlt3x3[] = {  1.0f,  2.0f,  1.0f,
							  2.0f,  4.0f,  2.0f,
							  1.0f,  2.0f,  1.0f,
							  1.0f };

const float HighPassFilter3x3[] = { -1.0f, -1.0f, -1.0f,
									-1.0f,  9.0f, -1.0f,
									-1.0f, -1.0f, -1.0f,
									 1.0f };

const float LaplaceEdge3x3[] = { 0.0f, -1.0f,  0.0f,
								-1.0f,  4.0f, -1.0f,
								 0.0f, -1.0f,  0.0f,
								 1.0f };

const float Gaussian3x3[] = { 1.0f, 2.0f, 1.0f,
							  2.0f, 4.0f, 2.0f,
							  1.0f, 2.0f, 1.0f,
							  16.0f };

const float Emboss3x3[] = { -2.0f, -1.0f, 0.0f,
							-1.0f,  1.0f, 1.0f,
							 0.0f,  1.0f, 2.0f,
							 1.0f };


static void WriteCheckeredTexture(rgbaint8 *psTexture, cl_uint w, cl_uint h)
{
	cl_uint i, j;

	/* fill texture with checkered pattern */
	for (i = 0; i < h; i++)
	{
		for (j = 0; j < w; j++)
		{
			if ((i ^ j) & 0x8)
			{
				psTexture[i * w + j].r = 0xFF;
				psTexture[i * w + j].g = 0xFF;
				psTexture[i * w + j].b = 0xFF;
				psTexture[i * w + j].a = 0xFF;
			}
			else
			{
				psTexture[i * w + j].r = 0x00;
				psTexture[i * w + j].g = 0x00;
				psTexture[i * w + j].b = 0x00;
				psTexture[i * w + j].a = 0x00;
			}
		}
	}
}

static int Read24BPPBMP(char *pszFileName, rgbaint8 **ppsTexture, unsigned int *puiWidth, unsigned int *puiHeight)
{
	BMPHEAD sHeader;
	FILE *fp;
	int i;

	/* open file */
	if (!(fp = fopen(pszFileName, "rb")))
	{
		OCLTestLog("Failed to open %s for reading\n", pszFileName);
		return 0;
	}

	/* read header */
	if(fread(&sHeader, 1, sizeof(BMPHEAD), fp) != sizeof(BMPHEAD))
	{
		OCLTestLog("ERROR: Unexpected return value from fread.");
		fclose(fp);
		return 0;
	}

	/* check format */
	if (!(sHeader.id[0] == 'B' && sHeader.id[1] == 'M'))
	{
		OCLTestLog("%s does not appear to be a 24bpp bitmap\n", pszFileName);
		fclose(fp);
		return 0;
	}

	/* check bit depth */
	if (sHeader.bits != 24)
	{
		OCLTestLog("err: cannot read %s, unsupported number of bits (%d)\n", pszFileName, sHeader.bits);
		fclose(fp);
		return 0;
	}

	/* save dimensions */
	*puiWidth = sHeader.width;
	*puiHeight = sHeader.depth;

	/* Impose an arbitrary limit (16K) on textures to make sure we don't run out of memory */
	if(sHeader.width > MAX_IMAGE_WIDTH || sHeader.depth > MAX_IMAGE_HEIGHT)
	{
		fclose(fp);
		return 0;
	}

	/* allocate texture space */
	*ppsTexture = malloc(sizeof(rgbaint8) * sHeader.width * sHeader.depth);

	if (!*ppsTexture)
	{
		OCLTestLog("Allocation failure while loading %lix%li image from %s\n",
				   sHeader.width, sHeader.depth, pszFileName);
		fclose(fp);
		return 0;
	}

	memset(*ppsTexture, 0, sizeof(rgbaint8) * sHeader.width * sHeader.depth);

	/* read image data */
	for (i = 0; i < sHeader.width * sHeader.depth; ++i)
	{
		/* Read 3 components of 1 byte each */
		if(fread(&(*ppsTexture)[i], 1, 3, fp) != 3)
		{
			OCLTestLog("ERROR: Unexpected return value from fread.");
			fclose(fp);
			return 0;
		}

		/* Set the alpha channel to opaque */
		(*ppsTexture)[i].a = 0xFF;
	}
	fclose(fp);
	return 1;
}

static cl_int RunConv3x3Kernel(ImagesData *psData, const float *pfMatrix)
{
	cl_int eResult;
	size_t auOrigin[3] = {0, 0, 0};
	size_t auRegion[3] = {psData->uImageWidth, psData->uImageHeight, 1};
	size_t global[2] = {psData->uImageWidth, psData->uImageHeight};
	size_t local[2] = {1, 1};

	/* Write the Matrix */
	eResult = clEnqueueWriteBuffer(psData->psCommandQueue, psData->psValues, CL_TRUE, 0, sizeof(float) * 9, pfMatrix, 0, NULL, NULL);
	if (eResult != CL_SUCCESS) return eResult;

	/* Set kernel arguments */
	eResult = clSetKernelArg(psData->ps3x3Kernel, 0, sizeof(cl_mem), &psData->psImage);
	if (eResult != CL_SUCCESS) return eResult;
	eResult = clSetKernelArg(psData->ps3x3Kernel, 1, sizeof(cl_mem), &psData->psOutput);
	if (eResult != CL_SUCCESS) return eResult;
	eResult = clSetKernelArg(psData->ps3x3Kernel, 2, sizeof(cl_sampler), &psData->psSampler);
	if (eResult != CL_SUCCESS) return eResult;
	eResult = clSetKernelArg(psData->ps3x3Kernel, 3, sizeof(cl_mem), &psData->psValues);
	if (eResult != CL_SUCCESS) return eResult;
	eResult = clSetKernelArg(psData->ps3x3Kernel, 4, sizeof(cl_float), &pfMatrix[9]);
	if (eResult != CL_SUCCESS) return eResult;
	eResult = clEnqueueNDRangeKernel(psData->psCommandQueue, psData->ps3x3Kernel, 2, NULL, global, local, 0, NULL, NULL);
	if (eResult != CL_SUCCESS) return eResult;
	clFinish(psData->psCommandQueue);

	eResult = clEnqueueReadImage(psData->psCommandQueue,
								 psData->psOutput,
								 CL_TRUE,
								 auOrigin,
								 auRegion,
								 0,
								 0,
								 psData->psOutTextureData,
								 0,
								 NULL,
								 NULL);

	return eResult;
}

/***********************************************************************************
 Function Name      : Init_Images
 Description        : Initialises the images test
************************************************************************************/
cl_int Init_Images(OCLTestInstance *psInstance)
{
	cl_int eResult;
	char* ppszSources[2] = {0, 0};
	cl_image_format sFormat;
	ImagesData *psData = (ImagesData*)calloc(1, sizeof(ImagesData));
	size_t auOrigin[3] = {0, 0, 0};
	size_t auRegion[3] = {0, 0, 1};

	if(!psData)
	{
		return CL_OUT_OF_RESOURCES;
	}

	/* Initialise data */
	psInstance->pvPrivateData = (void*)psData;

	eResult = clGetPlatformIDs(1, &psData->psPlatformID, NULL);
	CheckAndReportError(psInstance, "clGetPlatformIDs", eResult, init_images_cleanup);

	eResult = clGetDeviceIDs(psData->psPlatformID, CL_DEVICE_TYPE_GPU, 1, &psData->psDeviceID, NULL);
	CheckAndReportError(psInstance, "clGetDeviceIDs", eResult, init_images_cleanup);

	psData->psContext = clCreateContext(NULL, 1, &psData->psDeviceID, NULL, NULL, &eResult);
	CheckAndReportError(psInstance, "clCreateContext", eResult, init_images_cleanup);

	/* Generate the test program */
	ppszSources[0] = g_psImageSources;
	ppszSources[1] = "\0";
	psData->psProgram = clCreateProgramWithSource(psData->psContext, 1, (const char**)ppszSources, NULL, &eResult);
	CheckAndReportError(psInstance, "clCreateProgramWithSource", eResult, init_images_cleanup);

	eResult = clBuildProgram(psData->psProgram, 1, &psData->psDeviceID, "-cl-fast-relaxed-math", NULL, NULL);
	if (eResult != CL_SUCCESS)
	{
		char aszBuildLog[512];
		size_t uBuildLogSize;
		cl_int eLogError;

		eLogError = clGetProgramBuildInfo(psData->psProgram, psData->psDeviceID, CL_PROGRAM_BUILD_LOG, 512, aszBuildLog, &uBuildLogSize);
		CheckAndReportError(psInstance, "clGetProgramBuildInfo", eLogError, init_images_cleanup);

		aszBuildLog[uBuildLogSize - 1] = '\0';
		OCLTestLog("[%zu] CL_PROGRAM_BUILD_LOG:\n%s\n", uBuildLogSize, aszBuildLog);
	}
	CheckAndReportError(psInstance, "clBuildProgram", eResult, init_images_cleanup);

	psData->psCommandQueue = clCreateCommandQueue(psData->psContext, psData->psDeviceID, 0, &eResult);
	CheckAndReportError(psInstance, "clCreateCommandQueue", eResult, init_images_cleanup);

	eResult = Read24BPPBMP(IMAGE_FILENAME, &psData->psInTextureData, &psData->uImageWidth, &psData->uImageHeight);
	if (eResult == 0 ||
		psData->uImageWidth > MAX_IMAGE_WIDTH ||
		psData->uImageHeight > MAX_IMAGE_HEIGHT)
	{
		/* If any allocation was performed, undo it */
		if (psData->psInTextureData != NULL)
		{
			free(psData->psInTextureData);
		}
		psData->psInTextureData = malloc(sizeof(rgbaint8) * 256 * 256);
		if (psData->psInTextureData == NULL)
		{
			OCLTestLog("Unable to allocate memory for texture data\n");
			return CL_OUT_OF_HOST_MEMORY;
		}
		WriteCheckeredTexture(psData->psInTextureData, 256, 256);
		psData->uImageHeight = psData->uImageWidth = 256;
	}

	sFormat.image_channel_order = CL_RGBA;
	sFormat.image_channel_data_type = CL_UNORM_INT8;
	psData->psImage = clCreateImage2D(psData->psContext, CL_MEM_READ_WRITE, &sFormat, psData->uImageWidth, psData->uImageHeight, 0, NULL, &eResult);
	CheckAndReportError(psInstance, "clCreateImage2D", eResult, init_images_cleanup);
	auRegion[0] = psData->uImageWidth;
	auRegion[1] = psData->uImageHeight;
	clEnqueueWriteImage(psData->psCommandQueue, psData->psImage, CL_TRUE, auOrigin, auRegion, 0, 0, psData->psInTextureData, 0, NULL, NULL);
	CheckAndReportError(psInstance, "clEnqueueWriteImage", eResult, init_images_cleanup);

	psData->psOutput = clCreateImage2D(psData->psContext, CL_MEM_READ_WRITE, &sFormat, psData->uImageWidth, psData->uImageHeight,
	                                   0, NULL, &eResult);
	CheckAndReportError(psInstance, "clCreateImage2D", eResult, init_images_cleanup);

	psData->psSampler = clCreateSampler(psData->psContext, CL_TRUE, CL_ADDRESS_REPEAT, CL_FILTER_NEAREST, &eResult);
	CheckAndReportError(psInstance, "clCreateSampler", eResult, init_images_cleanup);

	/* Memory buffer for the matrix of the filters*/
	psData->psValues = clCreateBuffer(psData->psContext, CL_MEM_READ_ONLY, sizeof(cl_float) * 9, NULL, &eResult);
	CheckAndReportError(psInstance, "clCreateBuffer", eResult, init_images_cleanup);

	/* Fetch the convolution kernel */
	psData->ps3x3Kernel = clCreateKernel(psData->psProgram, "convolve3x3", &eResult);
	CheckAndReportError(psInstance, "clCreateKernel", eResult, init_images_cleanup);

	/* Fetch the image copy kernel */
	psData->psCopyKernel = clCreateKernel(psData->psProgram, "copyImage", &eResult);
	CheckAndReportError(psInstance, "clCreateKernel", eResult, init_images_cleanup);

	psData->psOutTextureData = malloc(psData->uImageWidth * psData->uImageHeight * sizeof(rgbaint8));
	if (psData->psOutTextureData == NULL)
	{
		OCLTestLog("Failed to allocate memory for Images return data\n");
		return CL_OUT_OF_HOST_MEMORY;
	}

	return CL_SUCCESS;

init_images_cleanup:
	return eResult;
}

/***********************************************************************************
 Function Name      : Compute_Template
 Inputs             : psInstance - test instance data
 Description        : Runs the images kernels
************************************************************************************/
cl_int Compute_Images(OCLTestInstance *psInstance)
{
	ImagesData *psData = (ImagesData*) psInstance->pvPrivateData;
	cl_int eResult;
	size_t auOrigin[3] = {0, 0, 0};
	size_t auRegion[3] = {psData->uImageWidth, psData->uImageHeight, 1};
	size_t global[2] = {psData->uImageWidth, psData->uImageHeight};
	size_t local[2] = {1, 1};
	cl_uint frame = 0;

	/* Run image copy kernel */
	OCLTestLog("Running image copy, frame %u\n", frame++);
	eResult = clSetKernelArg(psData->psCopyKernel, 0, sizeof(cl_mem), &psData->psImage);
	CheckAndReportError(psInstance, "clSetKernelArg", eResult, compute_images_cleanup);
	eResult = clSetKernelArg(psData->psCopyKernel, 1, sizeof(cl_mem), &psData->psOutput);
	CheckAndReportError(psInstance, "clSetKernelArg", eResult, compute_images_cleanup);
	eResult = clSetKernelArg(psData->psCopyKernel, 2, sizeof(cl_sampler), &psData->psSampler);
	CheckAndReportError(psInstance, "clSetKernelArg", eResult, compute_images_cleanup);
	eResult = clEnqueueNDRangeKernel(psData->psCommandQueue, psData->psCopyKernel, 2, NULL, global, local, 0, NULL, NULL);
	CheckAndReportError(psInstance, "clEnqueueNDRangeKernel", eResult, compute_images_cleanup);
	clFinish(psData->psCommandQueue);
	eResult = clEnqueueReadImage(psData->psCommandQueue, psData->psOutput, CL_TRUE, auOrigin,
	                              auRegion, 0, 0, psData->psOutTextureData, 0, NULL, NULL);

	/* Run convolution kernels */
	OCLTestLog("Running Sharpen kernel, frame %u\n", frame++);
	eResult = RunConv3x3Kernel(psData, Sharpen3x3);
	CheckAndReportError(psInstance, "Sharpen", eResult, compute_images_cleanup);
	OCLTestLog("Running Blur kernel, frame %u\n", frame++);
	eResult = RunConv3x3Kernel(psData, Blur3x3);
	CheckAndReportError(psInstance, "Blur", eResult, compute_images_cleanup);
	OCLTestLog("Running Gaussian kernel, frame %u\n", frame++);
	eResult = RunConv3x3Kernel(psData, Gaussian3x3);
	CheckAndReportError(psInstance, "Gaussian", eResult, compute_images_cleanup);
	OCLTestLog("Running Laplace Edge detection kernel, frame %u\n", frame++);
	eResult = RunConv3x3Kernel(psData, LaplaceEdge3x3);
	CheckAndReportError(psInstance, "Laplace", eResult, compute_images_cleanup);

	/* Done */
	return CL_SUCCESS;

compute_images_cleanup:
	return eResult;
}

/***********************************************************************************
 Function Name      : Verify_Template
 Description        : Reads the image data back
************************************************************************************/
cl_int Verify_Images(OCLTestInstance *psInstance)
{
	ImagesData *psData = (ImagesData*) psInstance->pvPrivateData;

	/* Cleanup */
	clReleaseKernel(psData->ps3x3Kernel);
	clReleaseKernel(psData->psCopyKernel);
	clReleaseSampler(psData->psSampler);
	clReleaseMemObject(psData->psImage);
	clReleaseMemObject(psData->psOutput);
	clReleaseMemObject(psData->psValues);
	clReleaseCommandQueue(psData->psCommandQueue);
	clReleaseProgram(psData->psProgram);
	clReleaseContext(psData->psContext);

	free(psData->psInTextureData);
	free(psData->psOutTextureData);

	if (psInstance->pvPrivateData)
	{
		free(psInstance->pvPrivateData);
		psInstance->pvPrivateData = NULL;
	}

	return CL_SUCCESS;
}

/******************************************************************************
 End of file (images.c)
******************************************************************************/
