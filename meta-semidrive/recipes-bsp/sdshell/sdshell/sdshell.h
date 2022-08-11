#ifndef _SDSHELL_H_
#define _SDSHELL_H_

#include <stdint.h>

#define SUCC				0
#define FAIL				-1

#define SDSHELL_SERVICE_EPT		97
#define MAX_RPMSG_BUFF_SIZE		512
#define SDSHELL_PROMPT_LEN		32

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

#define PRINTK_DIR "/proc/sys/kernel/printk"
#define KERNEL_LOG_OFF			1
#define KERNEL_LOG_ON			0

typedef enum {
	SDRV_SAFETY = 0,
	SDRV_SECURE,
	SDRV_MP,
	SDRV_AP1,
	SDRV_AP2,
	SDRV_CORE_MAX
} core_type_t;

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

typedef enum {
	DP_CR5_SAF = 0,
	DP_CR5_SEC = 1,
	DP_CR5_MPC = 2,
	DP_CA_AP1  = 3,
	DP_CA_AP2  = 4,
	DP_DSP_V   = 5,
	DP_CPU_MAX,
} rpmsg_cpu_id_t;

typedef struct sdshell_msg {
	uint32_t type;
	uint32_t size;
	uint8_t data[0];
} sdshell_msg_t;

typedef struct target_desc {
	const char *name;
	uint32_t core_id;
	uint32_t rpmsg_id;
} target_desc_t;

typedef struct sdshell_control {
	uint8_t dlevel;
	int fd;  /* file descriptor for socket */
	int ttys; /* 1: ttyS*, 0: other */
	const target_desc_t *tgt;
} sdshell_ctl_t;

typedef enum {
	LOG_NONE = 0,
	LOG_ERROR,
	LOG_INFO,
	LOG_DEBUG,
} log_level_t;

log_level_t program_log_level = LOG_INFO;
#define FILENAME(x) strrchr(x, '/') ? strrchr(x, 'x') + 1 : x
#define debug(log_level, fmt, args...)									\
	do{												\
		if(log_level <= program_log_level) {							\
			if (log_level == LOG_DEBUG) {							\
				printf("[%s, L%d]%s: ", FILENAME(__FILE__), __LINE__, __FUNCTION__);	\
			}										\
			printf(fmt, ##args);								\
		}											\
	}while(0)

#endif /* _SDSHELL_H_ */
