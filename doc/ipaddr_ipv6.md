# NAME

 ipaddr_ipv6 - resolve the address of a local network interface

# SYNOPSIS

```
#include <libdill.h>

int ipaddr_ipv6(
    struct ipaddr* addr,
    const char* name,
    int port);
```

# DESCRIPTION

 Converts an IPv6 address in human-readable format into an **ipaddr** structure.

 **addr**: Out parameter, The IP address object.

 **name**: Human-readable formatted IPv6 address.

 **port**: Port number. Valid values are 1-65535.

 This function is not available if libdill is compiled with **--disable-sockets** option.

# RETURN VALUE

 In case of success the function returns 0. In case of error it returns -1 and sets **errno** to one of the values below.

# ERRORS

- **EINVAL**: Address supplied could not be parsed as an IPv6 address.

# EXAMPLE

```c
struct ipaddr addr;
ipaddr_ipv6(&addr, "::1", 5555, 0);
int s = tcp_connect(&addr, -1);
```

# SEE ALSO

**ipaddr_equal**(3) **ipaddr_family**(3) **ipaddr_len**(3) **ipaddr_port**(3) **ipaddr_remote**(3) **ipaddr_remotes**(3) **ipaddr_setport**(3) **ipaddr_sockaddr**(3) **ipaddr_str**(3) **ipaddr_ipv4**(3)
