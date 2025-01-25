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

#ifndef LIBDILL_TLS_H_INCLUDED
#define LIBDILL_TLS_H_INCLUDED

#include <dill/core.h>
#include <dill/ip.h>
#include <dill/bsock.h>

#ifdef __cplusplus
extern "C" {
#endif

#if !defined DILL_DISABLE_SOCKETS

#if !defined DILL_DISABLE_TLS

/******************************************************************************/
/*  TLS protocol.                                                             */
/******************************************************************************/

struct dill_tls_storage {char _[72];} DILL_ALIGN;

#define dill_tls_attach_client(s, deadline) dill_tls_attach_client_sni(s, NULL, deadline)
#define dill_tls_attach_client_mem(s, mem, deadline) dill_tls_attach_client_sni_mem(s, NULL, mem, deadline)

DILL_EXPORT int dill_tls_attach_server(
    int s,
    const char *cert,
    const char *pkey,
    int64_t deadline);
DILL_EXPORT int dill_tls_attach_server_mem(
    int s,
    const char *cert,
    const char *pkey,
    struct dill_tls_storage *mem,
    int64_t deadline);
DILL_EXPORT int dill_tls_attach_client_sni(
    int s,
	const char * sni,
    int64_t deadline);
DILL_EXPORT int dill_tls_attach_client_sni_mem(
    int s,
	const char * sni,
    struct dill_tls_storage *mem,
    int64_t deadline);
DILL_EXPORT int dill_tls_done(
    int s,
    int64_t deadline);
DILL_EXPORT int dill_tls_detach(
    int s,
    int64_t deadline);

#if !defined DILL_DISABLE_RAW_NAMES
#define tls_storage dill_tls_storage
#define tls_attach_server dill_tls_attach_server
#define tls_attach_server_mem dill_tls_attach_server_mem
#define tls_attach_client dill_tls_attach_client
#define tls_attach_client_mem dill_tls_attach_client_mem
#define tls_done dill_tls_done
#define tls_detach dill_tls_detach
#define tls_attach_client_sni dill_tls_attach_client_sni
#define tls_attach_client_sni_mem dill_tls_attach_client_sni_mem
#endif

#endif

#ifdef __cplusplus
}
#endif

#endif

#endif
