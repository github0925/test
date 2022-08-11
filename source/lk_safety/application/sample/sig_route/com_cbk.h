#ifndef COM_CBK_H__
#define COM_CBK_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

extern void com_rx_frame(uint16_t port_id, uint16_t bus_id,
                         uint32_t prot_id, uint8_t *data);

#ifdef __cplusplus
}
#endif

#endif
