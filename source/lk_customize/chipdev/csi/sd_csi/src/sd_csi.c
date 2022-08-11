/*
* csi.c
*
* Copyright (c) 2018 Semidrive Semiconductor.
* All rights reserved.
*
* csi interface function
*
* Revision History:
* -----------------
* 0.1, 12/21/2018 init version
* 0.2, 1/16/2019 add csi preview
*/

#include <sys/types.h>
#include <debug.h>
#include <compiler.h>
#include <bits.h>
#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <lib/page_alloc.h>

#include <platform.h>
#include <platform/interrupts.h>
#include <__regs_base.h>
//#include <__regs_int.h>

//#include "v4l2.h"
#include "sd_csi.h"
#include <kernel/thread.h>
#include "res.h"

#if WITH_KERNEL_VM
#include <kernel/vm.h>
#endif

#ifdef PFM_POOL_BASE
#include <dev/sd_ddr_pfmon.h>
#endif

#if WITH_LLL_DEBUG
#define csi_writel(val, reg) writel(val, reg); \
    dprintf(INFO, "w(0x%lx, 0x%08x), r(0x%08x)\n", reg, val, readl(reg));
#else
#define csi_writel(val, reg) writel(val, reg);
#endif

#define CSI_DRV_LOG 4
#define DEBUG4 1

#if DEBUG4
static int logf = 0;
#endif

static int bit2num(uint8_t val)
{
    int ret = 0, i;

    for (i = 0; i < 8; i++) {
        if (val & (1 << i))
            ret++;
    }

    return ret;
}
static void csi_reset(struct csi_device *dev)
{
    uint32_t val;
    struct v4l2_device *vdev;
    vdev = dev->vdev;

    dprintf(CSI_DRV_LOG, "%s: vc=0x%x.\n", __func__, vdev->ex_info.vc);

    if ((bit2num(vdev->ex_info.vc) == 1) || (vdev->ex_info.sync)) {
        val = readl(dev->regs + CSI_ENABLE);
        val &= ~CSI_ENABLE_IMAGE_EN_MASK;
        csi_writel(val, dev->regs + CSI_ENABLE);

        val |= (1 << CSI_ENABLE_GLOBAL_RESET_SHIFT);
        val |= (0x0F << CSI_ENABLE_SOFT_RESET_SHIFT);
        csi_writel(val, dev->regs + CSI_ENABLE);

        val &= ~CSI_ENABLE_GLOBAL_RESET_MASK;
        val &= ~CSI_ENABLE_SOFT_RESET_MASK;
        csi_writel(val, dev->regs + CSI_ENABLE);
    }
}

static inline void csi_init_baddr(struct csi_device *dev,
                                  uint32_t reg_h, uint32_t reg_l, addr_t baddr)
{
    uint32_t val;

    if (!baddr)
        return;

#if ARCH_ARM64
    val = (baddr >> CSI_IMG_BADDR_L_SIZE) & CSI_IMG_BADDR_H_MASK;
    csi_writel(val, dev->regs + reg_h);

    val = baddr & CSI_IMG_BADDR_L_MASK;
    csi_writel(val, dev->regs + reg_l);
#else
    val = 0;
    csi_writel(val, dev->regs + reg_h);

    val = p2ap(baddr & CSI_IMG_BADDR_L_MASK);

    csi_writel(val, dev->regs + reg_l);
#endif
}


int csi_cfg_mem(struct csi_device *dev, uint32_t mask)
{
    int i;
    struct csi_image *img;
    uint32_t reg_load;
    struct csi_buffer *b = NULL;
    struct v4l2_device *vdev;

    vdev = dev->vdev;


    reg_load = readl(dev->regs + CSI_REG_LOAD);

    for (i = 0; i < IMG_COUNT; i++) {
        img = &dev->csi_image[i];

        if (!img->enable)
            continue;

        if (((1 << i) & mask) == 0)
            continue;

        if (dev->using_queue) {
            if (!vdev->ex_info.sync) {
                if (list_is_empty(&dev->csi_image[i].buf_list)) {
                    dprintf(CSI_DRV_LOG, "%s: %d list is empty\n", __func__, i);
                    continue;
                }

                b = list_peek_head_type(&dev->csi_image[i].buf_list, struct csi_buffer,
                                        node);
            }
            else if ((vdev->ex_info.sync == true)) {
                if (list_is_empty(&dev->csi_image[0].buf_list)) {
                    dprintf(CSI_DRV_LOG, "%s: %d list is empty\n", __func__, i);
                    continue;
                }

                b = list_peek_head_type(&dev->csi_image[0].buf_list, struct csi_buffer,
                                        node);
            }
        }

        if (dev->using_queue) {
            img->buf_pos = b->index;

            if (img->q_flag < 2)
                img->q_flag++;
        }
        else
            img->buf_pos = CSI_INC_BUF_POS(img->buf_pos);

        csi_init_baddr(dev, CSI_IMG_RGBY_BADDR_H_(i),
                       CSI_IMG_RGBY_BADDR_L_(i),
                       img->rgby_baddr[img->buf_pos]);

        csi_init_baddr(dev, CSI_IMG_U_BADDR_H_(i),
                       CSI_IMG_U_BADDR_L_(i),
                       img->u_baddr[img->buf_pos]);

        csi_init_baddr(dev, CSI_IMG_V_BADDR_H_(i),
                       CSI_IMG_V_BADDR_L_(i),
                       img->v_baddr[img->buf_pos]);
#if DEBUG4

        if (logf < 20)
            dprintf(CSI_DRV_LOG, "%d-%d: 0x%lx, 0x%lx, 0x%lx\n", i, img->buf_pos,
                   img->rgby_baddr[img->buf_pos], img->u_baddr[img->buf_pos],
                   img->v_baddr[img->buf_pos]);

#endif
        reg_load |= ((1 << i) << CSI_REG_LOAD_SHADOW_SET_OFFSET);

        if (dev->using_queue) {
            if (vdev->ex_info.sync) {
                dev->sdw_cnt |= 1 << i;

                if ((dev->sdw_cnt == 0xf) && (b)) {
                    list_delete(&b->node);

                    if (dev->streaming == false)
                        dev->sdw_cnt = 0;
                }
            }
            else {
                if (b)
                    list_delete(&b->node);
            }
        }
    }


    csi_writel(reg_load, dev->regs + CSI_REG_LOAD);
    return 0;
}

