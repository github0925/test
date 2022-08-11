/*
 * doip_test.c
 *
 *  Created on: Oct 4, 2014
 *    Author: etsam
 */

/*
 * Test application that data integraty of inter processor
 * communication from linux userspace to a remote software
 * context. The application sends chunks of data to the
 * remote processor. The remote side echoes the data back
 * to application which then validates the data returned.
 */

#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <time.h>
#include <fcntl.h>
#include <string.h>
#include <linux/rpmsg.h>
#include <pthread.h>
#include <dirent.h>
#include <sys/mman.h>
#include <signal.h>
// used for UDP socket
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define DEBUG                   1

#define MIN(a, b)               (((a) < (b)) ? (a) : (b))
#define in_range(c, lo, up)     ((unsigned char)c >= lo && (unsigned char)c <= up)
#define isprint(c)              in_range(c, 0x20, 0x7f)

#define DOIP_INVALID_U32        0xffffffff

#define DOIP_MAX_DATA_LEN       4096

#define DOIP_NORMAL_TEST_MODE   0
#define DOIP_LOOP_TEST_MODE     1

#define RPMSG_DEV_NAME          "/dev/sdpe-doip"

#define SDPE_RPC_DOIP_SERVICE   5

#define SDPE_RPC_REQ            0
#define SDPE_RPC_RSP            1
#define SDPE_RPC_IND            2

struct rpc_msg_head {
    uint16_t msg_id;
    uint16_t msg_type;
    uint32_t cookie;
} __attribute__((packed));

typedef struct sdpe_doip_addr {
    uint8_t mtype;
    uint8_t tatype;
    uint16_t sa;
    uint16_t ta;
    uint16_t ae;
} sdpe_doip_addr_t;

struct vdoip_data_req_s {
    struct sdpe_doip_addr t_addr;
    uint32_t t_len;
    uint8_t t_data[0];
} __attribute__((packed));

struct vdoip_data_cnf_s {
    struct sdpe_doip_addr t_addr;
    uint32_t t_result;
} __attribute__((packed));

struct vdoip_data_som_ind_s {
    struct sdpe_doip_addr t_addr;
    uint32_t t_len;
} __attribute__((packed));

struct vdoip_data_ind_s {
    struct sdpe_doip_addr t_addr;
    uint32_t t_result;
    uint32_t t_len;
    uint8_t t_data[0];
} __attribute__((packed));

enum vdoip_cb_types_e {
    VDOIP_MSG_START = 0,
    VDOIP_DATA_REQ,
    VDOIP_DATA_CNF,
    VDOIP_DATA_SOM_IND,
    VDOIP_DATA_IND,
    VDOIP_MSG_END
};

static int rpmsg_fd = -1;

static void hexdump8_ex(const void *ptr, size_t len, u_int64_t disp_addr)
{
    caddr_t address = (caddr_t)ptr;
    size_t count;
    size_t i;
    const char *addr_fmt = ((disp_addr + len) > 0xFFFFFFFF)
                           ? "0x%016llx: "
                           : "0x%08llx: ";

    for (count = 0 ; count < len; count += 16) {
        printf(addr_fmt, disp_addr + count);

        for (i = 0; i < MIN(len - count, 16); i++) {
            printf("%02hhx ", *(const unsigned char *)(address + i));
        }

        for (; i < 16; i++) {
            printf("   ");
        }

        printf("|");

        for (i = 0; i < MIN(len - count, 16); i++) {
            char c = ((const char *)address)[i];
            printf("%c", isprint(c) ? c : '.');
        }

        printf("\n");
        address += 16;
    }
}

static inline void hexdump8(const void *ptr, size_t len)
{
    hexdump8_ex(ptr, len, (u_int64_t)((caddr_t)ptr));
}

static void doip_send_data(int fd, uint32_t ta, uint8_t *send_data,
                           uint32_t data_len)
{
    struct rpc_msg_head *head;
    struct vdoip_data_req_s *data_req;
    uint32_t total_len;
    uint8_t *send_buffer;

    total_len = sizeof(struct rpc_msg_head) + sizeof(struct vdoip_data_req_s) +
                data_len;
    send_buffer = (uint8_t *)malloc(total_len);

    head = (struct rpc_msg_head *)send_buffer;
    head->msg_id = (SDPE_RPC_DOIP_SERVICE << 8) + VDOIP_DATA_REQ;
    head->msg_type = SDPE_RPC_IND;

    data_req = (struct vdoip_data_req_s *)(send_buffer + sizeof(
            struct rpc_msg_head));
    data_req->t_addr.mtype = 0;
    data_req->t_addr.tatype = 0;
    data_req->t_addr.sa = 0x101;
    data_req->t_addr.ta = ta;
    data_req->t_len = data_len;
    memcpy(data_req->t_data, send_data, data_len);

    write(fd, send_buffer, total_len);

#if DEBUG
    printf("%s(): sa 0x%x, ta 0x%x, len %d\n", __func__,
           data_req->t_addr.sa, data_req->t_addr.ta, data_req->t_len);
    hexdump8(data_req->t_data, data_req->t_len);
#endif

    free(send_buffer);
}

