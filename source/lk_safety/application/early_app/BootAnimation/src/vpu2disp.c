#include <vpu_hal.h>
#include <sdm_display.h>
#include "data_structure_def.h"

void vpu2disp_proc(void* prod_rsp, void* cons_req)
{
    vpu_rsp_t* vpu_rsp = prod_rsp;
    disp_req_t* disp_req = cons_req;

    disp_req->bufY = vpu_rsp->bufY;
    disp_req->bufCb = vpu_rsp->bufCb;
    disp_req->bufCr = vpu_rsp->bufCr;
    disp_req->strideY = vpu_rsp->strideY;
    disp_req->height = vpu_rsp->height;
    disp_req->width = vpu_rsp->width;
    disp_req->end = vpu_rsp->end;
}