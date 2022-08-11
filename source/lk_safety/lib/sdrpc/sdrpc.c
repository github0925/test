
#include <stdint.h>
#include <reg.h>
#include <bits.h>
#include "sdrpc.h"




static inline void general_reg_set(uint8_t bit, bool val)
{
    uint32_t reg = readl(SAF_WRITE_REG);
    if(val)
    {
        reg |= (1UL << bit);
    }
    else
    {
        reg &= ~(1UL <<bit);
    }

    writel(reg,SAF_WRITE_REG);
}

static inline uint32_t general_reg_read(uint8_t bit)
{
    uint32_t reg = readl(SEC_WRITE_REG);
    return (reg & (1UL << bit)) >> bit;
}


void sdrpc_notify_msg(sdrpc_handle_t sdrpc,rpc_commands_type cmd,sdprc_payload_t* payload)
{
    (void)sdrpc;


    switch(cmd)
    {
    case COM_HANDOVER_STATUS:
        general_reg_set(1, payload ? false:true);
    break;
    case COM_DC_STATUS:
        general_reg_set(2, payload ? false:true);
    break;
    case COM_VDSP_STATUS:
        general_reg_set(3, payload ? false:true);
    break;
    case COM_VPU_STATUS:
        general_reg_set(4, payload ? false:true);
    break;
    default:
        general_reg_set(0, payload ? false:true);
    break;

    }

}

uint32_t sdrpc_read_msg(sdrpc_handle_t sdrpc,rpc_commands_type cmd, sdprc_payload_t* payload)
{
    (void)sdrpc;
    (void)payload;

    uint32_t val = 0;

    switch(cmd)
    {
    case COM_DDR_STATUS:
        val = general_reg_read(0);
    break;
    case COM_PORT_STATUS:
        val = general_reg_read(1);
    break;
    case COM_PLL_CLK_STATUS:
        val = general_reg_read(2);
    break;
    default:
    break;

    }

    return val;

}