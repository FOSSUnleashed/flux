# NAME

 bundle_wait - wait while coroutines in the bundle finish

# SYNOPSIS

```c
#include <libdill.h>

int bundle_wait(
    int bndl,
    int64_t deadline);
```

# DESCRIPTION

 If there are no coroutines in the bundle the function will succeed immediately. Otherwise, it will wait until all the coroutines in the bundle finish or until deadline is reached.

 **bndl**: Handle of a coroutine bundle.

 **deadline**: A point in time when the operation should time out, in milliseconds. Use the **now** function to get your current point in time. 0 means immediate timeout, i.e., perform the operation if possible or return without blocking if not. -1 means no deadline, i.e., the call will block forever if the operation cannot be performed.

# RETURN VALUE

 In case of success the function returns 0. In case of error it returns -1 and sets **errno** to one of the values below.

# ERRORS

* **EBADF**: Invalid handle.
* **ECANCELED**: Current coroutine was canceled.
* **ENOTSUP**: The handle does not support this operation.
* **ETIMEDOUT**: Deadline was reached.

# EXAMPLE

```c
int b = bundle();
bundle_go(b, worker());
bundle_go(b, worker());
bundle_go(b, worker());
/* Give wrokers 1 second to finish. */
bundle_wait(b, now() + 1000);
/* Cancel any remaining workers. */
hclose(b);
```

# SEE ALSO

 **bundle**(3) **bundle_go**(3) **bundle_go_mem**(3) **bundle_mem**(3) **go**(3) **go_mem**(3) **now**(3) **yield**(3) 