static int csi_init_stride(struct csi_device *dev)
{
    int i;
    struct csi_image *img;
    struct v4l2_device *vdev;
    struct v4l2_mbus_framefmt fmt;
    uint32_t rgby_stride, u_stride, v_stride;
    uint32_t width, height, stride_width;
    uint32_t size;
    dprintf(CSI_DRV_LOG, "csi_init_stride()\n");
    vdev = dev->vdev;
    fmt = vdev->fmt;

    for (i = 0; i < IMG_COUNT; i++) {
        img = &dev->csi_image[i];
        dprintf(CSI_DRV_LOG, "enable=%d\n", img->enable);

        if ((!img->enable) || (img->initialized))
            continue;

        stride_width = width = fmt.width;

        if (vdev->ex_info.sync)
            height = fmt.height / vdev->ex_info.vcn;
        else
            height = fmt.height;

        height -= 1;

        if (img->crop_enable)
            stride_width = img->crop_len;

        switch (fmt.code) {
            case V4L2_PIX_FMT_YUV420SP:
                rgby_stride = stride_width;
                u_stride = stride_width;
                v_stride = 0;
                width -= 2;
                break;

            case V4L2_PIX_FMT_YUV420XP:
                rgby_stride = stride_width;
                width -= 2;
                u_stride = stride_width / 2;
                v_stride = stride_width / 2;
                break;

            case V4L2_PIX_FMT_UYVYSP:
            case V4L2_PIX_FMT_YUYVSP:
                rgby_stride = stride_width;
                width -= 2;
                u_stride = stride_width;
                v_stride = 0;
                break;

            case V4L2_PIX_FMT_UYVY:
            case V4L2_PIX_FMT_YUYV:
            case V4L2_PIX_FMT_RGB565:
                rgby_stride = stride_width * 2;
#if EMULATION_PLATFORM_FPGA
                width -= 2;
#else

                if (dev->id == 0 || dev->id == 1)
                    width -= 1;
                else
                    width -= 2;

#endif
                u_stride = 0;
                v_stride = 0;
                break;

            case V4L2_PIX_FMT_RGB24:
            case V4L2_PIX_FMT_YUV444:
                rgby_stride = stride_width * 3;
                width -= 1;
                u_stride = 0;
                v_stride = 0;
                break;

            default:
                dprintf(0, "unsupport format!\n");
                return -1;
        }

        rgby_stride = (rgby_stride << CSI_IMG_YUV_STRIDE_SHIFT) &
                      CSI_IMG_YUV_STRIDE_MASK;
        u_stride = (u_stride << CSI_IMG_YUV_STRIDE_SHIFT) &
                   CSI_IMG_YUV_STRIDE_MASK;
        v_stride = (v_stride << CSI_IMG_YUV_STRIDE_SHIFT) &
                   CSI_IMG_YUV_STRIDE_MASK;

        size = ((width << CSI_IMG_SIZE_WIDTH_SHIFT) &
                CSI_IMG_SIZE_WIDTH_MASK) |
               ((height << CSI_IMG_SIZE_HEIGHT_SHIFT) &
                CSI_IMG_SIZE_HEIGHT_MASK);


        csi_writel(rgby_stride, dev->regs + CSI_IMG_RGBY_STRIDE_(i));
        csi_writel(u_stride, dev->regs + CSI_IMG_U_STRIDE_(i));
        csi_writel(v_stride, dev->regs + CSI_IMG_V_STRIDE_(i));

        csi_writel(size, dev->regs + CSI_IMG_SIZE_(i));
        dprintf(CSI_DRV_LOG, "stride: 0x%x, 0x%lx\n", rgby_stride,
                dev->regs + CSI_IMG_RGBY_STRIDE_(i));
        dprintf(CSI_DRV_LOG, "stride: 0x%x, 0x%lx\n", u_stride,
                dev->regs + CSI_IMG_U_STRIDE_(i));
        dprintf(CSI_DRV_LOG, "stride: 0x%x, 0x%lx\n", v_stride,
                dev->regs + CSI_IMG_V_STRIDE_(i));
        dprintf(CSI_DRV_LOG, "stride: 0x%x, 0x%lx\n", size,
                dev->regs + CSI_IMG_SIZE_(i));
        img->rgby_stride = rgby_stride >> CSI_IMG_YUV_STRIDE_SHIFT;
        img->u_stride = u_stride >> CSI_IMG_YUV_STRIDE_SHIFT;
        img->v_stride = v_stride >> CSI_IMG_YUV_STRIDE_SHIFT;

        if (vdev->ex_info.sync)
            img->height = fmt.height / vdev->ex_info.vcn;
        else
            img->height = fmt.height;
    }

    return 0;
}

