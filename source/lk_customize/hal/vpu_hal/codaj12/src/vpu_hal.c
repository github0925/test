/**
 * File: vpu_hal.c
 * Brief: Imple vpu driver adapter layer
 * Data:  10/11/2019
 *
 * Copyright (C) 2019 Semidrive Technology Co., Ltd.
 * Author: chentianming<tianming.chen@semidrive.com>
 *
 */
/*******************************************************************************
 *                       Include header files                                 *
 ******************************************************************************/
#include <platform/interrupts.h>
#include <sys/types.h>
#include <platform/debug.h>
#include <trace.h>
#include <string.h>
#include <assert.h>
#include <platform.h>
#include <kernel/thread.h>
#include <kernel/event.h>
#include <kernel/spinlock.h>
#include <stdlib.h>
#include <err.h>

#include "vpu_hal.h"
#include "jpuapifunc.h"
#include "irq.h"

/*******************************************************************************
 *                        Macro definitions                                   *
 ******************************************************************************/
#define CHECK_PARAM(X)   do {if(!(X))  { return JPG_RET_INVALID_PARAM; }} while(0)
#define VPU_MJPEG_REGISTER_SIZE                         0x1000

/*******************************************************************************
 *                        Date struct  declarations                          *
 ******************************************************************************/


/*******************************************************************************
 *                        Global function declarations                          *
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
bool hal_vpu_create_handle(void **handle, uint32_t vpuGlbIdx)
{
    int idx = 0;
    unsigned long addr = 0;
    struct vpu_instance  *vpu_inst = NULL;

    if (NULL == handle)
        return false;

    if (RES_MJPEG_MJPEG != vpuGlbIdx) {
        log_err("vpu create handle glbidx:%d error \n", vpuGlbIdx);
        return false;
    }

    if (0 != res_get_info_by_id(vpuGlbIdx, &addr, &idx)) {
        log_err("vpu create instance error, can not get resource \n");
        return false;
    }

    if (NULL == (vpu_inst = (struct vpu_instance *) malloc(sizeof(
                                struct vpu_instance)))) {
        log_err("vpu create handle memory alloce error \n");
        return false;
    }

    memset(vpu_inst, 0, sizeof(struct vpu_instance));
    vpu_inst->vpu_res.res_info.phy_addr = addr;
    vpu_inst->vpu_res.res_info.addr_range = VPU_MJPEG_REGISTER_SIZE;
    vpu_inst->irq_num = MJPEG_O_INTRPT_REQ_NUM;
    vpu_inst->using_id = idx;

#if (1 == CODAJ12_FPGA)
    vpu_inst->vpu_res.res_info.phy_addr = APB_MJPEG_BASE;
    vpu_inst->vpu_res.res_info.addr_range = VPU_MJPEG_REGISTER_SIZE;
    vpu_inst->irq_num = MJPEG_IRQ_NUM;
    vpu_inst->using_id = 0;
#endif

    log_debug("vpu codaj12 handle info: global_idx %d; regsiter_phy %p; register_size 0x%x, irq_num %d, using id %d\n",
              vpuGlbIdx,
              (void *)vpu_inst->vpu_res.res_info.phy_addr,
              (int)vpu_inst->vpu_res.res_info.addr_range,
              (int)vpu_inst->irq_num,
              (int)vpu_inst->using_id);

    *handle = vpu_inst;
    return true;
}

/**
 * @brief Release hardware handle of mjpeg codec
 *
 * @param  resHandle  the handle that needs to be destroried
 * @return success true; false failure
 * @note   must be set handle = NULL in caller
 */
bool hal_vpu_release_handle(void *resHandle)
{
    if (resHandle) {
        free(resHandle);
        resHandle = NULL;
    }

    log_debug("codaj12 release handle success \n");
    return true;
}

//forward declaration
static enum handler_return __hal_vpu_low_level_int_handler(void *arg);

