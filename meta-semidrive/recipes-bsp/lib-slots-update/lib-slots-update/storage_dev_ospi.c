/*
 * SEMIDRIVE Copyright Statement
 * Copyright (c) SEMIDRIVE. All rights reserved
 *
 * This software and all rights therein are owned by SEMIDRIVE, and are
 * protected by copyright law and other relevant laws, regulations and
 * protection. Without SEMIDRIVE's prior written consent and/or related rights,
 * please do not use this software or any potion thereof in any form or by any
 * means. You may not reproduce, modify or distribute this software except in
 * compliance with the License. Unless required by applicable law or agreed to
 * in writing, software distributed under the License is distributed on
 * an "AS IS" basis, WITHOUT WARRANTIES OF ANY KIND, either express or implied.
 *
 * You should have received a copy of the License along with this program.
 * If not, see <http://www.semidrive.com/licenses/>.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <time.h>
#include <fcntl.h>
#include <string.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <linux/types.h>
#include <sys/utsname.h>
#include "system_cfg.h"
#include "storage_device.h"
#include "storage_dev_ospi.h"
#include "crc32.h"

#define RPMSG_CREATE_EPT_IOCTL _IOW(0xb5, 0x1, struct rpmsg_endpoint_info)
#define RPMSG_DESTROY_EPT_IOCTL _IO(0xb5, 0x2)

#define RPMSG_BUS_SYS "/sys/bus/rpmsg"
#define FILE_PATH_LEN 256
#define FILE_PATH_LEN_L 512
#define RECV_BUF_LEN 256
#define CMD_LEN 256
//#define DEBUGMODE 1
#ifndef MIN
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#endif

bool ospi_use_handover = USE_HAND_OVER;
static const char *ota_cmd_str[OTA_CMD_MAX] = {
    [OTA_CMD_START             ] =  "OTA_CMD_START",
    [OTA_CMD_START_OK          ] =  "OTA_CMD_START_OK",
    [OTA_CMD_START_FAIL        ] =  "OTA_CMD_START_FAIL ",
    [OTA_CMD_SEEK              ] =  "OTA_CMD_SEEK",
    [OTA_CMD_SEEK_OK           ] =  "OTA_CMD_SEEK_OK",
    [OTA_CMD_SEEK_FAIL         ] =  "OTA_CMD_SEEK_FAIL",
    [OTA_CMD_READ              ] =  "OTA_CMD_READ",
    [OTA_CMD_READ_OK           ] =  "OTA_CMD_READ_OK",
    [OTA_CMD_READ_FAIL         ] =  "OTA_CMD_READ_FAIL",
    [OTA_CMD_WRITE             ] =  "OTA_CMD_WRITE",
    [OTA_CMD_WRITE_OK          ] =  "OTA_CMD_WRITE_OK",
    [OTA_CMD_WRITE_FAIL        ] =  "OTA_CMD_WRITE_FAIL",
    [OTA_CMD_WRITE_INFO        ] =  "OTA_CMD_WRITE_INFO",
    [OTA_CMD_WRITE_INFO_OK     ] =  "OTA_CMD_WRITE_INFO_OK",
    [OTA_CMD_WRITE_INFO_FAIL   ] =  "OTA_CMD_WRITE_INFO_FAIL",
    [OTA_CMD_GET_CAPACITY      ] =  "OTA_CMD_GET_CAPACITY",
    [OTA_CMD_GET_CAPACITY_OK   ] =  "OTA_CMD_GET_CAPACITY_OK",
    [OTA_CMD_GET_CAPACITY_FAIL ] =  "OTA_CMD_GET_CAPACITY_FAIL ",
    [OTA_CMD_GET_BLOCK         ] =  "OTA_CMD_GET_BLOCK",
    [OTA_CMD_GET_BLOCK_OK      ] =  "OTA_CMD_GET_BLOCK_OK",
    [OTA_CMD_GET_BLOCK_FAIL    ] =  "OTA_CMD_GET_BLOCK_FAIL",
    [OTA_CMD_GET_ERASESIZE     ] =  "OTA_CMD_GET_ERASESIZE",
    [OTA_CMD_GET_ERASESIZE_OK  ] =  "OTA_CMD_GET_ERASESIZE_OK",
    [OTA_CMD_GET_ERASESIZE_FAIL] =  "OTA_CMD_GET_ERASESIZE_FAIL",
    [OTA_CMD_COPY              ] =  "OTA_CMD_COPY",
    [OTA_CMD_COPY_OK           ] =  "OTA_CMD_COPY_OK",
    [OTA_CMD_COPY_FAIL         ] =  "OTA_CMD_COPY_FAIL",
    [OTA_CMD_HANDOVER          ] =  "OTA_CMD_HANDOVER",
    [OTA_CMD_HANDOVER_OK       ] =  "OTA_CMD_HANDOVER_OK",
    [OTA_CMD_HANDOVER_FAIL     ] =  "OTA_CMD_HANDOVER_FAIL",
    [OTA_CMD_CLOSE             ] =  "OTA_CMD_CLOSE",
    [OTA_CMD_CLOSE_OK          ] =  "OTA_CMD_CLOSE_OK",
    [OTA_CMD_CLOSE_FAIL        ] =  "OTA_CMD_CLOSE_FAIL",
};

static const char  *get_ota_cmd_str(ota_cmd_enum num)
{
    return ota_cmd_str[num];
}

#ifdef DEBUGMODE
static void hexdump8(const void *ptr, size_t len)
{
    unsigned long address = (unsigned long)ptr;
    size_t count;
    size_t i;

    for (count = 0 ; count < len; count += 16) {
        for (i = 0; i < MIN(len - count, 16); i++) {
            PRINTF_INFO("%02hhx ", *(const uint8_t *)(address + i));
        }

        PRINTF_INFO("\n");
        address += 16;
    }
}
#endif

static int put_data(uint8_t *target, int offset, const uint8_t *value,
                    size_t size)
{
    for (size_t i = 0; i < size; i++, offset++) {
        *(target + offset) = value[i];
    }

    return offset;
}

static int put_char(uint8_t *target, int offset, uint8_t value)
{
    return put_data(target, offset, &value, sizeof(value));
}

static int rpmsg_create_ept(int rpfd, struct rpmsg_endpoint_info *eptinfo)
{
    int ret;
    ret = ioctl(rpfd, RPMSG_CREATE_EPT_IOCTL, eptinfo);

    if (ret)
        perror("Failed to create endpoint.\n");

    return ret;
}

static char *rpmsg_get_ept_dev_name(const char *rpmsg_char_name,
                                    const char *ept_name, char *ept_dev_name)
{
    char sys_rpmsg_ept_name_path[64] = {0};
    char svc_name[64] = {0};
    char *sys_rpmsg_path = "/sys/class/rpmsg";
    FILE *fp;
    int i;
    int ept_name_len;

    for (i = 0; i < 128; i++) {
        sprintf(sys_rpmsg_ept_name_path, "%s/%s/rpmsg%d/name",
                sys_rpmsg_path, rpmsg_char_name, i);

        PRINTF_INFO("checking %s\n", sys_rpmsg_ept_name_path);

        if (access(sys_rpmsg_ept_name_path, F_OK) < 0)
            continue;

        fp = fopen(sys_rpmsg_ept_name_path, "r");

        if (!fp) {
            PRINTF_CRITICAL("failed to open %s\n", sys_rpmsg_ept_name_path);
            break;
        }

        fgets(svc_name, sizeof(svc_name), fp);
        PRINTF_INFO("svc_name: %s.\n", svc_name);
        fclose(fp);

        ept_name_len = strlen(ept_name);

        if (ept_name_len > sizeof(svc_name))
            ept_name_len = sizeof(svc_name);

        if (!strncmp(svc_name, ept_name, ept_name_len)) {
            sprintf(ept_dev_name, "rpmsg%d", i);
            return ept_dev_name;
        }
    }

    PRINTF_INFO("Not able to RPMsg endpoint file for %s:%s.\n", rpmsg_char_name,
                ept_name);
    return NULL;
}

static int rpmsg_bind_chrdev(const char *rpmsg_dev_name)
{
    char fpath[FILE_PATH_LEN] = {0};
    char *rpmsg_chdrv = "rpmsg_chrdev";
    int fd;
    int ret;

    /* rpmsg dev overrides path */
    sprintf(fpath, "%s/devices/%s/driver_override",
            RPMSG_BUS_SYS, rpmsg_dev_name);
    fd = open(fpath, O_WRONLY);

    if (fd < 0) {
        PRINTF_CRITICAL("Failed to open %s, %s\n", fpath, strerror(errno));
        return -EINVAL;
    }

    ret = write(fd, rpmsg_chdrv, strlen(rpmsg_chdrv) + 1);

    if (ret < 0) {
        PRINTF_CRITICAL("Failed to write %s to %s, %s\n",
                        rpmsg_chdrv, fpath, strerror(errno));
        close(fd);
        return -EINVAL;
    }

    close(fd);
    /* bind the rpmsg device to rpmsg char driver */
    sprintf(fpath, "%s/drivers/%s/bind", RPMSG_BUS_SYS, rpmsg_chdrv);
    fd = open(fpath, O_WRONLY);

    if (fd < 0) {
        PRINTF_CRITICAL("Failed to open %s, %s\n", fpath, strerror(errno));
        return -EINVAL;
    }

    ret = write(fd, rpmsg_dev_name, strlen(rpmsg_dev_name) + 1);

    if (ret < 0) {
        PRINTF_CRITICAL("Failed to write %s to %s, %s\n",
                        rpmsg_dev_name, fpath, strerror(errno));
        PRINTF_CRITICAL("If it's already bind, please unbind with below command, then try again\n");
        PRINTF_CRITICAL("echo %s > %s/drivers/%s/unbind\n", rpmsg_dev_name,
                        RPMSG_BUS_SYS, rpmsg_chdrv);
        close(fd);
        return -EINVAL;
    }

    close(fd);
    return 0;
}

