/*
* Copyright (c) 2019 Semidrive Semiconductor Inc.
* All rights reserved.
*
* Description: domain communication common define for kunlun
*
*/

#ifndef _DCF_COMMOM_H_
#define _DCF_COMMOM_H_

#include <reg.h>
#include <stdio.h>
#include <trace.h>
#include <errno.h>
#include <mbox_hal.h>

typedef enum {
    DP_CR5_SAF,
    DP_CR5_SEC,
    DP_CR5_MPC,
    DP_CA_AP1,
    DP_CA_AP2,
    DP_DSP_V,
    DP_CPU_MAX,
} domain_cpu_id_t;

typedef enum {
    DMP_ID_DDR_STATUS  = 0,
    DMP_ID_PORT_STATUS = 1,
    DMP_ID_PLL_CLK_STATUS  = 2,
    DMP_ID_HANDOVER_STATUS = 3,
    DMP_ID_DC_STATUS   = 4,
    DMP_ID_VDSP_STATUS = 5,
    DMP_ID_VPU_STATUS  = 6,
    DMP_ID_I2C_STATUS   = 7,
    DMP_ID_REBOOT_STATUS   = 8,
    DMP_ID_AVM_STATUS = 9,
    DMP_ID_BA_STATUS = 10,
    DMP_ID_DMS_STATUS = 11,
    DMP_ID_CLUSTER_STATUS = 12,
    DMP_ID_CP_STATUS = 13,
    DMP_ID_MAX

} domain_property_id_t;

/* APU and RPU bus offset for DDR base address */
#define SHM_PA_R_A_OFFSET       (0x10000000)
/* APU bus DDR address from here */
#define SHM_AP_DDR_BASE         (0x40000000)

#define MEM_IN_RANGE(val, start, size)	\
	((val >= start) && (val < (start + size)))

#if defined(CONFIG_SUPPORT_DCF) && (CONFIG_SUPPORT_DCF == 1)
/* use device config for rpmsg rtos by default */
#define CONFIG_RPMSG_DEVCONF    (1)
/* set 1 when use system property instead of SoC general register */
#define CONFIG_USE_SYS_PROPERTY (1)

#include <rpmsg_common.h>
#include <property.h>
#include <ipcc_rpc.h>

#if (CONFIG_USE_IPCC_RPC == 0)
/* sys property rdepends on RPC, so double check after include ipcc config */
#undef CONFIG_USE_SYS_PROPERTY
#define CONFIG_USE_SYS_PROPERTY (0)
#endif

#define CONFIG_HAS_DCF_MSGHDR   (0)

typedef struct ipcc_service {
    int rproc;
    struct ipcc_device  *rpdev;
    u32                 capabilities;
    bool                ready;

#if CONFIG_IPCC_ECHO_EPT
    struct ipcc_channel *echo_chan;
#endif

#if CONFIG_USE_IPCC_TTY
    struct ipcc_channel *tty_dev;
#endif

#if CONFIG_USE_IPCC_RPC
    struct ipcc_rpc_server rpc_server;
    struct ipcc_rpc_client rpc_client;
#endif
    void                *rpmsg_dev; // compatible only, do not use directly
} *ipcc_service_handle_t;


#if CONFIG_IPCC_RPMSG
ipcc_service_handle_t dcf_get_service_handle(int rproc);
inline static struct rpmsg_dcf_instance *dcf_get_raw_device(int rproc)
{
    ipcc_service_handle_t h = dcf_get_service_handle(rproc);

    return h->rpdev ? h->rpdev->rpmsg_dev : NULL;
}
#else
inline static ipcc_service_handle_t dcf_get_service_handle(int rproc)
{
    return NULL;
}
inline static struct rpmsg_dcf_instance *dcf_get_raw_device(int rproc)
{
    return  NULL;
}
#endif

inline static ipcc_service_handle_t find_service_handle(int rproc)
{
    return dcf_get_service_handle(rproc);
}

#if CONFIG_USE_IPCC_RPC
status_t __dcf_ping(ipcc_service_handle_t dp);
status_t __dcf_gettimeofday(ipcc_service_handle_t dp);
int __dcf_call(ipcc_service_handle_t dp, rpc_call_request_t *request,
                  rpc_call_result_t *result, lk_time_t timeout);
int dcf_setup_func(ipcc_service_handle_t dp, rpc_server_impl_t *tables, int num);
int dcf_setup_func_detail(ipcc_service_handle_t dp, u32 cmd, rpc_server_func_t fn);
void start_ipcc_rpc_service(rpc_server_impl_t *tables, int num);
void dcf_register_rpc_implement(rpc_server_impl_t *fn);
void dcf_register_rpc_implement_detailed(u32 cmd, rpc_server_func_t fn);

/* the helper function for system default ipcc services */
inline static int dcf_call(int rproc, rpc_call_request_t *request, rpc_call_result_t *result, lk_time_t timeout)
{
    return __dcf_call(dcf_get_service_handle(rproc), request, result, timeout);
}

inline static int dcf_ping(int rproc)
{
    return __dcf_ping(dcf_get_service_handle(rproc));
}

