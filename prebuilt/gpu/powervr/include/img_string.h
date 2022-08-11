/*************************************************************************/ /*!
@Title          Growable or fixed-size string buffers
@Copyright      Copyright (c) Imagination Technologies Ltd. All Rights Reserved
@License        Strictly Confidential.
*/ /**************************************************************************/

#include <stdlib.h>
#include <stdarg.h>
#include "img_defs.h"

#ifndef IMGSTRING_H
#define IMGSTRING_H

#ifdef __cplusplus
extern "C" {
#endif

/* This file provides a simple string struct and some functions to print
 * into it. It supports dynamically growing a string, or using a fixed-size
 * char array.
 *
 * Example of a non-growable string:
 *
 *   char buf[1024];
 *   string_t str = STRING_FROM_ARRAY(buf);
 *   string_append(&str, "%s", args);  // truncates if not enough space
 *   do_something_with(string_ptr(&str));
 *
 * Growable string:
 *
 *   string_t str = STRING_ALLOC(128);
 *   string_append(&str, "%s", args);  // grows string to fit
 *   do_something_with(string_ptr(&str));
 *   string_free(&str);
 *
 * To declare a string and create it later:
 *
 *   string_t str;
 *   // [...]
 *   string_alloc(&str, 128);
 *   do_something_with(&str);
 *   string_free(&str);
 */

typedef enum
{
	STRING_FIXED_SIZE,
	STRING_GROWABLE,
} string_type_t;

typedef struct
{
	char *buf;
	size_t size;
	size_t used;
	string_type_t type;
} string_t;

/*! Initialise a growable string with the specified initial length. */
#define STRING_ALLOC(len) _string_alloc_initializer(len)

/*! Initialise a fixed-size string from a char array. */
#define STRING_FROM_ARRAY(array) _string_array_initializer((array), sizeof(array))

/*! Initialise a string with a pointer that could come from anywhere. The
 *  last argument should be STRING_FIXED_SIZE or STRING_GROWABLE. If
 *  STRING_GROWABLE is used, realloc() will be called on the supplied
 *  pointer if the string runs out of space.
 */
#define STRING_INIT(ptr, len, type) _string_init_initializer((ptr), (len), (type))

/*! Initialise a growable string with no memory. Memory will be allocated on
 *  the first string_append().
 */
#define STRING_NULL {NULL, 0, 0, STRING_GROWABLE}

/*! Allocate a growable string. This is a non-initialiser version of
 *  STRING_ALLOC, and can be used anywhere.
 *
 * @Input    str        String to allocate space for.
 * @Input    size       Initial size of the string.
 * @Return              0 if size > 0 but malloc failed, 1 otherwise.
 */
int string_alloc(string_t *str, size_t size);

/*! Create a string from an existing pointer. This is a non-initialiser
 *  equivalent of STRING_INIT, and can be used anywhere.
 *
 * @Input    str        String to initialise.
 * @Input    ptr        Pointer to use as the string data.
 * @Input    size       Size of the memory area that ptr points to.
 * @Input    type       STRING_FIXED_SIZE or STRING_GROWABLE. If
 *                      STRING_GROWABLE, realloc() will be called on 'ptr'
 *                      when the string runs out of space.
 */
void string_init(string_t *str, char *ptr, size_t size, string_type_t type);

/*! Free a string created with STRING_ALLOC or string_alloc.
 *
 * @Input    str        String to free.
 */
void string_free(string_t *str);

/*! Append to the end of a string.
 *
 * If the string is growable, this function will try to grow the string to
 * the right size if it runs out of space.
 *
 * @Input    str        String to append to
 * @Input    fmt        printf format string
 * @Input    ...        Format string arguments
 * @Return              Return value from the underlying vsnprintf() call.
 *                      Usually this is the size of the string written,
 *                      excluding the terminating null byte. If the buffer
 *                      was too small, most C libraries return the number of
 *                      characters that would have been written if the
 *                      buffer was large enough.
 */
int string_append(string_t *str, const char *fmt, ...)
	__printf(2, 3);

/*! Append to the end of a string, using arguments from a va_list.
 *
 * If the string is growable, this function will try to grow the string to
 * the right size if it runs out of space.
 *
 * @Input    str        String to append to
 * @Input    fmt        printf format string
 * @Input    args       Format string arguments
 * @Return              Return value from the underlying vsnprintf() call.
 *                      Usually this is the size of the string written,
 *                      excluding the terminating null byte. If the buffer
 *                      was too small, most C libraries return the number of
 *                      characters that would have been written if the
 *                      buffer was large enough.
 */
int string_vappend(string_t *str, const char *fmt, va_list args)
	__printf(2, 0);

/*! Append to the end of a string, using arguments from a va_list.
 *
 * Note: This function will not grow the string if it runs out of space.
 *
 * @Input    str        String to append to
 * @Input    fmt        printf format string
 * @Input    args       va_list containing arguments
 * @Return              Return value from the underlying vsnprintf() call.
 */
int string_raw_vappend(string_t *str, const char *fmt, va_list args)
	__printf(2, 0);

/*! Rewind the current position back to the start of the string. The next
 *  string_append() will start at the beginning.
 */
void string_rewind(string_t *str);

/* Return the underlying char pointer. */
#define string_ptr(str) ((str)->buf)

/* Return the underlying string length (used length) */
#define string_len(str) ((str)->used)

/* Returns true if the last string_append() to this string was truncated. */
#define string_truncated(str) ((str)->used == (str)->size)

/* Returns true if the string is currently full.
 *
 * string_full(str) && !string_truncated(str) means the last string_append()
 * fit exactly in the available space.
 */
#define string_full(str) ((str)->used >= (str)->size - 1)

/* Inline functions used by the initialisation macros above. */
static INLINE string_t _string_alloc_initializer(size_t size)
{
	string_t str;

	if (size) {
		str.buf =
#ifdef __cplusplus
			(char *)
#endif
			malloc(size);
	} else {
		str.buf = NULL;
	}

	if (str.buf) {
		str.buf[0] = '\0';
		str.size = size;
	} else {
		/* malloc failed or size was zero */
		str.size = 0;
	}

	str.used = 0;
	str.type = STRING_GROWABLE;

	return str;
}

static INLINE string_t _string_array_initializer(char array[], size_t size)
{
	string_t str;
	str.buf = array;
	str.size = size;
	str.used = 0;
	str.type = STRING_FIXED_SIZE;
	return str;
}

static INLINE string_t _string_init_initializer(char *ptr, size_t size, string_type_t type)
{
	string_t str;
	str.buf = ptr;
	str.size = size;
	str.used = 0;
	str.type = type;
	return str;
}

#ifdef __cplusplus
}
#endif

#endif /* IMGSTRING_H */