int rpmsg_unbind_chrdev(const char *rpmsg_dev_name)
{
    char fpath[FILE_PATH_LEN] = {0};
    char *rpmsg_chdrv = "rpmsg_chrdev";
    int fd;
    int ret;
    /* unbind the rpmsg device to rpmsg char driver */
    sprintf(fpath, "%s/drivers/%s/unbind", RPMSG_BUS_SYS, rpmsg_chdrv);
    fd = open(fpath, O_WRONLY);

    if (fd < 0) {
        PRINTF_CRITICAL("Failed to open %s, %s\n", fpath, strerror(errno));
        return -EINVAL;
    }

    ret = write(fd, rpmsg_dev_name, strlen(rpmsg_dev_name) + 1);

    if (ret < 0) {
        close(fd);
        return -EINVAL;
    }

    close(fd);
    return 0;
}

static int rpmsg_get_chrdev_fd(const char *rpmsg_dev_name,
                               char *rpmsg_ctrl_name)
{
    char *rpmsg_ctrl_prefix = "rpmsg_ctrl";
    DIR *dir;
    struct dirent *ent;
    int fd;
    char dpath[FILE_PATH_LEN] = {0};
    char fpath[FILE_PATH_LEN_L] = {0};

    sprintf(dpath, "%s/devices/%s/rpmsg", RPMSG_BUS_SYS, rpmsg_dev_name);
    dir = opendir(dpath);

    if (dir == NULL) {
        PRINTF_CRITICAL("Failed to open dir %s\n", dpath);
        return -EINVAL;
    }

    while ((ent = readdir(dir)) != NULL) {
        if (!strncmp(ent->d_name, rpmsg_ctrl_prefix, strlen(rpmsg_ctrl_prefix))) {
            printf("Opening file %s.\n", ent->d_name);
            sprintf(fpath, "/dev/%s", ent->d_name);
            fd = open(fpath, O_RDWR | O_NONBLOCK);

            if (fd < 0) {
                PRINTF_CRITICAL("Failed to open rpmsg char dev %s,%s\n", fpath,
                                strerror(errno));
                return -EINVAL;
            }

            sprintf(rpmsg_ctrl_name, "%s", ent->d_name);
            return fd;
        }
    }

    PRINTF_CRITICAL("No rpmsg char dev file is found\n");
    return -EINVAL;
}


