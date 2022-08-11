//*****************************************************************************
//
// rsa.c - hal for rsa Module.
//
// Copyright (c) 2019-2029 SemiDrive Incorporated.  All rights reserved.
// Software License Agreement
//
//*****************************************************************************
#include <sd_rsa.h>
#include <trace.h>

#define LOCAL_TRACE 0 //close local trace 1->0

#define RSA_1024_LEN        128
#define RSA_2048_LEN        256
#define RSA_3072_LEN        384
#define RSA_4096_LEN        512

#define RSA_SUPPORT_PKA_ID_MASK 0x04 // 0~3 support pka 4~7 donot support

uint8_t __attribute__((aligned(CACHE_LINE))) rsa_p_1024[RSA_1024_LEN] =
    "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
    "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
    "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
    "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
    "\xee\xcf\xae\x81\xb1\xb9\xb3\xc9\x08\x81\x0b\x10\xa1\xb5\x60\x01"
    "\x99\xeb\x9f\x44\xae\xf4\xfd\xa4\x93\xb8\x1a\x9e\x3d\x84\xf6\x32"
    "\x12\x4e\xf0\x23\x6e\x5d\x1e\x3b\x7e\x28\xfa\xe7\xaa\x04\x0a\x2d"
    "\x5b\x25\x21\x76\x45\x9d\x1f\x39\x75\x41\xba\x2a\x58\xfb\x65\x99";
uint8_t __attribute__((aligned(CACHE_LINE))) rsa_q_1024[RSA_1024_LEN] =
    "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
    "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
    "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
    "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
    "\xc9\x7f\xb1\xf0\x27\xf4\x53\xf6\x34\x12\x33\xea\xaa\xd1\xd9\x35"
    "\x3f\x6c\x42\xd0\x88\x66\xb1\xd0\x5a\x0f\x20\x35\x02\x8b\x9d\x86"
    "\x98\x40\xb4\x16\x66\xb4\x2e\x92\xea\x0d\xa3\xb4\x32\x04\xb5\xcf"
    "\xce\x33\x52\x52\x4d\x04\x16\xa5\xa4\x41\xe7\x00\xaf\x46\x15\x03";

uint8_t __attribute__((aligned(CACHE_LINE))) rsa_p_2048[RSA_2048_LEN] =
    "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
    "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
    "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
    "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
    "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
    "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
    "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
    "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
    "\xec\xf5\xae\xcd\x1e\x55\x15\xff\xfa\xcb\xd7\x5a\x28\x16\xc6\xeb"
    "\xf4\x90\x18\xcd\xfb\x46\x38\xe1\x85\xd6\x6a\x73\x96\xb6\xf8\x09"
    "\x0f\x80\x18\xc7\xfd\x95\xcc\x34\xb8\x57\xdc\x17\xf0\xcc\x65\x16"
    "\xbb\x13\x46\xab\x4d\x58\x2c\xad\xad\x7b\x41\x03\x35\x23\x87\xb7"
    "\x03\x38\xd0\x84\x04\x7c\x9d\x95\x39\xb6\x49\x62\x04\xb3\xdd\x6e"
    "\xa4\x42\x49\x92\x07\xbe\xc0\x1f\x96\x42\x87\xff\x63\x36\xc3\x98"
    "\x46\x58\x33\x68\x46\xf5\x6e\x46\x86\x18\x81\xc1\x02\x33\xd2\x17"
    "\x6b\xf1\x5a\x5e\x96\xdd\xc7\x80\xbc\x86\x8a\xa7\x7d\x3c\xe7\x69";
uint8_t __attribute__((aligned(CACHE_LINE))) rsa_q_2048[RSA_2048_LEN] =
    "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
    "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
    "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
    "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
    "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
    "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
    "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
    "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
    "\xbc\x46\xc4\x64\xfc\x6a\xc4\xca\x78\x3b\x0e\xb0\x8a\x3c\x84\x1b"
    "\x77\x2f\x7e\x9b\x2f\x28\xba\xbd\x58\x8a\xe8\x85\xe1\xa0\xc6\x1e"
    "\x48\x58\xa0\xfb\x25\xac\x29\x99\x90\xf3\x5b\xe8\x51\x64\xc2\x59"
    "\xba\x11\x75\xcd\xd7\x19\x27\x07\x13\x51\x84\x99\x2b\x6c\x29\xb7"
    "\x46\xdd\x0d\x2c\xab\xe1\x42\x83\x5f\x7d\x14\x8c\xc1\x61\x52\x4b"
    "\x4a\x09\x94\x6d\x48\xb8\x28\x47\x3f\x1c\xe7\x6b\x6c\xb6\x88\x6c"
    "\x34\x5c\x03\xe0\x5f\x41\xd5\x1b\x5c\x3a\x90\xa3\xf2\x40\x73\xc7"
    "\xd7\x4a\x4f\xe2\x5d\x9c\xf2\x1c\x75\x96\x0f\x3f\xc3\x86\x31\x83";

