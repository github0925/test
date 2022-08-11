#ifndef _ARCH_H_
#define _ARCH_H_

#include <stdint.h>
#include <stddef.h>

#define CACHE_LINE      64

#if !defined(ASSEMBLY)
#define isb()
#define dsb()
#define dmb()
#define nop()
#endif


#define el3_fiq_enable()  do { \
                                uint32_t scr = 0U; \
                                __asm__ __volatile__("mrs %0, scr_el3 \n" \
                                                     "orr %0, %0, #4  \n" \
                                                     "msr scr_el3, %0 \n" \
                                                     "isb             \n" \
                                                     "msr daifclr, #1 \n" \
                                                     : "+r"(scr)); \
                            } while (0)


#define local_irq_enable()  __asm__ volatile("msr	daifclr, #2" : : : "memory")
#define local_irq_disable() __asm__ volatile("msr	daifset, #2" : : : "memory")
#define local_fiq_enable()	__asm__ volatile("msr	daifclr, #1" : : : "memory")
#define local_fiq_disable()	__asm__ volatile("msr	daifset, #1" : : : "memory")

static inline unsigned long arch_local_irq_save(void)
{
	unsigned long flags;
	__asm__ volatile(
		"mrs	%0, daif		// arch_local_irq_save\n"
		"msr	daifset, #2"
		: "=r" (flags)
		:
		: "memory");
	return flags;
}

void arch_clean_invalidate_cache_range(const void *start, size_t len);
void arch_invalidate_cache_range(const void *start, size_t len);

#include "arch/arch_ops.h"
#include "armv8_mmu.h"
#include "mem_opt.h"

static inline void flush_dcache_range(uintptr_t addr, size_t size)
{
    arch_clean_invalidate_cache_range((const void *)addr, size);
}

#endif  /* _ARCH_H_ */
