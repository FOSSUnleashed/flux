#pragma once

#include <flux/dial.h>

typedef struct {
	handle h;
	FluxSockType socktype;
	char * dialaddr;
} Factotum;


int flux_factotum_rpc(Factotum *f, char * resp, uint64_t deadline, const char * fmt, ...);


