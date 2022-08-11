#include <mutex.h>
#include <heap.h>
#include <string.h>
#include "arbitrator.h"


typedef struct disp_arb_slave_t
{
    sdm_display_t* disp;
    disp_preempt_level_t level;
    disp_token_t ctk;
    struct sdm_post_config* pd;

}disp_arb_slave_t;

typedef struct disp_arb_master_t
{
    int slave_num;
    int conn_num;
    disp_arb_slave_t* slave_cluster;
    struct list_node conn_ls;

}disp_arb_master_t;

typedef struct disp_arb_conn_t
{
    struct list_node node;
    disp_token_t token;
    struct
    {
        disp_arb_slave_t* pslave;
        disp_preempt_level_t level;
    }* slave;

    int slave_num;

}disp_arb_conn_t;



static disp_token_t tkc = 1;
static disp_arb_master_t display_arbitrator;


static inline disp_arb_conn_t* disp_arb_get_conn(disp_token_t token)
{
    struct list_node* node = &display_arbitrator.conn_ls;

    while(node->next)
    {
        node = node->next;
        disp_arb_conn_t* conn = containerof(node,disp_arb_conn_t,node);
        if(conn->token == token)
        {
            return conn;
        }
    }

    ARB_DBG("Could not find disp conn from token:%d\n",token);
}

static inline disp_arb_slave_t* disp_arb_get_slave(int id)
{
    for(int i=0;i<display_arbitrator.slave_num;i++)
    {
        if(display_arbitrator.slave_cluster[i].disp->id == id)
        {
            return &display_arbitrator.slave_cluster[i];
        }
    }

    ARB_DBG("Could not find disp slave from id:%d\n",id);

    return NULL;
}

void disp_arb_init(void)
{
    ARB_DBG("init display arbitrator\n");
    display_arbitrator.slave_num = hal_display_num();
    ARB_DBG("get disp slave num:%d\n",display_arbitrator.slave_num);
    display_arbitrator.slave_cluster = malloc(sizeof(disp_arb_slave_t) * display_arbitrator.slave_num);
    if(!display_arbitrator.slave_cluster)
    {
        ARB_DBG("malloc arb slave cluster fail\n");
        return;
    }

    //init backend
    struct list_node * node = sdm_get_display_list();

    for(int i=0;i<display_arbitrator.slave_num;i++)
    {
        display_arbitrator.slave_cluster[i].level = DISP_PRMPT_NEVER;
        display_arbitrator.slave_cluster[i].ctk = 0;
        display_arbitrator.slave_cluster[i].pd = NULL;
        display_arbitrator.slave_cluster[i].disp = containerof(node->next,sdm_display_t,node);
        node = node->next;
    }

    //init frontend
    display_arbitrator.conn_num = 0;
    display_arbitrator.conn_ls.next = display_arbitrator.conn_ls.prev = NULL;

}

disp_token_t disp_arb_register_client(void)
{
    disp_arb_conn_t* conn = malloc(sizeof(disp_arb_conn_t));
    if(!conn)
    {
        ARB_DBG("malloc arb conn fail\n");
        return 0;
    }

    conn->pslave = malloc(sizeof(void*) * display_arbitrator.slave_num);

    if(!conn)
    {
        ARB_DBG("malloc arb conn pslave fail\n");
        free(conn);
        return 0;
    }

    memset(conn->pslave,0,sizeof(void*) * display_arbitrator.slave_num);

    display_arbitrator.conn_num++;

    conn->slave_num = 0;
    conn->token = tkc++;
    conn->node.prev = &display_arbitrator.conn_ls;
    conn->node.next = display_arbitrator.conn_ls.next;
    display_arbitrator.conn_ls.next = &conn->node;

    return conn->token;

}

bool disp_arb_append_display(disp_token_t token, int id, disp_preempt_level_t level)
{
    disp_arb_conn_t* conn = disp_arb_get_conn(token);
    if(!conn)
    {
        ARB_DBG("Invalid token:%d while append display\n",token);
        return false;
    }

    disp_arb_slave_t* slave = disp_arb_get_slave(id);

    if(!slave)
    {
        ARB_DBG("Invalid id:%d while append display\n",id);
        return false;
    }

    //check if disp is already appended.

    for(int i=0;i<conn->slave_num;i++)
    {
        if(conn->pslave[i] == slave)
        {
            ARB_DBG("display id %d already appended on token %d\n",id,token);
            return true;
        }
    }

    //check if disp is already connected to another token.

    if(slave->ctk)
    {

    }

    conn->pslave[conn->slave_num] = slave;

    conn->slave_num++;

}



// disp_preempt_level_t disp_arb_get_preemption(disp_token_t token)
// {
//     disp_arb_slave_t* slave = disp_arb_get_slave(token);
//     if(!slave)
//     {
//         ARB_DBG("could not find relative slave:%s\n",disp->handle->name);
//         return DISP_PRMPT_INVALID;
//     }
//     else
//     {
//         ARB_DBG("get slave %s prmpt:%d\n",disp->handle->name,slave->level);
//         return slave->level;
//     }
// }

// bool disp_arb_set_preemption(sdm_display_t* disp, disp_preempt_level_t level)
// {
//     disp_arb_slave_t* slave = disp_arb_get_slave_by_sdm(disp);
//     if(!slave)
//     {
//         ARB_DBG("could not find relative slave:%s\n",disp->handle->name);
//         return false;
//     }
//     else
//     {
//         ARB_DBG("set slave %s prmpt:%d\n",disp->handle->name,slave->level);
//         slave->level = level;
//         return true;
//     }
// }