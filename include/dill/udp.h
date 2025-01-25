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

#ifndef LIBDILL_UDP_H_INCLUDED
#define LIBDILL_UDP_H_INCLUDED

#include <dill/core.h>
#include <dill/msock.h>
#include <dill/ip.h>

#ifdef __cplusplus
extern "C" {
#endif

#if !defined DILL_DISABLE_SOCKETS

/******************************************************************************/
/*  UDP protocol.                                                             */
/*  Each UDP packet is treated as a separate message.                         */
/******************************************************************************/

struct dill_udp_storage {char _[72];} DILL_ALIGN;

DILL_EXPORT int dill_udp_open(
    struct dill_ipaddr *local,
    const struct dill_ipaddr *remote);
DILL_EXPORT int dill_udp_open_mem(
    struct dill_ipaddr *local,
    const struct dill_ipaddr *remote,
    struct dill_udp_storage *mem);
DILL_EXPORT int dill_udp_send(
    int s,
    const struct dill_ipaddr *addr,
    const void *buf,
    size_t len);
DILL_EXPORT ssize_t dill_udp_recv(
    int s,
    struct dill_ipaddr *addr,
    void *buf,
    size_t len,
    int64_t deadline);
DILL_EXPORT int dill_udp_sendl(
    int s,
    const struct dill_ipaddr *addr,
    struct dill_iolist *first,
    struct dill_iolist *last);
DILL_EXPORT ssize_t dill_udp_recvl(
    int s,
    struct dill_ipaddr *addr,
    struct dill_iolist *first,
    struct dill_iolist *last,
    int64_t deadline);

#if !defined DILL_DISABLE_RAW_NAMES
#define udp_storage dill_udp_storage
#define udp_open dill_udp_open
#define udp_open_mem dill_udp_open_mem
#define udp_send dill_udp_send
#define udp_recv dill_udp_recv
#define udp_sendl dill_udp_sendl
#define udp_recvl dill_udp_recvl
#endif

#endif

#ifdef __cplusplus
}
#endif

#endif

