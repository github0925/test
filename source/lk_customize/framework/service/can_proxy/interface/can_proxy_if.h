#ifndef __CAN_PROXY_IF_H__
#define __CAN_PROXY_IF_H__

#include <stdint.h>

#define MAX_INTERFACE_PER_BUS   4
#define MAX_IF_DATA_SIZE    80


struct cp_if_t;
typedef int (*cp_if_init_t)(struct cp_if_t* cpif);
typedef int (*cp_if_tx_t)(struct cp_if_t* cpif,void* from, int len);
typedef int (*cp_if_rx_cb_t)(struct cp_if_t* cpif, void* data, int len);

typedef enum
{
    cpifBusCan,
    cpifBusSPI,
    cpifBusRPMSG,
    cpifBusMax,
}cp_if_bus_t;

typedef enum
{
    cpifStateUnkown,
    cpifStateInited,
    cpifStateRunning,
    cpifStateStopped,
    cpifStateMax,
}cp_if_state_t;

typedef struct cp_if_frame_t
{
    cp_if_bus_t bus; // bus type
    uint32_t if_id; //interface id
    size_t len; //interface data length
    uint8_t data[0]; //interface data
}cp_if_frame_t;


typedef struct cp_if_t
{

    cp_if_bus_t     bus;
    uint32_t        if_id;
    void*           container;
    cp_if_state_t   if_state;
    size_t          ctn_size;
    cp_if_init_t    init;
    cp_if_tx_t      tx;
    cp_if_rx_cb_t   rxcb;

}cp_if_t;

void cp_interface_setting(
    cp_if_bus_t bus,
    uint32_t if_id,
    cp_if_init_t init_proc,
    cp_if_tx_t tx_proc,
    cp_if_rx_cb_t rxcb_proc,
    void* container,
    size_t ctn_size);

void cp_interface_duplicate(cp_if_bus_t bus,uint32_t if_id,void* container);

void cp_interface_start(const uint32_t* bundle_rules, size_t bundle_rules_n);
//get whole interface frame size, including header.
static inline size_t cp_if_get_frame_size(cp_if_frame_t* frame)
{
    return sizeof(cp_if_frame_t)+frame->len;
}

cp_if_t* cp_get_interface(int bus, uint32_t if_id);

#define BITMAP_FIXED_POINT MAX_INTERFACE_PER_BUS
#define INTERFACE_ID_BITMAP(if_id) ( 1 << (if_id))
#define BUS_BITMAP(bus)            ( (1 << (bus)) << BITMAP_FIXED_POINT)


#endif
