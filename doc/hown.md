# NAME

 hown - transfer ownership of a handle

# SYNOPSIS

```c
#include <libdill.h>

int hown(
    int h);
```

# DESCRIPTION

 Handles are integers. You own a handle if you know its numeric value. To transfer ownership this function changes the number associated with the object. The old number becomes invalid and using it will result in undefined behavior. The new number can be used in exactly the same way as the old one would.

 If the function fails, the old handle is closed.

 **h**: Handle to transfer.

# RETURN VALUE

 In case of success the function returns new handle. In case of error it returns -1 and sets **errno** to one of the values below.

# ERRORS

* **EBADF**: Invalid handle.
* **EMFILE**: The maximum number of file descriptors in the process are already open.
* **ENFILE**: The maximum number of file descriptors in the system are already open.
* **ENOMEM**: Not enough memory.

# EXAMPLE

```c
int h1 = tcp_connect(&addr, deadline);
int h2 = hown(h1);
/* h1 is invalid here */
hclose(h2);
```

# SEE ALSO

 **hclose**(3) **hclose**(3) **hmake**(3) **hquery**(3) 

