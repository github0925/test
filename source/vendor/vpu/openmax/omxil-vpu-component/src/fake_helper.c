#include "main_helper.h"

#define UNUSED(x) (void)(x)

uint8_t* __attribute__((weak)) GetYUVFromFrameBuffer(
    DecHandle       decHandle,
    FrameBuffer*    fb,
    VpuRect         rcFrame,
    uint32_t*       retWidth,
    uint32_t*       retHeight,
    uint32_t*       retBpp,
    size_t*         retSize
    )
{
    UNUSED(decHandle);
    UNUSED(fb);
    UNUSED(rcFrame);
    UNUSED(retWidth);
    UNUSED(retHeight);
    UNUSED(retBpp);
    UNUSED(retSize);

    return 0;
}
