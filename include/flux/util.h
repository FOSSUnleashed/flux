#pragma once

#include <stddef.h> // For offsetof

#define flux_min(a, b) (((a) < (b)) ? (a) : (b))
#define flux_max(a, b) (((a) > (b)) ? (a) : (b))
#define flux_abs(n) ((n) > 0 ? (n) : -(n))

#define flux_containerof(ptr, type, member) (ptr ? ((type*) (((char*) ptr) - offsetof(type, member))) : NULL)
#define flux_endof(arr) ((arr) + (sizeof(arr)/sizeof(*(arr))))

#define flux_unstruct(Type) typedef struct Type Type

#define _flux_cat(a, b) a ## b

#define _flux_va_size(_01, _02, _03, _04, _05, _06, _07, _08, _09, _10, N, ...) N
#define flux_va_size(...) _flux_va_size("dead", ##__VA_ARGS__, 09, 08, 07, 06, 05, 04, 03, 02, 01, 00)

#define flux_cat(a, b) _flux_cat(a, b)

#define flux_fe_00(_call, ...)
#define flux_fe_01(_call, x) _call(x)
#define flux_fe_02(_call, x, ...) _call(x) flux_fe_01(_call, __VA_ARGS__)
#define flux_fe_03(_call, x, ...) _call(x) flux_fe_02(_call, __VA_ARGS__)
#define flux_fe_04(_call, x, ...) _call(x) flux_fe_03(_call, __VA_ARGS__)
#define flux_fe_05(_call, x, ...) _call(x) flux_fe_04(_call, __VA_ARGS__)
#define flux_fe_06(_call, x, ...) _call(x) flux_fe_05(_call, __VA_ARGS__)
#define flux_fe_07(_call, x, ...) _call(x) flux_fe_06(_call, __VA_ARGS__)
#define flux_fe_08(_call, x, ...) _call(x) flux_fe_07(_call, __VA_ARGS__)
#define flux_fe_09(_call, x, ...) _call(x) flux_fe_08(_call, __VA_ARGS__)
#define flux_fe_10(_call, x, ...) _call(x) flux_fe_09(_call, __VA_ARGS__)

#if !defined FLUX_DISABLE_RAW_NAMES
#define min flux_min
#define max flux_max
#define abs flux_abs

#define containerof flux_containerof
#define endof flux_endof

#endif
