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

#ifndef LIBDILL_IPC_H_INCLUDED
#define LIBDILL_IPC_H_INCLUDED

#include <dill/core.h>
#include <dill/bsock.h>

#ifdef __cplusplus
extern "C" {
#endif

#if !defined DILL_DISABLE_SOCKETS

/******************************************************************************/
/*  IPC protocol.                                                            */
/******************************************************************************/

struct dill_ipc_listener_storage {char _[24];} DILL_ALIGN;

struct dill_ipc_storage {char _[72];} DILL_ALIGN;

struct dill_ipc_pair_storage {char _[144];} DILL_ALIGN;

DILL_EXPORT int dill_ipc_listen(
    const char *addr,
    int backlog);
DILL_EXPORT int dill_ipc_listen_mem(
    const char *addr,
    int backlog,
    struct dill_ipc_listener_storage *mem);
DILL_EXPORT int dill_ipc_accept(
    int s,
    int64_t deadline);
DILL_EXPORT int dill_ipc_accept_mem(
    int s,
    struct dill_ipc_storage *mem,
    int64_t deadline);
DILL_EXPORT int dill_ipc_connect(
    const char *addr,
    int64_t deadline);
DILL_EXPORT int dill_ipc_connect_mem(
    const char *addr,
    struct dill_ipc_storage *mem,
    int64_t deadline);
DILL_EXPORT int dill_ipc_sendfd(
    int s,
    int fd,
    int64_t deadline);
DILL_EXPORT int dill_ipc_recvfd(
    int s,
    int64_t deadline);
DILL_EXPORT int dill_ipc_done(
    int s,
    int64_t deadline);
DILL_EXPORT int dill_ipc_close(
    int s,
    int64_t deadline);
DILL_EXPORT int dill_ipc_listener_fromfd(
    int fd);
DILL_EXPORT int dill_ipc_listener_fromfd_mem(
    int fd,
    struct dill_ipc_listener_storage *mem);
DILL_EXPORT int dill_ipc_fromfd(
    int fd);
DILL_EXPORT int dill_ipc_fromfd_mem(
    int fd,
    struct dill_ipc_storage *mem);
DILL_EXPORT int dill_ipc_pair(
    int s[2]);
DILL_EXPORT int dill_ipc_pair_mem(
    struct dill_ipc_pair_storage *mem,
    int s[2]);

#if !defined DILL_DISABLE_RAW_NAMES
#define ipc_listener_storage dill_ipc_listener_storage
#define ipc_storage dill_ipc_storage
#define ipc_pair_storage dill_ipc_pair_storage
#define ipc_listen dill_ipc_listen
#define ipc_listen_mem dill_ipc_listen_mem
#define ipc_accept dill_ipc_accept
#define ipc_accept_mem dill_ipc_accept_mem
#define ipc_connect dill_ipc_connect
#define ipc_connect_mem dill_ipc_connect_mem
#define ipc_sendfd dill_ipc_sendfd
#define ipc_recvfd dill_ipc_recvfd
#define ipc_done dill_ipc_done
#define ipc_close dill_ipc_close
#define ipc_listener_fromfd dill_ipc_listener_fromfd
#define ipc_listener_fromfd_mem dill_ipc_listener_fromfd_mem
#define ipc_fromfd dill_ipc_fromfd
#define ipc_fromfd_mem dill_ipc_fromfd_mem
#define ipc_pair dill_ipc_pair
#define ipc_pair_mem dill_ipc_pair_mem
#endif

#endif

#ifdef __cplusplus
}
#endif

#endif
