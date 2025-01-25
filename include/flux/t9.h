#pragma once

#include <dill/util.h>
#include <dill/bsock.h>
#include <c9_protoT.h>

#define T9_MSZ (1 << 13)

typedef struct {
	C9ctx ctx;
	handle h;
	uint8_t rbuf[T9_MSZ], wbuf[T9_MSZ];
	uint32_t woff;
} T9client;

typedef struct {
	T9client *c;
	C9fid id;

	// flags
} T9fid;

typedef struct {
	T9fid *f;
} T9session;

T9client *t9openClient(handle h);