static int csi_init_channel(struct csi_device *dev)
{
    int i;
    struct csi_image *img;
    struct v4l2_device *vdev;
    struct v4l2_mbus_framefmt fmt;
    uint32_t ipi_reg, interface;
    uint32_t chnl_ctl = 0, chnl_split0 = 0, chnl_split1 = 0;
    uint32_t chnl_pack0 = 0, chnl_pack1 = 0;
    uint32_t pixel_mask0 = 0, pixel_mask1 = 0;
    dprintf(CSI_DRV_LOG, "csi_init_channel()\n");
    vdev = dev->vdev;
    fmt = vdev->fmt;

    for (i = 0; i < IMG_COUNT; i++) {
        img = &dev->csi_image[i];
        dprintf(CSI_DRV_LOG, "cnt=%d, enable=%d, fmt.code=0x%x\n", IMG_COUNT,
                img->enable, fmt.code);
        dprintf(CSI_DRV_LOG, "img=%d, img->enable=%d, img->initialized=%d\n", i,
                img->enable,
                img->initialized);

        if ((!img->enable) || (img->initialized))
            continue;

        ipi_reg = readl(dev->regs + CSI_IMG_IPI_CTRL_(i));
        dprintf(CSI_DRV_LOG, "ipi_ctrl: 0x%x\n", ipi_reg);
        ipi_reg &= ~(CSI_IMG_YUV420_MASK | CSI_IMG_YUV420_LEGACY_MASK |
                     CSI_IMG_RAW_MASK | CSI_IMG_YUV422_MASK |
                     CSI_IMG_VLD_ODD_MASK | CSI_IMG_VLD_EVEN_MASK |
                     CSI_IMG_INTERFACE_MASK | CSI_IMG_VSYNC_MASK_MASK);

        if (dev->interface == CSI_INTERFACE_DC_PARALLEL)
            interface = CSI_INTERFACE_OTH_PARALLEL;
        else if (dev->interface == CSI_INTERFACE_DC_PARALLEL2)
            interface = CSI_INTERFACE_MIPI_CSI;
        else if (dev->interface == CSI_INTERFACE_OTH_PARALLEL2)
            interface = CSI_INTERFACE_GATE_MODE2;
        else
            interface = dev->interface;

        ipi_reg |= (interface << CSI_IMG_INTERFACE_SHIFT) &
                   CSI_IMG_INTERFACE_MASK;

        ipi_reg |= 0x01 << CSI_IMG_VSYNC_MASK_SHIFT;

        switch (fmt.code) {
            case V4L2_PIX_FMT_YUV420SP:
            case V4L2_PIX_FMT_YUV420XP:
                if (dev->interface == CSI_INTERFACE_MIPI_CSI)
                    ipi_reg |= (0x01 << CSI_IMG_YUV420_SHIFT) &
                               CSI_IMG_YUV420_MASK;

                ipi_reg |= ((0x01 << CSI_IMG_VLD_ODD_SHIFT) &
                            CSI_IMG_VLD_ODD_MASK) |
                           ((0x01 << CSI_IMG_VLD_EVEN_SHIFT) &
                            CSI_IMG_VLD_EVEN_MASK) |
                           (0x01 << CSI_IMG_FC_DUALLINE_SHIFT);

                break;

            case V4L2_PIX_FMT_UYVY:
            case V4L2_PIX_FMT_YUYV:
            case V4L2_PIX_FMT_UYVYSP:
            case V4L2_PIX_FMT_YUYVSP:
            case V4L2_PIX_FMT_RGB565:
                if (dev->interface == CSI_INTERFACE_MIPI_CSI)
                    ipi_reg |= (0x01 << CSI_IMG_YUV422_SHIFT) &
                               CSI_IMG_YUV422_MASK;

#if EMULATION_PLATFORM_FPGA
                ipi_reg |= ((0x01 << CSI_IMG_VLD_ODD_SHIFT) &
                            CSI_IMG_VLD_ODD_MASK) |
                           ((0x01 << CSI_IMG_VLD_EVEN_SHIFT) &
                            CSI_IMG_VLD_EVEN_MASK);
#else

                if (dev->id == 0 || dev->id == 1) {
                    ipi_reg |= ((0x00 << CSI_IMG_VLD_ODD_SHIFT) &
                                CSI_IMG_VLD_ODD_MASK) |
                               ((0x00 << CSI_IMG_VLD_EVEN_SHIFT) &
                                CSI_IMG_VLD_EVEN_MASK);
                }
                else {
                    ipi_reg |= ((0x01 << CSI_IMG_VLD_ODD_SHIFT) &
                                CSI_IMG_VLD_ODD_MASK) |
                               ((0x01 << CSI_IMG_VLD_EVEN_SHIFT) &
                                CSI_IMG_VLD_EVEN_MASK);
                }

#endif

                break;

            case V4L2_PIX_FMT_RGB24:
            case V4L2_PIX_FMT_YUV444:
                ipi_reg |= ((0x0 << CSI_IMG_VLD_ODD_SHIFT) &
                            CSI_IMG_VLD_ODD_MASK) |
                           ((0x0 << CSI_IMG_VLD_EVEN_SHIFT) &
                            CSI_IMG_VLD_EVEN_MASK);
                break;

            default:
                dprintf(INFO, "Unsupport format!\n");
                return -1;
        }

        csi_writel(ipi_reg, dev->regs + CSI_IMG_IPI_CTRL_(i));
        dprintf(CSI_DRV_LOG, "s-0x%x, 0x%lx\n", ipi_reg, dev->regs + CSI_IMG_IPI_CTRL_(i));

        switch (fmt.code) {
            case V4L2_PIX_FMT_YUV420SP:
                chnl_ctl = (0x03 << CSI_IMG_STREAM_ENABLE_SHIFT) &
                           CSI_IMG_STREAM_ENABLE_MASK;

                /* YUV420 semi planer */
                chnl_split0 = (SPLIT_VALID_ALL <<
                               CSI_IMG_CHN_SPLIT0_VALID_SEL_SHIFT) |
                              (SPLIT_CHN_MAP_48_63 << SPLIT_CHN0_SHIFT) |
                              (SPLIT_CHN_MAP_32_47 << SPLIT_CHN1_SHIFT) |
                              (SPLIT_CHN_MAP_NONE << SPLIT_CHN2_SHIFT) |
                              (SPLIT_CHN_MAP_NONE << SPLIT_CHN3_SHIFT);

                chnl_pack0 = (COLOR_DEPTH_8BIT <<
                              CSI_IMG_CHN_PACK_COLOR_DEPTH0_SHIFT) |
                             (COLOR_DEPTH_8BIT <<
                              CSI_IMG_CHN_PACK_COLOR_DEPTH1_SHIFT);

                chnl_split1 = (SPLIT_VALID_UV <<
                               CSI_IMG_CHN_SPLIT1_VALID_SEL_SHIFT) |
                              (SPLIT_CHN_MAP_16_31 << SPLIT_CHN0_SHIFT) |
                              (SPLIT_CHN_MAP_0_15 << SPLIT_CHN1_SHIFT);

                chnl_pack1 = (COLOR_DEPTH_8BIT <<
                              CSI_IMG_CHN_PACK_COLOR_DEPTH1_SHIFT) |
                             (COLOR_DEPTH_8BIT <<
                              CSI_IMG_CHN_PACK_COLOR_DEPTH0_SHIFT);

                break;

            /*YVYYVYY.../YUYYUYY... */
            case V4L2_PIX_FMT_YUV420XP:
                chnl_ctl = (0x07 << CSI_IMG_STREAM_ENABLE_SHIFT) &
                           CSI_IMG_STREAM_ENABLE_MASK;

                /* YUV420 legacy planer */
                chnl_split0 = (SPLIT_VALID_ALL <<
                               CSI_IMG_CHN_SPLIT0_VALID_SEL_SHIFT) |
                              (SPLIT_CHN_MAP_48_63 << SPLIT_CHN0_SHIFT) |
                              (SPLIT_CHN_MAP_16_31 << SPLIT_CHN1_SHIFT) |
                              (SPLIT_CHN_MAP_NONE << SPLIT_CHN2_SHIFT) |
                              (SPLIT_CHN_MAP_NONE << SPLIT_CHN3_SHIFT);

                chnl_pack0 = (COLOR_DEPTH_8BIT <<
                              CSI_IMG_CHN_PACK_COLOR_DEPTH0_SHIFT) |
                             (COLOR_DEPTH_8BIT <<
                              CSI_IMG_CHN_PACK_COLOR_DEPTH1_SHIFT);

                chnl_split1 = (SPLIT_VALID_ALL <<
                               CSI_IMG_CHN_SPLIT1_VALID_SEL_SHIFT) |
                              (SPLIT_CHN_MAP_32_47 << SPLIT_CHN0_SHIFT) |
                              (SPLIT_CHN_MAP_NONE << SPLIT_CHN1_SHIFT);

                chnl_pack1 = (COLOR_DEPTH_8BIT <<
                              CSI_IMG_CHN_PACK_COLOR_DEPTH0_SHIFT);
                break;

            case V4L2_PIX_FMT_UYVYSP:
                chnl_ctl = (0x03 << CSI_IMG_STREAM_ENABLE_SHIFT) &
                           CSI_IMG_STREAM_ENABLE_MASK;

                /* UYVY semi planer */
                chnl_split0 = (SPLIT_VALID_ALL <<
                               CSI_IMG_CHN_SPLIT0_VALID_SEL_SHIFT) |
                              (SPLIT_CHN_MAP_32_47 << SPLIT_CHN0_SHIFT) |
                              (SPLIT_CHN_MAP_0_15 << SPLIT_CHN1_SHIFT) |
                              (SPLIT_CHN_MAP_NONE << SPLIT_CHN2_SHIFT) |
                              (SPLIT_CHN_MAP_NONE << SPLIT_CHN3_SHIFT);

                chnl_pack0 = (COLOR_DEPTH_8BIT <<
                              CSI_IMG_CHN_PACK_COLOR_DEPTH0_SHIFT) |
                             (COLOR_DEPTH_8BIT <<
                              CSI_IMG_CHN_PACK_COLOR_DEPTH1_SHIFT);

                chnl_split1 = (SPLIT_VALID_UV <<
                               CSI_IMG_CHN_SPLIT1_VALID_SEL_SHIFT) |
                              (SPLIT_CHN_MAP_48_63 << SPLIT_CHN0_SHIFT) |
                              (SPLIT_CHN_MAP_16_31 << SPLIT_CHN1_SHIFT);

                chnl_pack1 = (COLOR_DEPTH_8BIT <<
                              CSI_IMG_CHN_PACK_COLOR_DEPTH1_SHIFT) |
                             (COLOR_DEPTH_8BIT <<
                              CSI_IMG_CHN_PACK_COLOR_DEPTH0_SHIFT);
                break;

            case V4L2_PIX_FMT_YUYVSP:
                chnl_ctl = (0x03 << CSI_IMG_STREAM_ENABLE_SHIFT) &
                           CSI_IMG_STREAM_ENABLE_MASK;

                /* YUYV semi planer */
                chnl_split0 = (SPLIT_VALID_ALL <<
                               CSI_IMG_CHN_SPLIT0_VALID_SEL_SHIFT) |
                              (SPLIT_CHN_MAP_48_63 << SPLIT_CHN0_SHIFT) |
                              (SPLIT_CHN_MAP_16_31 << SPLIT_CHN1_SHIFT) |
                              (SPLIT_CHN_MAP_NONE << SPLIT_CHN2_SHIFT) |
                              (SPLIT_CHN_MAP_NONE << SPLIT_CHN3_SHIFT);

                chnl_pack0 = (COLOR_DEPTH_8BIT <<
                              CSI_IMG_CHN_PACK_COLOR_DEPTH0_SHIFT) |
                             (COLOR_DEPTH_8BIT <<
                              CSI_IMG_CHN_PACK_COLOR_DEPTH1_SHIFT);

                chnl_split1 = (SPLIT_VALID_UV <<
                               CSI_IMG_CHN_SPLIT1_VALID_SEL_SHIFT) |
                              (SPLIT_CHN_MAP_32_47 << SPLIT_CHN0_SHIFT) |
                              (SPLIT_CHN_MAP_0_15 << SPLIT_CHN1_SHIFT);

                chnl_pack1 = (COLOR_DEPTH_8BIT <<
                              CSI_IMG_CHN_PACK_COLOR_DEPTH1_SHIFT) |
                             (COLOR_DEPTH_8BIT <<
                              CSI_IMG_CHN_PACK_COLOR_DEPTH0_SHIFT);
                break;

            case V4L2_PIX_FMT_UYVY:
            case V4L2_PIX_FMT_YUYV:
            case V4L2_PIX_FMT_RGB565:
                chnl_ctl = (0x01 << CSI_IMG_STREAM_ENABLE_SHIFT) &
                           CSI_IMG_STREAM_ENABLE_MASK;

#if EMULATION_PLATFORM_FPGA
                chnl_split0 = (SPLIT_VALID_ALL <<
                               CSI_IMG_CHN_SPLIT0_VALID_SEL_SHIFT) |
                              (SPLIT_CHN_MAP_48_63 << SPLIT_CHN0_SHIFT) |
                              (SPLIT_CHN_MAP_32_47 << SPLIT_CHN1_SHIFT) |
                              (SPLIT_CHN_MAP_16_31 << SPLIT_CHN2_SHIFT) |
                              (SPLIT_CHN_MAP_0_15 << SPLIT_CHN3_SHIFT);

                chnl_pack0 = (COLOR_DEPTH_8BIT <<
                              CSI_IMG_CHN_PACK_COLOR_DEPTH0_SHIFT) |
                             (COLOR_DEPTH_8BIT <<
                              CSI_IMG_CHN_PACK_COLOR_DEPTH1_SHIFT) |
                             (COLOR_DEPTH_8BIT <<
                              CSI_IMG_CHN_PACK_COLOR_DEPTH2_SHIFT) |
                             (COLOR_DEPTH_8BIT <<
                              CSI_IMG_CHN_PACK_COLOR_DEPTH3_SHIFT);
#else

                if (dev->id == 0 || dev->id == 1) {
                    chnl_split0 = (SPLIT_VALID_ALL <<
                                   CSI_IMG_CHN_SPLIT0_VALID_SEL_SHIFT) |
                                  (SPLIT_CHN_MAP_48_63 << SPLIT_CHN0_SHIFT) |
                                  (SPLIT_CHN_MAP_32_47 << SPLIT_CHN1_SHIFT);

                    chnl_pack0 = (COLOR_DEPTH_8BIT <<
                                  CSI_IMG_CHN_PACK_COLOR_DEPTH0_SHIFT) |
                                 (COLOR_DEPTH_8BIT <<
                                  CSI_IMG_CHN_PACK_COLOR_DEPTH1_SHIFT);
                }
                else {
                    chnl_split0 = (SPLIT_VALID_ALL <<
                                   CSI_IMG_CHN_SPLIT0_VALID_SEL_SHIFT) |
                                  (SPLIT_CHN_MAP_48_63 << SPLIT_CHN0_SHIFT) |
                                  (SPLIT_CHN_MAP_32_47 << SPLIT_CHN1_SHIFT) |
                                  (SPLIT_CHN_MAP_16_31 << SPLIT_CHN2_SHIFT) |
                                  (SPLIT_CHN_MAP_0_15 << SPLIT_CHN3_SHIFT);

                    chnl_pack0 = (COLOR_DEPTH_8BIT <<
                                  CSI_IMG_CHN_PACK_COLOR_DEPTH0_SHIFT) |
                                 (COLOR_DEPTH_8BIT <<
                                  CSI_IMG_CHN_PACK_COLOR_DEPTH1_SHIFT) |
                                 (COLOR_DEPTH_8BIT <<
                                  CSI_IMG_CHN_PACK_COLOR_DEPTH2_SHIFT) |
                                 (COLOR_DEPTH_8BIT <<
                                  CSI_IMG_CHN_PACK_COLOR_DEPTH3_SHIFT);
                }

#endif
                chnl_split1 = chnl_pack1 = 0;
                break;

            case V4L2_PIX_FMT_RGB24:
            case V4L2_PIX_FMT_YUV444:
                chnl_ctl = (0x01 << CSI_IMG_STREAM_ENABLE_SHIFT) &
                           CSI_IMG_STREAM_ENABLE_MASK;

                chnl_split0 = (SPLIT_VALID_ALL <<
                               CSI_IMG_CHN_SPLIT0_VALID_SEL_SHIFT) |
                              (SPLIT_CHN_MAP_48_63 << SPLIT_CHN0_SHIFT) |
                              (SPLIT_CHN_MAP_32_47 << SPLIT_CHN1_SHIFT) |
                              (SPLIT_CHN_MAP_16_31 << SPLIT_CHN2_SHIFT) |
                              (SPLIT_CHN_MAP_0_15 << SPLIT_CHN3_SHIFT);

                chnl_pack0 = (COLOR_DEPTH_8BIT <<
                              CSI_IMG_CHN_PACK_COLOR_DEPTH0_SHIFT) |
                             (COLOR_DEPTH_8BIT <<
                              CSI_IMG_CHN_PACK_COLOR_DEPTH1_SHIFT) |
                             (COLOR_DEPTH_8BIT <<
                              CSI_IMG_CHN_PACK_COLOR_DEPTH2_SHIFT) |
                             (COLOR_DEPTH_8BIT <<
                              CSI_IMG_CHN_PACK_COLOR_DEPTH3_SHIFT);


                chnl_split1 = chnl_pack1 = 0;

                break;
        }

        pixel_mask0 = pixel_mask1 = 0xFFFFFFFF;
        csi_writel(pixel_mask0, dev->regs + CSI_IMG_PIXEL_MASK0_(i));
        csi_writel(pixel_mask1, dev->regs + CSI_IMG_PIXEL_MASK1_(i));
        dprintf(CSI_DRV_LOG, "a- 0x%x-0x%lx\n", pixel_mask0,
                dev->regs + CSI_IMG_PIXEL_MASK0_(i));
        dprintf(CSI_DRV_LOG, "- 0x%x-0x%lx\n", pixel_mask1,
                dev->regs + CSI_IMG_PIXEL_MASK1_(i));
        csi_writel(chnl_ctl, dev->regs + CSI_IMG_CHN_CTRL_(i));
        csi_writel(chnl_split0, dev->regs + CSI_IMG_CHN_SPLIT0_(i));
        csi_writel(chnl_split1, dev->regs + CSI_IMG_CHN_SPLIT1_(i));

        csi_writel(chnl_pack0, dev->regs + CSI_IMG_CHN_PACK0_(i));
        csi_writel(chnl_pack1, dev->regs + CSI_IMG_CHN_PACK1_(i));
        dprintf(CSI_DRV_LOG, "- 0x%x-0x%lx\n", chnl_ctl,
                dev->regs + CSI_IMG_CHN_CTRL_(i));
        dprintf(CSI_DRV_LOG, "- 0x%x-0x%lx\n", chnl_split0,
                dev->regs + CSI_IMG_CHN_SPLIT0_(i));
        dprintf(CSI_DRV_LOG, "- 0x%x-0x%lx\n", chnl_split1,
                dev->regs + CSI_IMG_CHN_SPLIT1_(i));
        dprintf(CSI_DRV_LOG, "- 0x%x-0x%lx\n", chnl_pack0,
                dev->regs + CSI_IMG_CHN_PACK0_(i));
        dprintf(CSI_DRV_LOG, "- 0x%x-0x%lx\n", chnl_pack1,
                dev->regs + CSI_IMG_CHN_PACK1_(i));
    }

    return 0;
}

