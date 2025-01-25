# NAME

 term_attach - creates TERM protocol on top of underlying socket

# SYNOPSIS

```c
#include <libdill.h>

int term_attach(
    int s,
    const void* buf,
    size_t len);
```

# DESCRIPTION

 TERM is a protocol that implements clean terminal handshake between peers. When creating the protocol instance user specifies the terminal message to use. When closing the protocol, terminal messages are exchanged between peers in both directions. After the protocol shuts down the peers agree on their position in the message stream.

 This function instantiates TERM protocol on top of the underlying protocol.

 **s**: Handle of the underlying socket. It must be a message protocol.

 **buf**: The terminal message.

 **len**: Size of the terminal message, in bytes.

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

 **hclose**(3) **mrecv**(3) **mrecvl**(3) **msend**(3) **msendl**(3) **now**(3) **term_attach_mem**(3) **term_detach**(3) **term_done**(3) 

