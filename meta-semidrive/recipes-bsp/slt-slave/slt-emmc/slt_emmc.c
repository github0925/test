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
#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <fcntl.h>
#include <fcntl.h>
#include <linux/fs.h>
#include <linux/ioctl.h>
#include <openssl/md5.h>
#include <stdbool.h>
#include <sys/ioctl.h>

#define EMMC_BLK_PART_DEV     "/dev/mmcblk0p"
#define EMMC_BLK_SYS_PATH     "/sys/class/block/mmcblk0"
#define EMMC_BLK_PART_PREFIX  "mmcblk0p"
#define EMMC_BLK_RESVERD_PART "reserved"
#define PART_NAME_KEY         "PARTNAME="
#define EMMC_BLOCK_SIZE       512
#define URAMDOM               "/dev/urandom"

#define READ_OPS  0
#define WRITE_OPS 1

#define ERROR(format, args...) printf("ERROR:%s %d "format, __func__, __LINE__, ##args)
#define DBG(format, args...) //fprintf(stdout, "DEBUG:%s %d "format, __func__, __LINE__, ##args)

static const char testcase_identifier[] = "[TEST_CPU2_SS_01]";

static char *find_reserved_part(char *block_part_name, uint32_t len)
{
    FILE *fp;
    DIR *dir;
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

            if (!fp)
                continue;

            while (fgets(part_name, sizeof(part_name), fp)) {
                if (!strncmp(part_name, PART_NAME_KEY, strlen(PART_NAME_KEY))
                        && strstr(part_name, EMMC_BLK_RESVERD_PART)) {
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

static int open_block(uint64_t *size)
{
    int fd = -1;
    char fpath[256];
    char block_dev[32];
    uint64_t part_size_in_blk = 0;

    if (!find_reserved_part(block_dev, sizeof(block_dev))) {
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

    if (fd > 0)
        close(fd);

    *size = 0;
    return -1;
}

static bool check_emmc(void)
{
    bool ret = false;
    int fd_blk = -1;
    ssize_t len = 0;
    MD5_CTX md5_ctx;
    int fd_random = -1;
    uint8_t *buf = NULL;
    uint64_t origin_size = 0;
    uint64_t part_size_in_blk = 0;
    uint8_t md5_random[MD5_DIGEST_LENGTH];
    uint8_t md5_read[MD5_DIGEST_LENGTH];

    buf = calloc(1, EMMC_BLOCK_SIZE);

    if (!buf) {
        ERROR("allocate memory fail!\n");
        goto out;
    }

    fd_blk = open_block(&part_size_in_blk);
    fd_random = open(URAMDOM, O_RDONLY);

    if (fd_random < 0 || fd_blk < 0) {
        ERROR("open file fail!\n");
        goto out;
    }

    origin_size = part_size_in_blk;
    MD5_Init(&md5_ctx);

    while (part_size_in_blk) {
        len = read(fd_random, buf, EMMC_BLOCK_SIZE);

        if ( len != EMMC_BLOCK_SIZE) {
            ERROR("size:%ld read urandom fail, error:%s!\n", len, strerror(errno));
            goto out;
        }

        MD5_Update(&md5_ctx, buf, EMMC_BLOCK_SIZE);

        if (write(fd_blk, buf, EMMC_BLOCK_SIZE) != EMMC_BLOCK_SIZE) {
            ERROR("write partition fail!\n");
            goto out;
        }

        part_size_in_blk--;
    }

    MD5_Final(md5_random, &md5_ctx);

    ioctl(fd_blk, BLKFLSBUF, NULL);
    lseek(fd_blk, 0, SEEK_SET);

    part_size_in_blk = origin_size;
    MD5_Init(&md5_ctx);

    while (part_size_in_blk) {
        memset(buf, 0x0, EMMC_BLOCK_SIZE);
        len = read(fd_blk, buf, EMMC_BLOCK_SIZE);

        if ( len != EMMC_BLOCK_SIZE) {
            ERROR(" size:%ld read blk fail, error:%s! errno:%d\n",
                  len, strerror(errno), errno);
            goto out;
        }

        MD5_Update(&md5_ctx, buf, EMMC_BLOCK_SIZE);
        part_size_in_blk--;
    }

    MD5_Final(md5_read, &md5_ctx);

    if (memcmp(md5_random, md5_read, MD5_DIGEST_LENGTH)) {
        ERROR("md5 check fail!\n");
        goto out;
    }

    ioctl(fd_blk, BLKFLSBUF, NULL);
    ret = true;
out:

    if (fd_random > 0)
        close(fd_random);

    if (fd_blk > 0)
        close(fd_blk);

    if (buf)
        free(buf);

    return ret;
}

int slt_start(uint times, uint timeout, char* result_string)
{
    int ret = -1;

    printf(" entry, times= %d, timeout =%d\n", times, timeout);

    strcpy(result_string, testcase_identifier);
    result_string = &result_string[sizeof(testcase_identifier) - 1];

    if (!check_emmc())
    {
        if (result_string != NULL) {
            strcpy(result_string, "fail to check eMMC!");
        }
        goto out;
    }
    ret = 0;

    printf("%s testcase success\n", testcase_identifier);

out:
    return ret;
}

// test case time: run test times in test case, if not support set default 1
// test case timeout: run test timeout default value, must bigger than case us time, or case will be kill
