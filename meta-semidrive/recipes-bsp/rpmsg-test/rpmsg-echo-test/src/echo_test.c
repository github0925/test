/*
 * echo_test.c
 *
 * Copyright (c) 2020 SemiDrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */
/*
 * Test application that data integraty of inter processor
 * communication from linux userspace to a remote software
 * context. The application sends chunks of data to the
 * remote processor. The remote side echoes the data back
 * to application which then validates the data returned.
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
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <linux/rpmsg.h>
#include <pthread.h>

struct _payload {
    uint16_t magic;
    uint16_t num;
    uint32_t size;
    char data[];
} __attribute__((__packed__));

#define ECHO_TEST_VERSION "v201130"

static int charfd = -1, fd = -1, err_cnt;
static int verbose = 0;
static int benchmark = 0;
static int interval = 0;
static int server_mode;
static int is_socket;
static int rproc_id;
static int sa_addr;
static struct timeval start_test, end_test;

struct _payload *i_payload;
struct _payload *r_payload;

#define RPMSG_GET_KFIFO_SIZE 1
#define RPMSG_GET_AVAIL_DATA_SIZE 2
#define RPMSG_GET_FREE_SPACE 3

#define RPMSG_HEADER_LEN 16
#define MAX_RPMSG_BUFF_SIZE (508 - RPMSG_HEADER_LEN)
#define PAYLOAD_MIN_SIZE    1
#define PAYLOAD_MAX_SIZE    (MAX_RPMSG_BUFF_SIZE - sizeof(struct _payload))
#define NUM_PAYLOADS        (PAYLOAD_MAX_SIZE/PAYLOAD_MIN_SIZE)

static int max_rpmsg_buf_size = MAX_RPMSG_BUFF_SIZE;
static int payload_max_size = PAYLOAD_MAX_SIZE;
static int num_payloads = NUM_PAYLOADS;

#define RPMSG_BUS_SYS "/sys/bus/rpmsg"

static int rpmsg_create_ept(int rpfd, struct rpmsg_endpoint_info *eptinfo)
{
    int ret;

    ret = ioctl(rpfd, RPMSG_CREATE_EPT_IOCTL, eptinfo);

    if (ret)
        perror("Failed to create endpoint.\n");

    return ret;
}

static char *get_rpmsg_ept_dev_name(const char *rpmsg_char_name,
                                    const char *ept_name,
                                    char *ept_dev_name)
{
    char sys_rpmsg_ept_name_path[64];
    char svc_name[64];
    char *sys_rpmsg_path = "/sys/class/rpmsg";
    FILE *fp;
    int i;
    int ept_name_len;

    for (i = 0; i < 128; i++) {
        sprintf(sys_rpmsg_ept_name_path, "%s/%s/rpmsg%d/name",
                sys_rpmsg_path, rpmsg_char_name, i);
        printf("checking %s\n", sys_rpmsg_ept_name_path);

        if (access(sys_rpmsg_ept_name_path, F_OK) < 0)
            continue;

        fp = fopen(sys_rpmsg_ept_name_path, "r");

        if (!fp) {
            printf("failed to open %s\n", sys_rpmsg_ept_name_path);
            break;
        }

        fgets(svc_name, sizeof(svc_name), fp);
        fclose(fp);
        printf("svc_name: %s.\n", svc_name);
        ept_name_len = strlen(ept_name);

        if (ept_name_len > sizeof(svc_name))
            ept_name_len = sizeof(svc_name);

        if (!strncmp(svc_name, ept_name, ept_name_len)) {
            sprintf(ept_dev_name, "rpmsg%d", i);
            return ept_dev_name;
        }
    }

    printf("Not able to RPMsg endpoint file for %s:%s.\n",
           rpmsg_char_name, ept_name);
    return NULL;
}

static int bind_rpmsg_chrdev(const char *rpmsg_dev_name)
{
    char fpath[256];
    char *rpmsg_chdrv = "rpmsg_chrdev";
    int fd;
    int ret;


    /* rpmsg dev overrides path */
    sprintf(fpath, "%s/devices/%s/driver_override",
            RPMSG_BUS_SYS, rpmsg_dev_name);
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
    sprintf(fpath, "%s/drivers/%s/bind", RPMSG_BUS_SYS, rpmsg_chdrv);
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
                RPMSG_BUS_SYS, rpmsg_chdrv);
        return -EINVAL;
    }

    close(fd);
    return 0;
}

