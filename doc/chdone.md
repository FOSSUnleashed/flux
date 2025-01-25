# NAME

 chdone - half-closes a channel

# SYNOPSIS

```c
#include <libdill.h>

int chdone(
    int ch);
```

# DESCRIPTION

 Closes an outbound half of the channel. When the peer receives all the messages sent prior to the calling **chdone** all its subsequent attempts to receive will fail with **EPIPE** error.

 **ch**: The channel.

# RETURN VALUE

 In case of success the function returns 0. In case of error it returns -1 and sets **errno** to one of the values below.

# ERRORS

* **EBADF**: Invalid handle.
* **ENOTSUP**: The handle does not support this operation.
* **EPIPE**: chdone was already called on this channel.

# EXAMPLE

```c
int ch[2];
chmake(ch);
chsend(ch, "ABC", 3, -1);
chdone(ch);
```

# SEE ALSO

 **chmake**(3) **chmake_mem**(3) **choose**(3) **chrecv**(3) **chsend**(3) 