static int install_ospi_driver(char *module_name)
{
    struct utsname name;
    int ret;
    char module_path[FILE_PATH_LEN] = {0};
    char command[CMD_LEN] = {0};
    char *default_name = "cadence-quadspi.ko";
    struct stat sb;

    if (module_name)
        default_name = module_name;

    ret = uname(&name);

    if (ret) {
        PRINTF_CRITICAL("get certain system information failed\n");
        return -1;
    }

    /* try standard kernel module path */
    sprintf(module_path, "/lib/modules/%s/kernel/driver/mtd/spi-nor/%s",
            name.release, default_name);

    ret = stat(module_path, &sb);

    if (ret) {
        /* try local directory */
        ret = stat(default_name, &sb);

        if (ret) {
            PRINTF_INFO("failed to find ospi module file\n");
            return -1;
        }

        sprintf(command, "insmod %s", default_name);
    }
    else {
        sprintf(command, "insmod %s", module_path);
    }

    ret = system(command);

    if (ret == -1 || ret == 127) {
        PRINTF_INFO("failed to execute insmod commond\n");
        return -1;
    }

    return 0;
}

static int send_rpmsg(ota_op_t *op, uint8_t *data, uint16_t data_len)
{
    uint8_t buf[MAX_SEND_LENGTH] = {0};
    uint16_t i = 0;
    int ret = -1;
    ota_msg_head_struct_t *head;
    uint16_t total_len = data_len + MSG_HEAD_SIZE;
    int offset = MSG_HEAD_SIZE;

    if ((NULL == op)) {
        PRINTF_CRITICAL("sendMsg para error\n");
        return ret;
    }

    if ((op->cmd >= OTA_CMD_MAX) || (op->fd < 0)) {
        PRINTF_CRITICAL("sendMsg op error, cmd = %s, fd = %d", get_ota_cmd_str(op->cmd),
                        op->fd);
        return ret;
    }

    if (total_len > MAX_SEND_LENGTH) {
        PRINTF_CRITICAL("sendMsg send length error,cmd = %s length = %d ",
                        get_ota_cmd_str(op->cmd), data_len);
        return ret;
    }

    /* head */
    head = (ota_msg_head_struct_t *)buf;
    head->flag1 = OTA_START_MAGIC;
    head->len = data_len;
    head->cmd = op->cmd;
    head->crc = 0;
    head->flag2 = OTA_END_MAGIC;

    /* data */
    if ((0 != data_len) && (NULL != data)) {
        for (i = 0; i < data_len; i++) {
            offset = put_char(buf, offset, data[i]);
        }
    }

    /* crc */
    head->crc = crc32(0, buf, total_len);

#ifdef DEBUGMODE
    fprintf(stdout, "send msg:\n");
    hexdump8((void *)buf, total_len);
#endif

    ret = write(op->fd, buf, total_len);

    if (ret < 0) {
        PRINTF_CRITICAL("sendMsg write msg error ...\n");
    }

    return ret;
}