static int unbind_rpmsg_chrdev(const char *rpmsg_dev_name)
{
    char fpath[256];
    char *rpmsg_chdrv = "rpmsg_chrdev";
    int fd;
    int ret;

    /* unbind the rpmsg device to rpmsg char driver */
    sprintf(fpath, "%s/drivers/%s/unbind", RPMSG_BUS_SYS, rpmsg_chdrv);
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

static int get_rpmsg_chrdev_fd(const char *rpmsg_dev_name,
                               char *rpmsg_ctrl_name)
{
    char dpath[256];
    char fpath[256];
    int path_len = 0;
    char *rpmsg_ctrl_prefix = "rpmsg_ctrl";
    DIR *dir;
    struct dirent *ent;
    int fd;

    sprintf(dpath, "%s/devices/%s/rpmsg", RPMSG_BUS_SYS, rpmsg_dev_name);
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

            if (path_len <= 0)
                continue;

            fd = open(fpath, O_RDWR | O_NONBLOCK);

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
#define RPMSG_DEVICE_NAME       "virtio1.rpmsg-echo.-1.30"
static void usage(const char *cmd)
{
    printf("This is s rpmsg echo test %s\n", ECHO_TEST_VERSION);
    printf("command options:\n");
    printf("%s -h\t\t: Show this help info\n", cmd);
    printf("%s -d [devname]\t: Specify device name (default: %s)\n",
            cmd, RPMSG_DEVICE_NAME);
    printf("\t\t\t: Show more devices with %s -l\n", cmd);
    printf("%s -n [times]\t: Specify test round name (default: 1)\n", cmd);
    printf("%s -t [hours]\e in hot: Specify test overall timur\n", cmd);
    printf("%s -b\t\t: Benchmark test mode\n", cmd);
    printf("%s -p\t\t: iperf-like test mode\n", cmd);
    printf("%s -i [seconds]\t: test round interval in seconds\n", cmd);
    printf("%s -m [bytes]\t: max rpmsg buffer size in range (%ld, %d]\n",
            cmd, sizeof(struct _payload), MAX_RPMSG_BUFF_SIZE);
    printf("%s -S [remote]\t: socket mode to rproc [0:safety 1:secure]\n", cmd);
    printf("%s -a [addr]\t: socket address to rproc app\n", cmd);
    printf("%s -v\t\t: Verbose mode\n", cmd);

    exit(0);
}

static void property_usage(const char *cmd)
{
    printf("This is a property test %s\n", ECHO_TEST_VERSION);
    printf("%s set [id] [val]", cmd);
    printf("\t:Set a property [id] with value [val]\n");
    printf("%s get [id]", cmd);
    printf("\t:Get a property value with [id]\n");

    exit(0);
}

long time_elapsed_us(struct timeval start, struct timeval end)
{
    return (end.tv_sec - start.tv_sec) * (1000000) + end.tv_usec - start.tv_usec;
}

#if 0
static int get_thread_priority(pthread_attr_t *attr)
{
    struct sched_param param;
    int rs = pthread_attr_getschedparam(attr, &param);
    assert(rs == 0);
    printf("priority=%d", param.__sched_priority);
    return param.__sched_priority;
}

static void set_thread_policy(pthread_attr_t *attr, int policy)
{
    int rs = pthread_attr_setschedpolicy(attr, policy);
    assert(rs == 0);
}
#endif

#include <sched.h>

#define DEFAULT_BENCH_ROUND     (1)
int benchmark_test(int fd, int round)
{
    int i = 0, j = 0;
    int size, bytes_rcvd, bytes_sent;
    long elapsed;
    struct sched_param sched;

    err_cnt = 0;
    i_payload = (struct _payload *)malloc(sizeof(struct _payload) +
                                          payload_max_size);
    r_payload = (struct _payload *)malloc(sizeof(struct _payload) +
                                          payload_max_size);

    if (i_payload == 0 || r_payload == 0) {
        printf("ERROR: Failed to allocate memory for payload.\n");
        return -1;
    }

    pid_t pid = getpid();
    sched.sched_priority = 20;
//  set_thread_policy(&attr, SCHED_RR);
    sched_setscheduler(pid, SCHED_RR, &sched);

    /* Mark the data buffer. */
    memset(&(i_payload->data[0]), 0xA5, payload_max_size);
    i_payload->magic = 0xA5;

    read(fd, r_payload, sizeof(struct _payload) + payload_max_size);

    printf("**************************************\r\n");
    printf("*Echo Benchmark Test\r\n");
    printf("**************************************\r\n");

    for (i = 0, size = PAYLOAD_MIN_SIZE; size <= payload_max_size; i++) {
        int k;

        i_payload->num = i;
        i_payload->size = size;
        gettimeofday(&start_test, NULL);

        for (j = 0; j < round; j++) {
            bytes_sent = write(fd, i_payload, sizeof(struct _payload) + size);

            if (bytes_sent <= 0) {
                err_cnt++;
                perror("\r\n Error sending data");
                break;
            }

            r_payload->num = 0;
            bytes_rcvd = read(fd, r_payload, sizeof(struct _payload) + size);

            while (bytes_rcvd <= 0) {
                usleep(100);
                bytes_rcvd = read(fd, r_payload, sizeof(struct _payload) + size);
            }

            /* Validate data buffer integrity. */
            for (k = 0; k < r_payload->size; k++) {
                if (r_payload->data[k] != 0xA5) {
                    printf(" \r\n Data corruption at index %d \r\n", k);
                    err_cnt++;
                    break;
                }
            }
        }

        gettimeofday(&end_test, NULL);

        elapsed = time_elapsed_us(start_test, end_test);
        printf("Benchmark packet size = %d in %ld us\n", size, elapsed);
        printf("Round-trip avg/min/max = %ld us\r\n\n", elapsed / round);

        if (interval)
            sleep(interval);

        if (size < payload_max_size / 2) {
            size *= 2;
        }
        else
            size += 32;
    }

    printf("**************************************\r\n");
    printf("Test Done. Error count = %d\r\n", err_cnt);
    printf("**************************************\r\n");

    free(i_payload);
    free(r_payload);

    return 0;
}

int iperf_test(int fd, int round)
{
    int j = 0;
    int size, bytes_sent;
    long elapsed;
    struct sched_param sched;
    int count;

    err_cnt = 0;
    count = 0;
    size = payload_max_size;
    i_payload = (struct _payload *)malloc(sizeof(struct _payload) + size);

    if (i_payload == 0) {
        printf("ERROR: Failed to allocate memory for payload.\n");
        return -1;
    }

    pid_t pid = getpid();
    sched.sched_priority = 20;
//  set_thread_policy(&attr, SCHED_RR);
    sched_setscheduler(pid, SCHED_RR, &sched);

    /* Mark the data buffer. */
    memset(&(i_payload->data[0]), 0xA5, payload_max_size);
    i_payload->magic = 0xAA;

    printf("**************************************\r\n");
    printf("*iperf Test\n");
    printf("**************************************\r\n");

    i_payload->num = 0;
    i_payload->size = size;
    gettimeofday(&start_test, NULL);

    for (j = 0; j < round; j++) {
        bytes_sent = write(fd, i_payload, sizeof(struct _payload) + size);

        if (bytes_sent <= 0) {
            err_cnt++;
            perror("\r\n Error sending data");
            break;
        }

        count++;

    }

    gettimeofday(&end_test, NULL);

    elapsed = time_elapsed_us(start_test, end_test);

    printf("%d packets transmitted, %d bytes, %d loss\n", count, count * size,
           err_cnt);
    printf("throughput avg. = %.2f MB/s\n", (float)(count * size) / elapsed);
    printf("tx latency avg. = %ld us\n", elapsed / count);

    free(i_payload);

    return 0;
}

int payload_test(int fd, int ntimes, int hours)
{
    int i = 0, j = 0;
    int size, bytes_rcvd, bytes_sent;
    long elapse = 0;

    err_cnt = 0;
    i_payload = (struct _payload *)malloc(sizeof(struct _payload) +
                                          payload_max_size);
    r_payload = (struct _payload *)malloc(sizeof(struct _payload) +
                                          payload_max_size);

    if (i_payload == 0 || r_payload == 0) {
        printf("ERROR: Failed to allocate memory for payload.\n");
        return -1;
    }

    if (hours)
        gettimeofday(&start_test, NULL);

    i_payload->magic = 0xA5;

    for (j = 0; j < ntimes; j++) {
        printf("\n**************************************\n");
        printf("* Echo Test Round %d\n", j);
        printf("**************************************\n");

        for (i = 0, size = PAYLOAD_MIN_SIZE; i < num_payloads;
                i++, size++) {
            int k;

            i_payload->num = i;
            i_payload->size = size;

            /* Mark the data buffer. */
            memset(&(i_payload->data[0]), 0xA5, size);

            if (verbose) printf("\r\n sending payload number");

            if (verbose) printf(" %d of size %lu\r\n", i_payload->num,
                                    (sizeof(struct _payload)) + size);

            bytes_sent = write(fd, i_payload, sizeof(struct _payload) + size);

            if (bytes_sent <= 0) {
                if (verbose) {
                    perror("\r\n Error sending data\n");
                    break;
                }
                else
                    fprintf(stderr, "#");

                err_cnt++;
                continue;
            }

            if (verbose) printf("echo test: sent : %d\n", bytes_sent);

            r_payload->num = 0;
            bytes_rcvd = read(fd, r_payload,
                              sizeof(struct _payload) + payload_max_size);

            while (bytes_rcvd <= 0) {
                usleep(10000);
                bytes_rcvd = read(fd, r_payload,
                                  sizeof(struct _payload) + payload_max_size);
            }

            if (verbose) printf(" received payload number ");

            if (verbose) printf("%d of size %d\r\n", r_payload->num, bytes_rcvd);

            /* Validate data buffer integrity. */
            for (k = 0; k < r_payload->size; k++) {
                if (r_payload->data[k] != 0xA5) {
                    err_cnt++;

                    if (verbose) {
                        printf("  Data corruption at index %d \r\n", k);
                        break;
                    }
                    else
                        fprintf(stderr, "X");

                    continue;
                }
            }

            bytes_rcvd = read(fd, r_payload, sizeof(struct _payload) + payload_max_size);

            if (!verbose) fprintf(stderr, ".");
        }

        printf("\n**************************************\n");
        printf("* Echo Test Round %d, Results: Error count = %d\n", j, err_cnt);
        printf("**************************************\n");

        if (hours) {
            gettimeofday(&end_test, NULL);
            elapse = end_test.tv_sec - start_test.tv_sec;

            elapse /= 3600; // to hours

            if (elapse >= hours)
                break;
        }

        if (interval)
            sleep(interval);
    }

    free(i_payload);
    free(r_payload);

    return 0;
}

int server_mode_test(int fd)
{
    int bytes_rcvd, bytes_sent;

    r_payload = (struct _payload *)malloc(sizeof(struct _payload) +
                                          payload_max_size);

    if (r_payload == 0) {
        printf("ERROR: Failed to allocate memory for payload.\n");
        return -1;
    }

    while (1) {
        bytes_rcvd = read(fd, r_payload, sizeof(struct _payload) + payload_max_size);

        if (verbose) printf(" received payload number %d of size %d\r\n",
                                r_payload->num, bytes_rcvd);

        bytes_sent = write(fd, r_payload, bytes_rcvd);

        if (verbose) printf(" echo payload %d ?= %d\r\n", bytes_sent, bytes_rcvd);
    }

    free(r_payload);
}

void list_rpmsg_device(void)
{
    system("ls /sys/bus/rpmsg/devices -w 1");

    exit(0);
}

struct dcf_ioc_setproperty {
    __u32 property_id;
    __u32 property_value;
    __u32 reserved1;
    __u32 reserved2;
};

#define DCF_IOC_MAGIC       'D'
#define DCF_IOC_SET_PROPERTY    _IOWR(DCF_IOC_MAGIC, 2,\
                    struct dcf_ioc_setproperty)

#define DCF_IOC_GET_PROPERTY    _IOWR(DCF_IOC_MAGIC, 3,\
                    struct dcf_ioc_setproperty)

