#include <stdlib.h>
#include <string.h>

#include "jpu_middleware_api.h"
#include "jpulog.h"

/*
 *   When jpu implements the wrap aroud of the bs buffer,
 *   the minimum reserved space is JPU_GBU_WRAP_SIZE (1024),
 *   so the bs buffer cannot be filled, and half of the value is used here
 */
#define DEFAULT_FEEDING_SIZE (STREAM_BUF_SIZE / 2)

#define ENC_SRC_BUF_NUM 1
#define PAGE_SIZE 4096

const int InvScanTable[64] = {0,  1,  5,  6,  14, 15, 27, 28, 2,  4,  7,  13, 16, 26, 29, 42,
                              3,  8,  12, 17, 25, 30, 41, 43, 9,  11, 18, 24, 31, 40, 44, 53,
                              10, 19, 23, 32, 39, 45, 52, 54, 20, 22, 33, 38, 46, 51, 55, 60,
                              21, 34, 37, 47, 50, 56, 59, 61, 35, 36, 48, 49, 57, 58, 62, 63};
static BYTE global_huffBit[8][256] = {
    // DC index 0 (Luminance DC) Bits
    {0x00, 0x01, 0x05, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00},
    // AC index 0 (Luminance AC) Bits
    {0x00, 0x02, 0x01, 0x03, 0x03, 0x02, 0x04, 0x03, 0x05, 0x05, 0x04, 0x04, 0x00, 0x00, 0x01,
     0x7D},
    // DC index 1 (Chrominance DC) Bits
    {0x00, 0x03, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00,
     0x00},
    // AC index 1 (Chrominance AC) Bits
    {0x00, 0x02, 0x01, 0x02, 0x04, 0x04, 0x03, 0x04, 0x07, 0x05, 0x04, 0x04, 0x00, 0x01, 0x02,
     0x77},
    {0},
    {0},
    {0},
    {0}};
static BYTE global_huffVal[8][256] = {
    // DC index 0 (Luminance DC) HuffValue
    {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B},
    // AC index 0 (Luminance AC) HuffValue
    {0x01, 0x02, 0x03, 0x00, 0x04, 0x11, 0x05, 0x12, 0x21, 0x31, 0x41, 0x06, 0x13, 0x51, 0x61,
     0x07, 0x22, 0x71, 0x14, 0x32, 0x81, 0x91, 0xA1, 0x08, 0x23, 0x42, 0xB1, 0xC1, 0x15, 0x52,
     0xD1, 0xF0, 0x24, 0x33, 0x62, 0x72, 0x82, 0x09, 0x0A, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x25,
     0x26, 0x27, 0x28, 0x29, 0x2A, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x43, 0x44, 0x45,
     0x46, 0x47, 0x48, 0x49, 0x4A, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0x63, 0x64,
     0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0x83,
     0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8A, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99,
     0x9A, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xAA, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6,
     0xB7, 0xB8, 0xB9, 0xBA, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8, 0xC9, 0xCA, 0xD2, 0xD3,
     0xD4, 0xD5, 0xD6, 0xD7, 0xD8, 0xD9, 0xDA, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7, 0xE8,
     0xE9, 0xEA, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7, 0xF8, 0xF9, 0xFA},
    // DC index 1 (Chrominance DC) HuffValue
    {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B},
    // AC index 1 (Chrominance AC)) HuffValue
    {0x00, 0x01, 0x02, 0x03, 0x11, 0x04, 0x05, 0x21, 0x31, 0x06, 0x12, 0x41, 0x51, 0x07, 0x61,
     0x71, 0x13, 0x22, 0x32, 0x81, 0x08, 0x14, 0x42, 0x91, 0xA1, 0xB1, 0xC1, 0x09, 0x23, 0x33,
     0x52, 0xF0, 0x15, 0x62, 0x72, 0xD1, 0x0A, 0x16, 0x24, 0x34, 0xE1, 0x25, 0xF1, 0x17, 0x18,
     0x19, 0x1A, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x43, 0x44,
     0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0x63,
     0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A,
     0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8A, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97,
     0x98, 0x99, 0x9A, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xAA, 0xB2, 0xB3, 0xB4,
     0xB5, 0xB6, 0xB7, 0xB8, 0xB9, 0xBA, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8, 0xC9, 0xCA,
     0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7, 0xD8, 0xD9, 0xDA, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7,
     0xE8, 0xE9, 0xEA, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7, 0xF8, 0xF9, 0xFA},
    {0},
    {0},
    {0},
    {0}};
static short global_qMatTab[4][64] = {
    // Quality 3 Luma
    {
        0x0006, 0x0004, 0x0004, 0x0004, 0x0005, 0x0004, 0x0006, 0x0005, 0x0005, 0x0006, 0x0009,
        0x0006, 0x0005, 0x0006, 0x0009, 0x000B, 0x0008, 0x0006, 0x0006, 0x0008, 0x000B, 0x000C,
        0x000A, 0x000A, 0x000B, 0x000A, 0x000A, 0x000C, 0x0010, 0x000C, 0x000C, 0x000C, 0x000C,
        0x000C, 0x000C, 0x0010, 0x000C, 0x000C, 0x000C, 0x000C, 0x000C, 0x000C, 0x000C, 0x000C,
        0x000C, 0x000C, 0x000C, 0x000C, 0x000C, 0x000C, 0x000C, 0x000C, 0x000C, 0x000C, 0x000C,
        0x000C, 0x000C, 0x000C, 0x000C, 0x000C, 0x000C, 0x000C, 0x000C, 0x000C,
    },
    // Quality 3 Chroma
    {
        0x0007, 0x0007, 0x0007, 0x000D, 0x000C, 0x000D, 0x0018, 0x0010, 0x0010, 0x0018, 0x0014,
        0x000E, 0x000E, 0x000E, 0x0014, 0x0014, 0x000E, 0x000E, 0x000E, 0x000E, 0x0014, 0x0011,
        0x000C, 0x000C, 0x000C, 0x000C, 0x000C, 0x0011, 0x0011, 0x000C, 0x000C, 0x000C, 0x000C,
        0x000C, 0x000C, 0x0011, 0x000C, 0x000C, 0x000C, 0x000C, 0x000C, 0x000C, 0x000C, 0x000C,
        0x000C, 0x000C, 0x000C, 0x000C, 0x000C, 0x000C, 0x000C, 0x000C, 0x000C, 0x000C, 0x000C,
        0x000C, 0x000C, 0x000C, 0x000C, 0x000C, 0x000C, 0x000C, 0x000C, 0x000C,
    },
    {0},
    {0}};

int get_drm_format_from_jpu_frame_format(FrameFormat frame_format)
{
    int ret = -1;

    switch (frame_format) {
    case FORMAT_420:
        ret = DRM_FORMAT_YUV420;
        break;
    case FORMAT_422:
        ret = DRM_FORMAT_YUV422;
        break;
    case FORMAT_440:
        ret = DRM_FORMAT_YUV422;
        break;
    case FORMAT_444:
        ret = DRM_FORMAT_YUV444;
        break;
    case FORMAT_400:
        ret = DRM_FORMAT_R8;
        break;
    default:
        break;
    }

    return ret;
}

void GetFrameBufStride(FrameFormat subsample, CbCrInterLeave cbcrIntlv, PackedFormat packed,
                       BOOL scalerOn, Uint32 width, Uint32 height, Uint32 bytePerPixel,
                       Uint32 *oLumaStride, Uint32 *oLumaHeight, Uint32 *oChromaStride,
                       Uint32 *oChromaHeight)
{
    Uint32 lStride, cStride;
    Uint32 lHeight, cHeight;
    lStride = JPU_CEIL(8, width);
    lHeight = height;
    cHeight = height / 2;

    if (packed == PACKED_FORMAT_NONE) {
        Uint32 chromaDouble = (cbcrIntlv == CBCR_SEPARATED) ? 1 : 2;

        switch (subsample) {
        case FORMAT_400:
            cStride = 0;
            cHeight = 0;
            break;

        case FORMAT_420:
            cStride = (lStride / 2) * chromaDouble;
            cHeight = (height + 1) / 2;
            break;

        case FORMAT_422:
            cStride = (lStride / 2) * chromaDouble;
            cHeight = height;
            break;

        case FORMAT_440:
            cStride = lStride * chromaDouble;
            cHeight = (height + 1) / 2;
            break;

        case FORMAT_444:
            cStride = lStride * chromaDouble;
            cHeight = height;
            break;

        default:
            cStride = 0;
            lStride = 0;
            cHeight = 0;
            break;
        }
    } else {
        switch (packed) {
        case PACKED_FORMAT_422_YUYV:
        case PACKED_FORMAT_422_UYVY:
        case PACKED_FORMAT_422_YVYU:
        case PACKED_FORMAT_422_VYUY:
            lStride = JPU_CEIL(32, lStride * 2);
            cStride = 0;
            cHeight = 0;
            break;

        case PACKED_FORMAT_444:
            lStride = lStride * 3;
            cStride = 0;
            cHeight = 0;
            break;

        default:
            lStride = 0;
            cStride = 0;
            break;
        }
    }

    if (scalerOn == TRUE) {
        /* Luma stride */
        if (subsample == FORMAT_420 || subsample == FORMAT_422 ||
            (PACKED_FORMAT_422_YUYV <= packed && packed <= PACKED_FORMAT_422_VYUY)) {
            lStride = JPU_CEIL(32, lStride);
        } else {
            lStride = JPU_CEIL(16, lStride);
        }

        /* Chroma stride */
        if (cbcrIntlv == CBCR_SEPARATED) {
            if (subsample == FORMAT_444) {
                cStride = JPU_CEIL(16, cStride);
            } else {
                cStride = JPU_CEIL(8, cStride);
            }
        } else {
            cStride = JPU_CEIL(32, cStride);
        }
    } else {
        if (subsample == FORMAT_420 || subsample == FORMAT_422) {
            lStride = JPU_CEIL(16, lStride);
        } else {
            lStride = JPU_CEIL(8, lStride);
        }
    }

    // if (subsample == FORMAT_420 || subsample == FORMAT_422) {
    // 	cStride = JPU_CEIL(16, cStride);
    // } else {
    // 	cStride = JPU_CEIL(8, cStride);
    // }

    if (subsample == FORMAT_420 || subsample == FORMAT_440) {
        lHeight = JPU_CEIL(16, lHeight);
    } else {
        lHeight = JPU_CEIL(8, lHeight);
    }

    cHeight = JPU_CEIL(8, cHeight);
    lStride *= bytePerPixel;
    cStride *= bytePerPixel;

    if (oLumaStride)
        *oLumaStride = lStride;

    if (oLumaHeight)
        *oLumaHeight = lHeight;

    if (oChromaStride)
        *oChromaStride = cStride;

    if (oChromaHeight)
        *oChromaHeight = cHeight;
}

int calculate_frame_size(int picWidth, int picHeight, Uint32 bitDepth, FrameFormat format,
                         PackedFormat packed)
{
    int size;
    int y, nY = 0, nCb, nCr;
    int lumaSize, chromaSize = 0, chromaWidth = 0, chromaHeight = 0;
    Uint8 *puc;
    Uint32 bytesPerPixel = (bitDepth + 7) / 8;

    switch (format) {
    case FORMAT_420:
        nY = picHeight;
        nCb = nCr = picHeight / 2;
        chromaSize = (picWidth / 2) * (picHeight / 2);
        chromaWidth = picWidth / 2;
        chromaHeight = nY;
        break;

    case FORMAT_440:
        nY = picHeight;
        nCb = nCr = picHeight / 2;
        chromaSize = (picWidth) * (picHeight / 2);
        chromaWidth = picWidth;
        chromaHeight = nY;
        break;

    case FORMAT_422:
        nY = picHeight;
        nCb = nCr = picHeight;
        chromaSize = (picWidth / 2) * picHeight;
        chromaWidth = (picWidth / 2);
        chromaHeight = nY * 2;
        break;

    case FORMAT_444:
        nY = picHeight;
        nCb = nCr = picHeight;
        chromaSize = picWidth * picHeight;
        chromaWidth = picWidth;
        chromaHeight = nY * 2;
        break;

    case FORMAT_400:
        nY = picHeight;
        nCb = nCr = 0;
        chromaSize = 0;
        chromaWidth = 0;
        chromaHeight = 0;
        break;

    default:
        return 0;
    }

    if (packed) {
        if (packed == PACKED_FORMAT_444)
            picWidth *= 3;
        else
            picWidth *= 2;

        chromaSize = 0;
    }

    lumaSize = picWidth * nY;
    size = lumaSize + chromaSize * 2;
    size *= bytesPerPixel;

    return size;
}

