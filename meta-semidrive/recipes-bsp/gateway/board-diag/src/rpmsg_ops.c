/*
 * rpmsg-ops.c
 *
 * Copyright (c) 2020 SemiDrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */

#include <dirent.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <string.h>

#include <sys/ioctl.h>
#include <linux/rpmsg.h>

#include "debug.h"

#define RPMSG_BUS_SYS     "/sys/bus/rpmsg"
#define RPMSG_CLASS_PATH  "/sys/class/rpmsg"

#define RPMSG_CHDRV       "rpmsg_chrdev"
#define RPMSG_CTRL_PREFIX "rpmsg_ctrl"

#define RPMSG_EP_MAX_NUM  128
#define PATH_MAX_LEN      256

static char rpmsg_dev[PATH_MAX_LEN];

static int rpmsg_create_ept(int rpfd, struct rpmsg_endpoint_info *eptinfo)
{
    int ret;

    ret = ioctl(rpfd, RPMSG_CREATE_EPT_IOCTL, eptinfo);

    if (ret)
        perror("Failed to create endpoint.\n");

    return ret;
}

static int rpmsg_destory_ept(int ep_fd)
{
    int ret;

    ret = ioctl(ep_fd, RPMSG_DESTROY_EPT_IOCTL);

    if (ret)
        perror("Failed to create endpoint.\n");

    return ret;
}

static int bind_rpmsg_chrdev(const char *rpmsg_dev_name)
{
    char fpath[PATH_MAX_LEN];
    int fd;
    int ret;

    if (!rpmsg_dev_name || strlen(rpmsg_dev_name) == 0)
        return -1;

    /* rpmsg dev overrides path */
    sprintf(fpath, "%s/devices/%s/driver_override",
            RPMSG_BUS_SYS, rpmsg_dev_name);
    fd = open(fpath, O_WRONLY);

    if (fd < 0) {
        ERROR("Failed to open %s, %s\n", fpath, strerror(errno));
        return -EINVAL;
    }

    ret = write(fd, RPMSG_CHDRV, strlen(RPMSG_CHDRV) + 1);

    if (ret < 0) {
        ERROR("Failed to write %s to %s, %s\n",
              RPMSG_CHDRV, fpath, strerror(errno));
        return -EINVAL;
    }

    close(fd);

    /* bind the rpmsg device to rpmsg char driver */
    sprintf(fpath, "%s/drivers/%s/bind", RPMSG_BUS_SYS, RPMSG_CHDRV);
    fd = open(fpath, O_WRONLY);

    if (fd < 0) {
        ERROR("Failed to open %s, %s\n", fpath, strerror(errno));
        return -EINVAL;
    }

    ret = write(fd, rpmsg_dev_name, strlen(rpmsg_dev_name) + 1);

    if (ret < 0) {
        ERROR("Failed to write %s to %s, %s\n",
              rpmsg_dev_name, fpath, strerror(errno));
        return -EINVAL;
    }

    close(fd);
    return 0;
}

static int unbind_rpmsg_chrdev(const char *rpmsg_dev_name)
{
    int fd;
    int ret;
    char fpath[PATH_MAX_LEN];

    if (!rpmsg_dev_name)
        return -EINVAL;

    /* unbind the rpmsg device to rpmsg char driver */
    sprintf(fpath, "%s/drivers/%s/unbind", RPMSG_BUS_SYS, RPMSG_CHDRV);
    fd = open(fpath, O_WRONLY);

    if (fd < 0) {
        ERROR("Failed to open %s, %s\n", fpath, strerror(errno));
        return -EINVAL;
    }

    ret = write(fd, rpmsg_dev_name, strlen(rpmsg_dev_name) + 1);

    if (ret < 0) {
        ERROR("Failed to write %s to %s, %s\n",
              rpmsg_dev_name, fpath, strerror(errno));
        return -EINVAL;
    }

    close(fd);
    return 0;
}

static int get_rpmsg_chrdev_fd(const char *rpmsg_dev_name,
                               char *rpmsg_ctrl_name)
{
    DIR *dir;
    int fd = -1;
    int path_len = 0;
    bool found = false;
    struct dirent *ent;
    char dpath[PATH_MAX_LEN];
    char fpath[PATH_MAX_LEN];

    path_len = snprintf(dpath, sizeof(dpath), "%s/devices/%s/rpmsg",
                        RPMSG_BUS_SYS, rpmsg_dev_name);

    if (path_len <= 0)
        return -EINVAL;

    dir = opendir(dpath);

    if (dir == NULL) {
        ERROR("Failed to open dir %s\n", dpath);
        return -EINVAL;
    }

    while ((ent = readdir(dir)) != NULL) {
        if (!strncmp(ent->d_name, RPMSG_CTRL_PREFIX,
                     strlen(RPMSG_CTRL_PREFIX))) {
            DBG("Opening file %s.\n", ent->d_name);
            path_len = snprintf(fpath, sizeof(fpath), "/dev/%s", ent->d_name);

            if (path_len <= 0)
                goto out;

            fd = open(fpath, O_RDWR | O_NONBLOCK);
            found = true;

            if (fd < 0) {
                ERROR("Failed to open rpmsg char dev %s,%s\n",
                      fpath, strerror(errno));
                goto out;
            }

            sprintf(rpmsg_ctrl_name, "%s", ent->d_name);
            goto out;
        }
    }

out:

    return found ? fd : -EINVAL;
}

