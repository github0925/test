/********************************************************
 *          Copyright(c) 2018   Semidrive               *
 ********************************************************/
#ifndef __MAILBOX_H__
#define __MAILBOX_H__

#include <common_hdr.h>

U32 mb_cfg_mid(module_e m, U8 cpu_id, U8 mid, bool lock);
U32 mb_tx_smsg(module_e m, U32 cpu_msk, U8 msg_id, U8 *msg, U32 len);
BOOL mb_msg_received(module_e m, U8 msg_id, U32 cpu_id);
U32 mb_rx_smsg(module_e m, U8 cpu_id, U8 msg_id, U8 *msg, U32 *len);
bool mb_is_lock(module_e m, U8 cpu_id);
bool mb_is_rx_msg_in(module_e m, U8 cpu_from, U8 msg_id);
uint32_t semphore_get_owner(module_e m, uint32_t id);
bool semphore_try_lock(module_e m, uint32_t id);
bool semphore_unlock(module_e m, uint32_t id);

#endif  /* __MAILBOX_H__ */
