# NAME

 ipc_done - half-closes a IPC connection

# SYNOPSIS

```c
#include <libdill.h>

int ipc_done(
    int s,
    int64_t deadline);
```

# DESCRIPTION

 IPC  protocol is a bytestream protocol for transporting data among processes on the same machine.  It is an equivalent to POSIX **AF_LOCAL** sockets.

 This function closes the outbound half of ICP connection. This will cause the peer to get **EPIPE** error after it has received all the data.

 **s**: The IPC connection handle.

 **deadline**: A point in time when the operation should time out, in milliseconds. Use the **now** function to get your current point in time. 0 means immediate timeout, i.e., perform the operation if possible or return without blocking if not. -1 means no deadline, i.e., the call will block forever if the operation cannot be performed.

 This function is not available if libdill is compiled with **--disable-sockets** option.

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
int ls = ipc_listen("/tmp/test.ipc", 10);
int s = ipc_accept(ls, -1);
bsend(s, "ABC", 3, -1);
char buf[3];
brecv(s, buf, sizeof(buf), -1);
ipc_close(s);
ipc_close(ls);
```

# SEE ALSO

 **brecv**(3) **brecvl**(3) **bsend**(3) **bsendl**(3) **ipc_accept**(3) **ipc_accept_mem**(3) **ipc_close**(3) **ipc_connect**(3) **ipc_connect_mem**(3) **ipc_fromfd**(3) **ipc_fromfd_mem**(3) **ipc_listen**(3) **ipc_listen_mem**(3) **ipc_listener_fromfd**(3) **ipc_listener_fromfd_mem**(3) **ipc_pair**(3) **ipc_pair_mem**(3) **now**(3) 

