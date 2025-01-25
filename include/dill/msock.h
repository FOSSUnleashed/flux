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

#ifndef LIBDILL_MSOCK_H_INCLUDED
#define LIBDILL_MSOCK_H_INCLUDED

#include <dill/core.h>

#ifdef __cplusplus
extern "C" {
#endif

#if !defined DILL_DISABLE_SOCKETS

/******************************************************************************/
/*  Message sockets.                                                          */
/******************************************************************************/

DILL_EXPORT int dill_msend(
    int s,
    const void *buf,
    size_t len,
    int64_t deadline);
DILL_EXPORT ssize_t dill_mrecv(
    int s,
    void *buf,
    size_t len,
    int64_t deadline);
DILL_EXPORT int dill_msendl(
    int s,
    struct dill_iolist *first,
    struct dill_iolist *last,
    int64_t deadline);
DILL_EXPORT ssize_t dill_mrecvl(
    int s,
    struct dill_iolist *first,
    struct dill_iolist *last,
    int64_t deadline);

#if !defined DILL_DISABLE_RAW_NAMES
#define msend dill_msend
#define mrecv dill_mrecv
#define msendl dill_msendl
#define mrecvl dill_mrecvl
#endif

#endif

#ifdef __cplusplus
}
#endif

#endif