/**
 * @brief Initial the vpu
 *
 * @param resHandle the opened handle of resource
 * @return success 0 failture != 0
 * @note
 */
vpuRet hal_vpu_init(void *resHandle)
{
    CHECK_PARAM(resHandle);
    vpuRet ret = 0;
    struct vpu_instance* vpu_inst = (struct vpu_instance *)resHandle;
    event_init(&vpu_inst->signal,0,false);

    ret = JPU_Init(vpu_inst->vpu_res.res_info.phy_addr,vpu_inst->vpu_res.res_info.addr_range);

    register_int_handler(vpu_inst->irq_num, __hal_vpu_low_level_int_handler, vpu_inst);

    unmask_interrupt(vpu_inst->irq_num);

    return ret;
}

/**
 * @brief Deinit the vpu
 *
 * @param resHandle the opened handle of resource
 * @return void
 * @note
 */
void hal_vpu_deinit(void *resHandle)
{
    struct vpu_instance* vpu_inst = (struct vpu_instance *)resHandle;
    event_destroy(&vpu_inst->signal);
    JPU_DeInit();
}

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
                           Uint32 *hwVersion, Uint32 *hwProductId)
{
    CHECK_PARAM(resHandle);
    return JPU_GetVersionInfo(apiVersion, hwVersion, hwProductId);
}

/**
 * @brief Get instance num opened
 *
 * @param resHandle the opened handle of resource
 * @param decHandle the opened handle of instance
 * @return the num of have been opened
 * @note
 */
int hal_vpu_get_open_instance_num(void *resHandle)
{
    CHECK_PARAM(resHandle);
    return JPU_GetOpenInstanceNum();
}

/**
 * @brief Open vpu for decode
 *
 * @param resHandle the opened handle of vpu resource
 * @param phandle the opened handle of instance
 * @param openParam  param for opening
 * @return success 0 failture != 0
 * @note
 */
vpuRet hal_vpu_dec_open(void *resHandle, vpu_codec_handle_t *pdecHandle,
                        vpuDecOpenParam *openParam)
{
    JpgRet      ret;
    CHECK_PARAM(resHandle && pdecHandle && openParam);
    *pdecHandle = malloc(sizeof(struct vpu_codec_instance_t));
    if(*pdecHandle == NULL)
    {
        printf("dec handle malloc fail\n");
        return JPG_RET_FAILURE;
    }

    ret = JPU_DecOpen(&((*pdecHandle)->instance), openParam);

    if(ret != JPG_RET_SUCCESS){
        free(*pdecHandle);
    }

    return ret;
}

/**
 * @brief Close the vpu
 *
 * @param resHandle the opened handle of resource
 * @param decHandle  the opened handle of instance
 * @return success 0 failture != 0
 * @note
 */
vpuRet hal_vpu_dec_close(void *resHandle, vpu_codec_handle_t decHandle)
{
    CHECK_PARAM(resHandle && decHandle);
    vpuRet ret = JPG_RET_SUCCESS;
    ret |= JPU_DecClose(decHandle->instance);
    if(decHandle) {
        free(decHandle);
        decHandle = NULL;
    }
    return ret;
}
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
                                 vpuDecInitialInfo *info)
{
    CHECK_PARAM(resHandle && decHandle && info);
    return JPU_DecGetInitialInfo(decHandle->instance, info);

}

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
                                        vpu_codec_handle_t decHandle, vpuFrameBuffer *bufArray, int num, int stride)
{
    CHECK_PARAM(resHandle && decHandle && bufArray);
    return JPU_DecRegisterFrameBuffer(decHandle->instance, bufArray, num, stride);
}

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
                                        int *size )
{
    CHECK_PARAM(resHandle && decHandle);
    return JPU_DecGetBitstreamBuffer(decHandle->instance, prdPrt, pwrPtr, size);
}

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
        vpu_codec_handle_t decHandle, int size)
{
    CHECK_PARAM(resHandle && decHandle);
    return JPU_DecUpdateBitstreamBuffer(decHandle->instance, size);
}

