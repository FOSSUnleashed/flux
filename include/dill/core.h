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

#ifndef LIBDILL_CORE_H_INCLUDED
#define LIBDILL_CORE_H_INCLUDED

#include <errno.h>
#include <setjmp.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>

#if defined __linux__
#include <alloca.h>
#endif

/******************************************************************************/
/*  ABI versioning support                                                    */
/******************************************************************************/

/*  Don't change this unless you know exactly what you're doing and have      */
/*  read and understood the following documents:                              */
/*  www.gnu.org/software/libtool/manual/html_node/Libtool-versioning.html     */
/*  www.gnu.org/software/libtool/manual/html_node/Updating-version-info.html  */

/*  The current interface version. */
#define DILL_VERSION_CURRENT 25

/*  The latest revision of the current interface. */
#define DILL_VERSION_REVISION 0

/*  How many past interface versions are still supported. */
#define DILL_VERSION_AGE 5

/******************************************************************************/
/*  Symbol visibility                                                         */
/******************************************************************************/

#if !defined __GNUC__ && !defined __clang__
#error "Unsupported compiler!"
#endif

#if DILL_NO_EXPORTS
#define DILL_EXPORT
#else
#define DILL_EXPORT __attribute__ ((visibility("default")))
#endif

/* Old versions of GCC don't support visibility attribute. */
#if defined __GNUC__ && __GNUC__ < 4
#undef DILL_EXPORT
#define DILL_EXPORT
#endif

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
/*  Memory alignment                                                          */
/******************************************************************************/

/* Opaque structures will violate alignments and cause crashes on certain
   architectures
 */

#if (defined __arm__) || (defined __thumb__)
#define DILL_ALIGN __attribute__ ((aligned(__SIZEOF_POINTER__)))
#else
#define DILL_ALIGN
#endif

/******************************************************************************/
/*                                                                            */
/******************************************************************************/

#define DILL_DEFINE_STORAGE(x, sz) struct dill_##x##storage {char _[sz];} DILL_ALIGN;

typedef int dill_handle;
typedef const void * dill_query_type;

typedef unsigned int Bit;

#if !defined DILL_DISABLE_RAW_NAMES
#define handle dill_handle

typedef unsigned int Bit;

#endif

/******************************************************************************/
/*  Helpers                                                                   */
/******************************************************************************/

DILL_EXPORT int dill_fdclean(int fd);
DILL_EXPORT int dill_fdin(int fd, int64_t deadline);
DILL_EXPORT int dill_fdout(int fd, int64_t deadline);
DILL_EXPORT int64_t dill_now(void);
DILL_EXPORT int dill_msleep(int64_t deadline);
DILL_EXPORT int dill_getcr(void);

DILL_EXPORT void ** dill_cr_getdata(void);

#if !defined DILL_DISABLE_RAW_NAMES
#define cr_getdata dill_cr_getdata
#define fdclean dill_fdclean
#define fdin dill_fdin
#define fdout dill_fdout
#define now dill_now
#define msleep dill_msleep
#define getcr dill_getcr
#endif

/******************************************************************************/
/*  Handles                                                                   */
/******************************************************************************/

DILL_EXPORT int dill_hown(int h);
DILL_EXPORT int dill_hclose(int h);

#if !defined DILL_DISABLE_RAW_NAMES
#define hown dill_hown
#define hclose dill_hclose
#endif

/******************************************************************************/
/*  Coroutines                                                                */
/******************************************************************************/

#define dill_coroutine __attribute__((noinline))

DILL_EXPORT extern volatile void *dill_unoptimisable;

DILL_EXPORT __attribute__((noinline)) int dill_prologue(sigjmp_buf **ctx,
    void **ptr, size_t len, int bndl, const char *file, int line);
DILL_EXPORT __attribute__((noinline)) void dill_epilogue(void);

/* The following macros use alloca(sizeof(size_t)) because clang
   doesn't support alloca with size zero. */

/* This assembly setjmp/longjmp mechanism is in the same order as glibc and
   musl, but glibc implements pointer mangling, which is hard to support.
   This should be binary-compatible with musl, though. */

