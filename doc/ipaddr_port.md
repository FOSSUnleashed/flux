# NAME

 ipaddr_port - returns the port part of the address

# SYNOPSIS

```c
#include <libdill.h>

int ipaddr_port(
    const struct ipaddr* addr);
```

# DESCRIPTION

 Returns the port part of the address.

 **addr**: IP address object.

 This function is not available if libdill is compiled with **--disable-sockets** option.

# RETURN VALUE

 The port number.

# ERRORS

 None.

# EXAMPLE

```c
int port = ipaddr_port(&addr);
```

# SEE ALSO

 **ipaddr_equal**(3) **ipaddr_family**(3) **ipaddr_len**(3) **ipaddr_local**(3) **ipaddr_remote**(3) **ipaddr_remotes**(3) **ipaddr_setport**(3) **ipaddr_sockaddr**(3) **ipaddr_str**(3) 

