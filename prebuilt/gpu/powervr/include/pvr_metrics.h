/*************************************************************************/ /*!
@File
@Title          Functions and Macros used to measure time.
@Copyright      Copyright (c) Imagination Technologies Ltd. All Rights Reserved
@License        Strictly Confidential.
*/ /**************************************************************************/

#ifndef _PVRMETRICS_
#define _PVRMETRICS_

#if defined (__cplusplus)
extern "C" {
#endif

#if defined(TIMING) || defined(DEBUG)

#if defined(__linux__)
	IMG_EXPORT void   PVRSRVGetTimerRegister(void	*pvSOCTimerRegisterUM);
	IMG_EXPORT void   PVRSRVReleaseTimerRegister(void);
#endif
IMG_EXPORT IMG_UINT32 PVRSRVMetricsTimeNow(void);
IMG_EXPORT IMG_FLOAT  PVRSRVMetricsGetCPUFreq(void);

/*!
 *******************************************************************************
 * @brief Controls ARM Performance Counters.
 *
 * Allows to enable/disable and control some of the parameter of ARM Performance
 * counters.
 *
 * Counters have divider that when enabled will increase possible measurement
 * time at a cost of lowering the precision.
 *
 * When reset is set to IMG_TRUE counters are reset/set to zero.
 *
 * @param bEnable IMG_TRUE to enable counter and IMG_FALSE to disable them
 * @param bReset IMG_TRUE to reset/zero the counters, IMG_FALSE does nothing
 * @param bDividerEn IMG_TRUE to enable divider for counters (divides by 64),
 *        IMG_FALSE set divider to 1
 *
 * @note For the counters to give a valid result the could has to be executed
 *       in only one core/CPU. One of the method is to set CPU affinity
 *       for example by using PVRSRVSetCpuAffinity.
 * @note This function affects only counter of the core/CPU on which it is
 *       executed. Other cores/CPUs are unaffected.
 ******************************************************************************/
IMG_EXPORT void PVRSRVMetricsPerfCounterCtrl(IMG_BOOL bEnable,
                                             IMG_BOOL bReset,
                                             IMG_BOOL bDividerEn);
IMG_EXPORT IMG_UINT32 PVRSRVMetricsGetPerfCounter(void);

IMG_EXPORT void PVRSRVInitProfileOutput(void **ppvFileInfo);
IMG_EXPORT void PVRSRVDeInitProfileOutput(void **ppvFileInfo);
IMG_EXPORT void PVRSRVProfileOutput(void *pvFileInfo, const IMG_CHAR *psString);

typedef struct
{
	IMG_UINT32 ui32Start, ui32Stop, ui32Count, ui32Max;
	IMG_UINT64 ui32Total;
#if TIMING_LEVEL != 0
	IMG_UINT64 ui64Current;
#endif
	IMG_FLOAT fStack;

} PVR_Temporal_Data;


/*
  Parameter explanation:

  X       : a Temporal_Data instance
  SWAP    : a *_TIMES_SWAP_BUFFERS Temporal_Data instance
  H       : an application hint
  CPU     : float for CPU speed
  TAG     : a define
  Y       : numerical parameter
*/

#if defined(__arm__) && defined(METRICS_USE_ARM_COUNTERS)
#define __PVR_MTR_GET_NOW() PVRSRVMetricsGetPerfCounter()
#else
#define __PVR_MTR_GET_NOW() PVRSRVMetricsTimeNow()
#endif

#define PVR_MTR_CALLS(X)				X.ui32Count

#define PVR_MTR_TIME_PER_CALL(X, CPU)	( (X.ui32Count) ? \
										(X.ui32Total*CPU/X.ui32Count) : 0.0f )

#define PVR_MTR_MAX_TIME(X, CPU)		( X.ui32Max*CPU )

#define PVR_MTR_PIXELS_PER_CALL(X)		( (X.ui32Count) ? \
										(X.fStack*1000.0f/X.ui32Count) : 0.0f )

/* generic per frame metrics - an integer is passed as second parameter */

