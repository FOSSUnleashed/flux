#pragma once

#include <dill/util.h>

enum FluxSockType {
	FLUX_MSOCK
	,FLUX_BSOCK
};

//enum FluxSockType flux_dial(const char * addr, handle *h);
handle flux_dial0(const uint8_t *addr, enum FluxSockType *fst, int64_t deadline);
handle flux_dial(const uint8_t *addr, const uint8_t *eaddr, enum FluxSockType *fst, int64_t deadline);

handle flux_listen(const uint8_t * addr, int backlog);

const uint8_t * flux_getns();
const uint8_t * flux_getdisplay();
const uint8_t * flux_getuser();

handle flux_nsconnect(const uint8_t *addr, const uint8_t *eaddr, int64_t deadline);
handle flux_tcpconnect(const uint8_t *addr, const uint8_t *eaddr, uint16_t port, int64_t deadline);
handle flux_tcpconnect_s(const uint8_t *addr, const uint8_t *eaddr, uint8_t *port, uint8_t *eport, int64_t deadline);

#define dial flux_dial
#define dial0 flux_dial0
#define listen flux_listen
#define getns flux_getns
#define getdisplay flux_getdisplay
#define getuser flux_getuser
