#ifndef __HEAP_API_H
#define __HEAP_API_H

#include <stddef.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Just keep compiler quiet.*/
void heap_init(void);

/**************** Heap management on Internal Heap ****************/

/* Standard malloc function
 */
void* malloc(size_t size);

/* Standard memalign function
 * Para
 * boundary - aligned boundary, must be pow of 2
 * size - wanted bytes
 */
void* memalign(size_t boundary, size_t size);

/* Standard clear alloc function
 */
void* calloc(size_t count, size_t size);

/* Standard free function
 */
void free(void* p);


/* get current heap free size
 */
size_t heap_remaining(void);

#if (SDRV_KERNEL_MONITOR == 1)

void heap_info(void);

#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif

