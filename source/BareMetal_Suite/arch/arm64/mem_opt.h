/********************************************************
 *          Copyright(c) 2020   Semidrive               *
 ********************************************************/

#ifndef __MEM_OPT_H_
#define __MEM_OPT_H_

void memcpy_aligned(void *dst, const void *src, size_t sz);
void mem_wr_only_aligned(void *dst, size_t sz);
void mem_rd_only_aligned(void *dst, size_t sz);
void memset_aligned(void *dst, uint8_t v, size_t sz);
void memclr_aligned(void *dst, size_t sz);

#endif  /* __MEM_OPT_H_ */
