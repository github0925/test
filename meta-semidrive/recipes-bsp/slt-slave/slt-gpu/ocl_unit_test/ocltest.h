/*************************************************************************/ /*!
@File           ocltest.h
@Copyright      Copyright (c) Imagination Technologies Ltd. All Rights Reserved
@License        Strictly Confidential.
*/ /**************************************************************************/

#ifndef _OCLTEST_H_
#define _OCLTEST_H_

#include <CL/opencl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <time.h>
#include "img_defs.h"

/***********************************************************************************
* Test definitions
************************************************************************************/
#define TEST_VENDOR_STRING "Imagination Technologies"

/***********************************************************************************
* Test instance struct
************************************************************************************/
typedef struct _OCLTestInstance_
{
	char *pszTestID;
	char *pszTestDesc;
	cl_int (*pfnInit)    (struct _OCLTestInstance_ *);
	cl_int (*pfnCompute) (struct _OCLTestInstance_ *);
	cl_int (*pfnVerify)  (struct _OCLTestInstance_ *);

	void *pvPrivateData;
} OCLTestInstance;

/***********************************************************************************
* Util functions
************************************************************************************/
const char*    OCLGetErrorStr(cl_int eError);
const char*    OCLGetDeviceTypeStr(cl_device_type eDeviceType);
unsigned char *OCLLoadBinary(char *pszBinaryName, size_t *puLength);
int            OCLOutputBinariesToFile(cl_program psProgram, char *pszPrefix, unsigned int *pui32NumBinaries);
void           PrintBanner(void);
unsigned long  OCLGetTime(void);
void           OCLTestLog(char *pszFormat, ...) __printf(1,2);

#define CheckAndReportError(INSTANCE,FUNC,RESULT,CLEANUP) \
	if(RESULT != CL_SUCCESS) \
	{ \
		printf("%s:%s:%d: call to %s aborted with error %s (%08x)\n", \
		        INSTANCE->pszTestID,__func__,__LINE__,FUNC,OCLGetErrorStr(RESULT),RESULT); \
		goto CLEANUP; \
	} \

#define UNREFD(X) X = X

#include "ocltestmetrics.h"
#include "ocltestmetrics.c"

/***********************************************************************************
* Test code includes
************************************************************************************/

#include "tests/platform.c"
#include "tests/device.c"
#include "tests/bbox.c"
#include "tests/memcpy.c"
#include "tests/memcpy_stride.c"
#include "tests/memread.c"
#include "tests/memread_stride.c"
#include "tests/memwrite.c"
#include "tests/memwrite_stride.c"
#include "tests/imgcpy.c"
#include "tests/binary.c"
#include "tests/add.c"
#include "tests/float.c"
#include "tests/floatvec.c"
#include "tests/halfvec.c"
#include "tests/int.c"
#include "tests/int_mod.c"
#include "tests/short.c"
#include "tests/short_mod.c"
#include "tests/char.c"
#include "tests/errorlog.c"
#include "tests/devicetransfer.c"
#include "tests/memcpy_workgroups.c"
#include "tests/mulevt.c"
#include "tests/conversions.c"
#include "tests/memcpy_global_offsets.c"
#include "tests/images.c"
#include "tests/atomics.c"
#include "tests/nop.c"
#include "tests/enqueue_native_kernel.c"
#include "tests/spirv_add.c"

/* We do not support mipmap under 1.2 */
#include "tests/mipmap.c"
#include "tests/async.c"