// JPU_DecGetBitstreamBuffer cannot calculate how much space
// is available for writing, and rdPtr is not an accurate value.
// for FEEDING_METHOD_FRAME_SIZE mode, the bs buffer needs to be
// cleaned up in time after decoding a frame
int jpu_feeding_bs_buffer(jpu_hw_instance *jpu)
{
    PhysicalAddress wrPtr;
    int room;
    void *virtual_wrPrt = NULL;
    int rightSize = 0, leftSize = 0;
    int feedSize = 0;
    int expect_size = 0;
    int fill_input_size = 0;

    JPU_DecGetBitstreamBuffer(jpu->handle, NULL, &wrPtr, &room);

    // Minimum of room / DEFAULT_FEEDING_SIZE
    expect_size = room < DEFAULT_FEEDING_SIZE ? room : DEFAULT_FEEDING_SIZE;

    if ((wrPtr + expect_size) >= (jpu->bs_stream.phys_addr + jpu->bs_stream.size)) {
        // right size
        rightSize = jpu->bs_stream.phys_addr + jpu->bs_stream.size - wrPtr;
        virtual_wrPrt = jdi_get_memory(wrPtr);
        jpu->fill_buffer_callback(jpu->user_private_ptr, rightSize, virtual_wrPrt,
                                  &fill_input_size);
        if (fill_input_size < 0) {
            JLOG(ERR, "[%s][%d] fill_buffer_callback failed, ret:%d", __func__, __LINE__,
                 fill_input_size);
            return -1;
        }

        feedSize += fill_input_size;

        // left size
        if (fill_input_size == rightSize) {
            expect_size -= fill_input_size;
            wrPtr = jpu->bs_stream.phys_addr;

            virtual_wrPrt = jdi_get_memory(wrPtr);
            jpu->fill_buffer_callback(jpu->user_private_ptr, expect_size, virtual_wrPrt,
                                      &fill_input_size);
            if (fill_input_size < 0) {
                JLOG(ERR, "[%s][%d] fill_buffer_callback failed, ret:%d", __func__, __LINE__,
                     fill_input_size);
                return -1;
            }
            feedSize += fill_input_size;
        }
    } else {
        virtual_wrPrt = jdi_get_memory(wrPtr);
        jpu->fill_buffer_callback(jpu->user_private_ptr, expect_size, virtual_wrPrt,
                                  &fill_input_size);
        if (fill_input_size < 0) {
            JLOG(ERR, "[%s][%d] fill_buffer_callback failed, ret:%d", __func__, __LINE__,
                 fill_input_size);
            return -1;
        }
        feedSize += fill_input_size;
    }

    JPU_DecUpdateBitstreamBuffer(jpu->handle, feedSize);

    JLOG(INFO,
         "[%s][%d] wrPtr:%x jpu->bs_stream.phys_addr:%x  expect_size:%d "
         "room:%d rightSize:%d feedSize:%d jpu->bs_stream.size:%d\n",
         __func__, __LINE__, wrPtr, jpu->bs_stream.phys_addr, expect_size, room, rightSize,
         feedSize, jpu->bs_stream.size);

    return feedSize;
}

int jpu_get_output_buffer_yuv_ptr(jpu_hw_instance *jpu)
{
    FrameBuffer *fb;
    int y, nY = 0, nCb, nCr;
    int lumaSize, chromaSize = 0, chromaWidth = 0, chromaHeight = 0;
    int chromaStride_i = 0;
    Uint32 bytesPerPixel = (jpu->dec_output.bit_depth + 7) / 8;

    {
        jpu->dec_output.y_ptr = NULL;
        jpu->dec_output.u_ptr = NULL;
        jpu->dec_output.v_ptr = NULL;
        jpu->dec_output.y_size = 0;
        jpu->dec_output.u_size = 0;
        jpu->dec_output.v_size = 0;
    }

    if (jpu->dec_output.current_output_index < 0) {
        JLOG(ERR, "error index:%d jpu has no effective output buffer\n",
             jpu->dec_output.output_frame_size);
        return -1;
    }

    fb = &jpu->dec_frameBuf[jpu->dec_output.current_output_index];

    int picWidth = jpu->dec_output.output_width;
    int picHeight = jpu->dec_output.output_height;
    chromaStride_i = fb->strideC;

    switch (fb->format) {
    case FORMAT_420:
        nY = picHeight;
        nCb = nCr = picHeight / 2;
        chromaSize = (picWidth / 2) * (picHeight / 2);
        chromaWidth = picWidth / 2;
        chromaHeight = nY;
        break;

    case FORMAT_440:
        nY = picHeight;
        nCb = nCr = picHeight / 2;
        chromaSize = (picWidth) * (picHeight / 2);
        chromaWidth = picWidth;
        chromaHeight = nY;
        break;

    case FORMAT_422:
        nY = picHeight;
        nCb = nCr = picHeight;
        chromaSize = (picWidth / 2) * picHeight;
        chromaWidth = (picWidth / 2);
        chromaHeight = nY * 2;
        break;

    case FORMAT_444:
        nY = picHeight;
        nCb = nCr = picHeight;
        chromaSize = picWidth * picHeight;
        chromaWidth = picWidth;
        chromaHeight = nY * 2;
        break;

    case FORMAT_400:
        nY = picHeight;
        nCb = nCr = 0;
        chromaSize = 0;
        chromaWidth = 0;
        chromaHeight = 0;
        break;

    default:
        return 0;
    }

    if (jpu->dec_output.output_packed_format) {
        if (jpu->dec_output.output_packed_format == PACKED_FORMAT_444)
            picWidth *= 3;
        else
            picWidth *= 2;

        chromaSize = 0;
    }

    lumaSize = picWidth * nY;
    lumaSize *= bytesPerPixel;
    chromaSize *= bytesPerPixel;
    picWidth *= bytesPerPixel;
    chromaWidth *= bytesPerPixel;

    if (jpu->dec_output.output_cbcrInterleave) {
        chromaSize = chromaSize * 2;
        chromaWidth = chromaWidth * 2;
        // chromaStride_i = chromaStride_i;
    }

    if (jpu->dec_output.output_packed_format) {
        JLOG(ERR, "[%s][%d] packed format, set chroma width to 0\n", __func__, __LINE__);
        chromaWidth = 0;
    }

    if ((picWidth == fb->stride) && (chromaWidth == chromaStride_i)) {
        jpu->dec_output.y_ptr = jdi_get_memory(fb->bufY);
        jpu->dec_output.y_size = lumaSize;

        if (jpu->dec_output.output_packed_format)
            return 0;

        if (jpu->dec_output.output_cbcrInterleave) {
            jpu->dec_output.u_ptr = jdi_get_memory(fb->bufCb);
            jpu->dec_output.u_size = chromaSize;
        } else {
            jpu->dec_output.u_ptr = jdi_get_memory(fb->bufCb);
            jpu->dec_output.u_size = chromaSize;

            jpu->dec_output.v_ptr = jdi_get_memory(fb->bufCr);
            jpu->dec_output.v_size = chromaSize;
        }
    } else {
        JLOG(ERR, "[%s][%d] nY=%d picWidth:%d fb->stride:%d chromaWidth:%d chromaStride_i:%d\n",
             __func__, __LINE__, nY, picWidth, fb->stride, chromaWidth, chromaStride_i);
        return -1;
    }

    return 0;
}

int jpu_dec_register_frame_buffer(jpu_hw_instance *jpu, jpu_buffer_t *frame_buffer, int from_fd)
{
    int ret = 0;
    int last_addr;
    JpgDecInfo *decInfo = &jpu->handle->JpgInfo->decInfo;
    Uint32 fbLumaStride, fbLumaHeight, fbChromaStride, fbChromaHeight;
    Uint32 fbLumaSize, fbChromaSize, fbSize;
    Uint32 bytePerPixel = (decInfo->bitDepth + 7) / 8;

    GetFrameBufStride(jpu->dec_output.output_format, jpu->dec_output.output_cbcrInterleave,
                      jpu->dec_output.output_packed_format, FALSE, jpu->dec_output.output_width,
                      jpu->dec_output.output_height, bytePerPixel, &fbLumaStride, &fbLumaHeight,
                      &fbChromaStride, &fbChromaHeight);
    fbLumaSize = fbLumaStride * fbLumaHeight;
    fbChromaSize = fbChromaStride * fbChromaHeight;

    if (jpu->dec_output.output_cbcrInterleave == CBCR_SEPARATED) {
        /* fbChromaSize MUST be zero when format is packed mode */
        fbSize = fbLumaSize + 2 * fbChromaSize;
    } else {
        /* Semi-planar */
        fbSize = fbLumaSize + fbChromaSize;
    }

    if (from_fd) {
        frame_buffer->buf_handle = from_fd;
        if (jdi_device_memory_map(frame_buffer)) {
            JLOG(ERR, "jdi_device_memory_map error\n");
            return -1;
        }

        if (JPU_CEIL(PAGE_SIZE, jpu->dec_output.output_frame_size) != frame_buffer->size) {
            JLOG(ERR,
                 "The size of the output dma fd buffer does not match the output parameters, "
                 "dma buffer size:%x output buffer calculated size:%x\n",
                 frame_buffer->size, jpu->dec_output.output_frame_size);
            return -1;
        }
    } else {
        frame_buffer->size = fbSize;
        frame_buffer->cached = JDI_CACHED_BUFFER;

        if (jdi_allocate_dma_memory(frame_buffer) < 0) {
            JLOG(ERR, "Fail to allocate frame buffer size=%d\n", frame_buffer->size);
            return -1;
        }
    }

    last_addr = frame_buffer->phys_addr;

    jpu->dec_frameBuf[0].bufY = last_addr;
    last_addr += fbLumaSize;
    last_addr = JPU_CEIL(8, last_addr);

    if (fbChromaSize) {
        jpu->dec_frameBuf[0].bufCb = last_addr;
        last_addr += fbChromaSize;
        last_addr = JPU_CEIL(8, last_addr);

        if (jpu->dec_output.output_cbcrInterleave == CBCR_SEPARATED) {
            jpu->dec_frameBuf[0].bufCr = last_addr;
            last_addr += fbChromaSize;
            last_addr = JPU_CEIL(8, last_addr);
        }
    } else {
        jpu->dec_frameBuf[0].bufCb = 0;
        jpu->dec_frameBuf[0].bufCr = 0;
    }
    jpu->dec_frameBuf[0].stride = fbLumaStride;
    jpu->dec_frameBuf[0].strideC = fbChromaStride;
    jpu->dec_frameBuf[0].fbLumaHeight = fbLumaHeight;
    jpu->dec_frameBuf[0].fbChromaHeight = fbChromaHeight;
    jpu->dec_frameBuf[0].endian = decInfo->frameEndian;
    jpu->dec_frameBuf[0].format = jpu->dec_output.output_format;
    jpu->dec_frameBuf[0].fbsize = fbSize;

    ret =
        JPU_DecRegisterFrameBuffer(jpu->handle, jpu->dec_frameBuf, 1, jpu->dec_frameBuf[0].stride);
    if (ret != JPG_RET_SUCCESS) {
        JLOG(ERR, "JPU_DecRegisterFrameBuffer failed Error code is 0x%x \n", ret);
        return -1;
    }

    return 0;
}

