#include <assert.h>
#include <sys_diagnosis.h>
#include <res.h>
#include <reg.h>
#include <chip_res.h>

#if WITH_FRAMEWORK_SERVICE_PROPERTY
#include <dcf.h>
#include <dcf_common.h>
#endif

static int hard_code_wdt_clear(uint32_t resid)
{
    addr_t addr;
    int32_t idx;
    res_get_info_by_id(resid,&addr,&idx);

    if(!addr)
    {
        printf("wrong wdt res:0x%x\n",resid);
        ASSERT(addr);
    }

    //below part should be replaced by driver or hal.
    uint32_t val = readl(addr+0x24);
    writel((val | (1 << 8) ),addr+0x24);


    return 0;
}

__attribute__((weak)) void reboot_loading(int signal)
{
    printf("reboot loading stub: signal %d\n",signal);
}

__attribute__((weak)) void reboot_trigger(int signal)
{
    printf("reboot trigger stub: signal %d\n",signal);
}

enum rb_state_t
{
    rbStateObserved = 1,
    rbStateConfRecv,
    rbStateDone,
    rbStateFin,
};

#define PROP_MASK(x)   (0xF << (x-1)*4)
#define PROP_STATE_GET(x,v) (uint32_t)( (v & PROP_MASK(x)) >> (x-1)*4 )
#define PROP_STATE_SET(x,v,s) (uint32_t)( (v & ~PROP_MASK(x)) | s << (x-1)*4)

static void rb_state_wait(enum rb_state_t state, int src)
{
#if WITH_FRAMEWORK_SERVICE_PROPERTY
    int val = 0;
    do
    {
        system_property_get(DMP_ID_REBOOT_STATUS,&val);
    } while (PROP_STATE_GET(src,val) < state);
#endif
}

static void rb_state_update(enum rb_state_t state,int src)
{
#if WITH_FRAMEWORK_SERVICE_PROPERTY
    int val = 0;
    system_property_get(DMP_ID_REBOOT_STATUS,&val);
    val = PROP_STATE_SET(src,val,state);
    system_property_set(DMP_ID_REBOOT_STATUS,val);
#endif
}



int domain_reboot_probe(int signal, void* args)
{
    int val = 0;

#if WITH_FRAMEWORK_SERVICE_PROPERTY
    system_property_get(DMP_ID_REBOOT_STATUS,&val);
#endif

    switch(signal)
    {
    case WDT5____ovflow_int:

        if(!PROP_STATE_GET(5,val)) //no one observe, just go pass
        {
            hard_code_wdt_clear(RES_WATCHDOG_WDT5);
            reboot_loading(signal);
            reboot_trigger(signal);
        }
        else
        {
            rb_state_wait(rbStateConfRecv,5);

            hard_code_wdt_clear(RES_WATCHDOG_WDT5);

            reboot_loading(signal);

            rb_state_wait(rbStateDone,5);//wait other observer to do specific loading action

            reboot_trigger(signal);

            rb_state_update(rbStateFin,5);
        }

    break;

    case WDT6____ovflow_int:
        if(!PROP_STATE_GET(6,val)) //no one observe, just go pass
        {
            hard_code_wdt_clear(RES_WATCHDOG_WDT6);
            reboot_loading(signal);
            reboot_trigger(signal);
        }
        else
        {
            rb_state_wait(rbStateConfRecv,6);

            hard_code_wdt_clear(RES_WATCHDOG_WDT6);

            reboot_loading(signal);

            rb_state_wait(rbStateDone,6);//wait other observer to do specific loading action

            reboot_trigger(signal);

            rb_state_update(rbStateFin,6);
        }
    break;

    case WDT4____ovflow_int:
        if(!PROP_STATE_GET(4,val)) //no one observe, just go pass
        {
            hard_code_wdt_clear(RES_WATCHDOG_WDT4);
            reboot_loading(signal);
            reboot_trigger(signal);
        }
        else
        {
            rb_state_wait(rbStateConfRecv,4);

            hard_code_wdt_clear(RES_WATCHDOG_WDT4);

            reboot_loading(signal);

            rb_state_wait(rbStateDone,4);//wait other observer to do specific loading action

            reboot_trigger(signal);

            rb_state_update(rbStateFin,4);
        }
    break;

    default:break;
    }

    printf("domain reboot probe fin.\n");

    return 0;

}

#if 0
SYSD_CALL(reboot_probe)
{
    sysd_register_daemon_handler(domain_reboot_probe,NULL,WDT5____ovflow_int);
    sysd_register_daemon_handler(domain_reboot_probe,NULL,WDT6____ovflow_int);
    sysd_register_daemon_handler(domain_reboot_probe,NULL,WDT4____ovflow_int);
}
#endif
