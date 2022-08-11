/********************************************************
 *          Copyright(c) 2019   Semidrive               *
 *******************************************************/

#include <common_hdr.h>

extern void mini_memclr_s(void *dst, size_t size);
extern void *mini_memcpy_s(void *dst, const void *src, size_t size);

void __aeabi_memcpy4(void *d, const void *s, size_t n)
__attribute__((alias("__aeabi_memcpy")));
void __eabi_memcpy8(void *d, const void *s, size_t n)
__attribute__((alias("__aeabi_memcpy")));
void __aeabi_memcpy(void *d, const void *s, size_t n)
{
    mini_memcpy_s(d, s, n);
}

void __aeabi_memclr4(void *d, size_t n)
__attribute__((alias("__aeabi_memclr")));
void __aeabi_memclr8(void *d, size_t n)
__attribute__((alias("__aeabi_memclr")));
void __aeabi_memclr(void *d, size_t n)
{
    mini_memclr_s(d, n);
}
