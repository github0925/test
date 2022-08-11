/*************************************************************************/ /*!
@File           platform.c
@Copyright      Copyright (c) Imagination Technologies Ltd. All Rights Reserved
@License        Strictly Confidential.
*/ /**************************************************************************/
#define TEST_PLATFORM_MAX_PLATFORMS 32

cl_int Verify_Platform(OCLTestInstance *psInstance);

/***********************************************************************************
 Function Name      : Verify_Platform
 Inputs             : None
 Outputs            : None
 Returns            : None
 Description        : Verifies output data
************************************************************************************/
cl_int Verify_Platform(OCLTestInstance *psInstance)
{
	cl_platform_id ppsPlatform[TEST_PLATFORM_MAX_PLATFORMS] = {0};
	cl_uint ui32NumPlatforms                 = 0;
	cl_uint p;
	cl_int  eResult = CL_SUCCESS;
	cl_bool bImaginationIsPresent = CL_FALSE;
	char *pszPlatformInfo = NULL;

	eResult = clGetPlatformIDs(TEST_PLATFORM_MAX_PLATFORMS,ppsPlatform,&ui32NumPlatforms);
	CheckAndReportError(psInstance,"clGetPlatformIDs",eResult,verify_platform_cleanup);

	OCLTestLog("%s: Enumerating %d platforms\n",__func__,ui32NumPlatforms);

	for(p=0; p < ui32NumPlatforms; p++)
	{
		char aszString[512] = {0};
		cl_platform_id psPlatform = ppsPlatform[p];

		eResult = clGetPlatformInfo(psPlatform,CL_PLATFORM_PROFILE,512,aszString,NULL);
		CheckAndReportError(psInstance,"clGetPlatformInfo",eResult,verify_platform_cleanup);
		OCLTestLog("%s: CL_PLATFORM_PROFILE    %s\n",__func__,aszString);

		eResult = clGetPlatformInfo(psPlatform,CL_PLATFORM_VERSION,512,aszString,NULL);
		CheckAndReportError(psInstance,"clGetPlatformInfo",eResult,verify_platform_cleanup);
		OCLTestLog("%s: CL_PLATFORM_VERSION    %s\n",__func__,aszString);

		eResult = clGetPlatformInfo(psPlatform,CL_PLATFORM_NAME,512,aszString,NULL);
		CheckAndReportError(psInstance,"clGetPlatformInfo",eResult,verify_platform_cleanup);
		OCLTestLog("%s: CL_PLATFORM_NAME       %s\n",__func__,aszString);

		eResult = clGetPlatformInfo(psPlatform,CL_PLATFORM_VENDOR,512,aszString,NULL);
		CheckAndReportError(psInstance,"clGetPlatformInfo",eResult,verify_platform_cleanup);
		OCLTestLog("%s: CL_PLATFORM_VENDOR     %s\n",__func__,aszString);

		if(!strcmp(TEST_VENDOR_STRING,aszString))
		{
			bImaginationIsPresent = CL_TRUE;
		}

		size_t config_size_set;
		size_t config_size_ret;
		eResult = clGetPlatformInfo(psPlatform, CL_PLATFORM_EXTENSIONS, 0, NULL, &config_size_set);
		CheckAndReportError(psInstance,"clGetPlatformInfo",eResult,verify_platform_cleanup);

		if (config_size_set > 0)
		{
			pszPlatformInfo = (char*)malloc(config_size_set);
			if (!pszPlatformInfo)
			{
				eResult = CL_OUT_OF_RESOURCES;
				goto verify_platform_cleanup;
			}

			eResult = clGetPlatformInfo(psPlatform, CL_PLATFORM_EXTENSIONS, config_size_set, pszPlatformInfo, &config_size_ret);
			CheckAndReportError(psInstance,"clGetPlatformInfo",eResult,verify_platform_cleanup);
			OCLTestLog("%s: CL_PLATFORM_EXTENSIONS %s\n", __func__, pszPlatformInfo);
			free(pszPlatformInfo);
			pszPlatformInfo = NULL;
		}
	}

	if (!bImaginationIsPresent)
	{
		psInstance->pvPrivateData = strdup("Vendor string was not present.");
		return CL_INVALID_VALUE;
	}

	return eResult;

verify_platform_cleanup:
	free(pszPlatformInfo);
	return eResult;
}

/******************************************************************************
 End of file (platform.c)
******************************************************************************/