#define PVR_MTR_TIME_PER_FRAME_GENERIC(X, Y, CPU)  ((X.ui32Total*CPU)/(Y))

#define PVR_MTR_PIXELS_PER_FRAME_GENERIC(X, Y)	   ((X.fStack*1000.0f)/(Y))

#define PVR_MTR_METRIC_PER_FRAME_GENERIC(X, Y)	   (X.ui32Count/(Y))

#define PVR_MTR_PARAM_PER_FRAME_GENERIC(X, Y)	   ((X)/(Y))


/*
   per frame metrics - a timer is passed as second parameter,
   typically a *_TIMER_SWAP_HW_TIME timer
*/

#define PVR_MTR_TIME_PER_FRAME(SWAP, X, CPU)	( (SWAP.ui32Count) ? \
										        (X.ui32Total*CPU/SWAP.ui32Count) : 0.0f )

#define PVR_MTR_PIXELS_PER_FRAME(SWAP, X)       ( (SWAP.ui32Count) ? \
										        (X.fStack*1000.0f/SWAP.ui32Count) : 0.0f )

#define PVR_MTR_METRIC_PER_FRAME(SWAP, X)       ( (SWAP.ui32Count) ? \
										        (X.ui32Count/SWAP.ui32Count) : 0 )

#define PVR_MTR_PARAM_PER_FRAME(SWAP, X)	    X/SWAP.ui32Count


#define PVR_MTR_CHECK_BETWEEN_START_END_FRAME(H, SWAP)	((H.ui32ProfileStartFrame <= SWAP.ui32Count) && \
												        (H.ui32ProfileEndFrame >= SWAP.ui32Count))


#define PVR_MTR_TIME_RESET(X)      {                    \
									X.ui32Count = 0;	\
									X.ui32Total = 0;	\
									X.ui32Start = 0; 	\
									X.ui32Stop  = 0;    \
									X.fStack    = 0.0f;	\
								   }

#define PVR_MTR_TIME_START(X, H, SWAP)            {                                                              \
									               if(PVR_MTR_CHECK_BETWEEN_START_END_FRAME(H, SWAP))            \
									               {                                                             \
										            X.ui32Count += 1;			                                 \
										            X.ui32Count |= 0x80000000L;                                  \
										            X.ui32Start = __PVR_MTR_GET_NOW(); X.ui32Stop = 0;           \
									               }                                                             \
								                  }

#define PVR_MTR_SWAP_TIME_START(X)                {                                                              \
									               {                                                             \
										            X.ui32Count += 1;			                                 \
										            X.ui32Count |= 0x80000000L;                                  \
										            X.ui32Start = __PVR_MTR_GET_NOW(); X.ui32Stop = 0;           \
									               }                                                             \
								                  }

#define PVR_MTR_TIME_SUSPEND(X)             { IMG_UINT32 ui32Tmp = __PVR_MTR_GET_NOW(); X.ui32Stop += ui32Tmp - X.ui32Start; }

#define PVR_MTR_TIME_RESUME(X)              { X.ui32Start = __PVR_MTR_GET_NOW(); }

#define PVR_MTR_TIME_STOP(X, H, SWAP)             {                                                                         \
									               if(PVR_MTR_CHECK_BETWEEN_START_END_FRAME(H, SWAP))                       \
									               {                                                                        \
										            IMG_UINT32 ui32Tmp = __PVR_MTR_GET_NOW();                                                         \
										            IMG_UINT32 ui32TimeLapse = ui32Tmp - X.ui32Start; \
										            X.ui32Stop +=  ui32TimeLapse;                                           \
										            if(X.ui32Stop > X.ui32Max)                                           \
                                                    {                                                                       \
											           X.ui32Max = X.ui32Stop;                                           \
										            }                                                                       \
										            X.ui32Total += X.ui32Stop; X.ui32Count &= 0x7FFFFFFFL;                  \
									               }                                                                        \
								                  }

