/*
 * func_emmc.c
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
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/ioctl.h>
#include <linux/fs.h>
#include <linux/ioctl.h>
#include <openssl/md5.h>

#include "board_diag.h"
#include "debug.h"

#define EMMC_BLK_PART_DEV     "/dev/mmcblk0p"
#define EMMC_BLK_SYS_PATH     "/sys/class/block/mmcblk0"
#define EMMC_BLK_PART_PREFIX  "mmcblk0p"
#define PART_NAME_KEY         "PARTNAME="
#define EMMC_BLK_RESVERD_PART "reserved"
#define EMMC_BLK_USERDAT_PART "userdata"

#define EMMC_BLOCK_SIZE       512         //512 byte
#define EMMC_BLOCK_SIZE_L     256*512
#define EMMC_BLOCK_SIZE_H     1024*1024*1 //1M*x byte
#define URAMDOM               "/dev/urandom"

#define READ_OPS  0
#define WRITE_OPS 1
#define CHECK_OPS 2

typedef enum {
    low  = 0,
    high = 1
} block_level_e;

static uint8_t emmc_buf_l[EMMC_BLOCK_SIZE_L];
static uint8_t emmc_buf_h[EMMC_BLOCK_SIZE_H];

typedef struct {
    uint8_t num;
    char *name;
    block_level_e level;
    uint32_t rate;
} emmc_blk_path_t;

const static emmc_blk_path_t emmc_blk_path[] = {
    {0x0, "hsm_fw",     low,  1},  //256*512
    {0x1, "preloader",  high, 1},  //4096*512
    {0x2, "vbmeta",     low,  1},  //2048*512
    {0x3, "reserved",   high, 1},  //4096*512
    {0x4, "bootloader", low,  1},  //2048*512
    {0x5, "dtb",        low,  1},  //1024*512
    {0x6, "kernel",     high, 1},  //57344*512
    {0x7, "userdata",   high, 10}, //123780502*512
    {0x8, "null",       low,  1}   //null
};

static char *find_test_block_part(char *block_part_name, uint32_t len,
                                  char *blk_part)
{
    FILE *fp = NULL;
    DIR *dir = NULL;
    int size = 0;
    bool found = false;
    char fpath[256] = {0};
    char part_name[100];
    struct dirent *ent;

    dir = opendir(EMMC_BLK_SYS_PATH);

    if (dir == NULL) {
        ERROR("Failed to open dir %s\n", EMMC_BLK_SYS_PATH);
        return NULL;
    }

    while ((ent = readdir(dir)) != NULL) {
        if (!strncmp(ent->d_name, EMMC_BLK_PART_PREFIX,
                     strlen(EMMC_BLK_PART_PREFIX))) {
            size = snprintf(fpath, sizeof(fpath), "%s/%s/uevent", EMMC_BLK_SYS_PATH,
                            ent->d_name);

            if (size <= 0)
                continue;

            fp = fopen(fpath, "r");

            if (fp == NULL)
                continue;

            while (fgets(part_name, sizeof(part_name), fp)) {
                if (!strncmp(part_name, PART_NAME_KEY, strlen(PART_NAME_KEY))
                        && strstr(part_name, blk_part)) {
                    memset(block_part_name, 0x0, len);
                    strncpy(block_part_name, ent->d_name, len - 1);
                    found = true;
                    break;
                }
            }

            fclose(fp);
        }
    }

    closedir(dir);
    return found ? block_part_name : NULL;
}

static int open_block(uint64_t *size, char *blk_part)
{
    int fd = -1;
    char fpath[256];
    char block_dev[32];
    uint64_t part_size_in_blk = 0;

    if (!find_test_block_part(block_dev, sizeof(block_dev), blk_part)) {
        ERROR("cann't find reserved partition\n");
        goto fail;
    }

    snprintf(fpath, sizeof(fpath), "/dev/%s", block_dev);
    fd = open(fpath, O_RDWR);

    if (fd < 0) {
        ERROR("open file fail!\n");
        goto fail;
    }

    if (ioctl(fd, BLKGETSIZE, &part_size_in_blk)) {
        ERROR("blk ioctl fail!\n");
        goto fail;
    }

    *size = part_size_in_blk;
    return fd;
fail:

    *size = 0;

    if (fd >= 0)
        close(fd);

    return -1;
}
#if 0
static uint32_t umount_userdata_block(void)
{
    pid_t status;
    uint32_t ret = -1;
    static bool exec_flg = false;

    if (exec_flg == false) {
        status = system("umount /dev/mmcblk0p10");

        if (status < 0) {
            ERROR("umount rootfs fail!\n");
            goto out;
        }
        else {
            if (WIFEXITED(status)) {
                if (WEXITSTATUS(status) != 0) {
                    ERROR("WEXITSTATUS error!\n");
                    goto out;
                }
            }
            else {
                ERROR("WIFEXITED error!\n");
                goto out;
            }
        }

        exec_flg = true;
    }

    ret = 1;

out:
    return ret;
}
#endif
static bool check_emmc(test_exec_t *exec)
{
    bool ret = false;
    int fd_blk = -1;
    uint64_t lenx;
    uint64_t len;
    MD5_CTX md5_ctx;
    int fd_random = -1;
    uint8_t *emmc_buf = NULL;
    uint64_t origin_size = 0;
    uint64_t part_size = 0;
    uint64_t part_size_in_blk = 0;
    CMD_STATUS cmdStatus = CMD_PARA_ERR;
    uint8_t md5_random[MD5_DIGEST_LENGTH];
    uint8_t md5_read[MD5_DIGEST_LENGTH];
    uint8_t sub_cmd = exec->cmd[0];
    uint8_t block_start_nr;
    uint8_t block_end_nr;

    if (sub_cmd == SUBCMD_EMMC) {
        /*without userdata partion*/
        block_start_nr = emmc_blk_path[3].num;
        block_end_nr = emmc_blk_path[4].num;
    }
    else if (sub_cmd == SUBCMD_STORE_STRESS) {
        /*all partion*/
        block_start_nr = emmc_blk_path[0].num;
        block_end_nr = emmc_blk_path[8].num;
    }
    else {
        ERROR("no block_count!\n");
        goto out;
    }

