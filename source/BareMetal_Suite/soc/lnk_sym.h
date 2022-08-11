/********************************************************
 *          Copyright(c) 2018   Semidrive               *
 ********************************************************/

#ifndef __LNK_SYM_H__
#define __LNK_SYM_H__

#if !defined(ASSEMBLY)
#if !defined(__ARMCC_VERSION)
extern uintptr_t __load_ram_func_start[];
extern uintptr_t __ram_func_start[];
extern uintptr_t __ram_func_end[];
extern uintptr_t __load_data_start[];
extern uintptr_t __data_start[];
extern uintptr_t __data_end[];
extern uintptr_t __bss_start[];
extern uintptr_t __bss_end;
extern uintptr_t __heap_start[];
extern uintptr_t __stack_start[];
extern uintptr_t __stack_end[];
extern uintptr_t __bss_2_start[];
extern uintptr_t __bss_2_end[];
extern uintptr_t __start_EARLY_TEST_SECTION[];
extern uintptr_t __stop_EARLY_TEST_SECTION[];
extern uintptr_t __start_TEST_SECTION[];
extern uintptr_t __stop_TEST_SECTION[];
extern uintptr_t __start_POST_TEST_SECTION[];
extern uintptr_t __stop_POST_TEST_SECTION[];
extern uintptr_t __tb_trigger_start[];
extern uintptr_t __tb_trigger_end[];
extern uintptr_t __rom_start[];
extern uintptr_t __rom_end[];
#else
extern uintptr_t Load$$ram_func$$Base[];
extern uintptr_t Image$$ram_func$$Base[];
extern uintptr_t Image$$ram_func$$Limit[];
extern uintptr_t Load$$data$$Base[];
extern uintptr_t Image$$data$$Base[];
extern uintptr_t Image$$data$$Limit[];
extern uintptr_t Image$$bss$$ZI$$Base[];
extern uintptr_t Image$$bss$$ZI$$Limit[];
extern uintptr_t Image$$bss_2$$Base[];
extern uintptr_t Image$$bss_2$$Limit[];
extern uintptr_t Image$$stack$$ZI$$Base[];
extern uintptr_t Image$$stack$$ZI$$Limit[];
extern uintptr_t Image$$tb_trigger$$ZI$$Base[];
extern uintptr_t Image$$tb_trigger$$ZI$$Limit[];
extern uintptr_t Image$$TEST_SECTION$$Base[];
extern uintptr_t Image$$TEST_SECTION$$Limit[];
extern uintptr_t Image$$EARLY_TEST_SECTION$$Base[];
extern uintptr_t Image$$EARLY_TEST_SECTION$$Limit[];
extern uintptr_t Image$$POST_TEST_SECTION$$Base[];
extern uintptr_t Image$$POST_TEST_SECTION$$Limit[];
#endif
#endif

#if defined(__ARMCC_VERSION)
#define __load_ram_func_start   Load$$ram_func$$Base
#define __ram_func_start        Image$$ram_func$$Base
#define __ram_func_end          Image$$ram_func$$Limit
#define __load_data_start       Load$$data$$Base
#define __data_start            Image$$data$$Base
#define __data_end              Image$$data$$Limit
#define __bss_start             Image$$bss$$ZI$$Base
#define __bss_end               Image$$bss$$ZI$$Limit
#define __bss_2_start           Image$$bss_2$$Base
#define __bss_2_end             Image$$bss_2$$Limit
#define __stack_start           Image$$stack$$ZI$$Base
#define __stack_end             Image$$stack$$ZI$$Limit
#define __tb_trigger_start      Image$$tb_trigger$$ZI$$Base
#define __tb_trigger_end        Image$$tb_trigger$$ZI$$Limit
#define __start_EARLY_TEST_SECTION      Image$$EARLY_TEST_SECTION$$Base
#define __stop_EARLY_TEST_SECTION       Image$$EARLY_TEST_SECTION$$Limit
#define __start_TEST_SECTION            Image$$TEST_SECTION$$Base
#define __stop_TEST_SECTION             Image$$TEST_SECTION$$Limit
#define __start_POST_TEST_SECTION       Image$$POST_TEST_SECTION$$Base
#define __stop_POST_TEST_SECTION        Image$$POST_TEST_SECTION$$Limit
#endif

#endif  /* __LNK_SYM_H__ */