static int csi_cfg_crop(struct csi_device *dev)
{
    int i;
    struct csi_image *img;
    struct v4l2_device *vdev;
    struct v4l2_mbus_framefmt fmt;
    uint32_t chnl_ctl;
    uint32_t chnl_crop0 = 0, chnl_crop1 = 0;
    uint8_t crop_div;
    dprintf(CSI_DRV_LOG, "csi_cfg_crop()\n");
    vdev = dev->vdev;
    fmt = vdev->fmt;

    if (fmt.code == V4L2_PIX_FMT_RGB24 || fmt.code == V4L2_PIX_FMT_YUV444)
        crop_div = 1;
    else
        crop_div = 2;

    for (i = 0; i < IMG_COUNT; i++) {
        img = &dev->csi_image[i];
        dprintf(CSI_DRV_LOG, "enable: %d-%d\n", img->enable, img->crop_enable);

        if ((!img->enable) || (!img->crop_enable))
            continue;

        if (img->initialized)
            continue;

        chnl_ctl = readl(dev->regs + CSI_IMG_CHN_CTRL_(i));
        chnl_ctl &= ~(CSI_IMG_CROP_ENABLE_MASK);

        chnl_ctl |= ((chnl_ctl & CSI_IMG_STREAM_ENABLE_MASK) >>
                     CSI_IMG_STREAM_ENABLE_SHIFT) <<
                    CSI_IMG_CROP_ENABLE_SHIFT;
        dprintf(CSI_DRV_LOG, "=%d\n", img->rgby_stride);

        if (!img->rgby_stride)
            continue;

        chnl_crop0 = ((img->crop_pos / crop_div - 1) <<
                      CSI_IMG_CHN_CROP_POS_SHIFT) |
                     ((img->crop_len / crop_div) <<
                      CSI_IMG_CHN_CROP_LENGTH_SHIFT);

        if (img->u_stride || img->v_stride)
            chnl_crop1 = chnl_crop0;

        csi_writel(chnl_crop0, dev->regs + CSI_IMG_CHN_CROP0_(i));
        csi_writel(chnl_crop1, dev->regs + CSI_IMG_CHN_CROP1_(i));
        csi_writel(chnl_ctl, dev->regs + CSI_IMG_CHN_CTRL_(i));
        dprintf(CSI_DRV_LOG, "crop: 0x%x, 0x%lx\n", chnl_crop0,
                dev->regs + CSI_IMG_CHN_CROP0_(i));
        dprintf(CSI_DRV_LOG, "	0x%x, 0x%lx\n", chnl_crop1,
                dev->regs + CSI_IMG_CHN_CROP1_(i));
        dprintf(CSI_DRV_LOG, "	0x%x, 0x%lx\n", chnl_ctl,
                dev->regs + CSI_IMG_CHN_CTRL_(i));
    }

    return 0;
}