int jpu_dec_and_wait_interrupt(jpu_hw_instance *jpu, int dec_to_fd)
{
    int ret = 0;
    int int_reason = 0;
    static int frameIdx = 0;

    JpgDecParam decParam = {0};
    JpgDecOutputInfo outputInfo = {0};
    JpgRet jpu_ret = JPG_RET_SUCCESS;
    JpgDecInfo *decInfo = &jpu->handle->JpgInfo->decInfo;

    while (1) {
        if (jpu->dec_feedong_method == FEED_METHOD_FRAME_SIZE &&
            decInfo->streamWrPtr == decInfo->streamBufStartAddr) {
            /*
             for FEED_METHOD_FRAME_SIZE mode,
             streamRdPtr is greater than or equal to streamWrPtr,
             so the buffer must be filled before decoding,
             otherwise jpu may directly decode the bs data of the previous frame
            */
            ret = jpu_feeding_bs_buffer(jpu);
            if (ret < 0) {
                JLOG(ERR, "jpu_feeding_bs_buffer failed, errno :%d\n", ret);
                return ret;
            }
        }

        jpu_ret = JPU_DecStartOneFrame(jpu->handle, &decParam);

        if (jpu_ret != JPG_RET_SUCCESS && jpu_ret != JPG_RET_EOS) {
            if (jpu_ret == JPG_RET_BIT_EMPTY) {
                JLOG(INFO, "BITSTREAM NOT ENOUGH!\n");

                ret = jpu_feeding_bs_buffer(jpu);
                if (ret < 0) {
                    JLOG(ERR, "jpu_feeding_bs_buffer failed, errno :%d\n", ret);
                    return ret;
                }
                continue;
            }

            JLOG(ERR, "JPU_DecStartOneFrame failed Error code is 0x%x \n", jpu_ret);
            return -1;
        }

        if (jpu_ret == JPG_RET_EOS) {
            JLOG(INFO, "DECODE FINISH\n");
            JPU_DecGetOutputInfo(jpu->handle, &outputInfo); // unlock

            jpu->dec_output.decode_finish = TRUE;
            jpu->dec_output.current_output_index = -1;

            break;
        }

        while (1) {
            if ((int_reason = JPU_WaitInterrupt(jpu->handle, JPU_INTERRUPT_TIMEOUT_MS)) == -1) {
                JLOG(ERR, "[%s][%d] Error : timeout happened\n", __func__, __LINE__);
                JPU_SWReset(jpu->handle);
                break;
            }

            if (int_reason == -2) {
                JLOG(ERR, "Interrupt occurred. but this interrupt is not for my instance \n");
                break;
            }

            if (int_reason &
                ((1 << INT_JPU_DONE) | (1 << INT_JPU_ERROR) | (1 << INT_JPU_SLICE_DONE))) {
                JLOG(INFO, "int_reason: %08x\n", int_reason);
                break;
            }

            if (int_reason & (1 << INT_JPU_BIT_BUF_EMPTY)) {
                JLOG(INFO, " %s interrupt reason %#x INT_JPU_BIT_BUF_EMPTY now \n", __func__,
                     int_reason);

                ret = jpu_feeding_bs_buffer(jpu);
                if (ret < 0) {
                    JLOG(ERR, "jpu_feeding_bs_buffer failed, errno :%d\n", ret);
                    return ret;
                }

                JPU_ClrStatus(jpu->handle, (1 << INT_JPU_BIT_BUF_EMPTY));
                JPU_EnableInterrput(jpu->handle, decInfo->intrEnableBit);
            }
        }

        jpu_ret = JPU_DecGetOutputInfo(jpu->handle, &outputInfo);
        if (jpu_ret != JPG_RET_SUCCESS) {
            JLOG(ERR, "JPU_DecGetOutputInfo failed Error code is 0x%x \n", jpu_ret);
            return -1;
        }

        if (outputInfo.decodingSuccess == 0)
            JLOG(ERR, "JPU_DecGetOutputInfo decode fail framdIdx %d \n", frameIdx);

        JLOG(INFO,
             "frameIdx:%d, outputInfo.indexFrameDisplay:%d outputInfo.bytePosFrameStart:%x "
             "outputInfo.ecsPtr:%x outputInfo.consumedByte:%d  outputInfo.rdPtr:%x "
             "outputInfo.wrPtr:%x outputInfo.frameCycle:%d\n",
             frameIdx, outputInfo.indexFrameDisplay, outputInfo.bytePosFrameStart,
             outputInfo.ecsPtr, outputInfo.consumedByte, outputInfo.rdPtr, outputInfo.wrPtr,
             outputInfo.frameCycle);

        jpu->dec_output.current_output_index = outputInfo.indexFrameDisplay;

        if (!dec_to_fd) {
            ret = jpu_get_output_buffer_yuv_ptr(jpu);
            if (ret < 0) {
                JLOG(ERR, "jpu_get_output_buffer_yuv_ptr filed\n");
                return -1;
            }
        }

        if (outputInfo.indexFrameDisplay == -1) // end of sequence decoding
            break;

        if (outputInfo.numOfErrMBs) {
            int errRstIdx, errPosX, errPosY;
            errRstIdx = (outputInfo.numOfErrMBs & 0x0F000000) >> 24;
            errPosX = (outputInfo.numOfErrMBs & 0x00FFF000) >> 12;
            errPosY = (outputInfo.numOfErrMBs & 0x00000FFF);
            JLOG(ERR, "Error restart Idx : %d, MCU x:%d, y:%d, in Frame : %d \n", errRstIdx,
                 errPosX, errPosY, frameIdx);
        }

        if (jpu->dec_feedong_method == FEED_METHOD_FRAME_SIZE)
            JPU_DecSetRdPtrEx(jpu->handle, decInfo->streamBufStartAddr, TRUE);

        frameIdx++;
        break;
    }

    return 0;
}

jpu_hw_instance *jpu_hw_init()
{
    JpgRet ret = JPG_RET_SUCCESS;
    Uint32 apiVersion, hwRevision, hwProductId;

    jpu_hw_instance *jpu = (jpu_hw_instance *)malloc(sizeof(jpu_hw_instance));
    if (jpu == NULL) {
        JLOG(ERR, "malloc failed \n");
        return NULL;
    }
    memset(jpu, 0, sizeof(jpu_hw_instance));

    // set defalut value
    jpu->dec_output.current_output_index = -1;
    jpu->dec_output.output_format = FORMAT_MAX;

    ret = JPU_Init();
    if (ret != JPG_RET_SUCCESS && ret != JPG_RET_CALLED_BEFORE) {
        JLOG(ERR, "JPU_Init failed Error code is 0x%x \n", ret);
        goto ERR_INIT_HW;
    }

    JPU_GetVersionInfo(&apiVersion, &hwRevision, &hwProductId);
    JLOG(INFO, "JPU Version API_VERSION=0x%x, HW_REVISION=%#x, HW_PRODUCT_ID=0x%x\n", apiVersion,
         hwRevision, hwProductId);

    if (hwProductId != PRODUCT_ID_CODAJ12) {
        JLOG(ERR, "Error JPU HW_PRODUCT_ID=0x%x is not match with API_VERSION=0x%x\n", hwProductId,
             apiVersion);
        goto ERR_INIT_HW;
    }

    jpu->bs_stream.size = STREAM_BUF_SIZE;

    if (jdi_allocate_dma_memory(&jpu->bs_stream) < 0) {
        JLOG(ERR, "fail to allocate bitstream buffer\n");
        goto ERR_INIT_HW;
    }

    JLOG(INFO, "%s, size %#x, jpu_buffer_t length %d\n", __func__, jpu->bs_stream.size,
         sizeof(jpu_buffer_t));

    return jpu;
ERR_INIT_HW:
    jdi_free_dma_memory(&jpu->bs_stream);
    free(jpu);

    DeInitLog();
    return NULL;
}

int StoreYuvImageBurstFormat(int chromaStride, Uint8 *dst, int picWidth, int picHeight,
                             Uint32 bitDepth, PhysicalAddress addrY, PhysicalAddress addrCb,
                             PhysicalAddress addrCr, int stride, FrameFormat format, int endian,
                             CbCrInterLeave interLeave, PackedFormat packed)
{
    int size;
    int y, nY = 0, nCb, nCr;
    PhysicalAddress addr;
    int lumaSize, chromaSize = 0, chromaWidth = 0, chromaHeight = 0;
    Uint8 *puc;
    int chromaStride_i = 0;
    Uint32 bytesPerPixel = (bitDepth + 7) / 8;
    chromaStride_i = chromaStride;

    switch (format) {
    case FORMAT_420:
        nY = picHeight;
        nCb = nCr = picHeight / 2;
        chromaSize = (picWidth / 2) * (picHeight / 2);
        chromaWidth = picWidth / 2;
        chromaHeight = nY;
        break;

    case FORMAT_440:
        nY = picHeight;
        nCb = nCr = picHeight / 2;
        chromaSize = (picWidth) * (picHeight / 2);
        chromaWidth = picWidth;
        chromaHeight = nY;
        break;

    case FORMAT_422:
        nY = picHeight;
        nCb = nCr = picHeight;
        chromaSize = (picWidth / 2) * picHeight;
        chromaWidth = (picWidth / 2);
        chromaHeight = nY * 2;
        break;

    case FORMAT_444:
        nY = picHeight;
        nCb = nCr = picHeight;
        chromaSize = picWidth * picHeight;
        chromaWidth = picWidth;
        chromaHeight = nY * 2;
        break;

    case FORMAT_400:
        nY = picHeight;
        nCb = nCr = 0;
        chromaSize = 0;
        chromaWidth = 0;
        chromaHeight = 0;
        break;

    default:
        return 0;
    }

    puc = dst;
    addr = addrY;

    if (packed) {
        if (packed == PACKED_FORMAT_444)
            picWidth *= 3;
        else
            picWidth *= 2;

        chromaSize = 0;
    }

    lumaSize = picWidth * nY;
    size = lumaSize + chromaSize * 2;
    lumaSize *= bytesPerPixel;
    chromaSize *= bytesPerPixel;
    size *= bytesPerPixel;
    picWidth *= bytesPerPixel;
    chromaWidth *= bytesPerPixel;

    if (interLeave) {
        chromaSize = chromaSize * 2;
        chromaWidth = chromaWidth * 2;
        // chromaStride_i = chromaStride_i;
    }

    if ((picWidth == stride) && (chromaWidth == chromaStride_i)) {
        JpuReadMem(addr, (Uint8 *)(puc), lumaSize, endian); // Y, or packed YUV

        if (packed)
            return size;

        if (interLeave) {
            puc = dst + lumaSize;
            addr = addrCb;
            JpuReadMem(addr, (Uint8 *)(puc), chromaSize, endian); // UV  interLeave mode
        } else {
            puc = dst + lumaSize;
            addr = addrCb;
            JpuReadMem(addr, (Uint8 *)(puc), chromaSize, endian); // U
            puc = dst + lumaSize + chromaSize;
            addr = addrCr;
            JpuReadMem(addr, (Uint8 *)(puc), chromaSize, endian); // V
        }
    } else {
        for (y = 0; y < nY; ++y) {
            JpuReadMem(addr + stride * y, (Uint8 *)(puc + y * picWidth), picWidth,
                       endian); // Y, or packed YUV
        }

        if (packed)
            return size;

        if (interLeave) {
            JLOG(INFO, "nC=%d\n", chromaHeight / 2);
            puc = dst + lumaSize;
            addr = addrCb;

            for (y = 0; y < (chromaHeight / 2); ++y) {
                JpuReadMem(addr + (chromaStride_i)*y, (Uint8 *)(puc + y * (chromaWidth)),
                           (chromaWidth), endian); // UV  interLeave mode
            }
        } else {
            puc = dst + lumaSize;
            addr = addrCb;

            for (y = 0; y < nCb; ++y) {
                JpuReadMem(addr + chromaStride_i * y, (Uint8 *)(puc + y * chromaWidth), chromaWidth,
                           endian); // U
            }

            puc = dst + lumaSize + chromaSize;
            addr = addrCr;

            for (y = 0; y < nCr; ++y) {
                JpuReadMem(addr + chromaStride_i * y, (Uint8 *)(puc + y * chromaWidth), chromaWidth,
                           endian); // V
            }
        }
    }

    return size;
}

