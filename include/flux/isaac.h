/*
This header is kind of an ugly mess, but I wanted to preserve the original parts as much as possible
This is a mashing of rand.h and isaac64.h to provide the 32-bit and 64-bit implimentations of ISAAC in a single header
It also removes the requirement of "standard.h" which led to the requirement to create the "63" functions
DO NOT USE THE "63" FUNCTIONS UNLESS YOU NEED THAT SPECIFIC BITSTREAM (HINT: YOU DO NOT)
The "63" functions are 64-bit ints on 32-bit logic.  Do not use them.

General usage:

isaac32_ctx ctx = {};

// Seed ctx.randrsl (array of RANDSIZ uint32_t)

isaac32_init(&ctx, 0);

// Do this as often as you need
uint32_t res = rand32(&ctx);

Replace all 32 with 64 for the 64 bit version.

*/

/*
------------------------------------------------------------------------------
rand.h: definitions for a random number generator
By Bob Jenkins, 1996, Public Domain
MODIFIED:
  960327: Creation (addition of randinit, really)
  970719: use context, not global variables, for internal state
  980324: renamed seed to flag
  980605: recommend RANDSIZL=4 for noncryptography.
  010626: note this is public domain
------------------------------------------------------------------------------
*/
/*
------------------------------------------------------------------------------
isaac64.h: definitions for a random number generator
Bob Jenkins, 1996, Public Domain
------------------------------------------------------------------------------
*/
#include <stdint.h>

#pragma once

#define FLUX_ISAAC_RANDSIZL   (8)
#define FLUX_ISAAC_RANDSIZ    (1<<FLUX_ISAAC_RANDSIZL)

/* context of random number generator */
struct flux_isaac32_ctx {
	uint32_t randcnt;
	uint32_t randrsl[FLUX_ISAAC_RANDSIZ];
	uint32_t randmem[FLUX_ISAAC_RANDSIZ];
	uint32_t randa;
	uint32_t randb;
	uint32_t randc;
};

struct flux_isaac64_ctx {
	uint64_t randcnt;
	uint64_t randrsl[FLUX_ISAAC_RANDSIZ];
	uint64_t randmem[FLUX_ISAAC_RANDSIZ];
	uint64_t randa;
	uint64_t randb;
	uint64_t randc;
};

typedef  struct flux_isaac32_ctx  flux_isaac32_ctx;
typedef  struct flux_isaac64_ctx  flux_isaac64_ctx;

/*
------------------------------------------------------------------------------
 If (flag==TRUE), then use the contents of randrsl[0..RANDSIZ-1] as the seed.
------------------------------------------------------------------------------
*/
void flux_isaac32_init(flux_isaac32_ctx *, int flag);
void flux_isaac32(flux_isaac32_ctx *);

void flux_isaac64_init(flux_isaac64_ctx *, int flag);
void flux_isaac64(flux_isaac64_ctx *);

void flux_isaac63_init(flux_isaac64_ctx *, int flag);
void flux_isaac63(flux_isaac64_ctx *);


/*
------------------------------------------------------------------------------
 Call rand(/o_ isaac32_ctx *r _o/) to retrieve a single 32-bit random value
------------------------------------------------------------------------------
*/
#define flux_rand32(r) (!(r)->randcnt-- ? (flux_isaac32(r), (r)->randcnt=RANDSIZ-1, (r)->randrsl[(r)->randcnt]) : (r)->randrsl[(r)->randcnt])
#define flux_rand64(r) (!(r)->randcnt-- ? (flux_isaac64(r), (r)->randcnt=RANDSIZ-1, (r)->randrsl[(r)->randcnt]) : (r)->randrsl[(r)->randcnt])

#if !defined FLUX_DISABLE_RAW_NAMES
#define isaac64_ctx flux_isaac64_ctx
#define isaac64 flux_isaac64
#define isaac64_init flux_isaac64_init
#define rand64 flux_rand64

#define isaac32_ctx flux_isaac32_ctx
#define isaac32 flux_isaac32
#define isaac32_init flux_isaac32_init
#define rand32 flux_rand32

#define isaac63_ctx flux_isaac64_ctx
#define isaac63 flux_isaac63
#define isaac63_init flux_isaac63_init
#endif
