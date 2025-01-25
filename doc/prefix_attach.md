# NAME

 prefix_attach - creates PREFIX protocol on top of underlying socket

# SYNOPSIS

```c
#include <libdill.h>

int prefix_attach(
    int s,
    size_t hdrlen,
    int flags);
```

# DESCRIPTION

 PREFIX  is a message-based protocol to send binary messages prefixed by size. The protocol has no initial handshake. Terminal handshake is accomplished by each peer sending size field filled by 0xff bytes.

 This function instantiates PREFIX protocol on top of the underlying protocol.

 **s**: Handle of the underlying socket. It must be a bytestream protocol.

 **hdrlen**: Size of the length field, in bytes.

 **flags**: If set to **PREFIX_BIG_ENDIAN** (also known as network byte order, the default option) the most significant byte of the size will be sent first on the wire. If set to **PREFIX_LITTLE_ENDIAN** the least signiticant byte will come first.

 The socket can be cleanly shut down using **prefix_detach** function.

 This function is not available if libdill is compiled with **--disable-sockets** option.

# RETURN VALUE

 In case of success the function returns newly created socket handle. In case of error it returns -1 and sets **errno** to one of the values below.

# ERRORS

* **EBADF**: Invalid handle.
* **EINVAL**: Invalid argument.
* **EMFILE**: The maximum number of file descriptors in the process are already open.
* **ENFILE**: The maximum number of file descriptors in the system are already open.
* **ENOMEM**: Not enough memory.
* **ENOTSUP**: The handle does not support this operation.
* **EPROTO**: Underlying socket is not a bytestream socket.

# EXAMPLE

```c
int s = tcp_connect(&addr, -1);
s = prefix_attach(s, 2, 0);
msend(s, "ABC", 3, -1);
char buf[256];
ssize_t sz = mrecv(s, buf, sizeof(buf), -1);
s = prefix_detach(s, -1);
tcp_close(s);
```

# SEE ALSO

 **hclose**(3) **mrecv**(3) **mrecvl**(3) **msend**(3) **msendl**(3) **prefix_attach_mem**(3) **prefix_detach**(3) 