/* Stack-switching on X86-64. */
#if defined(__x86_64__) && !defined DILL_ARCH_FALLBACK
#define dill_setjmp(ctx) __extension__ ({\
    int ret;\
    asm("lea     LJMPRET%=(%%rip), %%rcx\n\t"\
        "xor     %%rax, %%rax\n\t"\
        "mov     %%rbx, (%%rdx)\n\t"\
        "mov     %%rbp, 8(%%rdx)\n\t"\
        "mov     %%r12, 16(%%rdx)\n\t"\
        "mov     %%r13, 24(%%rdx)\n\t"\
        "mov     %%r14, 32(%%rdx)\n\t"\
        "mov     %%r15, 40(%%rdx)\n\t"\
        "mov     %%rsp, 48(%%rdx)\n\t"\
        "mov     %%rcx, 56(%%rdx)\n\t"\
        "LJMPRET%=:\n\t"\
        : "=a" (ret)\
        : "d" (ctx)\
        : "memory", "rcx", "rsi", "rdi", "r8", "r9", "r10", "r11", "cc");\
    ret;\
})
#define dill_longjmp(ctx) \
    asm("movq   56(%%rdx), %%rcx\n\t"\
        "movq   48(%%rdx), %%rsp\n\t"\
        "movq   40(%%rdx), %%r15\n\t"\
        "movq   32(%%rdx), %%r14\n\t"\
        "movq   24(%%rdx), %%r13\n\t"\
        "movq   16(%%rdx), %%r12\n\t"\
        "movq   8(%%rdx), %%rbp\n\t"\
        "movq   (%%rdx), %%rbx\n\t"\
        ".cfi_def_cfa %%rdx, 0 \n\t"\
        ".cfi_offset %%rbx, 0 \n\t"\
        ".cfi_offset %%rbp, 8 \n\t"\
        ".cfi_offset %%r12, 16 \n\t"\
        ".cfi_offset %%r13, 24 \n\t"\
        ".cfi_offset %%r14, 32 \n\t"\
        ".cfi_offset %%r15, 40 \n\t"\
        ".cfi_offset %%rsp, 48 \n\t"\
        ".cfi_offset %%rip, 56 \n\t"\
        "jmp    *%%rcx\n\t"\
        : : "d" (ctx), "a" (1))
#define dill_setsp(x) \
    asm(""::"r"(alloca(sizeof(size_t))));\
    asm volatile("leaq (%%rax), %%rsp"::"rax"(x));

/* Stack switching on X86. */
#elif defined(__i386__) && !defined DILL_ARCH_FALLBACK
#define dill_setjmp(ctx) __extension__ ({\
    int ret;\
    asm("movl   $LJMPRET%=, %%ecx\n\t"\
        "movl   %%ebx, (%%edx)\n\t"\
        "movl   %%esi, 4(%%edx)\n\t"\
        "movl   %%edi, 8(%%edx)\n\t"\
        "movl   %%ebp, 12(%%edx)\n\t"\
        "movl   %%esp, 16(%%edx)\n\t"\
        "movl   %%ecx, 20(%%edx)\n\t"\
        "xorl   %%eax, %%eax\n\t"\
        "LJMPRET%=:\n\t"\
        : "=a" (ret) : "d" (ctx) : "memory");\
    ret;\
})
#define dill_longjmp(ctx) \
    asm("movl   (%%edx), %%ebx\n\t"\
        "movl   4(%%edx), %%esi\n\t"\
        "movl   8(%%edx), %%edi\n\t"\
        "movl   12(%%edx), %%ebp\n\t"\
        "movl   16(%%edx), %%esp\n\t"\
        "movl   20(%%edx), %%ecx\n\t"\
        ".cfi_def_cfa %%edx, 0 \n\t"\
        ".cfi_offset %%ebx, 0 \n\t"\
        ".cfi_offset %%esi, 4 \n\t"\
        ".cfi_offset %%edi, 8 \n\t"\
        ".cfi_offset %%ebp, 12 \n\t"\
        ".cfi_offset %%esp, 16 \n\t"\
        ".cfi_offset %%eip, 20 \n\t"\
        "jmp    *%%ecx\n\t"\
        : : "d" (ctx), "a" (1))
#define dill_setsp(x) \
    asm(""::"r"(alloca(sizeof(size_t))));\
    asm volatile("leal (%%eax), %%esp"::"eax"(x));

