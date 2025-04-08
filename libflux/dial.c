#include <flux/dial.h>
#include <errno.h>
#include <dill/ipc.h>
#include <dill/udp.h>
#include <dill/tcp.h>

// getenv()
#include <stdlib.h>
#include <flux/str.h>

#define getenv (uint8_t *)getenv

// getuid()
// getpwuid()
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>

/*
       int getpwuid_r(uid_t uid, struct passwd *restrict pwd,
                      char buf[restrict .buflen], size_t buflen,
                      struct passwd **restrict result);
// */


static uint8_t _NAMESPACE[256] = {};
static uint8_t _DISPLAY[256] = {};

static Buffer NAMESPACE = BUFLIT(_NAMESPACE);
static Buffer DISPLAY	= BUFLIT(_DISPLAY);

const uint8_t * getdisplay() {
	uint8_t * env;

	if (!*DISPLAY.start) {
		env = getenv("DISPLAY");

		if (env) {
			DISPLAY	= flux_buflcpy(DISPLAY, string2buffer(env, 256));
		} else {
			return NULL;
		}
	}

	return DISPLAY.start;
}

const uint8_t * getns() {
	uint8_t * env;

	if (!*NAMESPACE.start) {
		if (NULL != (env = getenv("NAMESPACE"))) {
			NAMESPACE	= flux_buflcpy(NAMESPACE, string2buffer(env, 256));
			goto end;
		}

		// WMII_ADDRESS actually
		if (NULL != (env = getenv("WMII_NAMESPACE"))) {
			NAMESPACE = flux_buflcpy(NAMESPACE, string2buffer(env, 256));
			goto end;
		}

#if 0
		if (!*DISPLAY) {
			getdisplay();
		}
#endif

		// /tmp/ns.$USER.$DISPLAY
		return NULL;
	}

	end:
	return NAMESPACE.start;
}

#define BUF(a, b) ((Buffer){(uint8_t *)(a), ((uint8_t *)b)})

handle flux_dial0(const uint8_t *addr, enum FluxSockType *fst, int64_t deadline) {
	return flux_dial(addr, flux_bufend(BUF(addr, addr + 256)), fst, deadline);
}

handle flux_dial(const uint8_t *_addr, const uint8_t *eaddr, enum FluxSockType *fst, int64_t deadline) {
	uint8_t *idx, parts;
	Buffer part[3];
	Buffer addr = BUF(_addr, eaddr);

	if (isbadbuffer(addr)) {
		goto error_inval;
	}

	idx = flux_bufindex(addr, '!');

	if (NULL == idx) {
		goto error_inval;
	}

	part[0].start	= addr.start;
	part[0].end	= idx;
	part[1].start	= idx + 1;
	part[1].end	= addr.end;

	idx = flux_bufindex(part[1], '!');

	if (NULL == idx) {
		parts = 2;
	} else {
		parts = 3;
		part[1].end	= idx;
		part[2].start	= idx + 1;
		part[2].end	= addr.end;

		idx = flux_bufindex(part[2], '!');

		// We should not have a third '!'
		if (NULL != idx) {
			goto error_inval;
		}
	}

#define match(str) (0 == flux_bufcmp(part[0], BUFLIT(str), NULL))

	if (match("ns")) {
		if (2 != parts) {
			goto error_inval;
		}

		return flux_nsconnect(part[1].start, part[1].end, deadline);
	} else if (match("tcp")) {
		if (3 != parts) {
			goto error_inval;
		}

		return flux_tcpconnect_s(part[1].start, part[1].end, part[2].start, part[2].end, deadline);
	}

	// ns!plumb
	// unix!/tmp/ns.USER.:0/plumb
	// tcp!127.1!5555
	// udp!10.2.0.0!4444

	error_inval:
	errno	= EINVAL;
	return -1;
}

handle flux_nsconnect(const uint8_t *addr, const uint8_t *eaddr, int64_t deadline) {
	uint8_t _b[512];
	Buffer b = BUFLIT(_b), p;

	if (!*NAMESPACE.start) {
		getns();
	}

	if (!*NAMESPACE.start) {
		errno = 1; // TODO
		return -1;
	}

	p = flux_bufcpy(b, NAMESPACE);

	if (p.start == b.end) {
		errno = ENOMEM; // TODO
		return -1;
	}

	p = bufcontinue(p, b);

	*p.start++ = '/';

	flux_buflcpy(p, BUF(addr, eaddr));

	return ipc_connect(b.start, deadline);
}

handle flux_tcpconnect_s(const uint8_t *addr, const uint8_t *eaddr, uint8_t *port, uint8_t *eport, int64_t deadline) {
	return -1;
}
