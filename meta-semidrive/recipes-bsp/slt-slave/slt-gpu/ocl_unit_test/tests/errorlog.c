/*************************************************************************/ /*!
@File           errorlog.c
@Copyright      Copyright (c) Imagination Technologies Ltd. All Rights Reserved
@License        Strictly Confidential.
*/ /**************************************************************************/
typedef struct _ErrorlogData_
{
	/* CL Objects */
	cl_context       psContext;
	cl_program       psProgram;
	cl_device_id     psDeviceID;
	cl_platform_id   psPlatformID;
} ErrorlogData;

cl_int Init_Errorlog(OCLTestInstance *psInstance);
cl_int Compute_Errorlog(OCLTestInstance *psInstance);
cl_int Verify_Errorlog(OCLTestInstance *psInstance);

static char *g_pszErrorSource =
{
	"__kernel void AdditionKernel(__global int* a, __global int* b)\n"
	"{\n"
	"\tundeclared identifier\n"
	"}"
};

/***********************************************************************************
 Function Name      : Init_Errorlog
 Inputs             : None
 Outputs            : None
 Returns            : None
 Description        : Initialises input data
************************************************************************************/
cl_int Init_Errorlog(OCLTestInstance *psInstance)
{
	cl_int        eResult;
	ErrorlogData *psData = (ErrorlogData*)calloc(1, sizeof(ErrorlogData));

	if(!psData)
	{
		return CL_OUT_OF_RESOURCES;
	}

	/* Initialise data */
	psInstance->pvPrivateData = (void*)psData;

	eResult = clGetPlatformIDs(1,&psData->psPlatformID,NULL);
	CheckAndReportError(psInstance, "clGetPlatformIDs", eResult, init_errorlog_cleanup);

	eResult = clGetDeviceIDs(psData->psPlatformID,CL_DEVICE_TYPE_GPU,1,&psData->psDeviceID,NULL);
	CheckAndReportError(psInstance, "clGetDeviceIDs", eResult, init_errorlog_cleanup);

	psData->psContext = clCreateContext(NULL,1,&psData->psDeviceID,NULL,NULL,&eResult);
	CheckAndReportError(psInstance, "clCreateContext", eResult, init_errorlog_cleanup);

	return CL_SUCCESS;

init_errorlog_cleanup:
	return eResult;
}

/***********************************************************************************
 Function Name      : Verify_Errorlog
 Inputs             : None
 Outputs            : None
 Returns            : None
 Description        : Verifies output data
************************************************************************************/
cl_int Verify_Errorlog(OCLTestInstance *psInstance)
{
	ErrorlogData *psData = (ErrorlogData*) psInstance->pvPrivateData;
	size_t auLengths[2]  = {0,0};
	char *ppszSource[2]  = { 0 };
	cl_int        eResult = CL_SUCCESS;
	char         aszBuildLog[512];
	size_t       uBuildLogSize;

	/* Use the global program */
	auLengths[0]  = strlen(g_pszErrorSource);
	ppszSource[0] = g_pszErrorSource;

	psData->psProgram = clCreateProgramWithSource(psData->psContext, 1, (const char**)ppszSource, auLengths, &eResult);
	CheckAndReportError(psInstance, "clCreateProgramWithSource", eResult, verify_errorlog_cleanup);

	eResult = clBuildProgram(psData->psProgram,1,&psData->psDeviceID,"",NULL,NULL);

	if(eResult != CL_BUILD_PROGRAM_FAILURE)
	{
		free(psInstance->pvPrivateData);
		psInstance->pvPrivateData = strdup("Test succeeded in compiling an error-riddled shader!");
		return CL_INVALID_VALUE;
	}

	eResult = clGetProgramBuildInfo(psData->psProgram, psData->psDeviceID, CL_PROGRAM_BUILD_LOG, 512, aszBuildLog, &uBuildLogSize);
	CheckAndReportError(psInstance, "clGetProgramBuildInfo", eResult, verify_errorlog_cleanup);
	aszBuildLog[uBuildLogSize-1] = '\0';

	OCLTestLog("%s:\n"
	       "*** Build Log ***\n"
	       "%s",
	       psInstance->pszTestID,
	       aszBuildLog);

	if(strstr(aszBuildLog,"error") &&
	   strstr(aszBuildLog,"undeclared")  &&
	   strstr(aszBuildLog,"identifier"))
	{
		OCLTestLog("%s: Verified build log contained required error.\n",psInstance->pszTestID);
	}
	else
	{
		free(psInstance->pvPrivateData);
		psInstance->pvPrivateData = strdup("Test failed to verify the build log contained our error string.");
		return CL_INVALID_VALUE;
	}

	clReleaseProgram(psData->psProgram);
	clReleaseContext(psData->psContext);

	free(psInstance->pvPrivateData);
	return eResult;

verify_errorlog_cleanup:
	return eResult;
}

/******************************************************************************
 End of file (errorlog.c)
******************************************************************************/
