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

#ifndef LIBDILL_TERM_H_INCLUDED
#define LIBDILL_TERM_H_INCLUDED

#include <dill/core.h>
#include <dill/msock.h>

#ifdef __cplusplus
extern "C" {
#endif

#if !defined DILL_DISABLE_SOCKETS

/******************************************************************************/
/*  TERM protocol.                                                            */
/*  Implementes terminal handshake on the top of any message-based protocol.  */
/******************************************************************************/

struct dill_term_storage {char _[88];} DILL_ALIGN;

DILL_EXPORT int dill_term_attach(
    int s,
    const void *buf,
    size_t len);
DILL_EXPORT int dill_term_attach_mem(
    int s,
    const void *buf,
    size_t len,
    struct dill_term_storage *mem);
DILL_EXPORT int dill_term_done(
    int s,
    int64_t deadline);
DILL_EXPORT int dill_term_detach(
    int s,
    int64_t deadline);

#if !defined DILL_DISABLE_RAW_NAMES
#define term_storage dill_term_storage
#define term_attach dill_term_attach
#define term_attach_mem dill_term_attach_mem
#define term_done dill_term_done
#define term_detach dill_term_detach
#endif

#endif

#ifdef __cplusplus
}
#endif

#endif
