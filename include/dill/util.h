
// TODO: Replace with GPL3
/*

  Copyright (c) 2017 Martin Sustrik

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"),
  to deal in the Software without restriction, including without limitation
  the rights to use, copy, modify, merge, publish, distribute, sublicense,
  and/or sell copies of the Software, and to permit persons to whom
  the Software is furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included
  in all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
  IN THE SOFTWARE.

*/

#ifndef LIBDILL_UTIL_H_INCLUDED
#define LIBDILL_UTIL_H_INCLUDED

#include <dill/core.h>

#ifdef __cplusplus
extern "C" {
#endif

// TODO: iol functions for implimenting custom handles

// Control structure definitions, TODO: doc-pages
#define dill_forever for (;;)
#define dill_if_slow(x) if (dill_slow((x)))
#define dill_if_fast(x) if (dill_fast((x)))

// Helper defines for types to make code clearer; TODO: change public interfaces
typedef int dill_handle;

// <MIT> from libdill
#define dill_concat(x,y) x##y

/* Defines a unique identifier of type const void*. */
#define dill_unique_id(name) \
	static const int dill_concat(name, ___) = 0;\
	dill_query_type name = (dill_query_type)&dill_concat(name, ___);

/*  Takes a pointer to a member variable and computes pointer to the structure
    that contains it. 'type' is type of the structure, not the member. */
#define dill_cont(ptr, type, member) \
    (ptr ? ((type*) (((char*) ptr) - offsetof(type, member))) : NULL)

/* Optimisation hints. */
#if defined __GNUC__ || defined __llvm__
#define dill_fast(x) (__builtin_expect(!!(x), 1))
#define dill_slow(x) (__builtin_expect(!!(x), 0))
#else
#define dill_fast(x) (x)
#define dill_slow(x) (x)
#endif

/* Compile-time assert. */
#define DILL_CT_ASSERT_HELPER2(prefix, line) \
    prefix##line
#define DILL_CT_ASSERT_HELPER1(prefix, line) \
    DILL_CT_ASSERT_HELPER2(prefix, line)
#define DILL_CT_ASSERT(x) \
    typedef int DILL_CT_ASSERT_HELPER1(ct_assert_,__COUNTER__) [(x) ? 1 : -1];

/* Define our own assert. This way we are sure that it stays in place even
   if the standard C assert would be thrown away by the compiler. It also
   allows us to overload it as needed. */
#define dill_assert(x) \
    do {\
        if (dill_slow(!(x))) {\
            fprintf(stderr, "Assert failed: " #x " (%s:%d)\n",\
                __FILE__, __LINE__);\
            fflush(stderr);\
            abort();\
        }\
    } while (0)

/* DILL_CHECK_STORAGE checks whether struct 'type' fits into struct 'storage'.
   If DILL_PRINT_SIZES macro is defined it will also print the size
   of 'type'. */
#if defined DILL_PRINT_SIZES
#define DILL_CHECK_STORAGE(type, storage) \
    static void dill_print_size_2_##type(void) { \
        char x[sizeof(struct type)]; \
        dill_print_size(&x); \
    } \
    DILL_CT_ASSERT(sizeof(struct type) <= sizeof(struct storage))
#else
#define DILL_CHECK_STORAGE(type, storage) \
    DILL_CT_ASSERT(sizeof(struct type) <= sizeof(struct storage))
#endif

void dill_print_size(char x);

// </MIT>

#if !defined DILL_DISABLE_RAW_NAMES
#define forever dill_forever
#define if_slow dill_if_slow
#define if_fast dill_if_fast
#define slow dill_slow
#define fast dill_fast
#endif

#ifdef __cplusplus
}
#endif

#endif