static int recv_rpmsg(ota_op_t *op, uint8_t *recv_msg)
{
    struct timeval tv;
    fd_set rfds;
    int fd = op->fd;
    int ret = -1;

    if ((NULL == op) || (NULL == recv_msg)) {
        PRINTF_CRITICAL("recvMsg para error\n");
        return ret;
    }

    if ((op->cmd >= OTA_CMD_MAX) || (op->fd < 0)) {
        PRINTF_CRITICAL("recvMsg op error, cmd = %s, fd = %d", get_ota_cmd_str(op->cmd),
                        op->fd);
        return ret;
    }

    tv.tv_sec =   (op->timeout_ms) / 1000;
    tv.tv_usec = ((op->timeout_ms) % 1000) * 1000;
    FD_ZERO(&rfds);
    FD_SET(fd, &rfds);
    ret = select(fd + 1, &rfds, NULL, NULL, &tv);

    if (ret <= 0) {
        PRINTF_CRITICAL("recvMsg select error\n");
        return -1;
    }
    else {
        ret = read(fd, recv_msg, MAX_SEND_LENGTH);

        if (ret < 0) {
            PRINTF_CRITICAL("recvMsg read error ...\n");
            return ret;
        }
    }

    return ret;
}

static int check_rpmsg(ota_op_t *op, uint8_t *recv_msg)
{
    ota_msg_head_struct_t *head;
    uint16_t total_len;
    uint32_t crc_val;

    if (!recv_msg) {
        PRINTF_CRITICAL("recv_msg para is NULL");
        return -1;
    }

    head = (ota_msg_head_struct_t *)recv_msg;
    total_len = head->len + MSG_HEAD_SIZE;

    if ((total_len > MAX_RECV_LENGTH) || (total_len < MSG_HEAD_SIZE)) {
        PRINTF_CRITICAL("msg len error");
        return -1;
    }

    if (head->flag1 != OTA_START_MAGIC) {
        PRINTF_CRITICAL("flag1 is 0x%04x, expect 0x%04x \n", head->flag1,
                        OTA_START_MAGIC);
        return -1;
    }

    if (head->flag2 != OTA_END_MAGIC) {
        PRINTF_CRITICAL("flag2 is 0x%04x, expect 0x%04x \n", head->flag2,
                        OTA_END_MAGIC);
        return -1;
    }

    crc_val = head->crc;
    head->crc = 0;
    head->crc = crc32(0, recv_msg, total_len);

    if (crc_val != head->crc) {
        PRINTF_CRITICAL("crc = 0x%08x error, expect 0x%08x \n", crc_val, head->crc);
        return -1;
    }

    if (head->cmd != (op->expect_recv_cmd)) {
        PRINTF_CRITICAL("cmd = %s  error, expect %d \n", get_ota_cmd_str(head->cmd),
                        (op->expect_recv_cmd));
        return -1;
    }

    return 0;
}

