/*
* Copyright (c) 2019 Semidrive Semiconductor.
* All rights reserved.
*
*/

#ifndef _SD_SYSDEF_H
#define _SD_SYSDEF_H

inline static uint32_t reg_value(uint32_t val, uint32_t src, uint32_t shift, uint32_t mask)
{
  return (src & ~mask) | ((val << shift) & mask);
}

#endif
