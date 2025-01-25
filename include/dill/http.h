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

#ifndef LIBDILL_HTTP_H_INCLUDED
#define LIBDILL_HTTP_H_INCLUDED

#include <dill/core.h>
#include <dill/tcp.h>
#include <dill/msock.h>

#ifdef __cplusplus
extern "C" {
#endif

#if !defined DILL_DISABLE_SOCKETS

/******************************************************************************/
/*  HTTP                                                                      */
/******************************************************************************/

struct dill_http_storage {char _[1296];} DILL_ALIGN;

DILL_EXPORT int dill_http_attach(
    int s);
DILL_EXPORT int dill_http_attach_mem(
    int s,
    struct dill_http_storage *mem);
DILL_EXPORT int dill_http_done(
    int s,
    int64_t deadline);
DILL_EXPORT int dill_http_detach(
    int s,
    int64_t deadline);
DILL_EXPORT int dill_http_sendrequest(
    int s,
    const char *command,
    const char *resource,
    int64_t deadline);
DILL_EXPORT int dill_http_recvrequest(
    int s,
    char *command,
    size_t commandlen,
    char *resource,
    size_t resourcelen,
    int64_t deadline);
DILL_EXPORT int dill_http_sendstatus(
    int s,
    int status,
    const char *reason,
    int64_t deadline);
DILL_EXPORT int dill_http_recvstatus(
    int s,
    char *reason,
    size_t reasonlen,
    int64_t deadline);
DILL_EXPORT int dill_http_sendfield(
    int s,
    const char *name,
    const char *value,
    int64_t deadline);
DILL_EXPORT int dill_http_recvfield(
    int s,
    char *name,
    size_t namelen,
    char *value,
    size_t valuelen,
    int64_t deadline);

#if !defined DILL_DISABLE_RAW_NAMES
#define http_storage dill_http_storage
#define http_attach dill_http_attach
#define http_attach_mem dill_http_attach_mem
#define http_done dill_http_done
#define http_detach dill_http_detach
#define http_sendrequest dill_http_sendrequest
#define http_recvrequest dill_http_recvrequest
#define http_sendstatus dill_http_sendstatus
#define http_recvstatus dill_http_recvstatus
#define http_sendfield dill_http_sendfield
#define http_recvfield dill_http_recvfield
#endif

#endif

#ifdef __cplusplus
}
#endif

#endif

