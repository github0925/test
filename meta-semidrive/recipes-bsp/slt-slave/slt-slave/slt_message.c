/*
 * Copyright (c) 2019, SemiDrive, Inc. All rights reserved
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "slt_message.h"

#define LOCAL_TRACE 1
/*
typedef enum {
    IPCC_RRPOC_SAF  = 0,
    IPCC_RRPOC_SEC  = 1,
    IPCC_RRPOC_MP   = 2,
    IPCC_RRPOC_AP1  = 3,
    IPCC_RRPOC_AP2  = 4,
    IPCC_RRPOC_VDSP = 5,
    IPCC_RRPOC_MAX  = IPCC_RRPOC_VDSP,
} hal_mb_proc_t;
*/

//#define SLT_MES_RPMSG_DEVICE_NAME       "virtio1.slt-test-ap1.-1.30"
//soc:ipcc@ ipcc_channel_create
//0 from safety 1 from sec
//slt-test-ap2 channel name
//-1 dst
//96 src
#define SLT_MES_RPMSG_DEVICE_NAME       "soc:ipcc@0.slt-test-ap2.-1.96"
#define SLT_MES_RPMSG_BUS_SYS "/sys/bus/rpmsg"

#define SLT_TEST_EPT_PAYSZ               (4028)

#define SLT_TEST_IPC_RECEIVE_TIMEOUT     INFINITE_TIME //1000
#define SLT_TEST_IPC_SEND_TIMEOUT        100

//------uart send-------------
int slt_uart_send_msg(slt_app_context_t* pcontext, void* msg)
{
    uint32_t i;
    struct slt_init_info* init_info;

    if (msg != NULL) {
        init_info = (struct slt_init_info*)msg;
        printf("software_version:          %d\n", init_info->software_version);
        printf("is_test_ready:             %d\n", init_info->selftesty_ready);
        return 1;
    }

    printf("chip_id:          %d\n", pcontext->slt_test_result.chip_id);
    printf("test_result:      %s\n", (pcontext->slt_test_result.test_result) ? "fail" : "pass");
    printf("test_num:         all:%d,pass:%d,fail:%d\n", pcontext->slt_test_result.all_test_num,
           pcontext->slt_test_result.all_test_pass_num,
           pcontext->slt_test_result.all_test_fail_num);
    printf("use_time:         %d ms\n", pcontext->slt_test_result.all_test_usetime);
    printf("test_cfg:         voltage:%d\n", pcontext->slt_test_result.test_cfg_vo);
    printf("test_cfg:         freq:%d\n", pcontext->slt_test_result.test_cfg_frq);
    printf("software_version: %d\n", pcontext->software_version);
    printf("test_info:\n");
    printf("                  domain: test_num:%d result:%s\n", pcontext->slt_test_result.test_result_info.test_num,
           (pcontext->slt_test_result.test_result_info.test_result) ? "fail" : "pass");

    for (i = 0; i < pcontext->slt_test_result.test_result_info.test_num; i++) {
        printf("                  case%d level:%d name:%s result:%s result_code:%d use_time:%d ms\n", pcontext->slt_test_result.test_result_info.test_case_info[i].case_id,
               pcontext->slt_test_result.test_result_info.test_case_info[i].level,
               pcontext->slt_test_result.test_result_info.test_case_info[i].test_name,
               (pcontext->slt_test_result.test_result_info.test_case_info[i].result_value) ? "fail" : "pass",
               pcontext->slt_test_result.test_result_info.test_case_info[i].result_value,
               pcontext->slt_test_result.test_result_info.test_case_info[i].use_time);

        if (pcontext->slt_test_result.test_result_info.test_case_info[i].result_value != 0) {
            printf("                         fail reason:%s\n", pcontext->slt_test_result.test_result_info.test_case_info[i].result_string);
        }
    }

    return i;
}
//------uart send-------------

//------uart receive-------------

