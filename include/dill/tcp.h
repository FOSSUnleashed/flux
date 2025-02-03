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

#ifndef LIBDILL_TCP_H_INCLUDED
#define LIBDILL_TCP_H_INCLUDED

#include <dill/core.h>
#include <dill/bsock.h>
#include <dill/ip.h>

#ifdef __cplusplus
extern "C" {
#endif

#if !defined DILL_DISABLE_SOCKETS

/******************************************************************************/
/*  TCP protocol.                                                             */
/******************************************************************************/

struct dill_tcp_listener_storage {char _[56];} DILL_ALIGN;

struct dill_tcp_storage {char _[72];} DILL_ALIGN;

DILL_EXPORT int dill_tcp_listen(
    struct dill_ipaddr *addr,
    int backlog);
DILL_EXPORT int dill_tcp_listen_mem(
    struct dill_ipaddr *addr,
    int backlog,
    struct dill_tcp_listener_storage *mem);
DILL_EXPORT int dill_tcp_accept(
    int s,
    struct dill_ipaddr *addr,
    int64_t deadline);
DILL_EXPORT int dill_tcp_accept_mem(
    int s,
    struct dill_ipaddr *addr,
    struct dill_tcp_storage *mem,
    int64_t deadline);
DILL_EXPORT int dill_tcp_connect(
    const struct dill_ipaddr *addr,
    int64_t deadline);
DILL_EXPORT int dill_tcp_connect_mem(
    const struct dill_ipaddr *addr,
    struct dill_tcp_storage *mem,
    int64_t deadline);
DILL_EXPORT int dill_tcp_done(
    int s,
    int64_t deadline);
DILL_EXPORT int dill_tcp_close(
    int s,
    int64_t deadline);
DILL_EXPORT int dill_tcp_listener_fromfd(
    int fd);
DILL_EXPORT int dill_tcp_listener_fromfd_mem(
    int fd,
    struct dill_tcp_listener_storage *mem);
DILL_EXPORT int dill_tcp_fromfd(
    int fd);
DILL_EXPORT int dill_tcp_fromfd_mem(
    int fd,
    struct dill_tcp_storage *mem);

#if !defined DILL_DISABLE_RAW_NAMES
#define tcp_listener_storage dill_tcp_listener_storage
#define tcp_storage dill_tcp_storage
#define tcp_listen dill_tcp_listen
#define tcp_listen_mem dill_tcp_listen_mem
#define tcp_accept dill_tcp_accept
#define tcp_accept_mem dill_tcp_accept_mem
#define tcp_connect dill_tcp_connect
#define tcp_connect_mem dill_tcp_connect_mem
#define tcp_done dill_tcp_done
#define tcp_close dill_tcp_close
#define tcp_listener_fromfd dill_tcp_listener_fromfd
#define tcp_listener_fromfd_mem dill_tcp_listener_fromfd_mem
#define tcp_fromfd dill_tcp_fromfd
#define tcp_fromfd_mem dill_tcp_fromfd_mem
#endif

#endif

#ifdef __cplusplus
}
#endif

#endif

