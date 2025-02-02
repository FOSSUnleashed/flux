#pragma once

#include <stdint.h>
#include <flux/ulid.h>
#include <flux/isaac.h>

// TODO ULID function with private context?
uint8_t * flux_ulid32(uint8_t * out, uint8_t * oend, flux_isaac32_ctx * ctx);
uint8_t * flux_ulid64(uint8_t * out, uint8_t * oend, flux_isaac64_ctx * ctx);

// Crawford's base32:

uint8_t * flux_enbase32(uint8_t * out, uint8_t * oend, uint8_t * in, uint8_t * iend);
uint8_t * flux_debase32(uint8_t * out, uint8_t * oend, uint8_t * in, uint8_t * iend);
