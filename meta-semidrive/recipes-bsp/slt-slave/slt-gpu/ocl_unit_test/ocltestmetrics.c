/*************************************************************************/ /*!
@File           ocltestmetrics.c
@Copyright      Copyright (c) Imagination Technologies Ltd. All Rights Reserved
@License        Strictly Confidential.
*/ /**************************************************************************/

#include "ocltestmetrics.h"
#include "img_defs.h"
#include <stdarg.h>
#include <stdio.h>

static void
OCLTestLogMetric(char *pszFormat, ...) __printf(1,2);

static void
OCLTestLogMetric(char *pszFormat, ...)
{
	FILE *fpOutputLog;
	char aszBuffer[512];
	va_list vaList;

	/* Process arguments */
	va_start(vaList,pszFormat);

	/* Format output */
	vsnprintf(aszBuffer,512,pszFormat,vaList);

	fpOutputLog = fopen("oclunittestmetric.log","a");

	if(fpOutputLog)
	{
		fprintf(fpOutputLog,"%s",aszBuffer);
		fclose(fpOutputLog);
	}

	va_end(vaList);
}

static void _OCLMetricOutput(const char * testName, const char * subTestName, const char * subsubTestName, double dmetric, unsigned long ulmetric, enum metricUnit unit, int isDouble)
{
	char * outputStringD = "%s,%s%s,%f,%s\n";
	char * outputStringUL = "%s,%s%s,%lu,%s\n";
	if(isDouble)
	{
		OCLTestLogMetric(outputStringD,testName,subTestName,subsubTestName,dmetric,metricUnitStr[unit]);
	} else
	{
		OCLTestLogMetric(outputStringUL,testName,subTestName,subsubTestName,ulmetric,metricUnitStr[unit]);
	}
}


static void OCLMetricOutputDouble(const char * testName, const char * subTestName, double metric, enum metricUnit unit)
{
	  /* The double quote "" is to conform with the method signature of _OCLMetricOutput */
	_OCLMetricOutput(testName,subTestName,"",metric,0,unit,1);
}

static void OCLMetricOutputDouble2(const char * testName, const char * subTestName, const char * subsubTestName, double metric, enum metricUnit unit)
{
	_OCLMetricOutput(testName,subTestName,subsubTestName,metric,0,unit,1);
}

/******************************************************************************
 End of file (ocltestmetrics.c)
******************************************************************************/