uint8_t __attribute__((aligned(CACHE_LINE))) rsa_p_3072[RSA_3072_LEN] =
    "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
    "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
    "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
    "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
    "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
    "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
    "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
    "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
    "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
    "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
    "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
    "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
    "\xff\x9b\xc2\x96\xf0\x06\x1f\xf1\x68\x13\x06\x76\xfb\x48\x9d\xc3"
    "\x43\xb9\x43\x6c\xd6\x4e\x5d\x01\x31\x2e\xf3\xcc\x10\xc2\xb7\xdb"
    "\x86\xad\xcf\x88\x08\x40\xdf\xef\x87\x8c\xf8\xde\x57\x58\xe8\xd9"
    "\xfa\x9a\xc8\xd1\x6e\x7d\xeb\xde\x9a\xa5\xb3\xf5\x9a\x1b\x9a\xa6"
    "\x03\x1a\x2c\x89\x2a\x9f\x37\x5f\x7f\xb3\xf9\xb1\x01\xb3\x29\xd1"
    "\x55\x5d\x7d\xe3\xb5\x93\xd1\xb9\xb2\x20\x45\xf2\x87\x1a\xf6\xf5"
    "\x76\x9d\xc9\xfe\x14\x5a\x78\x10\xb5\x1f\x92\x2f\xd3\xef\x70\xec"
    "\x12\x5a\x88\xf5\xc0\x25\xb7\x60\xb3\x27\x78\x6c\x1e\xb4\xa0\x33"
    "\x39\x50\x29\x7e\xbd\x47\x08\x42\x5d\xbd\x09\x08\x5e\x16\x28\xfc"
    "\x40\xbf\xe0\x4d\x14\x5c\x7f\xee\xfd\xc0\x2c\x73\xdd\x2d\x86\xc8"
    "\x80\x32\x7e\xac\xd8\x73\xdf\x0c\x9b\xb4\xda\x8d\x30\xda\x77\xed"
    "\xbc\x09\x30\x00\x6d\xbf\x3d\x6e\xf9\x69\xde\x1b\xfd\x05\x7f\x03";
uint8_t __attribute__((aligned(CACHE_LINE))) rsa_q_3072[RSA_3072_LEN] =
    "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
    "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
    "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
    "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
    "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
    "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
    "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
    "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
    "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
    "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
    "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
    "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
    "\xfa\x77\x6b\xfb\xc5\x93\x0c\x78\x8b\xc0\x80\x66\xa9\xb5\xaa\x01"
    "\x2f\x6f\xdb\xe9\xe2\xe2\x7d\x70\xa4\x5e\x1c\xd5\x24\xf7\x26\x3e"
    "\x61\x78\x7c\x76\x51\xc6\x80\xca\x53\xc8\x06\x76\x24\x40\x12\x96"
    "\xa9\xf8\x24\x8d\x1c\x69\x4a\x5b\x51\xab\xc2\x58\x96\x21\xa4\xb8"
    "\x65\xf8\x87\x76\x7e\xff\x28\xc9\xfd\x86\xa8\xfc\x33\xf2\x11\x3b"
    "\x2a\x4a\x38\xc3\xee\xbc\x1f\x61\x46\x18\x9b\x57\xc5\xa9\xc5\xe5"
    "\xcf\xc7\x66\xca\x40\xae\x4a\x98\xae\x1d\xfa\x01\x91\xdc\xbb\x02"
    "\xe9\xef\xc0\x5d\x17\x0e\xae\x47\x20\x5a\xa4\x14\xb0\xf4\x56\x7d"
    "\xd8\x7b\x2f\x4f\x7f\x98\x36\xa3\x22\xa8\xea\x13\x5b\x1d\xf6\x61"
    "\xf8\x26\xf6\xed\xce\xa7\x16\x2e\x37\xcc\x70\xee\xd1\x06\x81\xa1"
    "\xf1\xf3\x0b\x30\x16\x27\x0d\xd8\xc3\x0e\xad\x74\xe6\x99\xf2\xd6"
    "\x8e\x28\x5e\x89\x2e\xcf\x55\x70\x06\x22\xea\x67\xf6\x51\x2f\x3f";