int rpmsg_init(const char *monitor_channel)
{
    int ret = -1;
    int charfd = -1;
    int fd = -1;
    char rpmsg_char_name[16] = {0};
    char ept_dev_name[16] = {0};
    char ept_dev_path[32] = {0};
    struct rpmsg_endpoint_info eptinfo;
    char fpath[FILE_PATH_LEN] = {0};

    sprintf(fpath, "%s/devices/%s", RPMSG_BUS_SYS, monitor_channel);

    if (access(fpath, F_OK)) {
        PRINTF_CRITICAL("Not able to access rpmsg device %s, %s\n", fpath,
                        strerror(errno));
        goto end;
    }

    /* unbind first */
    rpmsg_unbind_chrdev(monitor_channel);
    /* bind */
    ret = rpmsg_bind_chrdev(monitor_channel);

    if (ret < 0) {
        PRINTF_CRITICAL("Failed to bind rpmsg device %s\n", fpath);
        goto end;
    }

    usleep(100000);
    charfd = rpmsg_get_chrdev_fd(monitor_channel, rpmsg_char_name);

    if (charfd < 0) {
        PRINTF_INFO("failed to get chrdev fd %d\n", charfd);
        goto end;
    }

    /* Create endpoint from rpmsg char driver */
    strcpy(eptinfo.name, "rpmsg-update-monitor-channel");
    eptinfo.src = 81;
    eptinfo.dst = 0xFFFFFFFF;
    ret = rpmsg_create_ept(charfd, &eptinfo);

    if (ret) {
        PRINTF_INFO("failed to create RPMsg endpoint.\n");
        goto end;
    }

    if (!rpmsg_get_ept_dev_name(rpmsg_char_name, eptinfo.name, ept_dev_name)) {
        goto end;
    }

    /* TODO polling /dev/rpmsgX */
    usleep(100000);
    sprintf(ept_dev_path, "/dev/%s", ept_dev_name);
    fd = open(ept_dev_path, O_RDWR);

    if (fd < 0) {
        PRINTF_CRITICAL("Failed to open rpmsg device.\n");
    }

end:

    if (charfd >= 0)
        close(charfd);

    return fd;
}

static int remote_ospi_ops(ota_op_t *op, uint8_t *send_data, uint16_t send_len,
                           uint8_t *recv_data)
{
    int ret = -1;

    /* send msg */
    ret = send_rpmsg(op, send_data, send_len);

    if (ret < 0) {
        PRINTF_CRITICAL("remote_ospi_ops:send msg error, cmd=%s\n",
                        get_ota_cmd_str(op->cmd));
        return -1;
    }

    /* recv msg */
    ret = recv_rpmsg(op, recv_data);

    if (ret < MSG_HEAD_SIZE) {
        PRINTF_CRITICAL("remote_ospi_ops:recv msg error, len=%d , cmd=%s\n", ret,
                        get_ota_cmd_str(op->cmd));
        return -1;
    }

    /* check msg */
    ret = check_rpmsg(op, recv_data);

    if (ret < 0) {
        PRINTF_CRITICAL("remote_ospi_ops:recv msg check error, cmd=%s\n",
                        get_ota_cmd_str(op->cmd));
        return -1;
    }

    return ret;
}

static int remote_ospi_start(int fd)
{
    ota_op_t op;
    int ret = -1;
    uint8_t recv_msg[MAX_SEND_LENGTH] = {0};

    op.fd = fd;
    op.cmd = OTA_CMD_START;
    op.timeout_ms = 5000;
    op.expect_recv_cmd = OTA_CMD_START_OK;
    ret = remote_ospi_ops(&op, NULL, 0, recv_msg);

    if (ret < 0) {
        PRINTF_CRITICAL("remote_ospi_start error \n");
        return -1;
    }

    return 0;
}

static int remote_ospi_close(int fd)
{
    ota_op_t op;
    int ret = -1;
    uint8_t recv_msg[MAX_SEND_LENGTH] = {0};

    op.fd = fd;
    op.cmd = OTA_CMD_CLOSE;
    op.timeout_ms = 5000;
    op.expect_recv_cmd = OTA_CMD_CLOSE_OK;
    ret = remote_ospi_ops(&op, NULL, 0, recv_msg);

    if (ret < 0) {
        PRINTF_CRITICAL("remote_ospi_close error \n");
        return -1;
    }

    return 0;
}

