/*************************************************************************/ /*!
@File           ocltestmetrics.h
@Copyright      Copyright (c) Imagination Technologies Ltd. All Rights Reserved
@License        Strictly Confidential.
*/ /**************************************************************************/

#ifndef _OCLTESTMETRICS_H_
#define _OCLTESTMETRICS_H_


/***********************************************************************************
* Enum for the different metric units available
************************************************************************************/

enum metricUnit {
	MS = 0,
	SEC,
	MIN,
	TICK,
	BYTE,
	MBYTE,
	GBYTE,
	GFLOP,
	GIOP,
	BYTESEC,
	MBSEC,
	GBSEC,
	GFLOPSEC,
	GIOPSEC
};

/***********************************************************************************
* The string which should be outputed for each different metric unit
************************************************************************************/

static const char * metricUnitStr[] =
{
	"mS",
	"S",
	"M",
	"Ticks",
	"Bytes",
	"MBytes",
	"GBytes",
	"GFLOPs",
	"GIOP",
	"B/S",
	"MB/S",
	"GB/S",
	"GFLOP/S",
	"GIOP/S"
};

/***********************************************************************************
* The functions to use to record the metric
* OCLMetricOutputDouble2 is used when you have a subsub test in order to tell the
* difference between the two. E.g. Verify_Float Float Add is run in three modes
* Normal, Parallel and Chain, uses *Double2 to append Chain and Parallel to the
* Sub Test name
************************************************************************************/

static void OCLMetricOutputDouble(const char * testName, const char * subTestName, double metric, enum metricUnit unit);

static void OCLMetricOutputDouble2(const char * testName, const char * subTestName, const char * subsubTestName, double metric, enum metricUnit unit);

#endif
/******************************************************************************
* End of file (ocltestmetrics.h)
******************************************************************************/
