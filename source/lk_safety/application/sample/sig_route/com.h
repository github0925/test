#ifndef COM_H__
#define COM_H__

#include <stdint.h>
#include <spinlock.h>
#include <list.h>

#ifdef __cplusplus
extern "C" {
#endif

struct frame_desc;

enum port {
    PORT_CAN,
    PORT_LIN,

    PORT_MAX
};

typedef enum {
    BYTE_ORDER_INTEL,
    BYTE_ORDER_MOTOROLA_LSB,
    BYTE_ORDER_MOTOROLA_MSB
} signal_order_t;

typedef struct signal_desc {
    struct frame_desc *frame;
    signal_order_t    order;
    uint16_t          start_bit;
    uint16_t          width;
} signal_desc_t;

typedef struct signal_rule {
    signal_desc_t src_sig;
    signal_desc_t tgt_sig;
} signal_rule_t;

typedef struct frame_desc {
    struct list_node node;
    spin_lock_t      lock;
    uint16_t         port_id;
    uint16_t         bus_id;
    uint32_t         prot_id;
    uint8_t          data[64];
    uint8_t          len;
    uint16_t         period;
    /**
     * Signal routing rule definition.
     */
    uint16_t         sig_rule_nr;
    signal_rule_t    sig_rule[];
} frame_desc_t;

extern void com_init(frame_desc_t *rx_frame[], frame_desc_t *tx_frame[]);
extern void com_tx_frame(void);
extern uint32_t com_read_signal(signal_desc_t *signal);
extern void com_write_signal(signal_desc_t *signal, uint32_t value);

#ifdef __cplusplus
}
#endif

#endif