void do_property_test(int argc, char **argv)
{
    int fd;
    int ret;
    struct dcf_ioc_setproperty setprop;

    if (argc == 1) {
        property_usage("property");
    }

    fd = open("/dev/property", O_RDWR);

    if (fd < 0) {
        perror("Failed to open property device.");
        return;
    }

    if (strncmp(argv[1], "set", 3) == 0) {
        if (argc != 4) {
            property_usage("property");
        }

        setprop.property_id = strtol(argv[2], NULL, 0);
        setprop.property_value = strtol(argv[3], NULL, 0);
        ret = ioctl(fd, DCF_IOC_SET_PROPERTY, &setprop);

        if (ret)
            perror("Failed to create endpoint.\n");
    }

    if (strncmp(argv[1], "get", 3) == 0) {
        if (argc != 3) {
            property_usage("property");
        }

        setprop.property_id = atol(argv[2]);
        ret = ioctl(fd, DCF_IOC_GET_PROPERTY, &setprop);

        if (ret)
            perror("Failed to create endpoint.\n");
    }

    printf("%s property[%d] = 0x%x\n", argv[1], setprop.property_id,
           setprop.property_value);

    close(fd);
}

/* Copied from uapi/linux/rpmsg_socket.h
 * http://gerrit.semidrive.net:8081/#/c/android/kernel/common/+/1263/8/include/uapi/linux/rpmsg_socket.h
 */
