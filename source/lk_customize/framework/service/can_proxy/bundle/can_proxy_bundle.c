#include <stdio.h>
#include <heap.h>
#include "can_proxy.h"
#include "can_proxy_bundle.h"


typedef struct cp_bundle_pack_t
{
    struct cp_bundle_pack_t* next;
    size_t size;
    uint8_t* data;
}cp_bundle_pack_t;


typedef struct cp_bundle_t
{
    uint32_t bundle_id;
    int pack_n;
    cp_bundle_pack_t* pack;

}cp_bundle_t;

typedef struct cp_bundle_node_t
{
    cp_bundle_t bundle;
    struct cp_bundle_node_t* next;
}cp_bundle_node_t;

static cp_bundle_node_t* pxBundle = NULL;

static cp_bundle_t* cp_bundle_get_handle(uint32_t bundle_id)
{
    cp_bundle_node_t* node = pxBundle;

    while(node)
    {
        if(node->bundle.bundle_id == bundle_id)
        {
            return &node->bundle;
        }

        node = node->next;
    }

    return NULL;
}

static void cp_bundle_create_handle(uint32_t bundle_id)
{
    if(cp_bundle_get_handle(bundle_id))
    {
        printf("Already create %d bundle.\n",bundle_id);
        return;
    }
    else
    {
        cp_bundle_node_t* node = malloc(sizeof(cp_bundle_node_t));
        if(!node)
        {
            printf("Cannot create bundle.\n");
            return;
        }

        node->bundle.bundle_id = bundle_id;
        node->bundle.pack_n = 0;
        node->bundle.pack = NULL;

        node->next = pxBundle;
        pxBundle = node;

        printf("Create bundle id:0x%x\n",bundle_id);

    }

    return;

}

bool cp_bundle_attach(uint32_t bundle_id,void* data,size_t size)
{
    cp_bundle_t* bundle = cp_bundle_get_handle(bundle_id);

    if(!bundle)
    {
        cp_bundle_create_handle(bundle_id);
        bundle = cp_bundle_get_handle(bundle_id);
    }

    cp_bundle_pack_t* pack = malloc(sizeof(cp_bundle_pack_t));

    if(!pack)
    {
        printf("Cannot malloc bundle pack.\n");
        return false;
    }

    pack->size = size;
    pack->data = data;

    pack->next = bundle->pack;
    bundle->pack = pack;
    bundle->pack_n++;

    return true;
}

size_t cp_bundle_detach(uint32_t bundle_id, void** pdata)
{
    cp_bundle_t* bundle = cp_bundle_get_handle(bundle_id);

    if(!bundle)
    {
        CPDBG("No bundle 0x%x\n",bundle_id);
        return 0;
    }

    if(!pdata)
    {
        CPDBG("pdata NULL!\n");
        return 0;
    }

    size_t ret = 0;

    cp_bundle_pack_t* pack = bundle->pack;

    if(!pack)
    {
        return 0;
    }

    *pdata = pack->data;
    ret = pack->size;

    bundle->pack = pack->next;

    bundle->pack_n--;

    free(pack);

    return ret;
}

