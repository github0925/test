/*
 * Copyright (c) 2019, Semidrive, Inc. All rights reserved
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#ifndef _LIB_SDUNITTEST_INCLUDE_UNITTEST_H_
#define _LIB_SDUNITTEST_INCLUDE_UNITTEST_H_
/*
 * Macros for writing unit testsuites .
 *
 * Sample usage:
 *
 * A test case runs a collection of tests like this, with
 * BEGIN_TEST_CASE and END_TEST_CASE and the beginning and end of the
 * function and RUN_TEST to call each individual test, as follows:
 *
 *  BEGIN_TEST_CASE(foo_tests);
 *
 *  RUN_TEST(test_foo);
 *  RUN_TEST(test_bar);
 *  RUN_TEST(test_baz);
 *
 *  END_TEST_CASE;
 *
 * This creates a static function foo_tests() and registers it with the
 * unit test framework.  foo_tests() can be executed either by a shell
 * command or by a call to run_all_tests(), which runs all registered
 * unit tests.
 *
 * A test looks like this, using the BEGIN_TEST and END_TEST macros at
 * the beginning and end of the test and the EXPECT_* macros to
 * validate test results, as shown:
 *
 * static bool test_foo(void)
 * {
 *      BEGIN_TEST;
 *
 *      ...declare variables and do stuff...
 *      int foo_value = foo_func();
 *      ...See if the stuff produced the correct value...
 *      EXPECT_EQ(1, foo_value, "foo_func failed");
 *      ... there are EXPECT_* macros for many conditions...
 *      EXPECT_TRUE(foo_condition(), "condition should be true");
 *      EXPECT_NEQ(ERR_TIMED_OUT, foo_event(), "event timed out");
 *
 *      END_TEST;
 * }
 *
 * To your rules.mk file, add lib/unittest to MODULE_DEPS:
 *
 * MODULE_DEPS += \
 *         lib/unittest   \
 */
#include <stdbool.h>
#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <trace.h>
#include <stdarg.h>

/*
 * The list of test suite is made up of these elements.
 */

struct test_suite_element {
    struct test_suite_element *next;
    const char *name;
};

/*
 * The list of test command is made up of these elements.
 */
struct test_command_element {
    struct test_command_element *next;
    struct test_command_element *failed_next;
    struct test_suite_element *testsuite;
    const char *name;
    const char *suitename;
    const char *cmdline;
};

#define DEFINE_REGISTER_TEST_COMMAND(case_name,suite_name,cmd_line)                   \
    static struct test_command_element _##case_name##_element = {                   \
        .next = NULL,                                                               \
        .failed_next = NULL,                                                        \
        .testsuite = NULL,                                                          \
        .name = #case_name,                                                         \
        .suitename = #suite_name,                                                   \
        .cmdline= #cmd_line,                                                         \
    };                                                                              \
    static void _register_testcommand_##case_name(void)                             \
    {                                                                               \
        sdunittest_register_test_command(&_##case_name##_element);                  \
    }                                                                               \
    void (*_register_testcommand_##case_name##_ptr)(void) __SECTION(".ctors") =     \
        _register_testcommand_##case_name;


/*
 * Registers a test case with the unit test framework.
 */
void sdunittest_register_test_command(struct test_command_element *elem);

/*
 * Runs all registered test cases.
 */
bool run_all_testcommands(void);
/*
 * Runs one registered test case.
 */
bool run_one_testcommand(const char *name);
/*
 * Runs one registered test case.
 */
bool run_one_testsuite(const char *suitename);


/*
 * List all registered test commands.
 */
bool list_all_registered_testcomands(void);

/*
 * List all registered test commands.
 */
bool list_all_registered_testsuites(void);

/*
 * List all registered test commands in one testsuite.
 */
bool list_all_testcommands_in_testsuite(const char * name);



#endif  /* _LIB_SDUNITTEST_INCLUDE_UNITTEST_H_ */
