#ifndef _RES_LOADER_H_
#define _RES_LOADER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define RL_DBG_ENABLE    0

#if (RL_DBG_ENABLE == 1)
#include <stdio.h>
#define RLDBG(...) printf(__VA_ARGS__)
#else
#define RLDBG(...) do{} while(0)
#endif


int res_size(const char* path);
int res_load(const char* path, void* buf, uint32_t size, uint32_t offset);

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif
