#pragma once

#include <stddef.h> // For offsetof

#define flux_min(a, b) (((a) < (b)) ? (a) : (b))
#define flux_max(a, b) (((a) > (b)) ? (a) : (b))
#define flux_abs(n) ((n) > 0 ? (n) : -(n))

#define flux_containerof(ptr, type, member) (ptr ? ((type*) (((char*) ptr) - offsetof(type, member))) : NULL)
#define flux_endof(arr) ((arr) + (sizeof(arr)/sizeof(*(arr))))

#define flux_unstruct(Type) typedef struct Type Type

#define flux_cat(a, b) a ## b

#if !defined FLUX_DISABLE_RAW_NAMES
#define min flux_min
#define max flux_max
#define abs flux_abs

#define containerof flux_containerof
#define endof flux_endof

#endif
