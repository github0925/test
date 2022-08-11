/*************************************************************************/ /*!
@File           device.c
@Copyright      Copyright (c) Imagination Technologies Ltd. All Rights Reserved
@License        Strictly Confidential.
*/ /**************************************************************************/
#include <inttypes.h>

#define TEST_DEVICE_MAX_DEVICES 32

typedef struct _DeviceData_
{
	cl_platform_id psPlatformID;
} DeviceData;

cl_int Init_Device(OCLTestInstance *psInstance);
cl_int Compute_Device(OCLTestInstance *psInstance);
cl_int Verify_Device(OCLTestInstance *psInstance);

/***********************************************************************************
 Function Name      : Init_Device
 Inputs             : None
 Outputs            : None
 Returns            : None
 Description        : Initialises input data
************************************************************************************/
cl_int Init_Device(OCLTestInstance *psInstance)
{
	DeviceData *psData = (DeviceData*)calloc(1, sizeof(DeviceData));
	cl_int eResult;

	if(!psData)
	{
		return CL_OUT_OF_RESOURCES;
	}

	/* Initialise data */
	psInstance->pvPrivateData = (void*)psData;

	eResult = clGetPlatformIDs(1,&psData->psPlatformID,NULL);
	CheckAndReportError(psInstance, "clGetPlatformIDs", eResult, init_device_cleanup);

	return CL_SUCCESS;

init_device_cleanup:
	return eResult;
}

