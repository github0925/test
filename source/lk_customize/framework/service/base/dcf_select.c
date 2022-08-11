/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 * Copyright (c) 2019-2020, Semidrive Semiconductor Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#include <reg.h>
#include <stdio.h>
#include <trace.h>
#include <string.h>
#include <stdlib.h>
#include <err.h>
#include <list.h>
#include <kernel/semaphore.h>
#include <kernel/mutex.h>
#include <platform/debug.h>
#include "dcf.h"

#if defined(CONFIG_SUPPORT_POSIX) && (CONFIG_SUPPORT_POSIX == 1)

/** Description for a task waiting in select */
struct select_cb {
    /** Pointer to the waiting task */
    struct list_node node;
    struct pollfd *fds;
    unsigned int nfds;
    /** don't signal the same semaphore twice: set to 1 when signalled */
    int signalled;
    lk_time_t timeout;
    /** semaphore to wake up a task waiting for select */
    semaphore_t sem;
};

/** The global list of tasks waiting for select */
static struct list_node scb_list;
static mutex_t scb_lock;

static void fdszero(fd_set *set, int nfds)
{
    fd_mask *m;
    int n;

    /*
      The 'sizeof(fd_set)' of the system space may differ from user space,
      so the actual size of the 'fd_set' is determined here with the parameter 'nfds'
    */
    m = (fd_mask *)set;
    for (n = 0; n < nfds; n += (sizeof(fd_mask) * 8))
    {
        memset(m, 0, sizeof(fd_mask));
        m ++;
    }
}

static int scan_pollfd(struct pollfd *fds, unsigned int nfds)
{
    struct pollfd *pf;
    int mask = 0;
    unsigned int i;
    int num;

    for (i = 0, num = 0, pf = fds; i < nfds; i++, pf++) {
        mask = POLLNVAL;
        if (pf->fd >= 0) {
            mask = dcf_file_poll(pf);
            /* dealwith the device return error -1*/
            if (mask <= 0) {
                pf->revents = 0;
                continue;
            }
            /* Mask out unneeded events. */
            mask &= pf->events | POLLERR | POLLHUP;
            num++;
        }
        pf->revents = mask;
    }

    return num;
}

static int do_poll(struct pollfd *fds, unsigned int nfds, struct select_cb *scb)
{
    int num;
    int ret = 0;

    num = scan_pollfd(fds, nfds);
    if (num == 0) {
        if (scb->timeout == 0) {
            return 0;
        }

        ret = sem_timedwait(&scb->sem, scb->timeout);
        if (ret == ERR_TIMED_OUT) {
            /* Timeout */
            posix_set_errno(ETIME);
            return 0;
        }

        num = scan_pollfd(fds, nfds);
    }

    posix_set_errno(0);

    return num;
}

int dcf_handle_pollevent(int fd)
{
    struct select_cb *scb;
    struct pollfd *pf;
    unsigned int i;

    mutex_acquire_timeout(&scb_lock, INFINITE_TIME);
    list_for_every_entry(&scb_list, scb, struct select_cb, node) {
        if (scb->signalled == 0) {
            for (i = 0, pf = scb->fds; i < scb->nfds; i++, pf++) {
                if ((pf->fd == fd) && (pf->events & POLLMASK_DEFAULT)) {
                    dprintf(2, "poll fd %d event signalled\n", fd);
                    scb->signalled = 1;
                    break;
                }
            }

            if (scb->signalled) {
                sem_post(&scb->sem, false);
            }
        }
    }
    mutex_release(&scb_lock);

    return 0;
}

