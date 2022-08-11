/*
 * Copyright (c) 2019, SemiDrive, Inc. All rights reserved
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
/*
 * All unit tests get registered here.  A call to run_all_tests() will run
 * them and provide results.
 */
#include <sdunittest.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <lib/console.h>

static struct test_suite_element *test_suite_list = NULL;

static struct test_command_element *test_command_list = NULL;
static struct test_command_element *failed_test_command_list = NULL;

static unsigned int n_tests = 0;
static unsigned int n_success = 0;
static unsigned int n_failed  = 0;

struct test_suite_element * find_test_suite(const char * suitename)
{
    struct test_suite_element *current = test_suite_list;

    while (current) {
        if (strcmp(suitename,current->name) == 0)
        {
            return current;
        }
        current = current->next;
    }

    if ( !current)
    {
        struct test_suite_element *pSuite = (struct test_suite_element *)malloc(sizeof(struct test_suite_element ));
        char * pName= (char *) malloc(strlen(suitename)+1);
        strcpy(pName , suitename);
        pSuite->name = pName;
        pSuite->next = test_suite_list ;
        test_suite_list = pSuite;

        return pSuite;
    }
    return NULL;
}

/*
 * Registers a test case with the unit test framework.
 */
void sdunittest_register_test_command(struct test_command_element *elem)
{
    DEBUG_ASSERT(elem);
    DEBUG_ASSERT(elem->next == NULL);
    elem->testsuite = find_test_suite(elem->suitename);
    elem->next = test_command_list;
    test_command_list = elem;
}

void print_brief_test_report(void)
{
    printf("\n====================================================\n");
    printf  ("    CASES:  %d     SUCCESS:  %d     FAILED:  %d   ",
                      n_tests, n_success, n_failed);
    printf("\n====================================================\n");
    if (n_failed>0)
    {
        printf("\n=================Fail Case List=====================\n");
        struct test_command_element *failed = failed_test_command_list;
        int i=0;
        while (failed) {
            struct test_command_element *failed_next = failed->failed_next;
            printf("CASE %d:%s\n",i,failed->name);
            failed = failed_next;
            i++;
        }
        printf("\n====================================================\n");
    }

}

void clean_fail_case_list(void)
{
    struct test_command_element *failed = failed_test_command_list;
    while (failed) {
        struct test_command_element *failed_next = failed->failed_next;
        failed->failed_next = NULL;
        failed = failed_next;
    }
    failed_test_command_list = NULL;
}

bool invoke_shell_command(const char *name,const char * cmdline)
{
    /*
     * Call shell command
     */
    printf("Begin to run test command %s:%s\n",name,cmdline);
    bool ret= console_run_script_locked(cmdline);
    printf("Finish run test command %s:ret=%d\n",name,ret);
    return ret;
}

/*
 * Runs all registered test cases.
 */
bool run_all_testcommands(void)
{
    n_tests   = 0;
    n_success = 0;
    n_failed  = 0;

    bool all_success = true;
    printf("Begin to run all test command...");
    printf("\n====================================================\n");
    struct test_command_element *current = test_command_list;
    while (current) {
        if (!invoke_shell_command(current->name,current->cmdline)) {
            current->failed_next = failed_test_command_list;
            failed_test_command_list = current;
            all_success = false;
        }
        current = current->next;
        n_tests++;
    }

    if (all_success) {
        n_success = n_tests;
        printf("SUCCESS!  All test cases passed!\n");
    }
    else {
        struct test_command_element *failed = failed_test_command_list;
        while (failed) {
            struct test_command_element *failed_next =
                    failed->failed_next;
            failed = failed_next;
            n_failed++;
        }
        n_success = n_tests - n_failed;
    }
    print_brief_test_report();
    clean_fail_case_list();
    return all_success;
}


bool run_one_testsuite(const char *suitename)
{
    n_tests   = 0;
    n_success = 0;
    n_failed  = 0;

    bool all_success = true;
    struct test_command_element *current = test_command_list;

    while (current) {
        if (strcmp(current->testsuite->name,suitename)==0)
        {
            if (!invoke_shell_command(current->name,current->cmdline)) {
                current->failed_next = failed_test_command_list;
                failed_test_command_list = current;
                all_success = false;
            }
            n_tests++;
        }
        current = current->next;
    }

    if (all_success) {
        n_success = n_tests;
        printf("SUCCESS!  All test cases passed!\n");
    }
    else {
        struct test_command_element *failed = failed_test_command_list;
        while (failed) {
            struct test_command_element *failed_next =
                    failed->failed_next;
            failed = failed_next;
            n_failed++;
        }
        n_success = n_tests - n_failed;
    }
    print_brief_test_report();
    clean_fail_case_list();
    return all_success;
}

