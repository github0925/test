#include "lvgl_gui.h"

#include <debug.h>
#include <stdio.h>
#include <stdlib.h>

#include "lv_examples.h"

#if defined(WITH_LIB_CONSOLE)

//#if LK_DEBUGLEVEL > 1
#if 1

#include <lib/console.h>
static int cmd_lv_demo(int argc, const cmd_args *argv);
STATIC_COMMAND_START
STATIC_COMMAND("lv_demo", "lvgl example release v7", &cmd_lv_demo)
STATIC_COMMAND_END(lv_demo);
typedef void (*func_t)(void);

/**
 * Create a simple 'Hello world!' label
 */
static void lv_tutorial_hello_world(void)
{
    lv_disp_t *main_disp = lv_disp_get_next(NULL); //first display
    lv_disp_t *sub_disp = lv_disp_get_next(main_disp);
    lv_disp_set_default(main_disp);
    lv_demo_widgets();
    lv_disp_set_default(sub_disp);
    lv_demo_stress();

}

#define NUM_CASES 20
struct test_case_ {
    const char *desc;
    func_t func;
} lv_test_cases[NUM_CASES] = {
    {"lv_tutorial_hello_world", lv_tutorial_hello_world},
    {"lv_ex_get_started_1", lv_ex_get_started_1},
    {"lv_ex_get_started_2", lv_ex_get_started_2},
    {"lv_ex_get_started_3", lv_ex_get_started_3},
    {"lv_demo_widgets", lv_demo_widgets},
    {"lv_demo_stress", lv_demo_stress}

};

static int usage(void) {
    int i = 0;
    int n;
    for (i = 0; i < NUM_CASES; i++) {
        if (lv_test_cases[i].desc == NULL)
            break;
    }
    n = i;
    printf("check lvgl version: %d.%d\n", LVGL_VERSION_MAJOR, LVGL_VERSION_MINOR);
    printf("have %d test case:\n", n);
    for (i = 0; i < n; i++) {
        printf("    %d. %s\n", i, lv_test_cases[i].desc);
    }
    return n;
}


static int cmd_lv_demo(int argc, const cmd_args *argv) {
    int id = -1;
    func_t case_func = NULL;
    int case_num;

    case_num = usage();
    if (argc < 2) {
        char ch;
        printf("Choose number of test case: ");
        ch = getchar();
        id = atoi(&ch);
    }

    if (argc == 2) {
        id = atoi(argv[1].str);
    }
    if (id > case_num && id <= 0) {
        printf("you choose invalid number: %d\n", id);
        return -2;
    }
    case_func = lv_test_cases[id].func;
    lvgl_init();
    case_func();

    lvgl_mainloop();

    return 0;
}

#endif
#endif