static char *get_rpmsg_ept_dev_name(const char *rpmsg_char_name,
                                    const char *ept_name,
                                    char *ept_dev_name)
{
    int i;
    FILE *fp;
    char svc_name[64];
    unsigned long ept_name_len;
    char sys_rpmsg_ept_name_path[64];

    for (i = 0; i < RPMSG_EP_MAX_NUM; i++) {
        sprintf(sys_rpmsg_ept_name_path, "%s/%s/rpmsg%d/name",
                RPMSG_CLASS_PATH, rpmsg_char_name, i);
        DBG("checking %s\n", sys_rpmsg_ept_name_path);

        if (access(sys_rpmsg_ept_name_path, F_OK) < 0)
            continue;

        fp = fopen(sys_rpmsg_ept_name_path, "r");

        if (!fp) {
            ERROR("failed to open %s\n", sys_rpmsg_ept_name_path);
            break;
        }

        fgets(svc_name, sizeof(svc_name), fp);
        fclose(fp);
        DBG("svc_name: %s\n", svc_name);
        ept_name_len = strlen(ept_name);

        if (ept_name_len > sizeof(svc_name))
            ept_name_len = sizeof(svc_name);

        if (!strncmp(svc_name, ept_name, ept_name_len)) {
            sprintf(ept_dev_name, "rpmsg%d", i);
            return ept_dev_name;
        }
    }

    DBG("Not able to RPMsg endpoint file for %s:%s.\n",
        rpmsg_char_name, ept_name);
    return NULL;
}

static bool rpmsg_find_rpmsg_dev(int rpmsg_ept)
{
    DIR *dir;
    struct dirent *ptr;
    char array[128];
    bool find = false;

    sprintf(array, "%s/devices/", RPMSG_BUS_SYS);

    if ((dir = opendir(array)) == NULL) {
        printf("\r\n Open rpmsg dev dir fail\r\n");
        return NULL;
    }

    sprintf(array, "%d", rpmsg_ept);

    while ((ptr = readdir(dir)) != NULL) {
        if (strcmp(ptr->d_name, ".") == 0 || strcmp(ptr->d_name, "..") == 0)
            continue;

        if (strstr(ptr->d_name, "sdpe-eth-channel") &&
                strstr(ptr->d_name, array)) {
            printf("find eth channel:%s\n", ptr->d_name);
            sprintf(rpmsg_dev, "%s", ptr->d_name);
            find = true;
            break;
        }
    }

    closedir(dir);
    return find;
}

int open_rpmsg_ep(int ep_num, bool block)
{
    char ept_dev_name[16];
    char ept_dev_path[32];
    u_int32_t ops_flag = 0;
    char fpath[PATH_MAX_LEN];
    char rpmsg_char_name[16];
    int ret = -1, char_fd, rpmsg_fd;
    struct rpmsg_endpoint_info eptinfo;

    eptinfo.dst = 0xFFFFFFFF;

    if (ep_num == 1025) {
        eptinfo.src = 1025;
        strcpy(eptinfo.name, "sdpe-eth-linux");

    }
    else { /* use default ept */
        eptinfo.src = 1024;
        strcpy(eptinfo.name, "sdpe-eth-linux-1");
    }

    if (!rpmsg_find_rpmsg_dev(eptinfo.src)) {
        printf("\r\n Cannot find rpmsg dev \r\n");
        return -EINVAL;
    }

    printf("\r\n Open rpmsg dev %s! \r\n", rpmsg_dev);
    sprintf(fpath, "%s/devices/%s", RPMSG_BUS_SYS, rpmsg_dev);

    if (access(fpath, F_OK)) {
        ERROR("Not able to access rpmsg device %s, %s\n",
              fpath, strerror(errno));
        return -EINVAL;
    }

    ret = bind_rpmsg_chrdev(rpmsg_dev);

    if (ret < 0)
        return ret;

    char_fd = get_rpmsg_chrdev_fd(rpmsg_dev, rpmsg_char_name);

    if (char_fd < 0)
        return char_fd;

    ret = rpmsg_create_ept(char_fd, &eptinfo);

    if (ret) {
        ERROR("failed to create RPMsg endpoint.\n");
        return -EINVAL;
    }

    if (!get_rpmsg_ept_dev_name(rpmsg_char_name, eptinfo.name,
                                ept_dev_name))
        return -EINVAL;

    if (!block)
        ops_flag = O_NONBLOCK;

    sprintf(ept_dev_path, "/dev/%s", ept_dev_name);
    rpmsg_fd = open(ept_dev_path, O_RDWR | ops_flag);

    if (rpmsg_fd < 0) {
        perror("Failed to open rpmsg device.");
        close(char_fd);
        return -1;
    }

    return rpmsg_fd;
}

int close_rpmsg_ep(int ep_fd)
{
    rpmsg_destory_ept(ep_fd);
    close(ep_fd);
    unbind_rpmsg_chrdev(rpmsg_dev);
    return 0;
}
