# NAME

 brecvl - receives data from a bytestream socket

# SYNOPSIS

```c
#include <libdill.h>

int brecvl(
    int s,
    struct iolist* first,
    struct iolist* last,
    int64_t deadline);
```

# DESCRIPTION

 This function receives data from a bytestream socket. It is a blocking operation that unblocks only after the requested amount of data is received.  There is no such thing as partial receive. If a problem, including timeout, occurs while receiving the data, an error is returned to the user and the socket cannot be used for receiving from that point on.

 This function accepts a linked list of I/O buffers instead of a single buffer. Argument **first** points to the first item in the list, **last** points to the last buffer in the list. The list represents a single, fragmented message, not a list of multiple messages. Structure **iolist** has the following members:

```c
void *iol_base;          /* Pointer to the buffer. */
size_t iol_len;          /* Size of the buffer. */
struct iolist *iol_next; /* Next buffer in the list. */
int iol_rsvd;            /* Reserved. Must be set to zero. */
```

 When receiving, **iol_base** equal to NULL means that **iol_len** bytes should be skipped.

 The function returns **EINVAL** error in the case the list is malformed:

* If **last->iol_next** is not **NULL**.
* If **first** and **last** don't belong to the same list.
* If there's a loop in the list.
* If **iol_rsvd** of any item is non-zero.

 The list (but not the buffers themselves) can be temporarily modified while the function is in progress. However, once the function returns the list is guaranteed to be the same as before the call.

 **s**: The socket.

 **first**: Pointer to the first item of a linked list of I/O buffers.

 **last**: Pointer to the last item of a linked list of I/O buffers.

 **deadline**: A point in time when the operation should time out, in milliseconds. Use the **now** function to get your current point in time. 0 means immediate timeout, i.e., perform the operation if possible or return without blocking if not. -1 means no deadline, i.e., the call will block forever if the operation cannot be performed.

# RETURN VALUE

 In case of success the function returns 0. In case of error it returns -1 and sets **errno** to one of the values below.

# ERRORS

* **EBADF**: Invalid handle.
* **EBUSY**: The handle is currently being used by a different coroutine.
* **ECANCELED**: Current coroutine was canceled.
* **ECONNRESET**: Broken connection.
* **EINVAL**: Invalid argument.
* **ENOTSUP**: The handle does not support this operation.
* **EPIPE**: Closed connection.
* **ETIMEDOUT**: Deadline was reached.

# SEE ALSO

 **brecv**(3) **bsend**(3) **bsendl**(3) **now**(3) 

