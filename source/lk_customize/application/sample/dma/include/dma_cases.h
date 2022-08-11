/*
* dma_cases.h
*
* Copyright (c) 2019 Semidrive Semiconductor.
* All rights reserved.
*
* dma cases function head file
*
* Revision History:
* -----------------
* 0.1, 10/19/2019 yishao init version
*/
#ifndef __DMA_CASES_h
#define __DMA_CASES_h

#include <stdio.h>
#include <stdlib.h>
typedef enum
{
    SINGLE_CASE = 0,
    MULTI_CASE = 1,
    DRIVER_CASE = 2,
    TEST_TYPE_NUM,
} TEST_TYPE;

typedef struct
{
    char name[50];
    TEST_TYPE tst_type;
    u32 param_index;
    u32 flag; //0: just current channel. 1 : for all channel.
} dma_case_t;

const char *case_type_str[] = {
    "Single Case",
    "Multi  Case",
    "Driver Case",
};

dma_case_t dma_sample_cases[] = {
    {"[1 DMA memcpy]", DRIVER_CASE, 0, 0},
    {"[2 DMA memset]", DRIVER_CASE, 1, 0},
#ifdef ENABLE_SD_I2S
    {"[3 DMA i2s loopback]", DRIVER_CASE, 2, 0},
#endif
};
#define SAMPLE_CASES_NUM (sizeof(dma_sample_cases) / sizeof(dma_sample_cases[0]))
bool init_dma_cases(void);
bool run_case(int caseid);
#endif