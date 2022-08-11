/********************************************************
 *          Copyright(c) 2018   Semidrive               *
 ********************************************************/

#include <common_hdr.h>
#include <soc.h>

#if !defined(SOC_host)
/*
int _read(int file, char *ptr, int len) {
    return len;
}

int _lseek(int file, int ptr, int dir)
{
    return 0;
}

int _open(const char *path, int flags, ...)
{
    return 0;
}

int _close(int file)
{
    return 0;
}
*/

#if !defined(NO_STDLIB)
int _write(int file, char *ptr, int len)
{
    return len;
}

void _exit(int n)
{
    DBG("\n!!!Opps, _exit!!!\n\n");
}

int _kill(int n, int m)
{
    system_reset();

    return 0;
}

int _getpid(int n)
{
    return 1;
}

extern uintptr_t __heap_start[];
extern uintptr_t __stack_start[];

static char *heap_ptr = NULL;

char *_sbrk(int nbytes)
{
    char *base;

    if (NULL == heap_ptr) {
        heap_ptr = (char *)__heap_start;
    }

    base = heap_ptr;
    heap_ptr += nbytes;

    if (heap_ptr < (char *)__stack_start) {
        return base;
    } else {
        return (char *) -1;
    }
}
#endif
#endif  /* SOC_host */