int jpu_dec_init(jpu_hw_instance *jpu, int align_to_16_bit, int dec_to_fd)
{
    int feedingSize = 0;

    BOOL scalerOn = FALSE;
    Uint32 framebufWidth = 0, framebufHeight = 0;
    Uint32 decodingWidth, decodingHeight;
    Uint32 displayWidth, displayHeight;

    FrameFormat output_format;
    JpgDecOpenParam decOP = {0};
    JpgDecInitialInfo initialInfo = {0};
    JpgRet ret = JPG_RET_SUCCESS;

    JpgEncOpenParam encOP = {0};

    if (!jpu || jpu->bs_stream.size <= 0) {
        JLOG(ERR, "jpu_dec_init Invalid input parameter\n");
        return -1;
    }

    // set dec default values
    decOP.streamEndian = 0;
    decOP.frameEndian = 0;
    decOP.bitstreamBuffer = jpu->bs_stream.phys_addr;
    decOP.bitstreamBufferSize = jpu->bs_stream.size;
    // set virtual address mapped of physical address
    decOP.pBitStream = (BYTE *)jpu->bs_stream.virt_addr; // lint !e511
    decOP.chromaInterleave =
        jpu->dec_output
            .output_cbcrInterleave; // Cb data and Cr data are located in each separate plan
    decOP.packedFormat = jpu->dec_output.output_packed_format; // PACKED_FORMAT_NONE for defalut
    decOP.roiEnable = 0;
    decOP.roiOffsetX = 0;
    decOP.roiOffsetY = 0;
    decOP.roiWidth = 0;
    decOP.roiHeight = 0;
    decOP.rotation = 0;
    decOP.mirror = 0;
    decOP.pixelJustification = 0;
    // defalut output format set to FORMAT_444
    // Only valid when output_packed_format is set to PACKED_FORMAT_NONE
    decOP.outputFormat =
        (jpu->dec_output.output_format == FORMAT_MAX) ? FORMAT_444 : jpu->dec_output.output_format;
    decOP.intrEnableBit =
        ((1 << INT_JPU_DONE) | (1 << INT_JPU_ERROR) | (1 << INT_JPU_BIT_BUF_EMPTY));

    JLOG(INFO, "------------------------------ DECODER OPTIONS ------------------------------\n");
    JLOG(INFO, "[streamEndian           ]: %d\n", decOP.streamEndian);
    JLOG(INFO, "[frameEndian            ]: %d\n", decOP.frameEndian);
    JLOG(INFO, "[chromaInterleave       ]: %d\n", decOP.chromaInterleave);
    JLOG(INFO, "[packedFormat           ]: %d\n", decOP.packedFormat);
    JLOG(INFO, "[roiEnable              ]: %d\n", decOP.roiEnable);
    JLOG(INFO, "[roiOffsetX             ]: %d\n", decOP.roiOffsetX);
    JLOG(INFO, "[roiOffsetY             ]: %d\n", decOP.roiOffsetY);
    JLOG(INFO, "[bitstreamBuffer        ]: 0x%08x\n", decOP.pBitStream);
    JLOG(INFO, "[bitstreamBufferSize    ]: %#x\n", decOP.bitstreamBufferSize);
    JLOG(INFO, "[roiWidth               ]: %d\n", decOP.roiWidth);
    JLOG(INFO, "[roiHeight              ]: %d\n", decOP.roiHeight);
    JLOG(INFO, "[rotation               ]: %d\n", decOP.rotation);
    JLOG(INFO, "[mirror                 ]: %d\n", decOP.mirror);
    JLOG(INFO, "[pixelJustification     ]: %d\n", decOP.pixelJustification);
    JLOG(INFO, "[outputFormat           ]: %d\n", decOP.outputFormat);
    JLOG(INFO, "[intrEnableBit          ]: %d\n", decOP.intrEnableBit);
    JLOG(INFO, "-----------------------------------------------------------------------------\n");

    // 1.open dec
    ret = JPU_DecOpen(&jpu->handle, &decOP);
    if (ret != JPG_RET_SUCCESS) {
        JLOG(ERR, "JPU_DecOpen failed Error code is 0x%x \n", ret);
        return -1;
    }

    // 2.fill input buffer to dec jpg header
    feedingSize = jpu_feeding_bs_buffer(jpu);
    if (feedingSize < 0) {
        JLOG(ERR, "jpu_feeding_bs_buffer failed, errno :%d\n", feedingSize);
        return feedingSize;
    }

    ret = JPU_DecGetInitialInfo(jpu->handle, &initialInfo);
    if (ret != JPG_RET_SUCCESS) {
        JLOG(ERR, "JPU_DecGetInitialInfo failed Error code is 0x%x\n", ret);
        goto ERR_INIT_DEC;
    }

    JLOG(INFO, "-------------------------------- Initial Info -------------------------------\n");
    JLOG(INFO, "[source picture size  ]: (%d X %d)\n", initialInfo.picWidth, initialInfo.picHeight);
    JLOG(INFO, "[sourceFormat         ]: %d\n", initialInfo.sourceFormat);
    JLOG(INFO, "[needFrameBufCount    ]: %d\n", initialInfo.minFrameBufferCount);
    JLOG(INFO, "[bitDepth             ]: %d\n", initialInfo.bitDepth);
    JLOG(INFO, "-----------------------------------------------------------------------------\n");

    // 3.dec params set
    if (align_to_16_bit) {
        framebufWidth = JPU_CEIL(16, initialInfo.picWidth);
        framebufHeight = JPU_CEIL(16, initialInfo.picHeight);
    } else {
        if (initialInfo.sourceFormat == FORMAT_420 || initialInfo.sourceFormat == FORMAT_422 ||
            decOP.outputFormat == FORMAT_420 || decOP.outputFormat == FORMAT_422)
            framebufWidth = JPU_CEIL(16, initialInfo.picWidth);
        else
            framebufWidth = JPU_CEIL(8, initialInfo.picWidth);

        if (initialInfo.sourceFormat == FORMAT_420 || initialInfo.sourceFormat == FORMAT_440 ||
            decOP.outputFormat == FORMAT_420 || decOP.outputFormat == FORMAT_440)
            framebufHeight = JPU_CEIL(16, initialInfo.picHeight);
        else
            framebufHeight = JPU_CEIL(8, initialInfo.picHeight);
    }

    // TODO: support scale
    // scalerOn = (BOOL)(decConfig.iHorScaleMode || decConfig.iVerScaleMode);
    // decodingWidth = framebufWidth >> decConfig.iHorScaleMode; //0(none), 1(1/2), 2(1/4), 3(1/8)
    // decodingHeight = framebufHeight >> decConfig.iVerScaleMode; //0(none), 1(1/2), 2(1/4), 3(1/8)
    // if (decOP.packedFormat != PACKED_FORMAT_NONE && decOP.packedFormat != PACKED_FORMAT_444)
    // {
    // 	// When packed format, scale-down resolution should be multiple of 2.
    // 	decodingWidth = JPU_CEIL(2, decodingWidth);
    // }
    decodingWidth = framebufWidth;
    decodingHeight = framebufHeight;

    // TODO:support rotation
    // Uint32 temp = decodingWidth;
    // decodingWidth = (decOP.rotation == 90 || decOP.rotation == 270) ? decodingHeight :
    // decodingWidth; decodingHeight = (decOP.rotation == 90 || decOP.rotation == 270) ? temp :
    // decodingHeight;

    if (decOP.roiEnable == TRUE) {
        decodingWidth = framebufWidth = initialInfo.roiFrameWidth;
        decodingHeight = framebufHeight = initialInfo.roiFrameHeight;
    }

    // TODO: support scale
    // if (0 != decConfig.iHorScaleMode || 0 != decConfig.iVerScaleMode)
    // {
    //     displayWidth = JPU_FLOOR(2, (framebufWidth >> decConfig.iHorScaleMode));
    //     displayHeight = JPU_FLOOR(2, (framebufHeight >> decConfig.iVerScaleMode));
    // }
    // else
    {
        displayWidth = decodingWidth;
        displayHeight = decodingHeight;
    }

    output_format =
        (decOP.outputFormat == FORMAT_MAX) ? initialInfo.sourceFormat : decOP.outputFormat;

    jpu->dec_output.pic_width = initialInfo.picWidth;
    jpu->dec_output.pic_height = initialInfo.picHeight;
    jpu->dec_output.pic_format = initialInfo.sourceFormat;
    jpu->dec_output.output_width = decodingWidth;
    jpu->dec_output.output_height = decodingHeight;
    jpu->dec_output.output_cbcrInterleave = decOP.chromaInterleave;
    jpu->dec_output.output_packed_format = decOP.packedFormat;
    jpu->dec_output.output_format = output_format;
    jpu->dec_output.bit_depth = initialInfo.bitDepth;
    jpu->dec_output.output_frame_size = calculate_frame_size(
        jpu->dec_output.output_width, jpu->dec_output.output_height, jpu->dec_output.bit_depth,
        jpu->dec_output.output_format, jpu->dec_output.output_packed_format);

    // 4.alloca frame buffer and register to jpu
    if (!dec_to_fd) {
        ret = jpu_dec_register_frame_buffer(jpu, &jpu->dec_frame_buffer_stream, 0);
        if (ret < 0)
            goto ERR_INIT_DEC;
    }

    JLOG(INFO, "[source picture size  ]: (%d X %d)\n", initialInfo.picWidth, initialInfo.picHeight);
    JLOG(INFO, "[decode picture size  ]: (%d X %d)\n", decodingWidth, decodingHeight);
    JLOG(INFO, "[display width size   ]: (%d     )\n", displayWidth);
    JLOG(INFO, "[display height size  ]: (%d     )\n", displayHeight);
    JLOG(INFO, "[outputformat         ]: %d\n", output_format);
    JLOG(INFO, "[needFrameBufCount    ]: %d\n", initialInfo.minFrameBufferCount);
    JLOG(INFO, "[bitDepth             ]: %d\n", initialInfo.bitDepth);
    JLOG(INFO, "[packedFormat         ]: %d\n", decOP.packedFormat);

    // TODO: support scale
    // JPU_DecGiveCommand(handle, SET_JPG_SCALE_HOR,  &(decConfig.iHorScaleMode));
    // JPU_DecGiveCommand(handle, SET_JPG_SCALE_VER,  &(decConfig.iVerScaleMode));

    return feedingSize;

ERR_INIT_DEC:
    if (jpu->dec_frame_buffer_stream.size > 0) {
        jdi_free_dma_memory(&jpu->dec_frame_buffer_stream);
        jpu->dec_frame_buffer_stream.base = 0;
        jpu->dec_frame_buffer_stream.size = 0;
    }

    JPU_DecSetRdPtrEx(jpu->handle, jpu->bs_stream.phys_addr, TRUE);
    memset(jpu->dec_frameBuf, 0, sizeof(jpu->dec_frameBuf));

    JPU_DecClose(jpu->handle);
    return -1;
}

int jpu_decompress(jpu_hw_instance *jpu)
{
    int ret = 0;

    if (!jpu || jpu->bs_stream.size <= 0 || !jpu->fill_buffer_callback) {
        JLOG(ERR, "jpu_dec_init Invalid input parameter\n");
        return -1;
    }

    if (!jpu->jpu_dec_init_success) {
        ret = jpu_dec_init(jpu, FALSE, FALSE);
        if (ret < 0) {
            JLOG(ERR, "jpu_dec_init failed\n");
            return -1;
        }
        jpu->jpu_dec_init_success = 1;
    }

    ret = jpu_dec_and_wait_interrupt(jpu, 0);
    if (ret < 0)
        JLOG(ERR, "jpu_dec_and_wait_interrupt failed\n");

    // dump output buffer
    // {
    // 	int frameSize = 0;
    // 	unsigned char * pYuv = malloc(jpu->dec_output.output_width * jpu->dec_output.output_height *
    // 3 * ((jpu->dec_output.bit_depth + 7) / 8));

    // 	FrameBuffer * fb = &jpu->frameBuf[outputInfo.indexFrameDisplay];
    // 	frameSize = StoreYuvImageBurstFormat(fb->strideC, pYuv, jpu->dec_output.output_width,
    // jpu->dec_output.output_height, jpu->dec_output.bit_depth, 			fb->bufY,
    //             fb->bufCb,
    //             fb->bufCr,
    //             fb->stride,
    //             fb->format,
    //             fb->endian,
    // 			jpu->dec_output.output_cbcrInterleave,
    // 			jpu->dec_output.output_packed_format);

    // 	{
    // 		FILE * fp = fopen("./output.yuv", "wb");
    // 		fwrite(pYuv, 1, frameSize, fp);
    // 		fclose(fp);
    // 	}
    // 	free(pYuv);
    // }

    return ret;
}

int jpu_decompress_to_fd(jpu_hw_instance *jpu, int *yuv_fd)
{
    int ret = 0;
    int decode_to_fd = TRUE;
    jpu_buffer_t frame_buffer = {0};

    if (!jpu || jpu->bs_stream.size <= 0 || !jpu->fill_buffer_callback) {
        JLOG(ERR, "jpu_dec_init Invalid input parameter\n");
        return -1;
    }

    if (!jpu->jpu_dec_init_success) {
        ret = jpu_dec_init(jpu, FALSE, decode_to_fd);
        if (ret < 0) {
            JLOG(ERR, "jpu_dec_init failed\n");
            return -1;
        }

        // allocate decode dest
        if (!jpu->drm_device)
            jpu->drm_device = dm_drm_create();
        else
            JLOG(WARN, "drm device has been created\n");

        if (!jpu->bo)
            jpu->bo = dm_drm_bo_create(
                jpu->drm_device, jpu->dec_output.output_width, jpu->dec_output.output_height,
                get_drm_format_from_jpu_frame_format(jpu->dec_output.output_format), 0);
        else
            JLOG(WARN, "drm bo has been created\n");

        jpu->jpu_dec_init_success = 1;
    }

    *yuv_fd = jpu->bo->handle->prime_fd;

    // register output frame buffer to jpu
    if (decode_to_fd) {
        ret = jpu_dec_register_frame_buffer(jpu, &frame_buffer, *yuv_fd);
        if (ret < 0)
            goto DEC_TO_FD_DONE;
    }

    ret = jpu_dec_and_wait_interrupt(jpu, decode_to_fd);
    if (ret < 0)
        JLOG(ERR, "jpu_dec_and_wait_interrupt failed\n");

    // dump output buffer
    // if (ret >= 0)
    // {
    //     void *output_buffer = NULL;
    //     dm_drm_bo_lock(jpu->bo, 0, 0, 0, jpu->dec_output.output_width,
    //         jpu->dec_output.output_height, &output_buffer);

    //     FILE *fp = fopen("output.yuv", "wb");
    //     fwrite(output_buffer, 1, jpu->dec_output.output_frame_size, fp);
    //     fclose(fp);

    //     dm_drm_bo_unlock(jpu->bo);
    // }

DEC_TO_FD_DONE:
    if (frame_buffer.phys_addr > 0)
        jdi_device_memory_unmap(&frame_buffer);

    return ret;
}

int jpu_decompress_16_bit_align(jpu_hw_instance *jpu, int decode_to_fd, int yuv_fd)
{
    int ret = 0;
    jpu_buffer_t frame_buffer = {0};

    if (!jpu || jpu->bs_stream.size <= 0 || !jpu->fill_buffer_callback) {
        JLOG(ERR, "jpu_dec_init Invalid input parameter\n");
        return -1;
    }

    if (!jpu->jpu_dec_init_success) {
        ret = jpu_dec_init(jpu, TRUE, decode_to_fd);
        if (ret < 0) {
            JLOG(ERR, "jpu_dec_init failed\n");
            return -1;
        }
        jpu->jpu_dec_init_success = 1;
    }

    // register output frame buffer to jpu
    if (decode_to_fd) {
        ret = jpu_dec_register_frame_buffer(jpu, &frame_buffer, yuv_fd);
        if (ret < 0)
            goto DEC_TO_FD_DONE;
    }

    ret = jpu_dec_and_wait_interrupt(jpu, decode_to_fd);
    if (ret < 0)
        JLOG(ERR, "jpu_dec_and_wait_interrupt failed\n");

DEC_TO_FD_DONE:
    if (frame_buffer.phys_addr > 0)
        jdi_device_memory_unmap(&frame_buffer);

    return ret;
}

