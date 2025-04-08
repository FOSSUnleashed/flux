#include <dill/all.h>
#include <stdio.h>
#include <assert.h>
#include <flux/str.h>

#define MSZ (1 << 13)

int i = 0;

coroutine void proxy(handle ag, handle cli) {
	uint8_t buffer[MSZ], out[MSZ], *oe = out + MSZ;
	FILE *fp;
	ssize_t sz, rc;

	sprintf(buffer, "log_%04d.txt", ++i);

	fp = fopen(buffer, "w");

	assert(NULL != fp);

	forever {
		// CLIENT -> AGENT
		sz = mrecv(cli, buffer, MSZ, -1);

		if (-1 == sz) {
			printf("ERR: %d\n", errno);
			break;
		}

		oe = flux_bufdump(out, out + MSZ, buffer, buffer + sz);
		printf("%.*s", oe - out, out);
		rc = fwrite(buffer, sz, 1, fp);
		fflush(fp);

		sz = msend(ag, buffer, sz, -1);

		if (-1 == sz) {
			printf("ERR: %d\n", errno);
			break;
		}

		// AGENT -> CLIENT
		sz = mrecv(ag, buffer, MSZ, -1);

		if (-1 == sz) {
			printf("ERR: %d\n", errno);
			break;
		}

		oe = flux_bufdump(out, out + MSZ, buffer, buffer + sz);
		printf("%.*s", oe - out, out);
		rc = fwrite(buffer, sz, 1, fp);
		fflush(fp);

		sz = msend(cli, buffer, sz, -1);

		if (-1 == sz) {
			printf("ERR: %d\n", errno);
			break;
		}
	}
}

int main(int argc, char **argv) {
	handle srv, ag, cli;

	srv = ipc_listen("sock", 10);
	assert(-1 != srv);

	ag = ipc_connect("/tmp/ssh-XXXXXXZMz6hd/agent.28544", -1);
	assert(-1 != ag);

	ag = prefix_attach(ag, 4, PREFIX_BIG_ENDIAN);
	assert(-1 != ag);

	forever {
		cli = ipc_accept(srv, -1);
		assert(-1 != cli);

		cli = prefix_attach(cli, 4, PREFIX_BIG_ENDIAN);
		assert(-1 != cli);

		go(proxy(ag, cli));
	}

	return 0;
}
