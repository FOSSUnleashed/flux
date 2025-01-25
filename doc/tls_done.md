# NAME

 tls_done - half-closes a TLS protocol

# SYNOPSIS

```c
#include <libdill.h>

int tls_done(
    int s,
    int64_t deadline);
```

# DESCRIPTION

 **WARNING: This is experimental functionality and the API may change in the future.**

 TLS is a cryptographic protocol to provide secure communication over the network. It is a bytestream protocol.

 This function closes the outbound half of TLS connection. This will, in turn, cause the peer to get **EPIPE** error after it has received all the data.

 **s**: The TLS connection handle.

 **deadline**: A point in time when the operation should time out, in milliseconds. Use the **now** function to get your current point in time. 0 means immediate timeout, i.e., perform the operation if possible or return without blocking if not. -1 means no deadline, i.e., the call will block forever if the operation cannot be performed.

 This function is not available if libdill is compiled with **--disable-sockets** option.

 This function is not available if libdill is compiled without **--enable-tls** option.

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
s = tls_attach_client(s, -1);
bsend(s, "ABC", 3, -1);
char buf[3];
ssize_t sz = brecv(s, buf, sizeof(buf), -1);
s = tls_detach(s, -1);
tcp_close(s);
```

# SEE ALSO

 **brecv**(3) **brecvl**(3) **bsend**(3) **bsendl**(3) **now**(3) **tls_attach_client**(3) **tls_attach_client_mem**(3) **tls_attach_server**(3) **tls_attach_server_mem**(3) **tls_detach**(3) 

