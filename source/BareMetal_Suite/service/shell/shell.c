/********************************************************
 *      Copyright(c) 2020   Semidrive                   *
 *      All rights reserved.                            *
 ********************************************************/

#include <soc.h>
#include <uart/uart.h>
#include "mini_libc/mini_libc.h"
#include "shell.h"
#include <stdio.h>
#include "bt_dev/include/usb_if.h"

#define MAX_SHELL_HISTORY   0x4
#define MAX_SHELL_LINE_SZ   0x80
#define MAX_SHELL_ARGC      8u

#define IS_HEX_CHAR(x)   \
        (((x >= 'a') && (x <= 'z'))\
            || ((x >= 'A') && (x <= 'Z'))\
            || ((x >= '0') && (x <= '9')))

extern uintptr_t __start_shell_cmd[];
extern uintptr_t __stop_shell_cmd[];

extern module_e tty;

static char str_buf[MAX_SHELL_HISTORY][MAX_SHELL_LINE_SZ];
static uint32_t history_n = 0;

int32_t shell_parse_run(char *line)
{
    char buf[MAX_SHELL_LINE_SZ];
    strcpy(buf, line);
    char *p = buf;
    char *argv[MAX_SHELL_ARGC];

    for (int i = 0; i < MAX_SHELL_ARGC; i++) {
        argv[i] = NULL;
    }

    bool wd_begin = false;
    uint32_t argc = 0;

    while (*p != '\0') {
        if (IS_HEX_CHAR(*p) || (*p == '_') || (*p == '=')) {
            if (!wd_begin) {
                wd_begin = true;
                argv[argc++] = p;
            }
        } else {
            *p = '\0';
            wd_begin = false;
        }

        p++;
    }

    if (argc < 1) {
        return -1;
    }

    shell_cmd_t *cmd = (shell_cmd_t *)(uintptr_t)__start_shell_cmd;
    bool found = false;

    for (; cmd < (shell_cmd_t *)(uintptr_t)__stop_shell_cmd; cmd++) {
        if (0 == strcmp(argv[0], cmd->cmd_str)) {
            found = true;
            break;
        }
    }

    if (found && (NULL != cmd->func)) {
        return cmd->func(argc, argv);
    } else {
        DBG("\nInvalid commond: %s\n", argv[0]);
        return -2;
    }
}

void shell_loop(module_e tty)
{
    char c;
    int32_t cur_buf_idx = 0;
    int32_t cur_line_pos = 0;

    DBG("\n\r#>");

    while (1) {
        if (0 == uart_rx(tty, (uint8_t *)&c, 1)) {
            if (cur_line_pos < MAX_SHELL_LINE_SZ - 1) {
                str_buf[cur_buf_idx][cur_line_pos++] = c;

                if (IS_HEX_CHAR(c)
                    || (c == 0x0d) || (c == 0x20)   /* CR || Space */
                    || (c == '_') || (c == '=')
                    || (c == 0x08u) || (c == 0x7f)) { /* BS || DEL  */
                    if ((c == 0x08u) || (c == 0x7f)) {
                        if (cur_line_pos > 0) {
                            cur_line_pos--;
                            c = 0x08;
                            uart_tx(tty, (uint8_t *)&c, 1);
                        }
                    } else {
                        uart_tx(tty, (uint8_t *)&c, 1);   /* echo */
                    }
                }

                if (c == 0x0du) {
                    DBG("\n");
                    str_buf[cur_buf_idx][cur_line_pos + 1] = '\0';

                    if (0 == shell_parse_run(&str_buf[cur_buf_idx][0])) {
                        history_n++;
                        cur_buf_idx++;

                        if (cur_buf_idx >= MAX_SHELL_HISTORY) {
                            cur_buf_idx = 0u;
                        }
                    }

                    cur_line_pos = 0;
                    DBG("\n\r#>");
                }

#if defined(SHELL_UP_DOWN)
                else if (c == 38u) {   /* up */
                    if (history_n > 1) {
                        cur_buf_idx--;

                        if (cur_buf_idx < 0) {
                            cur_buf_idx = history_n < MAX_SHELL_HISTORY ? history - 1 : MAX_SHELL_HISTORY - 1;
                        }

                        DBG("#>%s", &str_buf[cur_buf_idx][0]);
                    }
                } else if (c == 40u) {  /* down */
                    if (history_n > 1) {
                        cur_buf_idx++;
                        cur_buf_idx = cur_buf_idx % MAX_SHELL_HISTORY;
                        DBG("%s", &str_buf[cur_buf_idx][0]);
                    }
                }

#endif
            }
        }
    }
}

