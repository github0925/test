#ifndef CRC32_H_
#define CRC32_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>

#define CRC_BUFFER_SIZE  8192

unsigned long Crc32_ComputeBuf(unsigned long inCrc32, const void *buf,
                               size_t bufLen);

#define crc32 Crc32_ComputeBuf

#ifdef __cplusplus
extern "C" {
#endif

#endif
