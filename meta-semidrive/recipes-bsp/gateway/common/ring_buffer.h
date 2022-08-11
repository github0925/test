/*
 * ring_buffer.h
 *
 * Copyright (c) 2020 SemiDrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */
#ifndef _RING_BUFFER_H_
#define _RING_BUFFER_H_

#include <stdint.h>
#include <stdlib.h>
#include <assert.h>

#define ring_mask(ring) ((ring)->size - 1)
#define min(a, b)	((a) <= (b) ? (a) : (b))

#if SMP
#define SMP_RMB()	__asm__ __volatile__("dmb ishld" : : : "memory")
#define SMP_WMB()	__asm__ __volatile__("dmb ishst" : : : "memory")
#else
#define SMP_RMB()	__asm__ __volatile__("" : : : "memory")
#define SMP_WMB()	__asm__ __volatile__("" : : : "memory")
#endif

/*
 * Lock-free ring buffer.
 * in & out are both written only by one thread.
 */
typedef struct _ring {
	/* Writing ring index. */
	uint32_t in;
	/* Reading ring index. */
	uint32_t out;
	/* size must be power of 2. */
	uint32_t size;
	uint8_t buf[0];
} ring_t;

static inline ring_t *ring_alloc(uint32_t size)
{
	assert(!(size & (size - 1)));

	ring_t *ring = (ring_t *)malloc(sizeof(ring_t) + size);
	if (ring) {
		ring->in = ring->out = 0;
		ring->size = size;
	}
	return ring;
}

static inline void ring_free(ring_t *ring)
{
	assert(ring);

	free(ring);
}

static inline uint32_t ring_used(ring_t *ring)
{
	/* Make newest "in" & "out" observed by the core. */
	SMP_RMB();

	return ring->in - ring->out;
}

static inline uint32_t ring_unused(ring_t *ring)
{
	return ring->size - ring_used(ring);
}

static inline uint32_t ring_in(ring_t *ring, uint8_t *data, uint32_t len)
{
	assert(ring && data && len);

	if (len <= ring_unused(ring)) {
		uint32_t in_offset = ring->in & ring_mask(ring);
		uint32_t out_offset = ring->out & ring_mask(ring);
		uint32_t l;
		if (in_offset >= out_offset) {
			l = ring->size - in_offset;
		}
		else {
			l = out_offset - in_offset;
		}
		uint32_t first_cpy_l = min(len, l);
		memcpy(&ring->buf[in_offset], data, first_cpy_l);
		if (len > first_cpy_l) {
			memcpy(ring->buf, data + first_cpy_l, len - first_cpy_l);
		}
		/* Make updating ring buffer observed before "in" updated. */
		SMP_WMB();
		ring->in += len;
		/* Drain store buffer to make updated "in" observed. */
		SMP_WMB();
		return len;
	}
	else {
		return 0;
	}
}

static inline uint32_t ring_out(ring_t *ring, uint8_t *buf, uint32_t len)
{
	assert(ring && buf && len);

	uint32_t used = ring_used(ring);
	if (used) {
		uint32_t actual_rd_len = min(len, used);
		uint32_t in_offset = ring->in & ring_mask(ring);
		uint32_t out_offset = ring->out & ring_mask(ring);
		uint32_t l;
		if (in_offset <= out_offset) {
			l = ring->size - out_offset;
		}
		else {
			l = in_offset - out_offset;
		}
		uint32_t first_cpy_l = min(actual_rd_len, l);
		memcpy(buf, &ring->buf[out_offset], first_cpy_l);
		if (actual_rd_len > first_cpy_l) {
			memcpy(buf + first_cpy_l, ring->buf, actual_rd_len - first_cpy_l);
		}
		/* Make wrinting read buffer observed before out updated. */
		SMP_WMB();
		ring->out += actual_rd_len;
		/* Drain store buffer to make updated "out" observed. */
		SMP_WMB();
		return actual_rd_len;
	}
	else {
		return 0;
	}
}

#endif