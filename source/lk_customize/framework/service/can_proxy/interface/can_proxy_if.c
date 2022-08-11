/*
 * Interface of can proxy
 * Copyright (c) 2020 Semidrive Semiconductor, Inc.
 * All rights reserved.
 */
#include <thread.h>
#include <heap.h>
#include <assert.h>
#include <sys_priority.h>
#include "can_proxy.h"

typedef struct cp_if_set_t
{
    cp_if_t interface[cpifBusMax][MAX_INTERFACE_PER_BUS];
    uint32_t if_id_bitmap[cpifBusMax];
    const uint32_t* bundle_rules;
    size_t    bundle_rules_n;
}cp_if_set_t;


static cp_if_set_t xLocalCPIFS;

cp_if_t* cp_get_interface(int bus, uint32_t if_id)
{
    if (bus >= cpifBusMax || if_id >= MAX_INTERFACE_PER_BUS)
        return NULL;

    return &xLocalCPIFS.interface[bus][if_id];
}

static void cp_interface_dispatch(cp_if_set_t* cpifs)
{
    void* pdata = NULL;
    size_t size = 0;
    uint32_t bus_bitmap;
    uint32_t if_id_bitmap;

    for(uint32_t i=0;i<cpifs->bundle_rules_n;i++)
    {
        size = cp_bundle_detach(cpifs->bundle_rules[i],&pdata);
        if(!size)
            continue;

        CPDBG("==== Dispatcher %d Bundle:0x%x ====\n",size,cpifs->bundle_rules[i]);
        REV_BUNDLE_ID(cpifs->bundle_rules[i],bus_bitmap,if_id_bitmap);

        for(int bus = 0;bus < cpifBusMax;bus++)
        {
            if( BUS_BITMAP(bus) & bus_bitmap)
            {
                CPDBG("Found Target bus %d\n",bus);
                for(int if_id = 0; if_id < MAX_INTERFACE_PER_BUS;if_id++)
                {
                    if(INTERFACE_ID_BITMAP(if_id) & if_id_bitmap &&
                    INTERFACE_ID_BITMAP(if_id) & cpifs->if_id_bitmap[bus])
                    {
                        CPDBG("Found Target Interface %d\n",if_id);
                        if(cpifs->interface[bus][if_id].tx)
                        {
                            cpifs->interface[bus][if_id].tx(&cpifs->interface[bus][if_id],pdata,size);
                        }
                    }
                }
            }
        }
    }

}


/* main worker of routing */
static void cp_interface_worker(cp_if_set_t* cpifs)
{
    cp_if_t* pif = NULL;

    size_t len = 0;

    __CP_CREATE_PFRAME(cp_if_frame_t,MAX_IF_DATA_SIZE,frame);


    CPDBG("cp_interface_worker start\n");

    while(1)
    {
        pif = NULL;

        len = cp_if_frame_get(frame,MAX_IF_DATA_SIZE);

        if(len)
        {
            CPDBG("Worker recv:%d\n",len);
            pif = &cpifs->interface[frame->bus][frame->if_id];
            if(pif->rxcb)
            {
                pif->rxcb(pif,frame->data,len);
            }
            else
            {
                CPDBG("if type %d bus %d has no rxcb\n",frame->bus,frame->if_id);
            }
        }
        else
        {
            CPDBG("Worker recv error!\n");
        }

        cp_interface_dispatch(cpifs);

    }

}

void cp_interface_start(const uint32_t* bundle_rules, size_t bundle_rules_n)
{

    xLocalCPIFS.bundle_rules = bundle_rules;
    xLocalCPIFS.bundle_rules_n = bundle_rules_n;

    for(int bus = 0;bus < cpifBusMax;bus++)
    {
        for(int if_id = 0; if_id < MAX_INTERFACE_PER_BUS;if_id++)
        {
            if( INTERFACE_ID_BITMAP(if_id) & xLocalCPIFS.if_id_bitmap[bus])
            {
                if(xLocalCPIFS.interface[bus][if_id].init)
                {
                    CPDBG("init %d:%d cp interface\n",bus,if_id);
                    if(xLocalCPIFS.interface[bus][if_id].init(&xLocalCPIFS.interface[bus][if_id]))
                    {
                        CPDBG("init %d:%d cp interface fail.\n",bus,if_id);
                        xLocalCPIFS.if_id_bitmap[bus] &= ~(INTERFACE_ID_BITMAP(if_id));
                    }
                }
            }
        }
    }

    thread_t* interface_worker = thread_create(
        "cp_interface_worker",
        (thread_start_routine)cp_interface_worker,
        &xLocalCPIFS,
        THREAD_PRI_CANPROXY,
        1024);

    thread_detach_and_resume(interface_worker);

}

void cp_interface_setting(
    cp_if_bus_t bus,
    uint32_t if_id,
    cp_if_init_t init_proc,
    cp_if_tx_t tx_proc,
    cp_if_rx_cb_t rxcb_proc,
    void* container,
    size_t ctn_size)
{

    ASSERT(bus < cpifBusMax && if_id < MAX_INTERFACE_PER_BUS);
    cp_if_t* pif = &xLocalCPIFS.interface[bus][if_id];

    pif->bus = bus;
    pif->if_id = if_id;
    pif->init = init_proc;
    pif->tx = tx_proc;
    pif->rxcb = rxcb_proc;
    pif->container = container;
    pif->ctn_size = ctn_size;

    xLocalCPIFS.if_id_bitmap[bus] |= INTERFACE_ID_BITMAP(if_id);

    CPDBG("Setting interface:%d:%d bitmap:0x%x\n",bus,if_id,xLocalCPIFS.if_id_bitmap[bus]);

}

void cp_interface_duplicate(cp_if_bus_t bus,uint32_t if_id,void* container)
{
    ASSERT(bus < cpifBusMax && if_id < MAX_INTERFACE_PER_BUS);
    cp_if_t* pif = &xLocalCPIFS.interface[bus][if_id];

    for(int i = 0; i < MAX_INTERFACE_PER_BUS;i++)
    {
        if(INTERFACE_ID_BITMAP(i) & xLocalCPIFS.if_id_bitmap[bus])
        {
            CPDBG("Found Duplicated Interface %d\n",i);
            pif->bus = bus;
            pif->if_id = if_id;
            pif->init = xLocalCPIFS.interface[bus][i].init;
            pif->tx = xLocalCPIFS.interface[bus][i].tx;
            pif->rxcb = xLocalCPIFS.interface[bus][i].rxcb;
            pif->container = container;
            pif->ctn_size = xLocalCPIFS.interface[bus][i].ctn_size;

            break;

        }
    }

    xLocalCPIFS.if_id_bitmap[bus] |= INTERFACE_ID_BITMAP(if_id);

    CPDBG("Init interface:%d:%d bitmap:0x%x\n",bus,if_id,xLocalCPIFS.if_id_bitmap[bus]);
}





