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

#ifndef LIBDILL_SUFFIX_H_INCLUDED
#define LIBDILL_SUFFIX_H_INCLUDED

#include <dill/core.h>
#include <dill/msock.h>
#include <dill/bsock.h>

#ifdef __cplusplus
extern "C" {
#endif

#if !defined DILL_DISABLE_SOCKETS

/******************************************************************************/
/*  SUFFIX protocol.                                                          */
/*  Messages are suffixed by specified string of bytes.                       */
/******************************************************************************/

struct dill_suffix_storage {char _[128];} DILL_ALIGN;

DILL_EXPORT int dill_suffix_attach(
    int s,
    const void *suffix,
    size_t suffixlen);
DILL_EXPORT int dill_suffix_attach_mem(
    int s,
    const void *suffix,
    size_t suffixlen,
    struct dill_suffix_storage *mem);
DILL_EXPORT int dill_suffix_detach(
    int s,
    int64_t deadline);

#if !defined DILL_DISABLE_RAW_NAMES
#define suffix_storage dill_suffix_storage
#define suffix_attach dill_suffix_attach
#define suffix_attach_mem dill_suffix_attach_mem
#define suffix_detach dill_suffix_detach
#endif

struct dill_file_storage {char _[72];} DILL_ALIGN;

#endif

#ifdef __cplusplus
}
#endif

#endif

