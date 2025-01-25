#pragma once

#include <dill/util.h>

enum FluxSockType {
	FLUX_MSOCK
	,FLUX_BSOCK
};

FluxSockType flux_dial(const char * addr, handle *h);

handle flux_listen(const char * addr);

const char * flux_getns();

#define dial flux_dial
#define listen flux_listen
#define getns flux_getns
