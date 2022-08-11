/*************************************************************************/ /*!
@File           ocltest.c
@Copyright      Copyright (c) Imagination Technologies Ltd. All Rights Reserved
@License        Strictly Confidential.
*/ /**************************************************************************/


#include "ocltest.h"
#include <stdarg.h>
#include <time.h>
#include <sys/time.h>
#include "slt_gpu.h"
#include "pvrversion.h"

unsigned long
OCLGetTime(void)
{
	struct timeval tv;
	gettimeofday(&tv,NULL);
	return (unsigned long)(((unsigned long)tv.tv_sec*(unsigned long)1000) + (tv.tv_usec/1000.0));
}

int Ocl_EglMainArgs(char* result_string)
{
	OCLTestInstance *psInstance;
	unsigned int     t;
	unsigned int     ui32NumPassed       = 0;
	unsigned int     ui32NumRan          = 0;
	float            fPercentage;
	unsigned long    ulStartTime;
	unsigned long    ulEndTime;
	unsigned long    ulInitTime;
	unsigned long    ulComputeTime;
	unsigned long    ulVerifyTime;
	float            fTimeInSeconds;
	float            fTotalTimeInSeconds = 0.0f;

	cl_int eError;
	cl_int eResult;
	cl_bool bContinue = CL_TRUE;

	time_t rawtime;
	time(&rawtime);
	int ret = 0;

	OCLTestLog("OpenCL Unit Test(s) (%s) at %s",PVRVERSION_STRING,ctime(&rawtime));

	for(t=0; t < OCLTEST_LIST_SIZE; t++)
	{
		psInstance = &g_TestList[t];
		bContinue = CL_TRUE;

		/* Output test information */
		OCLTestLog("%02d******************************************************************************\n",ui32NumRan);
		OCLTestLog("%s\n",psInstance->pszTestDesc);
		OCLTestLog("********************************************************************************\n");

		ulStartTime = OCLGetTime();

		if(psInstance->pfnInit)
		{
			eError  = psInstance->pfnInit(psInstance);

			if(eError != CL_SUCCESS)
			{
				OCLTestLog("CL_ERROR_TEST: %s\n",OCLGetErrorStr(eError));
				if (ret == 0) ret = ERR_CLCASE(t, OCL_INIT_ERR);
				bContinue = CL_FALSE;
			}
		}

		ulInitTime = OCLGetTime();

		if(bContinue && psInstance->pfnCompute)
		{
			eError  = psInstance->pfnCompute(psInstance);

			if(eError != CL_SUCCESS)
			{
				OCLTestLog("CL_ERROR_TEST: %s\n",OCLGetErrorStr(eError));
				OCLTestLog("%s --> failed with error: %s\n",psInstance->pszTestID,psInstance->pvPrivateData ? (char*)psInstance->pvPrivateData : "unknown");
				if (ret == 0) ret = ERR_CLCASE(t, OCL_CMPT_ERR);
				bContinue = CL_FALSE;
			}
		}

		ulComputeTime = OCLGetTime();

		if(bContinue && psInstance->pfnVerify)
		{
			eResult = psInstance->pfnVerify(psInstance);

			if(eResult == CL_SUCCESS)
			{
				OCLTestLog("%s --> passed\n",psInstance->pszTestID);
				/* Record the number of passing tests */
				ui32NumPassed++;
			}
			else
			{
				OCLTestLog("%s --> failed\n", psInstance->pszTestID);
				if (ret == 0) ret = ERR_CLCASE(t, OCL_VRFY_ERR);

				if(psInstance->pvPrivateData)
				{
					free(psInstance->pvPrivateData);
				}
			}
		}

		ulVerifyTime = OCLGetTime();
		ulEndTime    = OCLGetTime();

		/* Calculate actual segment times (order important) */
		ulVerifyTime  -= ulComputeTime;
		ulComputeTime -= ulInitTime;
		ulInitTime    -= ulStartTime;

		fTimeInSeconds = (ulEndTime - ulStartTime) * 0.001f;
		fTotalTimeInSeconds += fTimeInSeconds;

		OCLTestLog("%s: Test took %.2f seconds to run:\n",psInstance->pszTestID,fTimeInSeconds);
		OCLTestLog("%s: ",psInstance->pszTestID);
		if(ulInitTime > 0)
			OCLTestLog("Init %.2fs (%03.2f%%)  ",ulInitTime * 0.001f,(ulInitTime * 0.1f) / fTimeInSeconds);
		if(ulComputeTime > 0)
			OCLTestLog("Compute %.2fs (%03.2f%%)",ulComputeTime * 0.001f,(ulComputeTime * 0.1f) / fTimeInSeconds);
		if(ulVerifyTime > 0)
			OCLTestLog("Verify %.2fs (%03.2f)%%",ulVerifyTime  * 0.001f,(ulVerifyTime  * 0.1f) / fTimeInSeconds);
		OCLTestLog("\n");

		/* Record the number of tests that have run */
		ui32NumRan++;
	}

	fPercentage = ((float)ui32NumPassed/(float)ui32NumRan) * 100.0f;
	OCLTestLog("Finished %d tests in %.1f seconds: %d passed, %d failed (%.2f%%)\n",
	       ui32NumRan,fTotalTimeInSeconds,ui32NumPassed,ui32NumRan-ui32NumPassed,fPercentage);
	return ret;
}

