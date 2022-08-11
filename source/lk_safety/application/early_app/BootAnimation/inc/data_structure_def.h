#ifndef _BA_DATA_STRUCTURE_DEF_H_
#define _BA_DATA_STRUCTURE_DEF_H_


typedef struct vpu_req_t
{
    void* dummy;
}vpu_req_t;

typedef struct
{
    unsigned long bufY;
    unsigned long bufCb;
    unsigned long bufCr;
    unsigned long strideY;
    unsigned int width;
    unsigned int height;
    bool end;

}disp_req_t,vpu_rsp_t;

typedef struct
{
    void* dummy;
}vdsp_req_t;

typedef struct
{
    void* dummy;
}adsp_req_t;

typedef struct
{
    void* dummy;
}file_req_t;


#endif