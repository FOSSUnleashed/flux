# NAME

 fdout - wait on file descriptor to become writable

# SYNOPSIS

```c
#include <libdill.h>

int fdout(
    int fd,
    int64_t deadline);
```

# DESCRIPTION

 Waits on a file descriptor (true OS file descriptor, not libdill socket handle) to either become writeable or get into an error state. Either case leads to a successful return from the function. To distinguish the two outcomes, follow up with a write operation on the file descriptor.

 **fd**: file descriptor (OS-level one, not a libdill handle)

 **deadline**: A point in time when the operation should time out, in milliseconds. Use the **now** function to get your current point in time. 0 means immediate timeout, i.e., perform the operation if possible or return without blocking if not. -1 means no deadline, i.e., the call will block forever if the operation cannot be performed.

# RETURN VALUE

 In case of success the function returns 0. In case of error it returns -1 and sets **errno** to one of the values below.

# ERRORS

* **EBADF**: Not a file descriptor.
* **EBUSY**: Another coroutine already blocked on **fdout** with this file descriptor.
* **ECANCELED**: Current coroutine was canceled.
* **ETIMEDOUT**: Deadline was reached.

# EXAMPLE

```c
int result = fcntl(fd, F_SETFL, O_NONBLOCK);
assert(result == 0);
while(len) {
    result = fdout(fd, -1);
    assert(result == 0);
    ssize_t sent = send(fd, buf, len, 0);
    assert(len > 0);
    buf += sent;
    len -= sent;
}
```

# SEE ALSO

 **fdclean**(3) **fdin**(3) **now**(3) 