/*
 * Utility Functions
 */

const char *OCLGetDeviceTypeStr(cl_device_type eDeviceType)
{
	const char *pszDeviceTypeStr = NULL;

	switch(eDeviceType)
	{
	    case CL_DEVICE_TYPE_ACCELERATOR: pszDeviceTypeStr = "CL_DEVICE_TYPE_ACCELERATOR"; break;
	    case CL_DEVICE_TYPE_CPU: pszDeviceTypeStr         = "CL_DEVICE_TYPE_CPU"; break;
	    case CL_DEVICE_TYPE_GPU: pszDeviceTypeStr         = "CL_DEVICE_TYPE_GPU"; break;
	    case CL_DEVICE_TYPE_DEFAULT: pszDeviceTypeStr     = "CL_DEVICE_TYPE_DEFAULT"; break;

	    default: pszDeviceTypeStr = "Unknown Device Type";
	}

	return pszDeviceTypeStr;
}

const char* OCLGetErrorStr(cl_int eError)
{
	const char *pszErrorStr = NULL;

	switch(eError)
	{
	    case CL_SUCCESS: pszErrorStr                         = "CL_SUCCESS"; break;
	    case CL_DEVICE_NOT_FOUND: pszErrorStr                = "CL_DEVICE_NOT_FOUND"; break;
	    case CL_DEVICE_NOT_AVAILABLE: pszErrorStr            = "CL_DEVICE_NOT_AVAILABLE"; break;
	    case CL_COMPILER_NOT_AVAILABLE: pszErrorStr          = "CL_COMPILER_NOT_AVAILABLE"; break;
	    case CL_MEM_OBJECT_ALLOCATION_FAILURE: pszErrorStr   = "CL_MEM_OBJECT_ALLOCATION_FAILURE"; break;
	    case CL_OUT_OF_RESOURCES: pszErrorStr                = "CL_OUT_OF_RESOURCES"; break;
	    case CL_OUT_OF_HOST_MEMORY: pszErrorStr              = "CL_OUT_OF_HOST_MEMORY"; break;
	    case CL_PROFILING_INFO_NOT_AVAILABLE: pszErrorStr    = "CL_PROFILING_INFO_NOT_AVAILABLE"; break;
	    case CL_MEM_COPY_OVERLAP: pszErrorStr                = "CL_MEM_COPY_OVERLAP"; break;
	    case CL_IMAGE_FORMAT_MISMATCH: pszErrorStr           = "CL_IMAGE_FORMAT_MISMATCH"; break;
	    case CL_IMAGE_FORMAT_NOT_SUPPORTED: pszErrorStr      = "CL_IMAGE_FORMAT_NOT_SUPPORTED"; break;
	    case CL_BUILD_PROGRAM_FAILURE: pszErrorStr           = "CL_BUILD_PROGRAM_FAILURE"; break;
	    case CL_MAP_FAILURE: pszErrorStr                     = "CL_MAP_FAILURE"; break;
	    case CL_INVALID_VALUE: pszErrorStr                   = "CL_INVALID_VALUE"; break;
	    case CL_INVALID_DEVICE_TYPE: pszErrorStr             = "CL_INVALID_DEVICE_TYPE"; break;
	    case CL_INVALID_PLATFORM: pszErrorStr                = "CL_INVALID_PLATFORM"; break;
	    case CL_INVALID_DEVICE: pszErrorStr                  = "CL_INVALID_DEVICE"; break;
	    case CL_INVALID_CONTEXT: pszErrorStr                 = "CL_INVALID_CONTEXT"; break;
	    case CL_INVALID_QUEUE_PROPERTIES: pszErrorStr        = "CL_INVALID_QUEUE_PROPERTIES"; break;
	    case CL_INVALID_COMMAND_QUEUE: pszErrorStr           = "CL_INVALID_COMMAND_QUEUE"; break;
	    case CL_INVALID_HOST_PTR: pszErrorStr                = "CL_INVALID_HOST_PTR"; break;
	    case CL_INVALID_MEM_OBJECT: pszErrorStr              = "CL_INVALID_MEM_OBJECT"; break;
	    case CL_INVALID_IMAGE_FORMAT_DESCRIPTOR: pszErrorStr = "CL_INVALID_IMAGE_FORMAT_DESCRIPTOR"; break;
	    case CL_INVALID_IMAGE_SIZE: pszErrorStr              = "CL_INVALID_IMAGE_SIZE"; break;
	    case CL_INVALID_SAMPLER: pszErrorStr                 = "CL_INVALID_SAMPLER"; break;
	    case CL_INVALID_BINARY: pszErrorStr                  = "CL_INVALID_BINARY"; break;
	    case CL_INVALID_BUILD_OPTIONS: pszErrorStr           = "CL_INVALID_BUILD_OPTIONS"; break;
	    case CL_INVALID_PROGRAM: pszErrorStr                 = "CL_INVALID_PROGRAM"; break;
	    case CL_INVALID_PROGRAM_EXECUTABLE: pszErrorStr      = "CL_INVALID_PROGRAM_EXECUTABLE"; break;
	    case CL_INVALID_KERNEL_NAME: pszErrorStr             = "CL_INVALID_KERNEL_NAME"; break;
	    case CL_INVALID_KERNEL_DEFINITION: pszErrorStr       = "CL_INVALID_KERNEL_DEFINITION"; break;
	    case CL_INVALID_KERNEL: pszErrorStr                  = "CL_INVALID_KERNEL"; break;
	    case CL_INVALID_ARG_INDEX: pszErrorStr               = "CL_INVALID_ARG_INDEX"; break;
	    case CL_INVALID_ARG_VALUE: pszErrorStr               = "CL_INVALID_ARG_VALUE"; break;
	    case CL_INVALID_ARG_SIZE: pszErrorStr                = "CL_INVALID_ARG_SIZE"; break;
	    case CL_INVALID_KERNEL_ARGS: pszErrorStr             = "CL_INVALID_KERNEL_ARGS"; break;
	    case CL_INVALID_WORK_DIMENSION: pszErrorStr          = "CL_INVALID_WORK_DIMENSION"; break;
	    case CL_INVALID_WORK_GROUP_SIZE: pszErrorStr         = "CL_INVALID_WORK_GROUP_SIZE"; break;
	    case CL_INVALID_WORK_ITEM_SIZE: pszErrorStr          = "CL_INVALID_WORK_ITEM_SIZE"; break;
	    case CL_INVALID_GLOBAL_OFFSET: pszErrorStr           = "CL_INVALID_GLOBAL_OFFSET"; break;
	    case CL_INVALID_EVENT_WAIT_LIST: pszErrorStr         = "CL_INVALID_EVENT_WAIT_LIST"; break;
	    case CL_INVALID_EVENT: pszErrorStr                   = "CL_INVALID_EVENT"; break;
	    case CL_INVALID_OPERATION: pszErrorStr               = "CL_INVALID_OPERATION"; break;
	    case CL_INVALID_GL_OBJECT: pszErrorStr               = "CL_INVALID_GL_OBJECT"; break;
	    case CL_INVALID_BUFFER_SIZE: pszErrorStr             = "CL_INVALID_BUFFER_SIZE"; break;
	    case CL_INVALID_MIP_LEVEL: pszErrorStr               = "CL_INVALID_MIP_LEVEL"; break;
	    case CL_INVALID_GLOBAL_WORK_SIZE: pszErrorStr        = "CL_INVALID_GLOBAL_WORK_SIZE"; break;
	    case CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST: pszErrorStr	 = "CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST"; break;

	    default:
	    {
		    pszErrorStr = "Unknown Error Code";
	    }
	}

	return pszErrorStr;
}

