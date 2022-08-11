/********************************************************
 *          Copyright(c) 2018   Semidrive               *
 *******************************************************/

/**
 * @file    testbench.c
 * @brief   routines to access test bench features
 */

#include <stdarg.h>
#include <common_hdr.h>
#include <soc.h>
#include <arch.h>
#include "testbench.h"

#if defined(CFG_TB_SYS_TB_CTRL_EN)
void tb_set_msg_severity(U32 severity)
{
    writel(SYS_TB_CTRL_BASE_ADDR + 0x4, severity);
}

void tb_stop_simulation(void)
{
    writel(SYS_TB_CTRL_BASE_ADDR + 0x0, 10);
}

void tb_start_dump(void)
{
    writel(SYS_TB_CTRL_BASE_ADDR + 0x1c, 1);
}

void tb_stop_dump(void)
{
    writel(SYS_TB_CTRL_BASE_ADDR + 0x1c, 0);
}

void tb_putchar(U8 c)
{
    writel(SYS_TB_CTRL_BASE_ADDR + 0x08, (U32)c);
}

#elif defined(CFG_TB_SYS_BOOT_MON_EN)

void tb_stop_simulation(void)
{
    *(volatile U32 *)TB_TRIG_CMD_ADDR = TB_CMD_FINISH;
    dsb();
}

void tb_cmd(U32 cmd, U32 arg1, U32 arg2)
{
    *(volatile U32 *)TB_TRIG_ARG1_ADDR = arg1;
    *(volatile U32 *)TB_TRIG_ARG2_ADDR = arg2;
    dsb();
    *(volatile U32 *)TB_TRIG_CMD_ADDR = cmd;
    dsb();
}

void tb_trigger(U32 trig)
{
    *(volatile U32 *)TB_TRIG_CMD_ADDR = trig;
    dsb();
}

void ramfunc_tb_stop_simulation(void) __RAM_FUNC__;
void ramfunc_tb_stop_simulation(void)
{
    *(volatile U32 *)TB_TRIG_CMD_ADDR = TB_CMD_FINISH;
    dsb();
}

#if defined(CFG_TB_API_ramfunc_tb_trigger)
void ramfunc_tb_trigger(U32 trig) __RAM_FUNC__;
void ramfunc_tb_trigger(U32 trig)
{
    *(volatile U32 *)TB_TRIG_CMD_ADDR = trig;
    dsb();
}
#endif

void tb_putchar(U8 c)
{

}

int tb_printf(const char *fmt, ...)
{
#if defined(__ARMCC_VERSION)
    /* armclang does not pass the 'printf' arguments by stack */
    uintptr_t va[9];
    va[0] = (U32)(uintptr_t)fmt;
    va_list args;
    va_start(args, fmt);

    for (int i = 1; i < ARRAY_SZ(va); i++) {
        va[i] = va_arg(args, uintptr_t);
    }

    va_end(args);

    *(volatile U32 *)TB_TRIG_ARG1_ADDR = (U32)(uintptr_t)va;
#else
    /* the implement above is a more common one per compatibility perspective,
     * but has performance penalty */
    *(volatile U32 *)TB_TRIG_ARG1_ADDR = (U32)(uintptr_t)&fmt;
#endif
    dsb();
    *(volatile U32 *)TB_TRIG_CMD_ADDR = TB_CMD_PRINTF;
    dsb();

    return 0;
}

int ramfunc_tb_printf(const char *fmt, ...) __RAM_FUNC__;
int ramfunc_tb_printf(const char *fmt, ...)
{
#if defined(__ARMCC_VERSION)
    /* armclang does not pass the 'printf' arguments by stack */
    uintptr_t va[9];
    va[0] = (U32)(uintptr_t)fmt;
    va_list args;
    va_start(args, fmt);

    for (int i = 1; i < ARRAY_SZ(va); i++) {
        va[i] = va_arg(args, uintptr_t);
    }

    va_end(args);

    *(volatile U32 *)TB_TRIG_ARG1_ADDR = (U32)(uintptr_t)va;
#else
    *(volatile U32 *)TB_TRIG_ARG1_ADDR = (U32)(uintptr_t)&fmt;
#endif
    dsb();
    *(volatile U32 *)TB_TRIG_CMD_ADDR = TB_CMD_PRINTF;
    dsb();

    return 0;
}

int tb_bprintf(const char *fmt, ...)
{
    va_list args;
    int num = 0, v = 0;
    char *str = NULL;
    uintptr_t vbuf[16];
    char xprntf_buf[256];

    char *f = strcpy(xprntf_buf, fmt);
    vbuf[num++] = (uintptr_t)f;
    f += strlen(f) + 1;

    const char *p = fmt;
    va_start(args, fmt);

    for (p = fmt; *p; p++) {
        if (*p != '%') {
            continue;
        }

        p++;

        if (*p >= '0' && *p <= '9') {
            p++;

            if (*p >= '0' && *p <= '9') {
                p++;
            }
        }

        switch (*p) {
        case 'd':
        case 'x':
        case 'c':
            v = va_arg(args, int);
            vbuf[num++] = v;
            break;

        case 's':
            str = va_arg(args, char *);
            f = strcpy(f, str);
            vbuf[num++] = (uintptr_t)f;
            f += strlen(f) + 1;
            break;

        default:
            break;
        }
    }

    va_end(args);
    tb_cmd(TB_CMD_PRINTF, (U32)(uintptr_t)vbuf, 0);

    return 0;
}
#endif
