#include <flux/isaacbhc.h>
#include <stdio.h>
#include <assert.h>

#define RANDSIZ FLUX_ISAAC_RANDSIZ

// sizeof(ctx->randrsl)
#define LOCUS32 (RANDSIZ * 4)
#define LOCUS63 (RANDSIZ * 4)
#define LOCUS64 (RANDSIZ * 8)

// Table of bit maps where 01000101 -> 10100010
static uint8_t reverse_table[256] = {
	 0x00 ,0x80 ,0x40 ,0xc0 ,0x20 ,0xa0 ,0x60 ,0xe0
	,0x10 ,0x90 ,0x50 ,0xd0 ,0x30 ,0xb0 ,0x70 ,0xf0
	,0x08 ,0x88 ,0x48 ,0xc8 ,0x28 ,0xa8 ,0x68 ,0xe8
	,0x18 ,0x98 ,0x58 ,0xd8 ,0x38 ,0xb8 ,0x78 ,0xf8
	,0x04 ,0x84 ,0x44 ,0xc4 ,0x24 ,0xa4 ,0x64 ,0xe4
	,0x14 ,0x94 ,0x54 ,0xd4 ,0x34 ,0xb4 ,0x74 ,0xf4
	,0x0c ,0x8c ,0x4c ,0xcc ,0x2c ,0xac ,0x6c ,0xec
	,0x1c ,0x9c ,0x5c ,0xdc ,0x3c ,0xbc ,0x7c ,0xfc
	,0x02 ,0x82 ,0x42 ,0xc2 ,0x22 ,0xa2 ,0x62 ,0xe2
	,0x12 ,0x92 ,0x52 ,0xd2 ,0x32 ,0xb2 ,0x72 ,0xf2
	,0x0a ,0x8a ,0x4a ,0xca ,0x2a ,0xaa ,0x6a ,0xea
	,0x1a ,0x9a ,0x5a ,0xda ,0x3a ,0xba ,0x7a ,0xfa
	,0x06 ,0x86 ,0x46 ,0xc6 ,0x26 ,0xa6 ,0x66 ,0xe6
	,0x16 ,0x96 ,0x56 ,0xd6 ,0x36 ,0xb6 ,0x76 ,0xf6
	,0x0e ,0x8e ,0x4e ,0xce ,0x2e ,0xae ,0x6e ,0xee
	,0x1e ,0x9e ,0x5e ,0xde ,0x3e ,0xbe ,0x7e ,0xfe
	,0x01 ,0x81 ,0x41 ,0xc1 ,0x21 ,0xa1 ,0x61 ,0xe1
	,0x11 ,0x91 ,0x51 ,0xd1 ,0x31 ,0xb1 ,0x71 ,0xf1
	,0x09 ,0x89 ,0x49 ,0xc9 ,0x29 ,0xa9 ,0x69 ,0xe9
	,0x19 ,0x99 ,0x59 ,0xd9 ,0x39 ,0xb9 ,0x79 ,0xf9
	,0x05 ,0x85 ,0x45 ,0xc5 ,0x25 ,0xa5 ,0x65 ,0xe5
	,0x15 ,0x95 ,0x55 ,0xd5 ,0x35 ,0xb5 ,0x75 ,0xf5
	,0x0d ,0x8d ,0x4d ,0xcd ,0x2d ,0xad ,0x6d ,0xed
	,0x1d ,0x9d ,0x5d ,0xdd ,0x3d ,0xbd ,0x7d ,0xfd
	,0x03 ,0x83 ,0x43 ,0xc3 ,0x23 ,0xa3 ,0x63 ,0xe3
	,0x13 ,0x93 ,0x53 ,0xd3 ,0x33 ,0xb3 ,0x73 ,0xf3
	,0x0b ,0x8b ,0x4b ,0xcb ,0x2b ,0xab ,0x6b ,0xeb
	,0x1b ,0x9b ,0x5b ,0xdb ,0x3b ,0xbb ,0x7b ,0xfb
	,0x07 ,0x87 ,0x47 ,0xc7 ,0x27 ,0xa7 ,0x67 ,0xe7
	,0x17 ,0x97 ,0x57 ,0xd7 ,0x37 ,0xb7 ,0x77 ,0xf7
	,0x0f ,0x8f ,0x4f ,0xcf ,0x2f ,0xaf ,0x6f ,0xef
	,0x1f ,0x9f ,0x5f ,0xdf ,0x3f ,0xbf ,0x7f ,0xff
};

static uint64_t reversebits(uint64_t W) {
	uint8_t * b=(uint8_t *)(&W);
	int i,j;
	uint8_t t;
	for (i=0,j=7; i<4; i++,j--) {
		t=b[j]=reverse_table[b[j]];
		b[j]=reverse_table[b[i]];	
		b[i]=t;
	}
	return W;
}

uint64_t flux_isaac_gen_weird(const uint8_t * P, size_t length) {
	uint64_t W=15583522643116493073ULL;
	for (int i=0, j=length-1; i<length; i++,j--) {
		W+=P[i];
		W^=((uint64_t)(P[j])<<32);
		W*=13168257484146441411ULL;
		W=reversebits(W);
	}
	W*=13168257484146441411ULL;
	W=reversebits(W);
	return W;
}

void flux_isaac64_map_passphrase(flux_isaac64_ctx * ctx, const uint8_t * pass, size_t length) {
	int i,l=0;
	for (i=0; i<256; i++) {
		for (l=0; l<length; ++l) {
			ctx->randrsl[i]+=(uint64_t)pass[l]; // ub4 ******TODO
			ctx->randrsl[i]=ctx->randrsl[i]<<8 | ctx->randrsl[i]>>24;
			i++;
			i%=256;
		}
	}
}

