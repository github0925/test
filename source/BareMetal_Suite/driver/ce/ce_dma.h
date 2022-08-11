/********************************************************
 *          Copyright(c); 2019  Semidrive               *
 ********************************************************/
#ifndef __CE_DMA_H__
#define __CE_DMA_H__

#include <common_hdr.h>

U32 ce_pkemem_read(U32 base, U32 from_off, U32 to, U32 len);
U32 ce_pkemem_write(U32 base, U32 from, U32 to_off, U32 len);
U32 ce_dma_copy(U32 base, U32 from, U32 to, U32 len, ce_dma_mem_type_e s_attr, ce_dma_mem_type_e d_attr);

#endif  /* __CE_DMA_H__ */