static int csi_init_bt_ctrl(struct csi_device *dev)
{
    struct v4l2_device *vdev;
    struct v4l2_mbus_framefmt fmt;
    uint32_t bt_ctrl0, bt_ctrl1, bt_ctrl2;
    dprintf(CSI_DRV_LOG, "csi_init_bt_ctrl()\n");
    vdev = dev->vdev;
    fmt = vdev->fmt;

    if (dev->interface != CSI_INTERFACE_DC_PARALLEL &&
            dev->interface != CSI_INTERFACE_DC_PARALLEL2) {
        bt_ctrl0 = readl(dev->regs + CSI_PARA_BT_CTRL0);
        bt_ctrl1 = readl(dev->regs + CSI_PARA_BT_CTRL1);
        bt_ctrl2 = readl(dev->regs + CSI_PARA_BT_CTRL2);
    }
    else {
        bt_ctrl0 = readl(dev->regs + CSI_PARA2_BT_CTRL0);
        bt_ctrl1 = readl(dev->regs + CSI_PARA2_BT_CTRL1);
        bt_ctrl2 = readl(dev->regs + CSI_PARA2_BT_CTRL2);
    }

    bt_ctrl0 &= ~(CSI_PARA_UV_PACK_CYCLE_EVEN_MASK |
                  CSI_PARA_UV_PACK_CYCLE_ODD_MASK |
                  CSI_PARA_PACK_CYCLE_EVEN_MASK |
                  CSI_PARA_PACK_CYCLE_ODD_MASK |
                  CSI_PARA_BT_CTRL0_EXIST_MASK);

    bt_ctrl1 &= ~(CSI_PARA_BT_FIELD_SEL_MASK |
                  CSI_PARA_BT_VSYNC_SEL_MASK |
                  CSI_PARA_BT_PROGRESSIVE_MASK);

    bt_ctrl2 &= ~CSI_PARA_PACK_SWAP_MASK;

    if (dev->interface != CSI_INTERFACE_MIPI_CSI &&
            dev->interface != CSI_INTERFACE_DC_PARALLEL2)
        bt_ctrl0 |= 0x01 << CSI_PARA_BT_CTRL0_EXIST_SHIFT;

    bt_ctrl0 |= 0x01 << CSI_PARA_CLK_POL_SHIFT;

    switch (fmt.code) {
        case V4L2_PIX_FMT_YUV420SP:
            bt_ctrl2 |= 1 << CSI_PARA_PACK_SWAP_SHIFT;
            bt_ctrl0 |= (0x01 << CSI_PARA_PACK_CYCLE_ODD_SHIFT) |
                        (0x03 << CSI_PARA_PACK_CYCLE_EVEN_SHIFT);

            bt_ctrl0 |= (0x00 << CSI_PARA_UV_PACK_CYCLE_ODD_SHIFT) |
                        (0x03 << CSI_PARA_UV_PACK_CYCLE_EVEN_SHIFT);
            break;

        /*VYYVYY.../UYYUYY...*/
        case V4L2_PIX_FMT_YUV420XP:
            bt_ctrl0 |= (0x02 << CSI_PARA_PACK_CYCLE_ODD_SHIFT) |
                        (0x02 << CSI_PARA_PACK_CYCLE_EVEN_SHIFT);

            bt_ctrl0 |= (0x01 << CSI_PARA_UV_PACK_CYCLE_ODD_SHIFT) |
                        (0x10 << CSI_PARA_UV_PACK_CYCLE_EVEN_SHIFT);
            break;

        case V4L2_PIX_FMT_YUYV:
        case V4L2_PIX_FMT_UYVY:
        case V4L2_PIX_FMT_YUYVSP:
        case V4L2_PIX_FMT_UYVYSP:
        case V4L2_PIX_FMT_RGB565:
#if 1
            bt_ctrl0 |= (0x03 << CSI_PARA_PACK_CYCLE_ODD_SHIFT) |
                        (0x03 << CSI_PARA_PACK_CYCLE_EVEN_SHIFT);
#endif
#if 0
            bt_ctrl0 |= (0x01 << CSI_PARA_PACK_CYCLE_ODD_SHIFT) |
                        (0x01 << CSI_PARA_PACK_CYCLE_EVEN_SHIFT);
#endif

            bt_ctrl0 |= (0x03 << CSI_PARA_UV_PACK_CYCLE_ODD_SHIFT) |
                        (0x03 << CSI_PARA_UV_PACK_CYCLE_EVEN_SHIFT);
            break;

        case V4L2_PIX_FMT_RGB24:
        case V4L2_PIX_FMT_YUV444:
            bt_ctrl0 |= (0x00 << CSI_PARA_PACK_CYCLE_ODD_SHIFT) |
                        (0x00 << CSI_PARA_PACK_CYCLE_EVEN_SHIFT);

            bt_ctrl0 |= (0x03 << CSI_PARA_UV_PACK_CYCLE_ODD_SHIFT) |
                        (0x03 << CSI_PARA_UV_PACK_CYCLE_EVEN_SHIFT);
            break;
    }

    bt_ctrl1 |= (1 << CSI_PARA_BT_PROGRESSIVE_SHIFT) |
                (0 << CSI_PARA_BT_VSYNC_SEL_SHIFT) |
                (1 << CSI_PARA_BT_FIELD_SEL_SHIFT);

    bt_ctrl1 |= (fmt.width << CSI_PARA_BT_VSYNC_POSTPONE_SHIFT);
    bt_ctrl2 |= (1 << CSI_PARA_BT_VSYNC_EDGE_SEL_SHIFT);

    if (dev->interface != CSI_INTERFACE_DC_PARALLEL &&
            dev->interface != CSI_INTERFACE_DC_PARALLEL2 &&
            dev->interface != CSI_INTERFACE_OTH_PARALLEL2) {
        csi_writel(bt_ctrl0, dev->regs + CSI_PARA_BT_CTRL0);
        csi_writel(bt_ctrl1, dev->regs + CSI_PARA_BT_CTRL1);
        csi_writel(bt_ctrl2, dev->regs + CSI_PARA_BT_CTRL2);
        dprintf(CSI_DRV_LOG, "non dc p1 p2-0x%x, 0x%lx\n", bt_ctrl0,
                dev->regs + CSI_PARA_BT_CTRL0);
        dprintf(CSI_DRV_LOG, "0x%x, 0x%lx\n", bt_ctrl1,
                dev->regs + CSI_PARA_BT_CTRL1);
        dprintf(CSI_DRV_LOG, "0x%x, 0x%lx\n", bt_ctrl2,
                dev->regs + CSI_PARA_BT_CTRL2);
    }
    else {
        csi_writel(bt_ctrl0, dev->regs + CSI_PARA2_BT_CTRL0);
        csi_writel(bt_ctrl1, dev->regs + CSI_PARA2_BT_CTRL1);
        csi_writel(bt_ctrl2, dev->regs + CSI_PARA2_BT_CTRL2);
        dprintf(CSI_DRV_LOG, "dc p1 p2-0x%x, 0x%lx\n", bt_ctrl0,
                dev->regs + CSI_PARA2_BT_CTRL0);
        dprintf(CSI_DRV_LOG, "0x%x, 0x%lx\n", bt_ctrl1,
                dev->regs + CSI_PARA2_BT_CTRL1);
        dprintf(CSI_DRV_LOG, "0x%x, 0x%lx\n", bt_ctrl2,
                dev->regs + CSI_PARA2_BT_CTRL2);
    }


    return 0;
}