/**
 * @brief Hardware reset vpu
 *
 * @param resHandle the opened handle of resource
 * @return success 0 failture != 0
 * @note
 */
vpuRet hal_vpu_hardware_reset(void *resHandle)
{
    CHECK_PARAM(resHandle);
    return JPU_HWReset();
}

/**
 * @brief Software reset vpu
 *
 * @param resHandle the opened handle of resource
 * @param decHandle the opened handle of instance
 * @return success 0 failture != 0
 * @note
 */
vpuRet hal_vpu_software_reset(void *resHandle, vpu_codec_handle_t  codecHandle)
{
    CHECK_PARAM(resHandle && codecHandle);
    return JPU_SWReset(codecHandle->instance);
}

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
                                   vpuDecParam *param )
{
    CHECK_PARAM(resHandle && decHandle && param);
    return JPU_DecStartOneFrame(decHandle->instance, param);
}

/**
 * @brief Get the info of output of decoding
 *
 * @param resHandle the opened handle of resource
 * @param decHandle the opened handle of instance
 * @param info  describes picture decoding results for the current decoder instance
 * @return success 0 failture != 0
 * @note    ** THIS FUNCTION WILL AUTOMATICALLY CLEAR INTERRUPT **
 */
vpuRet hal_vpu_dec_get_output_info(void *resHandle, vpu_codec_handle_t decHandle,
                                   vpuDecOutputInfo *info, int reson)
{
    CHECK_PARAM(resHandle && decHandle && info);
    return JPU_DecGetOutputInfoEx(decHandle->instance, info, reson);
}

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
                                vpuCommand cmd, void *parameter)
{
    CHECK_PARAM(resHandle && decHandle);
    return JPU_DecGiveCommand(decHandle->instance, cmd, parameter);

}
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
                                     vpu_codec_handle_t decHandle, PhysicalAddress addr, bool updateWrPtr)
{
    CHECK_PARAM(resHandle && decHandle);
    return JPU_DecSetRdPtr(decHandle->instance, addr, updateWrPtr);
}

/**
 * @brief Checking the vpu status
 *
 * @param resHandle the opened handle of resource
 * @param decHandle the opened handle of instance
 * @return busy  != 0
 * @note
 */
int hav_vpu_is_busy(void *resHandle, vpu_codec_handle_t codecHandle)
{
    CHECK_PARAM(resHandle && codecHandle);
    return JPU_IsBusy(codecHandle->instance);
}

/**
 * @brief Checking vpu status
 *
 * @param resHandle the opened handle of resource
 * @param decHandle the opened handle of instance
 * @return  inited 0
 * @note
 */
unsigned int hal_vpu_is_init(void *resHandle, vpu_codec_handle_t codecHandle)
{
    CHECK_PARAM(resHandle && codecHandle);
    return JPU_IsInit();

}

/**
 * @brief Allocate memory for yuv/bitstream
 *
 * @param vb memory info
 * @return success 0 failture != 0
 * @note
 */
int hal_vpu_allocate_dma_memory(vpu_buffer_t *vb)
{
    CHECK_PARAM(vb);
    return jdi_allocate_dma_memory(vb);

}

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
int hal_vpu_write_dma_memory(vpu_buffer_t* vb, unsigned char *data, int len, int endian)
{
    CHECK_PARAM(vb);
    return jdi_write_memory(vb->phys_addr,data,len,endian);
}

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
int hal_vpu_read_dma_memory(vpu_buffer_t* vb, unsigned char *data, int len, int endian)
{
    CHECK_PARAM(vb);
    return jdi_read_memory(vb->phys_addr,data,len,endian);
}

/**
 * @brief get decoder inst idx for further use.
 *
 * @param decHandle
 * @return < 0 fail or inst idx.
 * @note
 */
