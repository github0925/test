#include <sdm_display.h>
#include "vstreamer.h"
#include "avm_data_structure_def.h"

void vdsp2disp_proc(void* prod_rsp, void* cons_req)
{
    vdsp_rsp_t* vdsp_rsp = prod_rsp;
    disp_req_t* disp_req = cons_req;

    disp_req->bufY = vdsp_rsp->bufY;
    disp_req->bufCb = vdsp_rsp->bufCb;
    disp_req->bufCr = vdsp_rsp->bufCr;
    disp_req->strideY = vdsp_rsp->strideY;
    disp_req->height = vdsp_rsp->height;
    disp_req->width = vdsp_rsp->width;
    disp_req->end = vdsp_rsp->end;
}