#if 0

    if (umount_userdata_block() < 0) {
        ERROR("umount_userdata_block!\n");
        goto out;
    }

#endif

    for (uint8_t i = block_start_nr; i < block_end_nr; i++) {
        if (emmc_blk_path[i].level == low) {
            emmc_buf = emmc_buf_l;
            len = sizeof(emmc_buf_l);
        }
        else if (emmc_blk_path[i].level == high) {
            emmc_buf = emmc_buf_h;
            len = sizeof(emmc_buf_h);
        }
        else {
            ERROR("no block size!\n");
            goto out;
        }

        fd_blk = open_block(&part_size_in_blk, emmc_blk_path[i].name);
        fd_random = open(URAMDOM, O_RDONLY);

        if (fd_random < 0 || fd_blk < 0) {
            ERROR("open file  fail!\n");
            goto out;
        }

        part_size = (part_size_in_blk * EMMC_BLOCK_SIZE) / len;
        part_size = part_size / emmc_blk_path[i].rate;
        origin_size = part_size;

        MD5_Init(&md5_ctx);

        while (part_size) {
            lenx = read(fd_random, emmc_buf, len);

            if ( len != lenx) {
                ERROR("size:%ld read urandom fail, error:%s!\n", lenx, strerror(errno));
                break;
            }

            MD5_Update(&md5_ctx, emmc_buf, len);

            if (write(fd_blk, emmc_buf, len) != lenx) {
                ERROR("write partition fail!\n");
                break;
            }

            part_size--;
        }

        MD5_Final(md5_random, &md5_ctx);

        ioctl(fd_blk, BLKFLSBUF, NULL);
        lseek(fd_blk, 0, SEEK_SET);

        part_size = origin_size;
        MD5_Init(&md5_ctx);

        while (part_size) {
            memset(emmc_buf, 0x0, len);
            lenx = read(fd_blk, emmc_buf, len);

            if ( len != lenx) {
                hexdump8(emmc_buf, 16);
                ERROR(" size:%ld read blk fail, error:%s! errno:%d\n",
                      lenx, strerror(errno), errno);
                break;
            }

            MD5_Update(&md5_ctx, emmc_buf, len);
            part_size--;
        }

        MD5_Final(md5_read, &md5_ctx);

        if (memcmp(md5_random, md5_read, MD5_DIGEST_LENGTH)) {
            ERROR("md5 check fail!\n");
            goto out;
        }

        ioctl(fd_blk, BLKFLSBUF, NULL);
    }

    set_para_value(ret, true);
    set_para_value(cmdStatus, NORMAL_DEAL);

