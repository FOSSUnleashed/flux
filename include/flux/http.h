#pragma once

#include <flux/str.h>

FluxBuffer flux_http_urlencode(FluxBuffer dst, FluxBuffer key, FluxBuffer value);

#if !defined FLUX_DISABLE_RAW_NAMES
#define http_urlencode flux_http_urlencode
#endif
