#pragma once

#include <stdint.h>
#include <flux/base32.h>
#include <flux/isaac.h>

// TODO ULID function with private context?
uint8_t * flux_ulid32(uint8_t * out, uint8_t * oend, flux_isaac32_ctx * ctx);
uint8_t * flux_ulid64(uint8_t * out, uint8_t * oend, flux_isaac64_ctx * ctx);

#if !defined FLUX_DISABLE_RAW_NAMES
#define ulid32 flux_ulid32
#define ulid64 flux_ulid64
#endif
