# NAME

 term_attach_mem - creates TERM protocol on top of underlying socket

# SYNOPSIS

```c
#include <libdill.h>

int term_attach_mem(
    int s,
    const void* buf,
    size_t len,
    struct term_storage* mem,
    int64_t deadline);
```

# DESCRIPTION

 TERM is a protocol that implements clean terminal handshake between peers. When creating the protocol instance user specifies the terminal message to use. When closing the protocol, terminal messages are exchanged between peers in both directions. After the protocol shuts down the peers agree on their position in the message stream.

 This function instantiates TERM protocol on top of the underlying protocol.

 This function allows to avoid one dynamic memory allocation by storing the object in user-supplied memory. Unless you are hyper-optimizing use **term_attach** instead.

 **s**: Handle of the underlying socket. It must be a message protocol.

 **buf**: The terminal message.

 **len**: Size of the terminal message, in bytes.

 **mem**: The structure to store the newly created object in. It must not be deallocated before the object is closed.

 **deadline**: A point in time when the operation should time out, in milliseconds. Use the **now** function to get your current point in time. 0 means immediate timeout, i.e., perform the operation if possible or return without blocking if not. -1 means no deadline, i.e., the call will block forever if the operation cannot be performed.

 The socket can be cleanly shut down using **term_detach** function.

 This function is not available if libdill is compiled with **--disable-sockets** option.

# RETURN VALUE

 In case of success the function returns newly created socket handle. In case of error it returns -1 and sets **errno** to one of the values below.

# ERRORS

* **EBADF**: Invalid handle.
* **ECANCELED**: Current coroutine was canceled.
* **EMFILE**: The maximum number of file descriptors in the process are already open.
* **ENFILE**: The maximum number of file descriptors in the system are already open.
* **ENOMEM**: Not enough memory.
* **ENOTSUP**: The handle does not support this operation.
* **EPROTO**: Underlying socket is not a message socket.
* **ETIMEDOUT**: Deadline was reached.

# EXAMPLE

```c
s = term_attach(s, "STOP", 4);
...
/* Send terminal message to the peer. */
term_done(s, -1);
/* Process remaining inbound messages. */
while(1) {
    char buf[256];
    ssize_t sz = mrecv(s, buf, sizeof(buf), -1);
    /* Check whether terminal message was received from the peer. */
    if(sz < 0 && errno == EPIPE) break;
    frobnicate(buff, sz);
}
s = term_detach(s);
```

# SEE ALSO

 **hclose**(3) **mrecv**(3) **mrecvl**(3) **msend**(3) **msendl**(3) **now**(3) **term_attach**(3) **term_detach**(3) **term_done**(3) 

