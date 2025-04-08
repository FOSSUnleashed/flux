#include <dill/all.h>
#include <stdio.h>
#include <assert.h>
#include <flux/str.h>
#include <stdlib.h>
#include <string.h>

Buffer getDbusAddress(Buffer addr) {
	uint8_t *idx;
	Buffer e, f;

	e = envbuf("DBUS_SESSION_BUS_ADDRESS");

	assert(NULL != e.start);

	forever {
		idx	= flux_bufindex(e, '=');

		if (idx >= e.end || NULL == idx) {
			return (Buffer){NULL, NULL};
		}

		f.end = idx;
		f.start	= e.start;

		if (0 == flux_bufcmp(f, BUFLIT("unix:path"), NULL)) {
			e.start	= idx + 1;
			idx = flux_bufindex(e, ',');

			if (idx != NULL) {
				e.end = idx;
			}

			return flux_bufcpy(addr, e);
		}
	}
}

#define MSZ (1 << 11)

ssize_t breadline(handle h, uint8_t *b, uint8_t *be, int64_t deadline) {
	uint8_t *p = b;
	int rc;

	while (p + 3 < be) {
		rc = brecv(h, p, 2, deadline);

		if (-1 == rc) {
			return -1;
		}

		if ('\r' == *p && '\n' == p[1]) {
			p += 2;
			goto done;
		} else while ('\r' == p[1]) {
			rc = brecv(h, p + 2, 1, deadline);

			if ('\n' == p[2]) {
				p += 3;
				goto done;
			}
			p++;
		}
		p += 2;
	}

	done:
	return p - b;
}

int main(int argc, char **argv) {
	handle dbus;
	uint8_t _addr[512], _b[MSZ];
	Buffer addr = BUFLIT(_addr), b = BUFLIT(_b);
	int rc;

	addr = getDbusAddress(addr);

	assert(NULL != addr.end);

	printf("%.*s\n", addr.end - addr.start, addr.start);

	*addr.end = 0;

	dbus = ipc_connect(addr.start, -1);

	assert(-1 != dbus);

	// handshake Client sends NUL

	*b.start = 0;

	rc = bsend(dbus, b.start, 1, -1); assert(-1 != rc);

	flux_bufcpy(b, BUFLIT("AUTH EXTERNAL 3130303030\r\n"));

	rc = bsend(dbus, b.start, 26, -1); assert(-1 != rc);

	rc = breadline(dbus, b.start, b.end, -1); assert(-1 != rc);

	printf("%.*s\n", rc - 2, b.start);

	return 0;
}
