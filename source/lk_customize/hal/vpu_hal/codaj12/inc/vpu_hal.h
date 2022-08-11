/**
 * File: vpu_hal.h
 * Brief:
 * Data:10/11/2019
 *
 * Copyright (C) 2019 Semidrive Technology Co., Ltd.
 * Author: chentianming<tianming.chen@semidrive.com>
 *
 */
/*******************************************************************************
 *                       Include header files                                 *
 ******************************************************************************/
#ifndef __CODAJ12_HAL_H__
#define __CODAJ12_HAL_H__

#ifdef __cplusplus
extern "C"
{
#endif
#include "__regs_base.h"
#include <kernel/mutex.h>
#include <kernel/event.h>
#include <platform/interrupts.h>
#include "res.h"
#include "chip_res.h"
#include "system_cfg.h"

#include "jpuapifunc.h"
#include "jpuapi.h"
#include "jdi.h"
/*******************************************************************************
 *                        Macro definitions                                   *
 ******************************************************************************/
#define HAL_CODAJ12_DEBUG
#define log_err(...)        do { printf("%s:%d ", __FILE__, __LINE__);  printf(__VA_ARGS__); } while(0)
#ifdef HAL_CODAJ12_DEBUG
#define log_debug(...)      do { printf("%s:%d ", __FILE__, __LINE__); printf(__VA_ARGS__); } while(0)
#else
#define log_debug(...)
#endif


#define VPU_CEIL(_s, _n)        (((_n) + (_s-1))&~(_s-1))
#define VPU_FLOOR(_s, _n)       (_n&~(_s-1))
/*******************************************************************************
 *                        Type definitions                                    *
 ******************************************************************************/

typedef enum
{
    VPU_NO_INT_PENDING,
    VPU_INT_PROC_DONE,
    VPU_INT_PROC_ERR,
    VPU_INT_UNDERRUN,
    VPU_INT_OVERRUN,
    VPU_INT_SLICE_DONE,
}enum_vpu_int_source_t;

/*
 * @brief  struct  vpu_res_info_t
 *         Description of the hardware information for a jpeg codec
 * @param  res_glb_idx  global id in domain
 * @param  res_describe sample resource description
 * @param  paddr_t      register address
 * @param  size_t       register size
 */
struct vpu_res_info_t {
    unsigned int res_glb_idx;
    unsigned long phy_addr;
    unsigned long addr_range;
};

/*
 * @brief  struct  vpu_src_t
 *         Describes jpeg codec resource in the domain
 * @param  version       codec version
 * @param  res_category  codec category description
 * @param  res_num       codec number
 * @param  res_info      codec resource detain
 */
struct vpu_src_t {
    struct vpu_res_info_t res_info;
    int   res_num;
};


//forward declaration
struct vpu_codec_instance_t;
typedef void (*inst_cb_handler_t)(struct vpu_codec_instance_t* codec,void* args);
/*
 * @brief  struct  instance_cb_t
 * @Describes               codec callback
 * @param  cb               codec callback prototype
 * @param  args             callback arguments
 */
typedef struct instance_cb_t
{
    inst_cb_handler_t handler;
    void* args;
}instance_cb_t;

/*
 * @brief  struct  vpu_codec_instance_t
 * @Describes               codec instance type
 * @param  instance         codec instance
 * @param  signal           signal from interrupt
 * @param  on_fin           callback on enc/dec finish
 * @param  on_err           callback on error
 * @param  on_underrun      callback on underrun, e.g. bitstream empty
 * @param  on_overrun       callback on overrun, e.g. frame buffer full
 */

typedef struct vpu_codec_instance_t
{
    JpgInst* instance;
    instance_cb_t on_fin;
    instance_cb_t on_err;
    instance_cb_t on_underrun;
    instance_cb_t on_overrun;

}* vpu_codec_handle_t;

/*
 * @brief  struct  vpu_instance
 *         Describes hardware instance
 * @param  vpu_res  codec resource info
 * @param  irq_num  codec irq num
 */
struct vpu_instance {
    struct vpu_src_t  vpu_res;
    unsigned int irq_num;
    int  using_id;
    unsigned int glb_idx;
    unsigned int int_res;
    int instidx;
    event_t signal;
};

/*
 * Decode param
 */
typedef JpgRet              vpuRet;
typedef FrameFormat         vpuFrameFormat;

typedef JpgDecOpenParam     vpuDecOpenParam;
typedef JpgDecInitialInfo   vpuDecInitialInfo;
typedef JpgDecOutputInfo    vpuDecOutputInfo;
typedef JpgDecParam         vpuDecParam;

/*
 * Encode param
 */
typedef JpgEncOpenParam     vpuEncOpenParam;
typedef JpgEncOutputInfo    vpuEncOutputInfo;
typedef JpgEncParam         vpuEncParam;


/*
 * Frame buffer param
 */
typedef FrameBuffer         vpuFrameBuffer;
typedef jpu_buffer_t        vpu_buffer_t;
typedef JpgCommand          vpuCommand;

typedef struct {
    Uint32       Format;
    Uint32       Index;
    jpu_buffer_t vbY;
    jpu_buffer_t vbCb;
    jpu_buffer_t vbCr;
    Uint32       strideY;
    Uint32       strideC;
} FRAME_BUF;

typedef struct {
    FRAME_BUF frameBuf[MAX_FRAME];
    jpu_buffer_t vb_base;
    int instIndex;
    int last_num;
    int last_addr;
} fb_context;
/*******************************************************************************
 *                        Global function declarations                         *
 ******************************************************************************/
/*
 * @brief Create hardware resource handle of mjpeg codec
 *
 * @param  handle  the handle that needs to be created
 * @param  vpuGlbIdx  globle id in domain
 * @return success true; false failure
 * @note  Inerface both support fpga and real hardware,
 *        if run in fpag,  must be define (CODAJ12_FPGA = 1)
 *        and APB_MJPEG_BASE
 */
bool hal_vpu_create_handle(void **handle, uint32_t vpuGlbIdx);

/**
 * @brief Release hardware handle of mjpeg codec
 *
 * @param  resHandle  the handle that needs to be destroried
 * @return success true; false failure
 * @note   must be set handle = NULL in caller
 */
bool hal_vpu_release_handle(void *resHandle);

/**
 * @brief Initial the vpu
 *
 * @param resHandle the opened handle of resource
 * @return success 0 failture != 0
 * @note
 */
vpuRet hal_vpu_init(void *resHandle);

/**
 * @brief Deinit the vpu
 *
 * @param resHandle the opened handle of resource
 * @return void
 * @note
 */
void hal_vpu_deinit(void *resHandle);

/**
 * @brief Get vpu version
 *
 * @param resHandle the opened handle of resource
 * @param decHandle the opened handle of instance
 * @param apiVersion api version
 * @param hwVersion  hardware version
 * @param hwProductId product id
 * @return success 0 failture != 0
 * @note
 */
vpuRet hal_vpu_get_version(void *resHandle, Uint32 *apiVersion,
                           Uint32 *hwVersion, Uint32 *hwProductId);

/**
 * @brief Get instance num opened
 *
 * @param resHandle the opened handle of resource
 * @param decHandle the opened handle of instance
 * @return the num of have been opened
 * @note
 */
int hal_vpu_get_open_instance_num(void *resHandle);

/**
 * @brief Open vpu for decode
 *
 * @param resHandle the opened handle of vpu resource
 * @param decHandl the opened handle of instance
 * @param openParam  param for opening
 * @return success 0 failture != 0
 * @note
 */
vpuRet hal_vpu_dec_open(void *resHandle, vpu_codec_handle_t *decHandle,
                        vpuDecOpenParam *openParam);

/**
 * @brief Close the vpu
 *
 * @param resHandle the opened handle of resource
 * @param decHandle  the opened handle of instance
 * @return success 0 failture != 0
 * @note
 */
vpuRet hal_vpu_dec_close(void *resHandle, vpu_codec_handle_t decHandle);

/**
 * @brief Get initial info by vpu
 *
 * @param resHandle the opened handle of resource
 * @param decHandle the opened handle of instance
 * @param info  info want to get
 * @return success 0 failture != 0
 * @note
 */
vpuRet hal_vpu_dec_get_init_info(void *resHandle, vpu_codec_handle_t decHandle,
                                 vpuDecInitialInfo *info);

/**
 * @brief create a dedicated frame buffer for current stream.
 *
 * @param instIdx the opened decoder instance idx.
 * @param subsample subsample format.
 * @param cbcrIntlv cbcrinterleave format.
 * @param packed  pixel packed format.
 * @param rotation  rotation, in degree.
 * @param scalerOn  need scale or not.
 * @param width, height, bitdepth, num: depends on stream init info.
 * @return frame size per frame or 0 when create fail
 *
 */
Uint32 hal_vpu_create_frame_buffer(FrameFormat subsample, CbCrInterLeave cbcrIntlv, PackedFormat packed, EndianMode endian,
                         Uint32 rotation, BOOL scalerOn, Uint32 width, Uint32 height, Uint32 bitDepth, Uint32 num, vpuFrameBuffer** frame_buffer);


void hal_vpu_free_frame_buffer(vpuFrameBuffer* frame_buffer);

/**
 * @brief Used for registering frame buffers with acquired info from hal_vpu_dec_init_info()
 *        function
 *
 * @param resHandle the opened handle of resource
 * @param decHandle the opened handle of instance
 * @param bufArray  description the framebuffer info
 * @return success 0 failture != 0
 * @note
 */
vpuRet hal_vpu_dec_register_framebuffer(void *resHandle,
                                        vpu_codec_handle_t decHandle, vpuFrameBuffer *bufArray, int num, int stride);

/**
 * @brief Get bitstream buffer info
 *
 * @param resHandle the opened handle of resource
 * @param decHandle the opened handle of instance
 * @param prdPrt [output]  read pointer for the current decoder instance
 * @param pwrPtr [output] write pointer for the current decoder instance
 * @param size  specifying the available space in bitstream buffer for current decoder instance
 * @return success 0 failture != 0
 * @note app must feed the decoder with bitstream, known where to put bitstream and
 *       the maximum size. App can get info by calling this functioin.
 */
vpuRet hal_vpu_dec_get_bitstream_buffer(void *resHandle,
                                        vpu_codec_handle_t decHandle, PhysicalAddress *prdPrt, PhysicalAddress *pwrPtr,
                                        int *size );

/**
 * @brief Let decoder know how much bitstream has been transferred to the
 *        address obtained from hal_vpu_get_bitstream_buffer().
 *
 * @param resHandle the opened handle of resource
 * @param decHandle the opened handle of instance
 * @param size bits transferred into bitstream buffer for the current decoder instance
 * @return success 0 failture != 0
 * @note
 */
vpuRet hal_vpu_dec_update_bitstream_buffer(void *resHandle,
        vpu_codec_handle_t decHandle, int size);

/**
 * @brief Hardware reset vpu
 *
 * @param resHandle the opened handle of resource
 * @return success 0 failture != 0
 * @note
 */
vpuRet hal_vpu_hardware_reset(void *resHandle);

/**
 * @brief Software reset vpu
 *
 * @param resHandle the opened handle of resource
 * @param decHandle the opened handle of instance
 * @return success 0 failture != 0
 * @note
 */
vpuRet hal_vpu_software_reset(void *resHandle, vpu_codec_handle_t  vpuOperHandle);

/**
 * @brief Starts decoding one frame
 *
 * @param resHandle the opened handle of resource
 * @param decHandle the opened handle of instance
 * @param param describes picture decoding param for given decoder instance
 * @return success 0 failture != 0
 * @note
 */
vpuRet hal_vpu_dec_start_one_frame(void *resHandle, vpu_codec_handle_t decHandle,
                                   vpuDecParam *param );

/**
 * @brief Get the info of output of decoding
 *
 * @param resHandle the opened handle of resource
 * @param decHandle the opened handle of instance
 * @param info  describes picture decoding results for the current decoder instance
 * @return success 0 failture != 0
 * @note
 */
vpuRet hal_vpu_dec_get_output_info(void *resHandle, vpu_codec_handle_t decHandle,
                                   vpuDecOutputInfo *info, int reson);

/**
 * @brief Give commands to vpu for re-configuring decode operation
 *
 * @param resHandle the opened handle of resource
 * @param decHandle the opened handle of instance
 * @param cmd a avriable specifying the given command of vpuCommand type
 * @param param  data structure
 * @return success 0 failture != 0
 * @note
 */
vpuRet hal_vpu_dec_give_command(void *resHandle, vpu_codec_handle_t decHandle,
                                vpuCommand cmd, void *parameter);

/**
 * @brief Specifies the location of read pointer in bitstream buffer.
 *
 * @param resHandle the opened handle of resource
 * @param decHandle the opened handle of instance
 * @param addr updated read or write pointer
 * @param updateWrPtr flag wherther to update a write pointer
 * @return success 0 failture != 0
 * @note function allows to flush up the bitstream bufffer at once
 */
vpuRet hal_vpu_dec_set_bitstream_ptr(void *resHandle,
                                     vpu_codec_handle_t decHandle, PhysicalAddress addr, bool updateWrPtr);

/**
 * @brief Waits until an Interrupt arises and returns the
 *        Interrupt reason if it occurs
 *
 * @param resHandle the opened handle of resource
 * @param decHandle the opened handle of instance
 * @param timeout  a timeout value in millisecond
 * @return -1 timeout happened
 *         -2 normal operation by instance controller failed
 *         No-zero value Interrupt reason
 * @note
 */
int hal_vpu_wait_interrupt(void *resHandle, vpu_codec_handle_t vpuOperHandle,
                           int timeout);


/**
 * @brief convert irq source raw value to enumrator.
 */
enum_vpu_int_source_t hal_vpu_get_irq_source(Int32 source);

/**
 * @brief Get vpu status of interrupt.
 *
 * @param resHandle the opened handle of resource
 * @param decHandle the opened handle of instance
 * @return  No interrupt 0
 * @note
 */
unsigned int hal_vpu_get_status(void *resHandle, vpu_codec_handle_t vpuOperHandle);

/**
 * @brief Clearing Interrupt
 *
 * @param resHandle the opened handle of resource
 * @param decHandle the opened handle of instance
 * @return  void
 * @note
 */
void hal_vpu_clear_interrupt(void *resHandle, vpu_codec_handle_t vpuOperHandle,
                             unsigned int val);

/**
 * @brief Checking the vpu status
 *
 * @param resHandle the opened handle of resource
 * @param decHandle the opened handle of instance
 * @return busy  != 0
 * @note
 */
int hav_vpu_is_busy(void *resHandle, vpu_codec_handle_t vpuOperHandle);

/**
 * @brief Checking vpu status
 *
 * @param resHandle the opened handle of resource
 * @param decHandle the opened handle of instance
 * @return  inited 0
 * @note
 */
unsigned int hal_vpu_is_init(void *resHandle, vpu_codec_handle_t vpuOperHandle);

/**
 * @brief Allocate memory for yuv/bitstream
 *
 * @param vb memory info
 * @return success 0 failture != 0
 * @note
 */
int hal_vpu_allocate_dma_memory(vpu_buffer_t *vb);

/**
 * @brief write dma memory
 *
 * @param vb dest dma memory block
 * @param data source data
 * @param len size
 * @param endian endian
 * @return success 0 failture != 0
 * @note
 */
int hal_vpu_write_dma_memory(vpu_buffer_t* vb, unsigned char *data, int len, int endian);

/**
 * @brief read dma memory
 *
 * @param vb source dma memory block
 * @param data dest data area
 * @param len size
 * @param endian endian
 * @return success 0 failture != 0
 * @note
 */
int hal_vpu_read_dma_memory(vpu_buffer_t* vb, unsigned char *data, int len, int endian);
/**
 * @brief Free dma buffer
 *
 * @param vb
 * @return success 0 failture != 0
 * @note
 */
void hal_vpu_free_dma_memory(vpu_buffer_t *vb);

/**
 * @brief  Open vpu for new encode instance
 *
 * @param resHandle the opened handle of resource
 * @param encHandle the opened handle of instance
 * @param pointer to vpuEncOpenParam type structure
 * @return success 0 failture != 0
 * @note
 */
vpuRet hal_vpu_enc_open(void *resHandle, vpu_codec_handle_t *encHandle,
                        vpuEncOpenParam *openParam);

/**
 * @brief Close current vpu instance
 *
 * @param resHandle the opened handle of resource
 * @param encHandle the opened handle of instance
 * @param
 * @return success 0 failture != 0
 * @note
 */
vpuRet hal_vpu_enc_close(void *resHandle, vpu_codec_handle_t encHandle);

/**
 * @brief
 *
 * @param resHandle the opened handle of resource
 * @param encHandle the opened handle of instance
 * @param prdPrt A stream buffer read pointer for current encode instance
 * @param pwrPtr A stream buffer write pointer for current encode instance
 * @param size  A variable specifying the available space in bitstream
 *              buffer for current encode instance
 * @return success 0 failture != 0
 * @note
 */
vpuRet hal_vpu_enc_get_bitstream_buffer(void *resHandle,
                                        vpu_codec_handle_t encHandle, PhysicalAddress *prdPrt, PhysicalAddress *pwrPtr,
                                        int *size);

/**
 * @brief Let encoder know how much bitstream has been transferred
 *        from the address obtained from hal_vpu_enc_get_bitstream_buffer()
 *
 * @param resHandle the opened handle of resource
 * @param encHandle the opened handle of instance
 * @param size  tell vpu wroten size
 * @return success 0 failture != 0
 * @note
 */
vpuRet hal_vpu_enc_update_bitstream_buffer(void *resHandle,
        vpu_codec_handle_t encHandle, int size);

/**
* @brief Starts encoding one frame
*
* @param resHandle the opened handle of resource
* @param encHandle the opened handle of instance
* @param param  the param for endcode
* @return success 0 failture != 0
* @note Returning from this function does not mean the completion
*       of encoding one frames, and it is just that encoding one was
*       initiated.
*/
vpuRet hal_vpu_enc_start_one_frame(void *resHandle, vpu_codec_handle_t encHandle,
                                   vpuEncParam *param);

/**
 * @brief Gets info of the output of encoding
 *
 * @param resHandle the opened handle of resource
 * @param encHandle the opened handle of instance
 * @param info get the out info of have been encoded
 * @return success 0 failture != 0
 * @note  Know about picture type, the address and size of the generated bitstream
 *        the num of generated slices, the end address of the Slices, the end address of
 *        the slices and so on
 */
vpuRet hal_vpu_enc_get_output_info(void *resHandle, vpu_codec_handle_t encHandle,
                                   vpuEncOutputInfo *info, int reson);

/**
 * @brief Give the command to vpu
 *
 * @param resHandle the opened handle of resource
 * @param encHandle the opened handle of instance
 * @param cmd  cmd to set vpu
 * @param parameter the param set to vpu
 * @return success 0 failture != 0
 * @note
 */
vpuRet hal_vpu_enc_give_command(void *resHandle, vpu_codec_handle_t encHandle,
                                vpuCommand cmd, void *parameter);

/**
 * @brief Get irq number of vpu
 *
 * @param resHandle the opened handle of resource
 * @return irq vector
 * @note
 */
int hal_vpu_get_irq_vector(void *resHandle);

/**
 * @brief Register callback function set.
 *
 * @param resHandle the opened handle of resource
 * @param decHandle the opened handle of instance
 * @param cb callback function.
 * @param args  param for callback function
 * @return void
 *
 */

/* callback invoked while process finish. */
void hal_vpu_register_callback_on_fin(void *resHandle, vpu_codec_handle_t decHandle,
                                  inst_cb_handler_t cb, void *args);

/* callback invoked while occur process error. */
void hal_vpu_register_callback_on_err(void *resHandle, vpu_codec_handle_t decHandle,
                                  inst_cb_handler_t cb, void *args);

/* callback invoked while occur underrun. e.g. bs empty */
void hal_vpu_register_callback_on_underrun(void *resHandle, vpu_codec_handle_t decHandle,
                                  inst_cb_handler_t cb, void *args);

/* callback invoked while occur overrun. e.g. frm full */
void hal_vpu_register_callback_on_overrun(void *resHandle, vpu_codec_handle_t decHandle,
                                  inst_cb_handler_t cb, void *args);

#ifdef __cplusplus
}
#endif

#endif  /* End __CODAJ12_HAL_H__ */

