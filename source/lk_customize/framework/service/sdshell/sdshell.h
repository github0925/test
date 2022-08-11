#ifndef __SDSHELL_SERVICE_H__
#define __SDSHELL_SERVICE_H__
#include "dev/vuart.h"

#define SDSHELL_SERVICE_NAME            "sdshell-service"
#define SDSHELL_CMDBUF_LEN          (128)
#define MAX_IPCC_BUFF_SIZE          IPCC_MB_MTU

#define CMD_RXBUF_SIZE              64
#define LINE_TXBUF_SIZE             384

typedef enum {
    SDSHELL_MSG_READ = 1,
    SDSHELL_MSG_CLEAR,
    SDSHELL_MSG_RUN_CMD,
    SDSHELL_MSG_TARGET_SET,
    SDSHELL_MSG_QUIT,
    SDSHELL_MSG_PRINTON,
    SDSHELL_MSG_PRINTOFF,
    SDSHELL_MSG_MAX,
} msg_type_t;

typedef struct sdshell_msg {
    uint32_t type;
    uint32_t size;
    uint8_t data[0];
} sdshell_msg_t;

#endif /* __SDSHELL_SERVICE_H__ */
