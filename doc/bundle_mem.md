# NAME

 bundle_mem - create an empty coroutine bundle

# SYNOPSIS

```c
#include <libdill.h>

int bundle_mem(
    struct bundle_storage* mem);
```

# DESCRIPTION

 Coroutines are always running in bundles. Even a single coroutine created by **go** gets its own bundle. A bundle is a lifetime control mechanism. When it is closed, all coroutines within the bundle are canceled.

 This function creates an empty bundle. Coroutines can be added to the bundle using the **bundle_go** and **bundle_go_mem** functions.

 To wait until coroutines in the bundle finish use **bundle_wait** function.

 When **hclose()** is called on the bundle, all the coroutines contained in the bundle will be canceled. In other words, all the blocking functions within the coroutine will start failing with an **ECANCELED** error. The **hclose()** function itself won't exit until all the coroutines in the bundle exit.

 This function allows to avoid one dynamic memory allocation by storing the object in user-supplied memory. Unless you are hyper-optimizing use **bundle** instead.

 **mem**: The structure to store the newly created object in. It must not be deallocated before the object is closed.

# RETURN VALUE

 In case of success the function returns handle of the newly create coroutine bundle. In case of error it returns -1 and sets **errno** to one of the values below.

# ERRORS

* **EMFILE**: The maximum number of file descriptors in the process are already open.
* **ENFILE**: The maximum number of file descriptors in the system are already open.
* **ENOMEM**: Not enough memory.

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

 **bundle**(3) **bundle_go**(3) **bundle_go_mem**(3) **bundle_wait**(3) **go**(3) **go_mem**(3) **hclose**(3) **yield**(3) 