static int remote_ospi_handover(int fd)
{
    ota_op_t op;
    int ret = -1;
    uint8_t recv_msg[MAX_SEND_LENGTH] = {0};

    op.fd = fd;
    op.cmd = OTA_CMD_HANDOVER;
    op.timeout_ms = 5000;
    op.expect_recv_cmd = OTA_CMD_HANDOVER_OK;
    ret = remote_ospi_ops(&op, NULL, 0, recv_msg);

    if (ret < 0) {
        PRINTF_CRITICAL("remote_ospi_handover error \n");
        return -1;
    }

    return 0;
}

int remote_ospi_init(storage_device_t *storage_dev, char const *dev_name,
                     char const *rpmsg_name)
{
    int fd;
    int fd_rpmsg;
    struct stat buf;
    int ret = -1;

    if ((!storage_dev) || (!dev_name)) {
        PRINTF_CRITICAL("remote_ospi_init para error");
        return -1;
    }

    /* rpmsg init */
    fd_rpmsg = rpmsg_init(rpmsg_name);

    if (fd_rpmsg < 0) {
        PRINTF_CRITICAL("init %s device failed\n", rpmsg_name);
        goto unbind;
    }

    if (1 == ospi_use_handover) {
        /* try handover */
        ret = remote_ospi_handover(fd_rpmsg);
    }

    if (0 == ret) {
        PRINTF_INFO("update monitor handover ospi done\n");
        /* install mtdblock device driver */
        ret = install_ospi_driver(NULL);

        if (ret < 0) {
            PRINTF_CRITICAL("failed to install modules\n");
            goto unbind;
        }

        /* waiting for uevent raised */
        usleep(100000);
        ret = stat(dev_name, &buf);

        if (0 != ret) {
            PRINTF_CRITICAL("no such dev file: %s\n", dev_name);
            ret = -1;
            goto unbind;
        }

        fd = open(dev_name, O_RDWR);

        if (fd < 0) {
            PRINTF_CRITICAL("filed to open %s\n", dev_name);
            ret = -1;
            goto unbind;
        }

        storage_dev->dev_fd = fd;
        storage_dev->dev_name = dev_name;
        ret = OSPI_HANDOVER_MODE;

    }
    else {
        ret = remote_ospi_start(fd_rpmsg);

        if (ret < 0) {
            PRINTF_CRITICAL("failed to start communicate with update monitor\n");
            goto unbind;
        }

        PRINTF_INFO("ospi start \n");
        storage_dev->dev_fd = fd_rpmsg;
        storage_dev->dev_name = rpmsg_name;
        ret = OSPI_REMOTE_MODE;
        goto end;
    }

unbind:

    /* unbind rgmsg */
    if (rpmsg_unbind_chrdev(rpmsg_name) < 0) {
        PRINTF_CRITICAL("Failed to unbind rpmsg device %s\n", rpmsg_name);
    }

    if (fd_rpmsg >= 0)
        close(fd_rpmsg);

end:
    return ret;
}

static int remote_ospi_seek(int fd, off_t offset, int whence)
{
    ota_op_t op;
    int ret = -1;
    uint8_t recv_msg[MAX_SEND_LENGTH] = {0};

    op.fd = fd;
    op.cmd = OTA_CMD_SEEK;
    op.timeout_ms = 5000;
    op.expect_recv_cmd = OTA_CMD_SEEK_OK;
    ret = remote_ospi_ops(&op, (uint8_t *)&offset, sizeof(offset), recv_msg);

    if (ret < 0) {
        PRINTF_CRITICAL("remote_ospi_seek error \n");
        return -1;
    }

    return 0;
}

int remote_ospi_read(storage_device_t *storage_dev, uint64_t src, uint8_t *dst,
                     uint64_t size)
{
    ota_op_t op;
    int ret = -1;
    uint64_t cnt = 0;
    uint64_t i = 0;
    uint16_t  left = 0;
    uint16_t read_size = MAX_DATA_LENGTH;
    uint8_t  recv_msg[MAX_RECV_LENGTH] = {0};
    uint8_t *out = dst;
    ota_msg_head_struct_t *head;

    if ((!storage_dev) || (!dst)) {
        PRINTF_CRITICAL("remote_ospi_read para error");
        return -1;
    }

    if (remote_ospi_seek(storage_dev->dev_fd, src, SEEK_SET) < 0) {
        PRINTF_CRITICAL("remote_ospi_read:seek error\n");
        return -1;
    }

    op.fd = storage_dev->dev_fd;
    op.cmd = OTA_CMD_READ;
    op.timeout_ms = 5000;
    op.expect_recv_cmd = OTA_CMD_READ_OK;
    cnt = size / MAX_DATA_LENGTH;
    left = size % MAX_DATA_LENGTH;

    for (i = 0; i < cnt + 1; i++) {
#ifdef DEBUGMODE
        PRINTF_CRITICAL("remote_ospi_read: cnt=%lu\n", cnt);
#endif

        if (i == cnt) {
            if (left) {
                read_size = left;
            }
            else
                break;
        }

        ret = remote_ospi_ops(&op, (uint8_t *)&read_size, sizeof(read_size), recv_msg);

        if (ret < 0) {
            PRINTF_CRITICAL("remote_ospi_read error \n");
            return -1;
        }

        /* len check */
        head = (ota_msg_head_struct_t *)recv_msg;

        if (head->len != read_size) {
            PRINTF_CRITICAL("remote_ospi_read:recv len is %d, not %d  \n", head->len,
                            read_size);
            return -1;
        }

        /* copy the read data */
        memcpy(out, recv_msg + MSG_HEAD_SIZE, read_size);
        out = out + read_size;
    }

    return 0;
}

