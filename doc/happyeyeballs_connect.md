# NAME

 happyeyeballs_connect - creates a TCP connection using Happy Eyeballs algorithm

# SYNOPSIS

```c
#include <libdill.h>

int happyeyeballs_connect(
    const char* name,
    int port,
    int64_t deadline);
```

# DESCRIPTION

 Happy Eyeballs protocol (RFC 8305) is a way to create a TCP connection to a remote endpoint even if some of the IP addresses associated with the endpoint are not accessible.

 First, DNS queries to IPv4 and IPv6 addresses are launched in parallel. Then a connection is attempted to the first address, with IPv6 address being preferred. If the connection attempt doesn't succeed in 300ms connection to next address is attempted, alternating between IPv4 and IPv6 addresses. However, the first connection attempt isn't caneled. If, in the next 300ms, none of the previous attempts succeeds connection to the next address is attempted, and so on. First successful connection attempt will cancel all other connection attemps.

 This function executes the above protocol and returns the newly created TCP connection.

 **name**: Name of the host to connect to.

 **port**: Port to connect to.

 **deadline**: A point in time when the operation should time out, in milliseconds. Use the **now** function to get your current point in time. 0 means immediate timeout, i.e., perform the operation if possible or return without blocking if not. -1 means no deadline, i.e., the call will block forever if the operation cannot be performed.

# RETURN VALUE

 In case of success the function returns newly created TCP connection. In case of error it returns -1 and sets **errno** to one of the values below.

# ERRORS

* **ECANCELED**: Current coroutine was canceled.
* **EINVAL**: Invalid argument.
* **EMFILE**: The maximum number of file descriptors in the process are already open.
* **ENFILE**: The maximum number of file descriptors in the system are already open.
* **ENOMEM**: Not enough memory.
* **ETIMEDOUT**: Deadline was reached.

# EXAMPLE

```c
int s = happyeyeballs_connect("www.example.org", 80, -1);
int rc = bsend(s, "GET / HTTP/1.1", 14, -1);
```

# SEE ALSO

 **now**(3) 

