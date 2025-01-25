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

#ifndef LIBDILL_SOCK5_H_INCLUDED
#define LIBDILL_SOCK5_H_INCLUDED

#include <dill/core.h>
#include <dill/ip.h>

#ifdef __cplusplus
extern "C" {
#endif

#if !defined DILL_DISABLE_SOCKETS

/******************************************************************************/
/*  SOCKS5                                                                    */
/******************************************************************************/

// SOCKS5 client commands
#define DILL_SOCKS5_CONNECT (0x01)
#define DILL_SOCKS5_BIND (0x02)
#define DILL_SOCKS5_UDP_ASSOCIATE (0x03)

// SOCKS5 server reply codes
#define DILL_SOCKS5_SUCCESS (0x00)
#define DILL_SOCKS5_GENERAL_FAILURE (0x01)
#define DILL_SOCKS5_CONNECTION_NOT_ALLOWED (0x02)
#define DILL_SOCKS5_NETWORK_UNREACHABLE (0x03)
#define DILL_SOCKS5_HOST_UNREACHABLE (0x04)
#define DILL_SOCKS5_CONNECTION_REFUSED (0x05)
#define DILL_SOCKS5_TTL_EXPIRED (0x06)
#define DILL_SOCKS5_COMMAND_NOT_SUPPORTED (0x07)
#define DILL_SOCKS5_ADDRESS_TYPE_NOT_SUPPORTED (0x08)


typedef int dill_socks5_auth_function(const char *username,
    const char *password);

DILL_EXPORT int dill_socks5_client_connect(
    int s, const char *username, const char *password,
    struct dill_ipaddr *addr, int64_t deadline);

DILL_EXPORT int dill_socks5_client_connectbyname(
    int s, const char *username, const char *password, const char *hostname,
    int port, int64_t deadline);

DILL_EXPORT int dill_socks5_proxy_auth(
    int s, dill_socks5_auth_function *auth_fn, int64_t deadline);

DILL_EXPORT int dill_socks5_proxy_recvcommand(
    int s, struct dill_ipaddr *ipaddr, int64_t deadline);

DILL_EXPORT int dill_socks5_proxy_recvcommandbyname(
    int s, char *host, int *port, int64_t deadline);

DILL_EXPORT int dill_socks5_proxy_sendreply(
    int s, int reply, struct dill_ipaddr *ipaddr, int64_t deadline);

#if !defined DILL_DISABLE_RAW_NAMES

#define socks5_client_connect dill_socks5_client_connect
#define socks5_client_connectbyname dill_socks5_client_connectbyname
#define socks5_proxy_auth dill_socks5_proxy_auth
#define socks5_proxy_recvcommand dill_socks5_proxy_recvcommand
#define socks5_proxy_recvcommandbyname dill_socks5_proxy_recvcommandbyname
#define socks5_proxy_sendreply dill_socks5_proxy_sendreply

#define SOCKS5_CONNECT DILL_SOCKS5_CONNECT
#define SOCKS5_BIND DILL_SOCKS5_BIND
#define SOCKS5_UDP_ASSOCIATE DILL_SOCKS5_UDP_ASSOCIATE

#define SOCKS5_SUCCESS DILL_SOCKS5_SUCCESS
#define SOCKS5_GENERAL_FAILURE DILL_SOCKS5_GENERAL_FAILURE
#define SOCKS5_CONNECTION_NOT_ALLOWED DILL_SOCKS5_CONNECTION_NOT_ALLOWED
#define SOCKS5_NETWORK_UNREACHABLE DILL_SOCKS5_NETWORK_UNREACHABLE
#define SOCKS5_HOST_UNREACHABLE DILL_SOCKS5_HOST_UNREACHABLE
#define SOCKS5_CONNECTION_REFUSED DILL_SOCKS5_CONNECTION_REFUSED
#define SOCKS5_TTL_EXPIRED DILL_SOCKS5_TTL_EXPIRED
#define SOCKS5_COMMAND_NOT_SUPPORTED DILL_SOCKS5_COMMAND_NOT_SUPPORTED
#define SOCKS5_ADDRESS_TYPE_NOT_SUPPORTED DILL_SOCKS5_ADDRESS_TYPE_NOT_SUPPORTED

#endif /* !defined DILL_DISABLE_RAW_NAMES */

#endif

#ifdef __cplusplus
}
#endif

#endif
