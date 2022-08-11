/********************************************************
 *          Copyright(c) 2018,2020  Semidrive           *
 *******************************************************/

#include <common_hdr.h>

static inline uint32_t hexchar_to_num(char c)
{
    return  (((c) >= '0' && (c) <= '9') ? (c - '0') : \
             ((c) >= 'a' && (c) <= 'f') ? ((c - 'a') + 10) : \
             ((c) >= 'A' && (c) <= 'F') ? ((c - 'A') + 10) : 0);
}

/* Just as 'strlen', this function takes a null terminated byte string and
 * return its length. The length does not include the null character. */
size_t mini_strlen(const char *str)
{
    size_t sz = 0;

    while (*str++ != '\0') {
        sz++;
    }

    return sz;
}

char *mini_strcpy(char *to, const char *from)
{
    assert((NULL != to) && (NULL != from));

    char *p = to;

    while ('\0' != (*to++ = *from++));

    return p;
}

/* Note: this secure version's return is not aligned to strncmp_s */
int mini_strncmp_s(const char *s1, const char *s2, size_t n)
{
    assert((NULL != s1) && (NULL != s2));

    int res = 0;

    while (n && *s1) {
        res |= (*s1++ != *s2++);
        n--;
    }

    return res;
}

int mini_strcmp_s(const char *s1, const char *s2)
{
    assert((NULL != s1) && (NULL != s2));

    int res = 0;

    while ((*s1 != '\0') && (*s2 != '\0')) {
        res |= (*s1++ != *s2++);
    }

    return res | (*s1 != '\0') | (*s2 != '\0');
}

/* endptr/base ignored, they are taken as NULL/0 */
unsigned long int mini_strtoul(const char *str, char **endptr, int base)
{
    unsigned long int v = 0;
    const char *p = str;
    uint32_t n = 0;
    uint32_t radix = 1;

    while (*p++ != '\0') {
        n++;
    }

    p = str;

    if (n == 1) {   /* if only one digit, it has to be '0 - 9'*/
        if (*p >= '0' && *p <= '9') {
            v = *p - '0';
        }
    } else if ( (n >= 2)) {
        /* '0x' gives 0
         * '12a5' gives 1205
         * 0x12p5 gives 0x1205
         */
        if ((*p == '0') && (*(p + 1) == 'x' || *(p + 1) == 'X')) { /* radix = 16*/
            radix = 16;
            p += 2;
            n -= 2;
        } else {
            radix = 10;
        }

        while (n--) {
            v = radix * v + hexchar_to_num(*p++);
        }
    }

    return v;
}

/* endptr/base ignored, they are taken as NULL/0 */
unsigned long long mini_strtoull(const char *str, char **endptr, int base)
{
    unsigned long long v = 0;
    const char *p = str;
    uint32_t n = 0;
    uint32_t radix = 1;

    while (*p++ != '\0') {
        n++;
    }

    p = str;

    if (n == 1) {   /* if only one digit, it has to be '0 - 9'*/
        if (*p >= '0' && *p <= '9') {
            v = *p - '0';
        }
    } else if ( (n >= 2)) {
        /* '0x' gives 0
         * '12a5' gives 1205
         * 0x12p5 gives 0x1205
         */
        if ((*p == '0') && (*(p + 1) == 'x' || *(p + 1) == 'X')) { /* radix = 16*/
            radix = 16;
            p += 2;
            n -= 2;
        } else {
            radix = 10;
        }

        while (n--) {
            v = radix * v + hexchar_to_num(*p++);
        }
    }

    return v;
}