uint8_t __attribute__((aligned(CACHE_LINE))) rsa_p_4096[RSA_4096_LEN] =
    "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
    "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
    "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
    "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
    "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
    "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
    "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
    "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
    "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
    "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
    "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
    "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
    "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
    "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
    "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
    "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
    "\x86\x7f\x8a\xdb\x95\xd6\x81\xd5\x47\xbb\x4a\xc1\x67\x13\xa4\x9d"
    "\xad\xf5\xfd\x7b\x59\x8a\x38\x3d\x3e\x16\x8e\x93\x29\x87\x81\xc2"
    "\x43\x67\x5a\x9c\x35\xe9\x6d\x35\x37\xee\x2e\x37\x73\xb0\x8f\xac"
    "\x67\x50\x6f\x89\x12\x0e\x8b\x26\x79\xf2\xf7\xa8\x19\x77\x30\x9a"
    "\x9d\x39\x97\x40\xdf\xe2\xb1\x23\xf9\x8f\xa1\x2e\x7c\x51\xe7\x42"
    "\x1f\xe0\xdb\x5d\x3c\xa5\xae\xa7\x9e\x87\x04\xa9\xc0\xe4\x26\x00"
    "\xca\x50\xaf\x0c\x43\xaa\x51\x68\x6e\x0f\xe0\x39\xd0\xd3\xd2\xad"
    "\x46\x55\xf8\x52\x79\xa9\x73\x7e\x26\x3a\xc7\xdd\x06\xc0\xcc\xa3"
    "\x81\x3f\x53\x07\x1d\x5e\x43\xb9\xb9\xf0\x0c\x79\xa4\x4d\xcb\x7d"
    "\xde\x5c\xb2\xe6\x2c\xfa\x01\xc7\x4e\xa5\x59\x29\xda\x0c\x15\xce"
    "\x40\x96\x2f\xbb\x50\xa1\x37\x1f\x2f\x9c\x3f\xa3\x15\x57\xa2\x00"
    "\xf7\x22\x94\x71\xac\xa6\xcf\x47\xbe\xdb\x3a\x21\x45\x6d\x67\x32"
    "\x3f\x65\xcc\xd8\x9b\x09\x7f\xe2\xc1\xb0\x19\xcf\x91\x33\xfd\x0d"
    "\xa1\xba\xb4\xad\xce\x1d\xc0\xb2\x04\xd3\x7e\x7c\xb2\xa9\x6d\x55"
    "\xe2\x5d\xb3\xc5\xd3\xcc\xbe\x49\x4a\xd1\xf1\x62\xe7\x53\x32\x7f"
    "\x04\xd1\x47\x74\x17\xc2\xda\x68\x75\x15\x56\xd9\x0d\xa9\xae\x83";
