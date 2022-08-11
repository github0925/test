#ifndef __WORKER_H__
#define __WORKER_H__
#include <stdint.h>

/* worker: an one-shot function to be called in async
 * way. All worker called by FIFO rules which depends on
 * call sequence.
 */

/* worker function prototype.
 * pargs: pointer-type argument.
 * uargs: hex-type argument.
 */
typedef void(*worker_foo_t)(void* pargs, uint32_t uargs);

/* Start worker service. */
void start_worker_service(void);

/* Call a worker function in async way. This function could be
 * called in worker function.
 */
void call_worker(worker_foo_t foo, void* pargs, uint32_t uargs);


#endif