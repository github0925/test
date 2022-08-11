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
*/

#include "v4l2.h"
#include <stdlib.h>

static u32 abs(u32 a)
{
    return (a > 0) ? a : -a;
}

const void *
__v4l2_find_nearest_size(const void *array, size_t array_size,
                         size_t entry_size, size_t width_offset,
                         size_t height_offset, s32 width, s32 height)
{
    u32 error, min_error = UINT32_MAX;
    const void *best = NULL;
    unsigned int i;

    if (!array)
        return NULL;

    for (i = 0; i < array_size; i++, array = (u8 *)array + entry_size) {
        const u32 *entry_width = (u32 *)((u8 *)array + width_offset);
        const u32 *entry_height = (u32 *)((u8 *)array + height_offset);

        error = abs(*entry_width - width) + abs(*entry_height - height);

        if (error > min_error)
            continue;

        min_error = error;
        best = array;

        if (!error)
            break;
    }

    return best;
}