#include <sys/socket.h>
#include <linux/types.h>
#include <linux/socket.h>

#ifndef AF_RPMSG
#define AF_RPMSG	44
#define PF_RPMSG	AF_RPMSG
#endif

struct sockaddr_rpmsg {
    sa_family_t family;
    __u32 vproc_id;
    __u32 addr;
};

int socket_test(int fd, int ntimes, int hours)
{
    int i = 0, j = 0;
    int size, bytes_rcvd, bytes_sent;
    long elapse = 0;

    err_cnt = 0;
    i_payload = (struct _payload *)malloc(sizeof(struct _payload) + payload_max_size);
    r_payload = (struct _payload *)malloc(sizeof(struct _payload) + payload_max_size);

    if (i_payload == 0 || r_payload == 0) {
        printf("ERROR: Failed to allocate memory for payload.\n");
        return -1;
    }

    if (hours)
        gettimeofday(&start_test, NULL);

    i_payload->magic = 0xA5;

    for (j = 0; j < ntimes; j++) {
        printf("\n**************************************\n");
        printf("* Socket Test Round %d\n", j);
        printf("**************************************\n");
        for (i = 0, size = PAYLOAD_MIN_SIZE; i < num_payloads;
        i++, size++) {
            int k;

            i_payload->num = i;
            i_payload->size = size;

            /* Mark the data buffer. */
            memset(&(i_payload->data[0]), 0xA5, size);

            if (verbose) printf("\r\n sending payload number");
            if (verbose) printf(" %d of size %lu\r\n", i_payload->num,
                            (sizeof(struct _payload)) + size);

            bytes_sent = write(fd, i_payload, sizeof(struct _payload) + size);
            if (bytes_sent <= 0) {
                if (verbose) {
                    perror("\r\n Error sending data\n");
                    break;
                } else
                    fprintf(stderr, "#");

                err_cnt++;
                continue;
            }
            if (verbose) printf("echo test: sent : %d\n", bytes_sent);

            r_payload->num = 0;
            bytes_rcvd = read(fd, r_payload,
                            sizeof(struct _payload) + payload_max_size);
            while (bytes_rcvd <= 0) {
                usleep(10000);
                bytes_rcvd = read(fd, r_payload,
                            sizeof(struct _payload) + payload_max_size);
            }
            if (verbose) printf(" received payload number ");
            if (verbose) printf("%d of size %d\r\n", r_payload->num, bytes_rcvd);

            if (strncmp((const char*)r_payload, "rpcallack", sizeof("rpcallack")) == 0) {
                if (!verbose) fprintf(stderr, ".");
                continue;
            }
            if (strncmp((const char*)r_payload, "rpcallerr", sizeof("rpcallerr")) == 0) {
                err_cnt++;
                fprintf(stderr, "X");
                continue;
            }

            /* Validate data buffer integrity. */
            for (k = 0; k < r_payload->size; k++) {
                if (r_payload->data[k] != 0xA5) {
                    err_cnt++;
                    if (verbose) {
                        printf("  Data corruption at index %d \r\n", k);
                        break;
                    } else
                        fprintf(stderr, "X");
                    continue;
                }
            }
            if (!verbose) fprintf(stderr, ".");
        }
        printf("\n**************************************\n");
        printf("* Echo Test Round %d, Results: Error count = %d\n", j, err_cnt);
        printf("**************************************\n");

        if (hours) {
            gettimeofday(&end_test, NULL);
            elapse = end_test.tv_sec - start_test.tv_sec;
            elapse /= 3600; // to hours
            if (elapse >= hours)
                break;
        }

        if (interval)
            sleep(interval);
    }

    free(i_payload);
    free(r_payload);

    return 0;
}

