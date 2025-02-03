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

#ifndef LIBDILL_PREFIX_H_INCLUDED
#define LIBDILL_PREFIX_H_INCLUDED

#include <dill/core.h>
#include <dill/msock.h>

#ifdef __cplusplus
extern "C" {
#endif

#if !defined DILL_DISABLE_SOCKETS

/******************************************************************************/
/*  PREFIX protocol.                                                          */
/*  Messages are prefixed by size.                                            */
/******************************************************************************/

struct dill_prefix_storage {char _[56];} DILL_ALIGN;

#define DILL_PREFIX_BIG_ENDIAN 0
#define DILL_PREFIX_LITTLE_ENDIAN 1

DILL_EXPORT int dill_prefix_attach(
    int s,
    size_t hdrlen,
    int flags);
DILL_EXPORT int dill_prefix_attach_mem(
    int s,
    size_t hdrlen,
    int flags,
    struct dill_prefix_storage *mem);
DILL_EXPORT int dill_prefix_detach(
    int s);

#if !defined DILL_DISABLE_RAW_NAMES
#define PREFIX_BIG_ENDIAN DILL_PREFIX_BIG_ENDIAN
#define PREFIX_LITTLE_ENDIAN DILL_PREFIX_LITTLE_ENDIAN
#define prefix_storage dill_prefix_storage
#define prefix_attach dill_prefix_attach
#define prefix_attach_mem dill_prefix_attach_mem
#define prefix_detach dill_prefix_detach
#endif

#endif

#ifdef __cplusplus
}
#endif

#endif