int jpu_get_output_buffer(jpu_hw_instance *jpu, unsigned char *output_buffer, int buffer_size)
{
    FrameBuffer *fb;

    if (buffer_size < jpu->dec_output.output_frame_size) {
        JLOG(ERR, "error: not enough output buffer size, need:%d give us:%d\n",
             jpu->dec_output.output_frame_size, buffer_size);
        return -1;
    }

    if (jpu->dec_frame_buffer_stream.size <= 0) {
        JLOG(ERR, "error: jpu didn't decode, please decode first \n");
        return -1;
    }

    if (jpu->dec_output.current_output_index < 0) {
        JLOG(ERR, "error index:%d jpu has no effective output buffer\n",
             jpu->dec_output.output_frame_size);
        return -1;
    }

    JLOG(INFO, "[%s][%d] buffer_size:%d\n", __func__, __LINE__, buffer_size);

    fb = &jpu->dec_frameBuf[jpu->dec_output.current_output_index];

    StoreYuvImageBurstFormat(
        fb->strideC, output_buffer, jpu->dec_output.output_width, jpu->dec_output.output_height,
        jpu->dec_output.bit_depth, fb->bufY, fb->bufCb, fb->bufCr, fb->stride, fb->format,
        fb->endian, jpu->dec_output.output_cbcrInterleave, jpu->dec_output.output_packed_format);

    return 0;
}

int jpu_hw_release(jpu_hw_instance *jpu)
{
    if (!jpu)
        return 0;

    if (jpu->dec_frame_buffer_stream.size > 0)
        jdi_free_dma_memory(&jpu->dec_frame_buffer_stream);

    if (jpu->enc_frame_buffer_stream.size > 0)
        jdi_free_dma_memory(&jpu->enc_frame_buffer_stream);

    // for enc & dec
    JPU_DecClose(jpu->handle);

    if (jpu->bs_stream.size > 0) {
        jdi_free_dma_memory(&jpu->bs_stream);
        jpu->bs_stream.base = 0;
        jpu->bs_stream.size = 0;
    }

    if (jpu->bo) {
        dm_drm_bo_destroy(jpu->bo);
        jpu->bo = NULL;
    }

    if (jpu->drm_device) {
        dm_drm_destroy(jpu->drm_device);
        jpu->drm_device = NULL;
    }

    JPU_DeInit();
    DeInitLog();

    free(jpu);
    return 0;
}

/**************************************ENC**************************************************/

static void CalcSliceHeight(JpgEncOpenParam *encOP, Uint32 sliceHeight)
{
    Uint32 rotation = encOP->rotation;
    Uint32 width = (rotation == 90 || rotation == 270) ? encOP->picHeight : encOP->picWidth;
    Uint32 height = (rotation == 90 || rotation == 270) ? encOP->picWidth : encOP->picHeight;
    FrameFormat format = encOP->sourceFormat;

    if (rotation == 90 || rotation == 270) {
        if (format == FORMAT_422)
            format = FORMAT_440;
        else if (format == FORMAT_440)
            format = FORMAT_422;
    }

    encOP->sliceHeight = (sliceHeight == 0) ? height : sliceHeight;

    if (encOP->sliceHeight != height) {
        if (format == FORMAT_420 || format == FORMAT_422)
            encOP->restartInterval = width / 16;
        else
            encOP->restartInterval = width / 8;

        if (format == FORMAT_420 || format == FORMAT_440)
            encOP->restartInterval *= (encOP->sliceHeight / 16);
        else
            encOP->restartInterval *= (encOP->sliceHeight / 8);

        encOP->sliceInstMode = TRUE;
    }
}

int set_enc_params(jpu_hw_instance *jpu, JpgEncOpenParam *encOP)
{
    encOP->bitstreamBuffer = jpu->bs_stream.phys_addr;
    encOP->bitstreamBufferSize = jpu->bs_stream.size;
    encOP->picWidth = jpu->enc_input.pic_width;
    encOP->picHeight = jpu->enc_input.pic_height;
    encOP->sourceFormat = jpu->enc_input.frame_format;
    encOP->restartInterval = 0;
    encOP->streamEndian = 0;
    encOP->frameEndian = 0;

    if (jpu->enc_input.packageFormat == 1 || jpu->enc_input.packageFormat == 2)
        encOP->chromaInterleave = CBCR_INTERLEAVE;
    else
        encOP->chromaInterleave = CBCR_SEPARATED;

    memcpy(encOP->huffVal, global_huffVal, 4 * 256);
    memcpy(encOP->huffBits, global_huffBit, 4 * 256);
    for (int i = 0; i < 64; i++) {
        encOP->qMatTab[DC_TABLE_INDEX0][InvScanTable[i]] = global_qMatTab[DC_TABLE_INDEX0][i];
        encOP->qMatTab[AC_TABLE_INDEX0][InvScanTable[i]] = global_qMatTab[AC_TABLE_INDEX0][i];
    }
    memcpy(encOP->qMatTab[DC_TABLE_INDEX1], encOP->qMatTab[DC_TABLE_INDEX0], 64 * sizeof(short));
    memcpy(encOP->qMatTab[AC_TABLE_INDEX1], encOP->qMatTab[AC_TABLE_INDEX0], 64 * sizeof(short));

    encOP->jpg12bit = 0;
    encOP->q_prec0 = 0;
    encOP->q_prec1 = 0;
    encOP->packedFormat = (PackedFormat)(jpu->enc_input.packageFormat - 2);
    if (encOP->packedFormat >= PACKED_FORMAT_MAX)
        encOP->packedFormat = PACKED_FORMAT_NONE;
    encOP->pixelJustification = 0;
    encOP->tiledModeEnable = 0;
    encOP->sliceHeight = 0;
    encOP->intrEnableBit =
        ((1 << INT_JPU_DONE) | (1 << INT_JPU_ERROR) | (1 << INT_JPU_BIT_BUF_FULL));
    encOP->sliceInstMode = 0;
    encOP->rotation = 0;
    encOP->mirror = 0;

    if (encOP->packedFormat == PACKED_FORMAT_444 && encOP->sourceFormat != FORMAT_444) {
        JLOG(ERR, "Invalid operation mode : In case of using packed mode."
                  "sourceFormat must be FORMAT_444\n");
        return -1;
    }

    CalcSliceHeight(encOP, encOP->sliceHeight);

    JLOG(INFO, "------------------------------ ENCODER PARAM -------------------\n");
    JLOG(INFO, "[picWidth           ]: %d\n", encOP->picWidth);
    JLOG(INFO, "[picHeight          ]: %d\n", encOP->picHeight);
    JLOG(INFO, "[streamEndian       ]: %d\n", encOP->streamEndian);
    JLOG(INFO, "[FrameEndia         ]: %d\n", encOP->frameEndian);
    JLOG(INFO, "[sliceHeight        ]: %d\n", encOP->sliceHeight);
    JLOG(INFO, "[rotation           ]: %d\n", encOP->rotation);
    JLOG(INFO, "[tiledModeEnable    ]: %d\n", encOP->tiledModeEnable);
    JLOG(INFO, "[mirror             ]: %d\n", encOP->mirror);
    JLOG(INFO, "[srcFrameFormat     ]: %d\n", encOP->sourceFormat);
    JLOG(INFO, "[chromaInterleave   ]: %d\n", encOP->chromaInterleave);
    JLOG(INFO, "[packedFormat       ]: %d\n", encOP->packedFormat);
    JLOG(INFO, "[restartInterval    ]: %d\n", encOP->restartInterval);
    JLOG(INFO, "[jpg12bit           ]: %d\n", encOP->jpg12bit);
    JLOG(INFO, "[q_prec0            ]: %d\n", encOP->q_prec0);
    JLOG(INFO, "[q_prec1            ]: %d\n", encOP->q_prec1);
    JLOG(INFO, "[pixelJustification ]: %d\n", encOP->pixelJustification);
    JLOG(INFO, "[tiledModeEnable    ]: %d\n", encOP->tiledModeEnable);
    JLOG(INFO, "[sliceHeight        ]: %d\n", encOP->sliceHeight);
    JLOG(INFO, "[tiledModeEnable    ]: %d\n", encOP->tiledModeEnable);
    JLOG(INFO, "[sliceInstMode      ]: %d\n", encOP->sliceInstMode);
    JLOG(INFO, "----------------------------------------------------------------\n");

    return 0;
}

int calulate_enc_input_buffer_info(jpu_hw_instance *jpu, JpgEncOpenParam *encOP)
{
    Uint32 lSize = 0, cSize = 0, divc = 1;
    Uint32 Bpp = (encOP->jpg12bit == TRUE) ? 2 : 1; // the size of each pixel component
    Uint32 divw, divh;
    divw = divh = 1;
    FrameFormat format;

    if (encOP->packedFormat == PACKED_FORMAT_NONE) {
        format = encOP->sourceFormat;
    } else {
        switch (encOP->packedFormat) {
        case PACKED_FORMAT_422_YUYV:
        case PACKED_FORMAT_422_UYVY:
        case PACKED_FORMAT_422_YVYU:
        case PACKED_FORMAT_422_VYUY:
            format = FORMAT_422;
            break;

        case PACKED_FORMAT_444:
            format = FORMAT_444;
            break;

        default:
            format = FORMAT_MAX;
            break;
        }
    }

    switch (format) {
    case FORMAT_400:
        /* No chroma data */
        divw = divh = 0;
        break;

    case FORMAT_420:
        divw = 2;
        divh = 2;
        break;

    case FORMAT_422:
        divw = 2;
        divh = 1;
        break;

    case FORMAT_440:
        divw = 1;
        divh = 2;
        break;

    case FORMAT_444:
        break;

    default:
        JLOG(WARN, "%s:%d NOT SUPPORTED YUV FORMAT: %d\n", __FUNCTION__, __LINE__, format);
        break;
    }

    divc = divw * divh;
    lSize = encOP->picWidth * encOP->picHeight * Bpp;
    cSize =
        divc == 0 ? 0 : JPU_CEIL(2, encOP->picWidth) * JPU_CEIL(2, encOP->picHeight) * Bpp / divc;
    jpu->enc_input.frameSize = lSize + 2 * cSize;
    jpu->enc_input.lumaSize = lSize;
    jpu->enc_input.chromaSize = cSize;
    jpu->enc_input.lumaLineWidth = encOP->picWidth * Bpp;
    jpu->enc_input.lumaHeight = encOP->picHeight;
    jpu->enc_input.chromaLineWidth = divw == 0 ? 0 : encOP->picWidth * Bpp / divw; // note: floor
    jpu->enc_input.chromaHeight = divh == 0 ? 0 : encOP->picHeight / divh;

    if (encOP->packedFormat != PACKED_FORMAT_NONE) {
        if (encOP->packedFormat == PACKED_FORMAT_444) {
            jpu->enc_input.lumaLineWidth *= 3;
        } else {
            jpu->enc_input.lumaLineWidth *= 2;
        }

        jpu->enc_input.chromaLineWidth = 0;
        jpu->enc_input.chromaHeight = 0;
    } else {
        if (encOP->chromaInterleave != CBCR_SEPARATED) {
            jpu->enc_input.chromaLineWidth *= 2;
        }
    }
}

