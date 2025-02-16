#pragma once

#define flux_min(a, b) (((a) < (b)) ? (a) : (b))
#define flux_max(a, b) (((a) > (b)) ? (a) : (b))
#define flux_abs(n) ((n) > 0 ? (n) : -(n))

#if !defined FLUX_DISABLE_RAW_NAMES
#define min flux_min
#define max flux_max
#define abs flux_abs
#endif
