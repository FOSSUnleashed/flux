# NAME

 ipaddr_equal - compares two IP address

# SYNOPSIS

```c
#include <libdill.h>

int ipaddr_equal(
    const struct ipaddr* addr1,
    const struct ipaddr* addr2,
    int ignore_port);
```

# DESCRIPTION

 This function returns 1 is the two supplied IP addresses are the same or zero otherwise.

 **addr1**: First IP address.

 **addr2**: Second IP address.

 **ignore_port**: If set to zero addresses with different ports will be considered unequal. Otherwise, ports won't be taken into account.

 This function is not available if libdill is compiled with **--disable-sockets** option.

# RETURN VALUE

 1 if the arguments are equal, 0 otherwise.

# ERRORS

 None.

# EXAMPLE

```c
ipaddr addr;
ipaddr_remote(&addr, "www.example.org", 80, 0, -1);
int s = socket(ipaddr_family(addr), SOCK_STREAM, 0);
connect(s, ipaddr_sockaddr(&addr), ipaddr_len(&addr));
```

# SEE ALSO

 **ipaddr_family**(3) **ipaddr_len**(3) **ipaddr_local**(3) **ipaddr_port**(3) **ipaddr_remote**(3) **ipaddr_remotes**(3) **ipaddr_setport**(3) **ipaddr_sockaddr**(3) **ipaddr_str**(3) 