static int csi_init_wdma(struct csi_device *dev)
{
    struct csi_image *img;
    uint32_t val;
    int i, j;
    dprintf(CSI_DRV_LOG, "csi_init_wdma()\n");

    for (i = 0; i < IMG_COUNT; i++) {
        img = &dev->csi_image[i];
        dprintf(CSI_DRV_LOG, "enable=%d\n", img->enable);

        if ((!img->enable) || (img->initialized))
            continue;

        /* default value */
        for (j = 0; j < CHN_PER_IMG; j++) {
            /* DFIFO */
            val = (0x40 << CSI_WDMA_DFIFO_WML_SHIFT ) |
                  (0x08 << CSI_WDMA_DFIFO_DEPTH_SHIFT);
            csi_writel(val, dev->regs +
                       CSI_WDMA_CHN_DFIFO_(i * CHN_PER_IMG + j));
            dprintf(CSI_DRV_LOG, "1-0x%x-0x%lx\n", val,
                    dev->regs + CSI_WDMA_CHN_DFIFO_(i * CHN_PER_IMG + j));
            /* CFIFO */
            val = 0x04 << CSI_WDMA_CFIFO_DEPTH_SHIFT;
            csi_writel(val, dev->regs +
                       CSI_WDMA_CHN_CFIFO_(i * CHN_PER_IMG + j));
            dprintf(CSI_DRV_LOG, "0x%x-0x%lx\n", val,
                    dev->regs + CSI_WDMA_CHN_CFIFO_(i * CHN_PER_IMG + j));
            /* AXI CTRL0 */
            val = ((0x03 << 3) << CSI_WDMA_AXI_PRIO_0_SHIF) |
                  ((0x05 << 3) << CSI_WDMA_AXI_PRIO_1_SHIF);
            csi_writel(val, dev->regs +
                       CSI_WDMA_CHN_AXI_CTRL0_(i * CHN_PER_IMG + j));
            dprintf(CSI_DRV_LOG, " 0x%x-0x%lx\n", val,
                    dev->regs + CSI_WDMA_CHN_AXI_CTRL0_(i * CHN_PER_IMG + j));
            /* AXI CTRL1 */
            val = 0;
            csi_writel(val, dev->regs +
                       CSI_WDMA_CHN_AXI_CTRL1_(i * CHN_PER_IMG + j));
            dprintf(CSI_DRV_LOG, "0x%x-0x%lx\n", val,
                   dev->regs + CSI_WDMA_CHN_AXI_CTRL1_(i * CHN_PER_IMG + j));
            /*AXI CTRL2 */
            val = (3 << CSI_WDMA_AXI_LEN_SHIFT) |
                  (1 << CSI_WDMA_AXI_BUR_MODE_SHIFT) |
                  (0 << CSI_WDMA_AXI_BUFAB_SHIFT);
            csi_writel(val, dev->regs +
                       CSI_WDMA_CHN_AXI_CTRL2_(i * CHN_PER_IMG + j));
            dprintf(CSI_DRV_LOG, "0x%x-0x%lx\n", val,
                    dev->regs + CSI_WDMA_CHN_AXI_CTRL2_(i * CHN_PER_IMG + j));
        }

        img->initialized = true;
    }

    dprintf(CSI_DRV_LOG, "csi_init_wdma() end\n");
    return 0;
}

static int csi_init_pixel_map(struct csi_device *dev)
{
    uint32_t pixel_reg[MAP_COUNT] = {
        0x481c6144, 0x40c20402, 0x2481c614, 0xd24503ce,
        0xe34c2ca4, 0x4d24503c, 0x5c6da658, 0x85d65547,
        0x75c6da65, 0xe69648e2, 0x28607de9, 0x9e69648e
    };
    int i;
    dprintf(CSI_DRV_LOG, "csi_init_pixel_map()\n");

    for (i = 0; i < MAP_COUNT; i++) {
        if (dev->interface != CSI_INTERFACE_DC_PARALLEL &&
                dev->interface != CSI_INTERFACE_DC_PARALLEL2 &&
                dev->interface != CSI_INTERFACE_OTH_PARALLEL2) {
            csi_writel(pixel_reg[i], dev->regs + CSI_PIXEL_MAP_(i));
            dprintf(CSI_DRV_LOG, "1-0x%x, 0x%lx\n", pixel_reg[i],
                    dev->regs + CSI_PIXEL_MAP_(i));
        }
        else {
            csi_writel(pixel_reg[i], dev->regs + CSI_PIXEL2_MAP_(i));
            dprintf(CSI_DRV_LOG, "2-0x%x, 0x%lx\n", pixel_reg[i],
                    dev->regs + CSI_PIXEL2_MAP_(i));
        }
    }

    return 0;
}

static int csi_cfg_interface(struct csi_device *dev,
                             struct v4l2_fwnode_endpoint endpoint)
{
    uint32_t csi_interface;
    //struct csi_image *img;
    //int i;

    switch (endpoint.bus_type) {
        case V4L2_MBUS_PARALLEL:
//          csi_interface = CSI_INTERFACE_GATE_MODE2;
            csi_interface = CSI_INTERFACE_OTH_PARALLEL;
            break;

        case V4L2_MBUS_BT656:
            csi_interface = CSI_INTERFACE_BT656;
            break;

        case V4L2_MBUS_CSI2:
            csi_interface = CSI_INTERFACE_MIPI_CSI;
            break;

        case V4L2_MBUS_DC_PARALLEL:
            csi_interface = CSI_INTERFACE_DC_PARALLEL;
            break;

        case V4L2_MBUS_DC_PARALLEL2:
            csi_interface = CSI_INTERFACE_DC_PARALLEL2;
            break;

        case V4L2_MBUS_PARALLEL2:
            csi_interface = CSI_INTERFACE_OTH_PARALLEL2;
            break;

        default:
            return -1;
    }

    dev->interface = csi_interface;
#if 0

    for (i = 0; i < IMG_COUNT; i++) {
        img = &dev->csi_image[i];

        img->id = i;
        img->csi = dev;
        img->enable = false;
        event_init(&img->completion, false, EVENT_FLAG_AUTOUNSIGNAL);

        switch (dev->interface) {
            case CSI_INTERFACE_DC_PARALLEL:
                if (i == 1)
                    img->enable = true;
                else
                    img->enable = false;

                break;

            case CSI_INTERFACE_DC_PARALLEL2:
                if (i == 2)
                    img->enable = true;
                else
                    img->enable = false;

                break;

            case CSI_INTERFACE_MIPI_CSI:
                img->enable = true;
                break;

            case CSI_INTERFACE_OTH_PARALLEL:
                if (i == 0)
                    img->enable = true;
                else
                    img->enable = false;

                break;

            case CSI_INTERFACE_OTH_PARALLEL2:
                if (i == 1)
                    img->enable = true;
                else
                    img->enable = false;

                break;

            default:
                if (i == 0)
                    img->enable = true;
                else
                    img->enable = false;

                break;
        }
    }

#endif
    return 0;
}

static int csidrv_error_thread(void *arg)
{
    struct csi_device *dev = (struct csi_device *)arg;
    dprintf(CSI_DRV_LOG, "%s(): start: \n", __func__);

    while (!dev->err_thread_done) {
        event_wait_timeout(&dev->err_completion, 10000);

        if (dev->err_stat0 || dev->err_stat1) {
            dprintf(CSI_DRV_LOG, "csi %d, reg base: %lx int stat:\n",
                    dev->id, dev->regs);
            dprintf(CSI_DRV_LOG, "##stat0=0x%08x, stat1=0x%08x, err_cnt=%u\n",
                    dev->err_stat0, dev->err_stat1, dev->err_cnt);
            dev->err_stat0 = dev->err_stat1 = 0;

            dprintf(CSI_DRV_LOG, "##bt_cof=%u, bt_fatal=%u, bt_ecc=%u\n",
                    dev->err_bt_cof, dev->err_bt_fatal, dev->err_bt_ecc);
        }
    }

    return 0;
}

