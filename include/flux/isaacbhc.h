#pragma once

#include <flux/isaac.h>
#include <stddef.h>

struct flux_isaac64_bhc {
	flux_isaac64_ctx ctx;
	uint64_t weird;
	uint16_t rng;
};

struct flux_isaac32_bhc {
	flux_isaac32_ctx ctx;
	uint64_t weird;
	uint16_t rng;
};
typedef struct flux_isaac64_bhc flux_isaac64_bhc;
typedef struct flux_isaac32_bhc flux_isaac32_bhc;

typedef void (*flux_isaac32_pw_cb)(flux_isaac32_ctx*, const uint8_t * pw, size_t len);
typedef void (*flux_isaac64_pw_cb)(flux_isaac64_ctx*, const uint8_t * pw, size_t len);

void flux_isaac32_bhc_init_cb(flux_isaac32_bhc *h, const uint8_t * pw, size_t len, flux_isaac32_pw_cb);
void flux_isaac63_bhc_init_cb(flux_isaac64_bhc *h, const uint8_t * pw, size_t len, flux_isaac64_pw_cb);
void flux_isaac64_bhc_init_cb(flux_isaac64_bhc *h, const uint8_t * pw, size_t len, flux_isaac64_pw_cb);

#define flux_isaac32_bhc_init(h, pw, len) flux_isaac32_bhc_init_cb(h, pw, len, flux_isaac32_map_passphrase);
#define flux_isaac63_bhc_init(h, pw, len) flux_isaac63_bhc_init_cb(h, pw, len, flux_isaac64_map_passphrase);
#define flux_isaac64_bhc_init(h, pw, len) flux_isaac64_bhc_init_cb(h, pw, len, flux_isaac64_map_passphrase);

void flux_isaac32_map_passphrase(flux_isaac32_ctx*, const uint8_t * pw, size_t len);
void flux_isaac64_map_passphrase(flux_isaac64_ctx*, const uint8_t * pw, size_t len);
uint64_t flux_isaac_gen_weird(const uint8_t * pw, size_t len);

void flux_isaac32_bhc_block(flux_isaac32_bhc *h, uint8_t *buffer, size_t dlen);
void flux_isaac63_bhc_block(flux_isaac64_bhc *h, uint8_t *buffer, size_t dlen);
void flux_isaac64_bhc_block(flux_isaac64_bhc *h, uint8_t *buffer, size_t dlen);

#if !defined FLUX_DISABLE_RAW_NAMES
#define isaac64_bhc flux_isaac64_bhc
#define isaac32_bhc flux_isaac32_bhc

#define isaac32_bhc_init flux_isaac32_bhc_init
#define isaac32_bhc_init_cb flux_isaac32_bhc_init_cb
#define isaac32_map_passphrase flux_isaac32_map_passphrase
#define isaac32_bhc_block flux_isaac32_bhc_block

#define isaac63_bhc_init flux_isaac63_bhc_init
#define isaac63_bhc_init_cb flux_isaac63_bhc_init_cb
#define isaac63_bhc_block flux_isaac63_bhc_block

#define isaac64_bhc_init flux_isaac64_bhc_init
#define isaac64_bhc_init_cb flux_isaac64_bhc_init_cb
#define isaac64_map_passphrase flux_isaac64_map_passphrase
#define isaac64_bhc_block flux_isaac64_bhc_block

#define isaac_gen_weird flux_isaac_gen_weird
#endif