bool run_one_testcommand(const char *name)
{
    n_tests   = 0;
    n_success = 0;
    n_failed  = 0;

    bool all_success = true;
    n_tests ++ ;

    struct test_command_element *current = test_command_list;

    while (current) {

        if (strcmp(name,current->name)==0)
            break;
        current = current->next;
    }

    if (!current) {
        printf("No such test command registered!\n");
        return false;
    }

    if ( !invoke_shell_command(current->name,current->cmdline)) {
        current->failed_next = failed_test_command_list;
        failed_test_command_list = current;
        all_success = false;
    }
    if (all_success) {
        n_success = n_tests;
        printf("SUCCESS!  All test cases passed!\n");
    }
    else {
        struct test_command_element *failed = failed_test_command_list;
        while (failed) {
            struct test_command_element *failed_next =
                    failed->failed_next;
            failed = failed_next;
            n_failed++;
        }
        n_success = n_tests - n_failed;
    }

    print_brief_test_report();
    clean_fail_case_list();
    return all_success;
}

bool list_all_registered_testcomands(void)
{
    struct test_command_element *current = test_command_list;
    printf("\n====================================================\n");
    printf("TestCommand\tTestSuite\tCommandLine\n");
    while (current) {
        printf("%s\t\t%s\t%s\n",current->name,current->suitename,
                current->cmdline);
        current = current->next;
    }
    printf("\n====================================================\n");

    return true;
}


bool list_all_registered_testsuites(void)
{
    struct test_suite_element *current = test_suite_list;
    int i=0;
    printf("\n====================================================\n");
    printf("Number\t\tName\n");
    while (current) {
        i++;
        printf("%03d\t\t%s\n",i,current->name);
        current = current->next;
    }

    printf("\n====================================================\n");
    return true;
}


bool list_all_testcommands_in_testsuite(const char * name)
{
    struct test_command_element *current = test_command_list;
    printf("\n****************************************************\n");
    printf("TestSuite Name:%s\n",name);
    printf("\n====================================================\n");
    printf("TestCommand\tTestSuite\tCommandLine\n");
    while (current) {
        if (strcmp(current->testsuite->name,name)==0)
        {
            printf("%s\t%s\t%s\n",current->name,current->testsuite->name,
                    current->cmdline);
        }
        current = current->next;
    }
    printf("\n====================================================\n");

    return true;
}

bool usage_sdunittest(void)
{
    printf("Usage: sdunittest\n");
    printf("           [-list_testcommand] \n");
    printf("           [-list_testsuite] \n");
    printf("           [-help]  \n");
    printf("           [-run_all]  \n");
    printf("           [-info_testsuite <suite name>] \n");
    printf("           [-run_testsuite <suite name>] \n");
    printf("           [-run_testcommand <command name>] \n");
    return true;
}

int do_sdunittest(int argc, const cmd_args *argv)
{
    bool success = true;
    if (argc == 1) {
        success = usage_sdunittest();
    }
    else if (argc ==2 ){
        if (strcmp(argv[1].str,"-list_testsuite")==0)
        {
            success=list_all_registered_testsuites();
        }
        else if (strcmp(argv[1].str,"-list_testcommand")==0)
        {
            success=list_all_registered_testcomands();
        }
        else if (strcmp(argv[1].str,"-run_all")==0)
        {
            success=run_all_testcommands();
        }
        else if (strcmp(argv[1].str,"-help")==0)
        {
            success=usage_sdunittest();
        }
    }
    else if (argc ==3 ){
        if (strcmp(argv[1].str,"-info_testsuite")==0)
        {
            success=list_all_testcommands_in_testsuite(argv[2].str);
        }
        else if (strcmp(argv[1].str,"-run_testsuite")==0)
        {
            success=run_one_testsuite(argv[2].str);
        }
        else if (strcmp(argv[1].str,"-run_testcommand")==0)
        {
            success=run_one_testcommand(argv[2].str);
        }
    }

    return (int)success;
}

#if defined(WITH_LIB_CONSOLE)
STATIC_COMMAND_START
STATIC_COMMAND("sdunittest", "sd unit test: run test command \n", (console_cmd)&do_sdunittest)
STATIC_COMMAND_END(unittest);
#endif