uint8_t __attribute__((aligned(CACHE_LINE))) rsa_q_4096[RSA_4096_LEN] =
    "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
    "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
    "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
    "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
    "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
    "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
    "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
    "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
    "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
    "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
    "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
    "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
    "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
    "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
    "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
    "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
    "\xfe\x03\xd9\x29\x36\x17\x45\x69\xc4\xd0\x91\x0d\x80\x01\x48\xba"
    "\x38\x4a\x32\x31\x1f\x91\xed\x99\x16\x54\x0f\x2d\xa2\xa9\x2f\x6f"
    "\x15\xa5\xc4\x01\x65\xff\x6d\xa3\x5a\x03\x27\x9a\x86\xf6\x08\xa5"
    "\x2f\x1f\x15\xed\x6c\x10\x80\x00\x7e\xa8\x25\x31\xda\x56\xdd\xb4"
    "\xc3\x5d\x40\x0f\x00\x82\x9e\xe6\xee\x24\x59\xc4\x81\x1b\x04\x4f"
    "\xae\x56\x7f\x18\xcf\xb4\xca\xad\x55\xdb\x18\x88\xdf\x99\x31\x72"
    "\x15\x71\x07\x35\x42\xbf\xec\x3f\x5f\xa6\x2f\x4f\xf7\x8c\xb0\xde"
    "\xd2\xa7\xee\xdb\x90\x0b\x30\x6c\xd9\x7e\x73\x09\xc8\xd9\x98\xba"
    "\xc7\xeb\xed\x77\xca\x6c\xc7\xe1\x37\x2b\xe1\x7c\x43\x81\xf8\x1f"
    "\x37\xa3\xa1\xb2\xa1\xfc\x6f\x0c\xe2\xda\x35\x52\xaa\xf1\x48\xf9"
    "\xa6\x1a\x65\x8a\x77\x44\xf1\x3d\x27\xb7\x68\x6a\xa4\x2d\xc3\x75"
    "\xea\xd0\x9c\x3c\x81\x1b\xe3\xc9\xab\xbf\xbe\xf8\x22\x52\x69\x31"
    "\x09\x2f\xc7\x35\x83\xa4\x71\xe6\xb2\x04\xd3\x30\xd7\x8e\x30\x1d"
    "\x71\x6b\x6d\xaf\xad\x97\xa0\xc3\x4d\xf1\x2e\x56\xce\xf0\x9d\xea"
    "\x8e\x55\x5f\x0d\x85\x5f\xcb\x6d\x3d\xad\x5a\x15\xf2\x89\xc5\x6b"
    "\x9b\x21\x29\x05\xd2\x9d\x90\x3d\x09\xfe\x7b\xf8\x40\xd7\xc9\xa1";

//*****************************************************************************
//
//! hal_rsa_encrypt.
//!
//! @param handle input, opration handle for uplayer,creat by ce hal
//! @param pub_key input, rsa pubkey includ n,e
//! @param src input, msg for encrypt
//! @param src_size input, in msg len
//! @param dst output, buff for encrypted msg
//! @param dst_size input, buff len
//! @param padding input, padding type(not used)
//!
//! This function is for rsa encrypt
//!
//! @return crypto_status_t CRYPTO_SUCCESS
//
//*****************************************************************************

crypto_status_t hal_rsa_encrypt(void* handle, const rsa_pubkey_t* pub_key,
                            const uint8_t* src, uint32_t src_size, uint8_t* dst, uint32_t dst_size,
                            int padding)
{
    uint32_t vce_id;
    buff_addr_type_t addr_type = HAL_EXT_MEM;
    crypto_status_t ret = CRYPTO_SUCCESS;
    crypto_instance_t* l_cryptoinstance = NULL;

    if (handle == NULL) {
        ret = CRYPTO_ERROR;
    }
    else {
        l_cryptoinstance = (crypto_instance_t*)handle;

        /*do ce init and enable here*/

        vce_id = l_cryptoinstance->vce_id;

        if ((vce_id & RSA_SUPPORT_PKA_ID_MASK) == 0) {
            if (l_cryptoinstance->rsa_method->encrypt) {
                ret = l_cryptoinstance->rsa_method->encrypt(vce_id, SD_RSA_PADDING_NONE
                        , block_t_convert(src, src_size, addr_type)
                        , block_t_convert(pub_key->n, pub_key->n_len, addr_type)
                        , block_t_convert(pub_key->e, pub_key->e_len, addr_type)
                        , block_t_convert(dst, dst_size, HAL_EXT_MEM)
                        , SD_ALG_SHA1);
            }
        }
        else {
            ret = CRYPTO_INVALID_CE;
        }
    }

    return ret;
}

//*****************************************************************************
//
//! hal_rsa_decrypt.
//!
//! @param handle input, opration handle for uplayer,creat by ce hal
//! @param pri_key input, rsa pri key includ d info
//! @param out_len input, decrypt out len
//! @param out output, buff for decrypt
//! @param max_out input, buff len
//! @param in input, msg for decrypt
//! @param in_len input, msg len
//! @param padding input, padding type(not used)
//!
//! This function is for rsa decrypt
//!
//! @return crypto_status_t CRYPTO_SUCCESS
//
//*****************************************************************************
crypto_status_t hal_rsa_decrypt(void* handle, const rsa_keypair_t* pri_key, size_t out_len, uint8_t* out, size_t max_out,
                            const uint8_t* in, size_t in_len, int padding)
{
    uint32_t vce_id;
    buff_addr_type_t addr_type = HAL_EXT_MEM;
    crypto_status_t ret = CRYPTO_SUCCESS;
    crypto_instance_t* l_cryptoinstance = NULL;
    uint32_t size = 128;

    if (handle == NULL) {
        ret = CRYPTO_ERROR;
    }
    else {
        l_cryptoinstance = (crypto_instance_t*)handle;

        /*do ce init and enable here*/

        vce_id = l_cryptoinstance->vce_id;

        LTRACEF("out_len = %d\n", (uint32_t)out_len);

        //out_len_temp = pri_key->n_len;

        if (l_cryptoinstance->rsa_method->decrypt) {
            ret = l_cryptoinstance->rsa_method->decrypt(vce_id,
                    ESEC_RSA_PADDING_NONE,
                    block_t_convert(in, in_len, addr_type),
                    block_t_convert(pri_key->n, pri_key->n_len, addr_type),
                    block_t_convert(pri_key->d, pri_key->d_len, addr_type),
                    block_t_convert(out, pri_key->n_len, addr_type),
                    0,
                    &size,
                    SD_ALG_SHA1);
        }
    }

    return ret;
}