void flux_isaac32_map_passphrase(flux_isaac32_ctx * ctx, const uint8_t * pass, size_t length) {
	int i,l=0;
	for (i=0; i<256; i++) {
		for (l=0; l<length; ++l) {
			ctx->randrsl[i]+=(uint32_t)pass[l]; // ub4 ******TODO
			ctx->randrsl[i]=ctx->randrsl[i]<<8 | ctx->randrsl[i]>>24;
			i++;
			i%=256;
		}
	}
}

void isaac32_bhc_init_cb(isaac32_bhc *h, const uint8_t * pw, size_t pwlen, flux_isaac32_pw_cb pwcb) {
	pwcb(&h->ctx, pw, pwlen);

	flux_isaac32_init(&h->ctx, 1);

	h->rng	= LOCUS32;
	h->weird = flux_isaac_gen_weird(pw, pwlen);
}

void isaac63_bhc_init_cb(isaac64_bhc *h, const uint8_t * pw, size_t pwlen, flux_isaac64_pw_cb pwcb) {
	pwcb(&h->ctx, pw, pwlen);

	flux_isaac63_init(&h->ctx, 1);

	h->rng	= LOCUS63;
	h->weird = flux_isaac_gen_weird(pw, pwlen);
}

void isaac64_bhc_init_cb(isaac64_bhc *h, const uint8_t * pw, size_t pwlen, flux_isaac64_pw_cb pwcb) {
	pwcb(&h->ctx, pw, pwlen);

	flux_isaac64_init(&h->ctx, 1);

	h->rng	= LOCUS64;
	h->weird = flux_isaac_gen_weird(pw, pwlen);
}

static int debug_mode = 0;

static void print_debug(uint8_t cipher, uint8_t plain, uint8_t random, uint8_t weirdbyte, uint64_t weird) {
	fprintf(stderr, "cipher: %02X plain: %02X random: %02X weirdbyte: %02X weird: %016X\n"
		,(unsigned int)cipher
		,(unsigned int)plain
		,(unsigned int)random
		,(unsigned int)weirdbyte
		,weird
	);
}

void flux_isaac32_bhc_block(flux_isaac32_bhc *h, uint8_t *buffer, size_t dlen) {
	uint8_t * W=(uint8_t *)&h->weird;
	uint8_t weirdbyte, random, cipher, plain;
	int locus;
	uint8_t * R=(uint8_t *)(h->ctx.randrsl);

	for (locus = 0; locus < dlen ; ++locus, ++h->rng) {
		if (h->rng >= LOCUS32) {
			isaac32(&h->ctx);
			h->rng = 0;
		}

		plain = buffer[locus];
		random = R[h->rng];
		weirdbyte = ((W[0]+W[1])^~(W[2]+W[3]))+~((W[4]+W[5])^~(W[6]+W[7]));
		cipher = plain^(random + weirdbyte);

		if (debug_mode) print_debug(cipher, plain, random, weirdbyte, h->weird);

		buffer[locus] = cipher;

		h->weird = (h->weird<<17) | (h->weird>>47);
		h->weird ^= (uint64_t)random;
		h->weird *= 2665364157445947747;
		h->weird = reversebits(h->weird);
	}
}

void flux_isaac63_bhc_block(flux_isaac64_bhc *h, uint8_t *buffer, size_t dlen) {
	uint8_t * W=(uint8_t *)&h->weird;
	uint8_t weirdbyte, random, cipher, plain;
	int locus;
	uint8_t * R=(uint8_t *)(h->ctx.randrsl);

	for (locus = 0; locus < dlen ; ++locus, ++h->rng) {
		if (h->rng >= LOCUS63) {
			isaac63(&h->ctx);
			h->rng = 0;
		}

		plain = buffer[locus];
		random = R[h->rng];
		weirdbyte = ((W[0]+W[1])^~(W[2]+W[3]))+~((W[4]+W[5])^~(W[6]+W[7]));
		cipher = plain^(random + weirdbyte);

		if (debug_mode) print_debug(cipher, plain, random, weirdbyte, h->weird);

		buffer[locus] = cipher;

		h->weird = (h->weird<<17) | (h->weird>>47);
		h->weird ^= (uint64_t)random;
		h->weird *= 2665364157445947747;
		h->weird = reversebits(h->weird);
	}
}

void flux_isaac64_bhc_block(flux_isaac64_bhc *h, uint8_t *buffer, size_t dlen) {
	uint8_t * W=(uint8_t *)&h->weird;
	uint8_t weirdbyte, random, cipher, plain;
	int locus;
	uint8_t * R=(uint8_t *)(h->ctx.randrsl);

	for (locus = 0; locus < dlen ; ++locus, ++h->rng) {
		if (h->rng >= LOCUS64) {
			isaac64(&h->ctx);
			h->rng = 0;
		}

		plain = buffer[locus];
		random = R[h->rng];
		weirdbyte = ((W[0]+W[1])^~(W[2]+W[3]))+~((W[4]+W[5])^~(W[6]+W[7]));
		cipher = plain^(random + weirdbyte);

		if (debug_mode) print_debug(cipher, plain, random, weirdbyte, h->weird);

		buffer[locus] = cipher;

		h->weird = (h->weird<<17) | (h->weird>>47);
		h->weird ^= (uint64_t)random;
		h->weird *= 2665364157445947747;
		h->weird = reversebits(h->weird);
	}
}
