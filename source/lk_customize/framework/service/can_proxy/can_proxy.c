#include <app.h>
#include "can_proxy.h"
#include "rpmsg_port.h"


const uint32_t bundle_rules[] =
{
    TARGET_CAN1_BUNDLE,
    TARGET_CAN2_BUNDLE,
    TARGET_CAN3_BUNDLE,
    TARGET_CAN4_BUNDLE,
    TARGET_CAN_BUNDLE,
    TARGET_IVI_BUNDLE,
    TARGET_CLU_BUNDLE,
    TARGET_VCLU_BUNDLE,
    TARGET_AP_BUNDLE,
    TARGET_VM_BUNDLE,
};


const rpmsg_if_map_t rpmsg_if_id_tlb[RIFN] =
{
#if CONFIG_DCF_HAS_AP1 == 1
    //if_id = 0, ivi
    { RIF1,{VIRCAN_IVI_EPT,DP_CA_AP1, "rpmsg-vircan"} },
    //if_id = 1 -> virtual cluster
    { RIF2,{VIRCAN_CLU_EPT,DP_CA_AP1, "rpmsg-vircan"} },
#else
    { RIF1,{0,0, ""} },
    //if_id = 1 -> virtual cluster
    { RIF2,{0,0, ""} },
#endif

#if CONFIG_DCF_HAS_AP2 == 1
    //if_id = 2 -> cluster
    { RIF3,{VIRCAN_CLU_EPT,DP_CA_AP2, "rpmsg-vircan"} },
#else
    { RIF3,{0,0, ""} },
#endif
};


void start_can_proxy(void)
{
    if(!can_proxy_buffer_init())
    {
        CPDBG("proxy buffer init fail.\n");
        return;
    }
    CPDBG("proxy buffer init done\n");


    for(int i=0;i<4;i++)
    {
        cp_interface_setting(cpifBusCan,
                            i,
                            can_if_init,
                            can_if_tx,
                            can_if_rx_cb,
                            NULL,0);
    }

    for(int i=0;i<RIFN;i++)
    {
        cp_interface_setting(cpifBusRPMSG,
                            rpmsg_if_id_tlb[i].if_id,
                            rpmsg_if_init,
                            rpmsg_if_tx,
                            rpmsg_if_rxcb,
                            (void*)&rpmsg_if_id_tlb[i].port,sizeof(rpmsg_cfg_container_t));
    }


    CPDBG("proxy interface setting done\n");
    cp_interface_start(bundle_rules,sizeof(bundle_rules)/sizeof(bundle_rules[0]));

}

static void can_proxy_entry(const struct app_descriptor *app, void *args)
{
    CPDBG("starting can proxy service\n");
    start_can_proxy();
}

APP_START(canproxy)
.flags = 0,
.entry = can_proxy_entry,
APP_END

