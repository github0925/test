/*************************************************************************/ /*!
@File           devicetransfer.c
@Copyright      Copyright (c) Imagination Technologies Ltd. All Rights Reserved
@License        Strictly Confidential.
*/ /**************************************************************************/
#define TEST_TRANSFER_NUMBER            16
#ifdef CUT_DOWN_UNIT_TEST
#define TEST_TRANSFER_SIZE_IN_MEGABYTES 4
#else
#define TEST_TRANSFER_SIZE_IN_MEGABYTES 16
#endif
#define TEST_TRANSFER_SIZE_IN_BYTES     (TEST_TRANSFER_SIZE_IN_MEGABYTES * /*1MB*/1048576)

typedef struct _TransferData_
{
	/* CL Objects */
	cl_context       psContext;
	cl_device_id     psDeviceID;
	cl_platform_id   psPlatformID;
	cl_command_queue psCommandQueue;
	cl_mem           psBufferA;
	cl_mem           psBufferB;
	cl_event         psWriteEvent[TEST_TRANSFER_NUMBER];
	cl_event         psCopyEvent[TEST_TRANSFER_NUMBER];
	cl_event         psReadEvent[TEST_TRANSFER_NUMBER];

	/* Host objects */
	char   *pcTemporary;
	char   *pcTemporaryDown;
	size_t  auWriteOffset[TEST_TRANSFER_NUMBER];
	size_t  auCopyOffset[TEST_TRANSFER_NUMBER];
	size_t  auReadOffset[TEST_TRANSFER_NUMBER];
	unsigned long   ulTime[TEST_TRANSFER_NUMBER][3];

} TransferData;

cl_int Init_Transfer(OCLTestInstance *psInstance);
cl_int Compute_Transfer(OCLTestInstance *psInstance);
cl_int Verify_Transfer(OCLTestInstance *psInstance);

/***********************************************************************************
 Function Name      : Init_Transfer
 Inputs             : None
 Outputs            : None
 Returns            : None
 Description        : Initialises input data
************************************************************************************/
cl_int Init_Transfer(OCLTestInstance *psInstance)
{
	TransferData *psData = (TransferData*)calloc(1, sizeof(TransferData));
	cl_int        eResult;
	unsigned int i;

	/* Use a seed of zero so we get the same random numbers each time we run the test */
	srand(0);

	if(!psData)
	{
		return CL_OUT_OF_RESOURCES;
	}

	eResult = clGetPlatformIDs(1,&psData->psPlatformID,NULL);
	CheckAndReportError(psInstance, "clGetPlatformIDs", eResult, init_transfer_cleanup);

	eResult = clGetDeviceIDs(psData->psPlatformID,CL_DEVICE_TYPE_ALL,1,&psData->psDeviceID,NULL);
	CheckAndReportError(psInstance, "clGetDeviceIDs", eResult, init_transfer_cleanup);

	psData->psContext = clCreateContext(NULL,1,&psData->psDeviceID,NULL,NULL,&eResult);
	CheckAndReportError(psInstance, "clCreateContext", eResult, init_transfer_cleanup);

	psData->psCommandQueue = clCreateCommandQueue(psData->psContext, psData->psDeviceID, CL_QUEUE_PROFILING_ENABLE, &eResult);
	CheckAndReportError(psInstance, "clCreateCommandQueue", eResult, init_transfer_cleanup);

	/* Upload */
	psData->pcTemporary = malloc(TEST_TRANSFER_SIZE_IN_BYTES);
	/* Download */
	psData->pcTemporaryDown = malloc(TEST_TRANSFER_SIZE_IN_BYTES);

	if(!psData->pcTemporary || !psData->pcTemporaryDown)
	{
		OCLTestLog("%s:%s:%d: failed to alloc host data\n",
			psInstance->pszTestID,
			__func__,__LINE__);
		eResult = CL_OUT_OF_HOST_MEMORY;
		goto init_transfer_cleanup;
	}

	for(i=0; i < TEST_TRANSFER_SIZE_IN_BYTES; i++)
	{
		psData->pcTemporary[i] = ((char)rand());
		psData->pcTemporaryDown[i] = 0;
	}

	psData->psBufferA = clCreateBuffer(psData->psContext, 0, TEST_TRANSFER_SIZE_IN_BYTES, NULL, &eResult);
	CheckAndReportError(psInstance, "clCreateBuffer", eResult, init_transfer_cleanup);

	psData->psBufferB = clCreateBuffer(psData->psContext, 0, TEST_TRANSFER_SIZE_IN_BYTES, NULL, &eResult);
	CheckAndReportError(psInstance, "clCreateBuffer", eResult, init_transfer_cleanup);

	/* Initialise data */
	psInstance->pvPrivateData = (void*)psData;
	return CL_SUCCESS;

init_transfer_cleanup:
	free(psData->pcTemporary);
	free(psData->pcTemporaryDown);
	free(psData);
	return eResult;
}