int slt_uart_receive_msg_start_test(void)
{
    int ret;
    slt_app_context_t* pcontext;
    slt_msg_node_t msg_node;

    //init msg_node
    msg_node.msg_item.msg_type = SLT_MESSAGE_FROM_PC_TEST;
    msg_node.msg_item.msg_len = 0;
    msg_node.msg_item.msg_value = NULL;

    //send cmd msg
    pcontext = slt_get_gcontext();
    ret = slt_putmsginqueue(pcontext, &msg_node);

    return ret;
}

//------uart receive-------------

//------ipc send-------------
static int rpmsg_create_ept(int rpfd, struct rpmsg_endpoint_info* eptinfo)
{
    int ret;

    ret = ioctl(rpfd, RPMSG_CREATE_EPT_IOCTL, eptinfo);

    if (ret) {
        perror("Failed to create endpoint.\n");
    }

    return ret;
}

static char* get_rpmsg_ept_dev_name(const char* rpmsg_char_name,
                                    const char* ept_name,
                                    char* ept_dev_name)
{
    char sys_rpmsg_ept_name_path[64];
    char svc_name[64];
    char* sys_rpmsg_path = "/sys/class/rpmsg";
    FILE* fp;
    int i;
    int ept_name_len;

    for (i = 0; i < 128; i++) {
        sprintf(sys_rpmsg_ept_name_path, "%s/%s/rpmsg%d/name",
                sys_rpmsg_path, rpmsg_char_name, i);
        printf("checking %s\n", sys_rpmsg_ept_name_path);

        if (access(sys_rpmsg_ept_name_path, F_OK) < 0) {
            continue;
        }

        fp = fopen(sys_rpmsg_ept_name_path, "r");

        if (!fp) {
            printf("failed to open %s\n", sys_rpmsg_ept_name_path);
            break;
        }

        fgets(svc_name, sizeof(svc_name), fp);
        fclose(fp);
        printf("svc_name: %s.\n", svc_name);
        ept_name_len = strlen(ept_name);

        if (ept_name_len > sizeof(svc_name)) {
            ept_name_len = sizeof(svc_name);
        }

        if (!strncmp(svc_name, ept_name, ept_name_len)) {
            sprintf(ept_dev_name, "rpmsg%d", i);
            return ept_dev_name;
        }
    }

    printf("Not able to RPMsg endpoint file for %s:%s.\n",
           rpmsg_char_name, ept_name);
    return NULL;
}

static int bind_rpmsg_chrdev(const char* rpmsg_dev_name)
{
    char fpath[256];
    char* rpmsg_chdrv = "rpmsg_chrdev";
    int fd;
    int ret;


    /* rpmsg dev overrides path */
    sprintf(fpath, "%s/devices/%s/driver_override",
            SLT_MES_RPMSG_BUS_SYS, rpmsg_dev_name);
    fd = open(fpath, O_WRONLY);

    if (fd < 0) {
        fprintf(stderr, "Failed to open %s, %s\n",
                fpath, strerror(errno));
        return -EINVAL;
    }

    ret = write(fd, rpmsg_chdrv, strlen(rpmsg_chdrv) + 1);

    if (ret < 0) {
        fprintf(stderr, "Failed to write %s to %s, %s\n",
                rpmsg_chdrv, fpath, strerror(errno));
        return -EINVAL;
    }

    close(fd);

    /* bind the rpmsg device to rpmsg char driver */
    sprintf(fpath, "%s/drivers/%s/bind", SLT_MES_RPMSG_BUS_SYS, rpmsg_chdrv);
    fd = open(fpath, O_WRONLY);

    if (fd < 0) {
        fprintf(stderr, "Failed to open %s, %s\n",
                fpath, strerror(errno));
        return -EINVAL;
    }

    ret = write(fd, rpmsg_dev_name, strlen(rpmsg_dev_name) + 1);

    if (ret < 0) {
        fprintf(stderr, "Failed to write %s to %s, %s\n",
                rpmsg_dev_name, fpath, strerror(errno));
        fprintf(stderr,
                "If it's already bind, please unbind with below command, then try again\n");
        fprintf(stderr, "echo %s > %s/drivers/%s/unbind\n", rpmsg_dev_name,
                SLT_MES_RPMSG_BUS_SYS, rpmsg_chdrv);
        return -EINVAL;
    }

    close(fd);
    return 0;
}

