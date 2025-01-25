# NAME

 http_done - half-closes a HTTP connection

# SYNOPSIS

```c
#include <libdill.h>

int http_done(
    int s,
    int64_t deadline);
```

# DESCRIPTION

 **WARNING: This is experimental functionality and the API may change in the future.**

 HTTP is an application-level protocol described in RFC 7230. This implementation handles only the request/response exchange. Whatever comes after that must be handled by a different protocol.

 This function closes the outbound half of an HTTP connection. Technically, it sends a blank line to the peer. This will, in turn, cause the peer to get **EPIPE** error after it has received all the data.

 Note that HTTP servers rely on getting a blank line before they send a response to a request. Therefore, client, after sending all the fields, must use **http_done** to inform the server that it should act upon the request.

 **s**: The HTTP connection handle.

 **deadline**: A point in time when the operation should time out, in milliseconds. Use the **now** function to get your current point in time. 0 means immediate timeout, i.e., perform the operation if possible or return without blocking if not. -1 means no deadline, i.e., the call will block forever if the operation cannot be performed.

 This function is not available if libdill is compiled with **--disable-sockets** option.

# RETURN VALUE

 In case of success the function returns 0. In case of error it returns -1 and sets **errno** to one of the values below.

# ERRORS

* **EBADF**: Invalid handle.
* **ECANCELED**: Current coroutine was canceled.
* **ENOTSUP**: The handle does not support this operation.
* **EPIPE**: The connection was already half-closed.
* **ETIMEDOUT**: Deadline was reached.

# EXAMPLE

```c
int s = tcp_connect(&addr, -1);
s = http_attach(s);
http_sendrequest(s, "GET", "/", -1);
http_sendfield(s, "Host", "www.example.org", -1);
http_done(s, -1);
char reason[256];
http_recvstatus(s, reason, sizeof(reason), -1);
while(1) {
    char name[256];
    char value[256];
    int rc = http_recvfield(s, name, sizeof(name), value, sizeof(value), -1);
    if(rc == -1 && errno == EPIPE) break;
}
s = http_detach(s, -1);
tcp_close(s);
```

# SEE ALSO

 **http_attach**(3) **http_attach_mem**(3) **http_detach**(3) **http_recvfield**(3) **http_recvrequest**(3) **http_recvstatus**(3) **http_sendfield**(3) **http_sendrequest**(3) **http_sendstatus**(3) **now**(3) 

