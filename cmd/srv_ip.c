#include <dill/all.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

int main(int argc, char **argv) {
	handle srv, cli;
	struct ipaddr addr, caddr;
	uint8_t buffer[64];
	size_t sz;
	int rc;

	ipaddr_local(&addr, "0.0.0.0", 1121, IPADDR_IPV4);

	srv = tcp_listen(&addr, 10);

	if (-1 == srv) {
		printf("Could not setup listen socket: %d\n", errno);
		return 1;
	}

	forever {
		cli = tcp_accept(srv, &caddr, -1);

		if (-1 == cli) {
			printf("Could not accept client: %d\n", cli);
			continue;
		}

		ipaddr_str(&caddr, buffer);

		sz = strlen(buffer);
		buffer[sz] = '\n';
		sz++;

		bsend(cli, buffer, sz, -1);

		hclose(cli);
	}

	return 0;
}
