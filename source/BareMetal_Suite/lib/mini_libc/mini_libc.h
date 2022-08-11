/********************************************************
 *          Copyright(c) 2018   Semidrive               *
 ********************************************************/

/**
 * @file    mini_libc.h
 * @brief   header file of mini_libc
 */

#ifndef __MINI_LIBC_H__
#define __MINI_LIBC_H__
#include <types_def.h>

int mini_printf(const char *fmt, ...);
int mini_sprintf(char *ss, const char *fmt, ...);
int32_t log_print(U32 *log, char *fmt_str, uintptr_t adjust);

void *mini_memcpy_s(void *dst, const void *src, size_t size);
void mini_memclr_s(void *dst, size_t size);
S32 mini_memcmp_s(const void *mem1, const void *mem2, size_t size);
void *mini_memset_s(void *dst, int val, size_t size);
void *mini_memmove_s(void *dst, const void *src, size_t size);
size_t mini_strlen(const char *str);
char *mini_strcpy(char *to, const char *from);
void *mini_mem_rvscpy_s(void *dst, const void *src, size_t sz);
void *mini_mem_rvs_s(void *mem, size_t sz);
int mini_strncmp_s(const char *s1, const char *s2, size_t n);
int mini_strcmp_s(const char *s1, const char *s2);
unsigned long int mini_strtoul(const char *str, char **endptr, int base);
unsigned long long mini_strtoull(const char *str, char **endptr, int base);

uint32_t rand32(void);

#if defined(NO_STDLIB)
void *memset(void *dst, int val, size_t sz);
void *memcpy (void *dst, const void *src, size_t sz);
#define memclr  mini_memclr_s
#define memcmp  mini_memcmp_s
#define memmove mini_memmove_s
#define strlen  mini_strlen
#define strcpy  mini_strcpy
#define strncmp mini_strncmp_s
#define strcmp mini_strcmp_s
#define strtoul mini_strtoul
#define strtoull mini_strtoull
#define sprintf mini_sprintf
#else
#include <string.h>
#include <stdlib.h>
#endif

#endif /*__MINI_LIBC_H__ */
