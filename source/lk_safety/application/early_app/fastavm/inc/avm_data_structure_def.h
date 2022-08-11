#ifndef _BA_DATA_STRUCTURE_DEF_H_
#define _BA_DATA_STRUCTURE_DEF_H_

#include "fastavm_api.h"

typedef struct
{
    unsigned long bufY;
    unsigned long bufCb;
    unsigned long bufCr;
    unsigned long strideY;
    unsigned int width;
    unsigned int height;
    bool end;

}disp_req_t, vdsp_rsp_t;

typedef struct
{
    void* dummy;
}vdsp_req_t;

struct vdsp_message {
    paddr_t output_addr;    /* addr for store processed image   */
    paddr_t input_addr;     /* base addr for the CSI img buffer */
};

#endif
