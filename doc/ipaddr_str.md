# NAME

 ipaddr_str - convert address to a human-readable string

# SYNOPSIS

```c
#include <libdill.h>

const char* ipaddr_str(
    const struct ipaddr* addr,
    char* buf);
```

# DESCRIPTION

 Formats address as a human-readable string.

 **addr**: IP address object.

 **buf**: Buffer to store the result in. It must be at least **IPADDR_MAXSTRLEN** bytes long.

 This function is not available if libdill is compiled with **--disable-sockets** option.

# RETURN VALUE

 The function returns **ipstr** argument, i.e.  pointer to the formatted string.

# ERRORS

 None.

# EXAMPLE

```c
char buf[IPADDR_MAXSTRLEN];
ipaddr_str(&addr, buf);
```

# SEE ALSO

 **ipaddr_equal**(3) **ipaddr_family**(3) **ipaddr_len**(3) **ipaddr_local**(3) **ipaddr_port**(3) **ipaddr_remote**(3) **ipaddr_remotes**(3) **ipaddr_setport**(3) **ipaddr_sockaddr**(3) 