static int remote_ospi_write_info(int fd, uint64_t data_len)
{
    ota_op_t op;
    int ret = -1;
    uint8_t recv_msg[MAX_SEND_LENGTH] = {0};

    op.fd = fd;
    op.cmd = OTA_CMD_WRITE_INFO;
    op.timeout_ms = 5000;
    op.expect_recv_cmd = OTA_CMD_WRITE_INFO_OK;
    ret = remote_ospi_ops(&op, (uint8_t *)&data_len, sizeof(data_len), recv_msg);

    if (ret < 0) {
        PRINTF_CRITICAL("remote_ospi_write_info error \n");
        return -1;
    }

    return 0;
}

int remote_ospi_write(storage_device_t *storage_dev, uint64_t dst,
                      const uint8_t *buf, uint64_t data_len)
{
    ota_op_t op;
    int ret = -1;
    uint64_t cnt = 0;
    uint64_t i = 0;
    uint16_t  left = 0;
    uint16_t write_size = MAX_DATA_LENGTH;
    uint8_t  recv_msg[MAX_SEND_LENGTH] = {0};
    uint8_t *in = (uint8_t *)buf;

    if ((!storage_dev) || (!buf)) {
        PRINTF_CRITICAL("remote_ospi_write para error");
        return -1;
    }

    /* seek */
    if (remote_ospi_seek(storage_dev->dev_fd, dst, SEEK_SET) < 0) {
        PRINTF_CRITICAL("remote_ospi_write:seek error\n");
        return -1;
    }

    /* write info */
    if (remote_ospi_write_info(storage_dev->dev_fd, data_len) < 0) {
        PRINTF_CRITICAL("remote_ospi_write: set info error\n");
        return -1;
    }

    op.fd = storage_dev->dev_fd;
    op.cmd = OTA_CMD_WRITE;
    op.timeout_ms = 5000;
    op.expect_recv_cmd = OTA_CMD_WRITE_OK;
    cnt  = data_len / MAX_DATA_LENGTH;
    left = data_len % MAX_DATA_LENGTH;

    for (i = 0; i < cnt + 1; i++) {
        if (i == cnt) {
            if (left)
                write_size = left;
            else
                break;
        }

#ifdef DEBUGMODE
        PRINTF_CRITICAL("remote_ospi_write: i=%lu, cnt=%lu\n", i, cnt);
        PRINTF_CRITICAL("write_size=%d\n", write_size);
        PRINTF_CRITICAL("write_size=%d\n", left);
#endif

        ret = remote_ospi_ops(&op, in, write_size, recv_msg);

        if (ret < 0) {
            PRINTF_CRITICAL("remote_ospi_write error \n");
            return -1;
        }

        /* increase the data source */
        in = in + write_size;
    }

    return 0;
}

uint64_t remote_ospi_get_capacity(storage_device_t *storage_dev)
{
    uint64_t capacity;
    ota_op_t op;
    int ret = -1;
    uint8_t recv_msg[MAX_SEND_LENGTH] = {0};
    ota_msg_head_struct_t *head;

    if (!storage_dev) {
        PRINTF_CRITICAL("remote_ospi_get_capacity para error");
        return 0;
    }

    op.fd = storage_dev->dev_fd;
    op.cmd = OTA_CMD_GET_CAPACITY;
    op.timeout_ms = 5000;
    op.expect_recv_cmd = OTA_CMD_GET_CAPACITY_OK;

    ret = remote_ospi_ops(&op, NULL, 0, recv_msg);

    if (ret < 0) {
        PRINTF_CRITICAL("remote_ospi_get_capacity error \n");
        return 0;
    }

    /* len check */
    head = (ota_msg_head_struct_t *)recv_msg;

    if (head->len != sizeof(capacity)) {
        PRINTF_CRITICAL("remote_ospi_get_capacity:recv len is %d, not %u  \n",
                        head->len, (uint32_t)sizeof(capacity));
        return 0;
    }

    /* copy the capacity */
    memcpy(&capacity, recv_msg + MSG_HEAD_SIZE, sizeof(capacity));
    return capacity;
}