Int32 hal_vpu_get_decoder_inst_idx(vpu_codec_handle_t decHandle)
{
    if(!decHandle)
        return -1;

    return decHandle->instance->instIndex;
}

/**
 * @brief Free dma buffer
 *
 * @param vb
 * @return success 0 failture != 0
 * @note
 */
void hal_vpu_free_dma_memory(vpu_buffer_t *vb)
{
    jdi_free_dma_memory(vb);
}

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
                        vpuEncOpenParam *openParam)
{
    CHECK_PARAM(resHandle && encHandle && openParam);

    *encHandle = malloc(sizeof(struct vpu_codec_instance_t));
    if(NULL == *encHandle)
    {
        log_err("enc handle malloc fail \n");
        return -1;
    }
    return JPU_EncOpen(&((*encHandle)->instance), openParam);
}

/**
 * @brief Close current vpu instance
 *
 * @param resHandle the opened handle of resource
 * @param encHandle the opened handle of instance
 * @param
 * @return success 0 failture != 0
 * @note
 */
vpuRet hal_vpu_enc_close(void *resHandle, vpu_codec_handle_t encHandle)
{

    CHECK_PARAM(resHandle && encHandle);
    vpuRet ret = JPG_RET_SUCCESS;
    ret |= JPU_DecClose(encHandle->instance);
    if(encHandle) {
        free(encHandle);
        encHandle = NULL;
    }
    return ret;
}

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
                                        int *size)
{
    CHECK_PARAM(resHandle && encHandle);
    return JPU_EncGetBitstreamBuffer(encHandle->instance, prdPrt, pwrPtr, size);
}

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
        vpu_codec_handle_t encHandle, int size)
{
    CHECK_PARAM(resHandle && encHandle);
    return JPU_EncUpdateBitstreamBuffer(encHandle->instance, size);

}

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
                                   vpuEncParam *param)
{
    CHECK_PARAM(resHandle && encHandle);
    return JPU_EncStartOneFrame(encHandle->instance, param);
}

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
                                   vpuEncOutputInfo *info, int reson)
{
    CHECK_PARAM(resHandle && encHandle && info);
    return JPU_EncGetOutputInfoEx(encHandle->instance, info, reson);
}

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
                                vpuCommand cmd, void *parameter)
{
    CHECK_PARAM(resHandle && encHandle);
    return JPU_EncGiveCommand(encHandle->instance, cmd, parameter);
}

/**
 * @brief Get irq number of vpu
 *
 * @param resHandle the opened handle of resource
 * @return irq vector
 * @note
 */
int hal_vpu_get_irq_vector(void *resHandle)
{
    int vector = -1;
    struct vpu_instance *vpu_inst = NULL;

    CHECK_PARAM(resHandle);
    vpu_inst = (struct vpu_instance *) resHandle;
    vector = vpu_inst->irq_num;

    log_debug("Get the vpu irq num %d\n", vector);
    return vector;
}

enum_vpu_int_source_t hal_vpu_get_irq_source(Int32 source)
{
    ASSERT(source != -2);//instance controller fail
    if(source & (1<<INT_JPU_DONE)) {
        return VPU_INT_PROC_DONE;
    }
    else if(source & (1<<INT_JPU_ERROR)) {
        return VPU_INT_PROC_ERR;
    }
    else if(source & (1<<INT_JPU_BIT_BUF_EMPTY)) {
        return VPU_INT_UNDERRUN;
    }
    // else if(source & ((1<<INT_JPU_BIT_BUF_FULL){
    //     return VPU_INT_OUT_EMPTY;
    // }  available on encoder
    else if(source & (1<<INT_JPU_SLICE_DONE)) {
        return VPU_INT_SLICE_DONE;
    }
    else {
        return VPU_NO_INT_PENDING;
    }
}

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
 * @note    This function may block the current thread to be waiting
 * state and be awaken once interrupt of the codec handle was asserted.
 */
