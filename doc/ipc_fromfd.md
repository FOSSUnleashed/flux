# NAME

 ipc_fromfd - wraps an existing OS-level file descriptor

# SYNOPSIS

```c
#include <libdill.h>

int ipc_fromfd(
    int fd);
```

# DESCRIPTION

 IPC  protocol is a bytestream protocol for transporting data among processes on the same machine.  It is an equivalent to POSIX **AF_LOCAL** sockets.

 This function wraps an existing OS-level file descriptor.

 **fd**: File descriptor of a connected UNIX doemain socket to wrap.

 The socket can be cleanly shut down using **ipc_close** function which will also close the underlying file descriptor.

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
int fd = socket(AF_UNIX, SOCK_STREAM, 0);
connect(fd, addr, sizeof(addr));
int s = ipc_fromfd(fd);
bsend(s, "ABC", 3, -1);
char buf[3];
brecv(s, buf, sizeof(buf), -1);
ipc_close(s);
```

# SEE ALSO

 **brecv**(3) **brecvl**(3) **bsend**(3) **bsendl**(3) **hclose**(3) **ipc_accept**(3) **ipc_accept_mem**(3) **ipc_close**(3) **ipc_connect**(3) **ipc_connect_mem**(3) **ipc_done**(3) **ipc_fromfd_mem**(3) **ipc_listen**(3) **ipc_listen_mem**(3) **ipc_listener_fromfd**(3) **ipc_listener_fromfd_mem**(3) **ipc_pair**(3) **ipc_pair_mem**(3) 

