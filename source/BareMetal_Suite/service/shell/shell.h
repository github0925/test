/********************************************************
 *      Copyright(c) 2020   Semidrive                   *
 *      All rights reserved.                            *
 ********************************************************/

#ifndef __SHELL_H__
#define __SHELL_H__

#include <common_hdr.h>

typedef struct {
    char *cmd_str;
    uint32_t (*func)(uint32_t argc, char *argv[]);
    char *help_str;
} shell_cmd_t;

#define SHELL_CMD(str, func, help)  \
    shell_cmd_t _shell_cmd_##func##_ __attribute__((section("shell_cmd"))) __attribute__((used))= {\
                    str,\
                    func,\
                    help\
                };

void shell_loop(module_e tty);
void shell_loop_usb(void);
#endif  /* __SHELL_H__ */
