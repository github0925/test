#ifndef __SPARSE_PARSER__H_
#define __SPARSE_PARSER__H_

#include "update.h"

uint64_t get_sparse_img_size(int type,unsigned long long in);
int is_sparse_img(int type,unsigned long long in);
int flash_sparse_img(int type,unsigned long long in,unsigned long long out,unsigned long long size,char *full_ptname,storage_device_t *storage);
#endif