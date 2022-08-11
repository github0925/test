#include <stdint.h>
#include <sem.h>
#include <app.h>

#define NULL_SIGNAL -1


typedef int (*sys_event_cb_t)(int signal, void* args);


/* register a callback to speicific signal(s)
 * e.g.
 * sysd_register_handler(callback,args,2,WDT1____ovflow_int,WDT2____ovflow_int);
 * return: < 0 if fail or =0 if succ
 */
int sysd_register_handler(sys_event_cb_t cb,void* args,uint32_t n,...);

/* register a daemon callback to speicific signal
 * This would be invoked at the last callback of the callback chain.
 */
int sysd_register_daemon_handler(sys_event_cb_t cb,void* args,int signal);

/* start sys diagnosis service. Invoked at platform_init phase.*/
void sysd_start(enum sem sem);

/* static entry definition macro. Be invoked after platform_init*/
#define SYSD_CALL(register_entry) void sysd_##register_entry(void);APP_START(sysd_##register_entry).init = (app_init)sysd_##register_entry,.entry = NULL,APP_END void sysd_##register_entry(void)