/***********************************************************************************
 Function Name      : Compute_Transfer
 Inputs             : psInstance - test instance data
 Returns            : 0 or 1
 Description        : Main compute func
************************************************************************************/
cl_int Compute_Transfer(OCLTestInstance *psInstance)
{
	unsigned int i;
	cl_int        eResult;
	TransferData *psData = (TransferData*) psInstance->pvPrivateData;
	float fUnitRand;
	float fSpace;

	OCLTestLog("%s: Starting transfer operations ... ",__func__);
	fflush(stdout);

	for(i=0; i < TEST_TRANSFER_NUMBER; i++)
	{
		unsigned int uiTransferSizeB  = TEST_TRANSFER_SIZE_IN_BYTES     / (TEST_TRANSFER_NUMBER - i);
		float fTransferSizeMB = (float)(TEST_TRANSFER_SIZE_IN_MEGABYTES) / (float)(TEST_TRANSFER_NUMBER - i);
		unsigned int uOffset          = 0;
		unsigned long ulStart;

		OCLTestLog(", %fMB", fTransferSizeMB );
		fflush(stdout);

		/* Try and randomize the offset so it's not always the same data being copied */
		fUnitRand = ((float)(rand() % 1000)) / 1000.0f;
		fSpace    = (float)uiTransferSizeB / (float)TEST_TRANSFER_SIZE_IN_BYTES;
		fSpace    = 1.0f - fSpace;

		/*        [0,1]       * allowed movement  * segment size  */
		uOffset = fUnitRand   * fSpace            * uiTransferSizeB;
		psData->auWriteOffset[i] = uOffset;

		/*Reset the timer*/
		ulStart = OCLGetTime();

		/* Upload */
		eResult = clEnqueueWriteBuffer(psData->psCommandQueue,
					       psData->psBufferA,
					       CL_TRUE,
					       uOffset,
					       uiTransferSizeB,
					       psData->pcTemporary,
					       0,
					       NULL,
					       &(psData->psWriteEvent[i]));
		CheckAndReportError(psInstance, "clEnqueueWriteBuffer", eResult, compute_transfer_cleanup);

		eResult = clFinish(psData->psCommandQueue);

		/* Timing */
		psData->ulTime[i][0] = OCLGetTime() - ulStart;

		CheckAndReportError(psInstance, "clFinish", eResult, compute_transfer_cleanup);

		/* Try and randomize the offset so it's not always the same data being copied */
		fUnitRand = ((float)(rand() % 1000)) / 1000.0f;
		fSpace    = (float)uiTransferSizeB / (float)TEST_TRANSFER_SIZE_IN_BYTES;
		fSpace    = 1.0f - fSpace;

		/*        [0,1]       * allowed movement  * segment size  */
		uOffset = fUnitRand   * fSpace            * uiTransferSizeB;
		psData->auCopyOffset[i] = uOffset;

		/*Reset the timer*/
		ulStart = OCLGetTime();

		/* Transfer */
		eResult = clEnqueueCopyBuffer(psData->psCommandQueue,
					      psData->psBufferA,
					      psData->psBufferB,
					      uOffset,
					      0,
					      uiTransferSizeB,
					      0,
					      NULL,
					      &(psData->psCopyEvent[i]));
		CheckAndReportError(psInstance, "clEnqueueCopyBuffer", eResult, compute_transfer_cleanup);

		eResult = clFinish(psData->psCommandQueue);

		/* Timing */
		psData->ulTime[i][1] = OCLGetTime() - ulStart;

		CheckAndReportError(psInstance, "clFinish", eResult, compute_transfer_cleanup);

		/* Try and randomize the offset so it's not always the same data being copied */
		fUnitRand = ((float)(rand() % 1000)) / 1000.0f;
		fSpace    = (float)uiTransferSizeB / (float)TEST_TRANSFER_SIZE_IN_BYTES;
		fSpace    = 1.0f - fSpace;

		/*        [0,1]       * allowed movement  * segment size  */
		uOffset = fUnitRand   * fSpace            * uiTransferSizeB;
		psData->auReadOffset[i] = uOffset;

		/*Reset the timer*/
		ulStart = OCLGetTime();

		/* Download */
		eResult = clEnqueueReadBuffer(psData->psCommandQueue,
					      psData->psBufferB,
					      CL_TRUE,
					      uOffset,
					      uiTransferSizeB,
					      psData->pcTemporaryDown,
					      0,
					      NULL,
					      &(psData->psReadEvent[i]));
		CheckAndReportError(psInstance, "clEnqueueReadBuffer", eResult, compute_transfer_cleanup);

		eResult = clFinish(psData->psCommandQueue);

		/* Timing */
		psData->ulTime[i][2] = OCLGetTime() - ulStart;

		CheckAndReportError(psInstance, "clFinish", eResult, compute_transfer_cleanup);
	}

	OCLTestLog("\n%s: Transfer tests complete.\n",__func__);
	fflush(stdout);

	return CL_SUCCESS;

compute_transfer_cleanup:
	return eResult;
}