static enum handler_return image_shadow_set_int(struct csi_device *dev,
        uint32_t stat0)
{
    uint32_t sdw_int, str_int;
    struct csi_image *img;
    struct v4l2_device *vdev;
    int i;

    vdev = dev->vdev;
    sdw_int = (stat0 & CSI_INT_STAT0_SDW_SET_INT_MASK) >>
              CSI_INT_STAT0_SDW_SET_INT_OFFSET;

    str_int = (stat0 & CSI_INT_STAT0_STORE_DONE_INT_MASK);

    if (sdw_int) {
        for (i = 0; i < IMG_COUNT; i++) {
            img = &dev->csi_image[i];

            if (!img->enable)
                continue;

            if (((1 << i) & sdw_int) == 0)
                continue;

            if (dev->using_queue) {
                if (vdev->ex_info.sync == false) {
                    if (list_is_empty(&img->buf_list)) {
                        dprintf(0, "img:%d list null\n", i);
                        continue;
                    }
                }
                else if ((vdev->ex_info.sync == true) && (i == 0)) {
                    if (list_is_empty(&img->buf_list)) {
                        dprintf(CSI_DRV_LOG, "img:%d list null\n", i);
                        continue;
                    }
                }
            }

            if (((1 << i) & str_int))
                csi_cfg_mem(dev, ((1 << i) & sdw_int));
            else if (img->q_flag < 2)
                csi_cfg_mem(dev, ((1 << i) & sdw_int));

            dev->stream_cnt++;

            if (dev->using_queue && (vdev->ex_info.sync)) {
                if (dev->sdw_cnt == 0xf) {
                    dev->sdw_cnt = 0;

                    if (str_int) {
                        dprintf(CSI_DRV_LOG, "send completion [%d]\n", current_time());
                        event_signal(&dev->csi_image[0].completion, false);
                    }
                }
            }
            else {
                if (((1 << i) & str_int)) {
                    event_signal(&img->completion, false);
                }

            }
        }
    }

    return INT_NO_RESCHEDULE;
}

static void csi_int_update_err(struct csi_device *dev,
                               uint32_t stat0, uint32_t stat1)
{
    if (stat0 & CSI_INT_STAT0_COF_INT_MASK)
        dev->err_bt_cof++;

    if (stat0 & CSI_INT_STAT0_BT_FATAL_INT_MASK)
        dev->err_bt_fatal++;

    if (stat0 & CSI_INT_STAT0_BT_ERR_INT_MASK)
        dev->err_bt_ecc++;

    if (stat1 & (0xFFF << CSI_INT_STAT1_BUS_ERR0_INT_OFFSET))
        dev->err_bus++;

    if (stat1 & (0x0F << CSI_INT_STAT1_OVERFLOW0_INT_OFFSET)) {
#ifdef PFM_POOL_BASE
        enum master_id id = MASTER_CSI1;
        pfm_notice_overflow(id + dev->id);
#endif
        dev->err_overflow++;
    }

    if (stat1 & (0x0F << CSI_INT_STAT1_PIXEL_ERR0_INT_OFFSET))
        dev->err_pixel++;

    if (stat1 & (0x0F << CSI_INT_STAT1_CROP_ERR0_INT_OFFSET))
        dev->err_crop++;
}


static enum handler_return csi_int_handler(void *data)
{

    struct csi_device *dev = (struct csi_device *)data;
    spin_lock_saved_state_t state;
    uint32_t stat0, stat1;
    uint32_t tmp1, tmp2 = 0;
    enum handler_return ret = INT_NO_RESCHEDULE;
    mask_interrupt(dev->irq);

    spin_lock_irqsave(&dev->lock, state);

    stat0 = readl(dev->regs + CSI_INT_STAT0);
    stat1 = readl(dev->regs + CSI_INT_STAT1);

#if DEBUG4

    if (logf < 4) {
        dprintf(0, "[%d]dev->id=%d ##stat0=0x%08x, stat1=0x%08x\n", logf,
                dev->id, stat0, stat1);
        logf++;
    }

#endif

    for (int i = 0; i < 4; i++) {
        tmp1 = 0x11 << i;

        if ((stat0 & tmp1) == tmp1)
            tmp2 |= 1 << i;
    }

    stat0 = (stat0 & (~(0xf))) | tmp2;

    csi_writel(stat1, dev->regs + CSI_INT_STAT1);
    csi_writel(stat0, dev->regs + CSI_INT_STAT0);

    if (!(dev->streaming) || (stat1 != 0) ||
            (stat0 & (CSI_INT_STAT0_COF_INT_MASK |
                      CSI_INT_STAT0_BT_FATAL_INT_MASK |
                      CSI_INT_STAT0_BT_ERR_INT_MASK))) {
        dev->err_stat0 = stat0;
        dev->err_stat1 = stat1;
        dev->err_cnt++;
        csi_int_update_err(dev, stat0, stat1);

        event_signal(&dev->err_completion, false);
        spin_unlock_irqrestore(&dev->lock, state);
        unmask_interrupt(dev->irq);
        dprintf(0, "err: ##stat0=0x%08x, stat1=0x%08x\n", stat0, stat1);
        return INT_NO_RESCHEDULE;
    }

    if (stat0 & (CSI_INT_STAT0_SDW_SET_INT_MASK |
                 CSI_INT_STAT0_STORE_DONE_INT_MASK)) {
        ret = image_shadow_set_int(dev, stat0);
    }

    spin_unlock_irqrestore(&dev->lock, state);

    unmask_interrupt(dev->irq);
    return ret;

}

static int csi_enable(struct csi_device *dev, uint32_t mask)
{
    int i;
    struct csi_image *img;
    uint32_t reg_load, val;
    uint32_t int_mask0 = 0, int_mask1 = 0;
    struct v4l2_device *vdev;
    vdev = dev->vdev;


    dprintf(CSI_DRV_LOG, "csi_enable(): \n");

    if ((mask < 0) || (mask > (1 << IMG_COUNT)))
        dprintf(0, "%s(): mask error.\n", __func__);

    if (dev->mask_en == 0) {
        dev->stream_cnt = 0;
        dev->err_cnt = 0;
        dev->frm_cnt = 0;
        dev->err_bt_cof = 0;
        dev->err_bt_fatal = 0;
        dev->err_bt_ecc = 0;
        dev->err_bus = 0;
        dev->err_overflow = 0;
        dev->err_pixel = 0;
        dev->err_crop = 0;

        int_mask0 = CSI_INT_STAT0_COF_INT_MASK | CSI_INT_STAT0_BT_FATAL_INT_MASK |
                    CSI_INT_STAT0_BT_ERR_INT_MASK | CSI_INT_STAT0_SDW_SET_INT_MASK;
        int_mask1 = (0xFFF << CSI_INT_STAT1_BUS_ERR0_INT_OFFSET) |
                    (0x0F << CSI_INT_STAT1_OVERFLOW0_INT_OFFSET) |
                    (0x0F << CSI_INT_STAT1_PIXEL_ERR0_INT_OFFSET) |
                    (0x0F << CSI_INT_STAT1_CROP_ERR0_INT_OFFSET);
        csi_writel(int_mask0, dev->regs + CSI_INT_MASK0);
        csi_writel(int_mask1, dev->regs + CSI_INT_MASK1);
        unmask_interrupt(dev->irq);
    }

    dev->mask_en |= mask;
    dprintf(CSI_DRV_LOG, "%s(): dev->mask_en=0x%x.\n", __func__, dev->mask_en);

    reg_load = readl(dev->regs + CSI_REG_LOAD);

    reg_load &= ~(CSI_REG_LOAD_SHADOW_UPDATE_MASK);

    reg_load |= (1 << CSI_REG_LOAD_WDMA_CFG_LOAD_OFFSET) |
                (0xFF << CSI_REG_LOAD_UPDATE_MASK_OFFSET);

    for (i = 0; i < IMG_COUNT; i++) {
        img = &dev->csi_image[i];

        if (!img->enable)
            continue;

        reg_load |= ((1 << i) << CSI_REG_LOAD_SHADOW_UPDATE_OFFSET);
    }

    csi_writel(reg_load, dev->regs + CSI_REG_LOAD);

    val = readl(dev->regs + CSI_ENABLE);
    val &= ~CSI_ENABLE_IMAGE_EN_MASK;

    for (i = 0; i < IMG_COUNT; i++) {
        img = &dev->csi_image[i];

        if (!img->enable)
            continue;

        if ((1 << i & vdev->ex_info.vc) == 0)
            continue;

        val |= ((1 << i) << CSI_ENABLE_IMAGE_EN_SHIFT);
    }

    dprintf(CSI_DRV_LOG, "enable: 0x%x\n", val);
    csi_writel(val, dev->regs + CSI_ENABLE);

    dev->streaming = true;

    return 0;
}

