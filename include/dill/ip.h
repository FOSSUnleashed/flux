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

#ifndef LIBDILL_IP_H_INCLUDED
#define LIBDILL_IP_H_INCLUDED

#include <dill/core.h>

#ifdef __cplusplus
extern "C" {
#endif


#if !defined DILL_DISABLE_SOCKETS

/******************************************************************************/
/*  IP address resolution.                                                    */
/******************************************************************************/

struct sockaddr;

#define DILL_IPADDR_IPV4 1
#define DILL_IPADDR_IPV6 2
#define DILL_IPADDR_PREF_IPV4 3
#define DILL_IPADDR_PREF_IPV6 4
#define DILL_IPADDR_MAXSTRLEN 46

struct dill_ipaddr {char _[32];};

DILL_EXPORT int dill_ipaddr_local(
    struct dill_ipaddr *addr,
    const char *name,
    int port,
    int mode);
DILL_EXPORT int dill_ipaddr_remote(
    struct dill_ipaddr *addr,
    const char *name,
    int port,
    int mode,
    int64_t deadline);
DILL_EXPORT int dill_ipaddr_remotes(
    struct dill_ipaddr *addrs,
    int naddrs,
    const char *name,
    int port,
    int mode,
    int64_t deadline);
DILL_EXPORT const char *dill_ipaddr_str(
    const struct dill_ipaddr *addr,
    char *ipstr);
DILL_EXPORT int dill_ipaddr_family(
    const struct dill_ipaddr *addr);
DILL_EXPORT const struct sockaddr *dill_ipaddr_sockaddr(
    const struct dill_ipaddr *addr);
DILL_EXPORT int dill_ipaddr_len(
    const struct dill_ipaddr *addr);
DILL_EXPORT int dill_ipaddr_port(
    const struct dill_ipaddr *addr);
DILL_EXPORT void dill_ipaddr_setport(
    struct dill_ipaddr *addr,
    int port);
DILL_EXPORT int dill_ipaddr_equal(
    const struct dill_ipaddr *addr1,
    const struct dill_ipaddr *addr2,
    int ignore_port);

int dill_ipaddr_ipv4_literal(struct dill_ipaddr *addr, const char *name, int port);
int dill_ipaddr_ipv6_literal(struct dill_ipaddr *addr, const char *name, int port);

#if !defined DILL_DISABLE_RAW_NAMES
#define IPADDR_IPV4 DILL_IPADDR_IPV4 
#define IPADDR_IPV6 DILL_IPADDR_IPV6
#define IPADDR_PREF_IPV4 DILL_IPADDR_PREF_IPV4 
#define IPADDR_PREF_IPV6 DILL_IPADDR_PREF_IPV6
#define IPADDR_MAXSTRLEN DILL_IPADDR_MAXSTRLEN
#define ipaddr dill_ipaddr
#define ipaddr_local dill_ipaddr_local
#define ipaddr_remote dill_ipaddr_remote
#define ipaddr_remotes dill_ipaddr_remotes
#define ipaddr_str dill_ipaddr_str
#define ipaddr_family dill_ipaddr_family
#define ipaddr_sockaddr dill_ipaddr_sockaddr
#define ipaddr_len dill_ipaddr_len
#define ipaddr_port dill_ipaddr_port
#define ipaddr_setport dill_ipaddr_setport
#define ipaddr_equal dill_ipaddr_equal

#define ipaddr_ipv4 dill_ipaddr_ipv4_literal
#define ipaddr_ipv6 dill_ipaddr_ipv6_literal
#endif

#endif

#ifdef __cplusplus
}
#endif

#endif