/***********************************************************************************
* Test descriptors
************************************************************************************/
static OCLTestInstance g_TestList[] =
{
	{ "platform",
	  /****************************************************************/
	  "Platform Test:\n"
	  "\tChecks that an OpenCL compatible platform is present\n"
	  "\tfor the unit test to run.",
	  /****************************************************************/
	  NULL, NULL, Verify_Platform, NULL
	},
	{ "device",
	  /****************************************************************/
	  "Device Test:\n"
	  "\tChecks that an OpenCL compatible device is present\n"
	  "\tfor the unit test to run.",
	  /****************************************************************/
	  Init_Device, NULL, Verify_Device, NULL
	},
	{ "bbox",
	  /****************************************************************/
	  "Bounding Box:\n"
	  "\tRuns the kernel that compute floating point minimum and maximum\n"
	  "\tfor large number of vertices.\n",
	  /****************************************************************/
	  Init_BBoxKernel, NULL, Verify_BBoxKernel, NULL
	},
	{ "add",
	  /****************************************************************/
	  "Addition Kernel:\n"
	  "\tPerforms an online compilation of an integer addition kernel\n"
	  "\tand verifies that the output buffer is correct.",
	  /****************************************************************/
	  Init_Add, Compute_Add, Verify_Add, NULL
	},
	{ "binary",
	  /****************************************************************/
	  "Binary Check:\n"
	  "\tRuns the same kernel as the addition test however first saves\n"
	  "\tthe binary version to the filsystem, recreates the OpenCL\n"
	  "\tcontext and ensures the binary test computes the same results\n"
	  "\tas the online test.",
	  /****************************************************************/
	  Init_Binary, Compute_Binary, Verify_Binary, NULL
	},
	{ "errorlog",
	  /****************************************************************/
	  "Error Log Check:\n"
	  "\tRuns an illegal kernel that contains a simple undeclared\n"
	  "\tidentifier error and verifies that the build log provided\n"
	  "\tby the OpenCL implementation is correct.",
	  /****************************************************************/
	  Init_Errorlog, NULL, Verify_Errorlog, NULL
	},
	{ "memcpy",
	  /****************************************************************/
	  "Memory Copy Kernel:\n"
	  "\tPerforms an online compilation of a kernel which copies input\n"
	  "\tto output, verifying the results and calculating the speed at\n"
	  "\twhich the data is transferred.",
	  /****************************************************************/
	  Init_MemCopyKernel, NULL, Verify_MemCopyKernel, NULL
	},
	{ "memcpy_stride",
	  /****************************************************************/
	  "Memory Strided Copy Kernel:\n"
	  "\tPerforms an online compilation of a kernel which copies input\n"
	  "\tto output, verifying the results and calculating the speed at\n"
	  "\twhich the data is transferred using strided pattern.",
	  /****************************************************************/
	  Init_MemStrideCopyKernel, NULL, Verify_MemStrideCopyKernel, NULL
	},
	{ "memread",
	  /****************************************************************/
	  "Memory Read Kernel:\n"
	  "\tPerforms an online compilation of a kernel which reads large\n"
	  "\tamounts of data each instance, calculating read bandwidth.",
	  /****************************************************************/
	  Init_MemReadKernel, NULL, Verify_MemReadKernel, NULL
	},
	{ "memread_stride",
	  /****************************************************************/
	  "Memory Strided Read Kernel:\n"
	  "\tPerforms an online compilation of a kernel which reads large\n"
	  "\tamounts of data each instance using stride pattern,\n"
	  "\tcalculating read bandwidth.",
	  /****************************************************************/
	  Init_MemStrideReadKernel, NULL, Verify_MemStrideReadKernel, NULL
	},
	{ "memwrite",
	  /****************************************************************/
	  "Memory Write Kernel:\n"
	  "\tPerforms an online compilation of a kernel which writes large\n"
	  "\tamounts of data each instance, calculating write bandwidth.",
	  /****************************************************************/
	  Init_MemWriteKernel, NULL, Verify_MemWriteKernel, NULL
	},
	{ "memwrite_stride",
	  /****************************************************************/
	  "Memory Strided Write Kernel:\n"
	  "\tPerforms an online compilation of a kernel which writes large\n"
	  "\tamounts of data each instance using stride pattern,\n"
	  "\tcalculating write bandwidth.",
	  /****************************************************************/
	  Init_MemStrideWriteKernel, NULL, Verify_MemStrideWriteKernel, NULL
	},
	{ "imgcpy",
	  /****************************************************************/
	  "Image Copy Kernel:\n"
	  "\tPerforms an image direct copy.",
	  /****************************************************************/
	  Init_ImgCopyKernel, NULL, Verify_ImgCopyKernel, NULL
	},
	{ "mipmap",
	  /****************************************************************/
	  "Image mipmaps:\n"
	  "\tPerforms an image direct copy with mipmaps.",
	  /****************************************************************/
	  Init_MipMap, NULL, Verify_MipMap, NULL
	},
	{ "float",
	  /****************************************************************/
	  "Floating Point Operations Kernel (Scalar):\n"
	  "\tPerforms an online compilation of floating point add/mul/mad/div\n"
	  "\tkernels which each perform a large number of one specific operation,\n"
	  "\tand calculates the floating point operations per second of the\n"
	  "\tdevice. This test uses (scalar) float as its unit of computation.",
	  /****************************************************************/
	  Init_Float, Compute_Float, Verify_Float, NULL
	},
	{ "float_parallel",
	  /****************************************************************/
	  "Floating Point Operations Kernel (Scalar):\n"
	  "\tPerforms an online compilation of floating point add/mul/mad/div\n"
	  "\tkernels which each perform a large number of one specific operation,\n"
	  "\tand calculates the floating point operations per second of the\n"
	  "\tdevice. This test uses (scalar) float as its unit of computation.\n"
	  "\tKernels run in parallel without synchronization on host side.",
	  /****************************************************************/
	  Init_Float, Compute_Float_Parallel, Verify_Float, NULL
	},
	{ "float_chain",
	  /****************************************************************/
	  "Floating Point Operations Kernel (Scalar):\n"
	  "\tPerforms an online compilation of floating point add/mul/mad/div\n"
	  "\tkernels which each perform a large number of one specific operation,\n"
	  "\tand calculates the floating point operations per second of the\n"
	  "\tdevice. This test uses (scalar) float as its unit of computation.\n"
	  "\tKernels run in order of their dependencies created by events.",
	  /****************************************************************/
	  Init_Float, Compute_Float_Chain, Verify_Float, NULL
	},
	{ "floatvec",
	  /****************************************************************/
	  "Floating Point Operations Kernels for all vector sizes:\n"
	  "\tPerforms an online compilation of floating point add/mul/mad/div\n"
	  "\tkernels which each perform a large number of one specific operation,\n"
	  "\tand calculates the floating point operations per second of the\n"
	  "\tdevice. This test uses float{1,2,3,4,8,16} as its unit of computation.",
	  /****************************************************************/
	  Init_Floatops, Compute_Floatops, Verify_Floatops, NULL
	},
	{ "halfvec",
		/****************************************************************/
		"16 bit floating Point Operations Kernels for all vector sizes:\n"
		"\tPerforms an online compilation of 16 bit floating point add/mul/mad/div\n"
		"\tkernels which each perform a large number of one specific operation,\n"
		"\tand calculates the 16 bit floating point operations per second of the\n"
		"\tdevice. This test uses half{1,2,3,4,8,16} as its unit of computation.",
		/****************************************************************/
		Init_Halfops, Compute_Halfops, Verify_Halfops, NULL
	},
	{ "int",
	  /****************************************************************/
	  "Integer Operations Kernels:\n"
	  "\tPerforms an online compilation of integer add/mul/mad/div\n"
	  "\tkernels which each perform a large number of one specific\n"
	  "\toperation, and calculates the operations per second of the\n"
	  "\tdevice. This test uses int{1,2,3,4} as its unit of computation.",
	  /****************************************************************/
	  Init_Int, Compute_Int, Verify_Int, NULL
	},
	{ "intmod",
	  /****************************************************************/
	  "Integer Operations Kernels:\n"
	  "\tPerforms an online compilation of integer modulo\n"
	  "\tkernels which each perform a large number of one specific\n"
	  "\toperation, and calculates the operations per second of the\n"
	  "\tdevice. This test uses int{1,2,3,4} as its unit of computation.",
	  /****************************************************************/
	  Init_IntMod, Compute_IntMod, Verify_IntMod, NULL
	},
	{ "short",
	  /****************************************************************/
	  "Short Operations Kernels:\n"
	  "\tPerforms an online compilation of short add/mul/mad/div\n"
	  "\tkernels which each perform a large number of one specific\n"
	  "\toperation, and calculates the operations per second of the\n"
	  "\tdevice. This test uses short{1,2,3,4} as its unit of computation.",
	  /****************************************************************/
	  Init_Short, Compute_Short, Verify_Short, NULL
	},
	{ "shortmod",
	  /****************************************************************/
	  "Short Operations Kernels:\n"
	  "\tPerforms an online compilation of short modulo\n"
	  "\tkernels which each perform a large number of one specific\n"
	  "\toperation, and calculates the operations per second of the\n"
	  "\tdevice. This test uses short{1,2,3,4} as its unit of computation.",
	  /****************************************************************/
	  Init_ShortMod, Compute_ShortMod, Verify_ShortMod, NULL
	},
	{ "char",
	  /****************************************************************/
	  "Char Operations Kernels:\n"
	  "\tPerforms an online compilation of char add/mul/mad/div\n"
	  "\tkernels which each perform a large number of one specific\n"
	  "\toperation, and calculates the operations per second of the\n"
	  "\tdevice. This test uses char{1,2,3,4} as its unit of computation.",
	  /****************************************************************/
	  Init_Char, Compute_Char, Verify_Char, NULL
	},
	{ "transfer",
	  /****************************************************************/
	  "Device Transfer Test:\n"
	  "\tPerforms purely transfer operations using an OpenCL buffer from:\n"
	  "\t host   --> device (Upload)\n"
	  "\t device --> device (Copy)\n"
	  "\t device --> host   (Download)\n"
	  "It reports the average speed of each operation on the device.",
	  /****************************************************************/
	  Init_Transfer, Compute_Transfer, Verify_Transfer, NULL
	},
	{ "memcpy_workgroups",
	  /****************************************************************/
	  "Work-group Memory Copy Kernel:\n"
	  "\tPerforms an online compilation of a kernel which copies input\n"
	  "\tto output using work-groups, verifying the results.\n",
	  /****************************************************************/
	  Init_MemcpyWorkgroup, NULL, Verify_MemcpyWorkgroup, NULL
	},
	{ "memcpy_global_offsets",
	  /****************************************************************/
	  "Global-offset Memory Copy Kernel:\n"
	  "\tPerforms an online compilation of a kernel which copies input\n"
	  "\tto output using global offsets, verifying the results.\n",
	  /****************************************************************/
	  Init_MemcpyGlobalOffsets, NULL, Verify_MemcpyGlobalOffsets, NULL
	},
	{ "convolution",
	  /****************************************************************/
	  "Image Convolution Test:\n"
	  "\tRuns a number of image convolution kernels on an image\n",
	  /****************************************************************/
	  Init_Images, Compute_Images, Verify_Images, NULL
	},
	{ "conversions",
	  /****************************************************************/
	  "Conversions Test:\n"
	  "\tTests conversions between various data types and\n"
	  "\tverifies the results.\n",
	  /****************************************************************/
	  Init_Conversions, NULL, Verify_Conversions, NULL
	},
	{ "events",
	  /****************************************************************/
	  "Events Test:\n"
	  "\tRuns a mixture of kernels using events and not using events\n"
	  "\tall of which perform a mem copy and verify the result\n",
	  /****************************************************************/
	  Init_MulEvtKernel, NULL, Verify_MulEvtKernel, NULL
	},
	{ "atomics",
	  /****************************************************************/
	  "Atomics Test:\n"
	  "\tRuns all atomic functions\n",
	  /****************************************************************/
	  Init_Atomics, NULL, Verify_Atomics, NULL
	},
	{ "nop",
	  /****************************************************************/
	  "NOP Test:\n"
	  "\tRuns a kernel with no body effectively making it a NOP kernel\n",
	  /****************************************************************/
	  Init_NOP, Compute_NOP, Verify_NOP, NULL
	},
	{ "enqueue_native_kernel",
	  /****************************************************************/
	  "CL Enqueue Native Kernel:\n"
	  "\tEnqueues and flushes a native kernel which reads data from a\n"
	  "\tpointer and writes it to another location before verifying\n"
	  "\tthe result\n",
	  /****************************************************************/
	  Init_EnqueueNativeKernel, Compute_EnqueueNativeKernel, Verify_EnqueueNativeKernel, NULL
	},
	{ "async",
	  /****************************************************************/
	  "Work-group Memory Copy Kernel using async_work_group_copy:\n"
	  "\tPerforms an online compilation of a kernel which copies input\n"
	  "\tto output using work-groups, verifying the results.\n",
	  /****************************************************************/
	  Init_AsyncMemCopyKernel, NULL, Verify_AsyncMemCopyKernel, NULL
	},
	{ "spirv_add",
	  /****************************************************************/
	  "Addition SPIR-V Kernel:\n"
	  "\tPerforms an online compilation of an integer addition kernel\n"
	  "\tand verifies that the output buffer is correct.",
	  /****************************************************************/
	  Init_SpirVAdd, Compute_SpirVAdd, Verify_SpirVAdd, NULL
	}
};

#define OCLTEST_LIST_SIZE (sizeof(g_TestList) / sizeof(OCLTestInstance))

#endif