#define PVR_MTR_SWAP_TIME_STOP(X)            {                                                            \
									          {                                                           \
	                                            IMG_UINT32 ui32Tmp = __PVR_MTR_GET_NOW();                                                         \
											    IMG_UINT32 ui32TimeLapse = ui32Tmp - X.ui32Start; \
											    X.ui32Stop +=  ui32TimeLapse;                                           \
											    if(X.ui32Stop > X.ui32Max)                                           \
	                                            {                                                                       \
												  X.ui32Max = X.ui32Stop;                                           \
											    }                                                                       \
											    X.ui32Total += X.ui32Stop; X.ui32Count &= 0x7FFFFFFFL;                  \
									          }                                                           \
								             }

#define PVR_MTR_INC_PERFRAME_COUNT(X)	{                       \
										 if (X.fStack == 0.0f)  \
										 {                      \
											X.ui32Count++;		\
											X.fStack = 1.0f;	\
										 }                      \
									    }

#define PVR_MTR_RESET_PERFRAME_COUNT(X) { X.fStack = 0.0f;}

#define PVR_MTR_DECR_COUNT(X)   	   	{ X.ui32Count -= 1; }

#define PVR_MTR_INC_PIXEL_COUNT(X, Y)   { X.fStack  += 0.001f*(IMG_FLOAT)(Y); }


#define PVR_MTR_INC_COUNT(TIMER, X, Y, H, SWAP, TAG) {                                                                         \
                                                      IMG_INT tempTimer = TIMER;                                                   \
									                  if((tempTimer == TAG) || PVR_MTR_CHECK_BETWEEN_START_END_FRAME(H, SWAP)) \
									                  {                                                                        \
										               X.ui32Count += 1;		                                               \
		                                               X.ui32Stop  += (Y);	                                                   \
										               X.ui32Total += (Y); 	                                                   \
									                  }                                                                        \
								                     }

#define PVR_MTR_SCALE_INC_COUNT(TIMER, X, Y, SCALE, H, SWAP, TAG) {                                                                         	\
																   IMG_INT tempTimer = TIMER;                                                   \
																   X.fStack = (SCALE);                                                          \
																   if((tempTimer == TAG) || PVR_MTR_CHECK_BETWEEN_START_END_FRAME(H, SWAP)) 	\
																   {                                                                        	\
																	IMG_FLOAT fScaledValue = (SCALE)*(IMG_FLOAT)(Y);                         	\
																	IMG_FLOAT fStop = (*((IMG_FLOAT*)&(X.ui32Stop))) + fScaledValue;       		\
																	IMG_FLOAT fTotal = (*((IMG_FLOAT*)&(X.ui32Total))) + fScaledValue;       	\
																    X.ui32Count += 1;		                                               		\
																    X.ui32Stop  = *((IMG_UINT32 *)(&fStop));	                           		\
																    X.ui32Total = *((IMG_UINT32 *)(&fTotal)); 	                           		\
																   }                                                                        	\
																  }

#else /* !(defined (TIMING) || defined (DEBUG)) */

#define PVR_MTR_TIME_RESET(X)
#define PVR_MTR_TIME_START(X, H, SWAP)
#define PVR_MTR_TIME_SUSPEND(X)
#define PVR_MTR_TIME_RESUME(X)
#define PVR_MTR_TIME_STOP(X, H, SWAP)
#define PVR_MTR_SWAP_TIME_START(X)
#define PVR_MTR_SWAP_TIME_STOP(X)
#define PVR_MTR_INC_COUNT(X, Y, H, SWAP, TAG)
#define PVR_MTR_DECR_COUNT(X)
#define PVR_MTR_INC_PIXEL_COUNT(X, Y)
#define PVR_MTR_INC_PERFRAME_COUNT(X)
#define PVR_MTR_RESET_PERFRAME_COUNT(X)
#define PVR_MTR_TIME_RESET_SUM(X)

  /* these macros are never used, so far */
#define PVR_MTR_RESET_FRAME()
#define PVR_MTR_INC_POLY_COUNT(X)

#endif /* defined (TIMING) || defined (DEBUG) */

#if defined(__cplusplus)
}
#endif

#endif /* _PVRMETRICS_ */

/******************************************************************************
 End of file (pvr_metrics.h)
******************************************************************************/

