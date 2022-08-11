/*************************************************************************/ /*!
@File
@Title          Device class interface
@Description    Client interface to Display Class devices which have been
                registered with services.
@Copyright      Copyright (c) Imagination Technologies Ltd. All Rights Reserved
@License        Strictly Confidential.
*/ /**************************************************************************/

#ifndef _DC_CLIENT_H
#define _DC_CLIENT_H

#if defined(SUPPORT_DISPLAY_CLASS) || defined(DOXYGEN)

#if defined (__cplusplus)
extern "C" {
#endif

#include "img_types.h"
#include "img_defs.h"
#include "pvrsrv_error.h"
#include "services.h"		/* For PVRSRV_DEV_CONNECTION */
#include "pvrsrv_surface.h"
#include "dc_external.h"
#include "pvrsrv_devmem.h"	/* Required for PVRSRV_MEMDESC */

/*************************************************************************/ /*!
@Function       PVRSRVDCDevicesQueryCount

@Description    Query services for the number of registered DC devices.

@Input          psConnection            Services connection

@Output         pui32Count              Number of display drivers

@Return         PVRSRV_OK if the query was successful
*/ /**************************************************************************/
IMG_EXPORT PVRSRV_ERROR PVRSRVDCDevicesQueryCount(const PVRSRV_DEV_CONNECTION *psConnection,
                                                  IMG_UINT32 *pui32Count);

/*************************************************************************/ /*!
@Function       PVRSRVDCDevicesEnumerate

@Description    Query services for the number of registered DC devices and
                return their indices in an array.

@Input          psConnection            Services connection

@Input          ui32DeviceArraySize     Array size of paui32DeviceIndex

@Output         pui32DeviceCount        Number of returned devices

@Output         paui32DeviceIndex       Array of device indices

@Return         PVRSRV_OK if the enumeration was successful
*/ /**************************************************************************/
IMG_EXPORT PVRSRV_ERROR PVRSRVDCDevicesEnumerate(const PVRSRV_DEV_CONNECTION *psConnection,
                                                 IMG_UINT32 ui32DeviceArraySize,
                                                 IMG_UINT32 *pui32DeviceCount,
                                                 IMG_UINT32 *paui32DeviceIndex);

/*************************************************************************/ /*!
@Function       PVRSRVDCDeviceAcquire

@Description    Acquire a DC device based on its device index.
                A valid index can be acquired with PVRSRVDCDevicesEnumerate.

@Input          psConnection            Services connection

@Input          ui32DeviceIndex         DC device index to acquire

@Output         phDevice                Handle to 3rd party display class device

@Return         PVRSRV_OK if the acquire was successful
*/ /**************************************************************************/
IMG_EXPORT PVRSRV_ERROR PVRSRVDCDeviceAcquire(const PVRSRV_DEV_CONNECTION *psConnection,
                                              IMG_UINT32 ui32DeviceIndex,
                                              IMG_HANDLE *phDevice);

/*************************************************************************/ /*!
@Function       PVRSRVDCDeviceRelease

@Description    Release a DC device.

@Input          psConnection            Services connection

@Input          hDevice                 Handle to 3rd party display class device
*/ /**************************************************************************/
IMG_EXPORT void PVRSRVDCDeviceRelease(const PVRSRV_DEV_CONNECTION *psConnection,
                                      IMG_HANDLE hDevice);

/*************************************************************************/ /*!
@Function       PVRSRVDCGetInfo

@Description    Gets a DC device's info structure.

   See #GetInfo()

@Input          psConnection            Services connection

@Input          hDevice                 Handle to 3rd party display class device

@Output         psDisplayInfo           Display info
*/ /**************************************************************************/
IMG_EXPORT PVRSRV_ERROR PVRSRVDCGetInfo(const PVRSRV_DEV_CONNECTION *psConnection,
                                        IMG_HANDLE hDevice,
                                        DC_DISPLAY_INFO *psDisplayInfo);

/*************************************************************************/ /*!
@Function       PVRSRVDCPanelQueryCount

@Description    Query the DC for how many panels are connected to it.

   See #PanelQueryCount()

@Input          psConnection            Services connection

@Input          hDevice                 3rd party display class device

@Output         pui32NumPanels          Number of panels

@Return         PVRSRV_OK if the query was successful
*/ /**************************************************************************/
IMG_EXPORT PVRSRV_ERROR PVRSRVDCPanelQueryCount(const PVRSRV_DEV_CONNECTION *psConnection,
                                                IMG_HANDLE hDevice,
                                                IMG_UINT32 *pui32NumPanels);

/*************************************************************************/ /*!
@Function       PVRSRVDCPanelQuery

@Description    Query the DC for information on what panel(s) are connected
                to it and their properties.

   See #PanelQuery()

@Input          psConnection            Services connection

@Input          hDevice                 3rd party display class device

@Input          ui32PanelsArraySize     Size of the format and dimension
                                        array size (i.e. number of panels
                                        that can be returned)

@Output         pui32NumPanels          Number of panels returned

@Output         pasPanelInfo            Array of panel infos

@Return         PVRSRV_OK if the query was successful
*/ /**************************************************************************/
IMG_EXPORT PVRSRV_ERROR PVRSRVDCPanelQuery(const PVRSRV_DEV_CONNECTION *psConnection,
                                           IMG_HANDLE hDevice,
                                           IMG_UINT32 ui32PanelsArraySize,
                                           IMG_UINT32 *pui32NumPanels,
                                           PVRSRV_PANEL_INFO *pasPanelInfo);

/*************************************************************************/ /*!
@Function       PVRSRVDCFormatQuery

@Description    Query the DC to see if it supports the specified format(s).

   See #FormatQuery()

@Input          psConnection            Services connection

@Input          hDevice                 3rd party display class device

@Input          ui32NumFormats          Number of formats to check

@Input          pasFormat               Array of formats to check

@Output         pui32Supported          For each format, the number of display
                                        pipes that support that format

@Return         PVRSRV_OK if the query was successful
*/ /**************************************************************************/
IMG_EXPORT PVRSRV_ERROR PVRSRVDCFormatQuery(const PVRSRV_DEV_CONNECTION *psConnection,
                                            IMG_HANDLE hDevice,
                                            IMG_UINT32 ui32NumFormats,
                                            PVRSRV_SURFACE_FORMAT *pasFormat,
                                            IMG_UINT32 *pui32Supported);

/*************************************************************************/ /*!
@Function       PVRSRVDCDimQuery

   See #DimQuery()

@Description    Query the specified display plane for the display dimensions
                it supports.

@Input          psConnection            Services connection

@Input          hDevice                 3rd party display class device

@Input          ui32NumDims             Number of dimensions to check

@Input          pasDim                  Array of dimensions to check

@Output         pui32Supported          For each dimension, the number of
                                        display pipes that support that
                                        dimension

@Return         PVRSRV_OK if the query was successful
*/ /**************************************************************************/
IMG_EXPORT PVRSRV_ERROR PVRSRVDCDimQuery(const PVRSRV_DEV_CONNECTION *psConnection,
                                         IMG_HANDLE hDevice,
                                         IMG_UINT32 ui32NumDims,
                                         PVRSRV_SURFACE_DIMS *pasDim,
                                         IMG_UINT32 *pui32Supported);

/*************************************************************************/ /*!
@Function       PVRSRVDCSetBlank

   See #SetBlank()

@Description    Enable/disable blanking of the screen

@Input          psConnection            Services connection

@Input          hDevice                 3rd party display class device

@Input          bEnable                 Enable/Disable the blanking

@Return         PVRSRV_OK on success
*/ /**************************************************************************/
IMG_EXPORT PVRSRV_ERROR PVRSRVDCSetBlank(const PVRSRV_DEV_CONNECTION *psConnection,
                                         IMG_HANDLE hDevice,
                                         IMG_BOOL bEnable);

/*************************************************************************/ /*!
@Function       PVRSRVDCSetVSyncReporting

   See #SetVSyncReporting()

@Description    Enable VSync reporting on every vsync.
                The DC device will be asked to notify services with
                a PVRSRVCheckStatus() call whenever a vsync event occurred.
                PVRSRVCheckStatus will signal the global event object to wake
                up the driver and the GPU to schedule unblocked work.

@Input          psConnection            Services connection

@Input          hDevice                 3rd party display class device

@Input          bEnable                 Enable/Disable the reporting

@Return         PVRSRV_OK on success
*/ /**************************************************************************/
IMG_EXPORT PVRSRV_ERROR PVRSRVDCSetVSyncReporting(const PVRSRV_DEV_CONNECTION *psConnection,
                                                  IMG_HANDLE hDevice,
                                                  IMG_BOOL bEnable);

/*************************************************************************/ /*!
@Function       PVRSRVDCLastVSyncQuery

@Description    Query the time the last vsync happened.

   See #LastVSyncQuery()

@Input          psConnection            Services connection

@Input          hDevice                 3rd party display class device

@Output         pi64Timestamp           The requested timestamp in ns

@Return         PVRSRV_OK if the query was successful
*/ /**************************************************************************/
IMG_EXPORT PVRSRV_ERROR PVRSRVDCLastVSyncQuery(const PVRSRV_DEV_CONNECTION *psConnection,
                                               IMG_HANDLE hDevice,
                                               IMG_INT64 *pi64Timestamp);

/*************************************************************************/ /*!
@Function       DCDisplayContextCreate

@Description    Create display context. Display reconfigurations are done on a
                display context and guaranteed to happen in order and when
                any dependencies have been meet.
                There is no connection between a display pipe or panel and a
                display context (i.e. a display context might control more than
                one display plane, and the number of display planes that
                are modified per reconfiguration can vary).
                A display context can be seen as a
                container in which buffers are allocated or imported and
                reconfigurations are made. It's purely a SW concept and is
                there to ensure that operations issued on the same display
                context are issued in order.

   See #ContextCreate()

@Input          psConnection            Services connection

@Input          hDevice                 3rd party display class device

@Output         phDisplayContext        Created display context

@Return         PVRSRV_OK if the context was created
*/ /**************************************************************************/
IMG_EXPORT PVRSRV_ERROR PVRSRVDCDisplayContextCreate(PVRSRV_DEV_CONNECTION *psConnection,
                                                     IMG_HANDLE hDevice,
                                                     IMG_HANDLE *phDisplayContext);

/*************************************************************************/ /*!
@Function       PVRSRVDCContextConfigureCheck

@Description    Check a configuration of the specified display context is
                valid. The arrays should be z-sorted, with the farthest plane
                first and the nearest plane last.

   See #ContextConfigureCheck()

@Input          psConnection            Services connection

@Input          hDisplayContext         Display context

@Input          ui32PipeCount           Number of display pipes to configure

@Input          pasConfigInfo           Array of surface attributes (one for
                                        each display plane)

@Input          ahBuffers               Array of buffers (one for
                                        each display plane)

@Return         PVRSRV_OK if the reconfiguration was valid
*/ /**************************************************************************/
IMG_EXPORT PVRSRV_ERROR PVRSRVDCContextConfigureCheck(const PVRSRV_DEV_CONNECTION *psConnection,
                                                      IMG_HANDLE hDisplayContext,
                                                      IMG_UINT32 ui32PipeCount,
                                                      PVRSRV_SURFACE_CONFIG_INFO *pasConfigInfo,
                                                      IMG_HANDLE *ahBuffers);

/*************************************************************************/ /*!
@Function       PVRSRVDCContextConfigureWithFence

@Description    Queue a configuration of the display pipeline to happen on
                the specified display context. The arrays should be z-sorted,
                with the farthest plane first and the nearest plane last.
                Makes use OS native syncs instead of Services sync prims.

   See #ContextConfigure()

@Input          psConnection            Services connection

@Input          hDisplayContext         Display context

@Input          ui32PipeCount           Number of display pipes to configure

@Input          pasConfigInfo           Array of surface attributes (one for
                                        each display plane)

@Input          ahBuffers               Array of buffers (one for
                                        each display plane)

@Input          ui32DisplayPeriod       The number of VSync periods this
                                        configuration should be displayed for

@Input          ui32MaxDepth            If non-zero, block until fewer than
                                        this number of reconfigure requests
                                        are outstanding

@Input          i32AcquireFenceFd       fd of fence to wait for before executing

@Output         pi32ReleaseFenceFd      fd of fence to signal when call has been
                                        executed

@Return         PVRSRV_OK if the reconfiguration was successfully queued
*/ /**************************************************************************/
IMG_EXPORT PVRSRV_ERROR PVRSRVDCContextConfigureWithFence(const PVRSRV_DEV_CONNECTION *psConnection,
                                                           IMG_HANDLE hDisplayContext,
                                                           IMG_UINT32 ui32PipeCount,
                                                           PVRSRV_SURFACE_CONFIG_INFO *pasConfigInfo,
                                                           IMG_HANDLE *ahBuffers,
                                                           IMG_UINT32 ui32DisplayPeriod,
                                                           IMG_UINT32 ui32MaxDepth,
                                                           IMG_INT32  i32AcquireFenceFd,
                                                           IMG_INT32 *pi32ReleaseFenceFd);

/*************************************************************************/ /*!
@Function       DCDisplayContextDestroy

@Description    Destroy a display context.

   See #ContextDestroy()

@Input          psConnection            Services connection

@Input          hDisplayContext         Display context to destroy

@Return         PVRSRV_OK if the context was created
*/ /**************************************************************************/
IMG_EXPORT PVRSRV_ERROR PVRSRVDCDisplayContextDestroy(PVRSRV_DEV_CONNECTION *psConnection,
                                                      IMG_HANDLE hDisplayContext);

/*************************************************************************/ /*!
@Function       PVRSRVDCBufferAlloc

@Description    Allocate a display buffer.
                The call requests the buffer from the 3rd party display class.

   See #BufferAlloc()

@Input          psConnection            Services connection

@Input          hDisplayContext         Display context this buffer will be
                                        used on

@Input          psSurfInfo              Attributes of the buffer

@Output         pui32ByteStride         Stride of the allocated surface

@Output         phBuffer                Handle to allocated buffer

@Return         PVRSRV_OK if the buffer was successfully allocated
*/ /**************************************************************************/
IMG_EXPORT PVRSRV_ERROR PVRSRVDCBufferAlloc(const PVRSRV_DEV_CONNECTION *psConnection,
                                            IMG_HANDLE hDisplayContext,
                                            PVRSRV_SURFACE_INFO *psSurfInfo,
                                            IMG_UINT32 *pui32ByteStride,
                                            IMG_HANDLE *phBuffer);

/*************************************************************************/ /*!
@Function       PVRSRVDCBufferFree

@Description    Free a display buffer allocated with PVRSRVDCBufferAlloc

   See #BufferFree()

@Input          psConnection            Services connection

@Input          hBuffer                 Handle to buffer to free

@Return         PVRSRV_OK if the buffer was successfully allocated
*/ /**************************************************************************/
IMG_EXPORT PVRSRV_ERROR PVRSRVDCBufferFree(const PVRSRV_DEV_CONNECTION *psConnection,
                                           IMG_HANDLE hBuffer);

/*************************************************************************/ /*!
@Function       PVRSRVDCBufferImport

@Description    Import Services device memory in form of services MemDescs into
                the display driver.

   See #BufferImport()

@Input          psConnection            Services connection

@Input          hDisplayContext         Display context this buffer will be
                                        used on

@Input          pasMemDesc              Array of MemDescs to import (one per
                                        colour channel)

@Input          psImportInfo            Attributes of the import buffer

@Output         phBuffer                Handle to imported buffer

@Return         PVRSRV_OK if the buffer was successfully imported
*/ /**************************************************************************/
IMG_EXPORT PVRSRV_ERROR PVRSRVDCBufferImport(const PVRSRV_DEV_CONNECTION *psConnection,
                                             IMG_HANDLE hDisplayContext,
                                             PVRSRV_MEMDESC *pasMemDesc,
                                             DC_BUFFER_IMPORT_INFO *psImportInfo,
                                             IMG_HANDLE *phBuffer);

/*************************************************************************/ /*!
@Function       PVRSRVDCBufferUnimport

@Description    Unimport a buffer

@Input          psConnection            Services connection

@Input          hBuffer                 Buffer to unimport
*/ /**************************************************************************/
IMG_EXPORT void PVRSRVDCBufferUnimport(const PVRSRV_DEV_CONNECTION *psConnection,
                                       IMG_HANDLE hBuffer);


/*************************************************************************/ /*!
@Function       PVRSRVDCSystemBufferAcquire

@Description    DEPRECATED, please use PVRSRVDCBufferAlloc
                Acquire the system buffer from the 3rd party display class.
                This is usually a buffer that has been created by the OS via a
                native interface compared to a buffer that was created on demand
                by the display driver. Can be used for direct frame buffer
                rendering.

   See #BufferSystemAcquire()

@Input          psConnection            Services connection

@Input          hDevice                 3rd party display class device

@Output         pui32ByteStride         Stride of the buffer in Byte

@Output         phBuffer                Buffer handle

@Return         PVRSRV_OK if the query was successful
*/ /**************************************************************************/
IMG_EXPORT PVRSRV_ERROR PVRSRVDCSystemBufferAcquire(const PVRSRV_DEV_CONNECTION *psConnection,
                                                    IMG_HANDLE hDevice,
                                                    IMG_UINT32 *pui32ByteStride,
                                                    IMG_HANDLE *phBuffer);

/*************************************************************************/ /*!
@Function       PVRSRVDCSystemBufferRelease

@Description    DEPRECATED, please use PVRSRVDCBufferFree
                Release the system buffer

   See #BufferSystemRelease()

@Input          psConnection            Services connection

@Input          hBuffer                 Buffer handle

@Return         PVRSRV_OK if the release was successful
*/ /**************************************************************************/
IMG_EXPORT PVRSRV_ERROR PVRSRVDCSystemBufferRelease(const PVRSRV_DEV_CONNECTION *psConnection,
                                                    IMG_HANDLE hBuffer);

/*************************************************************************/ /*!
@Function       PVRSRVDCBufferPin

@Description    Maps the buffer into the display controller if it hasn't already
                been mapped and make sure it won't get unmapped until
                PVRSRVDCBufferUnpin is called.

                Note: Without calling this the display controller will still
                be asked to map the buffer when that buffer is specified
                in a call to PVRSRVDCContextConfigure and will be asked to
                unmap the buffer once the configuration has been retired.
                This function is provided to allow the client to have control
                over when the display driver maps and unmaps surfaces because
                the application often knows better when it is most efficient
                to do the mapping.

   See #BufferMap()

@Input          psConnection            Services connection

@Input          hBuffer                 Buffer we're pinning

@Output         phPinHandle             Pin handle

@Return         PVRSRV_OK if the buffer was successfully pinned
*/ /**************************************************************************/
IMG_EXPORT PVRSRV_ERROR PVRSRVDCBufferPin(const PVRSRV_DEV_CONNECTION *psConnection,
                                          IMG_HANDLE hBuffer,
                                          IMG_HANDLE *phPinHandle);

/*************************************************************************/ /*!
@Function       PVRSRVDCBufferUnpin

@Description    Unpin the buffer and potentially unmap it from the display
                controller.

                Note: It is safe to call this with queued operations on
                the buffer due to the fact that PVRSRVDCContextConfigure will
                have already taken a mapping reference and so the buffer
                won't actually be unmapped until the configuration that is using
                the buffer has been retired.

   See #BufferUnmap()

@Input          psConnection            Services connection

@Input          hPinHandle              Pin handle we're releasing
*/ /**************************************************************************/
IMG_EXPORT void PVRSRVDCBufferUnpin(const PVRSRV_DEV_CONNECTION *psConnection,
                                    IMG_HANDLE hPinHandle);

/*************************************************************************/ /*!
@Function       PVRSRVDCBufferAcquire

@Description    Acquire a handle from the buffer which the device memory manager
                can use to turn into a MEMDESC. The buffer could come from
                either PVRSRVDCBufferAlloc or PVRSRVDCSystemBufferAcquire.
                Clients can make it available to services via
                PVRSRVDevmemLocalImport which returns a MEMDESC.

@Input          psConnection            Services connection

@Input          hBuffer                 Buffer we're acquiring

@Output         phExtMem                External memory handle

@Return         PVRSRV_OK if the buffer was successfully acquired
*/ /**************************************************************************/
IMG_EXPORT PVRSRV_ERROR PVRSRVDCBufferAcquire(const PVRSRV_DEV_CONNECTION *psConnection,
                                              IMG_HANDLE hBuffer,
                                              IMG_HANDLE *phExtMem);

/*************************************************************************/ /*!
@Function       PVRSRVDCBufferRelease

@Description    Release the buffers device memory manager handle

@Input          psConnection            Services connection

@Input          hExtMem                 External memory handle
*/ /**************************************************************************/
IMG_EXPORT void PVRSRVDCBufferRelease(const PVRSRV_DEV_CONNECTION *psConnection,
                                      IMG_HANDLE hExtMem);

/*************************************************************************/ /*!
@Function       PVRSRVDCGetDisplayContextServerHandle

@Description    Fetches the server-side display context handle.

@Input          hDisplayContextClient   Client-side display context handle

@Return         Server-side display context handle
*/ /**************************************************************************/
IMG_EXPORT IMG_HANDLE PVRSRVDCGetDisplayContextServerHandle(IMG_HANDLE hDisplayContextClient);

#if defined (__cplusplus)
}
#endif

#endif /* defined(SUPPORT_DISPLAY_CLASS) */

#endif /*_DC_CLIENT_H*/