/* Stack-switching on other microarchitectures. */
#else
#define dill_setjmp(ctx) sigsetjmp(ctx, 0)
#define dill_longjmp(ctx) siglongjmp(ctx, 1)
/* For newer GCCs, -fstack-protector breaks on this; use -fno-stack-protector.
   Alternatively, implement a custom dill_setsp for your microarchitecture. */
#define dill_setsp(x) \
    dill_unoptimisable = alloca((char*)alloca(sizeof(size_t)) - (char*)(x));
#endif

/* Statement expressions are a gcc-ism but they are also supported by clang.
   Given that there's no other way to do this, screw other compilers for now.
   See https://gcc.gnu.org/onlinedocs/gcc-3.2/gcc/Statement-Exprs.html */

/* A bug in gcc have been observed where name clash between variable in the
   outer scope and a local variable in this macro causes the variable to
   get weird values. To avoid that, we use fancy names (dill_*__). */ 

#define dill_go_(fn, ptr, len, bndl) \
    __extension__ ({\
        sigjmp_buf *dill_ctx__;\
        void *dill_stk__ = (ptr);\
        int dill_handle__ = dill_prologue(&dill_ctx__, &dill_stk__, (len),\
            (bndl), __FILE__, __LINE__);\
        if(dill_handle__ >= 0) {\
            if(!dill_setjmp(*dill_ctx__)) {\
                dill_setsp(dill_stk__);\
                fn;\
                dill_epilogue();\
            }\
        }\
        dill_handle__;\
    })

#define dill_go(fn) dill_go_(fn, NULL, 0, -1)
#define dill_go_mem(fn, ptr, len) dill_go_(fn, ptr, len, -1)

#define dill_bundle_go(bndl, fn) dill_go_(fn, NULL, 0, bndl)
#define dill_bundle_go_mem(bndl, fn, ptr, len) dill_go_(fn, ptr, len, bndl)

struct dill_bundle_storage {char _[64];} DILL_ALIGN;

DILL_EXPORT int dill_bundle(void);
DILL_EXPORT int dill_bundle_mem(struct dill_bundle_storage *mem);
DILL_EXPORT int dill_bundle_wait(int h, int64_t deadline);
DILL_EXPORT int dill_yield(void);

#if !defined DILL_DISABLE_RAW_NAMES
#define coroutine dill_coroutine
#define go dill_go
#define go_mem dill_go_mem
#define bundle_go dill_bundle_go
#define bundle_go_mem dill_bundle_go_mem
#define bundle_storage dill_bundle_storage
#define bundle dill_bundle
#define bundle_mem dill_bundle_mem
#define bundle_wait dill_bundle_wait
#define yield dill_yield
#endif

/******************************************************************************/
/*  Channels                                                                  */
/******************************************************************************/

#define DILL_CHSEND 1
#define DILL_CHRECV 2

struct dill_chclause {
    int op;
    int ch;
    void *val;
    size_t len;
};

struct dill_chstorage {char _[144];} DILL_ALIGN;

DILL_EXPORT int dill_chmake(
    int chv[2]);
DILL_EXPORT int dill_chmake_mem(
    struct dill_chstorage *mem,
    int chv[2]);
DILL_EXPORT int dill_chsend(
    int ch,
    const void *val,
    size_t len,
    int64_t deadline);
DILL_EXPORT int dill_chrecv(
    int ch,
    void *val,
    size_t len,
    int64_t deadline);
DILL_EXPORT int dill_chdone(
    int ch);
DILL_EXPORT int dill_choose(
    struct dill_chclause *clauses,
    int nclauses,
    int64_t deadline);

#if !defined DILL_DISABLE_RAW_NAMES
#define CHSEND DILL_CHSEND
#define CHRECV DILL_CHRECV
#define chclause dill_chclause
#define chstorage dill_chstorage
#define chmake dill_chmake
#define chmake_mem dill_chmake_mem
#define chsend dill_chsend
#define chrecv dill_chrecv
#define chdone dill_chdone
#define choose dill_choose
#endif

#if !defined DILL_DISABLE_SOCKETS

/******************************************************************************/
/*  Gather/scatter list.                                                      */
/******************************************************************************/

struct dill_iolist {
    void *iol_base;
    size_t iol_len;
    struct dill_iolist *iol_next;
    int iol_rsvd;
};

#if !defined DILL_DISABLE_RAW_NAMES
#define iolist dill_iolist
#endif

#endif

#ifdef __cplusplus
}
#endif

#endif