int jpu_enc_int(jpu_hw_instance *jpu)
{
    JpgRet ret = JPG_RET_SUCCESS;
    JpgEncOpenParam encOP = {0};
    Uint32 framebufWidth = 0, framebufHeight = 0;

    if (!jpu || jpu->bs_stream.size <= 0) {
        JLOG(ERR, "jpu_enc_ints Invalid input parameter\n");
        return -1;
    }

    // 0. enc params
    ret = set_enc_params(jpu, &encOP);
    if (ret < 0) {
        JLOG(ERR, "set_enc_params error \n");
        return -1;
    }

    // 1. open enc
    ret = JPU_EncOpen(&jpu->handle, &encOP);
    if (ret != JPG_RET_SUCCESS) {
        JLOG(ERR, "JPU_EncOpen failed Error code is 0x%x \n", ret);
        return -1;
    }

    // 2.alloca frame buffer and register to jpu
    int bEnStuffByte = 0;
    int needFrameBufCount = ENC_SRC_BUF_NUM;
    int bitDepth = (encOP.jpg12bit == 0) ? 8 : 12;
    JPU_EncGiveCommand(jpu->handle, SET_JPG_USE_STUFFING_BYTE_FF, &bEnStuffByte);
    framebufWidth = (encOP.sourceFormat == FORMAT_420 || encOP.sourceFormat == FORMAT_422)
                        ? JPU_CEIL(16, encOP.picWidth)
                        : JPU_CEIL(8, encOP.picWidth);
    framebufHeight = (encOP.sourceFormat == FORMAT_420 || encOP.sourceFormat == FORMAT_440)
                         ? JPU_CEIL(16, encOP.picHeight)
                         : JPU_CEIL(8, encOP.picHeight);

    {
        uint64_t last_addr;
        Uint32 fbLumaStride, fbLumaHeight, fbChromaStride, fbChromaHeight;
        Uint32 fbLumaSize, fbChromaSize, fbSize;
        Uint32 bytePerPixel = (bitDepth + 7) / 8;

        if (encOP.rotation == 90 || encOP.rotation == 270) {
            if (encOP.sourceFormat == FORMAT_422)
                encOP.sourceFormat = FORMAT_440;
            else if (encOP.sourceFormat == FORMAT_440)
                encOP.sourceFormat = FORMAT_422;
        }

        GetFrameBufStride(encOP.sourceFormat, encOP.chromaInterleave, encOP.packedFormat, 0,
                          framebufWidth, framebufHeight, bytePerPixel, &fbLumaStride, &fbLumaHeight,
                          &fbChromaStride, &fbChromaHeight);
        fbLumaSize = fbLumaStride * fbLumaHeight;
        fbChromaSize = fbChromaStride * fbChromaHeight;

        if (encOP.chromaInterleave == CBCR_SEPARATED) {
            /* fbChromaSize MUST be zero when format is packed mode */
            fbSize = fbLumaSize + 2 * fbChromaSize;
        } else {
            /* Semi-planar */
            fbSize = fbLumaSize + fbChromaSize;
        }

        jpu->enc_frame_buffer_stream.size = fbSize * needFrameBufCount;

        if (jdi_allocate_dma_memory(&jpu->enc_frame_buffer_stream) < 0) {
            JLOG(ERR, "Fail to allocate frame buffer size=%d\n", jpu->enc_frame_buffer_stream.size);
            goto ERR_INIT_ENC;
        }

        last_addr = jpu->enc_frame_buffer_stream.phys_addr;

        // clear frame buffer
        memset(jdi_get_memory(last_addr), 0x00, jpu->enc_frame_buffer_stream.size);

        for (int i = 0; i < needFrameBufCount; i++) {
            jpu->enc_frameBuf[i].bufY = last_addr;
            last_addr += fbLumaSize;
            last_addr = JPU_CEIL(8, last_addr);

            if (fbChromaSize) {
                jpu->enc_frameBuf[i].bufCb = last_addr;
                last_addr += fbChromaSize;
                last_addr = JPU_CEIL(8, last_addr);

                if (encOP.chromaInterleave == CBCR_SEPARATED) {
                    jpu->enc_frameBuf[i].bufCr = last_addr;
                    last_addr += fbChromaSize;
                    last_addr = JPU_CEIL(8, last_addr);
                }
            } else {
                jpu->enc_frameBuf[i].bufCb = 0;
                jpu->enc_frameBuf[i].bufCr = 0;
            }
            jpu->enc_frameBuf[i].stride = fbLumaStride;
            jpu->enc_frameBuf[i].strideC = fbChromaStride;
            jpu->enc_frameBuf[i].fbChromaHeight = fbLumaHeight;
            jpu->enc_frameBuf[i].fbLumaHeight = fbChromaHeight;
            jpu->enc_frameBuf[i].endian = encOP.frameEndian;
            jpu->enc_frameBuf[i].format = encOP.sourceFormat;
            jpu->enc_frameBuf[i].fbsize = fbSize;
        }
    }

    JLOG(INFO, "framebuffer stride = %d, width = %d, height = %d\n", jpu->enc_frameBuf[0].stride,
         framebufWidth, framebufHeight);

    // 3.calulate input buffer info
    calulate_enc_input_buffer_info(jpu, &encOP);

    return 0;

ERR_INIT_ENC:
    if (jpu->enc_frame_buffer_stream.size > 0) {
        jdi_free_dma_memory(&jpu->enc_frame_buffer_stream);
        jpu->enc_frame_buffer_stream.base = 0;
        jpu->enc_frame_buffer_stream.size = 0;
    }

    memset(jpu->enc_frameBuf, 0, sizeof(jpu->enc_frameBuf));

    JPU_EncClose(jpu->handle);
    return -1;
}

int CopyYuvDataFromBuf(jpu_hw_instance *jpu, PhysicalAddress fbAddr, Uint32 fbStride,
                       Uint32 fbHeight, Uint32 dataStride, Uint32 dataHeight, void *buf)
{
    PhysicalAddress addr = fbAddr;
    int feed_size = 0;

    for (int i = 0; i < dataHeight; i++) {
        memcpy(jdi_get_memory(addr), buf + feed_size, dataStride);

        addr += fbStride;
        feed_size += dataStride;
    }
    return feed_size;
}

int CopyYuvData(jpu_hw_instance *jpu, PhysicalAddress fbAddr, Uint32 fbStride, Uint32 fbHeight,
                Uint32 dataStride, Uint32 dataHeight)
{
    PhysicalAddress addr = fbAddr;
    int feed_size = 0;

    for (int i = 0; i < dataHeight; i++) {
        jpu->fill_buffer_callback(jpu->user_private_ptr, dataStride, jdi_get_memory(addr),
                                  &feed_size);

        if (feed_size != dataStride) {
            JLOG(ERR, "Not getting enough input buffer\n");
            return -1;
        }

        addr += fbStride;
    }
    return 0;
}

int jpu_enc_feed_input_buff(jpu_hw_instance *jpu, FrameBuffer *fb)
{
    Uint32 nread;
    BOOL success = TRUE;
    int fill_input_size = 0;

    PackedFormat packed_format = jpu->handle->JpgInfo->encInfo.packedFormat;

    // clear frame buffer, already do it when first apply for the framebuffer
    // memset(jdi_get_memory(fb->bufY), 0x00, fb->fbsize);

    switch (jpu->enc_input.frame_format) {
    case FORMAT_400:
        if (CopyYuvData(jpu, fb->bufY, fb->stride, fb->fbLumaHeight, jpu->enc_input.lumaLineWidth,
                        jpu->enc_input.lumaHeight) < 0)
            success = FALSE;
        break;
    case FORMAT_420:
    case FORMAT_422:
    case FORMAT_440:
    case FORMAT_444: // pEncConfig->packedFormat
        if (packed_format == PACKED_FORMAT_NONE) {
            if (CopyYuvData(jpu, fb->bufY, fb->stride, fb->fbLumaHeight,
                            jpu->enc_input.lumaLineWidth, jpu->enc_input.lumaHeight) < 0)
                success = FALSE;

            if (jpu->handle->JpgInfo->encInfo.chromaInterleave == CBCR_SEPARATED) {
                if (CopyYuvData(jpu, fb->bufCb, fb->strideC, fb->fbChromaHeight,
                                jpu->enc_input.chromaLineWidth, jpu->enc_input.chromaHeight) < 0)
                    success = FALSE;
                if (CopyYuvData(jpu, fb->bufCr, fb->strideC, fb->fbChromaHeight,
                                jpu->enc_input.chromaLineWidth, jpu->enc_input.chromaHeight) < 0)
                    success = FALSE;
            } else {
                if (CopyYuvData(jpu, fb->bufCb, fb->strideC, fb->fbChromaHeight,
                                jpu->enc_input.chromaLineWidth, jpu->enc_input.chromaHeight) < 0)
                    success = FALSE;
            }
        } else {
            if (CopyYuvData(jpu, fb->bufY, fb->stride, fb->fbLumaHeight,
                            jpu->enc_input.lumaLineWidth, jpu->enc_input.lumaHeight) < 0)
                success = FALSE;
        }
        break;
    default:
        JLOG(ERR, "%s:%d NOT SUPPORTED YUV FORMAT:%d\n", __FUNCTION__, __LINE__,
             jpu->enc_input.frame_format);
        success = FALSE;
        break;
    }

    // dump input buffer
    // {
    // 	FILE * fp = fopen("input.yuv", "wb");
    //     fwrite(jdi_get_memory(fb->bufY), 1, fb->fbsize, fp);
    // 	fclose (fp);
    // }

    return success == FALSE ? -1 : 0;
}

int jpu_enc_feed_input_buff_from_dma(jpu_hw_instance *jpu, FrameBuffer *fb, jpu_buffer_t *dma_buf)
{
    Uint32 nread;
    BOOL success = TRUE;
    int fill_input_size = 0;
    void *dma_buf_ptr = NULL;

    PackedFormat packed_format = jpu->handle->JpgInfo->encInfo.packedFormat;

    dma_buf_ptr = jdi_mmap_dma_buf(dma_buf);
    if (!dma_buf_ptr) {
        JLOG(ERR, "jdi_mmap_dma_buf failed\n");
        return -1;
    }

    switch (jpu->enc_input.frame_format) {
    case FORMAT_400:
        fill_input_size += CopyYuvDataFromBuf(
            jpu, fb->bufY, fb->stride, fb->fbLumaHeight, jpu->enc_input.lumaLineWidth,
            jpu->enc_input.lumaHeight, dma_buf_ptr + fill_input_size);
        break;
    case FORMAT_420:
    case FORMAT_422:
    case FORMAT_440:
    case FORMAT_444: // pEncConfig->packedFormat
        if (packed_format == PACKED_FORMAT_NONE) {
            fill_input_size += CopyYuvDataFromBuf(
                jpu, fb->bufY, fb->stride, fb->fbLumaHeight, jpu->enc_input.lumaLineWidth,
                jpu->enc_input.lumaHeight, dma_buf_ptr + fill_input_size);

            if (jpu->handle->JpgInfo->encInfo.chromaInterleave == CBCR_SEPARATED) {
                fill_input_size += CopyYuvDataFromBuf(
                    jpu, fb->bufCb, fb->strideC, fb->fbChromaHeight, jpu->enc_input.chromaLineWidth,
                    jpu->enc_input.chromaHeight, dma_buf_ptr + fill_input_size);

                fill_input_size += CopyYuvDataFromBuf(
                    jpu, fb->bufCr, fb->strideC, fb->fbChromaHeight, jpu->enc_input.chromaLineWidth,
                    jpu->enc_input.chromaHeight, dma_buf_ptr + fill_input_size);
            } else {
                fill_input_size += CopyYuvDataFromBuf(
                    jpu, fb->bufCb, fb->strideC, fb->fbChromaHeight, jpu->enc_input.chromaLineWidth,
                    jpu->enc_input.chromaHeight, dma_buf_ptr + fill_input_size);
            }
        } else {
            fill_input_size += CopyYuvDataFromBuf(
                jpu, fb->bufY, fb->stride, fb->fbLumaHeight, jpu->enc_input.lumaLineWidth,
                jpu->enc_input.lumaHeight, dma_buf_ptr + fill_input_size);
        }
        break;
    default:
        JLOG(ERR, "%s:%d NOT SUPPORTED YUV FORMAT:%d\n", __FUNCTION__, __LINE__,
             jpu->enc_input.frame_format);
        success = FALSE;
        break;
    }

    munmap(dma_buf_ptr, dma_buf->size);

    // dump input buffer
    // {
    // 	FILE * fp = fopen("input.yuv", "wb");
    //     fwrite(jdi_get_memory(fb->bufY), 1, fb->fbsize, fp);
    // 	fclose (fp);
    // }

    return success == FALSE ? -1 : 0;
}

static int send_output_buffer_to_user(jpu_hw_instance *jpu, jpu_buffer_t bs,
                                      JpgEncOutputInfo *pEncOutput)
{
    JpgRet ret = JPG_RET_SUCCESS;
    PhysicalAddress paBsBufStart;
    PhysicalAddress paBsBufEnd;
    PhysicalAddress paBsBufWrPtr;
    PhysicalAddress paBsBufRdPtr;
    Int32 size = 0;
    Uint32 loadSize = 0;
    paBsBufStart = bs.phys_addr;
    paBsBufEnd = bs.phys_addr + bs.size;

    if (pEncOutput) {
        paBsBufWrPtr = pEncOutput->streamWrPtr;
        paBsBufRdPtr = pEncOutput->streamRdPtr;
        size = (Uint32)(paBsBufWrPtr - paBsBufRdPtr);
        ;
    } else {
        if (JPG_RET_SUCCESS !=
            (ret = JPU_EncGetBitstreamBuffer(jpu->handle, &paBsBufRdPtr, &paBsBufWrPtr, &size))) {
            JLOG(ERR, "<%s:%d> JPU_EncGetBitstreamBuffer failed Error code is 0x%x \n",
                 __FUNCTION__, __LINE__, ret);
            return -1;
        }
    }

    loadSize = size;

    if (loadSize > 0) {
        int room = 0;

        if ((paBsBufRdPtr + size) > paBsBufEnd) {
            room = paBsBufEnd - paBsBufRdPtr;
            jpu->read_buffer_callback(jpu->user_private_ptr, jdi_get_memory(paBsBufRdPtr), room);
            jpu->read_buffer_callback(jpu->user_private_ptr, jdi_get_memory(paBsBufStart),
                                      (size - room));
        } else {
            jpu->read_buffer_callback(jpu->user_private_ptr, jdi_get_memory(paBsBufRdPtr), size);
        }

        if (pEncOutput && (ENCODE_STATE_FRAME_DONE == pEncOutput->encodeState)) {
            loadSize = STREAM_END_SIZE; // after done of one frame. set the current wrPtr to to the
                                        // base address in the bitstream buffer.
        }

        ret = JPU_EncUpdateBitstreamBuffer(jpu->handle, loadSize);

        if (JPG_RET_SUCCESS != ret) {
            JLOG(ERR, "<%s:%d> VPU_EncUpdateBitstreamBuffer failed Error code is 0x%x \n",
                 __FUNCTION__, __LINE__, ret);
            return -1;
        }
    }
    return 0;
}

