#pragma once

#include <stdint.h>

// Crawford's base32:

uint8_t * flux_enbase32(uint8_t * out, uint8_t * oend, uint8_t * in, uint8_t * iend);
uint8_t * flux_debase32(uint8_t * out, uint8_t * oend, uint8_t * in, uint8_t * iend);

#if !defined FLUX_DISABLE_RAW_NAMES
#define enbase32 flux_enbase32
#define debase32 flux_debase32
#endif
