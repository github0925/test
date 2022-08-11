/*
 * func_usb.c
 *
 * Copyright (c) 2020 SemiDrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

#include "func_usb.h"
#include "board_diag.h"
#include "debug.h"
#include "cfg.h"

#define USB1_CONNECT_STATE_REG 0x3122c70c
#define USB_REG_MAP_SIZE       0x2000

static bool usb_cmd(test_exec_t *exec)
{
    bool ret = false;
    int usb1_fd;
    unsigned int usb1_state = 0;
    unsigned int pa_offset, offset;

    usb1_fd = open("/dev/mem", O_RDWR | O_NDELAY);

    if (usb1_fd < 0) {
        printf("open(/dev/mem) failed.");
        goto fail;
    }

    offset = USB1_CONNECT_STATE_REG;
    pa_offset = offset & ~(sysconf(_SC_PAGE_SIZE) - 1);

    unsigned char *map_base = (unsigned char * )mmap(NULL,
                              offset + USB_REG_MAP_SIZE - pa_offset, PROT_READ, MAP_SHARED, usb1_fd,
                              pa_offset);

    if (map_base == NULL) {
        printf("MMap sys cnt failed\n");
        goto fail;
    }

    usb1_state = *(volatile unsigned int *)(map_base + offset - pa_offset);

    usb1_state = (usb1_state >> 17) & 0xf;

    if (usb1_state != USB_LINK) {
        usb1_state = USB_BREAK;
    }

    munmap(map_base, offset + USB_REG_MAP_SIZE - pa_offset);

    ret = true;

fail:
    if (usb1_fd >= 0)
        close(usb1_fd);

    set_para_value(exec->resp[1], (uint8_t)usb1_state);
    return ret;
}

bool usb1_reply_deal(test_exec_t *exec, test_state_e state)
{
    bool ret = false;
    uint32_t respCanID;
    CMD_STATUS cmdStatus = CMD_PARA_ERR;

    if (state == STATE_SINGLE) {
        respCanID = SINGLE_RESP;
        ret = usb_cmd(exec);
    }
    else {
        return ret;
    }

    if (ret)
        set_para_value(cmdStatus, NORMAL_DEAL);

    set_para_value(exec->resp[0], cmdStatus);
    common_response(exec, respCanID) ? (ret = true) : (ret = false);
    return ret;
}
