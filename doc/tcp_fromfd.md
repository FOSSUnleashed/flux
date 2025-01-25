# NAME

 tcp_fromfd - wraps an existing OS-level file descriptor

# SYNOPSIS

```c
#include <libdill.h>

int tcp_fromfd(
    int fd);
```

# DESCRIPTION

 TCP protocol is a reliable bytestream protocol for transporting data over network. It is defined in RFC 793.

 This function wraps an existing OS-level file descriptor.

 **fd**: File descriptor of a connected TCP socket to wrap.

 The socket can be cleanly shut down using **tcp_close** function which will also close the underlying file descriptor.

 There's no way to unwrap the file descriptor.

 This function is not available if libdill is compiled with **--disable-sockets** option.

# RETURN VALUE

 In case of success the function returns newly created socket handle. In case of error it returns -1 and sets **errno** to one of the values below.

# ERRORS

* **EMFILE**: The maximum number of file descriptors in the process are already open.
* **ENFILE**: The maximum number of file descriptors in the system are already open.
* **ENOMEM**: Not enough memory.

# EXAMPLE

```c
int fd = socket(AF_INET, SOCK_STREAM, 0);
connect(fd, addr, sizeof(addr));
int s = tcp_fromfd(fd);
bsend(s, "ABC", 3, -1);
char buf[3];
brecv(s, buf, sizeof(buf), -1);
tcp_close(s);
```

# SEE ALSO

 **brecv**(3) **brecvl**(3) **bsend**(3) **bsendl**(3) **hclose**(3) **tcp_accept**(3) **tcp_accept_mem**(3) **tcp_close**(3) **tcp_connect**(3) **tcp_connect_mem**(3) **tcp_done**(3) **tcp_fromfd_mem**(3) **tcp_listen**(3) **tcp_listen_mem**(3) **tcp_listener_fromfd**(3) **tcp_listener_fromfd_mem**(3) 