int main(int argc, char *argv[])
{
    int ret;
    int opt;
    char *rpmsg_dev = RPMSG_DEVICE_NAME;
    int ntimes = 1;
    char fpath[256];
    char rpmsg_char_name[16];
    struct rpmsg_endpoint_info eptinfo;
    char ept_dev_name[16];
    char ept_dev_path[32];
    int test_hours = 0;

    if (strstr(argv[0], "property")) {
        do_property_test(argc, argv);
        return 0;
    }

    if (argc == 1) {
        usage("echo_test");
    }

    while ((opt = getopt(argc, argv, "a:d:n:bpvst:hi:lm:S:")) != -1) {
        switch (opt) {
            case 'd':
                rpmsg_dev = optarg;
                break;

            case 'n':
                ntimes = atoi(optarg);
                break;

            case 'v':
                verbose = 1;
                break;

            case 'b':
                benchmark = 1;
                break;

            case 'p':
                benchmark = 2;
                break;

            case 'S':
                is_socket = 1;
                rproc_id = atoi(optarg);
                break;

            case 'a':
                if (is_socket == 0)
                    usage("echo_test");

                sa_addr = atoi(optarg);
                break;

            case 't':
                test_hours = atoi(optarg);
                break;

            case 'i':
                interval = atoi(optarg);
                break;

            case 'm':
                max_rpmsg_buf_size = atoi(optarg);

                if (max_rpmsg_buf_size <= sizeof(struct _payload)
                        || max_rpmsg_buf_size > MAX_RPMSG_BUFF_SIZE) {
                    printf("set max buf size in range (%ld,%d]\n", sizeof(struct _payload),
                           MAX_RPMSG_BUFF_SIZE);
                    exit(-1);
                }

                payload_max_size = (max_rpmsg_buf_size - sizeof(struct _payload));
                num_payloads = payload_max_size / PAYLOAD_MIN_SIZE;
                break;

            case 'h':
                usage("echo_test");
                break;

            case 'l':
                list_rpmsg_device();
                break;

            case 's':
                server_mode = 1;
                break;

            default:
                printf("getopt return unsupported option: -%c\n", opt);
                usage("echo_test");
                break;
        }
    }

    printf("\r\n Echo test start \r\n");

#if 0
    /* Load rpmsg_char driver */
    printf("\r\nMaster>probe rpmsg_char\r\n");
    ret = system("modprobe rpmsg_char");

    if (ret < 0) {
        perror("Failed to load rpmsg_char driver.\n");
        return -EINVAL;
    }

#endif
    if (is_socket) {
        struct sockaddr_rpmsg addr;

        if((fd = socket(PF_RPMSG, SOCK_SEQPACKET, 0)) == -1) {
            perror("Error while opening socket");
            return -1;
        }

        if (sa_addr == 0) {
            printf("please assign sa addr -a [addr]! \r\n");
            return -2;
        }

        addr.family  = AF_RPMSG;
        addr.vproc_id = rproc_id;
        addr.addr = sa_addr;
        if(connect(fd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
            perror("Error in socket connect");
            return -2;
        }

        socket_test(fd, ntimes, test_hours);

        return 0;
    }

    if (strncmp(rpmsg_dev, "/dev/", 4)) {
        /* managed by rpmsg chrdrv */
        printf("\r\n Open rpmsg dev %s! \r\n", rpmsg_dev);
        sprintf(fpath, "%s/devices/%s", RPMSG_BUS_SYS, rpmsg_dev);

        if (access(fpath, F_OK)) {
            fprintf(stderr, "Not able to access rpmsg device %s, %s\n",
                    fpath, strerror(errno));
            return -EINVAL;
        }

        ret = bind_rpmsg_chrdev(rpmsg_dev);

        if (ret < 0)
            return ret;

        charfd = get_rpmsg_chrdev_fd(rpmsg_dev, rpmsg_char_name);

        if (charfd < 0)
            return charfd;

        /* Create endpoint from rpmsg char driver */
        strcpy(eptinfo.name, "rpmsg-test-channel");
        eptinfo.src = 0;
        eptinfo.dst = 0xFFFFFFFF;
        ret = rpmsg_create_ept(charfd, &eptinfo);

        if (ret) {
            printf("failed to create RPMsg endpoint.\n");
            return -EINVAL;
        }

        if (!get_rpmsg_ept_dev_name(rpmsg_char_name, eptinfo.name,
                                    ept_dev_name))
            return -EINVAL;

        sprintf(ept_dev_path, "/dev/%s", ept_dev_name);
    }
    else {
        /* managed by dcf chrdrv */
        strncpy(ept_dev_path, rpmsg_dev, 32);
    }

    fd = open(ept_dev_path, O_RDWR | O_NONBLOCK );

    if (fd < 0) {
        perror("Failed to open rpmsg device.");
        close(charfd);
        return -1;
    }

    if (server_mode) {
        server_mode_test(fd);
    }
    else if (benchmark == 1) {
        benchmark_test(fd, ntimes);
    }
    else if (benchmark == 2) {
        iperf_test(fd, ntimes);
    }
    else {
        if (test_hours) {
            ntimes = 0x7fffffff;
        }

        payload_test(fd, ntimes, test_hours);
    }

    close(fd);

    if (charfd >= 0)
        close(charfd);

    if (strncmp(rpmsg_dev, "/dev/", 4)) {
        unbind_rpmsg_chrdev(rpmsg_dev);
    }

    return 0;
}
