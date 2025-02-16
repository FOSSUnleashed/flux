# NAME

 prefix_detach - terminates PREFIX protocol and returns the underlying socket

# SYNOPSIS

```c
#include <libdill.h>

int prefix_detach(int s);
```

# DESCRIPTION

 PREFIX  is a message-based protocol to send binary messages prefixed by size. The protocol has no initial handshake. Terminal handshake is accomplished by each peer sending size field filled by 0xff bytes.

 This function does the terminal handshake and returns underlying socket to the user. The socket is closed even in the case of error.

 **s**: Handle of the PREFIX socket.

 **deadline**: A point in time when the operation should time out, in milliseconds. Use the **now** function to get your current point in time. 0 means immediate timeout, i.e., perform the operation if possible or return without blocking if not. -1 means no deadline, i.e., the call will block forever if the operation cannot be performed.

 This function is not available if libdill is compiled with **--disable-sockets** option.

# RETURN VALUE

 In case of success the function returns underlying socket handle. In case of error it returns -1 and sets **errno** to one of the values below.

# ERRORS

* **EBADF**: Invalid handle.
* **ECANCELED**: Current coroutine was canceled.
* **ECONNRESET**: Broken connection.
* **ENOTSUP**: The handle is not a PREFIX protocol handle.
* **ETIMEDOUT**: Deadline was reached.

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

 **mrecv**(3) **mrecvl**(3) **msend**(3) **msendl**(3) **now**(3) **prefix_attach**(3) **prefix_attach_mem**(3) 