//*****************************************************************************
//
//! hal_rsa_sign.
//!
//! @param handle input, opration handle for uplayer,creat by ce hal
//! @param hash_nid input, hash type (not used )
//! @param in input, msg for sign
//! @param in_len input, msg len
//! @param out output
//! @param out_len output len
//! @param pri_key input, priv key
//! @param padding input, padding type
//! @param salt_len input, salt length
//!
//! This function is for rsa signature
//!
//! @return crypto_status_t
//
//*****************************************************************************
crypto_status_t hal_rsa_sign(void* handle, int hash_nid, const uint8_t* in, unsigned in_len, uint8_t* out,
                         unsigned out_len, const rsa_keypair_t* pri_key, int padding, uint32_t salt_len)
{
    uint32_t vce_id;
    buff_addr_type_t addr_type = HAL_EXT_MEM;
    crypto_status_t ret = CRYPTO_SUCCESS;
    crypto_instance_t* l_cryptoinstance = NULL;

    if (handle == NULL) {
        ret = CRYPTO_ERROR;
    }
    else {
        l_cryptoinstance = (crypto_instance_t*)handle;

        /*do ce init and enable here*/

        vce_id = l_cryptoinstance->vce_id;

        //out_len_temp = pri_key->n_len;

        if (l_cryptoinstance->rsa_method->sign) {
            ret = l_cryptoinstance->rsa_method->sign(vce_id, SD_ALG_SHA256, padding,
                    block_t_convert(in, in_len, addr_type),
                    //block_t_convert((void *)(sram_base + RSA_SRAM_DST_OFFSET), keysize, HAL_SRAM_PUB), //use sram offset based on current ce
                    block_t_convert(out, pri_key->n_len, HAL_EXT_MEM),
                    block_t_convert(pri_key->n, pri_key->n_len, addr_type),
                    block_t_convert(pri_key->d, pri_key->d_len, addr_type),
                    salt_len);
        }
    }

    return ret;
}

//*****************************************************************************
//
//! hal_rsa_verify.
//!
//! @param handle input, opration handle for uplayer,creat by ce hal
//! @param hash_nid input, hash type (not used )
//! @param msg input, msg for verify
//! @param msg_len input, msg len
//! @param sig input, sig msg for compare
//! @param sig_len input, sig msg len
//! @param pub_key input, pub key
//! @param padding input, padding type
//! @param salt_len input, salt length
//!
//! This function is for rsa Verification
//!
//! @return crypto_status_t CRYPTO_SUCCESS (value is 0) verify pass.
//
//*****************************************************************************
crypto_status_t hal_rsa_verify(void* handle, int hash_nid, const uint8_t* msg, size_t msg_len,
                           const uint8_t* sig, size_t sig_len, const rsa_pubkey_t* pub_key,
                           int padding, uint32_t salt_len)
{
    uint32_t vce_id;
    ce_addr_type_t addr_type = HAL_EXT_MEM;
    crypto_status_t ret = CRYPTO_SUCCESS;
    crypto_instance_t* l_cryptoinstance = NULL;

    if (handle == NULL) {
        ret = CRYPTO_ERROR;
    }
    else {
        l_cryptoinstance = (crypto_instance_t*)handle;

        /*do ce init and enable here*/

        vce_id = l_cryptoinstance->vce_id;

        if (l_cryptoinstance->rsa_method->verify) {
            ret = l_cryptoinstance->rsa_method->verify(vce_id, hash_nid, padding,
                    block_t_convert(msg, msg_len, addr_type),
                    block_t_convert(pub_key->n, pub_key->n_len, addr_type),
                    block_t_convert(pub_key->e, pub_key->e_len, addr_type),
                    block_t_convert(sig, sig_len, addr_type),
                    salt_len);
        }
    }

    return ret;
}

