/*
 * This file was transplanted with slight modifications from Linux sources
 * (fs/cifs/md5.h) into U-Boot by Bartlomiej Sieka <tur@semihalf.com>.
 */

#ifndef _MD5_H
#define _MD5_H

#include "compiler.h"

#define MD5_LEN 16
typedef unsigned int __u32;

struct MD5Context {
	__u32 buf[4];
	__u32 bits[2];
	union {
		unsigned char in[64];
		__u32 in32[16];
	};
};

/*
 * Calculate and store in 'output' the MD5 digest of 'len' bytes at
 * 'input'. 'output' must have enough space to hold 16 bytes.
 */
void md5 (const unsigned char *input, const int len, unsigned char output[16]);

/*
 * Calculate and store in 'output' the MD5 digest of 'len' bytes at 'input'.
 * 'output' must have enough space to hold 16 bytes. If 'chunk' Trigger the
 * watchdog every 'chunk_sz' bytes of input processed.
 */
void md5_wd (const unsigned char *input, const int len, unsigned char output[16],
		const unsigned int chunk_sz);

#endif /* _MD5_H */
