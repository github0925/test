#ifndef __RPMSG_CP_PORT_H__
#define __RPMSG_CP_PORT_H__
#include <dcf_common.h>
#include <dcf.h>
#include <rpmsg_rtos.h>


typedef struct rpmsg_cfg_t
{
    uint16_t ept_id;
    int rproc;
    const char* name;
}rpmsg_cfg_container_t;

typedef enum rpmsg_if_id_t
{
    RIF1,
    RIF2,
    RIF3,
    RIFN,
}rpmsg_if_id_t;

typedef struct rpmsg_if_map_t
{
    rpmsg_if_id_t if_id;
    rpmsg_cfg_container_t port;
}rpmsg_if_map_t;


typedef struct rpmsg_if_frame_t
{
    uint32_t bundle_id; //an abstract bundle id
    size_t len;   //payload length
    uint8_t payload[0]; // payload data
}rpmsg_if_frame_t;


#endif
