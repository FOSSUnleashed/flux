#include <dill/ip.h>
#include <dill/tcp.h>
#include <dill/bsock.h>
#include <dill/util.h>

#include <string.h>

#define end(cond) if (cond) { goto done; }

#define r2(buf) ((buf)[0] + ((buf)[1] << 8))

int ipaddr_jank(struct ipaddr* addr, const char* name, int port, int64_t deadline) {
	char buf[1 << 10], newname[1 << 6];
	struct ipaddr a;
	int rc, len;
	handle sock = -1;

	rc = ipaddr_remote(&a, "10.4.5.6", 1053, IPADDR_IPV4, deadline);

	end (0 != rc);

	sock = tcp_connect(&a, deadline);

	if (-1 == sock) {
		rc = -1;
		goto done;
	}

	len = strlen(name);

	buf[0] = 1;
	buf[1] = 0;
	buf[2] = 'A';
	buf[3] = len;
	buf[4] = 0;

	memcpy(buf + 5, name, len);

	rc = bsend(sock, buf, 5 + len, deadline);

	end (-1 == rc);

	rc = brecv(sock, buf, 2, deadline);

	end (-1 == rc);

	len = r2(buf);

	rc = brecv(sock, buf, len - 2, deadline);

	uint16_t count, sz;

	count = r2(buf);
	sz	= r2(buf + 2);

	memcpy(newname, buf + 4, sz);
	newname[sz] = 0;

	rc = ipaddr_remote(addr, newname, port, IPADDR_IPV4, deadline);

done:
	if (-1 != sock) {
		tcp_close(sock, deadline);
	}

	return rc;
}

int ipaddr_remote4(struct ipaddr* addr, const char* name, int port, int64_t deadline) {
	int rc;

	rc = ipaddr_remote(addr, name, port, IPADDR_IPV4, deadline);

	if (0 == rc) {
		goto done;
	}

	rc = ipaddr_jank(addr, name, port, deadline);

done:
	return rc;
}