inline static int dcf_gettimeofday(int rproc)
{
    return __dcf_gettimeofday(dcf_get_service_handle(rproc));
}

/***************************************************************
** POSIX IO API
****************************************************************/
#define DCF_POSIX_FILE_IO_NAMES  (1)
#define DCF_MAX_FILES   (16)

/* dcf file flags */
#define O_RDONLY        00000000
#define O_WRONLY        00000001
#define O_RDWR          00000002
#define O_CREAT         00000100
#define O_NONBLOCK      00000400
#define O_ACCMODE       (O_WRONLY|O_RDWR)

#define DEV_SM_I        "/dev/sdsm"
#define DEV_LOOPBACK    "/dev/loopback"
#define DEV_PROPERTY    "/dev/property"
#define DEV_DISP_C      "/dev/cluster"
#define DEV_DISP_I      "/dev/ivi"
#define DEV_VIRCAN_I    "/dev/vircan0"
#define DEV_VIRCAN_C    "/dev/vircan1"
#define DEV_SSYSTEM     "/dev/ssystem"
#define DEV_SA_VI2C     "socket:vi2c"

#if CONFIG_SUPPORT_POSIX

struct dcf_file {
    char file_name[DCF_NAM_MAX_LEN];
    char ns_name[DCF_NAM_MAX_LEN];
    int rpmsg_endpoint;
    int remote_processor;

    unsigned int f_flags;
    unsigned int f_state;
    struct ipcc_device *ipdev;
    struct ipcc_channel *rpchan;
    int refcount;

    s16 rcvevent;
    u16 sendevent;
    u16 errevent;
    /** last error that occurred on this file */

    void *private_data;
};

/* Helper functions */
struct dcf_file *dcf_get_file(int fd);

int dcf_open(const char *name, unsigned int flags);
int dcf_close(int fd);
int dcf_read(int fd, void *mem, size_t len);
int dcf_write(int fd, void *mem, size_t len);
int dcf_fcntl(int fd, int cmd, int val);
int dcf_file_init(void);
void dcf_file_list(int argc);
#include "dcf_poll.h"

int dcf_file_poll(struct pollfd *pf);

#if DCF_POSIX_FILE_IO_NAMES
#define posix_open(a,b)     dcf_open(a,b)
#define posix_read(a,b,c)   dcf_read(a,b,c)
#define posix_write(a,b,c)  dcf_write(a,b,c)
#define posix_close(s)      dcf_close(s)
#define posix_fcntl(a,b,c)  dcf_fcntl(a,b,c)
#define posix_select(a,b,c,d,e)  dcf_select(a,b,c,d,e)
#endif

#else
inline static int dcf_file_init(void)
{
    return 0;
}
#endif

inline static void posix_set_errno(int err)
{
    errno = err;
}

/***************************************************************
** Interrupt-based Domain Notification API
****************************************************************/
struct dcf_notifier_attribute {
    u32 rproc;
    u16 addr;
    u32 n_flags;
};

struct dcf_notifier {
    hal_mb_client_t client;
    hal_mb_chan_t *mchan;
};

void dcf_notify_attr_init(struct dcf_notifier_attribute *attr, int rproc, int addr);
struct dcf_notifier *
dcf_create_notifier(struct dcf_notifier_attribute *attr);
void dcf_destroy_notifier(struct dcf_notifier *ntf);
int dcf_do_notify(struct dcf_notifier *ntf, u8 *data, u16 len);

#else
inline static int dcf_call(int rproc, rpc_call_request_t *request, rpc_call_result_t *result, lk_time_t timeout)
{
    return 0;
}
inline static int dcf_ping(int rproc)
{
    return 0;
}
inline static int dcf_gettimeofday(int rproc)
{
    return 0;
}
inline static int dcf_setup_func(ipcc_service_handle_t dp, rpc_server_impl_t *tables, int num)
{
    return 0;
}
inline static int dcf_setup_func_detail(ipcc_service_handle_t dp, u32 cmd, rpc_server_func_t fn)
{
    return 0;
}
inline static void start_ipcc_rpc_service(rpc_server_impl_t *tables, int num)
{
}
#endif
int dcf_get_this_proc(void);

/* Alias of above interface */
inline static int dcf_current_proc(void)
{
    return dcf_get_this_proc();
}

#endif

#if defined(CONFIG_RPMSG_SERVICE) && (CONFIG_RPMSG_SERVICE == 1)
paddr_t platform_shm_get_local(paddr_t remote_pa);

paddr_t platform_shm_get_remote(paddr_t local_pa);
#endif

#if defined(CONFIG_RPMSG_DEVCONF) && (CONFIG_RPMSG_DEVCONF == 1)
/* this function is implemented in platform-specific dcf.c */
int platform_get_rpmsg_config(struct rpmsg_dev_config **p);
#else
struct rpmsg_dev_config;
inline static int platform_get_rpmsg_config(struct rpmsg_dev_config **p)
{
    return 0;
}
#endif

void dcf_early_init(void);

void dcf_init(void);

#endif //_DCF_COMMOM_H_