int dcf_select_ms(int maxfd, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, lk_time_t timeout)
{
    struct pollfd *pollset = NULL;
    struct select_cb scb;
    int fd;
    int npfds;
    int ndx;
    int num;

    /* How many pollfd structures do we need to allocate? */
    for (fd = 0, npfds = 0; fd < maxfd; fd++) {
        /* Check if any monitor operation is requested on this fd */
        if ((readfds   && FD_ISSET(fd, readfds))  ||
            (writefds  && FD_ISSET(fd, writefds)) ||
            (exceptfds && FD_ISSET(fd, exceptfds))) {
            npfds++;
        }
    }

    /* Allocate the descriptor list for poll() */
    if (npfds > 0) {
        pollset = (struct pollfd *)calloc(npfds, sizeof(struct pollfd));
        if (!pollset)  {
            return -1;
        }
    } else {
        dprintf(0, "ERROR: no fd in fdset\n");
        return -1;
    }

    /* Initialize the descriptor list for poll() */
    for (fd = 0, ndx = 0; fd < maxfd; fd++) {
        int incr = 0;

        /* The readfs set holds the set of FDs that the caller can be assured
         * of reading from without blocking.  Note that POLLHUP is included as
         * a read-able condition.  POLLHUP will be reported at the end-of-file
         * or when a connection is lost.  In either case, the read() can then
         * be performed without blocking.
         */

        if (readfds && FD_ISSET(fd, readfds)) {
            pollset[ndx].fd         = fd;
            pollset[ndx].events |= POLLIN;
            incr = 1;
        }

        if (writefds && FD_ISSET(fd, writefds)) {
            pollset[ndx].fd      = fd;
            pollset[ndx].events |= POLLOUT;
            incr = 1;
        }

        if (exceptfds && FD_ISSET(fd, exceptfds)) {
            pollset[ndx].fd = fd;
            incr = 1;
        }

        ndx += incr;
    }

    ASSERT(ndx == npfds);

    /* Then let poll do all of the real work. */
//    ret = do_poll(pollset, npfds, &scb);
    num = scan_pollfd(pollset, npfds);
    if (num == 0) {
        if (timeout == 0) {
            goto fail_free_1;
        }

        list_initialize(&scb.node);
        scb.signalled = 0;
        scb.fds = pollset;
        scb.nfds = npfds;
        sem_init(&scb.sem, 0);

        mutex_acquire_timeout(&scb_lock, INFINITE_TIME);
        list_add_tail(&scb_list, &scb.node);
        mutex_release(&scb_lock);

        sem_timedwait(&scb.sem, timeout);

        mutex_acquire_timeout(&scb_lock, INFINITE_TIME);
        list_delete(&scb.node);
        mutex_release(&scb_lock);
        sem_destroy(&scb.sem);
    }

    num = scan_pollfd(pollset, npfds);

    /* Now set up the return values */
    if (readfds) {
        fdszero(readfds, maxfd);
    }

    if (writefds) {
        fdszero(writefds, maxfd);
    }

    if (exceptfds) {
        fdszero(exceptfds, maxfd);
    }

    /* Convert the poll descriptor list back into selects 3 bitsets */
    if (num > 0) {
        num = 0;
        for (ndx = 0; ndx < npfds; ndx++) {
            /* Check for read conditions.  Note that POLLHUP is included as a
             * read condition.  POLLHUP will be reported when no more data will
             * be available (such as when a connection is lost).  In either
             * case, the read() can then be performed without blocking.
             */

            if (readfds) {
                if (pollset[ndx].revents & (POLLIN | POLLHUP)) {
                    FD_SET(pollset[ndx].fd, readfds);
                    num++;
                }
            }

            /* Check for write conditions */
            if (writefds) {
                if (pollset[ndx].revents & POLLOUT) {
                    FD_SET(pollset[ndx].fd, writefds);
                    num++;
                }
            }

            /* Check for exceptions */
            if (exceptfds) {
                if (pollset[ndx].revents & POLLERR) {
                    FD_SET(pollset[ndx].fd, exceptfds);
                    num++;
                }
            }
        }
    }

fail_free_1:

    if (pollset)
        free(pollset);

    return num;
}

int dcf_select(int maxfd, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout)
{
    int msec;

    /* Convert the timeout to milliseconds */
    if (timeout) {
        msec = timeout->tv_sec * 1000 + timeout->tv_usec / 1000;
    }
    else {
        msec = -1;
    }

    return dcf_select_ms(maxfd, readfds, writefds, exceptfds, msec);
}

void dcf_select_init(void)
{
    list_initialize(&scb_list);
    mutex_init(&scb_lock);
}

#endif //if CONFIG_SUPPORT_POSIX

