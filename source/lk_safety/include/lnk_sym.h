/********************************************************
 *          Copyright(c) 2018   Semidrive               *
 ********************************************************/

#ifndef __LNK_SYM_H__
#define __LNK_SYM_H__

#ifdef __ICCARM__

#define __fault_handler_table_start     FAULT_TABLE$$Base
#define __fault_handler_table_end       FAULT_TABLE$$Limit
#define __apps_start                    APPS$$Base
#define __apps_end                      APPS$$Limit
#define __commands_start                COMMANDS$$Base
#define __commands_end                  COMMANDS$$Limit
#define _nocacheable_start              FAULT_TABLE$$Base
#define _nocacheable_end                FAULT_TABLE$$Limit
#define __lk_init                       LK_INIT$$Base
#define __lk_init_end                   LK_INIT$$Limit
#define __ctor_list                     INIT_ARRAY$$Base
#define __ctor_end                      INIT_ARRAY$$Limit
#define __heap                          HEAP$$Base
#define __heap_size                     HEAP$$Length
#define __slt_module_test               SLT_MODULE$$Base
#define __slt_module_test_end           SLT_MODULE$$Limit

#endif

#endif  /* __LNK_SYM_H__ */