static int csi_stop(struct csi_device *dev, uint32_t mask)
{
    uint32_t val;
    uint32_t stat0, stat1;
    uint32_t int_mask0, int_mask1;
    struct list_node *n, *tn;
    int i;

    dprintf(CSI_DRV_LOG, "%s(): mask=0x%x\n", __func__, mask);

    if ((mask < 0) || (mask > (1 << IMG_COUNT)))
        dprintf(0, "%s(): mask error.\n", __func__);

    dev->mask_en &= ~mask;
    dprintf(CSI_DRV_LOG, "%s(): dev->mask_en=0x%x.\n", __func__, dev->mask_en);

    val = readl(dev->regs + CSI_ENABLE);
    //val &= ~CSI_ENABLE_IMAGE_EN_MASK;
    val &= ~(mask);
    csi_writel(val, dev->regs + CSI_ENABLE);

    if (dev->mask_en == 0) {
        dev->streaming = false;
        mask_interrupt(dev->irq);

        int_mask0 = int_mask1 = 0;
        csi_writel(int_mask0, dev->regs + CSI_INT_MASK0);
        csi_writel(int_mask1, dev->regs + CSI_INT_MASK1);

        stat0 = 0x7FF;
        stat1 = 0xFFFFF;
        csi_writel(stat0, dev->regs + CSI_INT_STAT0);
        csi_writel(stat1, dev->regs + CSI_INT_STAT1);

        dprintf(CSI_DRV_LOG,
                "csi id %u,stream count: %u frame count: %u error count: %u\n",
                dev->id, dev->stream_cnt, dev->frm_cnt, dev->err_cnt);
        dprintf(CSI_DRV_LOG, "bt_cof=%u, bt_fatal=%u, bt_ecc=%u\n",
                dev->err_bt_cof, dev->err_bt_fatal, dev->err_bt_ecc);
        dprintf(CSI_DRV_LOG, "bus_err=%u, overflow=%u, pixel=%u, crop=%u\n",
                dev->err_bus, dev->err_overflow, dev->err_pixel, dev->err_crop);
        dev->err_cnt = 0;
    }

#if DEBUG4
    logf = 0;
#endif

    for (i = 0; i < IMG_COUNT; i++) {
        if (1 << i & dev->mask_en)
            continue;

        dev->csi_image[i].q_flag = 0;

        if (list_length(&dev->csi_image[i].buf_list)) {
            list_for_every_safe(&dev->csi_image[i].buf_list, n, tn)
            list_delete(n);
        }
    }

    return 0;
}


#if WITH_CSI_DUMMY_REG
uint32_t csi_dummy_reg[2][1024 * 1024 * 2];
#endif

static int csi_init_regs(struct csi_device *dev)
{
    int ret;
    //struct v4l2_device *vdev;

    dprintf(CSI_DRV_LOG, "%s()\n", __func__);

    dev->sync = dev->vdev->ex_info.sync;
    dev->vcn = dev->vdev->ex_info.vcn;

    csi_reset(dev);

    ret = csi_init_channel(dev);

    if (ret < 0)
        return ret;

    ret = csi_init_bt_ctrl(dev);

    if (ret < 0)
        return ret;

    ret = csi_init_stride(dev);

    if (ret < 0)
        return ret;

    ret = csi_cfg_crop(dev);

    if (ret < 0)
        return ret;

    ret = csi_init_pixel_map(dev);

    if (ret < 0)
        return ret;

    ret = csi_init_wdma(dev);

    dev->initialized = true;
    dprintf(CSI_DRV_LOG, "%s() end.\n", __func__);
    return ret;
}

static struct csi_image *csi_get_image(struct csi_device *dev,
                                       uint8_t image_id)
{
    if (image_id > IMG_COUNT)
        return NULL;

    return &dev->csi_image[image_id];
}

static int csi_set_image(struct csi_device *dev, struct csi_image *img)
{
    int i;

    if (img->id > IMG_COUNT)
        return -EINVAL;

    for (i = 0; i < CAMERA_MAX_BUF; i++) {
        dev->csi_image[img->id].rgby_baddr[i] = img->rgby_baddr[i];
        dev->csi_image[img->id].u_baddr[i] = img->u_baddr[i];
        dev->csi_image[img->id].v_baddr[i] = img->v_baddr[i];
    }

    return 0;
}

static int csi_qbuf(struct csi_device *dev, int img, struct csi_buffer *b)
{
    dprintf(CSI_DRV_LOG, "%s(): [%d]listlen=%d, add list: b->index=%d\n",
            __func__, img,
            (int)list_length(&dev->csi_image[img].buf_list), b->index);

    list_add_tail(&dev->csi_image[img].buf_list, &b->node);

    if (!dev->using_queue)
        dev->using_queue = true;

    return 0;
}

static struct csi_device_ops csi_dev_ops = {
    .init = csi_init_regs,
    .start = csi_enable,
    .cfg_mem = csi_cfg_mem,
    .cfg_interface = csi_cfg_interface,
    .stop = csi_stop,
    .get_image = csi_get_image,
    .set_image = csi_set_image,
    .qbuf = csi_qbuf,
};

struct csi_device *csi_host_init(uint32_t id, addr_t addr, uint32_t irq)
{
    struct csi_device *dev;
    int i;
    struct csi_image *img;

    dprintf(0, "%s(): id=%d\n", __func__, id);
    dev = malloc(sizeof(*dev));

    if (!dev)
        return NULL;

    memset(dev, 0, sizeof(*dev));

    dev->id = id;
#if WITH_CSI_DUMMY_REG
    /* use dummy register mem  */
    dev->regs = (addr_t)&csi_dummy_reg[dev->id][0];
#elif WITH_KERNEL_VM
    //dev->regs = (uint64_t)paddr_to_kvaddr(CSI_BASE_ADDR(dev->id));
    dev->regs = (uint64_t)paddr_to_kvaddr(addr);
#else
    //dev->regs = CSI_BASE_ADDR(dev->id);
    dev->regs = addr;
#endif
    arch_clean_invalidate_cache_range(dev->regs, 0x700);

    //dev->irq = CSI_IRQ(dev->id);
    dev->irq = irq;
    dprintf(CSI_DRV_LOG, "reg base: 0x%lx, irq:%d\n", dev->regs, dev->irq);

    dev->streaming = false;
    dev->initialized = false;
    dev->using_queue = 0;
    dev->ops = csi_dev_ops;
    dev->int_handler = csi_int_handler;
    //list_initialize(&dev->buf_list);
    dev->sdw_cnt = 0;
    dev->err_cnt = 0;
    dev->err_thread_done = false;

    event_init(&dev->err_completion, false, EVENT_FLAG_AUTOUNSIGNAL);

    dev->err_thread = thread_create("csidrv error thread",
                                    &csidrv_error_thread, (void *)dev, HIGH_PRIORITY,
                                    DEFAULT_STACK_SIZE);
    thread_resume(dev->err_thread);

    register_int_handler(dev->irq, dev->int_handler, dev);


    for (i = 0; i < IMG_COUNT; i++) {
        img = &dev->csi_image[i];

        img->id = i;
        img->csi = dev;
        img->enable = false;
        img->initialized = false;
        img->q_flag = 0;
        list_initialize(&img->buf_list);
        event_init(&img->completion, false, EVENT_FLAG_AUTOUNSIGNAL);
    }

    dprintf(CSI_DRV_LOG, "%s() end.\n", __func__);
    return dev;
}

int csi_exit(struct csi_device *dev)
{
    dprintf(0, "%s().\n", __func__);
    dev->err_thread_done = true;
    thread_join(dev->err_thread, NULL, INFINITE_TIME);
    free(dev);
    return 0;
}