static int unbind_rpmsg_chrdev(const char* rpmsg_dev_name)
{
    char fpath[256];
    char* rpmsg_chdrv = "rpmsg_chrdev";
    int fd;
    int ret;

    /* unbind the rpmsg device to rpmsg char driver */
    sprintf(fpath, "%s/drivers/%s/unbind", SLT_MES_RPMSG_BUS_SYS, rpmsg_chdrv);
    fd = open(fpath, O_WRONLY);

    if (fd < 0) {
        fprintf(stderr, "Failed to open %s, %s\n",
                fpath, strerror(errno));
        return -EINVAL;
    }

    ret = write(fd, rpmsg_dev_name, strlen(rpmsg_dev_name) + 1);

    if (ret < 0) {
        fprintf(stderr, "Failed to write %s to %s, %s\n",
                rpmsg_dev_name, fpath, strerror(errno));
        return -EINVAL;
    }

    close(fd);
    return 0;
}

static int get_rpmsg_chrdev_fd(const char* rpmsg_dev_name,
                               char* rpmsg_ctrl_name)
{
    char dpath[256];
    char fpath[256];
    int path_len = 0;
    char* rpmsg_ctrl_prefix = "rpmsg_ctrl";
    DIR* dir;
    struct dirent* ent;
    int fd;

    sprintf(dpath, "%s/devices/%s/rpmsg", SLT_MES_RPMSG_BUS_SYS, rpmsg_dev_name);
    dir = opendir(dpath);

    if (dir == NULL) {
        fprintf(stderr, "Failed to open dir %s\n", dpath);
        return -EINVAL;
    }

    while ((ent = readdir(dir)) != NULL) {
        if (!strncmp(ent->d_name, rpmsg_ctrl_prefix,
                     strlen(rpmsg_ctrl_prefix))) {
            printf("Opening file %s.\n", ent->d_name);
            path_len = snprintf(fpath, sizeof(fpath),  "/dev/%s", ent->d_name);

            if (path_len <= 0) {
                continue;
            }

            fd = open(fpath, O_RDWR);

            if (fd < 0) {
                fprintf(stderr,
                        "Failed to open rpmsg char dev %s,%s\n",
                        fpath, strerror(errno));
                return fd;
            }

            sprintf(rpmsg_ctrl_name, "%s", ent->d_name);
            return fd;
        }
    }

    fprintf(stderr, "No rpmsg char dev file is found\n");
    return -EINVAL;
}

//hal_mb_proc_t remout_id
//chan_id == SLT_TEST_EPT_SAF/SEC/AP1
int slt_ipc_send_msg(slt_app_context_t* pcontext, slt_ipc_msg_t* msg)
{

    int ret = 0;
    char send_buf[IPCC_MB_MTU] = {0,};
    int len = SLT_TEST_EPT_PAYSZ;

    len = sizeof(slt_ipc_msg_t);

    memcpy(send_buf, msg, len);

    if ((pcontext->fd) >= 0) {
        ret = write(pcontext->fd, send_buf, IPCC_MB_MTU);
    }

    if (ret <= 0) {
        printf("ping: send failed %d\n", ret);
    }

    return ret;
}
//------ipc send-------------

//------ipc receive-------------

