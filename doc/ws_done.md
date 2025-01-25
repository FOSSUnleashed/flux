# NAME

 ws_done - half-closes a WebSocket connection

# SYNOPSIS

```c
#include <libdill.h>

int ws_done(
    int s,
    int status,
    const void* buf,
    size_t len,
    int64_t deadline);
```

# DESCRIPTION

 **WARNING: This is experimental functionality and the API may change in the future.**

 WebSocket is a message-based protocol defined in RFC 6455. It can be used as a bidirectional communication channel for communication with a web server.

 This function closes the outbound half of a WebSocket connection. This will, in turn, cause the peer to get **EPIPE** error after it has received all the data.

 **s**: The WebSocket connection handle.

 **status**: Status to send to the peer. If set zero, no status will be sent.

 **buf**: Message to send to the peer. If set to NULL, no message will be sent. Note that WebSocket protocol limits the sizeof the shutdown message to 125 bytes.

 **len**: Size of the message to send to the peer, in bytes.

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
struct ipaddr addr;
ipaddr_remote(&addr, "www.example.org", 80, 0, -1);
int s = tcp_connect(&addr, -1);
s = ws_attach_client(s, "/", "www.example.org", WS_TEXT, -1);
ws_send(s, WS_TEXT, "Hello, world!", 13, -1);
int flags;
char buf[256];
ssize_t sz = ws_recv(s, &flags, buf, sizeof(buf), -1);
assert(flags & WS_TEXT);
s = ws_detach(s, -1);
tcp_close(s, -1);
```

# SEE ALSO

 **mrecv**(3) **mrecvl**(3) **msend**(3) **msendl**(3) **now**(3) **ws_attach_client**(3) **ws_attach_client_mem**(3) **ws_attach_server**(3) **ws_attach_server_mem**(3) **ws_detach**(3) **ws_recv**(3) **ws_recvl**(3) **ws_request_key**(3) **ws_response_key**(3) **ws_send**(3) **ws_sendl**(3) **ws_status**(3) 