unsigned char *
OCLLoadBinary(char *pszBinaryName, size_t *puLength)
{
	FILE *fpBinary = NULL;
	size_t uSize = 0;
	unsigned char* pvBinary = NULL;

	/* open binary file */
	fpBinary = fopen(pszBinaryName,"rb");

	if(!fpBinary)
	{
		OCLTestLog("ERROR: Failed to load %s\n",pszBinaryName);
		return NULL;
	}

	/* get binary size */
	fseek(fpBinary,0,SEEK_END);
	uSize = ftell(fpBinary);
	fseek(fpBinary,0,SEEK_SET);

	/* allocate buffer */
	pvBinary = (unsigned char*)malloc(uSize);

	if(!pvBinary)
	{
		OCLTestLog("ERROR: Failed to allocate memory for binary file.");
		fclose(fpBinary);
		return NULL;
	}

	if(fread(pvBinary,uSize,1,fpBinary) != 1)
	{
		OCLTestLog("ERROR: Unexpected return value from fread.");
		free(pvBinary);
		fclose(fpBinary);
		return NULL;
	}

	/* close file handle */
	fclose(fpBinary);

	/* return size of binary */
	*puLength = uSize;

	/* return binary data */
	return pvBinary;
}

int
OCLOutputBinariesToFile(cl_program psProgram, char *pszPrefix, unsigned int *pui32NumBinaries)
{
	int            nBinaries      = 0;
	size_t         auBinSizes[10] = {0};
	unsigned char *apucBinaries[10];
	int i;

	if(!pszPrefix)
		pszPrefix = "cl_binary";

	if(clGetProgramInfo(psProgram, CL_PROGRAM_BINARY_SIZES, sizeof(size_t)*10, auBinSizes, NULL) != CL_SUCCESS)
	{
		OCLTestLog("ERROR: Failed to get CL_PROGRAM_BINARY_SIZES");
		return 0;
	}

	for(i=0; i < 10; i++)
	{
		if(!auBinSizes[i])
			break;

		apucBinaries[i] = (unsigned char*)malloc(auBinSizes[i]*sizeof(char));

		if(!apucBinaries[i])
		{
			OCLTestLog("ERROR: Allocation failed for dumping binary shaders.");
			return 0;
		}

		nBinaries++;
	}

	if(clGetProgramInfo(psProgram, CL_PROGRAM_BINARIES, sizeof(unsigned char *) * 10, apucBinaries, NULL) != CL_SUCCESS)
	{
		OCLTestLog("ERROR: Failed to get CL_PROGRAM_BINARIES");
		return 0;
	}

	for(i=0; i < nBinaries; i++)
	{
		char szFileName[256] = {0};
		FILE *fp             = NULL;

		snprintf(szFileName, 256, "%s%d.bin", pszPrefix, i);

		fp = fopen(szFileName,"wb");

		if(!fp)
		{
			OCLTestLog("ERROR: Failed to write out a binary shader");
			return 0;
		}

		fwrite(apucBinaries[i],1,auBinSizes[i],fp);
		fclose(fp);

		free(apucBinaries[i]);
	}

	if(pui32NumBinaries)
	{
		*pui32NumBinaries = nBinaries;
	}

	return 1;
}

void
OCLTestLog(char *pszFormat, ...)
{
	FILE *fpOutputLog;
	va_list vaList;

	/* Process arguments */
	va_start(vaList,pszFormat);

	/* Print output to user */
	vfprintf(stderr, pszFormat, vaList);

	va_end(vaList);

	/* Process arguments */
	va_start(vaList,pszFormat);

	fpOutputLog = fopen("oclunittest.log","a");

	if(fpOutputLog)
	{
		vfprintf(fpOutputLog, pszFormat, vaList);
		fclose(fpOutputLog);
	}

	va_end(vaList);
}

/******************************************************************************
 End of file (ocltest.c)
******************************************************************************/
