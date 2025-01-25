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

#ifndef LIBDILL_BSOCK_H_INCLUDED
#define LIBDILL_BSOCK_H_INCLUDED

#include <dill/core.h>
#include <dill/ip.h>

#ifdef __cplusplus
extern "C" {
#endif

#if !defined DILL_DISABLE_SOCKETS

/******************************************************************************/
/*  Bytestream sockets.                                                       */
/******************************************************************************/

DILL_EXPORT int dill_bsend(
    int s,
    const void *buf,
    size_t len,
    int64_t deadline);
DILL_EXPORT int dill_brecv(
    int s,
    void *buf,
    size_t len,
    int64_t deadline);
DILL_EXPORT int dill_bsendl(
    int s,
    struct dill_iolist *first,
    struct dill_iolist *last,
    int64_t deadline);
DILL_EXPORT int dill_brecvl(
    int s,
    struct dill_iolist *first,
    struct dill_iolist *last,
    int64_t deadline);

#if !defined DILL_DISABLE_RAW_NAMES
#define bsend dill_bsend
#define brecv dill_brecv
#define bsendl dill_bsendl
#define brecvl dill_brecvl
#endif

#endif

#ifdef __cplusplus
}
#endif

#endif