out:

    if (fd_random >= 0)
        close(fd_random);

    if (fd_blk >= 0)
        close(fd_blk);

    set_para_value(exec->resp[0], cmdStatus);
    return ret;
}
#if 0
static bool read_emmc(uint8_t size)
{
    bool ret = false;
    ssize_t len = 0;
    int fd_blk = -1;
    uint8_t *buf = NULL;
    uint64_t part_size_in_blk = 0;

    fd_blk = open_block(&part_size_in_blk);

    if (fd_blk < 0) {
        ERROR("open file fail!\n");
        goto out;
    }

    buf = calloc(1, EMMC_BLOCK_SIZE);

    if (buf == NULL) {
        ERROR("allocate memory fail!\n");
        goto out;
    }

    len = read(fd_blk, buf, size);

    if ( len != size) {
        ERROR("size:%ld read urandom fail, error:%s!\n", len, strerror(errno));
        goto out;
    }

    DBG("read %d bytes data from emmc\n", size);
    ret = true;
out:

    if (fd_blk >= 0)
        close(fd_blk);

    if (buf)
        free(buf);

    return ret;
}
#endif
#if 0
static bool write_emmc(uint8_t size)
{
    bool ret = false;
    ssize_t len = 0;
    int fd_blk = -1;
    uint8_t *buf = NULL;
    uint64_t part_size_in_blk = 0;

    fd_blk = open_block(&part_size_in_blk);

    if (fd_blk < 0) {
        ERROR("open file fail!\n");
        goto out;
    }

    buf = calloc(1, EMMC_BLOCK_SIZE);

    if (buf == NULL) {
        ERROR("allocate memory fail!\n");
        goto out;
    }

    memset(buf, 0x1, size);
    len = write(fd_blk, buf, size);

    if ( len != size) {
        ERROR("size:%ld write emmc fail, error:%s!\n", len, strerror(errno));
        goto out;
    }

    DBG("write %d bytes data into emmc\n", size);
    ret = true;
out:

    if (fd_blk >= 0)
        close(fd_blk);

    if (buf)
        free(buf);

    return ret;
}
#endif
static bool process_cmd(test_exec_t *exec)
{
    return check_emmc(exec);
}

bool emmc_reply_deal(test_exec_t *exec, test_state_e state)
{
    bool ret = false;
    uint32_t respCanID;

    set_para_value(exec->resp[0], CMD_PARA_ERR);

    memset(exec->resp, 0x0, TEST_RESP_LEN);

    if (state == STATE_SINGLE) {
        respCanID = SINGLE_RESP;
        ret = process_cmd(exec);
    }
    else {
        return ret;
    }

    if (ret)
        set_para_value(exec->resp[0], NORMAL_DEAL);

    common_response(exec, respCanID) ? (ret = true) : (ret = false);
    return ret;
}
