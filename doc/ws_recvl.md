# NAME

 ws_recvl - receives a WebSocket message

# SYNOPSIS

```c
#include <libdill.h>

ssize_t ws_recvl(
    int s,
    int* flags,
    struct iolist* first,
    struct iolist* last,
    int64_t deadline);
```

# DESCRIPTION

 **WARNING: This is experimental functionality and the API may change in the future.**

 WebSocket is a message-based protocol defined in RFC 6455. It can be used as a bidirectional communication channel for communication with a web server.

 This function receives one WebSocket message. It is very much the same as **mrecvl** except that it returns the type of the message (either text or binary).

 This function accepts a linked list of I/O buffers instead of a single buffer. Argument **first** points to the first item in the list, **last** points to the last buffer in the list. The list represents a single, fragmented message, not a list of multiple messages. Structure **iolist** has the following members:

```c
void *iol_base;          /* Pointer to the buffer. */
size_t iol_len;          /* Size of the buffer. */
struct iolist *iol_next; /* Next buffer in the list. */
int iol_rsvd;            /* Reserved. Must be set to zero. */
```

 When receiving, **iol_base** equal to NULL means that **iol_len** bytes should be skipped.

 The function returns **EINVAL** error in the case the list is malformed:

* If **last->iol_next** is not **NULL**.
* If **first** and **last** don't belong to the same list.
* If there's a loop in the list.
* If **iol_rsvd** of any item is non-zero.

 The list (but not the buffers themselves) can be temporarily modified while the function is in progress. However, once the function returns the list is guaranteed to be the same as before the call.

 **s**: Handle of the UDP socket.

 **flags**: Out parameter. Possible values are either **WS_BINARY**or **WS_TEXT**.

 **first**: Pointer to the first item of a linked list of I/O buffers.

 **last**: Pointer to the last item of a linked list of I/O buffers.

 **deadline**: A point in time when the operation should time out, in milliseconds. Use the **now** function to get your current point in time. 0 means immediate timeout, i.e., perform the operation if possible or return without blocking if not. -1 means no deadline, i.e., the call will block forever if the operation cannot be performed.

 This function is not available if libdill is compiled with **--disable-sockets** option.

# RETURN VALUE

 In case of success the function returns size of the received message, in bytes. In case of error it returns -1 and sets **errno** to one of the values below.

# ERRORS

* **EBADF**: Invalid handle.
* **EBUSY**: The handle is currently being used by a different coroutine.
* **ECANCELED**: Current coroutine was canceled.
* **ECONNRESET**: Broken connection.
* **EINVAL**: Invalid argument.
* **EMSGSIZE**: The data won't fit into the supplied buffer.
* **ENOTSUP**: The handle does not support this operation.
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

 **mrecv**(3) **mrecvl**(3) **msend**(3) **msendl**(3) **now**(3) **ws_attach_client**(3) **ws_attach_client_mem**(3) **ws_attach_server**(3) **ws_attach_server_mem**(3) **ws_detach**(3) **ws_done**(3) **ws_recv**(3) **ws_request_key**(3) **ws_response_key**(3) **ws_send**(3) **ws_sendl**(3) **ws_status**(3) 

