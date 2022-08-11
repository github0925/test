#ifndef __SMS_MONITOR_H__
#define __SMS_MONITOR_H__
//#include "vuart.h"

#define SMS_MONITORP_EPT        (100)
#define SMS_MONITOR_NAME            "sms_monitor"
#define SMS_CMDBUF_LEN          (128)
#define MAX_IPCC_BUFF_SIZE          (492) /* ipcc max msg */
#define MAX_AVM_SIZE            (5242880) /* AVM SIZE 5MB*/

/*
 * service thread stuff
 */
#define SAMPLE_INIT_LEVEL            (LK_INIT_LEVEL_PLATFORM + 1)
#define SAMPLE_SERVICE_STACK_SIZE    DEFAULT_STACK_SIZE
#define WAIT_RPMSG_DEV_TIMEOUT       (10000U)

struct ipcc_channel *sms_chan = NULL;

typedef enum {
    SMS_MSG_START = 1,
    SMS_MSG_DATA,
    SMS_MSG_END,
    SMS_MSG_MAX,
} msg_type_t;


int sms_con_init(void);
int sms_recv_cmd(char *buf, int len);
int sms_print_msg(const char *buf, int len);

#endif /* __SMS_MONITOR_H__ */