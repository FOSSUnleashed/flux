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

#ifndef LIBDILL_DTLS_H_INCLUDED
#define LIBDILL_DTLS_H_INCLUDED

#include <dill/core.h>
#include <dill/ip.h>

#ifdef __cplusplus
extern "C" {
#endif

#if !defined DILL_DISABLE_SOCKETS
#if !defined DILL_DISABLE_TLS

/******************************************************************************/
/*  DTLS protocol.                                                            */
/******************************************************************************/

struct dill_dtls_storage {char _[88];} DILL_ALIGN;

DILL_EXPORT int dill_dtls_attach_server(
    int s,
    const char *cert,
    const char *pkey,
    int64_t deadline);
DILL_EXPORT int dill_dtls_attach_server_mem(
    int s,
    const char *cert,
    const char *pkey,
    struct dill_dtls_storage *mem,
    int64_t deadline);
DILL_EXPORT int dill_dtls_attach_client(
    int s,
    int64_t deadline);
DILL_EXPORT int dill_dtls_attach_client_mem(
    int s,
    struct dill_dtls_storage *mem,
    int64_t deadline);
DILL_EXPORT int dill_dtls_done(
    int s,
    int64_t deadline);
DILL_EXPORT int dill_dtls_detach(
    int s,
    int64_t deadline);

#if !defined DILL_DISABLE_RAW_NAMES
#define dtls_storage dill_dtls_storage
#define dtls_attach_server dill_dtls_attach_server
#define dtls_attach_server_mem dill_dtls_attach_server_mem
#define dtls_attach_client dill_dtls_attach_client
#define dtls_attach_client_mem dill_dtls_attach_client_mem
#define dtls_done dill_dtls_done
#define dtls_detach dill_dtls_detach
#endif

#endif
#endif

#ifdef __cplusplus
}
#endif

#endif