int jpu_encompress(jpu_hw_instance *jpu)
{
    int ret = 0;
    int feedSize = 0;
    int int_reason = 0;
    int src_frame_idx = 0;
    static int frameIdx = 0;

    JpgEncParam encParam = {0};
    JpgEncOutputInfo outputInfo = {0};
    JpgEncParamSet encHeaderParam = {0};
    JpgRet jpu_ret = JPG_RET_SUCCESS;

    if (!jpu || jpu->bs_stream.size <= 0 || !jpu->fill_buffer_callback ||
        !jpu->read_buffer_callback || jpu->enc_input.pic_width <= 0 ||
        jpu->enc_input.pic_height <= 0) {
        JLOG(ERR, "jpu_enc_init Invalid input parameter\n");
        printf("jpu:%p  jpu->bs_stream.size:%d jpu->fill_buffer_callback:%p, "
               "jpu->read_buffer_callback:%p "
               "jpu->enc_input.pic_width:%d jpu->enc_input.pic_height:%d\n",
               jpu, jpu->bs_stream.size, jpu->fill_buffer_callback, jpu->read_buffer_callback,
               jpu->enc_input.pic_width, jpu->enc_input.pic_height);
        return -1;
    }

    if (!jpu->jpu_enc_init_success) {
        ret = jpu_enc_int(jpu);
        if (ret < 0) {
            JLOG(ERR, "jpu_enc_int failed\n");
            return -1;
        }
        jpu->jpu_enc_init_success = 1;
    }

    outputInfo.encodeState = ENCODE_STATE_NEW_FRAME;

    while (1) {
        if (outputInfo.encodeState == ENCODE_STATE_NEW_FRAME) { // means new frame start
            src_frame_idx = (frameIdx % ENC_SRC_BUF_NUM);

            encHeaderParam.size = jpu->bs_stream.size;
            encHeaderParam.pParaSet = malloc(STREAM_BUF_SIZE);
            encHeaderParam.headerMode = ENC_HEADER_MODE_NORMAL;
            encHeaderParam.quantMode = JPG_TBL_NORMAL;
            encHeaderParam.huffMode = JPG_TBL_NORMAL;
            encHeaderParam.disableAPPMarker = 0;
            encHeaderParam.enableSofStuffing = TRUE;

            if (encHeaderParam.pParaSet) {
                // make picture header
                JPU_EncGiveCommand(
                    jpu->handle, ENC_JPG_GET_HEADER,
                    &encHeaderParam); // return exact header size int endHeaderparam.siz;
                JLOG(INFO, "JPU_EncGiveCommand[ENC_JPG_GET_HEADER] header size=%d\n",
                     encHeaderParam.size);

                jpu->read_buffer_callback(jpu->user_private_ptr, encHeaderParam.pParaSet,
                                          encHeaderParam.size);
                free(encHeaderParam.pParaSet);
            }

            // fill input buffer
            ret = jpu_enc_feed_input_buff(jpu, &jpu->enc_frameBuf[src_frame_idx]);
            if (ret < 0) {
                JLOG(ERR, "jpu_enc_feed_input_buff failed Error code is 0x%x \n", ret);
                return -1;
            }

            encParam.sourceFrame = &jpu->enc_frameBuf[src_frame_idx];
        }

        // Start encoding a frame.
        ret = JPU_EncStartOneFrame(jpu->handle, &encParam);

        if (ret != JPG_RET_SUCCESS) {
            JLOG(ERR, "JPU_EncStartOneFrame failed Error code is 0x%x \n", ret);
            return -1;
        }

        while (1) {
            int_reason = JPU_WaitInterrupt(jpu->handle, JPU_INTERRUPT_TIMEOUT_MS);

            if (int_reason == -1) {
                JLOG(ERR, "[%s][%d] Error : timeout happened\n", __func__, __LINE__);
                JPU_SWReset(jpu->handle);
                return -1;
            }

            if (int_reason == -2) {
                JLOG(ERR, "Interrupt occurred. but this interrupt is not for my instance\n");
                return -1;
            }

            // Must catch PIC_DONE interrupt before catching EMPTY interrupt
            if (int_reason & (1 << INT_JPU_DONE) || int_reason & (1 << INT_JPU_SLICE_DONE)) {
                break;
            }

            if (int_reason & (1 << INT_JPU_BIT_BUF_FULL)) {
                JLOG(INFO, "INT_JPU_BIT_BUF_FULL interrupt issued\n");

                send_output_buffer_to_user(jpu, jpu->bs_stream, NULL);

                JPU_ClrStatus(jpu->handle, (1 << INT_JPU_BIT_BUF_FULL));
                JPU_EnableInterrput(jpu->handle, jpu->handle->JpgInfo->encInfo.intrEnableBit);
            }
        }

        if ((ret = JPU_EncGetOutputInfo(jpu->handle, &outputInfo)) != JPG_RET_SUCCESS) {
            JLOG(ERR, "JPU_EncGetOutputInfo failed Error code is 0x%x \n", ret);
            return -1;
        }

        send_output_buffer_to_user(jpu, jpu->bs_stream, &outputInfo);

        if (outputInfo.encodeState == ENCODE_STATE_SLICE_DONE) {
            JLOG(TRACE, "Enc Slice: %d, rdPtr=0x%x, wrPtr=0x%x, slice height=%d\n", frameIdx,
                 outputInfo.streamRdPtr, outputInfo.streamWrPtr, outputInfo.encodedSliceYPos);
        } else {
            JLOG(TRACE, "Enc: %d, rdPtr=0x%x, wrPtr=0x%x cycles=%d\n", frameIdx,
                 outputInfo.streamRdPtr, outputInfo.streamWrPtr, outputInfo.frameCycle);
            frameIdx++;
            break;
        }
    }
    return 0;
}

int jpu_enc_init_from_fd(jpu_hw_instance *jpu)
{
    JpgRet ret = JPG_RET_SUCCESS;
    JpgEncOpenParam encOP = {0};

    if (!jpu || jpu->bs_stream.size <= 0) {
        JLOG(ERR, "jpu_enc_ints Invalid input parameter\n");
        return -1;
    }

    // 0. enc params
    ret = set_enc_params(jpu, &encOP);
    if (ret < 0) {
        JLOG(ERR, "set_enc_params error \n");
        return -1;
    }

    // 1. open enc
    ret = JPU_EncOpen(&jpu->handle, &encOP);
    if (ret != JPG_RET_SUCCESS) {
        JLOG(ERR, "JPU_EncOpen failed Error code is 0x%x \n", ret);
        return -1;
    }

    // 2.alloca frame buffer and register to jpu
    int bEnStuffByte = 0;
    JPU_EncGiveCommand(jpu->handle, SET_JPG_USE_STUFFING_BYTE_FF, &bEnStuffByte);

    // 3.calulate input buffer info
    calulate_enc_input_buffer_info(jpu, &encOP);

    return 0;
}

int jpu_encompress_from_fd(jpu_hw_instance *jpu, int yuv_fd)
{
    int ret = 0;
    int feedSize = 0;
    int int_reason = 0;
    int src_frame_idx = 0;
    int bitDepth = 0;
    static int frameIdx = 0;

    JpgEncInfo *encInfo = NULL;
    JpgEncParam encParam = {0};
    JpgEncOutputInfo outputInfo = {0};
    JpgEncParamSet encHeaderParam = {0};
    JpgRet jpu_ret = JPG_RET_SUCCESS;
    jpu_buffer_t input_buffer = {0};
    Uint32 framebufWidth = 0, framebufHeight = 0;

    if (!jpu || jpu->bs_stream.size <= 0 || !jpu->read_buffer_callback ||
        jpu->enc_input.pic_width <= 0 || jpu->enc_input.pic_height <= 0) {
        JLOG(ERR, "jpu_enc_init Invalid input parameter\n");
        printf("jpu:%p  jpu->bs_stream.size:%d, jpu->read_buffer_callback:%p "
               "jpu->enc_input.pic_width:%d jpu->enc_input.pic_height:%d\n",
               jpu, jpu->bs_stream.size, jpu->read_buffer_callback, jpu->enc_input.pic_width,
               jpu->enc_input.pic_height);
        return -1;
    }

    if (!jpu->jpu_enc_init_success) {
        ret = jpu_enc_init_from_fd(jpu);
        if (ret < 0) {
            JLOG(ERR, "jpu_enc_init_from_fd\n");
            return -1;
        }

        jpu->jpu_enc_init_success = 1;
    }

    encInfo = &jpu->handle->JpgInfo->encInfo;
    bitDepth = (encInfo->jpg12bit == 0) ? 8 : 12;

    framebufWidth =
        (jpu->enc_input.frame_format == FORMAT_420 || jpu->enc_input.frame_format == FORMAT_422)
            ? JPU_CEIL(16, encInfo->picWidth)
            : JPU_CEIL(8, encInfo->picWidth);
    framebufHeight =
        (jpu->enc_input.frame_format == FORMAT_420 || jpu->enc_input.frame_format == FORMAT_440)
            ? JPU_CEIL(16, encInfo->picHeight)
            : JPU_CEIL(8, encInfo->picHeight);

    // checkout input fd size
    {
        uint64_t last_addr;
        Uint32 fbLumaStride, fbLumaHeight, fbChromaStride, fbChromaHeight;
        Uint32 fbLumaSize, fbChromaSize, fbSize;
        Uint32 bytePerPixel = (bitDepth + 7) / 8;

        // TODO: support rotation
        // if (encInfo->rotation == 90 || encInfo->rotation == 270) {
        // 	if (jpu->enc_input.frame_format == FORMAT_422) jpu->enc_input.frame_format = FORMAT_440;
        // 	else if (jpu->enc_input.frame_format == FORMAT_440) jpu->enc_input.frame_format =
        // FORMAT_422;
        // }

        GetFrameBufStride(jpu->enc_input.frame_format, encInfo->chromaInterleave,
                          encInfo->packedFormat, 0, framebufWidth, framebufHeight, bytePerPixel,
                          &fbLumaStride, &fbLumaHeight, &fbChromaStride, &fbChromaHeight);
        fbLumaSize = fbLumaStride * fbLumaHeight;
        fbChromaSize = fbChromaStride * fbChromaHeight;

        if (encInfo->chromaInterleave == CBCR_SEPARATED) {
            /* fbChromaSize MUST be zero when format is packed mode */
            fbSize = fbLumaSize + 2 * fbChromaSize;
        } else {
            /* Semi-planar */
            fbSize = fbLumaSize + fbChromaSize;
        }

        input_buffer.buf_handle = yuv_fd;

        if (jdi_device_memory_map(&input_buffer)) {
            JLOG(ERR, "jdi_device_memory_map error\n");
            return -1;
        }

        if (JPU_CEIL(PAGE_SIZE, jpu->enc_input.frameSize) != input_buffer.size) {
            JLOG(ERR,
                 "The size of the input dma fd buffer does not match the input parameters, "
                 "dma buffer size:%x input buffer calculated size:%x\n",
                 input_buffer.size, jpu->enc_input.frameSize);
            ret = -1;
            goto ENC_FROM_FD_DONE;
        }

        // checkout input width & height
        if (framebufWidth != encInfo->picWidth || framebufHeight != encInfo->picHeight) {
            JLOG(INFO,
                 "the width and height of the input buffer are not aligned, manual alignment\n");

            // assuming that the user input data format does not change during encoding process
            if (jpu->enc_frame_buffer_stream.size == 0) {
                jpu->enc_frame_buffer_stream.size = fbSize;
                if (jdi_allocate_dma_memory(&jpu->enc_frame_buffer_stream) < 0) {
                    JLOG(ERR, "Fail to allocate frame buffer size=%d\n",
                         jpu->enc_frame_buffer_stream.size);
                    ret = -1;
                    goto ENC_FROM_FD_DONE;
                }
                // clear frame buffer
                memset(jdi_get_memory(jpu->enc_frame_buffer_stream.phys_addr), 0x00,
                       jpu->enc_frame_buffer_stream.size);
            }
            last_addr = jpu->enc_frame_buffer_stream.phys_addr;
        } else {
            last_addr = input_buffer.phys_addr;
        }

        {
            jpu->enc_frameBuf[0].bufY = last_addr;
            last_addr += fbLumaSize;
            last_addr = JPU_CEIL(8, last_addr);

            if (fbChromaSize) {
                jpu->enc_frameBuf[0].bufCb = last_addr;
                last_addr += fbChromaSize;
                last_addr = JPU_CEIL(8, last_addr);

                if (encInfo->chromaInterleave == CBCR_SEPARATED) {
                    jpu->enc_frameBuf[0].bufCr = last_addr;
                    last_addr += fbChromaSize;
                    last_addr = JPU_CEIL(8, last_addr);
                }
            } else {
                jpu->enc_frameBuf[0].bufCb = 0;
                jpu->enc_frameBuf[0].bufCr = 0;
            }
            jpu->enc_frameBuf[0].stride = fbLumaStride;
            jpu->enc_frameBuf[0].strideC = fbChromaStride;
            jpu->enc_frameBuf[0].fbChromaHeight = fbLumaHeight;
            jpu->enc_frameBuf[0].fbLumaHeight = fbChromaHeight;
            jpu->enc_frameBuf[0].endian = encInfo->frameEndian;
            jpu->enc_frameBuf[0].format = jpu->enc_input.frame_format;
            jpu->enc_frameBuf[0].fbsize = fbSize;
        }
        JLOG(
            INFO,
            "fbLumaStride %d, fbChromaStride = %d, fbLumaHeight = %d fbChromaHeight:%d fbSize:%d\n",
            fbLumaStride, fbChromaStride, fbLumaHeight, fbChromaHeight, fbSize);
    }

    outputInfo.encodeState = ENCODE_STATE_NEW_FRAME;

    while (1) {
        if (outputInfo.encodeState == ENCODE_STATE_NEW_FRAME) { // means new frame start
            src_frame_idx = 0;

            encHeaderParam.size = jpu->bs_stream.size;
            encHeaderParam.pParaSet = malloc(STREAM_BUF_SIZE);
            encHeaderParam.headerMode = ENC_HEADER_MODE_NORMAL;
            encHeaderParam.quantMode = JPG_TBL_NORMAL;
            encHeaderParam.huffMode = JPG_TBL_NORMAL;
            encHeaderParam.disableAPPMarker = 0;
            encHeaderParam.enableSofStuffing = TRUE;

            if (encHeaderParam.pParaSet) {
                // make picture header
                JPU_EncGiveCommand(
                    jpu->handle, ENC_JPG_GET_HEADER,
                    &encHeaderParam); // return exact header size int endHeaderparam.siz;
                JLOG(INFO, "JPU_EncGiveCommand[ENC_JPG_GET_HEADER] header size=%d\n",
                     encHeaderParam.size);

                jpu->read_buffer_callback(jpu->user_private_ptr, encHeaderParam.pParaSet,
                                          encHeaderParam.size);
                free(encHeaderParam.pParaSet);
            }

            // if need to realign, fill the input buffer
            if (framebufWidth != encInfo->picWidth || framebufHeight != encInfo->picHeight) {
                ret = jpu_enc_feed_input_buff_from_dma(jpu, &jpu->enc_frameBuf[src_frame_idx],
                                                       &input_buffer);
                if (ret < 0) {
                    JLOG(ERR, "jpu_enc_feed_input_buff_from_dma failed Error code is %d \n", ret);
                    ret = -1;
                    goto ENC_FROM_FD_DONE;
                }
            }

            encParam.sourceFrame = &jpu->enc_frameBuf[src_frame_idx];
        }

        // Start encoding a frame.
        ret = JPU_EncStartOneFrame(jpu->handle, &encParam);

        if (ret != JPG_RET_SUCCESS) {
            JLOG(ERR, "JPU_EncStartOneFrame failed Error code is %d \n", ret);
            ret = -1;
            goto ENC_FROM_FD_DONE;
        }

        while (1) {
            int_reason = JPU_WaitInterrupt(jpu->handle, JPU_INTERRUPT_TIMEOUT_MS);

            if (int_reason == -1) {
                JLOG(ERR, "[%s][%d] Error : timeout happened\n", __func__, __LINE__);
                JPU_SWReset(jpu->handle);
                ret = -1;
                goto ENC_FROM_FD_DONE;
            }

            if (int_reason == -2) {
                JLOG(ERR, "Interrupt occurred. but this interrupt is not for my instance\n");
                ret = -1;
                goto ENC_FROM_FD_DONE;
            }

            // Must catch PIC_DONE interrupt before catching EMPTY interrupt
            if (int_reason & (1 << INT_JPU_DONE) || int_reason & (1 << INT_JPU_SLICE_DONE)) {
                break;
            }

            if (int_reason & (1 << INT_JPU_BIT_BUF_FULL)) {
                JLOG(INFO, "INT_JPU_BIT_BUF_FULL interrupt issued\n");

                send_output_buffer_to_user(jpu, jpu->bs_stream, NULL);

                JPU_ClrStatus(jpu->handle, (1 << INT_JPU_BIT_BUF_FULL));
                JPU_EnableInterrput(jpu->handle, jpu->handle->JpgInfo->encInfo.intrEnableBit);
            }
        }

        if ((ret = JPU_EncGetOutputInfo(jpu->handle, &outputInfo)) != JPG_RET_SUCCESS) {
            JLOG(ERR, "JPU_EncGetOutputInfo failed Error code is 0x%x \n", ret);
            ret = -1;
            goto ENC_FROM_FD_DONE;
        }

        send_output_buffer_to_user(jpu, jpu->bs_stream, &outputInfo);

        if (outputInfo.encodeState == ENCODE_STATE_SLICE_DONE) {
            JLOG(TRACE, "Enc Slice: %d, rdPtr=0x%x, wrPtr=0x%x, slice height=%d\n", frameIdx,
                 outputInfo.streamRdPtr, outputInfo.streamWrPtr, outputInfo.encodedSliceYPos);
        } else {
            JLOG(TRACE, "Enc: %d, rdPtr=0x%x, wrPtr=0x%x cycles=%d\n", frameIdx,
                 outputInfo.streamRdPtr, outputInfo.streamWrPtr, outputInfo.frameCycle);
            frameIdx++;
            break;
        }
    }

ENC_FROM_FD_DONE:
    if (input_buffer.phys_addr > 0)
        jdi_device_memory_unmap(&input_buffer);

    return ret;
}

