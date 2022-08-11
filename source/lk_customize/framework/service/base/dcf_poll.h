/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 * Copyright (c) 2019-2020, Semidrive Semiconductor Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#ifndef _DCF_POLL_H_
#define _DCF_POLL_H_

#include <reg.h>
#include <stdio.h>
#include <trace.h>
#include <errno.h>
#include <mbox_hal.h>

/***************************************************************
** POSIX select/poll API
****************************************************************/
/** DCF_TIMEVAL_PRIVATE: if you want to use the struct timeval provided
 * by your system, set this to 0 and include <sys/time.h> in cc.h */
#ifndef DCF_TIMEVAL_PRIVATE
#define DCF_TIMEVAL_PRIVATE (1)
#endif

#define FD_SETSIZE      32
#define NBBY            8       /* number of bits in a byte */

typedef long fd_mask;
#define NFDBITS (sizeof (fd_mask) * NBBY)   /* bits per mask */
#ifndef howmany
#define howmany(x,y)    (((x)+((y)-1))/(y))
#endif

/* We use a macro for fd_set so that including Sockets.h afterwards
   can work.  */
typedef struct _types_fd_set {
    fd_mask fds_bits[howmany(FD_SETSIZE, NFDBITS)];
} _types_fd_set;

#define fd_set _types_fd_set

#define FD_SET(n, p)    ((p)->fds_bits[(n)/NFDBITS] |= (1L << ((n) % NFDBITS)))
#define FD_CLR(n, p)    ((p)->fds_bits[(n)/NFDBITS] &= ~(1L << ((n) % NFDBITS)))
#define FD_ISSET(n, p)  ((p)->fds_bits[(n)/NFDBITS] & (1L << ((n) % NFDBITS)))
#define FD_ZERO(p)      memset((void*)(p), 0, sizeof(*(p)))

#define POLLIN          (0x01)
#define POLLRDNORM      (0x01)
#define POLLRDBAND      (0x01)
#define POLLPRI         (0x01)

#define POLLOUT         (0x02)
#define POLLWRNORM      (0x02)
#define POLLWRBAND      (0x02)

#define POLLERR         (0x04)
#define POLLHUP         (0x08)
#define POLLNVAL        (0x10)

struct pollfd
{
    int fd;
    short events;
    short revents;
};

#define POLLMASK_DEFAULT (POLLIN | POLLOUT | POLLRDNORM | POLLWRNORM)

#if DCF_TIMEVAL_PRIVATE
struct timeval {
  long    tv_sec;         /* seconds */
  long    tv_usec;        /* and microseconds */
};
#endif

int dcf_select(int maxfdp1, fd_set *readset, fd_set *writeset, fd_set *exceptset,
            struct timeval *timeout);
int dcf_select_ms(int maxfdp1, fd_set *readset, fd_set *writeset, fd_set *exceptset,
            lk_time_t timeout);

int dcf_handle_pollevent(int fd);

static inline int dcf_select_rd(int nfds, fd_set *readfds, lk_time_t timeout)
{
    return dcf_select_ms(nfds, readfds, NULL, NULL, timeout);
}

#endif //_DCF_POLL_H_
