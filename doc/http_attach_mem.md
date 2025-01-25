# NAME

 http_attach_mem - creates HTTP protocol on top of underlying socket

# SYNOPSIS

```c
#include <libdill.h>

int http_attach_mem(
    int s,
    struct http_storage* mem);
```

# DESCRIPTION

 **WARNING: This is experimental functionality and the API may change in the future.**

 HTTP is an application-level protocol described in RFC 7230. This implementation handles only the request/response exchange. Whatever comes after that must be handled by a different protocol.

 This function instantiates HTTP protocol on top of the underlying protocol.

 This function allows to avoid one dynamic memory allocation by storing the object in user-supplied memory. Unless you are hyper-optimizing use **http_attach** instead.

 **s**: Handle of the underlying socket. It must be a bytestream protocol.

 **mem**: The structure to store the newly created object in. It must not be deallocated before the object is closed.

 The socket can be cleanly shut down using **http_detach** function.

 This function is not available if libdill is compiled with **--disable-sockets** option.

# RETURN VALUE

 In case of success the function returns newly created socket handle. In case of error it returns -1 and sets **errno** to one of the values below.

# ERRORS

* **EBADF**: Invalid handle.
* **EMFILE**: The maximum number of file descriptors in the process are already open.
* **ENFILE**: The maximum number of file descriptors in the system are already open.
* **ENOMEM**: Not enough memory.
* **ENOTSUP**: The handle does not support this operation.
* **EPROTO**: Underlying socket is not a bytestream socket.

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

 **hclose**(3) **http_attach**(3) **http_detach**(3) **http_done**(3) **http_recvfield**(3) **http_recvrequest**(3) **http_recvstatus**(3) **http_sendfield**(3) **http_sendrequest**(3) **http_sendstatus**(3) 

