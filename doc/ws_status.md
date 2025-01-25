# NAME

 ws_status - retrieves the status after protocol termination

# SYNOPSIS

```c
#include <libdill.h>

ssize_t ws_status(
    int s,
    int* status,
    void* buf,
    size_t len);
```

# DESCRIPTION

 **WARNING: This is experimental functionality and the API may change in the future.**

 WebSocket is a message-based protocol defined in RFC 6455. It can be used as a bidirectional communication channel for communication with a web server.

 When peer shuts down the protocol, receiving functions will start failing with **EPIPE** error. Once that happens **ws_status** function allows to retrieve the status and message sent by the peer when it closed the protocol.

 **s**: The socket handle.

 **status**: Out parameter. Status number sent by the peer. If zero, there was no status.

 **buf**: Buffer to fill the terminal message into. If set to **NULL** the message won't be returned.

 **len**: Size of the buffer, in bytes.

 This function is not available if libdill is compiled with **--disable-sockets** option.

# RETURN VALUE

 In case of success the function returns length of the terminal message. In case of error it returns -1 and sets **errno** to one of the values below.

# ERRORS

* **EAGAIN**: The connection wasn't yet terminated by the peer.
* **EBADF**: Invalid handle.
* **EINVAL**: Invalid argument.
* **EMSGSIZE**: The message doesn't fit in the supplied buffer.
* **ENOTSUP**: The handle does not support this operation.

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

 **mrecv**(3) **mrecvl**(3) **msend**(3) **msendl**(3) **ws_attach_client**(3) **ws_attach_client_mem**(3) **ws_attach_server**(3) **ws_attach_server_mem**(3) **ws_detach**(3) **ws_done**(3) **ws_recv**(3) **ws_recvl**(3) **ws_request_key**(3) **ws_response_key**(3) **ws_send**(3) **ws_sendl**(3) 