void shell_loop_usb(void)
{
    int len;
    int32_t cur_buf_idx = 0;
    int32_t cur_line_pos = 0;
    DBG("\n\r#>");

    while (1) {
        len = usb_recv((U8*)&str_buf[cur_buf_idx][cur_line_pos],(MAX_SHELL_LINE_SZ-cur_line_pos>=64)?64:MAX_SHELL_LINE_SZ-cur_line_pos);
        if(len > 1){
            usb_send((u8*)&str_buf[cur_buf_idx][cur_line_pos],len-1);

            if (str_buf[cur_buf_idx][len-1] == 0x0d) {
                DBG("\n");
                str_buf[cur_buf_idx][len] = '\0';
                
                if (0 == shell_parse_run(&str_buf[cur_buf_idx][0])) {
                    history_n++;
                    cur_buf_idx++;

                    if (cur_buf_idx >= MAX_SHELL_HISTORY) {
                        cur_buf_idx = 0u;
                    }
                }
                DBG("\n\r#>");
            }else if(cur_line_pos < MAX_SHELL_LINE_SZ - 1 ){
                cur_line_pos += len-1;
            }
        }
        else if(len==1){
            if(str_buf[cur_buf_idx][0] == 0x0d){
                DBG("\n\r#>");
                cur_line_pos = 0;
            }
#if defined(SHELL_UP_DOWN)
            if (str_buf[cur_buf_idx][0] == 38u) {   /* up */
                if (history_n > 1) {
                    cur_buf_idx--;

                    if (cur_buf_idx < 0) {
                        cur_buf_idx = history_n < MAX_SHELL_HISTORY ? history - 1 : MAX_SHELL_HISTORY - 1;
                    }

                    DBG("#>%s", &str_buf[cur_buf_idx][0]);
                }
            } else if (str_buf[cur_buf_idx][0] == 40u) {  /* down */
                if (history_n > 1) {
                    cur_buf_idx++;
                    cur_buf_idx = cur_buf_idx % MAX_SHELL_HISTORY;
                    DBG("%s", &str_buf[cur_buf_idx][0]);
                }
            }
#endif
        }
    }
}
uint32_t help(uint32_t argc, char *argv[])
{
    shell_cmd_t *cmd = (shell_cmd_t *)(uintptr_t)__start_shell_cmd;

    if (1u == argc) {
        DBG("Command List\n");

        for (; cmd < (shell_cmd_t *)(uintptr_t)__stop_shell_cmd; cmd++) {
            DBG("%s\n", cmd->cmd_str);
        }

        DBG("Usage: help [cmd]\n");
        return 0;
    }

    bool found = false;

    for (; cmd < (shell_cmd_t *)(uintptr_t)__stop_shell_cmd; cmd++) {
        if (0 == strcmp(argv[1], cmd->cmd_str)) {
            found = true;
            break;
        }
    }

    if (found && (NULL != cmd->help_str)) {
        DBG("%s\n", cmd->help_str);
    }

    return 0;
}

uint32_t cmd_rd32(uint32_t argc, char *argv[])
{
    if (argc < 2 || argc > 3) {
        DBG("Usage: rd32 addr [words]\n");
        return 0;
    }

    uint32_t words = 1;
    unsigned long int addr = strtoul(argv[1], NULL, 0);

    if (argc == 3) {
        words = strtoul(argv[2], NULL, 0);
    }

    volatile uint32_t *p = (volatile uint32_t *)(addr_t)addr;

    arch_clean_invalidate_cache_range((const void *)p, words * sizeof(uint32_t));

    if (NULL != p) {
        while (words) {
            DBG("%p: ", p);

            for (int j = 0; j < MIN(words, 4); j++) {
                DBG("0x%08x ", *p++);
            }

            DBG("\n");
            words -= MIN(words, 4);
        }
    }

    return 0;
}

uint32_t cmd_wr32(uint32_t argc, char *argv[])
{
    if (argc != 3) {
        DBG("Usage: wr32 addr val\n");
        return 0;
    }

    unsigned long int addr = strtoul(argv[1], NULL, 0);
    uint32_t val = strtoul(argv[2], NULL, 0);
    volatile uint32_t *p = (volatile uint32_t *)(addr_t)addr;
    *p = val;
    arch_clean_invalidate_cache_range((const void *)p, sizeof(uint32_t));

    return 0;
}

SHELL_CMD("help", help, "Shell help")
SHELL_CMD("wr32", cmd_wr32, "Usage: wr32 addr val")
SHELL_CMD("rd32", cmd_rd32, "Usage: rd32 addr [words]")