static void doip_recv_data_cnf(uint8_t *msg)
{
    struct vdoip_data_cnf_s *cnf = (struct vdoip_data_cnf_s *)msg;

#if DEBUG
    printf("%s(): sa 0x%x, ta 0x%x, result %d\n", __func__,
           cnf->t_addr.sa, cnf->t_addr.ta, cnf->t_result);
#endif
}

static void doip_recv_data_ind(uint8_t *msg)
{
#if DEBUG
    struct vdoip_data_ind_s *ind = (struct vdoip_data_ind_s *)msg;

    printf("%s(): sa 0x%x, ta 0x%x, result %d\n", __func__,
           ind->t_addr.sa, ind->t_addr.ta, ind->t_result);

    if (ind->t_result == 0) {
        printf("data_len:%d\n", ind->t_len);
        hexdump8(ind->t_data, ind->t_len);
    }

#endif
}

static void doip_loopback(int fd, uint8_t *buffer, uint32_t ta,
                          uint32_t data_len)
{
    struct vdoip_data_ind_s *ind;
    uint32_t send_ta;
    uint32_t send_data_len;
    uint8_t *send_data;
    uint32_t pos;
    uint32_t left_data_len;

    ind = (struct vdoip_data_ind_s *)(buffer + sizeof(struct rpc_msg_head));

    if (ind->t_result == 0) {
        send_ta = (ta != DOIP_INVALID_U32) ? ta : ind->t_addr.sa;
        send_data_len = (data_len != DOIP_INVALID_U32) ? data_len : ind->t_len;
        send_data = malloc(send_data_len);

        if (send_data) {
            pos = 0;
            left_data_len = send_data_len;

            while (left_data_len > ind->t_len) {
                memcpy(&send_data[pos], ind->t_data, ind->t_len);
                pos += ind->t_len;
                left_data_len -= ind->t_len;
            }

            if (left_data_len) {
                memcpy(&send_data[pos], ind->t_data, left_data_len);
            }

            doip_send_data(fd, send_ta, send_data, send_data_len);
            free(send_data);
        }
    }
}

static void doip_recv_data_som_ind(uint8_t *msg)
{
#if DEBUG
    struct vdoip_data_som_ind_s *ind = (struct vdoip_data_som_ind_s *)msg;

    printf("%s(): sa 0x%x, ta 0x%x, len %d\n", __func__,
           ind->t_addr.sa, ind->t_addr.ta, ind->t_len);
#endif
}

static bool doip_recv_data(int fd, uint8_t *recv_buf)
{
    struct rpc_msg_head *head;
    uint8_t *msg_data;
    uint8_t msg_id;
    bool data_ind = false;

    if (read(fd, recv_buf, DOIP_MAX_DATA_LEN) < 0) {
        printf("doip recv err\n");
        goto out;
    }

    head = (struct rpc_msg_head *)recv_buf;
    msg_id = head->msg_id & 0xff;

    msg_data = recv_buf + sizeof(struct rpc_msg_head);

    switch (msg_id) {
        case VDOIP_DATA_CNF:
            doip_recv_data_cnf(msg_data);
            break;

        case VDOIP_DATA_IND:
            doip_recv_data_ind(msg_data);
            data_ind = true;
            break;

        case VDOIP_DATA_SOM_IND:
            doip_recv_data_som_ind(msg_data);
            break;

        default:
            break;
    }

out:
    return data_ind;
}

int main(int argc, char *argv[])
{
    uint8_t *buffer;
    uint32_t data_len = DOIP_INVALID_U32;
    uint32_t ta = DOIP_INVALID_U32;
    uint8_t test_mode = DOIP_NORMAL_TEST_MODE;
    uint32_t wait_time = 100;
    int opt;

    while ((opt = getopt(argc, argv, "l:a:m:t:")) != -1) {
        switch (opt) {
            case 'l':
                data_len = atoi(optarg);
                printf("getopt l for loop data len %d\n", data_len);
                break;

            case 'a':
                ta = strtol(optarg, NULL, 16);
                printf("getopt a for loop data ta 0x%x\n", ta);
                break;

            case 'm':
                test_mode = atoi(optarg);
                printf("getopt m for test mode %d\n", test_mode);
                break;

            case 't':
                wait_time = atoi(optarg);
                printf("getopt t for wait time %d\n", wait_time);
                break;

            default:
                printf("getopt return unsupported option: -%c\n", opt);
                break;
        }
    }

    printf("\r\n doip test start \r\n");

    rpmsg_fd = open(RPMSG_DEV_NAME, O_RDWR);

    if (rpmsg_fd < 0) {
        perror("Failed to open rpmsg device.");
        return -1;
    }

    buffer = (uint8_t *)malloc(DOIP_MAX_DATA_LEN);
    memset(buffer, 0x00, DOIP_MAX_DATA_LEN);

    while (1) {
        if (doip_recv_data(rpmsg_fd, buffer) && \
                test_mode == DOIP_LOOP_TEST_MODE) {
            usleep(wait_time * 1000);
            doip_loopback(rpmsg_fd, buffer, ta, data_len);
        }
    }

    free(buffer);

    close(rpmsg_fd);

    return 0;
}