void* slt_ipc_receive_handle(void* arg)
{
    char* rpmsg_dev = SLT_MES_RPMSG_DEVICE_NAME;
    slt_msg_node_t msg_node;
    slt_app_context_t* pcontext;
    slt_ipc_msg_t* ipc_msg;
    void* msg;
    unsigned long src;
    int ret = 0;
    char rx_buf[IPCC_MB_MTU] = {0,};
    int len = SLT_TEST_EPT_PAYSZ;

    pcontext = (slt_app_context_t*)arg;

    while (1) {

        len = SLT_TEST_EPT_PAYSZ;

        memset(rx_buf, 0, IPCC_MB_MTU);

        ret = read(pcontext->fd, rx_buf, IPCC_MB_MTU);

        if (ret <= 0) {
            printf("wait for reply timeout ret=%d\n", ret);
        }
        else {

            ipc_msg = (slt_ipc_msg_t*)&rx_buf;
            msg = slt_alloc_msg_value_buff(ipc_msg->msg_len);
            memcpy(msg, ipc_msg->msg, ipc_msg->msg_len);

            //init msg_node
            msg_node.msg_item.msg_type = ipc_msg->cmd_type;
            msg_node.msg_item.msg_len = ipc_msg->msg_len;
            msg_node.msg_item.msg_value = msg;

            ret = slt_putmsginqueue(pcontext, &msg_node);
        }
    }

    close(pcontext->fd);

    if (pcontext->charfd >= 0) {
        close(pcontext->charfd);
    }

    if (strncmp(rpmsg_dev, "/dev/", 4)) {
        unbind_rpmsg_chrdev(rpmsg_dev);
    }

    return NULL;
}

int slt_ipc_init(slt_app_context_t* pcontext)
{
    int ret;
    int err_cnt;
    pthread_t t_ipchandle;
    char* rpmsg_dev = SLT_MES_RPMSG_DEVICE_NAME;

    char fpath[256];
    char rpmsg_char_name[16];
    struct rpmsg_endpoint_info eptinfo;
    char ept_dev_name[16];
    char ept_dev_path[32];

    pcontext->charfd = -1;
    pcontext->fd = -1;

    if (strncmp(rpmsg_dev, "/dev/", 4)) {
        /* managed by rpmsg chrdrv */
        printf("\r\n Open rpmsg dev %s! \r\n", rpmsg_dev);
        sprintf(fpath, "%s/devices/%s", SLT_MES_RPMSG_BUS_SYS, rpmsg_dev);

        if (access(fpath, F_OK)) {
            fprintf(stderr, "Not able to access rpmsg device %s, %s\n",
                    fpath, strerror(errno));
            return -EINVAL;
        }

        ret = bind_rpmsg_chrdev(rpmsg_dev);

        if (ret < 0) {
            return ret;
        }

		usleep(100000);
        pcontext->charfd = get_rpmsg_chrdev_fd(rpmsg_dev, rpmsg_char_name);

        if (pcontext->charfd < 0) {
            return pcontext->charfd;
        }

        /* Create endpoint from rpmsg char driver */
        strcpy(eptinfo.name, SLT_MAIN_TEST_EPT_NAME);
        eptinfo.src = SLT_MAIN_TEST_EPT;
        eptinfo.dst = 0xFFFFFFFF;
        ret = rpmsg_create_ept(pcontext->charfd, &eptinfo);

        if (ret) {
            printf("failed to create RPMsg endpoint.\n");
            return -EINVAL;
        }

        if (!get_rpmsg_ept_dev_name(rpmsg_char_name, eptinfo.name,
                                    ept_dev_name)) {
            return -EINVAL;
        }

        sprintf(ept_dev_path, "/dev/%s", ept_dev_name);
    }
    else {
        /* managed by dcf chrdrv */
        strncpy(ept_dev_path, rpmsg_dev, 32);
    }

	usleep(100000);
    pcontext->fd = open(ept_dev_path, O_RDWR );

    if (pcontext->fd < 0) {
        perror("Failed to open rpmsg device.");
        close(pcontext->charfd);
        return -1;
    }

    //start saf chan
    ret = pthread_create(&t_ipchandle, NULL, slt_ipc_receive_handle, (void*)(pcontext));

    if (ret != 0) {
        printf("Ctreate Thread ERROR %d\n", ret);
        return ret;
    }

    return 0;
}



//------ipc receive-------------


