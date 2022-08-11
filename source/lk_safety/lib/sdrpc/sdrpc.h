#ifndef __SDRPC_H__
#define __SDRPC_H__

#include "rpc_define.h"

typedef void* sdrpc_handle_t;
typedef unsigned int sdprc_payload_t;


unsigned int sdrpc_read_msg(sdrpc_handle_t sdrpc,rpc_commands_type cmd, sdprc_payload_t* payload);
void sdrpc_notify_msg(sdrpc_handle_t sdrpc,rpc_commands_type cmd,sdprc_payload_t* payload);


#endif