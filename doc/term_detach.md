# NAME

 term_detach - cleanly terminates the protocol

# SYNOPSIS

```c
#include <libdill.h>

int term_detach(
    int s,
    int64_t deadline);
```

# DESCRIPTION

 TERM is a protocol that implements clean terminal handshake between peers. When creating the protocol instance user specifies the terminal message to use. When closing the protocol, terminal messages are exchanged between peers in both directions. After the protocol shuts down the peers agree on their position in the message stream.

 This function cleanly terminates TERM protocol. If termination message wasn't yet sent to the peer using **term_done** function it will be sent now. Afterwards, any outstanding inbound messages will be received and dropped silently. The function returns after termination message from the peer is received.

 **s**: The TERM protocol handle.

 **deadline**: A point in time when the operation should time out, in milliseconds. Use the **now** function to get your current point in time. 0 means immediate timeout, i.e., perform the operation if possible or return without blocking if not. -1 means no deadline, i.e., the call will block forever if the operation cannot be performed.

 This function is not available if libdill is compiled with **--disable-sockets** option.

# RETURN VALUE

 In case of success the function returns 0. In case of error it returns -1 and sets **errno** to one of the values below.

# ERRORS

* **EBADF**: Invalid handle.
* **ECANCELED**: Current coroutine was canceled.
* **ENOTSUP**: The handle is not a TERM protocol handle.
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

 **mrecv**(3) **mrecvl**(3) **msend**(3) **msendl**(3) **now**(3) **term_attach**(3) **term_attach_mem**(3) **term_done**(3) 