int hal_vpu_wait_interrupt(void *resHandle, vpu_codec_handle_t codecHandle,
                           int timeout)
{
    CHECK_PARAM(resHandle && codecHandle);
    Int32 source = 0;
    struct vpu_instance* vpu_inst = (struct vpu_instance *)resHandle;

    if(ERR_TIMED_OUT ==  event_wait_timeout(&vpu_inst->signal,timeout))
    {
        log_err("vpu waiting interrupt timedout!\n");
        return -1;
    }
    source = vpu_inst->int_res;
    enum_vpu_int_source_t irq_ret = hal_vpu_get_irq_source(source);
    switch(irq_ret)
    {
    case VPU_NO_INT_PENDING:
        log_err("unexpected interrupt:INT_STS:%x\n",source);
    break;

    case VPU_INT_PROC_DONE:
        if(codecHandle->on_fin.handler)
        {
            codecHandle->on_fin.handler(codecHandle, codecHandle->on_fin.args);
        }
    break;

    case VPU_INT_PROC_ERR:
        if(codecHandle->on_err.handler)
        {
            codecHandle->on_err.handler(codecHandle, codecHandle->on_err.args);
        }
    break;

    case VPU_INT_UNDERRUN:
        if(codecHandle->on_underrun.handler)
        {
            codecHandle->on_underrun.handler(codecHandle, codecHandle->on_underrun.args);
        }
    break;

    case VPU_INT_OVERRUN:
        if(codecHandle->on_overrun.handler)
        {
            codecHandle->on_overrun.handler(codecHandle, codecHandle->on_overrun.args);
        }
    break;

    case VPU_INT_SLICE_DONE:
        log_err("unimplemented slice done handler\n");
    break;

    }

    event_unsignal(&vpu_inst->signal);

    return source;

}

/**
 * @brief Get vpu status of interrupt.
 *
 * @param resHandle the opened handle of resource
 * @param decHandle the opened handle of instance
 * @return  No interrupt 0
 * @note
 */
unsigned int hal_vpu_get_status(void *resHandle, vpu_codec_handle_t codecHandle)
{
    CHECK_PARAM(resHandle && codecHandle);
    return JPU_GetStatus(codecHandle->instance);
}

/**
 * @brief Clearing Interrupt
 *
 * @param resHandle the opened handle of resource
 * @param decHandle the opened handle of instance
 * @return  void
 * @note
 */
void hal_vpu_clear_interrupt(void *resHandle, vpu_codec_handle_t codecHandle,
                             unsigned int val)
{
    if ((NULL == resHandle) || (NULL == codecHandle)) {
        log_err("hal vpu instance handle err...\n");
        return;
    }

    JPU_ClrStatus(codecHandle->instance, val);

}

static enum handler_return __hal_vpu_low_level_int_handler(void *arg)
{
    struct vpu_instance* vpu_inst = (struct vpu_instance *)arg;
    //sort out interrupt source
    JPU_InterruptSortOut(&vpu_inst->instidx,&vpu_inst->int_res);
    //clear interrupt
    JPU_ClrStatusEx(vpu_inst->instidx, vpu_inst->int_res);
    //signal to bottom part
    event_signal(&vpu_inst->signal,false);

    return 0;
}


static inline void __hal_vpu_register_callback_core(instance_cb_t* cbc, inst_cb_handler_t handler, void *args)
{
    cbc->handler = handler;
    cbc->args = args;
}


void hal_vpu_register_callback_on_fin(void *resHandle, vpu_codec_handle_t decHandle,
                                  inst_cb_handler_t cb, void *args)
{
    if (NULL == resHandle && NULL == decHandle) {
        log_err("Handle param error now...\n");
        return;
    }
    __hal_vpu_register_callback_core(&decHandle->on_fin,cb,args);
}

