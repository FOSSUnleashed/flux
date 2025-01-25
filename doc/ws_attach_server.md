# NAME

 ws_attach_server - creates WebSocket protocol on the top of underlying socket

# SYNOPSIS

```c
#include <libdill.h>

int ws_attach_server(
    int s,
    int flags,
    char* resource,
    size_t resourcelen,
    char* host,
    size_t hostlen,
    int64_t deadline);
```

# DESCRIPTION

 **WARNING: This is experimental functionality and the API may change in the future.**

 WebSocket is a message-based protocol defined in RFC 6455. It can be used as a bidirectional communication channel for communication with a web server.

 This function instantiates WebSocket protocol on top of the underlying bytestream protocol. WebSocket protocol being asymmetric, client and server sides are intialized in different ways. This particular function initializes the client side of the connection.

 The socket can be either text- (**WS_TEXT** flag) or binary- (**WS_BINARY** flag) based. Binary is the default. When sending messages via **msend** or **msendl** these will be typed based on the socket type. When receiving messages via **mrecv** or **mrecvl** encountering a message that doesn't match the socket type results in **EPROTO** error.

 If you want to combine text and binary messages you can do so by using functions such as **ws_send** and **ws_recv**.

 **WS_NOHTTP** flag can be combined with socket type flags. If set, the protocol will skip the initial HTTP handshake. In this case **resource** and **host** arguments won't be used and can be set to **NULL**.

 Skipping HTTP handshake is useful when you want to do the handshake on your own. For example, if you want to implement custom WebSocket extensions or if you want to write a multi-protocol application where initial HTTP handshake can be followed by different kinds of protocols (e.g. HTML and WebSocket).

 **s**: Handle of the underlying socket. It must be a bytestream protocol.

 **flags**: Flags. See above.

 **resource**: Out parameter. HTTP resource specified by the client.

 **resourcelen**: Size of the **resource** buffer.

 **host**: Out parameter. Virtual host specified by the client.

 **hostlen**: Size of the **host** buffer.

 **deadline**: A point in time when the operation should time out, in milliseconds. Use the **now** function to get your current point in time. 0 means immediate timeout, i.e., perform the operation if possible or return without blocking if not. -1 means no deadline, i.e., the call will block forever if the operation cannot be performed.

 The socket can be cleanly shut down using **ws_detach** function.

 This function is not available if libdill is compiled with **--disable-sockets** option.

# RETURN VALUE

 In case of success the function returns newly created socket handle. In case of error it returns -1 and sets **errno** to one of the values below.

# ERRORS

* **EBADF**: Invalid handle.
* **ECANCELED**: Current coroutine was canceled.
* **ECONNRESET**: Broken connection.
* **EINVAL**: Invalid argument.
* **EMFILE**: The maximum number of file descriptors in the process are already open.
* **ENFILE**: The maximum number of file descriptors in the system are already open.
* **ENOMEM**: Not enough memory.
* **ENOTSUP**: The handle does not support this operation.
* **EPROTO**: Underlying socket is not a bytestream socket.
* **ETIMEDOUT**: Deadline was reached.

# EXAMPLE

```c
int s = tcp_accept(listener, NULL, -1);
s = tls_attach_server(s, -1);
bsend(s, "ABC", 3, -1);
char buf[3];
ssize_t sz = brecv(s, buf, sizeof(buf), -1);
s = tls_detach(s, -1);
tcp_close(s);
```

# SEE ALSO

 **hclose**(3) **mrecv**(3) **mrecvl**(3) **msend**(3) **msendl**(3) **now**(3) **ws_attach_client**(3) **ws_attach_client_mem**(3) **ws_attach_server_mem**(3) **ws_detach**(3) **ws_done**(3) **ws_recv**(3) **ws_recvl**(3) **ws_request_key**(3) **ws_response_key**(3) **ws_send**(3) **ws_sendl**(3) **ws_status**(3) 

