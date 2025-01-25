# NAME

 udp_sendl - sends an UDP packet

# SYNOPSIS

```c
#include <libdill.h>

int udp_sendl(
    int s,
    const struct ipaddr* addr,
    struct iolist* first,
    struct iolist* last);
```

# DESCRIPTION

 UDP is an unreliable message-based protocol defined in RFC 768. The size of the message is limited. The protocol has no initial or terminal handshake. A single socket can be used to different destinations.

 This function sends an UDP packet.

 Given that UDP protocol is unreliable the function has no deadline. If packet cannot be sent it will be silently dropped.

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

 **s**: Handle of the UDP socket.

 **addr**: IP address to send the packet to. If set to **NULL** remote address specified in **udp_open** function will be used.

 **first**: Pointer to the first item of a linked list of I/O buffers.

 **last**: Pointer to the last item of a linked list of I/O buffers.

 This function is not available if libdill is compiled with **--disable-sockets** option.

# RETURN VALUE

 In case of success the function returns 0. In case of error it returns -1 and sets **errno** to one of the values below.

# ERRORS

* **EBADF**: Invalid handle.
* **EINVAL**: Invalid argument.
* **EMSGSIZE**: The message is too long to fit into an UDP packet.
* **ENOTSUP**: The handle does not support this operation.

# EXAMPLE

```c
struct ipaddr local;
ipaddr_local(&local, NULL, 5555, 0);
struct ipaddr remote;
ipaddr_remote(&remote, "server.example.org", 5555, 0, -1);
int s = udp_open(&local, &remote);
udp_send(s1, NULL, "ABC", 3);
char buf[2000];
ssize_t sz = udp_recv(s, NULL, buf, sizeof(buf), -1);
hclose(s);
```

# SEE ALSO

 **mrecv**(3) **mrecvl**(3) **msend**(3) **msendl**(3) **udp_open**(3) **udp_open_mem**(3) **udp_recv**(3) **udp_recvl**(3) **udp_send**(3) 