void hal_vpu_register_callback_on_err(void *resHandle, vpu_codec_handle_t decHandle,
                                  inst_cb_handler_t cb, void *args)
{
    if (NULL == resHandle && NULL == decHandle) {
        log_err("Handle param error now...\n");
        return;
    }
    __hal_vpu_register_callback_core(&decHandle->on_err,cb,args);
}

void hal_vpu_register_callback_on_underrun(void *resHandle, vpu_codec_handle_t decHandle,
                                  inst_cb_handler_t cb, void *args)
{
    if (NULL == resHandle && NULL == decHandle) {
        log_err("Handle param error now...\n");
        return;
    }
    __hal_vpu_register_callback_core(&decHandle->on_underrun,cb,args);
}

void hal_vpu_register_callback_on_overrun(void *resHandle, vpu_codec_handle_t decHandle,
                                  inst_cb_handler_t cb, void *args)
{
    if (NULL == resHandle && NULL == decHandle) {
        log_err("Handle param error now...\n");
        return;
    }
    __hal_vpu_register_callback_core(&decHandle->on_overrun,cb,args);
}

/**
 * @brief internal function. Calc frame stride for further use.
 *
 */
static void __hal_vpu_get_frame_buf_stride(FrameFormat subsample, CbCrInterLeave cbcrIntlv, PackedFormat packed, BOOL scalerOn,
                              Uint32 width, Uint32 height, Uint32 bytePerPixel,
                              Uint32 *oLumaStride, Uint32 *oLumaHeight, Uint32 *oChromaStride, Uint32 *oChromaHeight)
{
    Uint32 lStride, cStride;
    Uint32 lHeight, cHeight;

    lStride = JPU_CEIL(8, width);
    lHeight = height;
    cHeight = height/2;

    if (packed == PACKED_FORMAT_NONE) {
        Uint32 chromaDouble = (cbcrIntlv == CBCR_SEPARATED) ? 1 : 2;

        switch (subsample) {
            case FORMAT_400:
                cStride = 0;
                cHeight = 0;
                break;
            case FORMAT_420:
                cStride = (lStride/2)*chromaDouble;
                cHeight = height/2;
                break;
            case FORMAT_422:
                cStride = (lStride/2)*chromaDouble;
                cHeight = height;
                break;
            case FORMAT_440:
                cStride = lStride*chromaDouble;
                cHeight = height/2;
                break;
            case FORMAT_444:
                cStride = lStride*chromaDouble;
                cHeight = height;
                break;
            default:
                cStride = 0;
                lStride = 0;
                cHeight = 0;
                break;
        }
    }
    else {
        switch (packed) {
            case PACKED_FORMAT_422_YUYV:
            case PACKED_FORMAT_422_UYVY:
            case PACKED_FORMAT_422_YVYU:
            case PACKED_FORMAT_422_VYUY:
                lStride = lStride*2;
                cStride = 0;
                cHeight = 0;
                break;
            case PACKED_FORMAT_444:
                lStride = lStride*3;
                cStride = 0;
                cHeight = 0;
                break;
            default:
                lStride = 0;
                cStride  = 0;
                break;
        }
    }


    if (scalerOn == TRUE) {
        /* Luma stride */
        if (subsample == FORMAT_420 || subsample == FORMAT_422 || (PACKED_FORMAT_422_YUYV <= packed && packed <= PACKED_FORMAT_422_VYUY)) {
            lStride = JPU_CEIL(32, lStride);
        }
        else {
            lStride = JPU_CEIL(16, lStride);
        }
        /* Chroma stride */
        if (cbcrIntlv == CBCR_SEPARATED) {
            if (subsample == FORMAT_444) {
                cStride = JPU_CEIL(16, cStride);
            }
            else {
                cStride = JPU_CEIL(8, cStride);
            }
        }
        else {
            cStride = JPU_CEIL(32, cStride);

        }
    }
    else {
        lStride = JPU_CEIL(8, lStride);
        if (subsample == FORMAT_420 || subsample == FORMAT_422) {
            cStride = JPU_CEIL(16, cStride);
        }
        else {
            cStride = JPU_CEIL(8, cStride);
        }
    }
    lHeight = JPU_CEIL(8, lHeight);
    cHeight = JPU_CEIL(8, cHeight);

    lStride *= bytePerPixel;
    cStride *= bytePerPixel;

    if (oLumaStride)   *oLumaStride   = lStride;
    if (oLumaHeight)   *oLumaHeight   = lHeight;
    if (oChromaStride) *oChromaStride = cStride;
    if (oChromaHeight) *oChromaHeight = cHeight;
}


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
                         Uint32 rotation, BOOL scalerOn, Uint32 width, Uint32 height, Uint32 bitDepth, Uint32 num, vpuFrameBuffer** pframe_buffer)
{
    Uint32  fbLumaStride, fbLumaHeight, fbChromaStride, fbChromaHeight;
    Uint32  fbLumaSize, fbChromaSize, fbSize;
    Uint32  i;
    Uint32  bytePerPixel = (bitDepth + 7)/8;


    vpuFrameBuffer* pframe = malloc(sizeof(vpuFrameBuffer)*num);
    if(!pframe)
    {
        log_err("framebuf management block malloc fail\n");
        return 0;
    }


    if (rotation == 90 || rotation == 270) {
        if (subsample == FORMAT_422) subsample = FORMAT_440;
        else if (subsample == FORMAT_440) subsample = FORMAT_422;
    }

    __hal_vpu_get_frame_buf_stride(subsample, cbcrIntlv, packed, scalerOn, width, height, bytePerPixel, &fbLumaStride, &fbLumaHeight, &fbChromaStride, &fbChromaHeight);

    fbLumaSize   = fbLumaStride * fbLumaHeight;
    fbChromaSize = fbChromaStride * fbChromaHeight;

    if (cbcrIntlv == CBCR_SEPARATED) {
        /* fbChromaSize MUST be zero when format is packed mode */
        fbSize = fbLumaSize + 2*fbChromaSize;

    }
    else {
        /* Semi-planar */
        fbSize = fbLumaSize + fbChromaSize;
    }

    vpu_buffer_t buffer_space;

    buffer_space.size = fbSize;
    buffer_space.size *= num;
    if (jdi_allocate_dma_memory(&buffer_space) < 0) {
        log_debug("Fail to allocate frame buffer size=%d\n", buffer_space.size);
        return 0;
    }


    unsigned long vb_addr = buffer_space.phys_addr;

    for(i=0;i<num;i++)
    {
        pframe[i].format = subsample;
        pframe[i].bufY = vb_addr;
        vb_addr += fbLumaSize;
        vb_addr = JPU_CEIL(8, vb_addr);
        pframe[i].bufCb = vb_addr;
        vb_addr += fbChromaSize;
        vb_addr = JPU_CEIL(8, vb_addr);
        pframe[i].bufCr = (cbcrIntlv == CBCR_SEPARATED) ? vb_addr : 0;
        vb_addr += (cbcrIntlv == CBCR_SEPARATED) ? fbChromaSize : 0;
        vb_addr = JPU_CEIL(8, vb_addr);

        pframe[i].stride = fbLumaStride;
        pframe[i].strideC = fbChromaStride;

        pframe[i].endian = endian;
    }

    *pframe_buffer = pframe;

    return fbSize;
}


/**
 * @brief create a dedicated frame buffer for current stream.
 *
 * @param instIdx the opened decoder instance idx which prior
 * frame buffer registered.
 *
 * @return non-return.
 *
 */
void hal_vpu_free_frame_buffer(vpuFrameBuffer* frame_buffer)
{

    vpu_buffer_t buffer_space;
    buffer_space.phys_addr = frame_buffer[0].bufY;
    buffer_space.size = 0xF;//avoid return

    jdi_free_dma_memory(&buffer_space);


    free(frame_buffer);
}
