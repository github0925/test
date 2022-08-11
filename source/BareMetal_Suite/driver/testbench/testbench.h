/********************************************************
 *          Copyright(c) 2018   Semidrive               *
 *******************************************************/

/**
 * @file    testbench.h
 * @brief   header file for ATB access RTL test bench
 */

#ifndef __TESTBENCH_H__
#define __TESTBENCH_H__

#if defined(CFG_TB_SYS_TB_CTRL_EN) || defined(VTEST)
/* The below macros need be aligned with test bench */
#define UVM_INFO    0x0
#define UVM_WARNING 0x1
#define UVM_ERROR   0x2
#define UVM_FATAL   0x3
#endif

#if defined(CFG_TB_SYS_TB_CTRL_EN)
/* macros for ATB reference */
#define TB_MSG_INFO      UVM_INFO
#define TB_MSG_WARNING   UVM_WARNING
#define TB_MSG_ERROR     UVM_ERROR
#define TB_MSG_FATAL     UVM_FATAL

void tb_set_msg_severity(U32 severity);
void tb_start_dump(void);
void tb_stop_dump(void);
void tb_putchar(U8 c);
#endif  /* CFG_TB_SYS_TB_CTRL_EN */

void tb_stop_simulation(void);

#if defined(CFG_TB_SYS_BOOT_MON_EN)
int tb_printf(const char *fmt, ...);
int tb_bprintf(const char *fmt, ...);
void tb_trigger(U32 trig);
void tb_cmd(U32 cmd, U32 arg1, U32 arg2);
void ramfunc_tb_stop_simulation(void);
void ramfunc_tb_trigger(U32 trig);
int ramfunc_tb_printf(const char *fmt, ...);
#endif

#if defined(VTEST)
extern void sys_tb_ctrl_stop_sim(U32);
extern void send_message(char *, U32 type);
#define TB_START_DUMP()
#define TB_STOP_DUMP()
#define TB_STOP_SIM()           \
        do {\
            send_message("Sim stop", UVM_INFO);\
            sys_tb_ctrl_stop_sim(1);\
        } while (0)
#define TB_TRIGGER(trig)    send_message("event triggered", UVM_INFO)
#define TB_ERROR()              send_message("Error!!!", UVM_FATAL)
#define TB_ERROR_THEN_STOP()    do {TB_ERROR(); TB_STOP_SIM();} while(0)
#define TB_CMD(cmd, arg1, arg2)
#elif defined(CFG_TB_SYS_TB_CTRL_EN)
#define TB_START_DUMP()     tb_start_dump()
#define TB_STOP_DUMP()      tb_stop_dump()
#define TB_STOP_SIM()       tb_stop_simulation()
#define TB_ERROR()  \
            do {\
                tb_set_msg_severity(TB_MSG_FATAL);\
                tb_putchar('\n');\
            } while(0)
#define TB_ERROR_THEN_STOP()    do {TB_ERROR(); TB_STOP_SIM();} while(0)
#define TB_CMD(cmd, arg1, arg2)
#elif defined(CFG_TB_SYS_BOOT_MON_EN)
#define TB_CMD_PRINTF       0x01U
#define TB_CMD_FINISH       0xFFU
#define TB_CMD_ERROR        0xFEU
#define TB_CMD_EXCEPTION    0xFDU
#define TB_CMD_DEBUG_ENTER  0xFC
#define TB_CMD_NORMAL_BT_FAIL   0xFB
#define TB_CMD_USB_BT_ENTER     0xFA
#define TB_CMD_MEM_DUMP     0xF9
#define TB_STOP_SIM()       tb_stop_simulation()
#define TB_TRIGGER(trig)    tb_trigger(trig)
#define TB_ERROR()          TB_TRIGGER(TB_CMD_ERROR)
#define TB_ERROR_THEN_STOP() \
                do {TB_ERROR(); TB_STOP_SIM();} while(0)
#define TB_CMD(cmd, arg1, arg2)     tb_cmd(cmd, arg1, arg2)
#else
#define TB_TRIGGER(trig)
#define TB_ERROR()
#if defined(SOC_host)
#define TB_STOP_SIM()           exit(0)
#define TB_ERROR_THEN_STOP()    exit(-1)
#else
#define TB_STOP_SIM()
#define TB_ERROR_THEN_STOP()
#endif
#define TB_CMD(cmd, arg1, arg2)
#endif

#if defined(CFG_TB_SYS_BOOT_MON_EN)
#define RAMFUNC_TB_STOP_SIM()   ramfunc_tb_stop_simulation()
#else
#define RAMFUNC_TB_STOP_SIM()   TB_STOP_SIM()
#endif

#endif // __TESTBENCH_H__
