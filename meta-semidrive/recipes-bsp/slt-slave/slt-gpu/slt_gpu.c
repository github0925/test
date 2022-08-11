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
#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <fcntl.h>
#include <string.h>

#include <GLES3/gl3.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>


int Gles3_EglMainArgs(EGLNativeDisplayType eglDisplay, char* result_string);
int Ocl_EglMainArgs(char* result_string);


int slt_start(uint times, uint timeout, char* result_string)
{
    int ret = 0;

    printf(" entry, times= %d, timeout =%d\n", times, timeout);

	ret = Gles3_EglMainArgs(EGL_DEFAULT_DISPLAY, result_string);
    printf("after Gles3_EglMainArgs, ret= 0x%x\n", ret);

	ret |= Ocl_EglMainArgs(result_string);
    printf("after Gles3_EglMainArgs, ret= 0x%x\n", ret);

    return ret;
}

// test case time: run test times in test case, if not support set default 1
// test case timeout: run test timeout default value, must bigger than case us time, or case will be kill
