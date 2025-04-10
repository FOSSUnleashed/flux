# NAME

 tls_attach_client_mem - creates TLS protocol on top of underlying socket

# SYNOPSIS

```c
#include <libdill.h>

int tls_attach_client_mem(
    int s,
    struct tls_storage* mem,
    int64_t deadline);
```

# DESCRIPTION

 **WARNING: This is experimental functionality and the API may change in the future.**

 TLS is a cryptographic protocol to provide secure communication over the network. It is a bytestream protocol.

 This function instantiates TLS protocol on top of the underlying protocol. TLS protocol being asymmetric, client and server sides are intialized in different ways. This particular function initializes the client side of the connection.

 This function allows to avoid one dynamic memory allocation by storing the object in user-supplied memory. Unless you are hyper-optimizing use **tls_attach_client** instead.

 **s**: Handle of the underlying socket. It must be a bytestream protocol.

 **mem**: The structure to store the newly created object in. It must not be deallocated before the object is closed.

 **deadline**: A point in time when the operation should time out, in milliseconds. Use the **now** function to get your current point in time. 0 means immediate timeout, i.e., perform the operation if possible or return without blocking if not. -1 means no deadline, i.e., the call will block forever if the operation cannot be performed.

 The socket can be cleanly shut down using **tls_detach** function.

 This function is not available if libdill is compiled with **--disable-sockets** option.

 This function is not available if libdill is compiled without **--enable-tls** option.

# RETURN VALUE

 In case of success the function returns newly created socket handle. In case of error it returns -1 and sets **errno** to one of the values below.

# ERRORS

* **EBADF**: Invalid handle.
* **ECANCELED**: Current coroutine was canceled.
* **ECONNRESET**: Broken connection.
* **EMFILE**: The maximum number of file descriptors in the process are already open.
* **ENFILE**: The maximum number of file descriptors in the system are already open.
* **ENOMEM**: Not enough memory.
* **ENOTSUP**: The handle does not support this operation.
* **EPROTO**: Underlying socket is not a bytestream socket.
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

 **brecv**(3) **brecvl**(3) **bsend**(3) **bsendl**(3) **hclose**(3) **now**(3) **tls_attach_client**(3) **tls_attach_server**(3) **tls_attach_server_mem**(3) **tls_detach**(3) **tls_done**(3) 