/***********************************************************************************
 Function Name      : Verify_Device
 Inputs             : None
 Outputs            : None
 Returns            : None
 Description        : Verifies output data
************************************************************************************/
cl_int Verify_Device(OCLTestInstance *psInstance)
{
	DeviceData   *psData = (DeviceData*) psInstance->pvPrivateData;
	cl_int        eResult = CL_SUCCESS;
	char          aszString[512];
	cl_device_id  ppsDeviceID[TEST_DEVICE_MAX_DEVICES];
	char         *pszPlatformInfo = NULL;
	cl_uint       ui32NumDevices;
	cl_uint       d;

	eResult = clGetPlatformInfo(psData->psPlatformID,CL_PLATFORM_VENDOR,512,aszString,NULL);
	CheckAndReportError(psInstance, "clGetPlatformInfo", eResult, verify_device_cleanup);

	/* Imagination is not the vendor */
	if(strcmp(aszString,TEST_VENDOR_STRING))
	{
		free(psInstance->pvPrivateData);
		psInstance->pvPrivateData = strdup("Invalid vendor string");
		return CL_OUT_OF_RESOURCES;
	}

	/* Enumerate the devices */
	eResult = clGetDeviceIDs(psData->psPlatformID,CL_DEVICE_TYPE_ALL,TEST_DEVICE_MAX_DEVICES,ppsDeviceID,&ui32NumDevices);
	CheckAndReportError(psInstance, "clGetDeviceIDs", eResult, verify_device_cleanup);

	OCLTestLog("%s: Enumerating %d devices\n",__func__,ui32NumDevices);
	for(d=0; d < ui32NumDevices; d++)
	{
		cl_device_type uDeviceType;

		/* CL_DEVICE_TYPE */
		eResult = clGetDeviceInfo(ppsDeviceID[d],CL_DEVICE_TYPE,sizeof(cl_device_type),&uDeviceType,NULL);
		CheckAndReportError(psInstance,"clGetDeviceInfo",eResult,verify_device_cleanup);
		OCLTestLog("%s: CL_DEVICE_TYPE       %s\n",__func__,OCLGetDeviceTypeStr(uDeviceType));

		/* CL_DEVICE_NAME */
		eResult = clGetDeviceInfo(ppsDeviceID[d],CL_DEVICE_NAME,512,aszString,NULL);
		CheckAndReportError(psInstance,"clGetDeviceInfo",eResult,verify_device_cleanup);
		OCLTestLog("%s: CL_DEVICE_NAME       %s\n",__func__,aszString);

		/* CL_DEVICE_VENDOR */
		eResult = clGetDeviceInfo(ppsDeviceID[d],CL_DEVICE_VENDOR,512,aszString,NULL);
		CheckAndReportError(psInstance,"clGetDeviceInfo",eResult,verify_device_cleanup);
		OCLTestLog("%s: CL_DEVICE_VENDOR     %s\n",__func__,aszString);

		/* CL_DRIVER_VERSION */
		eResult = clGetDeviceInfo(ppsDeviceID[d],CL_DRIVER_VERSION,512,aszString,NULL);
		CheckAndReportError(psInstance,"clGetDeviceInfo",eResult,verify_device_cleanup);
		OCLTestLog("%s: CL_DRIVER_VERSION    %s\n",__func__,aszString);

		/* CL_DEVICE_PROFILE */
		eResult = clGetDeviceInfo(ppsDeviceID[d],CL_DEVICE_PROFILE,512,aszString,NULL);
		CheckAndReportError(psInstance,"clGetDeviceInfo",eResult,verify_device_cleanup);
		OCLTestLog("%s: CL_DEVICE_PROFILE    %s\n",__func__,aszString);

		/* CL_DEVICE_VERSION */
		eResult = clGetDeviceInfo(ppsDeviceID[d],CL_DEVICE_VERSION,512,aszString,NULL);
		CheckAndReportError(psInstance,"clGetDeviceInfo",eResult,verify_device_cleanup);
		OCLTestLog("%s: CL_DEVICE_VERSION    %s\n",__func__,aszString);

		/* CL_DEVICE_EXTENSIONS */
		size_t config_size_set;
		size_t config_size_ret;
		eResult = clGetDeviceInfo(ppsDeviceID[d], CL_DEVICE_EXTENSIONS, 0, NULL, &config_size_set);
		CheckAndReportError(psInstance,"clGetDeviceInfo",eResult,verify_device_cleanup);

		if (config_size_set > 0)
		{
			pszPlatformInfo = (char*)malloc(config_size_set);
			if (!pszPlatformInfo)
			{
				eResult = CL_OUT_OF_RESOURCES;
				goto verify_device_cleanup;
			}
			eResult = clGetDeviceInfo(ppsDeviceID[d], CL_DEVICE_EXTENSIONS, config_size_set, pszPlatformInfo, &config_size_ret);
			CheckAndReportError(psInstance,"clGetDeviceInfo",eResult,verify_device_cleanup);
			OCLTestLog("%s: CL_DEVICE_EXTENSIONS %s\n", __func__, pszPlatformInfo);
			free(pszPlatformInfo);
			pszPlatformInfo = NULL;
		}

		{
			size_t uiSize;
			eResult = clGetDeviceInfo(ppsDeviceID[d],CL_DEVICE_MAX_WORK_GROUP_SIZE,sizeof(uiSize),&uiSize,NULL);
			CheckAndReportError(psInstance,"clGetDeviceInfo",eResult,verify_device_cleanup);
			OCLTestLog("%s: CL_DEVICE_MAX_WORK_GROUP_SIZE %zu\n",__func__,uiSize);
		}

		{
			cl_uint uiSize;
			eResult = clGetDeviceInfo(ppsDeviceID[d],CL_DEVICE_MIN_DATA_TYPE_ALIGN_SIZE ,sizeof(uiSize),&uiSize,NULL);
			CheckAndReportError(psInstance,"clGetDeviceInfo",eResult,verify_device_cleanup);
			OCLTestLog("%s: CL_DEVICE_MIN_DATA_TYPE_ALIGN_SIZE %d\n",__func__,uiSize);
		}

		{
			cl_uint uiSize;
			eResult = clGetDeviceInfo(ppsDeviceID[d],CL_DEVICE_MEM_BASE_ADDR_ALIGN ,sizeof(uiSize),&uiSize,NULL);
			CheckAndReportError(psInstance,"clGetDeviceInfo",eResult,verify_device_cleanup);
			OCLTestLog("%s: CL_DEVICE_MEM_BASE_ADDR_ALIGN %u\n",__func__,uiSize);
		}

		{
			cl_ulong ulSize;
			eResult = clGetDeviceInfo(ppsDeviceID[d],CL_DEVICE_LOCAL_MEM_SIZE ,sizeof(ulSize),&ulSize,NULL);
			CheckAndReportError(psInstance,"clGetDeviceInfo",eResult,verify_device_cleanup);
			OCLTestLog("%s: CL_DEVICE_LOCAL_MEM_SIZE %" PRIu64 "\n",__func__,ulSize);
		}
	}

	free(psInstance->pvPrivateData);
	return eResult;

verify_device_cleanup:
	free(pszPlatformInfo);
	return eResult;
}

/******************************************************************************
 End of file (device.c)
******************************************************************************/