//*****************************************************************************
//
//! hal_rsa_keygen.
//!
//! @param handle input, opration handle for uplayer,creat by ce hal
//! @param key_e input, pub key
//! @param key_e_len input
//! @param key_len input,support RSA_1024_LEN 128 RSA_2048_LEN 256 RSA_3072_LEN 384 RSA_4096_LEN 512
//! @param key_n output, pub key
//! @param key_d output, priv key
//!
//! This function is for rsa key generation
//!
//! @return crypto_status_t CRYPTO_SUCCESS (value is 0) verify pass.
//
//*****************************************************************************
crypto_status_t hal_rsa_keygen(void* handle, uint8_t* key_e, uint32_t key_e_len, int32_t key_len, uint8_t* key_n, uint8_t* key_d)
{
    uint32_t vce_id;
    buff_addr_type_t addr_type = HAL_EXT_MEM;
    crypto_status_t ret = CRYPTO_SUCCESS;
    crypto_instance_t* l_cryptoinstance = NULL;

    uint8_t* p;
    uint8_t* q;

    if (handle == NULL) {
        ret = CRYPTO_ERROR;
    }
    else {
        l_cryptoinstance = (crypto_instance_t*)handle;

        /*do ce init and enable here*/
        /*use test p q for test, should change to get_p&get_q*/
        switch (key_len) {
            case RSA_1024_LEN:
                p = (uint8_t*)&rsa_p_1024;
                q = (uint8_t*)&rsa_q_1024;
                break;

            case RSA_2048_LEN:
                p = (uint8_t*)&rsa_p_2048;
                q = (uint8_t*)&rsa_q_2048;
                break;

            case RSA_3072_LEN:
                p = (uint8_t*)&rsa_p_3072;
                q = (uint8_t*)&rsa_q_3072;
                break;

            case RSA_4096_LEN:
                p = (uint8_t*)&rsa_p_4096;
                q = (uint8_t*)&rsa_q_4096;
                break;

            default:
                p = (uint8_t*)&rsa_p_1024;
                q = (uint8_t*)&rsa_q_1024;
                break;
        }

        vce_id = l_cryptoinstance->vce_id;

        if (l_cryptoinstance->rsa_method->sign) {
            ret = l_cryptoinstance->rsa_method->keygen(vce_id, block_t_convert(p, key_len, addr_type),
                    block_t_convert(q, key_len, addr_type),
                    block_t_convert(key_e, key_e_len, addr_type),
                    block_t_convert(key_n, key_len, HAL_EXT_MEM),
                    block_t_convert(key_d, key_len, HAL_EXT_MEM),
                    key_len,
                    0);
        }
    }

    return ret;
}

//*****************************************************************************
//
//! hal_rsa_keygen_ex.
//!
//! @param handle input, opration handle for uplayer,creat by ce hal
//! @param key_e input, pub key
//! @param key_len input,support RSA_1024_LEN 128 RSA_2048_LEN 256 RSA_3072_LEN 384 RSA_4096_LEN 512
//! @param key output, key combination, modulus+lambda+private key+exponent
//!
//! This function is for rsa key generation
//!
//! @return crypto_status_t CRYPTO_SUCCESS (value is 0) verify pass.
//
//*****************************************************************************
crypto_status_t hal_rsa_keygen_ex(void* handle, int32_t key_len, uint8_t* key_e, uint8_t* key)
{
    uint32_t vce_id;
    crypto_status_t ret = CRYPTO_SUCCESS;
    crypto_instance_t* l_cryptoinstance = NULL;

    if (handle == NULL) {
        ret = CRYPTO_ERROR;
    }
    else {
        l_cryptoinstance = (crypto_instance_t*)handle;

        vce_id = l_cryptoinstance->vce_id;

        if (l_cryptoinstance->rsa_method->sign) {
            ret = l_cryptoinstance->rsa_method->keygen_ex(vce_id, key_len,
                    block_t_convert(key_e, key_len, HAL_EXT_MEM),
                    block_t_convert(key, key_len * 4, HAL_EXT_MEM),
                    true, true);
        }
    }

    return ret;
}