/***********************************************************************************
 Function Name      : Verify_Transfer
 Inputs             : None
 Outputs            : None
 Returns            : None
 Description        : Verifies output data
************************************************************************************/
cl_int Verify_Transfer(OCLTestInstance *psInstance)
{
	unsigned int  i;
	unsigned int  j;
	unsigned int flag;
	cl_int        eResult = CL_SUCCESS;
	float         fTimeS;
	float         fTimeMBS;
	float         fAverageUpload   = 0.0f;
	float         fAverageCopy     = 0.0f;
	float         fAverageDownload = 0.0f;
	TransferData *psData           = (TransferData*) psInstance->pvPrivateData;

	char*  apszName[3]    = { "Upload", "Copy", "Download" };
	size_t *apuOffsets[3] = { psData->auWriteOffset, psData->auCopyOffset, psData->auReadOffset };
	float  *afAverage[3]  = { &fAverageUpload, &fAverageCopy, &fAverageDownload };

	/* The Probably Correct Output */
	OCLTestLog("Actual speed\n");
	OCLTestLog("%s: Transfer Type | Size (MB) |  Offset | Time (s) | MB/s\n",__func__);

	for(i=0; i < TEST_TRANSFER_NUMBER; i++)
	{
		float fTransferSizeMB = TEST_TRANSFER_SIZE_IN_MEGABYTES / (TEST_TRANSFER_NUMBER - i);

		for(j=0; j < 3; j++)
		{
			fTimeS = ((float)psData->ulTime[i][j])*0.001;
			fTimeMBS = fTransferSizeMB / fTimeS;

			OCLTestLog("%s: %-13s | %9f | %07zx | %6.6f | %6.2f\n",__func__,apszName[j],fTransferSizeMB,apuOffsets[j][i],fTimeS,fTimeMBS);

			*afAverage[j] += fTimeMBS;
		}
	}

	fAverageUpload   /= (float)TEST_TRANSFER_NUMBER;
	fAverageCopy     /= (float)TEST_TRANSFER_NUMBER;
	fAverageDownload /= (float)TEST_TRANSFER_NUMBER;

	OCLTestLog("%s: Average upload speed %.2f MB/s\n",__func__,fAverageUpload);
	OCLTestLog("%s: Average copy speed %.2f MB/s\n",__func__,fAverageCopy);
	OCLTestLog("%s: Average download speed %.2f MB/s\n",__func__,fAverageDownload);
	OCLMetricOutputDouble(__func__,"Upload",fAverageUpload,MBSEC);
	OCLMetricOutputDouble(__func__,"Copy",fAverageCopy,MBSEC);
	OCLMetricOutputDouble(__func__,"Download",fAverageDownload,MBSEC);
	*afAverage[0] = *afAverage[1] = *afAverage[2] = 0.0f;

	OCLTestLog("\n%s: Verifying data integrity\n",__func__);
	/*Verify data integrity*/
	flag = 0;
	for(i=0; i < TEST_TRANSFER_SIZE_IN_BYTES; i++)
	{
		if(psData->pcTemporaryDown[i] != psData->pcTemporary[i])
		{
			flag = 1;
			break;
		}
	}

	if(flag)
	{
		OCLTestLog("%s: Data integrity NOT ok starting at byte %d\n Expected %d actual %d\n",__func__,i,psData->pcTemporary[i],psData->pcTemporaryDown[i]);
		eResult = CL_MEM_COPY_OVERLAP;

	} else {
		OCLTestLog("%s: Data integrity OK\n",__func__);
	}

	/* Free host memory */
	free(psData->pcTemporary);
	free(psData->pcTemporaryDown);
	/* Free all events */
	for(i=0; i < TEST_TRANSFER_NUMBER; i++)
	{
		clReleaseEvent(psData->psWriteEvent[i]);
		clReleaseEvent(psData->psReadEvent[i]);
		clReleaseEvent(psData->psCopyEvent[i]);
	}

	/* Free CL objects */
	clReleaseMemObject   ( psData->psBufferA      );
	clReleaseMemObject   ( psData->psBufferB      );
	clReleaseCommandQueue( psData->psCommandQueue );
	clReleaseContext     ( psData->psContext      );

	free(psInstance->pvPrivateData);
	psInstance->pvPrivateData = NULL;

	return eResult;
}

/******************************************************************************
 End of file (devicetransfer.c)
******************************************************************************/
