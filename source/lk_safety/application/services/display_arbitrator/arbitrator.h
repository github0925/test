#ifndef DISP_ARBITRATOR_H
#define DISP_ARBITRATOR_H
#include <sdm_display.h>
#include <stdio.h>


typedef enum disp_preemption_level_t
{
    DISP_PRMPT_INVALID = -1,
    DISP_PRMPT_MUST = 0,
    DISP_PRMPT_AFAP,
    DISP_PRMPT_NEVER,
    DISP_PRMPT_LEVEL_MAX,

}disp_preempt_level_t;

typedef uint32_t disp_token_t;

#define ARB_DBG(...) printf(__VA_ARGS__)


#endif