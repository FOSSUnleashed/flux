# NAME

 suffix_attach - creates SUFFIX protocol on top of underlying socket

# SYNOPSIS

```c
#include <libdill.h>

int suffix_attach(
    int s,
    const void* suffix,
    size_t suffixlen);
```

# DESCRIPTION

 SUFFIX is a message-based protocol that delimits messages by specific byte sequences. For example, many protocols are line-based, with individual messages separated by CR+LF sequence.

 This function instantiates SUFFIX protocol on top of the underlying protocol.

 **s**: Handle of the underlying socket. It must be a bytestream protocol.

 **suffix**: The delimiter byte sequence.

 **suffixlen**: The size of the delimiter, in bytes.

 The socket can be cleanly shut down using **suffix_detach** function.

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
        s = suffix_attach(s, "
", 2);
        msend(s, "ABC", 3, -1);
        char buf[256];
        ssize_t sz = mrecv(s, buf, sizeof(buf), -1);
        s = suffix_detach(s, -1);
        tcp_close(s);
```

# SEE ALSO

 **hclose**(3) **mrecv**(3) **mrecvl**(3) **msend**(3) **msendl**(3) **suffix_attach_mem**(3) **suffix_detach**(3) 

