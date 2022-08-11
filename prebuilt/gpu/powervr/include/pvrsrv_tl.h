/*************************************************************************/ /*!
@File
@Title          Services Transport Layer UM Client API
@Copyright      Copyright (c) Imagination Technologies Ltd. All Rights Reserved
@Description    User mode Transport layer API for clients.
@License        Strictly Confidential.
*/ /**************************************************************************/

#ifndef PVRSRV_TL_H
#define PVRSRV_TL_H

#if defined (__cplusplus)
extern "C" {
#endif


#include "img_defs.h"
#include "pvrsrv_error.h"
#include "services_km.h"

#include "pvrsrv_tlcommon.h"
#include "pvrsrv_tlstreams.h"

/*************************************************************************/ /*!
@Function       PVRSRVTLDiscoverStreams
@Description    Allows the discovery of streams which names match a given
                pattern. If aszStreams == NULL and *pui32NumFound == 0 function
                will return number of streams that names matched the pattern.
                If aszStreams != NULL and *pui32NumFound != 0 the function will
                return names of discovered streams (in aszStreams) and the
                actual count in *pui32NumFound.
                If the *pui32NumFound and therefore aszStreams size is too
                small than the actual number of matching streams only as many
                stream names will be returned as can fit into the buffer. In
                such case *pui32NumFound will still be equal to the number of
                returned streams.
@Input          psConnection   Address of a pointer to a connection object
@Input          pszNamePattern Name pattern in a beginning of a name that will
                               be matched i.e. every name which starts with the
                               given pattern will be returned in aszStreams.
@InOut          aszStreams     Array of stream names that matched
                               pszNamePattern or NULL.
                               If not NULL must be big enough to fit at least
                               pui32NumFound number of elements.
@InOut          pui32NumFound  When treated as input holds maximum number of
                               elements that can fit into aszStreams, when
                               treated as output it holds number of discovered
                               streams
@Return         PVRSRV_ERROR   for system error codes
*/ /**************************************************************************/
IMG_EXPORT
PVRSRV_ERROR IMG_CALLCONV PVRSRVTLDiscoverStreams(
		PVRSRV_DEV_CONNECTION *psConnection,
		const IMG_CHAR *pszNamePattern,
		IMG_CHAR aszStreams[][PRVSRVTL_MAX_STREAM_NAME_SIZE],
		IMG_UINT32 *pui32NumFound);

/*************************************************************************/ /*!
@Function       PVRSRVTLOpenStream
@Description    Open a descriptor onto an existing PVR transport stream. If the
                stream does not exist it returns a NOT_FOUND error unless the
                OPEN_WAIT flag is supplied. In this case it will wait for the
                stream to be created. If it is not created in the wait period
                a TIMEOUT error is returned.
                If OPEN_WO flag is provide the stream is opened as write only.
                This allows to call reserve/commit/write function on the stream
                but makes it impossible to read from the stream using returned
                descriptor.
@Input          psConnection Address of a pointer to a connection object
@Input          pszName      Address of the stream name string, no longer than
                             PRVSRVTL_MAX_STREAM_NAME_SIZE.
@Input          ui32Mode     Flags defined in pvrsrv_tlcommon.h:
                             ACQUIRE_NONBLOCKING: Results in non-blocking reads
                                 on stream. Reads are blocking by default
                             OPEN_WAIT: Causes open to wait for a brief moment
                                 if the stream does not exist
@Output         phSD         Address of a pointer to an stream object
@Return         PVRSRV_ERROR_NOT_FOUND:        when named stream not found
@Return         PVRSRV_ERROR_ALREADY_OPEN:     stream already open by another
@Return         PVRSRV_ERROR_STREAM_ERROR:     internal driver state error
@Return         PVRSRV_ERROR_TIMEOUT:          block timed out, stream not found
@Return         PVRSRV_ERROR:                  for other system codes
*/ /**************************************************************************/
IMG_EXPORT
PVRSRV_ERROR IMG_CALLCONV PVRSRVTLOpenStream(PVRSRV_DEV_CONNECTION* psConnection,
		const IMG_CHAR* pszName,
		IMG_UINT32   ui32Mode,
		PVRSRVTL_SD* phSD);


/*************************************************************************/ /*!
@Function       PVRSRVTLCloseStream
@Description    Close and release the stream connection to Services kernel
                server transport layer. Any outstanding Acquire will be
                released.
@Input          psConnection Address of a pointer to a connection object
@Input          hSD          Handle of the stream object to close
@Return         PVRSRV_ERROR_HANDLE_NOT_FOUND: when SD handle is not known
@Return         PVRSRV_ERROR_STREAM_ERROR:     internal driver state error
@Return         PVRSRV_ERROR:                  for system codes
*/ /**************************************************************************/
IMG_EXPORT
PVRSRV_ERROR IMG_CALLCONV PVRSRVTLCloseStream(PVRSRV_DEV_CONNECTION* psConnection,
		PVRSRVTL_SD hSD);



/****************************************************************************
 * Stream Buffer Data retrieval API(s)
 *
 * The client must ensure their use of this acquire/release API for a single
 * connection/stream must not be shared with multiple execution contexts e.g.
 * between a kernel thread and an ISR handler. It is the client's
 * responsibility to ensure this API is not interrupted by a high priority
 * thread/ISR
 ****************************************************************************/

/*************************************************************************/ /*!
@Function       PVRSRVTLReserveStream
@Description    Reserves space in the buffer and returns pointer to the
                reserved range. This call will fail if the stream is already
                reserved by someone.
                Each reserve call has to followed by a matching call to
                PVRSRVTLCommitStream.
@Input          psConnection Address of a pointer to a connection object
@Input          hSD          Handle of the stream object to read
@Output         ppui8Data    Address of a pointer to an byte buffer. On exit
                             pointer contains address of buffer to write to
@Input          ui32Size     Size of the space to reserve in the buffer
@Return         PVRSRV_ERROR_STREAM_FULL        the reserve size requested
                                                is larger than the free space.
@Return         PVRSRV_ERROR_HANDLE_NOT_FOUND   when SD handle not known
@Return         PVRSRV_ERROR_NOT_READY          There are data previously
                                                reserved that are pending
                                                to be committed.
@Return         PVRSRV_ERROR_TLPACKET_SIZE_LIMIT_EXCEEDED  reserve size breaches
                                                           max TL packet size
@Return         PVRSRV_ERROR_INVALID_PARAMS     at least one of the parameters
                                                is invalid
@Return         PVRSRV_ERROR_STREAM_ERROR       internal driver state error
*/ /**************************************************************************/
IMG_EXPORT
PVRSRV_ERROR IMG_CALLCONV PVRSRVTLReserveStream(PVRSRV_DEV_CONNECTION* psConnection,
		PVRSRVTL_SD hSD,
		IMG_UINT8 **ppui8Data,
		IMG_UINT32 ui32Size);

/*************************************************************************/ /*!
@Function       PVRSRVTLReserveStream2
@Description    Reserves space in the buffer and returns pointer to the
                reserved range. This call will fail if the stream is already
                reserved by someone.
                Each reserve call has to followed by a matching call to
                PVRSRVTLCommitStream.
@Input          psConnection   Address of a pointer to a connection object
@Input          hSD            Handle of the stream object to read
@Output         ppui8Data      Address of a pointer to an byte buffer. On exit
                               pointer contains address of buffer to write to
@Input          ui32Size       Ideal number of bytes to reserve in buffer
@Input          ui32SizeMin    Minimum number of bytes to reserve in buffer
@Input          pui32Available if RESERVE_TOO_BIG error is returned, a size
                               suggestion is returned in this argument which
                               the caller can attempt to reserve again for a
                               successful allocation.
@Return         PVRSRV_ERROR_STREAM_FULL       the reserve size requested
                                               is larger than the free
                                               space.
@Return         PVRSRV_ERROR_HANDLE_NOT_FOUND  when SD handle not known
@Return         PVRSRV_ERROR_NOT_READY         There are data previously
                                               reserved that are pending
                                               to be committed.
@Return         PVRSRV_ERROR_TLPACKET_SIZE_LIMIT_EXCEEDED  reserve size breaches max
                                                           TL packet size
@Return         PVRSRV_ERROR_INVALID_PARAMS    at least one of the parameters
                                               is invalid
@Return         PVRSRV_ERROR_STREAM_ERROR      internal driver state error
*/ /**************************************************************************/
IMG_EXPORT
PVRSRV_ERROR IMG_CALLCONV PVRSRVTLReserveStream2(PVRSRV_DEV_CONNECTION* psConnection,
		PVRSRVTL_SD hSD,
		IMG_UINT8 **ppui8Data,
		IMG_UINT32 ui32Size,
		IMG_UINT32 ui32SizeMin,
		IMG_UINT32 *pui32Available);

/*************************************************************************/ /*!
@Function       PVRSRVTLCommitStream
@Description    Commits data written to the buffer. This call must be proceed
                call to PVRSRVTLStreamReserve function.
@Input          psConnection Address of a pointer to a connection object
@Input          hSD          Handle of the stream object to read
@Input          ui32ReqSize  Size of the data to commit into the buffer
@Return         PVRSRV_ERROR_HANDLE_NOT_FOUND  when SD handle not known
@Return         PVRSRV_ERROR_INVALID_PARAMS    at least one of the parameters
                                               is invalid
@Return         PVRSRV_ERROR_STREAM_ERROR      internal driver state error
*/ /**************************************************************************/
IMG_EXPORT
PVRSRV_ERROR IMG_CALLCONV PVRSRVTLCommitStream(PVRSRV_DEV_CONNECTION* psConnection,
		PVRSRVTL_SD hSD,
		IMG_UINT32 ui32ReqSize);

/*************************************************************************/ /*!
@Function       PVRSRVTLAcquireData
@Description    When there is data available in the stream buffer this call
                returns with the address and length of the data buffer the
                client can safely read. This buffer may contain one or more
                packets of data.
                If no data is available then this call blocks until it becomes
                available. However if the stream has been destroyed while
                waiting then a resource unavailable error will be returned
                to the caller. Clients must pair this call with a
                ReleaseData call.
@Input          psConnection Address of a pointer to a connection object
@Input          hSD          Handle of the stream object to read
@Output         ppPacketBuf  Address of a pointer to an byte buffer. On exit
                             pointer contains address of buffer to read from
@Output         puiBufLen    Pointer to an integer. On exit it is the size of
                             the data to read from the packet buffer
@Return         PVRSRV_ERROR_RESOURCE_UNAVAILABLE: when stream no longer exists
@Return         PVRSRV_ERROR_HANDLE_NOT_FOUND:     when SD handle not known
@Return         PVRSRV_ERROR_STREAM_ERROR:         internal driver state error
@Return         PVRSRV_ERROR_RETRY:                release not called beforehand
@Return         PVRSRV_ERROR_TIMEOUT:              block timed out, no data
@Return         PVRSRV_ERROR:                      for other system codes
*/ /**************************************************************************/
IMG_EXPORT
PVRSRV_ERROR IMG_CALLCONV PVRSRVTLAcquireData(PVRSRV_DEV_CONNECTION* psConnection,
		PVRSRVTL_SD hSD,
		IMG_PBYTE*  ppPacketBuf,
		IMG_UINT32* puiBufLen);


/*************************************************************************/ /*!
 @Function      PVRSRVTLReleaseData
 @Description   Called after client has read the stream data out of the buffer
                The data is subsequently flushed from the stream buffer to make
                room for more data packets from the stream source.
 @Input         psConnection Address of a pointer to a connection object
 @Input         hSD          Handle of the stream object to read
 @Return        PVRSRV_ERROR_RESOURCE_UNAVAILABLE: when stream no longer exists
 @Return        PVRSRV_ERROR_HANDLE_NOT_FOUND:     when SD handle not known to TL
 @Return        PVRSRV_ERROR_STREAM_ERROR:         internal driver state error
 @Return        PVRSRV_ERROR_RETRY:                acquire not called beforehand
 @Return        PVRSRV_ERROR:                      for system codes
*/ /**************************************************************************/
IMG_EXPORT
PVRSRV_ERROR IMG_CALLCONV PVRSRVTLReleaseData(PVRSRV_DEV_CONNECTION* psConnection,
		PVRSRVTL_SD hSD);

/*************************************************************************/ /*!
 @Function      PVRSRVTLReleaseDataLess
 @Description   Called after client has read only some data out of the buffer
                and wishes to complete the read early i.e. does not want to read
                the full data that the acquire call returned e.g read just one
                packet from the stream.
                The data is subsequently flushed from the stream buffer to make
                room for more data packets from the stream source.
 @Input         psConnection    Address of a pointer to a connection object
 @Input         hSD             Handle of the stream object to read
 @Input         uiActualReadLen Size of data read, in bytes. Must be on a TL
                                packet boundary.
 @Return        PVRSRV_ERROR_INVALID_PARAMS:       when read length too big
 @Return        PVRSRV_ERROR_RESOURCE_UNAVAILABLE: when stream no longer exists
 @Return        PVRSRV_ERROR_HANDLE_NOT_FOUND:     when SD handle not known to TL
 @Return        PVRSRV_ERROR_STREAM_ERROR:         internal driver state error
 @Return        PVRSRV_ERROR_RETRY:                acquire not called beforehand
 @Return        PVRSRV_ERROR:                      for system codes
*/ /**************************************************************************/
IMG_EXPORT
PVRSRV_ERROR IMG_CALLCONV PVRSRVTLReleaseDataLess(
		PVRSRV_DEV_CONNECTION* psConnection,
		PVRSRVTL_SD hSD,
		IMG_UINT32 uiActualReadLen);

/*************************************************************************/ /*!
 @Function      PVRSRVTLWriteData
 @Description   Function writes data to the client stream.
 @param         psConnection Address of a pointer to a connection object
 @param         hSD          Handle of the stream object to read
 @param         ui32Size     Size of the given data. Please note that due to
                             performance reasons data size should not exceed
                             10KB. When the data to write is larger than this
                             it is faster to use the reserve/commit API pair
                             instead.
 @param         pui8Data     Pointer to data
 @return        PVRSRV_ERROR
*/ /**************************************************************************/
IMG_EXPORT
PVRSRV_ERROR IMG_CALLCONV PVRSRVTLWriteData(PVRSRV_DEV_CONNECTION* psConnection,
	PVRSRVTL_SD hSD,
	IMG_UINT32 ui32Size,
	IMG_BYTE* pui8Data);

#if defined (__cplusplus)
}
#endif

#endif /* PVRSRV_TL_H */

/******************************************************************************
 End of file (pvrsrv_tl.h)
******************************************************************************/
