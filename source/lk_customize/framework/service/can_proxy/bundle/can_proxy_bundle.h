#ifndef __CAN_PROXY_BUNDLE_H__
#define __CAN_PROXY_BUNDLE_H__

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/* Attach a packet to specific bundle.
 * multiple packets could be attached with same bundle id, which
 * will be loaded in LIFO rules.
 */
bool cp_bundle_attach(uint32_t bundle_id,void* data,size_t size);

/* Detach a packet from specific bundle.
 * Packet to be detached would be fetch out in LIFO rules.
 */
size_t cp_bundle_detach(uint32_t bundle_id, void** pdata);


#endif