int jpu_enc_feed_input_buff_from_yuv_ptr(jpu_hw_instance *jpu, FrameBuffer *fb,
                                         unsigned char **y_ptr, unsigned char **u_ptr,
                                         unsigned char **v_ptr, int rows, int feed_row)
{
    int fill_input_size = 0;
    int divh = jpu->enc_input.chromaHeight == 0
                   ? 0
                   : jpu->enc_input.lumaHeight / jpu->enc_input.chromaHeight;

    PackedFormat packed_format = jpu->handle->JpgInfo->encInfo.packedFormat;

    if (packed_format != PACKED_FORMAT_NONE) {
        JLOG(ERR, "[%s][%d] this func only support 3 planar foramt\n", __func__, __LINE__);
        return -1;
    }

    for (int i = 0; i < rows; i++) {
        if (feed_row + i == jpu->enc_input.pic_height)
            return i; // Filling is complete

        memcpy(jdi_get_memory(fb->bufY + (feed_row + i) * fb->stride), y_ptr[i],
               jpu->enc_input.lumaLineWidth);
        if (divh && (i % divh == 0)) {
            memcpy(jdi_get_memory(fb->bufCb + (feed_row + i) / divh * fb->strideC), u_ptr[i / divh],
                   jpu->enc_input.chromaLineWidth);
            memcpy(jdi_get_memory(fb->bufCr + (feed_row + i) / divh * fb->strideC), v_ptr[i / divh],
                   jpu->enc_input.chromaLineWidth);
        }
    }

    return rows;
}

int jpu_encompress_fill_input_buffer(jpu_hw_instance *jpu, unsigned char **y_ptr,
                                     unsigned char **u_ptr, unsigned char **v_ptr, int rows)
{
    int ret = 0;
    int int_reason = 0;
    int src_frame_idx = 0;
    static int frameIdx = 0;
    static int feed_row = 0;

    JpgEncParam encParam = {0};
    JpgEncOutputInfo outputInfo = {0};
    JpgEncParamSet encHeaderParam = {0};
    JpgRet jpu_ret = JPG_RET_SUCCESS;

    if (!jpu || jpu->bs_stream.size <= 0 || !jpu->read_buffer_callback ||
        jpu->enc_input.pic_width <= 0 || jpu->enc_input.pic_height <= 0) {
        JLOG(ERR, "jpu_enc_init Invalid input parameter\n");
        printf("jpu:%p  jpu->bs_stream.size:%d jpu->fill_buffer_callback:%p, "
               "jpu->read_buffer_callback:%p "
               "jpu->enc_input.pic_width:%d jpu->enc_input.pic_height:%d\n",
               jpu, jpu->bs_stream.size, jpu->fill_buffer_callback, jpu->read_buffer_callback,
               jpu->enc_input.pic_width, jpu->enc_input.pic_height);
        return -1;
    }

    if (!jpu->jpu_enc_init_success) {
        ret = jpu_enc_int(jpu);
        if (ret < 0) {
            JLOG(ERR, "jpu_enc_int failed\n");
            return -1;
        }
        jpu->jpu_enc_init_success = 1;
    }

    ret = jpu_enc_feed_input_buff_from_yuv_ptr(jpu, &jpu->enc_frameBuf[0], y_ptr, u_ptr, v_ptr,
                                               rows, feed_row);
    if (ret < 0) {
        JLOG(ERR, "jpu_enc_feed_input_buff failed Error code is 0x%x \n", ret);
        return -1;
    }

    feed_row += ret;

    if (feed_row < jpu->enc_input.pic_height) {
        return -2; // input buffer is not enough
    }

    outputInfo.encodeState = ENCODE_STATE_NEW_FRAME;
    while (1) {
        if (outputInfo.encodeState == ENCODE_STATE_NEW_FRAME) { // means new frame start
            src_frame_idx = 0;

            encHeaderParam.size = jpu->bs_stream.size;
            encHeaderParam.pParaSet = malloc(STREAM_BUF_SIZE);
            encHeaderParam.headerMode = ENC_HEADER_MODE_NORMAL;
            encHeaderParam.quantMode = JPG_TBL_NORMAL;
            encHeaderParam.huffMode = JPG_TBL_NORMAL;
            encHeaderParam.disableAPPMarker = 0;
            encHeaderParam.enableSofStuffing = TRUE;

            if (encHeaderParam.pParaSet) {
                // make picture header
                JPU_EncGiveCommand(
                    jpu->handle, ENC_JPG_GET_HEADER,
                    &encHeaderParam); // return exact header size int endHeaderparam.siz;
                JLOG(INFO, "JPU_EncGiveCommand[ENC_JPG_GET_HEADER] header size=%d\n",
                     encHeaderParam.size);

                jpu->read_buffer_callback(jpu->user_private_ptr, encHeaderParam.pParaSet,
                                          encHeaderParam.size);
                free(encHeaderParam.pParaSet);
            }

            encParam.sourceFrame = &jpu->enc_frameBuf[src_frame_idx];
        }

        // Start encoding a frame.
        ret = JPU_EncStartOneFrame(jpu->handle, &encParam);

        if (ret != JPG_RET_SUCCESS) {
            JLOG(ERR, "JPU_EncStartOneFrame failed Error code is 0x%x \n", ret);
            return -1;
        }

        while (1) {
            int_reason = JPU_WaitInterrupt(jpu->handle, JPU_INTERRUPT_TIMEOUT_MS);

            if (int_reason == -1) {
                JLOG(ERR, "[%s][%d] Error : timeout happened\n", __func__, __LINE__);
                JPU_SWReset(jpu->handle);
                return -1;
            }

            if (int_reason == -2) {
                JLOG(ERR, "Interrupt occurred. but this interrupt is not for my instance\n");
                return -1;
            }

            // Must catch PIC_DONE interrupt before catching EMPTY interrupt
            if (int_reason & (1 << INT_JPU_DONE) || int_reason & (1 << INT_JPU_SLICE_DONE)) {
                break;
            }

            if (int_reason & (1 << INT_JPU_BIT_BUF_FULL)) {
                JLOG(INFO, "INT_JPU_BIT_BUF_FULL interrupt issued\n");

                send_output_buffer_to_user(jpu, jpu->bs_stream, NULL);

                JPU_ClrStatus(jpu->handle, (1 << INT_JPU_BIT_BUF_FULL));
                JPU_EnableInterrput(jpu->handle, jpu->handle->JpgInfo->encInfo.intrEnableBit);
            }
        }

        if ((ret = JPU_EncGetOutputInfo(jpu->handle, &outputInfo)) != JPG_RET_SUCCESS) {
            JLOG(ERR, "JPU_EncGetOutputInfo failed Error code is 0x%x \n", ret);
            return -1;
        }

        send_output_buffer_to_user(jpu, jpu->bs_stream, &outputInfo);

        if (outputInfo.encodeState == ENCODE_STATE_SLICE_DONE) {
            JLOG(TRACE, "Enc Slice: %d, rdPtr=0x%x, wrPtr=0x%x, slice height=%d\n", frameIdx,
                 outputInfo.streamRdPtr, outputInfo.streamWrPtr, outputInfo.encodedSliceYPos);
        } else {
            JLOG(TRACE, "Enc: %d, rdPtr=0x%x, wrPtr=0x%x cycles=%d\n", frameIdx,
                 outputInfo.streamRdPtr, outputInfo.streamWrPtr, outputInfo.frameCycle);
            frameIdx++;
            break;
        }
    }

    feed_row = 0;
    return 0;
}