uint32_t remote_ospi_get_erase_size(storage_device_t *storage_dev)
{
    uint32_t erase_size;
    ota_op_t op;
    int ret = -1;
    uint8_t recv_msg[MAX_SEND_LENGTH] = {0};
    ota_msg_head_struct_t *head;

    if (!storage_dev) {
        PRINTF_CRITICAL("remote_ospi_get_erase_size para error");
        return 0;
    }

    op.fd = storage_dev->dev_fd;
    op.cmd = OTA_CMD_GET_ERASESIZE;
    op.timeout_ms = 5000;
    op.expect_recv_cmd = OTA_CMD_GET_ERASESIZE_OK;

    ret = remote_ospi_ops(&op, NULL, 0, recv_msg);

    if (ret < 0) {
        PRINTF_CRITICAL("remote_ospi_get_erase_size error \n");
        return 0;
    }

    /* len check */
    head = (ota_msg_head_struct_t *)recv_msg;

    if (head->len != sizeof(erase_size)) {
        PRINTF_CRITICAL("remote_get_erase_size:recv len is %d, not %u \n", head->len,
                        (uint32_t)sizeof(erase_size));
        return 0;
    }

    /* copy the erase_size */
    memcpy(&erase_size, recv_msg + MSG_HEAD_SIZE, sizeof(erase_size));
    return erase_size;
}

uint32_t remote_ospi_get_block_size(storage_device_t *storage_dev)
{
    uint32_t block_size;
    ota_op_t op;
    int ret = -1;
    uint8_t recv_msg[MAX_SEND_LENGTH] = {0};
    ota_msg_head_struct_t *head;

    if (!storage_dev) {
        PRINTF_CRITICAL("remote_ospi_get_block_size para error");
        return 0;
    }

    op.fd = storage_dev->dev_fd;
    op.cmd = OTA_CMD_GET_BLOCK;
    op.timeout_ms = 5000;
    op.expect_recv_cmd = OTA_CMD_GET_BLOCK_OK;

    ret = remote_ospi_ops(&op, NULL, 0, recv_msg);

    if (ret < 0) {
        PRINTF_CRITICAL("remote_ospi_get_block_size error \n");
        return 0;
    }

    /* len check */
    head = (ota_msg_head_struct_t *)recv_msg;

    if (head->len != sizeof(block_size)) {
        PRINTF_CRITICAL("remote_get_block_size:recv len is %d, not %u  \n", head->len,
                        (uint32_t)sizeof(block_size));
        return 0;
    }

    /* copy the block_size */
    memcpy(&block_size, recv_msg + MSG_HEAD_SIZE, sizeof(block_size));
    return block_size;
}

int remote_ospi_release(storage_device_t *storage_dev)
{
    if (!storage_dev) {
        PRINTF_CRITICAL("remote_ospi_release para error");
        return -1;
    }

    if (storage_dev->dev_fd >= 0) {
        remote_ospi_close(storage_dev->dev_fd);
        close(storage_dev->dev_fd);
    }

    return 0;
}

int remote_ospi_copy(storage_device_t *storage_dev, uint64_t src, uint64_t dst,
                     uint64_t size)
{
    ota_op_t op;
    int ret = -1;
    uint8_t  send_data[sizeof(src) + sizeof(dst) + sizeof(size)];
    uint8_t  recv_msg[MAX_RECV_LENGTH] = {0};
    int offset = 0;

    if (!storage_dev) {
        PRINTF_CRITICAL("remote_ospi_copy para error");
        return -1;
    }

    op.fd = storage_dev->dev_fd;
    op.cmd = OTA_CMD_COPY;
    op.timeout_ms = 500000;
    op.expect_recv_cmd = OTA_CMD_COPY_OK;
    offset = put_data(send_data, offset, (uint8_t *)&src, sizeof(src));
    offset = put_data(send_data, offset, (uint8_t *)&dst, sizeof(dst));
    offset = put_data(send_data, offset, (uint8_t *)&size, sizeof(size));

    ret = remote_ospi_ops(&op, send_data, offset, recv_msg);

    if (ret < 0) {
        PRINTF_CRITICAL("remote_ospi_copy error \n");
        return -1;
    }

    return 0;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */
