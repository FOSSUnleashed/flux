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

#ifndef LIBDILL_WEBSOCK_H_INCLUDED
#define LIBDILL_WEBSOCK_H_INCLUDED

#include <dill/core.h>
#include <dill/http.h>
#include <dill/bsock.h>

#ifdef __cplusplus
extern "C" {
#endif

#if !defined DILL_DISABLE_SOCKETS

/******************************************************************************/
/*  WebSockets protocol.                                                      */
/******************************************************************************/

struct dill_ws_storage {char _[176];} DILL_ALIGN;

#define DILL_WS_BINARY 0
#define DILL_WS_TEXT 1
#define DILL_WS_NOHTTP 2

DILL_EXPORT int dill_ws_attach_client(
    int s,
    int flags,
    const char *resource,
    const char *host,
    int64_t deadline);
DILL_EXPORT int dill_ws_attach_client_mem(
    int s,
    int flags,
    const char *resource,
    const char *host,
    struct dill_ws_storage *mem,
    int64_t deadline);
DILL_EXPORT int dill_ws_attach_server(
    int s,
    int flags,
    char *resource,
    size_t resourcelen,
    char *host,
    size_t hostlen,
    int64_t deadline);
DILL_EXPORT int dill_ws_attach_server_mem(
    int s,
    int flags,
    char *resource,
    size_t resourcelen,
    char *host,
    size_t hostlen,
    struct dill_ws_storage *mem,
    int64_t deadline);
DILL_EXPORT int dill_ws_send(
    int s,
    int flags,
    const void *buf,
    size_t len,
    int64_t deadline);
DILL_EXPORT ssize_t dill_ws_recv(
    int s,
    int *flags,
    void *buf,
    size_t len,
    int64_t deadline);
DILL_EXPORT int dill_ws_sendl(
    int s,
    int flags,
    struct dill_iolist *first,
    struct dill_iolist *last,
    int64_t deadline);
DILL_EXPORT ssize_t dill_ws_recvl(
    int s,
    int *flags,
    struct dill_iolist *first,
    struct dill_iolist *last,
    int64_t deadline);
DILL_EXPORT int dill_ws_done(
    int s,
    int status,
    const void *buf,
    size_t len,
    int64_t deadline);
DILL_EXPORT int dill_ws_detach(
    int s,
    int status,
    const void *buf,
    size_t len,
    int64_t deadline);
DILL_EXPORT ssize_t dill_ws_status(
    int s,
    int *status,
    void *buf,
    size_t len);

/* Helper functions for those who want to implement HTTP exchange by hand. */

#define WS_KEY_SIZE 32

DILL_EXPORT int dill_ws_request_key(
    char *request_key);
DILL_EXPORT int dill_ws_response_key(
    const char *request_key,
    char *response_key);

#if !defined DILL_DISABLE_RAW_NAMES
#define WS_BINARY DILL_WS_BINARY
#define WS_TEXT DILL_WS_TEXT
#define WS_NOHTTP DILL_WS_NOHTTP
#define ws_storage dill_ws_storage
#define ws_attach_server dill_ws_attach_server
#define ws_attach_server_mem dill_ws_attach_server_mem
#define ws_attach_client dill_ws_attach_client
#define ws_attach_client_mem dill_ws_attach_client_mem
#define ws_send dill_ws_send
#define ws_recv dill_ws_recv
#define ws_sendl dill_ws_sendl
#define ws_recvl dill_ws_recvl
#define ws_done dill_ws_done
#define ws_detach dill_ws_detach
#define ws_status dill_ws_status
#define ws_request_key dill_ws_request_key
#define ws_response_key dill_ws_response_key
#endif

#endif

#ifdef __cplusplus
}
#endif

#endif
